//*****************************************************************************
//
//  Copyright (c) 2009-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  Static Model Instance
// 
//*****************************************************************************

#include "VuStaticModelInstance.h"
#include "VuEngine/Gfx/Camera/VuCamera.h"
#include "VuEngine/Gfx/Shaders/VuShadowShader.h"
#include "VuEngine/Gfx/Shaders/VuDropShadowShader.h"
#include "VuEngine/Gfx/Shadow/VuShadowVolume.h"
#include "VuEngine/Gfx/GfxScene/VuGfxStaticScene.h"
#include "VuEngine/Gfx/GfxScene/VuGfxSceneNode.h"
#include "VuEngine/Gfx/GfxScene/VuGfxSceneChunk.h"
#include "VuEngine/Gfx/GfxScene/VuGfxSceneMaterial.h"
#include "VuEngine/Gfx/GfxScene/VuGfxSceneMesh.h"
#include "VuEngine/Gfx/GfxScene/VuGfxSceneMeshInstance.h"
#include "VuEngine/Gfx/GfxScene/VuGfxSceneMeshPart.h"
#include "VuEngine/Gfx/VuGfxUtil.h"
#include "VuEngine/Gfx/GfxSort/VuGfxSort.h"
#include "VuEngine/Assets/VuAssetFactory.h"
#include "VuEngine/Assets/VuStaticModelAsset.h"
#include "VuEngine/Assets/VuMaterialAsset.h"
#include "VuEngine/HAL/Gfx/VuVertexDeclaration.h"
#include "VuEngine/HAL/Gfx/VuVertexBuffer.h"
#include "VuEngine/HAL/Gfx/VuIndexBuffer.h"
#include "VuEngine/HAL/Gfx/VuTexture.h"
#include "VuEngine/Math/VuPackedVector.h"
#include "VuEngine/Math/VuMathUtil.h"


// constants
#define DEPTH_PASS_OFFSET 0.001f

// static functions
struct ModelInstanceDrawData
{
	VuMatrix					mTransform;
	VuColor						mColor;
	float						mWaterZ;
	VuGfxSceneMeshPart			*mpPart;
	VuColor						mDynamicLightColor;
	VUUINT32					mDynamicLightGroupMask;
	VuVertexBuffer				*mpVertexColors;
	VuMaterialAsset::eFlavor	mFlavor;
};
static void ModelInstanceDrawCallback(void *data);

struct ModelInstanceDrawSSAODepthData
{
	VuMatrix			mTransform;
	VuGfxSceneMeshPart	*mpPart;
};
static void ModelInstanceDrawSSAODepthCallback(void *data);

struct ModelInstanceDrawShadowData
{
	VuMatrix			mTransform;
	VuGfxSceneMeshPart	*mpPart;
};
static void ModelInstanceDrawShadowCallback(void *data);
static void ModelInstanceDrawDropShadowCallback(void *data);


//*****************************************************************************
VuStaticModelInstance::VuStaticModelInstance() :
	mpModelAsset(VUNULL),
	mpGfxStaticScene(VUNULL),
	mRejectionScaleModifier(1.0f),
	mppVertexBuffers(VUNULL),
	mVertexBufferCount(0)
{
}

//*****************************************************************************
VuStaticModelInstance::~VuStaticModelInstance()
{
	reset();
}

//*****************************************************************************
void VuStaticModelInstance::reset()
{
	VuGfxSort::IF()->flush();

	if ( mpGfxStaticScene )
	{
		mpGfxStaticScene->removeRef();
		mpGfxStaticScene = VUNULL;
	}

	if ( mpModelAsset )
	{
		VuAssetFactory::IF()->releaseAsset(mpModelAsset);
		mpModelAsset = VUNULL;
	}

	clearVertexColors();

	onReset();
}

//*****************************************************************************
void VuStaticModelInstance::setModelAsset(const std::string &assetName)
{
	if ( mpModelAsset && mpModelAsset->getAssetName() == assetName )
		return;

	reset();

	if ( VuAssetFactory::IF()->doesAssetExist<VuStaticModelAsset>(assetName) )
	{
		mpModelAsset = VuAssetFactory::IF()->createAsset<VuStaticModelAsset>(assetName);

		if ( (mpGfxStaticScene = mpModelAsset->getGfxStaticScene()) != VUNULL)
		{
			mpGfxStaticScene->addRef();

			onSetModel();
		}
	}
}

//*****************************************************************************
void VuStaticModelInstance::setModel(VuGfxStaticScene *pGfxStaticScene)
{
	if ( mpGfxStaticScene == pGfxStaticScene )
		return;

	reset();

	mpGfxStaticScene = pGfxStaticScene;
	mpGfxStaticScene->addRef();

	onSetModel();
}

