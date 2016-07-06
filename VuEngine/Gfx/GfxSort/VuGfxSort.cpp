//*****************************************************************************
//
//  Copyright (c) 2008-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  GfxSort class
// 
//*****************************************************************************

#include "VuGfxSort.h"
#include "VuGfxSortDevStat.h"
#include "VuEngine/HAL/Gfx/VuGfx.h"
#include "VuEngine/HAL/Gfx/VuShaderProgram.h"
#include "VuEngine/HAL/Gfx/VuPipelineState.h"
#include "VuEngine/HAL/Gfx/VuDepthStencilState.h"
#include "VuEngine/HAL/Gfx/VuTexture.h"
#include "VuEngine/HAL/Thread/VuThread.h"
#include "VuEngine/Gfx/VuGfxUtil.h"
#include "VuEngine/Gfx/VuGfxSettings.h"
#include "VuEngine/Gfx/Camera/VuCamera.h"
#include "VuEngine/Gfx/GfxComposer/VuGfxComposer.h"
#include "VuEngine/Gfx/Light/VuLightManager.h"
#include "VuEngine/Water/VuWater.h"
#include "VuEngine/Util/VuRadixSort.h"
#include "VuEngine/Memory/VuScratchPad.h"
#include "VuEngine/Managers/VuTickManager.h"
#include "VuEngine/Dev/VuDevStat.h"
#include "VuEngine/Dev/VuDevProfile.h"


// the interface
IMPLEMENT_SYSTEM_COMPONENT(VuGfxSort, VuGfxSort);

// static camera to be accessed during rendering
static VuCamera sRenderCamera;

// static gfx settings to be accessed during rendering
static VuGfxSettings sRenderGfxSettings;

// translucency render state lookup
struct VuTranslucencyRenderState
{
	VuGfxCompFunc	mDepthCompFunc;
	bool			mDepthWriteEnabled;
};
static VuTranslucencyRenderState sTranslucencyRenderStates[] =
{
	{ VUGFX_COMP_LESS,      true  }, // TRANS_BEGIN,
	{ VUGFX_COMP_LESS,      true  }, // TRANS_OPAQUE,
	{ VUGFX_COMP_LESS,      true  }, // TRANS_ALPHA_TEST,
	{ VUGFX_COMP_LESS,      true  }, // TRANS_FOLIAGE,
	{ VUGFX_COMP_LESS,      false }, // TRANS_SKYBOX,
	{ VUGFX_COMP_LESS,      false }, // TRANS_TIRE_TRACK,
	{ VUGFX_COMP_LESSEQUAL, false }, // TRANS_BLOB_SHADOW,
	{ VUGFX_COMP_LESS,      false }, // TRANS_MODULATE_BELOW_WATER,
	{ VUGFX_COMP_LESS,      false }, // TRANS_ADDITIVE_BELOW_WATER,
	{ VUGFX_COMP_LESS,      false }, // TRANS_WATER_COLOR,
	{ VUGFX_COMP_LESS,      true  }, // TRANS_DEPTH_PASS,
	{ VUGFX_COMP_EQUAL,     false }, // TRANS_COLOR_PASS,
	{ VUGFX_COMP_LESS,      false }, // TRANS_MODULATE_ABOVE_WATER,
	{ VUGFX_COMP_LESS,      false }, // TRANS_ADDITIVE_ABOVE_WATER,
	{ VUGFX_COMP_LESS,      true  }, // TRANS_WATER_DEPTH,
	{ VUGFX_COMP_LESS,      false }, // TRANS_MODULATE_CLIP_WATER,
	{ VUGFX_COMP_LESS,      false }, // TRANS_ADDITIVE_CLIP_WATER,
	{ VUGFX_COMP_ALWAYS,    false }, // TRANS_UI_OPAQUE,
	{ VUGFX_COMP_ALWAYS,    false }, // TRANS_UI_MODULATE,
	{ VUGFX_COMP_ALWAYS,    false }, // TRANS_UI_ADDITIVE,
	{ VUGFX_COMP_LESS,      true  }, // TRANS_END,
};
VU_COMPILE_TIME_ASSERT(sizeof(sTranslucencyRenderStates)/sizeof(sTranslucencyRenderStates[0]) == VuGfxSort::TRANS_END + 1);


