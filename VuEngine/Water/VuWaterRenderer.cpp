//*****************************************************************************
//
//  Copyright (c) 2008-2008 Ralf Knoesel
//  Copyright (c) 2008-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  Water renderer implementation
// 
//*****************************************************************************

#include "VuWaterRenderer.h"
#include "VuWaterSurface.h"
#include "VuWaterShader.h"
#include "VuWaterTexture.h"
#include "VuWater.h"
#include "VuEngine/Assets/VuWaterMapAsset.h"
#include "VuEngine/Assets/VuLightMapAsset.h"
#include "VuEngine/Gfx/Camera/VuCamera.h"
#include "VuEngine/Managers/VuViewportManager.h"
#include "VuEngine/HAL/Gfx/VuGfx.h"
#include "VuEngine/HAL/Thread/VuThread.h"
#include "VuEngine/Gfx/Shaders/VuBasicShaders.h"
#include "VuEngine/Gfx/GfxSort/VuGfxSort.h"
#include "VuEngine/Gfx/VuGfxUtil.h"
#include "VuEngine/Math/VuRect.h"
#include "VuEngine/Math/VuPackedVector.h"
#include "VuEngine/Dev/VuDev.h"
#include "VuEngine/Dev/VuDevStat.h"
#include "VuEngine/Dev/VuDevMenu.h"
#include "VuEngine/Dev/VuDevProfile.h"
#include "VuEngine/Memory/VuScratchPad.h"
#include "VuEngine/Util/VuImageUtil.h"


// constants

#define INITIAL_MAX_PATCH_COUNT 512


// static functions

struct WaterRendererDrawData
{
	static void colorCallback(void *data)
	{
		WaterRendererDrawData *pData = static_cast<WaterRendererDrawData *>(data);
		pData->mpRenderer->drawColor(pData);
	}
	VuWaterRenderer			*mpRenderer;
	const VuWaterShader		*mpShader;
	const VuWaterSurface	*mpSurface;
};

static int PatchComp(const void *p1, const void *p2)
{
	const VuWaterRenderer::VuPatch *pPatch1 = static_cast<const VuWaterRenderer::VuPatch *>(p1);
	const VuWaterRenderer::VuPatch *pPatch2 = static_cast<const VuWaterRenderer::VuPatch *>(p2);

	if ( pPatch1->mDist > pPatch2->mDist ) return -1;
	if ( pPatch1->mDist < pPatch2->mDist ) return 1;

	return 0;
}


//*****************************************************************************
VuWaterRenderer::VuWaterRenderer(bool bAsynchronous):
	mbDebugNormals(false),
	mbDebugFlow(false),
	mClipMaps(true),
	mCurSubmitPatchBuffer(0),
	mCurRenderPatchBuffer(1),
	mbAsynchronousWaterRenderer(bAsynchronous),
	mbWorkerThreadActive(false),
	mbTerminateThread(false)
{
	// set up dev menu/stats
	VuDevMenu::IF()->addBool("Water/Normals", mbDebugNormals);
	VuDevMenu::IF()->addBool("Water/Flow", mbDebugFlow);
	VuDevMenu::IF()->addBool("Water/ClipMaps", mClipMaps);
	VuDevStat::IF()->addPage("WaterRenderer", VuRect(50, 10, 40, 40));

	// create water texture
	mpWaterTexture = new VuWaterTexture;

	// reserve memory
	mPatches.reserve(INITIAL_MAX_PATCH_COUNT);
	mPatchBuffers[0].mVertexArray.reserve(25*400);
	mPatchBuffers[0].mIndexArray.reserve(3*25*400);
	mPatchBuffers[1].mVertexArray.reserve(25*400);
	mPatchBuffers[1].mIndexArray.reserve(3*25*400);
	mDebugVerts.reserve(2048);

	// build buffers
	buildBuffers();

	// clear stats
	memset(&mStats, 0, sizeof(mStats));

	// threading
	mWorkAvailableEvent = VuThread::IF()->createEvent();
	mWorkCompletedEvent = VuThread::IF()->createEvent();
	mhThread = VuThread::IF()->createThread(threadProc, this);
}

//*****************************************************************************
VuWaterRenderer::~VuWaterRenderer()
{
	flush();

	VUPRINTF("Terminating VuWaterRenderer thread...\n");
	mbTerminateThread = true;
	VuThread::IF()->setEvent(mWorkAvailableEvent);
	VuThread::IF()->joinThread(mhThread);

	// destroy water texture
	delete mpWaterTexture;

	VuThread::IF()->destroyEvent(mWorkAvailableEvent);
	VuThread::IF()->destroyEvent(mWorkCompletedEvent);
}

//*****************************************************************************
void VuWaterRenderer::setWaterTextureDesc(const VuWaterRendererTextureDesc &desc)
{
	mpWaterTexture->setDesc(desc);
}