//*****************************************************************************
const VuAabb &VuStaticModelInstance::getAabb() const
{
	if ( mpGfxStaticScene )
		return mpGfxStaticScene->mStaticInfo.mAabb;

	return VuAabb::zero();
}

//*****************************************************************************
void VuStaticModelInstance::draw(const VuMatrix &modelMat, const VuGfxDrawParams &params) const
{
	if ( mpGfxStaticScene )
		for ( VuGfxStaticScene::Nodes::iterator iter = mpGfxStaticScene->mNodes.begin(); iter != mpGfxStaticScene->mNodes.end(); iter++ )
			drawRecursive(*iter, modelMat, params);
}

//*****************************************************************************
void VuStaticModelInstance::drawShadow(const VuMatrix &modelMat, const VuGfxDrawShadowParams &params) const
{
	if ( mpGfxStaticScene )
		for ( VuGfxStaticScene::Nodes::iterator iter = mpGfxStaticScene->mNodes.begin(); iter != mpGfxStaticScene->mNodes.end(); iter++ )
			drawShadowRecursive(*iter, modelMat, params);
}

//*****************************************************************************
void VuStaticModelInstance::drawDropShadow(const VuMatrix &modelMat, const VuGfxDrawShadowParams &params) const
{
	if ( mpGfxStaticScene )
		for ( VuGfxStaticScene::Nodes::iterator iter = mpGfxStaticScene->mNodes.begin(); iter != mpGfxStaticScene->mNodes.end(); iter++ )
			drawDropShadowRecursive(*iter, modelMat, params);
}

//*****************************************************************************
void VuStaticModelInstance::drawPrefetch() const
{
	if ( mpGfxStaticScene )
	{
		for ( VuGfxStaticScene::Meshes::iterator iter = mpGfxStaticScene->mMeshes.begin(); iter != mpGfxStaticScene->mMeshes.end(); iter++ )
		{
			VuGfxSceneMesh *pMesh = *iter;

			for ( VuGfxSceneMesh::Parts::iterator itPart = pMesh->mParts.begin(); itPart != pMesh->mParts.end(); itPart++ )
			{
				VuGfxSceneMeshPart *pPart = *itPart;
				VuGfxSceneMaterial *pMaterial = pPart->mpMaterial;
				VuMaterialAsset *pMaterialAsset = pMaterial->mpMaterialAsset;
				VuGfxSceneChunk *pChunk = pPart->mpChunk;
				VuGfxSortMaterial *pGfxSortMaterial = pMaterialAsset->mpGfxSortMaterials[VuMaterialAsset::FLV_OPAQUE];

				// substitute material
				if ( mpMaterialSubstIF )
					pGfxSortMaterial = mpMaterialSubstIF->substituteMaterial(VuMaterialAsset::FLV_OPAQUE, pMaterial->mIndex);

				// submit draw command
				ModelInstanceDrawData *pData = static_cast<ModelInstanceDrawData *>(VuGfxSort::IF()->allocateCommandMemory(sizeof(ModelInstanceDrawData)));
				pData->mTransform = VuMatrix::identity();
				pData->mColor = mColor;
				pData->mWaterZ = mWaterZ;
				pData->mpPart = pPart;
				pData->mDynamicLightColor = mDynamicLightColor;
				pData->mDynamicLightGroupMask = mDynamicLightGroupMask;
				if ( pChunk->mIndex < mVertexBufferCount )
					pData->mpVertexColors = mppVertexBuffers[pChunk->mIndex];
				else
					pData->mpVertexColors = VUNULL;
				pData->mFlavor = VuMaterialAsset::FLV_OPAQUE;

				VuGfxSort::IF()->submitDrawCommand<false>(VuGfxSort::TRANS_OPAQUE, pGfxSortMaterial, pChunk->mpGfxSortMesh, ModelInstanceDrawCallback, 0.5f);
			}
		}
	}
}

