//*****************************************************************************
//
//  Copyright (c) 2012-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  Tire Track Manager
// 
//*****************************************************************************

#include "VuTireTrackManager.h"
#include "VuEngine/Math/VuVector3.h"
#include "VuEngine/Components/3dDraw/Vu3dDrawComponent.h"
#include "VuEngine/Managers/VuTickManager.h"
#include "VuEngine/Dynamics/VuDynamics.h"
#include "VuEngine/Assets/VuAssetFactory.h"
#include "VuEngine/Assets/VuDBAsset.h"
#include "VuEngine/Assets/VuTextureAsset.h"
#include "VuEngine/Assets/VuCompiledShaderAsset.h"
#include "VuEngine/Gfx/GfxSort/VuGfxSort.h"
#include "VuEngine/Gfx/Camera/VuCamera.h"
#include "VuEngine/HAL/Gfx/VuGfx.h"
#include "VuEngine/HAL/Gfx/VuVertexDeclaration.h"
#include "VuEngine/HAL/Gfx/VuPipelineState.h"
#include "VuEngine/Util/VuColor.h"
#include "VuEngine/Util/VuDataUtil.h"
#include "VuEngine/Dev/VuDevStat.h"
#include "VuEngine/Dev/VuDev.h"


// the interface
IMPLEMENT_SYSTEM_COMPONENT(VuTireTrackManager, VuTireTrackManager);

// constants
#define DEFAULT_SEGMENT_COUNT 64
#define DEFAULT_NODE_COUNT 1024


class VuTireTrackMaterial
{
public:
	VuTireTrackMaterial() : mpGfxSortMaterial(VUNULL), mTexX0(0.0f), mTexX1(1.0f), mAmbientTint(1,1,1), mDiffuseTint(1,1,1) {}

	void				load(VuGfxSortMaterial *pGfxSortMaterial, const VuJsonContainer &data);

	VuGfxSortMaterial	*mpGfxSortMaterial;
	float				mTexX0;
	float				mTexX1;
	VuVector3			mAmbientTint;
	VuVector3			mDiffuseTint;
};

class VuTireTrackNode : public VuListElement<VuTireTrackNode>
{
public:
	VuVector3			mPos;
	VuVector3			mVert0;
	VuVector3			mVert1;
	float				mShadowValue;
	float				mAlpha;
	float				mDist;
};

class VuTireTrackSegment : public VuListElement<VuTireTrackSegment>
{
public:
	typedef VuList<VuTireTrackNode> Nodes;

	Nodes				mNodes;
	VuTireTrackMaterial	*mpMaterial;
	float				mWidth;
	float				mScaleV;
	float				mFadeSpeed;
	float				mDrawDist;
	float				mFadeDist;
	VuAabb				mAabb;
	bool				mOrphaned;
};

class VuTireTrack
{
public:
	VuTireTrack() : mpTireTrackType(VUNULL), mpCurMaterial(VUNULL), mpCurSegment(VUNULL) {}

	VuTireTrackParams	mParams;

	VuVector3			mPos;
	VuVector3			mNor;
	VuTireTrackType		*mpTireTrackType;
	VuTireTrackMaterial	*mpCurMaterial;
	VuTireTrackSegment	*mpCurSegment;
};

class VuTireTrackType
{
public:
	VuTireTrackType() : mpMaterials(VUNULL), mpGfxSortMaterial(VUNULL) {}
	VuTireTrackMaterial	*mpMaterials;
	VuGfxSortMaterial	*mpGfxSortMaterial;
};

struct VuTireTrackDrawData
{
	int		mVertexCount;
};

class VuTireTrackVertex
{
public:
	float		mXyz[3];
	float		mUv[2];
	VUUINT32	mColor;
};


