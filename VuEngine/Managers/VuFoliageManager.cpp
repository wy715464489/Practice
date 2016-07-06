//*****************************************************************************
//
//  Copyright (c) 2011-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  Foliage Manager
// 
//*****************************************************************************

#include "VuFoliageManager.h"
#include "VuEngine/Gfx/VuGfxUtil.h"
#include "VuEngine/Gfx/GfxSort/VuGfxSort.h"
#include "VuEngine/Gfx/GfxSort/VuGfxSortMaterial.h"
#include "VuEngine/Gfx/Camera/VuCamera.h"
#include "VuEngine/HAL/Gfx/VuGfx.h"
#include "VuEngine/HAL/Gfx/VuTexture.h"
#include "VuEngine/HAL/Gfx/VuShaderProgram.h"
#include "VuEngine/HAL/Gfx/VuVertexDeclaration.h"
#include "VuEngine/HAL/Gfx/VuPipelineState.h"
#include "VuEngine/Assets/VuAssetFactory.h"
#include "VuEngine/Assets/VuTextureAsset.h"
#include "VuEngine/Assets/VuCompiledShaderAsset.h"


// the interface
IMPLEMENT_SYSTEM_COMPONENT(VuFoliageManager, VuFoliageManager);


struct FoliageDrawData
{
	VuTextureAsset	*mpTextureAsset;
	int				mCount;
};
struct FoliageVertex
{
	float			mPos[3];
	float			mTexCoord[2];
	VUUINT32		mColor;
};

inline void BuildVerts(FoliageVertex *pVerts, const VuFoliageManager::DrawParams &params, float axisXX, float axisXY);


class VuFoliageManager::VuBucket : public VuRefObj
{
public:
	VuBucket(VuTextureAsset *pTextureAsset, bool fogEnabled) : mpTextureAsset(pTextureAsset), mFogEnabled(fogEnabled), mEntries(64) {}

	struct VuEntry
	{
		VuFoliageManager::DrawParams	mParams;
		float							mAxisXX;
		float							mAxisXY;
	};

	typedef VuArray<VuEntry> Entries;

	VuTextureAsset	*mpTextureAsset;
	bool			mFogEnabled;
	Entries			mEntries;
};


//*****************************************************************************
VuFoliageManager::VuFoliageManager()
{
}

//*****************************************************************************
bool VuFoliageManager::init()
{
	// get vertex shaders
	mFlavors[FLAVOR_SIMPLE].mpCompiledShaderAsset = VuAssetFactory::IF()->createAsset<VuCompiledShaderAsset>("Foliage/Simple");
	mFlavors[FLAVOR_FOG].mpCompiledShaderAsset = VuAssetFactory::IF()->createAsset<VuCompiledShaderAsset>("Foliage/Fog");

	// create vertex declaration
	VuVertexDeclarationParams vdParams;
	vdParams.mElements.push_back(VuVertexDeclarationElement(0,  0, VUGFX_DECL_TYPE_FLOAT3,  VUGFX_DECL_USAGE_POSITION, 0));
	vdParams.mElements.push_back(VuVertexDeclarationElement(0, 12, VUGFX_DECL_TYPE_FLOAT2,  VUGFX_DECL_USAGE_TEXCOORD, 0));
	vdParams.mElements.push_back(VuVertexDeclarationElement(0, 20, VUGFX_DECL_TYPE_UBYTE4N, VUGFX_DECL_USAGE_COLOR,    0));
	vdParams.mStreams.push_back(VuVertexDeclarationStream(24));

	// create material
	VuGfxSortMaterialDesc desc;
	for ( int i = 0; i < FLAVOR_COUNT; i++ )
	{
		VuVertexDeclaration *pVD = VuGfx::IF()->createVertexDeclaration(vdParams, mFlavors[i].mpCompiledShaderAsset->getShaderProgram());

		VuPipelineStateParams psParams;
		VuPipelineState *pPS = VuGfx::IF()->createPipelineState(mFlavors[i].mpCompiledShaderAsset->getShaderProgram(), pVD, psParams);

		mFlavors[i].mpMaterial = VuGfxSort::IF()->createMaterial(pPS, desc);

		pPS->removeRef();
		pVD->removeRef();
	}

	return true;
}