//*****************************************************************************
void VuStaticModelInstance::drawInfo(const VuMatrix &modelMat, const VuGfxDrawInfoParams &params) const
{
	if ( mpGfxStaticScene && params.mCamera.isAabbVisible(mpGfxStaticScene->mStaticInfo.mAabb, modelMat) )
	{
		for ( VuGfxStaticScene::Nodes::iterator iter = mpGfxStaticScene->mNodes.begin(); iter != mpGfxStaticScene->mNodes.end(); iter++ )
			drawInfoRecursive(*iter, modelMat, params);

		// scene info
		char str[256];
		VU_SPRINTF(str, sizeof(str),
			"%5d Stored Meshes\n"
			"%5d Stored Mesh Parts\n"
			"%5d Stored Verts\n"
			"%5d Stored Tris\n"
			"%5d Materials\n"
			"%5d Nodes\n"
			"%5d Mesh Instances\n"
			"%5d Drawn Verts\n"
			"%5d Num Drawn Tris\n",
			mpGfxStaticScene->mInfo.mNumMeshes,
			mpGfxStaticScene->mInfo.mNumMeshParts,
			mpGfxStaticScene->mInfo.mNumVerts,
			mpGfxStaticScene->mInfo.mNumTris,
			mpGfxStaticScene->mInfo.mNumMaterials,
			mpGfxStaticScene->mStaticInfo.mNumNodes,
			mpGfxStaticScene->mStaticInfo.mNumMeshInstances,
			mpGfxStaticScene->mStaticInfo.mNumDrawnVerts,
			mpGfxStaticScene->mStaticInfo.mNumDrawnTris
		);
		drawSceneInfo(modelMat, params, str);
	}
}

//*****************************************************************************
bool VuStaticModelInstance::collideRay(const VuMatrix &modelMat, const VuVector3 &v0, VuVector3 &v1, bool shadowing) const
{
	bool hit = false;

	if ( mpGfxStaticScene )
	{
		for ( VuGfxStaticScene::Nodes::iterator iter = mpGfxStaticScene->mNodes.begin(); iter != mpGfxStaticScene->mNodes.end(); iter++ )
		{
			if ( shadowing )
				hit |= collideRayRecursive<true>(*iter, modelMat, v0, v1);
			else
				hit |= collideRayRecursive<false>(*iter, modelMat, v0, v1);
		}
	}

	return hit;
}

//*****************************************************************************
bool VuStaticModelInstance::collideSphere(const VuMatrix &modelMat, const VuVector3 &pos, float radius) const
{
	bool hit = false;

	if ( mpGfxStaticScene )
		for ( VuGfxStaticScene::Nodes::iterator iter = mpGfxStaticScene->mNodes.begin(); iter != mpGfxStaticScene->mNodes.end(); iter++ )
			hit |= collideSphereRecursive(*iter, modelMat, pos, radius);

	return hit;
}

//*****************************************************************************
bool VuStaticModelInstance::setVertexColors(VuVertexBuffer **ppBuffers, int count)
{
	mppVertexBuffers = VUNULL;
	mVertexBufferCount = 0;

	// validate
	if ( count && mpGfxStaticScene )
	{
		int chunkCount = (int)mpGfxStaticScene->mChunks.size();
		if ( chunkCount == count )
		{
			for ( int i = 0; i < chunkCount; i++ )
			{
				VuGfxSceneChunk *pChunk = mpGfxStaticScene->mChunks[i];
				int vertexCount = pChunk->mpVertexBuffer->mSize/pChunk->mVertexStride;

				if ( vertexCount != ppBuffers[i]->mSize/4 )
					return false;
			}
		}

		mppVertexBuffers = ppBuffers;
		mVertexBufferCount = count;
	}

	return true;
}