//*****************************************************************************
inline static int CompareMaterials(const VuPipelineState *pPS0, VUUINT32 constCrc0, VUUINT32 texCrc0, const VuPipelineState *pPS1, VUUINT32 constCrc1, VUUINT32 texCrc1)
{
	if ( pPS0 != pPS1 )
		return pPS0 < pPS1 ? -1 : 1;

	if ( constCrc0 != constCrc1 )
		return constCrc0 < constCrc1 ? -1 : 1;

	if ( texCrc0 != texCrc1 )
		return texCrc0 < texCrc1 ? -1 : 1;

	return 0;
}

//*****************************************************************************
VuGfxSort::VuGfxSort():
	mSuspendedCount(0),
	mhThread(VUNULL),
	mbWorkerThreadActive(false),
	mMaterialCount(0),
	mMeshCount(0),
	mCommandMemoryOffset(0),
	mCurSubmitBuffer(0),
	mCurRenderBuffer(1),
	mCurSubmissionLayerKey(0),
	mCurRenderLayerKey(0),
	mTime(0),
	mbTerminateThread(false),
	mpDevStat(VUNULL),
	mDepthStencilStates(0)
{
	memset(&mStatsCur, 0, sizeof(mStatsCur));
	memset(&mStatsPrev, 0, sizeof(mStatsPrev));

	mKickWorkEvent = VuThread::IF()->createEvent();
	mWorkCompletedEvent = VuThread::IF()->createEvent();
}

//*****************************************************************************
VuGfxSort::~VuGfxSort()
{
	VuThread::IF()->destroyEvent(mKickWorkEvent);
	VuThread::IF()->destroyEvent(mWorkCompletedEvent);
}

//*****************************************************************************
bool VuGfxSort::init(bool bAsynchronous)
{
	// reserve some space
	mMaterials.reserve(512);
	mMeshes.reserve(512);
	mCommandMemory[0].reserve(512*1024);
	mCommandBuffer[0].reserve(8*1024);
	mCommandMemory[1].reserve(512*1024);
	mCommandBuffer[1].reserve(8*1024);
	mSortedKeys.reserve(8*1024);
	mKeySortOrder.reserve(8*1024);
	
	if ( bAsynchronous )
		mhThread = VuThread::IF()->createThread(threadProc, this);

	return true;
}

//*****************************************************************************
void VuGfxSort::postInit()
{
	if ( VuDevStat::IF() )
		mpDevStat = new VuGfxSortDevStat;

	// dev stats
	if ( VuDevStat::IF() )
		VuDevStat::IF()->addPage("GfxSort", VuRect(10, 10, 80, 80));

	// register phased ticks
	VuTickManager::IF()->registerHandler(this, &VuGfxSort::tickDecision, "Decision");

	// create depth-stencils states
	if ( VuGfx::IF() )
	{
		int numDepthStencilStates = sizeof(sTranslucencyRenderStates)/sizeof(sTranslucencyRenderStates[0]);
		mDepthStencilStates.resize(numDepthStencilStates);
		for ( int i = 0; i < numDepthStencilStates; i++ )
		{
			VuDepthStencilStateParams dssParams;
			dssParams.mDepthCompFunc = sTranslucencyRenderStates[i].mDepthCompFunc;
			dssParams.mDepthWriteEnabled = sTranslucencyRenderStates[i].mDepthWriteEnabled;
			mDepthStencilStates[i] = VuGfx::IF()->createDepthStencilState(dssParams);
		}
	}
}

//*****************************************************************************
void VuGfxSort::preRelease()
{
	flush();

	delete mpDevStat;

	// unregister phased ticks
	VuTickManager::IF()->unregisterHandlers(this);

	// release depth-stencils states
	for ( int i = 0; i < mDepthStencilStates.size(); i++ )
		mDepthStencilStates[i]->removeRef();
	mDepthStencilStates.clear();
}