//*****************************************************************************
void VuWaterRenderer::submit(const VuWaterRendererParams &params)
{
	VU_PROFILE_SIM("VuWaterRenderer::submit");

	const VuWaterSurface *pSurface = params.mpSurface;
	const VuWaterShader *pShader = params.mpShader;

	if ( params.mpCamera->getFrustum().isAabbVisible(pSurface->mWorldAabb, VuMatrix::identity()) )
	{
		// calculate depth
		float depth = pSurface->calcDistance3d(params.mpCamera->getEyePosition())/params.mpCamera->getFarPlane();
		depth = VuMin(depth, 1.0f);

		// submit commands
		WaterRendererDrawData *pData = static_cast<WaterRendererDrawData *>(VuGfxSort::IF()->allocateCommandMemory(sizeof(WaterRendererDrawData)));
		pData->mpRenderer = this;
		pData->mpShader = pShader;
		pData->mpSurface = pSurface;
		VuGfxSort::IF()->submitDrawCommand<true>(VuGfxSort::TRANS_WATER_COLOR, pShader->getMaterial(), VUNULL, &WaterRendererDrawData::colorCallback, depth);
	}
}

//*****************************************************************************
bool VuWaterRenderer::isBusy()
{
	return mbWorkerThreadActive;
}

//*****************************************************************************
void VuWaterRenderer::flush()
{
	if ( mbWorkerThreadActive )
	{
		VuThread::IF()->waitForSingleObject(mWorkCompletedEvent);
		mbWorkerThreadActive = false;
	}
}

//*****************************************************************************
void VuWaterRenderer::kick()
{
	// calculate time spent overlapping w/ main thread
	mWaterRendererOverlapTime = (float)VuSys::IF()->getTime() - mWaterRendererOverlapTime;

	// kick off next frame
	mbWorkerThreadActive = true;
	VuThread::IF()->setEvent(mWorkAvailableEvent);

	// if processing synchronously, flush now
	if ( !mbAsynchronousWaterRenderer )
		flush();
}

//*****************************************************************************
void VuWaterRenderer::synchronize()
{
	flush();

	mCurSubmitPatchBuffer = !mCurSubmitPatchBuffer;
	mCurRenderPatchBuffer = !mCurRenderPatchBuffer;
	mPatchBuffers[mCurSubmitPatchBuffer].mVertexArray.clear();
	mPatchBuffers[mCurSubmitPatchBuffer].mIndexArray.clear();
	for ( int iViewport = 0; iViewport < VuViewportManager::MAX_VIEWPORTS; iViewport++ )
		mPatchData[mCurSubmitPatchBuffer][iViewport].clear();

	if ( int lineCount = mDebugVerts.size()/2 )
	{
		VuVector3 *pv = &mDebugVerts.begin();
		for ( int i = 0; i < lineCount; i++ )
		{
			VuDev::IF()->drawLine(pv[0], pv[1], VuColor(255,255,255));
			pv += 2;
		}
	}
	mDebugVerts.clear();

	mWaterRendererOverlapTime = (float)VuSys::IF()->getTime();

	updateDevStats();

	// synchronize other systems
	if ( VuDevProfile::IF() )
		VuDevProfile::IF()->synchronizeWater();
}

//*****************************************************************************
void VuWaterRenderer::drawColor(const WaterRendererDrawData *pDrawData)
{
	VU_PROFILE_GFX("WaterColor");

	// find patch data
	PatchData &patchData = mPatchData[mCurRenderPatchBuffer][VuGfxSort::IF()->getRenderViewport()];
	for ( VuPatchData *pPatch = &patchData.begin(); pPatch != &patchData.end(); pPatch++ )
	{
		if ( pPatch->mpSurface == pDrawData->mpSurface )
		{
			// calculate transform/aabb
			VuMatrix transform;
			transform.loadIdentity();
			transform.translate(pPatch->mSurfacePos);
			transform.rotateZLocal(pPatch->mSurfaceRotZ);

			VuAabb aabb;
			aabb.mMin.mX = -0.5f*pPatch->mSurfaceSize.mX; aabb.mMin.mY = -0.5f*pPatch->mSurfaceSize.mY; aabb.mMin.mZ = -pPatch->mMaxWaveDepth;
			aabb.mMax.mX =  0.5f*pPatch->mSurfaceSize.mX; aabb.mMax.mY =  0.5f*pPatch->mSurfaceSize.mY; aabb.mMax.mZ =  pPatch->mMaxWaveHeight;

			// set shader constants
			pDrawData->mpShader->use(pPatch->mSurfacePos.mZ, transform, aabb);

			const VuPatchBuffer &patchBuffer = mPatchBuffers[mCurRenderPatchBuffer];
			const VuWaterShaderVertex *pVerts = &patchBuffer.mVertexArray[pPatch->mVertexStart];
			const VUUINT16 *pInds = &patchBuffer.mIndexArray[pPatch->mIndexStart];

			// draw patch
			VuGfx::IF()->drawIndexedPrimitiveUP(
				VUGFX_PT_TRIANGLELIST,		// PrimitiveType
				0,							// MinVertexIndex
				pPatch->mVertexCount,		// NumVertices
				pPatch->mIndexCount/3,		// PrimitiveCount
				pInds,						// pIndexData
				pVerts						// pVertexStreamZeroData
			);

			break;
		}
	}
}