//*****************************************************************************
void VuStaticModelInstance::drawRecursive(VuGfxSceneNode *pNode, const VuMatrix &transform, const VuGfxDrawParams &params) const
{
	if ( params.isVisible(pNode->mAabb, transform, mRejectionScaleModifier) )
	{
		VuMatrix nodeTransform = pNode->mTransform*transform;

		if ( pNode->mpMeshInstance )
		{
			VuGfxSceneMesh *pMesh = pNode->mpMeshInstance->mpMesh;

			// calculate depth
			VuVector3 vPos = nodeTransform.transform(pMesh->getAabb().getCenter());
			float depth = (vPos - params.mCamera.getEyePosition()).mag()/params.mCamera.getFarPlane();
			depth = VuMin(depth, 1.0f);

			for ( VuGfxSceneMesh::Parts::iterator itPart = pMesh->mParts.begin(); itPart != pMesh->mParts.end(); itPart++ )
			{
				VuGfxSceneMeshPart *pPart = *itPart;

				if ( params.isVisible(pPart->mAabb, nodeTransform, mRejectionScaleModifier) )
				{
					VuGfxSceneMaterial *pMaterial = pPart->mpMaterial;
					VuMaterialAsset *pMaterialAsset = pMaterial->mpMaterialAsset;
					VuGfxSceneChunk *pChunk = pPart->mpChunk;

					VuGfxSort::eTranslucencyType transType = (VuGfxSort::eTranslucencyType)pMaterialAsset->mTranslucencyType;
					if ( (mColor.mA != 255) && (transType <= VuGfxSort::TRANS_ALPHA_TEST) )
					{
						VuGfxSortMaterial *pModulatedMaterial = pMaterialAsset->mpGfxSortMaterials[VuMaterialAsset::FLV_MODULATED];

						// substitute material
						if ( mpMaterialSubstIF )
							pModulatedMaterial = mpMaterialSubstIF->substituteMaterial(VuMaterialAsset::FLV_MODULATED, pMaterial->mIndex);

						ModelInstanceDrawData *pData = static_cast<ModelInstanceDrawData *>(VuGfxSort::IF()->allocateCommandMemory(sizeof(ModelInstanceDrawData)));
						pData->mTransform = nodeTransform;
						pData->mColor = mColor;
						pData->mWaterZ = mWaterZ;
						pData->mpPart = pPart;
						pData->mDynamicLightColor = mDynamicLightColor;
						pData->mDynamicLightGroupMask = mDynamicLightGroupMask;
						if ( pChunk->mIndex < mVertexBufferCount )
							pData->mpVertexColors = mppVertexBuffers[pChunk->mIndex];
						else
							pData->mpVertexColors = VUNULL;
						pData->mFlavor = VuMaterialAsset::FLV_MODULATED;
							
						if ( mbTranslucentDepthEnabled )
						{
							VuGfxSortMaterial *pDepthMaterial = pMaterialAsset->mpGfxSortMaterials[VuMaterialAsset::FLV_DEPTH];

							// depth pass
							VuGfxSort::IF()->submitDrawCommand<false>(VuGfxSort::TRANS_DEPTH_PASS, pDepthMaterial, pChunk->mpGfxSortMesh, ModelInstanceDrawCallback, depth);

							// color pass
							VuGfxSort::IF()->submitDrawCommand<true>(VuGfxSort::TRANS_COLOR_PASS, pModulatedMaterial, pChunk->mpGfxSortMesh, ModelInstanceDrawCallback, depth);
						}
						else
						{
							VuGfxSort::IF()->submitDrawCommand<true>(VuGfxSort::TRANS_MODULATE_ABOVE_WATER, pModulatedMaterial, pChunk->mpGfxSortMesh, ModelInstanceDrawCallback, depth);
						}
					}
					else
					{
						VuMaterialAsset::eFlavor flavor = VuMaterialAsset::getFlavor(transType);

						VuGfxSortMaterial *pGfxSortMaterial = pMaterialAsset->mpGfxSortMaterials[flavor];

						// substitute material
						if ( mpMaterialSubstIF )
							pGfxSortMaterial = mpMaterialSubstIF->substituteMaterial(flavor, pMaterial->mIndex);

						// submit draw command
						ModelInstanceDrawData *pData = static_cast<ModelInstanceDrawData *>(VuGfxSort::IF()->allocateCommandMemory(sizeof(ModelInstanceDrawData)));
						pData->mTransform = nodeTransform;
						pData->mColor = mColor;
						pData->mWaterZ = mWaterZ;
						pData->mpPart = pPart;
						pData->mDynamicLightColor = mDynamicLightColor;
						pData->mDynamicLightGroupMask = mDynamicLightGroupMask;
						if ( pChunk->mIndex < mVertexBufferCount )
							pData->mpVertexColors = mppVertexBuffers[pChunk->mIndex];
						else
							pData->mpVertexColors = VUNULL;
						pData->mFlavor = flavor;

						if ( pMaterialAsset->mbDepthSort )
							VuGfxSort::IF()->submitDrawCommand<true>(transType, pGfxSortMaterial, pChunk->mpGfxSortMesh, ModelInstanceDrawCallback, depth);
						else
							VuGfxSort::IF()->submitDrawCommand<false>(transType, pGfxSortMaterial, pChunk->mpGfxSortMesh, ModelInstanceDrawCallback, depth);

						// submit SSAO commands
						if ( params.mbDrawSSAO && pMaterialAsset->mbDoesSSAO && (flavor == VuMaterialAsset::FLV_OPAQUE) )
						{
							VUUINT prevVPL = VuGfxSort::IF()->getViewportLayer();

							ModelInstanceDrawSSAODepthData *pData = static_cast<ModelInstanceDrawSSAODepthData *>(VuGfxSort::IF()->allocateCommandMemory(sizeof(ModelInstanceDrawSSAODepthData)));
							pData->mTransform = nodeTransform;
							pData->mpPart = pPart;

							VuGfxSort::IF()->setViewportLayer(VuGfxSort::VPL_SSAO);
							VuGfxSort::IF()->submitDrawCommand<false>(VuGfxSort::TRANS_OPAQUE, pMaterialAsset->mpGfxSortSSAODepthMaterial, pChunk->mpGfxSortMesh, ModelInstanceDrawSSAODepthCallback, depth);

							VuGfxSort::IF()->setViewportLayer(prevVPL);
						}
					}
				}
			}
		}

		// recurse
		for ( VuGfxSceneNode::Children::iterator iter = pNode->mChildren.begin(); iter != pNode->mChildren.end(); iter++ )
			drawRecursive(*iter, nodeTransform, params);
	}
}