//*****************************************************************************
VuTireTrackManager::VuTireTrackManager():
	mMaxSegmentCount(0),
	mMaxNodeCount(0),
	mpSegments(VUNULL),
	mpNodes(VUNULL)
{
	mp3dDrawComponent = new Vu3dDrawComponent(VUNULL);
	mp3dDrawComponent->setDrawMethod(this, &VuTireTrackManager::draw);
	mp3dDrawComponent->updateVisibility(VuAabb(VuVector3(-1e9, -1e9, -1e9), VuVector3(1e9, 1e9, 1e9)));
}

//*****************************************************************************
VuTireTrackManager::~VuTireTrackManager()
{
	delete mp3dDrawComponent;
}

//*****************************************************************************
bool VuTireTrackManager::init()
{
	// configure w/ default settings
	configure(DEFAULT_SEGMENT_COUNT, DEFAULT_NODE_COUNT);

	VuTickManager::IF()->registerHandler(this, &VuTireTrackManager::tick, "Final");

	// get vertex shaders
	VuCompiledShaderAsset *pShaderAsset = VuAssetFactory::IF()->createAsset<VuCompiledShaderAsset>("TireTrack");

	// create vertex declaration
	VuVertexDeclarationParams vdParams;
	vdParams.mElements.push_back(VuVertexDeclarationElement(0,  0, VUGFX_DECL_TYPE_FLOAT3,  VUGFX_DECL_USAGE_POSITION, 0));
	vdParams.mElements.push_back(VuVertexDeclarationElement(0, 12, VUGFX_DECL_TYPE_FLOAT2,  VUGFX_DECL_USAGE_TEXCOORD, 0));
	vdParams.mElements.push_back(VuVertexDeclarationElement(0, 20, VUGFX_DECL_TYPE_UBYTE4N, VUGFX_DECL_USAGE_COLOR,    0));
	vdParams.mStreams.push_back(VuVertexDeclarationStream(24));
	VuVertexDeclaration *pVD = VuGfx::IF()->createVertexDeclaration(vdParams, pShaderAsset->getShaderProgram());

	// create pipeline state
	VuPipelineStateParams psParams;
	psParams.mAlphaBlendEnabled = true;
	psParams.mSrcBlendMode = VUGFX_BLEND_ONE;
	psParams.mDstBlendMode = VUGFX_BLEND_INVSRCALPHA;
	VuPipelineState *pPS = VuGfx::IF()->createPipelineState(pShaderAsset->getShaderProgram(), pVD, psParams);

	VuDBAsset *pSurfaceMapDBAsset = VuAssetFactory::IF()->createAsset<VuDBAsset>("SurfaceTableDB");
	{
		const VuJsonContainer &types = pSurfaceMapDBAsset->getDB()["TireTracks"];
		for ( int iType = 0; iType < types.numMembers(); iType++ )
		{
			const std::string &typeName = types.getMemberKey(iType);
			const VuJsonContainer &data = types[typeName];
			VuTireTrackType &type = mTireTrackTypes[typeName];

			// create gfx material
			VuGfxSortMaterialDesc desc;
			desc.addTexture("tex0", VuGfxSortMaterialDesc::TEXTURE, data["Texture"].asCString());
			type.mpGfxSortMaterial = VuGfxSort::IF()->createMaterial(pPS, desc);

			// create tire track materials
			int surfaceTypeCount = VuDynamics::IF()->getSurfaceTypeCount();
			type.mpMaterials = new VuTireTrackMaterial[surfaceTypeCount*2];

			for ( VUUINT8 i = 0; i < surfaceTypeCount; i++ )
			{
				const std::string &surfaceName = VuDynamics::IF()->getSurfaceTypeName(i);

				type.mpMaterials[i*2 + 0].load(type.mpGfxSortMaterial, data["SlideOff"][surfaceName]);
				type.mpMaterials[i*2 + 1].load(type.mpGfxSortMaterial, data["SlideOn"][surfaceName]);
			}
		}
	}
	VuAssetFactory::IF()->releaseAsset(pSurfaceMapDBAsset);

	pPS->removeRef();
	pVD->removeRef();
	VuAssetFactory::IF()->releaseAsset(pShaderAsset);

	mp3dDrawComponent->show();

	if ( VuDevStat::IF() )
		VuDevStat::IF()->addPage("TireTracks", VuRect(50, 10, 40, 40));

	return true;
}