//*****************************************************************************
void VuWaterRenderer::updateDevStats()
{
	// update max
	mStats.mMaxVertexCount = VuMax(mStats.mMaxVertexCount, mStats.mVertexCount);
	mStats.mMaxIndexCount = VuMax(mStats.mMaxIndexCount, mStats.mIndexCount);

	// dev stats
	if ( VuDevStatPage *pPage = VuDevStat::IF()->getCurPage() )
	{
		if ( pPage->getName() == "WaterRenderer" )
		{
			int maxVertexBufferSize = mStats.mMaxVertexCount*sizeof(VuWaterShaderVertex);
			int maxIndexBufferSize = mStats.mMaxIndexCount*sizeof(VUUINT16);

			pPage->clear();
			pPage->printf("Surface Count: %d\n", mStats.mSurfaceCount);
			pPage->printf("Patch Count: %d\n", mStats.mPatchCount);
			pPage->printf("Vertex Count: %d\n", mStats.mVertexCount);
			pPage->printf("Triangle Count: %d\n", mStats.mIndexCount/3);
			pPage->printf("Max Vertex Count: %d (%d K)\n", mStats.mMaxVertexCount, maxVertexBufferSize/1024);
			pPage->printf("Max Triangle Count: %d (%d K)\n", mStats.mMaxIndexCount/3, maxIndexBufferSize/1024);
		}
	}

	// reset stats
	mStats.mSurfaceCount = 0;
	mStats.mPatchCount = 0;
	mStats.mVertexCount = 0;
	mStats.mIndexCount = 0;
}

//*****************************************************************************
void VuWaterRenderer::buildBuffers()
{
	for ( VUUINT32 mask = 0; mask < 16; mask++ )
	{
		VuBuffer *pBuffer = &mBuffers[mask];

		// calc vert, tri, & index counts
		int edgeFlagCount = VuBitCount(mask);
		int vertCount = 17 + 2*(4 - edgeFlagCount);
		int triCount = 24 + 2*(4 - edgeFlagCount);

		// allocate
		pBuffer->mVerts.reserve(vertCount);
		pBuffer->mIndices.reserve(triCount*3);

		// base verts (all buffer share these)
		addVert(pBuffer,  0.0f,  0.0f); // 0
		addVert(pBuffer, -0.5f,  0.0f); // 1
		addVert(pBuffer,  0.0f, -0.5f); // 2
		addVert(pBuffer,  0.5f,  0.0f); // 3
		addVert(pBuffer,  0.0f,  0.5f); // 4
		addVert(pBuffer, -0.5f, -0.5f); // 5
		addVert(pBuffer,  0.5f, -0.5f); // 6
		addVert(pBuffer,  0.5f,  0.5f); // 7
		addVert(pBuffer, -0.5f,  0.5f); // 8
		addVert(pBuffer, -1.0f,  0.0f); // 9
		addVert(pBuffer,  0.0f, -1.0f); // 10
		addVert(pBuffer,  1.0f,  0.0f); // 11
		addVert(pBuffer,  0.0f,  1.0f); // 12
		addVert(pBuffer, -1.0f, -1.0f); // 13
		addVert(pBuffer,  1.0f, -1.0f); // 14
		addVert(pBuffer,  1.0f,  1.0f); // 15
		addVert(pBuffer, -1.0f,  1.0f); // 16

		// base tris (all buffers share these)
		addTri(pBuffer,  0,  1,  5); // 0
		addTri(pBuffer,  0,  5,  2); // 1
		addTri(pBuffer,  0,  2,  6); // 2
		addTri(pBuffer,  0,  6,  3); // 3
		addTri(pBuffer,  0,  3,  7); // 4
		addTri(pBuffer,  0,  7,  4); // 5
		addTri(pBuffer,  0,  4,  8); // 6
		addTri(pBuffer,  0,  8,  1); // 7
		addTri(pBuffer,  1,  8,  9); // 8
		addTri(pBuffer,  1,  9,  5); // 9
		addTri(pBuffer,  2,  5, 10); // 10
		addTri(pBuffer,  2, 10,  6); // 11
		addTri(pBuffer,  3,  6, 11); // 12
		addTri(pBuffer,  3, 11,  7); // 13
		addTri(pBuffer,  4,  7, 12); // 14
		addTri(pBuffer,  4, 12,  8); // 15

		// left edge
		if ( mask & VuPatch::LEFT )
		{
			addTri(pBuffer,  8, 16,  9);
			addTri(pBuffer,  5,  9, 13);
		}
		else
		{
			VUUINT16 v0 = addVert(pBuffer, -1.0f,  0.5f);
			VUUINT16 v1 = addVert(pBuffer, -1.0f, -0.5f);
			addTri(pBuffer,  8, 16, v0);
			addTri(pBuffer,  8, v0,  9);
			addTri(pBuffer,  5,  9, v1);
			addTri(pBuffer,  5, v1, 13);
		}

		// bottom edge
		if ( mask & VuPatch::BOTTOM )
		{
			addTri(pBuffer,  5, 13, 10);
			addTri(pBuffer,  6, 10, 14);
		}
		else
		{
			VUUINT16 v0 = addVert(pBuffer, -0.5f, -1.0f);
			VUUINT16 v1 = addVert(pBuffer,  0.5f, -1.0f);
			addTri(pBuffer,  5, 13, v0);
			addTri(pBuffer,  5, v0, 10);
			addTri(pBuffer,  6, 10, v1);
			addTri(pBuffer,  6, v1, 14);
		}

		// right edge
		if ( mask & VuPatch::RIGHT )
		{
			addTri(pBuffer,  6, 14, 11);
			addTri(pBuffer,  7, 11, 15);
		}
		else
		{
			VUUINT16 v0 = addVert(pBuffer,  1.0f, -0.5f);
			VUUINT16 v1 = addVert(pBuffer,  1.0f,  0.5f);
			addTri(pBuffer,  6, 14, v0);
			addTri(pBuffer,  6, v0, 11);
			addTri(pBuffer,  7, 11, v1);
			addTri(pBuffer,  7, v1, 15);
		}

		// top edge
		if ( mask & VuPatch::TOP )
		{
			addTri(pBuffer,  7, 15, 12);
			addTri(pBuffer,  8, 12, 16);
		}
		else
		{
			VUUINT16 v0 = addVert(pBuffer,  0.5f,  1.0f);
			VUUINT16 v1 = addVert(pBuffer, -0.5f,  1.0f);
			addTri(pBuffer,  7, 15, v0);
			addTri(pBuffer,  7, v0, 12);
			addTri(pBuffer,  8, 12, v1);
			addTri(pBuffer,  8, v1, 16);
		}

		// verify sizes
		VUASSERT(pBuffer->mVerts.size() == vertCount, "VuWaterRenderer::buildBuffers vertex buffer size mismatch");
		VUASSERT(pBuffer->mIndices.size() == triCount*3, "VuWaterRenderer::buildBuffers index buffer size mismatch");
	}
}