//*****************************************************************************
void VuStaticModelInstance::drawShadowRecursive(VuGfxSceneNode *pNode, const VuMatrix &transform, const VuGfxDrawShadowParams &params) const
{
	if ( params.isVisible(pNode->mAabb, transform, mRejectionScaleModifier) )
	{
		VuMatrix nodeTransform = pNode->mTransform*transform;

		if ( pNode->mpMeshInstance )
		{
			VuGfxSceneMesh *pMesh = pNode->mpMeshInstance->mpMesh;

			for ( VuGfxSceneMesh::Parts::iterator iter = pMesh->mParts.begin(); iter != pMesh->mParts.end(); iter++ )
			{
				VuGfxSceneMaterial *pMaterial = (*iter)->mpMaterial;
				VuMaterialAsset *pMaterialAsset = pMaterial->mpMaterialAsset;

				if ( pMaterialAsset->mbDoesCastShadows )
				{
					for ( int i = 0; i < params.mShadowVolumeCount; i++ )
					{
						if ( params.isVisible(i, (*iter)->mAabb, nodeTransform, mRejectionScaleModifier) )
						{
							// submit draw shadow commands
							ModelInstanceDrawShadowData *pData = static_cast<ModelInstanceDrawShadowData *>(VuGfxSort::IF()->allocateCommandMemory(sizeof(ModelInstanceDrawShadowData)));
							pData->mTransform = nodeTransform*params.mpShadowVolumes[i].mCropMatrix;
							pData->mpPart = *iter;

							VuGfxSort::IF()->setViewportLayer(VuGfxSort::VPL_SHADOW1 + i);
							VuGfxSort::IF()->submitDrawCommand<false>(VuGfxSort::TRANS_OPAQUE, pMaterialAsset->mpGfxSortShadowMaterial, (*iter)->mpChunk->mpGfxSortMesh, ModelInstanceDrawShadowCallback);
						}
					}
				}
			}
		}

		// recurse
		for ( VuGfxSceneNode::Children::iterator iter = pNode->mChildren.begin(); iter != pNode->mChildren.end(); iter++ )
			drawShadowRecursive(*iter, nodeTransform, params);
	}
}

//*****************************************************************************
void VuStaticModelInstance::drawDropShadowRecursive(VuGfxSceneNode *pNode, const VuMatrix &transform, const VuGfxDrawShadowParams &params) const
{
	if ( params.isVisible(pNode->mAabb, transform, mRejectionScaleModifier) )
	{
		VuMatrix nodeTransform = pNode->mTransform*transform;

		if ( pNode->mpMeshInstance )
		{
			VuGfxSceneMesh *pMesh = pNode->mpMeshInstance->mpMesh;

			for ( VuGfxSceneMesh::Parts::iterator iter = pMesh->mParts.begin(); iter != pMesh->mParts.end(); iter++ )
			{
				VuGfxSceneMaterial *pMaterial = (*iter)->mpMaterial;
				VuMaterialAsset *pMaterialAsset = pMaterial->mpMaterialAsset;

				if ( pMaterialAsset->mbDoesCastShadows )
				{
					for ( int i = 0; i < params.mShadowVolumeCount; i++ )
					{
						if ( params.isVisible(i, (*iter)->mAabb, nodeTransform, mRejectionScaleModifier) )
						{
							// submit draw shadow commands
							ModelInstanceDrawShadowData *pData = static_cast<ModelInstanceDrawShadowData *>(VuGfxSort::IF()->allocateCommandMemory(sizeof(ModelInstanceDrawShadowData)));
							pData->mTransform = nodeTransform*params.mpShadowVolumes[i].mCropMatrix;
							pData->mpPart = *iter;

							VuGfxSort::IF()->setViewportLayer(VuGfxSort::VPL_SHADOW1 + i);
							VuGfxSort::IF()->submitDrawCommand<false>(VuGfxSort::TRANS_OPAQUE, pMaterialAsset->mpGfxSortDropShadowMaterial, (*iter)->mpChunk->mpGfxSortMesh, ModelInstanceDrawDropShadowCallback);
						}
					}
				}
			}
		}

		// recurse
		for ( VuGfxSceneNode::Children::iterator iter = pNode->mChildren.begin(); iter != pNode->mChildren.end(); iter++ )
			drawShadowRecursive(*iter, nodeTransform, params);
	}
}