//*****************************************************************************
void VuTireTrackManager::release()
{
	VUASSERT(mTireTracks.size() == 0, "VuTireTrackManager::release() called with active tire tracks");

	mp3dDrawComponent->hide();

	VuTickManager::IF()->unregisterHandlers(this);

	for ( TireTrackTypes::iterator iter = mTireTrackTypes.begin(); iter != mTireTrackTypes.end(); iter++ )
	{
		VuGfxSort::IF()->releaseMaterial(iter->second.mpGfxSortMaterial);
		delete[] iter->second.mpMaterials;
	}
	mTireTrackTypes.clear();

	delete[] mpSegments;
	delete[] mpNodes;
}

//*****************************************************************************
void VuTireTrackManager::configure(int maxSegmentCount, int maxNodeCount)
{
	VUASSERT(mTireTracks.size() == 0, "VuTireTrackManager::configure() called with active tire tracks");

	delete[] mpSegments;
	mMaxSegmentCount = maxSegmentCount;
	mpSegments = new VuTireTrackSegment[maxSegmentCount];
	memset(mpSegments, 0, maxSegmentCount*sizeof(mpSegments[0]));
	for ( int i = 0; i < maxSegmentCount; i++ )
		mFreeSegments.push_back(&mpSegments[i]);

	delete[] mpNodes;
	mMaxNodeCount = maxNodeCount;
	mpNodes = new VuTireTrackNode[maxNodeCount];
	memset(mpNodes, 0, maxNodeCount*sizeof(mpNodes[0]));
	for ( int i = 0; i < maxNodeCount; i++ )
		mFreeNodes.push_back(&mpNodes[i]);
}

//*****************************************************************************
void VuTireTrackManager::reset()
{
	VUASSERT(mTireTracks.size() == 0, "VuTireTrackManager::reset() called with active tire tracks");

	while ( mActiveSegments.front() )
		freeSegment(mActiveSegments.front());
}

//*****************************************************************************
VuTireTrack *VuTireTrackManager::createTireTrack(const VuTireTrackParams &params)
{
	TireTrackTypes::iterator iter = mTireTrackTypes.find(params.mType);
	if ( iter == mTireTrackTypes.end() )
		return VUNULL;

	VuTireTrack *pTireTrack = new VuTireTrack;
	mTireTracks.push_back(pTireTrack);

	pTireTrack->mParams = params;
	pTireTrack->mpTireTrackType = &iter->second;

	return pTireTrack;
}

//*****************************************************************************
void VuTireTrackManager::releaseTireTrack(VuTireTrack *pTireTrack)
{
	mTireTracks.remove(pTireTrack);
	delete pTireTrack;
}