//*****************************************************************************
VUUINT16 VuWaterRenderer::addVert(VuBuffer *pBuffer, float x, float y)
{
	pBuffer->mVerts.push_back(VuVector2(x, y));

	return (VUUINT16)(pBuffer->mVerts.size() - 1);
}

//*****************************************************************************
void VuWaterRenderer::addTri(VuBuffer *pBuffer, VUUINT16 i0, VUUINT16 i1, VUUINT16 i2)
{
	pBuffer->mIndices.push_back(i0);
	pBuffer->mIndices.push_back(i1);
	pBuffer->mIndices.push_back(i2);
}

//*****************************************************************************
void VuWaterRenderer::buildPatches()
{
	int powX = mJobData.mpSurface->mDesc.mPowSizeX;
	int powY = mJobData.mpSurface->mDesc.mPowSizeY;

	if ( powX > powY )
	{
		int num = 1 << (powX - powY);
		float fExtent = 0.5f*(1<<powY);
		for ( int i = 0; i < num; i++ )
			buildPatches(0, i, 0, VuVector2((2*i + 1)*fExtent, fExtent), fExtent);
	}
	else if( powY > powX )
	{
		int num = 1 << (powY - powX);
		float fExtent = 0.5f*(1<<powX);
		for ( int i = 0; i < num; i++ )
			buildPatches(0, 0, i, VuVector2(fExtent, (2*i + 1)*fExtent), fExtent);
	}
	else
	{
		float fExtent = 0.5f*(1<<powX);
		buildPatches(0, 0, 0, VuVector2(fExtent, fExtent), fExtent);
	}
}