//*****************************************************************************
void VuStaticModelInstance::drawInfoRecursive(VuGfxSceneNode *pNode, const VuMatrix &transform, const VuGfxDrawInfoParams &params) const
{
	if ( params.mCamera.isAabbVisible(pNode->mAabb, transform) )
	{
		VuMatrix nodeTransform = pNode->mTransform*transform;

		if ( pNode->mpMeshInstance )
		{
			// draw instance name
			if ( params.mFlags & VuGfxDrawInfoParams::INSTANCE_NAMES )
				drawName(pNode->mstrName.c_str(), pNode->mAabb, transform, params);

			// draw mesh info
			drawMeshInfo(pNode->mpMeshInstance->mpMesh, nodeTransform, params);
		}

		// recurse
		for ( VuGfxSceneNode::Children::iterator iter = pNode->mChildren.begin(); iter != pNode->mChildren.end(); iter++ )
			drawInfoRecursive(*iter, nodeTransform, params);
	}
}

//*****************************************************************************
template <bool SHADOWING>
bool VuStaticModelInstance::collideRayRecursive(VuGfxSceneNode *pNode, const VuMatrix &transform, const VuVector3 &v0, VuVector3 &v1) const
{
	bool hit = false;

	if ( testAabbRayCollision(pNode->mAabb, transform, v0, v1) )
	{
		VuMatrix nodeTransform = pNode->mTransform*transform;

		if ( pNode->mpMeshInstance )
		{
			// transform inputs to mesh space
			VuMatrix invNodeTransform = nodeTransform;
			invNodeTransform.invert();
			VuVector3 v0Model = invNodeTransform.transform(v0);
			VuVector3 v1Model = invNodeTransform.transform(v1);

			// collide
			if ( collideRayMesh<SHADOWING>(pNode->mpMeshInstance->mpMesh, v0Model, v1Model) )
			{
				hit = true;
				v1 = nodeTransform.transform(v1Model);
			}
		}

		// recurse
		for ( VuGfxSceneNode::Children::iterator iter = pNode->mChildren.begin(); iter != pNode->mChildren.end(); iter++ )
			hit |= collideRayRecursive<SHADOWING>(*iter, nodeTransform, v0, v1);
	}

	return hit;
}

//*****************************************************************************
template <bool SHADOWING>
bool VuStaticModelInstance::collideRayMesh(VuGfxSceneMesh *pMesh, const VuVector3 &v0, VuVector3 &v1) const
{
	bool bCollision = false;

	for ( VuGfxSceneMesh::Parts::iterator iter = pMesh->mParts.begin(); iter != pMesh->mParts.end(); iter++ )
	{
		VuGfxSceneMeshPart *pPart = *iter;

		if ( !SHADOWING || pPart->mpMaterial->mpMaterialAsset->mbDoesCastShadows )
		{
			VuGfxSceneChunk *pChunk = pPart->mpChunk;
			const VUBYTE *pVertexData = static_cast<const VUBYTE *>(pChunk->mpVertexBuffer->getShadowBuffer());
			const VUUINT16 *pIndexData = pChunk->mpIndexBuffer->getShadowBuffer();

			int vertStride = pChunk->mVertexStride;

			if ( testAabbRayCollision(pPart->mAabb, VuMatrix::identity(), v0, v1) )
			{
				const VUUINT16 *pIndex = pIndexData + pPart->mStartIndex;
				for ( int iTri = 0; iTri < pPart->mTriCount; iTri++, pIndex += 3 )
				{
					// assume position is defined as first 3 floats of vertex
					VuPackedVector3 *pPackedVertex0 = (VuPackedVector3 *)(pVertexData + vertStride*pIndex[0]);
					VuPackedVector3 *pPackedVertex1 = (VuPackedVector3 *)(pVertexData + vertStride*pIndex[1]);
					VuPackedVector3 *pPackedVertex2 = (VuPackedVector3 *)(pVertexData + vertStride*pIndex[2]);

					VuVector3 tri0(pPackedVertex0->mX, pPackedVertex0->mY, pPackedVertex0->mZ);
					VuVector3 tri1(pPackedVertex1->mX, pPackedVertex1->mY, pPackedVertex1->mZ);
					VuVector3 tri2(pPackedVertex2->mX, pPackedVertex2->mY, pPackedVertex2->mZ);

					if (SHADOWING)
					{
						// back?
						if (VuDot(v1 - v0, VuCross(tri1 - tri0, tri2 - tri0)) > 0.0f)
							bCollision |= VuMathUtil::triangleLineSegIntersection(tri0, tri1, tri2, v0, v1, v1);
					}
					else
					{
						// front?
						if (VuDot(v1 - v0, VuCross(tri1 - tri0, tri2 - tri0)) < 0.0f)
							bCollision |= VuMathUtil::triangleLineSegIntersection(tri0, tri1, tri2, v0, v1, v1);
					}
				}
			}
		}
	}

	return bCollision;
}