//*****************************************************************************
void VuTireTrackManager::updateTireTrack(VuTireTrack *pTireTrack, bool contact, bool skid, VUUINT8 surfaceType, const VuVector3 &pos, const VuVector3 &nor, const VuVector3 &vel, float shadowValue, float scaleX)
{
	VuVector3 axisX, offset;

	float tireTrackWidth = pTireTrack->mParams.mWidth*scaleX;

	// determine new material
	VuTireTrackMaterial *pNewMaterial = VUNULL;
	if ( contact )
	{
		VuVector3 dir = vel - nor*VuDot(nor, vel);
		if ( dir.mag() > pTireTrack->mParams.mMinVel )
		{
			VuTireTrackMaterial *pMaterial = &pTireTrack->mpTireTrackType->mpMaterials[surfaceType*2 + skid];
			if ( pMaterial->mpGfxSortMaterial )
			{
				pNewMaterial = pMaterial;

				axisX = VuCross(dir, nor);
				axisX.safeNormalize();

				offset = nor*pTireTrack->mParams.mOffsetZ + axisX*pTireTrack->mParams.mOffsetX;
			}
		}
	}

	VuTireTrackSegment *pCurSegment = pTireTrack->mpCurSegment;
	VuTireTrackNode *pCurNode = VUNULL;
	VuTireTrackNode *pPrevNode = VUNULL;
	if ( pCurSegment )
	{
		pCurNode = pCurSegment->mNodes.back();
		pPrevNode = pCurNode->prev();
	}

	if ( pNewMaterial != pTireTrack->mpCurMaterial )
	{
		// turn off tire track
		if ( pTireTrack->mpCurSegment )
			pTireTrack->mpCurSegment->mOrphaned = true;
		pTireTrack->mpCurMaterial = VUNULL;
		pTireTrack->mpCurSegment = VUNULL;

		if ( pNewMaterial )
		{
			// turn on tire track or switch materials
			if ( VuTireTrackSegment *pSegment = createSegment() )
			{
				pSegment->mpMaterial = pNewMaterial;
				pSegment->mWidth = tireTrackWidth;
				pSegment->mScaleV = pTireTrack->mParams.mScaleV;
				pSegment->mFadeSpeed = 1.0f/pTireTrack->mParams.mFadeTime;
				pSegment->mDrawDist = pTireTrack->mParams.mDrawDist;
				pSegment->mFadeDist = pTireTrack->mParams.mFadeDist;
				pSegment->mOrphaned = false;

				pTireTrack->mpCurMaterial = pNewMaterial;
				pTireTrack->mpCurSegment = pSegment;

				if ( pCurNode )
					copyNode(pCurNode, pSegment->mNodes.front());
				else
					setNode(pSegment->mNodes.front(), pos, offset, axisX, tireTrackWidth, 0.0f, shadowValue);
				setNode(pSegment->mNodes.back(), pos, offset, axisX, tireTrackWidth, 0.0f, shadowValue);
			}
		}
	}
	else if ( pTireTrack->mpCurMaterial )
	{
		float distPrev = VuDist(pPrevNode->mPos, pos);
		if ( distPrev > pTireTrack->mParams.mMinDist )
		{
			// new node
			if ( VuTireTrackNode *pNewNode = createNode(pCurSegment) )
			{
				setNode(pNewNode, pos, offset, axisX, tireTrackWidth, pPrevNode->mDist + distPrev, shadowValue);
			}
			else
			{
				// turn off tire track (out of nodes)
				if ( pTireTrack->mpCurSegment )
					pTireTrack->mpCurSegment->mOrphaned = true;
				pTireTrack->mpCurMaterial = VUNULL;
				pTireTrack->mpCurSegment = VUNULL;
			}
		}
		else
		{
			// update node
			setNode(pCurNode, pos, offset, axisX, tireTrackWidth, pPrevNode->mDist + distPrev, shadowValue);
		}
	}
}

//*****************************************************************************
VuTireTrackSegment *VuTireTrackManager::createSegment()
{
	if ( mFreeSegments.size() && mFreeNodes.size() >= 2 )
	{
		// remove from free list
		VuTireTrackSegment *pSegment = mFreeSegments.pop_back();

		// add to active list
		mActiveSegments.push_back(pSegment);

		createNode(pSegment);
		createNode(pSegment);

		return pSegment;
	}

	return VUNULL;
}

//*****************************************************************************
void VuTireTrackManager::freeSegment(VuTireTrackSegment *pSegment)
{
	while ( pSegment->mNodes.back() )
		freeNode(pSegment, pSegment->mNodes.back());

	mActiveSegments.remove(pSegment);
	mFreeSegments.push_back(pSegment);
}