//*****************************************************************************
void VuWaterRenderer::buildPatches(int clipDepth, int clipX, int clipY, const VuVector2 &vCenter, float fExtent)
{
	// calculate aabb
	VuAabb aabb;
	aabb.mMin = VuVector3(vCenter.mX - fExtent, vCenter.mY - fExtent, -mJobData.mMaxWaveDepth);
	aabb.mMax = VuVector3(vCenter.mX + fExtent, vCenter.mY + fExtent,  mJobData.mMaxWaveHeight);

	// check if visible
	if ( mJobData.mpCamera->getFrustum().isAabbVisible(aabb, mJobData.mMatModel) )
	{
		// check if clipped by water map
		if ( mClipMaps && mJobData.mpSurface->mpWaterMap )
			if ( !mJobData.mpSurface->mpWaterMap->isVisible(clipDepth, clipX, clipY) )
				return;

		// calculate 2d distance from camera to bounding sphere
		VuVector3 vWorldCenter = mJobData.mMatModel.transform(aabb.getCenter());
		float fBoundingRadius2d = aabb.getExtents().mag2d();
		float fHorzDist = (mJobData.mpCamera->getEyePosition() - vWorldCenter).mag2d() - fBoundingRadius2d;
		fHorzDist = VuMax(fHorzDist, 0.0f);

		// calculate vertical distance
		float fVertDist = VuAbs(mJobData.mpCamera->getEyePosition().mZ - vWorldCenter.mZ) - aabb.getExtents().mZ;
		fVertDist = VuMax(fVertDist, 0.0f);

		float fDist = VuSqrt(fHorzDist*fHorzDist + fVertDist*fVertDist);

		if ( clipDepth >= mJobData.mMinRecursionDepth && (fDist/fExtent > mJobData.mDistExtentRatio || fExtent <= 0.5f*mJobData.mMinPatchSize) )
		{
			// add this patch
			mPatches.resize(mPatches.size() + 1);
			mPatches.back().mCenter = vCenter;
			mPatches.back().mExtent = fExtent;
			mPatches.back().mDist = fDist;
			mPatches.back().mEdgeFlags = 0;
		}
		else
		{
			// recurse
			float fChildExtent = 0.5f*fExtent;
			buildPatches(clipDepth + 1, (clipX << 1) + 0, (clipY << 1) + 0, vCenter + VuVector2(-fChildExtent, -fChildExtent), fChildExtent);
			buildPatches(clipDepth + 1, (clipX << 1) + 1, (clipY << 1) + 0, vCenter + VuVector2(+fChildExtent, -fChildExtent), fChildExtent);
			buildPatches(clipDepth + 1, (clipX << 1) + 1, (clipY << 1) + 1, vCenter + VuVector2(+fChildExtent, +fChildExtent), fChildExtent);
			buildPatches(clipDepth + 1, (clipX << 1) + 0, (clipY << 1) + 1, vCenter + VuVector2(-fChildExtent, +fChildExtent), fChildExtent);
		}
	}
}

//*****************************************************************************
void VuWaterRenderer::calcEdgeFlags()
{
	for ( VuPatch *pPatch0 = &mPatches.begin(); pPatch0 != &mPatches.end(); pPatch0++ )
	{
		float fl0 = pPatch0->mCenter.mX - pPatch0->mExtent;
		float fr0 = pPatch0->mCenter.mX + pPatch0->mExtent;
		float fb0 = pPatch0->mCenter.mY - pPatch0->mExtent;
		float ft0 = pPatch0->mCenter.mY + pPatch0->mExtent;
		for ( VuPatch *pPatch1 = &mPatches.begin(); pPatch1 != &mPatches.end(); pPatch1++ )
		{
			// I only care if patch1 is larger that patch0 (trivial rejection)
			if ( pPatch1->mExtent > pPatch0->mExtent )
			{
				float fl1 = pPatch1->mCenter.mX - pPatch1->mExtent;
				float fr1 = pPatch1->mCenter.mX + pPatch1->mExtent;
				float fb1 = pPatch1->mCenter.mY - pPatch1->mExtent;
				float ft1 = pPatch1->mCenter.mY + pPatch1->mExtent;

				// check if any edge of patch0 matches any edge of patch1 and set edge flags accordingly
				if ( fl0 == fr1 && fb0 < ft1 && ft0 > fb1 ) pPatch0->mEdgeFlags |= VuPatch::LEFT;
				if ( fr0 == fl1 && fb0 < ft1 && ft0 > fb1 ) pPatch0->mEdgeFlags |= VuPatch::RIGHT;
				if ( fb0 == ft1 && fl0 < fr1 && fr0 > fl1 ) pPatch0->mEdgeFlags |= VuPatch::BOTTOM;
				if ( ft0 == fb1 && fl0 < fr1 && fr0 > fl1 ) pPatch0->mEdgeFlags |= VuPatch::TOP;
			}
		}
	}
}

//*****************************************************************************
void VuWaterRenderer::drawNormals(VuWaterRenderVertex *pVerts, int vertCount)
{
	mDebugVerts.resize(mDebugVerts.size() + vertCount*2);

	VuWaterRenderVertex *pSrc = pVerts;
	VuVector3 *pDst = &mDebugVerts.end() - vertCount*2;
	for ( int i = 0; i < vertCount; i++ )
	{
		pDst->mX = pSrc->mPosition.mX;
		pDst->mY = pSrc->mPosition.mY;
		pDst->mZ = pSrc->mPosition.mZ;
		pDst++;

		pDst->mX = pSrc->mPosition.mX - pSrc->mDzDxy.mX;
		pDst->mY = pSrc->mPosition.mY - pSrc->mDzDxy.mY;
		pDst->mZ = pSrc->mPosition.mZ + 1;
		pDst++;

		pSrc++;
	}
}