//*****************************************************************************
void VuGfxSort::release()
{
	flush();

	if ( mhThread )
	{
		VUPRINTF("Terminating VuGfxSort thread...\n");
		mbTerminateThread = true;
		VuThread::IF()->setEvent(mKickWorkEvent);
		VuThread::IF()->joinThread(mhThread);
	}
}

//*****************************************************************************
void VuGfxSort::draw()
{
	if ( VuDevProfile::IF() )
		VuDevProfile::IF()->endSim();

	// synchronize with thread
	flush();

	// dev stats
	if ( mpDevStat )
		printDevStats();

	VuGfx::IF()->printStats();
	VuGfx::IF()->resetStats();

	// swap buffers
	mCurSubmitBuffer = !mCurSubmitBuffer;
	mCurRenderBuffer = !mCurRenderBuffer;

	// synchronize other systems
	if ( VuDevProfile::IF() )
		VuDevProfile::IF()->synchronizeGfx();

	if ( VuLightManager::IF() )
		VuLightManager::IF()->synchronize();

	if ( VuWater::IF() )
		VuWater::IF()->renderer()->synchronize();

	if ( VuGfxComposer::IF() )
		VuGfxComposer::IF()->synchronizeGfx();

	// kick off next frame
	if ( !mSuspendedCount )
	{
		VuGfx::IF()->syncPreDraw();

		if ( mhThread )
		{
			VuGfx::IF()->releaseThreadOwnership();
			mbWorkerThreadActive = true;
			VuThread::IF()->setEvent(mKickWorkEvent);
		}
		else
		{
			drawFrame();
			VuGfx::IF()->syncPostDraw();
		}
	}

	// reset command memory
	mCommandMemoryOffset = 0;
	mCommandMemory[mCurSubmitBuffer].clear();
	mCommandBuffer[mCurSubmitBuffer].clear();

	if ( VuDevProfile::IF() )
		VuDevProfile::IF()->beginSim();
}

//*****************************************************************************
void VuGfxSort::suspend()
{
	mSuspendedCount++;
}

//*****************************************************************************
void VuGfxSort::resume()
{
	mSuspendedCount--;

	VUASSERT(mSuspendedCount >= 0, "VuGfxSort::suspend/resume mismatch");
}

//*****************************************************************************
void VuGfxSort::flush()
{
	if ( mbWorkerThreadActive )
	{
		VuThread::IF()->waitForSingleObject(mWorkCompletedEvent);
		VuGfx::IF()->acquireThreadOwnership();
		mbWorkerThreadActive = false;

		VuGfx::IF()->syncPostDraw();
	}
}

//*****************************************************************************
VuGfxSortMaterial *VuGfxSort::createMaterial(VuPipelineState *pPipelineState, const VuGfxSortMaterialDesc &desc)
{
	// calculate checksums
	VUUINT32 constHash = desc.mConstantArray.calcHash();
	VUUINT32 texHash = desc.mTextureArray.calcHash();

	int insertPos = 0;
	if ( mMaterials.size() )
	{
		// perform binary search to find material
		int start = 0, end = mMaterials.size();
		while ( start < end )
		{
			int middle = (start + end)>>1;
			VuGfxSortMaterial *pMat = mMaterials[middle];

			int result = CompareMaterials(pMat->mpPipelineState, pMat->mConstHash, pMat->mTexHash, pPipelineState, constHash, texHash);
			if ( result < 0 )
			{
				end = middle;
			}
			else if ( result > 0 )
			{
				start = middle + 1;
			}
			else
			{
				// found it!
				pMat->addRef();
				return pMat;
			}
		}
		insertPos = start;
	}

	// create new material
	VUASSERT(mMaterialCount < GFX_SORT_MAX_MATERIAL_COUNT, "Exceeded max # sort materials");
	VuGfxSortMaterial *pMat = new VuGfxSortMaterial(pPipelineState, desc);
	if ( VuGfxComposer::IF() )
		pMat->mpMaterialExtension = VuGfxComposer::IF()->createMaterialExt(pMat);
	pMat->mConstHash = constHash;
	pMat->mTexHash = texHash;

	// insert into array
	mMaterials.resize(mMaterials.size() + 1);
	for ( int i = mMaterials.size() - 1; i > insertPos; i-- )
		mMaterials[i] = mMaterials[i-1];
	mMaterials[insertPos] = pMat;

	// increment count
	mMaterialCount++;

	// recalculate sort keys
	for ( int i = 0; i < mMaterials.size(); i++ )
		mMaterials[i]->mSortKey = i;

	return pMat;
}