//*****************************************************************************
bool VuStaticModelInstance::testAabbRayCollision(const VuAabb &aabb, const VuMatrix &transform, const VuVector3 &v0, const VuVector3 &v1) const
{
	// transform aabb to world space (applies scaling too)
	VuVector3 vMinWorld = transform.transform(aabb.mMin);
	VuVector3 vMaxWorld = transform.transform(aabb.mMax);
	VuVector3 vCenter = 0.5f*(vMaxWorld + vMinWorld);
	VuVector3 vExtents = 0.5f*(vMaxWorld - vMinWorld);
	
	return VuMathUtil::distPointLineSeg(vCenter, v0, v1) < vExtents.mag();
}

//*****************************************************************************
bool VuStaticModelInstance::collideSphereRecursive(VuGfxSceneNode *pNode, const VuMatrix &transform, const VuVector3 &pos, float radius) const
{
	bool hit = false;

	if ( testAabbSphereCollision(pNode->mAabb, transform, pos, radius) )
	{
		VuMatrix nodeTransform = pNode->mTransform*transform;

		// collide
		if ( pNode->mpMeshInstance )
			hit |= collideSphereMesh(pNode->mpMeshInstance->mpMesh, nodeTransform, pos, radius);

		// recurse
		for ( VuGfxSceneNode::Children::iterator iter = pNode->mChildren.begin(); iter != pNode->mChildren.end(); iter++ )
			hit |= collideSphereRecursive(*iter, nodeTransform, pos, radius);
	}

	return hit;
}

//*****************************************************************************
bool VuStaticModelInstance::testAabbSphereCollision(const VuAabb &aabb, const VuMatrix &transform, const VuVector3 &pos, float radius) const
{
	// transform aabb to world space (applies scaling too)
	VuVector3 vMinWorld = transform.transform(aabb.mMin);
	VuVector3 vMaxWorld = transform.transform(aabb.mMax);
	VuVector3 vCenter = 0.5f*(vMaxWorld + vMinWorld);
	VuVector3 vExtents = 0.5f*(vMaxWorld - vMinWorld);
	
	return VuDist(vCenter, pos) < (vExtents.mag() + radius);
}

//*****************************************************************************
bool VuStaticModelInstance::collideSphereMesh(VuGfxSceneMesh *pMesh, const VuMatrix &transform, const VuVector3 &pos, float radius) const
{
	bool bCollision = false;

	for ( VuGfxSceneMesh::Parts::iterator iter = pMesh->mParts.begin(); iter != pMesh->mParts.end(); iter++ )
	{
		VuGfxSceneMeshPart *pPart = *iter;

		VuGfxSceneChunk *pChunk = pPart->mpChunk;
		const VUBYTE *pVertexData = static_cast<const VUBYTE *>(pChunk->mpVertexBuffer->getShadowBuffer());
		const VUUINT16 *pIndexData = pChunk->mpIndexBuffer->getShadowBuffer();

		int vertStride = pChunk->mVertexStride;

		if ( testAabbSphereCollision(pPart->mAabb, transform, pos, radius) )
		{
			const VUUINT16 *pIndex = pIndexData + pPart->mStartIndex;
			for ( int iTri = 0; iTri < pPart->mTriCount; iTri++, pIndex += 3 )
			{
				// assume position is defined as first 3 floats of vertex
				VuPackedVector3 *pPackedVertex0 = (VuPackedVector3 *)(pVertexData + vertStride*pIndex[0]);
				VuPackedVector3 *pPackedVertex1 = (VuPackedVector3 *)(pVertexData + vertStride*pIndex[1]);
				VuPackedVector3 *pPackedVertex2 = (VuPackedVector3 *)(pVertexData + vertStride*pIndex[2]);

				VuVector3 tri0(pPackedVertex0->mX, pPackedVertex0->mY, pPackedVertex0->mZ);
				VuVector3 tri1(pPackedVertex1->mX, pPackedVertex1->mY, pPackedVertex1->mZ);
				VuVector3 tri2(pPackedVertex2->mX, pPackedVertex2->mY, pPackedVertex2->mZ);

				// transform tri to world space
				VuVector3 triWorld0 = transform.transform(tri0);
				VuVector3 triWorld1 = transform.transform(tri1);
				VuVector3 triWorld2 = transform.transform(tri2);

				if ( VuMathUtil::distPointTriangle(pos, triWorld0, triWorld1, triWorld2) < radius )
					bCollision = true;
			}
		}
	}

	return bCollision;
}