//*****************************************************************************
void VuWaterRenderer::drawFlow(VuWaterRenderVertex *pVerts, int vertCount)
{
	mDebugVerts.resize(mDebugVerts.size() + vertCount*2);

	VuWaterRenderVertex *pSrc = pVerts;
	VuVector3 *pDst = &mDebugVerts.end() - vertCount*2;
	for ( int i = 0; i < vertCount; i++ )
	{
		VuWaterPhysicsVertex vert = VuWater::IF()->getPhysicsVertex(VuVector3(pSrc->mPosition.mX, pSrc->mPosition.mY, pSrc->mPosition.mZ));

		pDst->mX = pSrc->mPosition.mX;
		pDst->mY = pSrc->mPosition.mY;
		pDst->mZ = pSrc->mPosition.mZ;
		pDst++;

		pDst->mX = pSrc->mPosition.mX + vert.mDxyzDt.mX;
		pDst->mY = pSrc->mPosition.mY + vert.mDxyzDt.mY;
		pDst->mZ = pSrc->mPosition.mZ + vert.mDxyzDt.mZ;
		pDst++;

		pSrc++;
	}
}

//*****************************************************************************
void VuWaterRenderer::threadProc()
{
	VuThread::IF()->setThreadProcessor(1);

	VUPRINTF("VuWaterRenderer thread starting...\n");

	for (;;)
	{
		if ( !VuThread::IF()->waitForSingleObject(mWorkAvailableEvent) )
		{
			VUPRINTF("VuWaterRenderer::threadProc() wait error!\n");
			break;
		}
		if ( mbTerminateThread )
		{
			VUPRINTF("VuWaterRenderer thread exiting...\n");
			break;
		}

		// do work
		{
			if ( VuDevProfile::IF() )
				VuDevProfile::IF()->beginWater(mWaterRendererOverlapTime);

			// use number of viewports to determine detail
			float fDetail = VuWater::IF()->getDetail();

			// global job data
			mJobData.mMinPatchSize = 2.0f/fDetail;
			mJobData.mDistExtentRatio = 4.0f*fDetail;

			// determine which surfaces to draw
			for ( int iViewport = 0; iViewport < VuViewportManager::IF()->getViewportCount(); iViewport++ )
			{
				const VuCamera &camera = VuViewportManager::IF()->getViewport(iViewport).mCamera;
				const VuFrustum &frustum = camera.getFrustum();

				// per-camera job data
				mJobData.mpCamera = &camera;
				mJobData.mViewportIndex = iViewport;

				for ( VuWater::Surfaces::const_iterator iter = VuWater::IF()->getSurfaces().begin(); iter != VuWater::IF()->getSurfaces().end(); iter++ )
				{
					if ( frustum.isAabbVisible((*iter)->mWorldAabb, VuMatrix::identity()) )
					{
						// per-surface job data
						mJobData.mpSurface = *iter;

						buildSurfaceDrawData();
					}
				}
			}

			if ( VuDevProfile::IF() )
				VuDevProfile::IF()->endWater();
		}

		VuThread::IF()->setEvent(mWorkCompletedEvent);
	}

	VuThread::IF()->endThread();
}

//*****************************************************************************
void VuWaterRenderer::buildSurfaceDrawData()
{
	VU_PROFILE_WATER("buildSurfaceDrawData");

	const VuWaterSurface *pSurface = mJobData.mpSurface;

	// additional job data
	mJobData.mMaxWaveDepth = pSurface->mDesc.mMaxWaveDepth;
	mJobData.mMaxWaveHeight = pSurface->mDesc.mMaxWaveHeight;
	mJobData.mMinRecursionDepth = pSurface->mDesc.mMinRecursionDepth;

	// build matrix
	mJobData.mMatModel.loadIdentity();
	mJobData.mMatModel.translate(pSurface->mDesc.mPos);
	mJobData.mMatModel.rotateZLocal(pSurface->mDesc.mRotZ);
	mJobData.mMatModel.translateLocal(VuVector3(-0.5f*pSurface->mSizeX, -0.5f*pSurface->mSizeY, 0));

	// build patches (surface may be rectangular)
	buildPatches();

	if ( mPatches.size() )
	{
		// calculate edge flags
		calcEdgeFlags();

		// sort patches
		qsort(&mPatches[0], mPatches.size(), sizeof(VuPatch), PatchComp);

		// get current patch buffer
		VuPatchBuffer &patchBuffer = mPatchBuffers[mCurSubmitPatchBuffer];

		// add patch data entry
		PatchData &patchData = mPatchData[mCurSubmitPatchBuffer][mJobData.mViewportIndex];
		patchData.resize(patchData.size() + 1);
		VuPatchData &data = patchData.back();
		data.mpSurface = pSurface;
		data.mSurfacePos = pSurface->mDesc.mPos;
		data.mSurfaceRotZ = pSurface->mDesc.mRotZ;
		data.mSurfaceSize.mX = (float)pSurface->mSizeX;
		data.mSurfaceSize.mY = (float)pSurface->mSizeY;
		data.mMaxWaveDepth = pSurface->mDesc.mMaxWaveDepth;
		data.mMaxWaveHeight = pSurface->mDesc.mMaxWaveHeight;
		data.mPatchCount = mPatches.size();
		data.mVertexStart = patchBuffer.mVertexArray.size();
		data.mIndexStart = patchBuffer.mIndexArray.size();

		// build vertex and index data
		buildVertexIndexData(patchBuffer.mVertexArray, patchBuffer.mIndexArray);

		// count
		data.mVertexCount = patchBuffer.mVertexArray.size() - data.mVertexStart;
		data.mIndexCount = patchBuffer.mIndexArray.size() - data.mIndexStart;

		// update stats
		mStats.mPatchCount += data.mPatchCount;
		mStats.mVertexCount += data.mVertexCount;
		mStats.mIndexCount += data.mIndexCount;

		// were done... clean up (but don't deallocate memory)
		mPatches.clear();
	}

	// update stats
	mStats.mSurfaceCount++;
}