//*****************************************************************************
VuGfxSortMaterial *VuGfxSort::duplicateMaterial(VuGfxSortMaterial *pMaterial)
{
	pMaterial->addRef();

	return pMaterial;
}

//*****************************************************************************
void VuGfxSort::releaseMaterial(VuGfxSortMaterial *pMaterial)
{
	if ( pMaterial )
	{
		pMaterial->removeRef();
		if ( pMaterial->refCount() == 0 )
		{
			flush();

			mMaterials.remove(pMaterial);
			delete pMaterial;

			// decrement count
			mMaterialCount--;
		}
	}
}

//*****************************************************************************
VuGfxSortMesh *VuGfxSort::createMesh(const VuGfxSortMeshDesc &desc)
{
	int insertPos = 0;
	if ( mMeshes.size() )
	{
		// perform binary search to find material
		int start = 0, end = mMeshes.size();
		while ( start < end )
		{
			int middle = (start + end)>>1;
			VuGfxSortMesh *pMesh = mMeshes[middle];

			if ( desc < pMesh->mDesc )
			{
				end = middle;
			}
			else if ( pMesh->mDesc < desc )
			{
				start = middle + 1;
			}
			else
			{
				// found it!
				pMesh->addRef();
				return pMesh;
			}
		}
		insertPos = start;
	}

	// create new mesh
	VUASSERT(mMeshCount < GFX_SORT_MAX_MESH_COUNT, "Exceeded max # sort meshes");
	VuGfxSortMesh *pMesh = new VuGfxSortMesh(desc);

	// insert into array
	mMeshes.resize(mMeshes.size() + 1);
	for ( int i = mMeshes.size() - 1; i > insertPos; i-- )
		mMeshes[i] = mMeshes[i-1];
	mMeshes[insertPos] = pMesh;

	// increment count
	mMeshCount++;

	// recalculate sort keys
	for ( int i = 0; i < mMeshes.size(); i++ )
		mMeshes[i]->mSortKey = i;

	return pMesh;
}

//*****************************************************************************
VuGfxSortMesh *VuGfxSort::duplicateMesh(VuGfxSortMesh *pMesh)
{
	pMesh->addRef();

	return pMesh;
}

//*****************************************************************************
void VuGfxSort::releaseMesh(VuGfxSortMesh *pMesh)
{
	if ( pMesh )
	{
		pMesh->removeRef();
		if ( pMesh->refCount() == 0 )
		{
			flush();

			mMeshes.remove(pMesh);
			delete pMesh;

			// decrement count
			mMeshCount--;
		}
	}
}

//*****************************************************************************
void VuGfxSort::submitCamera(const VuCamera &camera, int sequenceNo)
{
	struct CommandData
	{
		static void callback(void *data)
		{
			CommandData *pData = static_cast<CommandData *>(data);

			sRenderCamera = pData->mCamera;
		}
		VuCamera	mCamera;
	};

	CommandData *pData = static_cast<CommandData *>(VuGfxSort::IF()->allocateCommandMemory(sizeof(CommandData)));
	pData->mCamera = camera;

	VUUINT prevVPL = VuGfxSort::IF()->getViewportLayer();
	setViewportLayer(VuGfxSort::VPL_BEGIN);
	submitCommand(VuGfxSort::TRANS_BEGIN, sequenceNo, &CommandData::callback);
	setViewportLayer(prevVPL);
}

//*****************************************************************************
const VuCamera &VuGfxSort::getRenderCamera()
{
	return sRenderCamera;
}