//*****************************************************************************
void VuFoliageManager::release()
{
	for ( int i = 0; i < FLAVOR_COUNT; i++ )
	{
		VuAssetFactory::IF()->releaseAsset(mFlavors[i].mpCompiledShaderAsset);
		VuGfxSort::IF()->releaseMaterial(mFlavors[i].mpMaterial);
	}
}

//*****************************************************************************
VuFoliageManager::VuBucket *VuFoliageManager::createBucket(VuTextureAsset *pTextureAsset, bool fogEnabled)
{
	VuFlavor &flavor = mFlavors[fogEnabled];
	for ( Buckets::iterator iter = flavor.mBuckets.begin(); iter != flavor.mBuckets.end(); iter++ )
	{
		VuBucket *pBucket = *iter;

		if ( pBucket->mpTextureAsset == pTextureAsset )
		{
			pBucket->addRef();
			return pBucket;
		}
	}

	VuBucket *pBucket = new VuBucket(pTextureAsset, fogEnabled);
	flavor.mBuckets.push_back(pBucket);
	return pBucket;
}

//*****************************************************************************
void VuFoliageManager::releaseBucket(VuBucket *pBucket)
{
	VuFlavor &flavor = mFlavors[pBucket->mFogEnabled];
	if ( pBucket->removeRef() == 0 )
		flavor.mBuckets.remove(pBucket);
}

//*****************************************************************************
void VuFoliageManager::drawLayout(VuTextureAsset *pTextureAsset, bool fogEnabled, const DrawParams &params, const VuCamera &camera)
{
	VuFlavor &flavor = mFlavors[fogEnabled];

	// submit draw command
	int dataSize = sizeof(FoliageDrawData) + 4*sizeof(FoliageVertex);
	FoliageDrawData *pData = static_cast<FoliageDrawData *>(VuGfxSort::IF()->allocateCommandMemory(dataSize));
	pData->mpTextureAsset = pTextureAsset;
	pData->mCount = 1;

	VuVector3 delta = params.mPos - camera.getEyePosition();
	VuVector3 vx(delta.mY, -delta.mX, 0.0f);
	vx.normalize();

	FoliageVertex *pVerts = reinterpret_cast<FoliageVertex *>(pData + 1);
	BuildVerts(pVerts, params, vx.mX, vx.mY);

	VuGfxSort::IF()->submitDrawCommand<false>(VuGfxSort::TRANS_OPAQUE, flavor.mpMaterial, VUNULL, staticDrawCallback);
}

//*****************************************************************************
void VuFoliageManager::draw(VuBucket *pBucket, const DrawParams &params, const VuCamera &camera)
{
	VuVector3 delta = params.mPos - camera.getEyePosition();
	VuVector3 vx(delta.mY, -delta.mX, 0.0f);
	vx.normalize();

	VuBucket::VuEntry entry;
	entry.mParams = params;
	entry.mAxisXX = vx.mX;
	entry.mAxisXY = vx.mY;

	pBucket->mEntries.push_back(entry);
}