//*****************************************************************************
void VuWaterRenderer::buildVertexIndexData(VertexArray &vertexArray, IndexArray &indexArray)
{
	int vertexOffset = vertexArray.size();

	VuPatch *pPatch = &mPatches[0];
	for ( int iPatch = 0; iPatch < mPatches.size(); iPatch++ )
	{
		VUUINT32 mask = pPatch->mEdgeFlags;
		VuBuffer *pBuffer = &mBuffers[mask];
		int vertCount = pBuffer->mVerts.size();
		int indexCount = pBuffer->mIndices.size();

		int vertexStart = vertexArray.size();
		int indexStart = indexArray.size();
		vertexArray.resize(vertexStart + vertCount);
		indexArray.resize(indexStart + indexCount);

		VuWaterRenderVertex *pScratchPadVerts = (VuWaterRenderVertex *)VuScratchPad::get(VuScratchPad::WATER);

		// build verts to scratch pad
		{
			VuVector2 *pSrc = &pBuffer->mVerts[0];
			VuWaterRenderVertex *pDst = pScratchPadVerts;
			for ( int iVert = 0; iVert < vertCount; iVert++ )
			{
				float fX = pPatch->mCenter.mX + pSrc->mX*pPatch->mExtent;
				float fY = pPatch->mCenter.mY + pSrc->mY*pPatch->mExtent;

				VuVector3 vPos = mJobData.mMatModel.transform(VuVector3(fX, fY, 0));
				pDst->mPosition.mX = vPos.mX;
				pDst->mPosition.mY = vPos.mY;
				pDst->mPosition.mZ = vPos.mZ;

				pSrc++;
				pDst++;
			}
		}

		// build indices
		{
			VUUINT16 *pSrc = &pBuffer->mIndices[0];
			VUUINT16 *pDst = &indexArray[indexStart];

			for ( int iIndex = 0; iIndex < indexCount; iIndex++ )
			{
				*pDst = *pSrc + VUUINT16(vertexStart - vertexOffset);
				pSrc++;
				pDst++;
			}
		}

		// calculate 2d bounding data
		VuVector3 vc = VuVector3(pScratchPadVerts[0].mPosition.mX, pScratchPadVerts[0].mPosition.mY, pScratchPadVerts[0].mPosition.mZ);
		VuVector2 v0 = VuVector2(pScratchPadVerts[13].mPosition.mX, pScratchPadVerts[13].mPosition.mY);
		VuVector2 v1 = VuVector2(pScratchPadVerts[14].mPosition.mX, pScratchPadVerts[14].mPosition.mY);
		VuVector2 v2 = VuVector2(pScratchPadVerts[15].mPosition.mX, pScratchPadVerts[15].mPosition.mY);
		VuVector2 v3 = VuVector2(pScratchPadVerts[16].mPosition.mX, pScratchPadVerts[16].mPosition.mY);
		VuVector2 vMin(FLT_MAX, FLT_MAX), vMax(-FLT_MAX, -FLT_MAX);
		VuMinMax(v0, vMin, vMax);
		VuMinMax(v1, vMin, vMax);
		VuMinMax(v2, vMin, vMax);
		VuMinMax(v3, vMin, vMax);

		// get surface data
		VuWaterSurfaceDataParams params(VuWaterSurfaceDataParams::VT_RENDER);
		params.mVertCount = vertCount;
		params.mBoundingAabb.mMin = VuVector3(vMin.mX, vMin.mY, mJobData.mpSurface->mDesc.mPos.mZ);
		params.mBoundingAabb.mMax = VuVector3(vMax.mX, vMax.mY, mJobData.mpSurface->mDesc.mPos.mZ);
		params.mBoundingCenter = vc;
		params.mBoundingRadius = VuSqrt(2.0f)*pPatch->mExtent;

		params.mpWaterSurfaceHint = mJobData.mpSurface;

		params.mpRenderVertex = pScratchPadVerts;
		params.mStride = sizeof(pScratchPadVerts[0]);

		VuWater::IF()->getSurfaceData(params);

		if ( mbDebugNormals | mbDebugFlow )
		{
			if ( mbDebugNormals )
				drawNormals(pScratchPadVerts, vertCount);
			if ( mbDebugFlow )
				drawFlow(pScratchPadVerts, vertCount);
		}

		// copy/transform from scratch pad (VuWaterRenderVertex) to vertex array (VuWaterShaderVertex)
		VuWaterShaderVertex *pVerts = &vertexArray[vertexStart];
		{
			VuWaterRenderVertex *pSrc = pScratchPadVerts;
			VuWaterShaderVertex *pDst = pVerts;
			for ( int iVert = 0; iVert < vertCount; iVert++ )
			{
				pDst->mPosition = pSrc->mPosition;
				pDst->mNormalX = (VUUINT16)VuRound(32767.0f*pSrc->mDzDxy.mX);
				pDst->mNormalY = (VUUINT16)VuRound(32767.0f*pSrc->mDzDxy.mY);
				pDst->mFoam = (VUUINT8)(VuMin(pSrc->mFoam, 1.0f)*255.0f + 0.5f);
				pDst->mShadow = 255;
				pDst->mDecal = 0;
				pDst->mLightR = 0;
				pDst->mLightG = 0;
				pDst->mLightB = 0;

				pSrc++;
				pDst++;
			}
		}

		// shadow/foam/decal map
		if ( mJobData.mpSurface->mpWaterMap )
			addWaterMapInfluence(pBuffer, pPatch, pVerts, vertCount, mJobData.mpSurface);

		// light map
		if ( mJobData.mpSurface->mpLightMap )
			addLightMapInfluence(pBuffer, pPatch, pVerts, vertCount, mJobData.mpSurface);

		pPatch++;
	}
}