//*****************************************************************************
void VuGfxSort::submitGfxSettings(const VuGfxSettings &settings)
{
	struct CommandData
	{
		static void callback(void *data)
		{
			CommandData *pData = static_cast<CommandData *>(data);

			sRenderGfxSettings = pData->mGfxSettings;
		}
		VuGfxSettings	mGfxSettings;
	};

	CommandData *pData = static_cast<CommandData *>(VuGfxSort::IF()->allocateCommandMemory(sizeof(CommandData)));
	pData->mGfxSettings = settings;

	VUUINT prevRL = VuGfxSort::IF()->getReflectionLayer();
	VUUINT prevVPL = VuGfxSort::IF()->getViewportLayer();
	setReflectionLayer(0);
	setViewportLayer(0);
	submitCommand(0, 0, &CommandData::callback);
	setReflectionLayer(prevRL);
	setViewportLayer(prevVPL);
}

//*****************************************************************************
const VuGfxSettings &VuGfxSort::getRenderGfxSettings()
{
	return sRenderGfxSettings;
}

//*****************************************************************************
void VuGfxSort::threadProc()
{
	VuThread::IF()->setThreadProcessor(2);

	VUPRINTF("VuGfxSort thread starting...\n");

	for (;;)
	{
		if ( !VuThread::IF()->waitForSingleObject(mKickWorkEvent) )
		{
			VUPRINTF("VuGfxSort::threadProc() wait error!\n");
			break;
		}
		if ( mbTerminateThread )
		{
			VUPRINTF("VuGfxSort thread exiting...\n");
			break;
		}

		// do work
		VuGfx::IF()->acquireThreadOwnership();
		{
			drawFrame();
		}
		VuGfx::IF()->releaseThreadOwnership();

		VuThread::IF()->setEvent(mWorkCompletedEvent);
	}

	VuThread::IF()->endThread();
}

//*****************************************************************************
void VuGfxSort::tickDecision(float fdt)
{
	mTime += fdt;
}

//*****************************************************************************
void VuGfxSort::drawFrame()
{
	if ( VuDevProfile::IF() )
		VuDevProfile::IF()->beginGfx();

	// sort commands
	sortCommands();

	// sort and submit commands
	submitCommands();

	if ( VuDevProfile::IF() )
		VuDevProfile::IF()->endGfx();
}

//*****************************************************************************
void VuGfxSort::sortCommands()
{
	VU_PROFILE_GFX("Sort");

	int count = mCommandBuffer[mCurRenderBuffer].size();

	mSortedKeys.resize(count);
	mKeySortOrder.resize(count);

	// pack keys
	VuCommand *pSrc = &mCommandBuffer[mCurRenderBuffer].begin();
	VUUINT64 *pDst = &mSortedKeys.begin();
	for ( int i = 0; i < count; i++ )
	{
		*pDst = pSrc->mSortKey;
		pSrc++;
		pDst++;
	}

	// key sort must start in order
	for ( int i = 0; i < mKeySortOrder.size(); i++ )
		mKeySortOrder[i] = i;

	// radix sort
	VuRadixSort::sort(&mSortedKeys.begin(), count, &mKeySortOrder.begin(), VuScratchPad::SIZE, VuScratchPad::get(VuScratchPad::GRAPHICS));
}