//*****************************************************************************
VuTireTrackNode *VuTireTrackManager::createNode(VuTireTrackSegment *pSegment)
{
	if ( mFreeNodes.size() )
	{
		VuTireTrackNode *pNode = mFreeNodes.pop_back();
		pSegment->mNodes.push_back(pNode);
		return pNode;
	}

	return VUNULL;
}

//*****************************************************************************
void VuTireTrackManager::freeNode(VuTireTrackSegment *pSegment, VuTireTrackNode *pNode)
{
	pSegment->mNodes.remove(pNode);
	mFreeNodes.push_back(pNode);
}

//*****************************************************************************
void VuTireTrackManager::copyNode(VuTireTrackNode *pSrcNode, VuTireTrackNode *pDstNode)
{
	pDstNode->mPos = pSrcNode->mPos;
	pDstNode->mVert0 = pSrcNode->mVert0;
	pDstNode->mVert1 = pSrcNode->mVert1;
	pDstNode->mShadowValue = pSrcNode->mShadowValue;
	pDstNode->mAlpha = pSrcNode->mAlpha;
	pDstNode->mDist = 0.0f;
}

//*****************************************************************************
void VuTireTrackManager::setNode(VuTireTrackNode *pNode, const VuVector3 &pos, const VuVector3 &offset, const VuVector3 &axisX, float width, float dist, float shadowValue)
{
	VuVector3 extent = (0.5f*width)*axisX;

	pNode->mPos = pos + offset;
	pNode->mVert0 = pNode->mPos - extent;
	pNode->mVert1 = pNode->mPos + extent;
	pNode->mShadowValue = shadowValue;
	pNode->mAlpha = 1.0f;
	pNode->mDist = dist;
}

//*****************************************************************************
void VuTireTrackManager::tick(float fdt)
{
	VuTireTrackSegment *pSegment = mActiveSegments.front();
	while ( pSegment )
	{
		VuTireTrackSegment *pNextSegment = pSegment->next();

		pSegment->mAabb.reset();
		float halfWidth = 0.5f*pSegment->mWidth;

		VuTireTrackNode *pNode = pSegment->mNodes.front();
		while ( pNode )
		{
			VuTireTrackNode *pNextNode = pNode->next();

			pNode->mAlpha -= pSegment->mFadeSpeed*fdt;
			if ( pNode->mAlpha <= 0.0f )
			{
				pNode->mAlpha = 0.0f;

				if ( pNextNode || pSegment->mOrphaned )
				{
					// free any nodes before this one
					while ( pNode->prev() )
						freeNode(pSegment, pNode->prev());
				}
			}

			pSegment->mAabb.addSphere(pNode->mPos, halfWidth);

			pNode = pNextNode;
		}

		// free segment?
		if ( pSegment->mNodes.back()->prev() == VUNULL )
			freeSegment(pSegment);

		pSegment = pNextSegment;
	}

	updateDevStats();
}

//*****************************************************************************
void VuTireTrackManager::updateDevStats()
{
	// dev stats
	if ( VuDevStat::IF() )
	{
		if ( VuDevStatPage *pPage = VuDevStat::IF()->getCurPage() )
		{
			if ( pPage->getName() == "TireTracks" )
			{
				pPage->clear();

				// running services
				{
					pPage->printf("Tire Tracks: %3d\n", mTireTracks.size());
					pPage->printf("Segments:    %3d / %d\n", mMaxSegmentCount - mFreeSegments.size(), mMaxSegmentCount);
					pPage->printf("Nodes:       %3d / %d\n", mMaxNodeCount - mFreeNodes.size(), mMaxNodeCount);
				}
			}
		}
	}
}