//*****************************************************************************
void VuWaterRenderer::addWaterMapInfluence(VuBuffer *pBuffer, VuPatch *pPatch, VuWaterShaderVertex *pVerts, int vertCount, const VuWaterSurface *pSurface)
{
	const VUUINT16 *pSFD16 = &pSurface->mpWaterMap->getSFD16().begin();

	int mapWidth = pSurface->mpWaterMap->getWidth();
	int mapHeight = pSurface->mpWaterMap->getHeight();
	float scaleX = (float)(mapWidth - 1);
	float scaleY = (float)(mapHeight - 1);

	float invWidth = 1.0f/pSurface->mSizeX;
	float invHeight = 1.0f/pSurface->mSizeY;

	VuVector2 *pSrc = &pBuffer->mVerts[0];
	VuWaterShaderVertex *pDst = pVerts;
	for ( int iVert = 0; iVert < vertCount; iVert++ )
	{
		float fX = (pPatch->mCenter.mX + pSrc->mX*pPatch->mExtent)*invWidth;
		float fY = (pPatch->mCenter.mY + pSrc->mY*pPatch->mExtent)*invHeight;

		int x = VuTruncate(fX*scaleX);
		int y = VuTruncate(fY*scaleY);

		VUUINT16 texel = pSFD16[y*mapWidth + x];

		pDst->mShadow = VU_EXTRACT_R_FROM_565(texel);
		pDst->mFoam = (VUUINT8)(VuMin(pDst->mFoam + VU_EXTRACT_G_FROM_565(texel), 255));
		pDst->mDecal = VU_EXTRACT_B_FROM_565(texel);

		pSrc++;
		pDst++;
	}
}

//*****************************************************************************
void VuWaterRenderer::addLightMapInfluence(VuBuffer *pBuffer, VuPatch *pPatch, VuWaterShaderVertex *pVerts, int vertCount, const VuWaterSurface *pSurface)
{
	const VUUINT16 *pRGB16 = &pSurface->mpLightMap->getRGB16().begin();

	int mapWidth = pSurface->mpLightMap->getWidth();
	int mapHeight = pSurface->mpLightMap->getHeight();
	float scaleX = (float)(mapWidth - 1);
	float scaleY = (float)(mapHeight - 1);

	float invWidth = 1.0f/pSurface->mSizeX;
	float invHeight = 1.0f/pSurface->mSizeY;

	VuVector2 *pSrc = &pBuffer->mVerts[0];
	VuWaterShaderVertex *pDst = pVerts;
	for ( int iVert = 0; iVert < vertCount; iVert++ )
	{
		float fX = (pPatch->mCenter.mX + pSrc->mX*pPatch->mExtent)*invWidth;
		float fY = (pPatch->mCenter.mY + pSrc->mY*pPatch->mExtent)*invHeight;

		int x = VuTruncate(fX*scaleX);
		int y = VuTruncate(fY*scaleY);

		VUUINT16 texel = pRGB16[y*mapWidth + x];

		pDst->mLightR = VU_EXTRACT_R_FROM_565(texel);
		pDst->mLightG = VU_EXTRACT_G_FROM_565(texel);
		pDst->mLightB = VU_EXTRACT_B_FROM_565(texel);

		pSrc++;
		pDst++;
	}
}