//*****************************************************************************
void VuGfxSort::submitCommands()
{
	VU_PROFILE_GFX("Submit");

	mStatsPrev = mStatsCur;
	memset(&mStatsCur, 0, sizeof(mStatsCur));

	VuGfxSortMaterial *pCurMaterial = VUNULL;
	VuGfxSortMesh *pCurMesh = VUNULL;
	VUUINT16 curTranslucencyType = TRANS_BEGIN;

	for ( int *order = &mKeySortOrder.begin(); order != &mKeySortOrder.end(); order++ )
	{
		VuCommand *pCmd = &mCommandBuffer[mCurRenderBuffer][*order];
		if ( pCmd->mpMaterial != pCurMaterial )
		{
			if ( pCmd->mpMaterial )
			{
				mStatsCur.mMaterialChanges++;
				changeMaterial(pCurMaterial, pCmd->mpMaterial);
			}
			pCurMaterial = pCmd->mpMaterial;
		}

		if ( pCmd->mpMesh != pCurMesh )
		{
			if ( pCmd->mpMesh )
			{
				mStatsCur.mMeshChanges++;
				changeMesh(pCurMesh, pCmd->mpMesh);
			}
			pCurMesh = pCmd->mpMesh;
		}

		if ( curTranslucencyType != pCmd->mTranslucencyType )
		{
			curTranslucencyType = pCmd->mTranslucencyType;
			VuGfx::IF()->setDepthStencilState(mDepthStencilStates[curTranslucencyType]);
		}

		mCurRenderLayerKey = pCmd->mSortKey;
#ifdef VUDEBUG
		mDebugRenderScreen = getRenderScreen();
		mDebugRenderFullScreenLayer = (eFullScreenLayer)getRenderFullScreenLayer();
		mDebugRenderViewport = getRenderViewport();
		mDebugRenderReflectionLayer = (eReflectionLayer)getRenderReflectionLayer();
		mDebugRenderViewportLayer = (eViewportLayer)getRenderViewportLayer();
		mDebugRenderTranslucencyType = (eTranslucencyType)getRenderTranslucencyType();
#endif
		{
			VU_PROFILE_GFX("Draw");
			pCmd->mCallback(&mCommandMemory[mCurRenderBuffer].begin() + pCmd->mCallbackDataOffset);
		}
	}
}

//*****************************************************************************
void VuGfxSort::changeMaterial(VuGfxSortMaterial *pOldMat, VuGfxSortMaterial *pNewMat)
{
	VuPipelineState *pOldPS = VUNULL;
	VUUINT32 oldConstHash = 0;
	VUUINT32 oldTextureHash = 0;
	if ( pOldMat )
	{
		pOldPS = pOldMat->mpPipelineState;
		oldConstHash = pOldMat->mConstHash;
		oldTextureHash = pOldMat->mTexHash;
	}

	VuPipelineState *pNewPS = pNewMat->mpPipelineState;
	VUUINT32 newConstHash = pNewMat->mConstHash;
	VUUINT32 newTextureHash = pNewMat->mTexHash;

	if ( pNewPS != pOldPS )
	{
		mStatsCur.mPSChanges++;
		VuGfx::IF()->setPipelineState(pNewPS);

		// constants
		setGlobalConstants(pNewMat);

		mStatsCur.mConstChanges++;
		pNewMat->setConstants();

		mStatsCur.mTextureChanges++;
		pNewMat->setTextures();
	}
	else
	{
		if ( newConstHash != oldConstHash )
		{
			mStatsCur.mConstChanges++;
			pNewMat->setConstants();
		}

		if ( newTextureHash != oldTextureHash )
		{
			mStatsCur.mTextureChanges++;
			pNewMat->setTextures();
		}
	}
}

//*****************************************************************************
void VuGfxSort::changeMesh(VuGfxSortMesh *pOldMesh, VuGfxSortMesh *pNewMesh)
{
	VuVertexBuffer *pOldVB = VUNULL;
	VuIndexBuffer *pOldIB = VUNULL;
	if ( pOldMesh )
	{
		pOldVB = pOldMesh->mDesc.mpVertexBuffer;
		pOldIB = pOldMesh->mDesc.mpIndexBuffer;
	}

	VuVertexBuffer *pNewVB = pNewMesh->mDesc.mpVertexBuffer;
	VuIndexBuffer *pNewIB = pNewMesh->mDesc.mpIndexBuffer;

	if ( pNewVB != pOldVB )
	{
		VuGfx::IF()->setVertexBuffer(pNewVB);
		mStatsCur.mVBChanges++;
	}

	if ( pNewIB != pOldIB )
	{
		VuGfx::IF()->setIndexBuffer(pNewIB);
		mStatsCur.mIBChanges++;
	}
}