//*****************************************************************************
void VuFoliageManager::draw()
{
	for ( int i = 0; i < FLAVOR_COUNT; i++ )
	{
		VuFlavor &flavor = mFlavors[i];
		for ( Buckets::iterator iter = flavor.mBuckets.begin(); iter != flavor.mBuckets.end(); iter++ )
		{
			VuBucket *pBucket = *iter;

			int entryCount = pBucket->mEntries.size();

			if ( entryCount )
			{
				// submit draw command
				int dataSize = sizeof(FoliageDrawData) + entryCount*4*sizeof(FoliageVertex);
				FoliageDrawData *pData = static_cast<FoliageDrawData *>(VuGfxSort::IF()->allocateCommandMemory(dataSize));
				pData->mpTextureAsset = pBucket->mpTextureAsset;
				pData->mCount = entryCount;

				FoliageVertex *pVerts = reinterpret_cast<FoliageVertex *>(pData + 1);
				VuBucket::VuEntry *pEntry = &pBucket->mEntries.begin();

				for ( int iEntry = 0; iEntry < entryCount; iEntry++ )
				{
					const DrawParams &params = pEntry->mParams;

					BuildVerts(pVerts, params, pEntry->mAxisXX, pEntry->mAxisXY);
					pVerts += 4;

					pEntry++;
				}

				VuGfxSort::IF()->submitDrawCommand<false>(VuGfxSort::TRANS_FOLIAGE, flavor.mpMaterial, VUNULL, staticDrawCallback);

				pBucket->mEntries.resize(0);
			}
		}
	}
}

//*****************************************************************************
void VuFoliageManager::staticDrawCallback(void *data)
{
	VuFoliageManager::IF()->drawCallback(data);
}

//*****************************************************************************
void VuFoliageManager::drawCallback(void *data)
{
	FoliageDrawData *pData = static_cast<FoliageDrawData *>(data);

	VuGfx::IF()->setTexture(0, pData->mpTextureAsset->getTexture());

	const VUUINT16 *pIndexData = VuGfxUtil::IF()->getQuadIndexBuffer(pData->mCount);

	VuGfx::IF()->drawIndexedPrimitiveUP(
		VUGFX_PT_TRIANGLELIST,	// PrimitiveType
		0,						// MinVertexIndex
		pData->mCount*4,		// NumVertices
		pData->mCount*2,		// PrimitiveCount
		pIndexData,				// IndexData
		pData + 1				// VertexStreamZeroData
	);
}

//*****************************************************************************
void BuildVerts(FoliageVertex *pVerts, const VuFoliageManager::DrawParams &params, float axisXX, float axisXY)
{
	pVerts->mPos[0] = params.mPos.mX - params.mScaleX*axisXX;
	pVerts->mPos[1] = params.mPos.mY - params.mScaleX*axisXY;
	pVerts->mPos[2] = params.mPos.mZ;
	pVerts->mTexCoord[0] = params.mUV0.mX;
	pVerts->mTexCoord[1] = params.mUV0.mY;
	pVerts->mColor = params.mColor;
	pVerts++;

	pVerts->mPos[0] = params.mPos.mX + params.mScaleX*axisXX;
	pVerts->mPos[1] = params.mPos.mY + params.mScaleX*axisXY;
	pVerts->mPos[2] = params.mPos.mZ;
	pVerts->mTexCoord[0] = params.mUV1.mX;
	pVerts->mTexCoord[1] = params.mUV0.mY;
	pVerts->mColor = params.mColor;
	pVerts++;

	pVerts->mPos[0] = params.mPos.mX + params.mScaleX*axisXX;
	pVerts->mPos[1] = params.mPos.mY + params.mScaleX*axisXY;
	pVerts->mPos[2] = params.mPos.mZ + 2*params.mScaleZ;
	pVerts->mTexCoord[0] = params.mUV1.mX;
	pVerts->mTexCoord[1] = params.mUV1.mY;
	pVerts->mColor = params.mColor;
	pVerts++;

	pVerts->mPos[0] = params.mPos.mX - params.mScaleX*axisXX;
	pVerts->mPos[1] = params.mPos.mY - params.mScaleX*axisXY;
	pVerts->mPos[2] = params.mPos.mZ + 2*params.mScaleZ;
	pVerts->mTexCoord[0] = params.mUV0.mX;
	pVerts->mTexCoord[1] = params.mUV1.mY;
	pVerts->mColor = params.mColor;
	pVerts++;
}