//*****************************************************************************
static void ModelInstanceDrawCallback(void *data)
{
	ModelInstanceDrawData *pData = static_cast<ModelInstanceDrawData *>(data);
	VuGfxSceneMeshPart *pMeshPart = pData->mpPart;
	VuGfxSceneMaterial *pMaterial = pMeshPart->mpMaterial;
	VuMaterialAsset *pMaterialAsset = pMaterial->mpMaterialAsset;

	pMaterialAsset->setModelMatrix(pData->mTransform);
	pMaterialAsset->setColor(pData->mColor);
	pMaterialAsset->setWaterZ(pData->mWaterZ);
	pMaterialAsset->setDynamicLightColor(pData->mDynamicLightColor);
	if ( pMaterialAsset->mbDynamicLighting )
		pMaterialAsset->setDynamicLights(pData->mTransform, pMeshPart->mAabb, pData->mDynamicLightGroupMask);

	if ( pMaterialAsset->mbSceneLighting )
	{
		VuVertexBuffer *pVertexColors = pData->mpVertexColors ? pData->mpVertexColors : VuGfxUtil::IF()->blackVertexColors();
		VuGfx::IF()->drawIndexedPrimitiveVC(VUGFX_PT_TRIANGLELIST, pMeshPart->mMinIndex, pMeshPart->mVertCount, pMeshPart->mStartIndex, pMeshPart->mTriCount, pVertexColors);
	}
	else
	{
		VuGfx::IF()->drawIndexedPrimitive(VUGFX_PT_TRIANGLELIST, pMeshPart->mMinIndex, pMeshPart->mVertCount, pMeshPart->mStartIndex, pMeshPart->mTriCount);
	}
}

//*****************************************************************************
static void ModelInstanceDrawSSAODepthCallback(void *data)
{
	ModelInstanceDrawSSAODepthData *pData = static_cast<ModelInstanceDrawSSAODepthData *>(data);
	VuGfxSceneMeshPart *pMeshPart = pData->mpPart;
	VuGfxSceneMaterial *pMaterial = pMeshPart->mpMaterial;
	VuMaterialAsset *pMaterialAsset = pMaterial->mpMaterialAsset;

	// set constants
	pMaterialAsset->setSSAODepthModelMatrix(pData->mTransform);

	// draw indexed triangles
	VuGfx::IF()->drawIndexedPrimitive(
		VUGFX_PT_TRIANGLELIST,
		pMeshPart->mMinIndex,
		pMeshPart->mVertCount,
		pMeshPart->mStartIndex,
		pMeshPart->mTriCount
	);
}

//*****************************************************************************
static void ModelInstanceDrawShadowCallback(void *data)
{
	ModelInstanceDrawShadowData *pData = static_cast<ModelInstanceDrawShadowData *>(data);
	VuGfxSceneMeshPart *pMeshPart = pData->mpPart;
	VuGfxSceneMaterial *pMaterial = pMeshPart->mpMaterial;
	VuMaterialAsset *pMaterialAsset = pMaterial->mpMaterialAsset;

	// set constants
	bool bAlphaTest = pMaterialAsset->mbAlphaTest;
	pMaterialAsset->setShadowMatrix(pData->mTransform);

	// draw indexed triangles
	VuGfx::IF()->drawIndexedPrimitive(
		VUGFX_PT_TRIANGLELIST,
		pMeshPart->mMinIndex,
		pMeshPart->mVertCount,
		pMeshPart->mStartIndex,
		pMeshPart->mTriCount
	);
}

//*****************************************************************************
static void ModelInstanceDrawDropShadowCallback(void *data)
{
	ModelInstanceDrawShadowData *pData = static_cast<ModelInstanceDrawShadowData *>(data);
	VuGfxSceneMeshPart *pMeshPart = pData->mpPart;
	VuGfxSceneMaterial *pMaterial = pMeshPart->mpMaterial;
	VuMaterialAsset *pMaterialAsset = pMaterial->mpMaterialAsset;

	// set constants
	bool bAlphaTest = pMaterialAsset->mbAlphaTest;
	pMaterialAsset->setDropShadowMatrix(pData->mTransform);

	// draw indexed triangles
	VuGfx::IF()->drawIndexedPrimitive(
		VUGFX_PT_TRIANGLELIST,
		pMeshPart->mMinIndex,
		pMeshPart->mVertCount,
		pMeshPart->mStartIndex,
		pMeshPart->mTriCount
	);
}