//*****************************************************************************
void VuGfxSort::setGlobalConstants(const VuGfxSortMaterial *pMat)
{
	VuShaderProgram *pSP = pMat->mpShaderProgram;

	#define SET_CONST(handle, type, value) if ( handle ) pSP->setConstant##type(handle, value);

	// camera
	SET_CONST(pMat->mhSpConstViewMatrix,		Matrix,		sRenderCamera.getViewMatrix());
	SET_CONST(pMat->mhSpConstViewProjMatrix,	Matrix,		sRenderCamera.getViewProjMatrix());
	SET_CONST(pMat->mhSpConstEyeWorld,		Vector3,	sRenderCamera.getEyePosition());
	SET_CONST(pMat->mhSpConstFarPlane,		Float,		sRenderCamera.getFarPlane());

	// misc
	SET_CONST(pMat->mhSpConstTime,			Float,		mTime);

	// ambient light
	SET_CONST(pMat->mhSpConstAmbLightColor, Color3, VuLightManager::IF()->ambientLight().mColor);

	// directional light
	const VuDirectionalLight *pDirLight = &VuLightManager::IF()->directionalLight();
	SET_CONST(pMat->mhSpConstDirLightWorld,			Vector3,	pDirLight->mDirection);
	SET_CONST(pMat->mhSpConstDirLightFrontColor,		Color3,		pDirLight->mFrontColor);
	SET_CONST(pMat->mhSpConstDirLightBackColor,		Color3,		pDirLight->mBackColor);
	SET_CONST(pMat->mhSpConstDirLightSpecularColor,	Color3,		pDirLight->mSpecularColor);

	// fog
	SET_CONST(pMat->mhSpConstFogStart,		Float,		sRenderGfxSettings.mFogStart);
	SET_CONST(pMat->mhSpConstFogInvRange,	Float,		1.0f/(sRenderGfxSettings.mFogEnd - sRenderGfxSettings.mFogStart));
	SET_CONST(pMat->mhSpConstFogColor,		Color3,		sRenderGfxSettings.mFogColor);

	// depth fog
	SET_CONST(pMat->mhPsConstDepthFogStart,			Float,		sRenderGfxSettings.mDepthFogStart);
	SET_CONST(pMat->mhPsConstDepthFogInvRange,		Float,		1.0f/sRenderGfxSettings.mDepthFogDist);
	SET_CONST(pMat->mhPsConstDepthFogColor,			Color3,		sRenderGfxSettings.mDepthFogColor);
	
	// clip
	SET_CONST(pMat->mhSpConstClipPlane,		Vector4,	VuGfx::IF()->getClipPlane());

	if ( pMat->mpMaterialExtension && VuGfxComposer::IF() )
		VuGfxComposer::IF()->setGlobalConstants(pMat->mpMaterialExtension, pSP);
}

//*****************************************************************************
void VuGfxSort::printDevStats()
{
	bool bVisible = false;
	if ( VuDevStatPage *pPage = VuDevStat::IF()->getCurPage() )
	{
		if ( pPage->getName() == "GfxSort" )
		{
			pPage->clear();

			pPage->printf("Material Count: %d\n", mMaterialCount);
			pPage->printf("Mesh Count: %d\n", mMeshCount);
			pPage->printf("Command Entries: %d\n", mCommandBuffer[mCurSubmitBuffer].size());
			pPage->printf("Command Memory: %dK\n", mCommandMemory[mCurSubmitBuffer].size()/1024);
			pPage->printf("Material Changes: %d\n", mStatsPrev.mMaterialChanges);
			pPage->printf("Mesh Changes: %d\n", mStatsPrev.mMeshChanges);
			pPage->printf("PipelineState Changes: %d\n", mStatsPrev.mPSChanges);
			pPage->printf("Const Changes: %d\n", mStatsPrev.mConstChanges);
			pPage->printf("Texture Changes: %d\n", mStatsPrev.mTextureChanges);
			pPage->printf("VertexBuffer Changes: %d\n", mStatsPrev.mVBChanges);
			pPage->printf("IndexBuffer Changes: %d\n", mStatsPrev.mIBChanges);

			mpDevStat->print(pPage, mSortedKeys);

			bVisible = true;
		}
	}
	mpDevStat->enable(bVisible);
}