//*****************************************************************************
void VuTireTrackManager::draw(const VuGfxDrawParams &params)
{
	VuVector3 eyePos = params.mCamera.getEyePosition();

	VuTireTrackSegment *pSegment = mActiveSegments.front();
	while ( pSegment )
	{
		VuVector3 center = pSegment->mAabb.getCenter();
		float radius = pSegment->mAabb.getExtents().mag();
		if ( VuDist(center, eyePos) - radius < pSegment->mDrawDist )
		{
			if ( params.mCamera.isSphereVisible(center, radius) )
			{
				VuTireTrackMaterial *pMaterial = pSegment->mpMaterial;
				float texX0 = pMaterial->mTexX0;
				float texX1 = pMaterial->mTexX1;
				float scaleV = (texX1 - texX0)/(pSegment->mScaleV*pSegment->mWidth);

				int vertexCount = pSegment->mNodes.size()*2;

				// submit draw command
				int dataSize = sizeof(VuTireTrackDrawData) + vertexCount*sizeof(VuTireTrackVertex);
				VuTireTrackDrawData *pData = static_cast<VuTireTrackDrawData *>(VuGfxSort::IF()->allocateCommandMemory(dataSize));
				pData->mVertexCount = vertexCount;

				VuTireTrackVertex *pVert = reinterpret_cast<VuTireTrackVertex *>(pData + 1);

				VuTireTrackNode *pNode = pSegment->mNodes.front();
				while ( pNode )
				{
					VuVector3 tint = VuLerp(pMaterial->mAmbientTint, pMaterial->mDiffuseTint, pNode->mShadowValue);
					float fade = 1.0f - VuLinStep(pSegment->mFadeDist, pSegment->mDrawDist, VuDist(pNode->mPos, eyePos));
					float alpha = pNode->mAlpha*fade;
					tint *= alpha;
					VuColor color;
					color.fromFloat4(tint.mX, tint.mY, tint.mZ, alpha);

					pVert->mXyz[0] = pNode->mVert0.mX; pVert->mXyz[1] = pNode->mVert0.mY; pVert->mXyz[2] = pNode->mVert0.mZ;
					pVert->mUv[0] = texX0; pVert->mUv[1] = pNode->mDist*scaleV;
					pVert->mColor = color;
					pVert++;

					pVert->mXyz[0] = pNode->mVert1.mX; pVert->mXyz[1] = pNode->mVert1.mY; pVert->mXyz[2] = pNode->mVert1.mZ;
					pVert->mUv[0] = texX1; pVert->mUv[1] = pNode->mDist*scaleV;
					pVert->mColor = color;
					pVert++;

					pNode = pNode->next();
				}

				// draw
				VuGfxSort::IF()->submitDrawCommand<false>(VuGfxSort::TRANS_TIRE_TRACK, pMaterial->mpGfxSortMaterial, VUNULL, staticDrawCallback);
			}
		}

		pSegment = pSegment->next();
	}
}

//*****************************************************************************
void VuTireTrackManager::staticDrawCallback(void *data)
{
	VuTireTrackManager::IF()->drawCallback(data);
}

//*****************************************************************************
void VuTireTrackManager::drawCallback(void *data)
{
	VuTireTrackDrawData *pData = static_cast<VuTireTrackDrawData *>(data);

	VuGfx::IF()->drawPrimitiveUP(VUGFX_PT_TRIANGLESTRIP, pData->mVertexCount - 2, pData + 1);
}

//*****************************************************************************
void VuTireTrackMaterial::load(VuGfxSortMaterial *pGfxSortMaterial, const VuJsonContainer &data)
{
	if ( data.isObject() )
	{
		mpGfxSortMaterial = pGfxSortMaterial;

		VuColor ambientTint(255,255,255);
		VuColor diffuseTint(255,255,255);

		VuDataUtil::getValue(data["TexX0"], mTexX0);
		VuDataUtil::getValue(data["TexX1"], mTexX1);
		VuDataUtil::getValue(data["AmbientTint"], ambientTint);
		VuDataUtil::getValue(data["DiffuseTint"], diffuseTint);

		mAmbientTint = ambientTint.toVector3();
		mDiffuseTint = diffuseTint.toVector3();
	}
}
