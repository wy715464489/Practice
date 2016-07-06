//*****************************************************************************
//
//  Copyright (c) 2009-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  Animated Model Instance
// 
//*****************************************************************************

#include "VuAnimatedModelInstance.h"
#include "VuEngine/Animation/VuSkeleton.h"
#include "VuEngine/Animation/VuAnimationTransform.h"
#include "VuEngine/Animation/VuAnimationUtil.h"
#include "VuEngine/Animation/VuAnimatedSkeleton.h"
#include "VuEngine/Gfx/Shaders/VuShadowShader.h"
#include "VuEngine/Gfx/Shaders/VuDropShadowShader.h"
#include "VuEngine/Gfx/Shadow/VuShadowVolume.h"
#include "VuEngine/Gfx/GfxScene/VuGfxAnimatedScene.h"
#include "VuEngine/Gfx/GfxScene/VuGfxSceneMesh.h"
#include "VuEngine/Gfx/GfxScene/VuGfxSceneMeshPart.h"
#include "VuEngine/Gfx/GfxScene/VuGfxSceneChunk.h"
#include "VuEngine/Gfx/GfxScene/VuGfxSceneMaterial.h"
#include "VuEngine/Gfx/Camera/VuCamera.h"
#include "VuEngine/Gfx/VuGfxUtil.h"
#include "VuEngine/Gfx/GfxSort/VuGfxSort.h"
#include "VuEngine/Assets/VuAssetFactory.h"
#include "VuEngine/Assets/VuAnimatedModelAsset.h"
#include "VuEngine/Assets/VuMaterialAsset.h"
#include "VuEngine/Dynamics/VuRagdoll.h"
#include "VuEngine/Memory/VuScratchPad.h"


// constants
#define DEPTH_PASS_OFFSET 0.001f

// static functions
struct ModelInstanceDrawData
{
	VuMatrix					mTransform;
	VuColor						mColor;
	float						mWaterZ;
	VuGfxSceneMeshPart			*mpPart;
	int							mBoneCount;
	VuMatrix					*mpMatrixArray;
	VuColor						mDynamicLightColor;
	VUUINT32					mDynamicLightGroupMask;
	VuMaterialAsset::eFlavor	mFlavor;
};
static void ModelInstanceDrawCallback(void *data);

struct ModelInstanceDrawSSAODepthData
{
	VuMatrix			mTransform;
	VuGfxSceneMeshPart	*mpPart;
	int					mBoneCount;
	VuMatrix			*mpMatrixArray;
};
static void ModelInstanceDrawSSAODepthCallback(void *data);

struct ModelInstanceDrawShadowData
{
	VuMatrix			mTransform;
	VuGfxSceneMeshPart	*mpPart;
	int					mBoneCount;
	VuMatrix			*mpMatrixArray;
};
static void ModelInstanceDrawShadowCallback(void *data);
static void ModelInstanceDrawDropShadowCallback(void *data);


//*****************************************************************************
VuAnimatedModelInstance::VuAnimatedModelInstance() :
	mpModelAsset(VUNULL),
	mpSkeleton(VUNULL),
	mpGfxAnimatedScene(VUNULL),
	mpModelMatrices(VUNULL),
	mCurSimRenderMatrices(0),
	mCurGfxRenderMatrices(1),
	mLocalAabb(VuAabb::zero()),
	mExtraAabbExtent(0),
	mRootTransform(VuMatrix::identity())
{
	mpRenderMatrices[0] = VUNULL;
	mpRenderMatrices[1] = VUNULL;
}

//*****************************************************************************
VuAnimatedModelInstance::~VuAnimatedModelInstance()
{
	reset();
}

//*****************************************************************************
void VuAnimatedModelInstance::reset()
{
	VuGfxSort::IF()->flush();

	if ( mpModelAsset )
	{
		VuAssetFactory::IF()->releaseAsset(mpModelAsset);
		mpModelAsset = VUNULL;
	}

	if ( mpSkeleton )
	{
		mpSkeleton->removeRef();
		mpSkeleton = VUNULL;
	}

	if ( mpGfxAnimatedScene )
	{
		mpGfxAnimatedScene->removeRef();
		mpGfxAnimatedScene = VUNULL;
	}

	delete[] mpModelMatrices;
	delete[] mpRenderMatrices[0];
	delete[] mpRenderMatrices[1];
	mpModelMatrices = VUNULL;
	mpRenderMatrices[0] = VUNULL;
	mpRenderMatrices[1] = VUNULL;

	mLocalAabb.reset();
}

//*****************************************************************************
void VuAnimatedModelInstance::setModelAsset(const std::string &assetName)
{
	if ( mpModelAsset && mpModelAsset->getAssetName() == assetName )
		return;

	reset();

	if ( VuAssetFactory::IF()->doesAssetExist<VuAnimatedModelAsset>(assetName) )
	{
		VuAnimatedModelAsset *pModelAsset = VuAssetFactory::IF()->createAsset<VuAnimatedModelAsset>(assetName);

		setModel(pModelAsset->getSkeleton(), pModelAsset->getGfxAnimatedScene());

		mpModelAsset = pModelAsset;
	}
}

//*****************************************************************************
void VuAnimatedModelInstance::setModel(VuSkeleton *pSkeleton, VuGfxAnimatedScene *pGfxAnimatedScene)
{
	if ( mpSkeleton == pSkeleton && mpGfxAnimatedScene == pGfxAnimatedScene )
		return;

	reset();

	mpSkeleton = pSkeleton;
	mpSkeleton->addRef();

	mpGfxAnimatedScene = pGfxAnimatedScene;
	mpGfxAnimatedScene->addRef();

	mpModelMatrices = new VuMatrix[mpSkeleton->mBoneCount];
	mpRenderMatrices[0] = new VuMatrix[mpSkeleton->mBoneCount];
	mpRenderMatrices[1] = new VuMatrix[mpSkeleton->mBoneCount];

	// initialize matrix arrays
	for ( int i = 0; i < mpSkeleton->mBoneCount; i++ )
	{
		mpSkeleton->mpModelPose[i].toMatrix(mpModelMatrices[i]);
		mpRenderMatrices[0][i].loadIdentity();
		mpRenderMatrices[1][i].loadIdentity();
	}

	// initialize aabb
	mLocalAabb = mpGfxAnimatedScene->mAnimatedInfo.mAabb;

	// calculate extra aabb extent
	{
		VuVector3 vMax = mpGfxAnimatedScene->mAnimatedInfo.mAabb.mMax - mpSkeleton->mSkeletalAabb.mMax;
		VuVector3 vMin = mpGfxAnimatedScene->mAnimatedInfo.mAabb.mMin - mpSkeleton->mSkeletalAabb.mMin;
		VuVector3 v = VuMax(vMax, -vMin);
		mExtraAabbExtent = VuMax(VuMax(v.mX, v.mY), v.mZ);
	}
}

//*****************************************************************************
void VuAnimatedModelInstance::setPose(const VuAnimatedSkeleton *pAnimatedSkeleton)
{
	if ( pAnimatedSkeleton )
	{
		int boneCount = pAnimatedSkeleton->getSkeleton()->mBoneCount;
		VUASSERT(boneCount == mpSkeleton->mBoneCount, "VuAnimatedModelInstance::setLocalPose() bone count mismatch");

		// calculate model space matrices
		const VuAnimationTransform *pLocalPose = pAnimatedSkeleton->getLocalPose();
		VuAnimationTransform *pModelPose = (VuAnimationTransform *)VuScratchPad::get();
		VuAnimationUtil::transformLocalPoseToModelPose(boneCount, mpSkeleton->mpParentIndices, pLocalPose, pModelPose, mpModelMatrices);

		// render matrix is difference between bind pose model matrix and model matrix
		for ( int iBone = 0; iBone < boneCount; iBone++ )
			mpRenderMatrices[mCurSimRenderMatrices][iBone] = mpSkeleton->mpInvModelPoseMatrices[iBone]*mpModelMatrices[iBone];

		// recalculate aabb
		mLocalAabb = pAnimatedSkeleton->getSkeletalLocalAabb();
		mLocalAabb.mMax += VuVector3(mExtraAabbExtent, mExtraAabbExtent, mExtraAabbExtent);
		mLocalAabb.mMin -= VuVector3(mExtraAabbExtent, mExtraAabbExtent, mExtraAabbExtent);

		mRootTransform = mpModelMatrices[0];
	}
	else
	{
		// bind pose
		for ( int i = 0; i < mpSkeleton->mBoneCount; i++ )
		{
			mpSkeleton->mpModelPose[i].toMatrix(mpModelMatrices[i]);
			mpRenderMatrices[mCurSimRenderMatrices][i].loadIdentity();
		}
		mRootTransform.loadIdentity();
	}
}

//*****************************************************************************
void VuAnimatedModelInstance::setPose(const VuMatrix &modelMat, const VuRagdoll *pRagdoll)
{
	int boneCount = mpSkeleton->mBoneCount;
	VUASSERT(boneCount == pRagdoll->getSkeleton()->mBoneCount, "VuAnimatedModelInstance::setPose() bone count mismatch");

	pRagdoll->updateModelMatrices(modelMat, mpModelMatrices);

	// render matrix is difference between bind pose model matrix and model matrix
	for ( int iBone = 0; iBone < boneCount; iBone++ )
		mpRenderMatrices[mCurSimRenderMatrices][iBone] = mpSkeleton->mpInvModelPoseMatrices[iBone]*mpModelMatrices[iBone];

	// recalculate aabb
	mLocalAabb = mpModelMatrices[1].getTrans();
	for ( int iBone = 2; iBone < boneCount; iBone++ )
		mLocalAabb.addPoint(mpModelMatrices[iBone].getTrans());
	mLocalAabb.mMax += VuVector3(mExtraAabbExtent, mExtraAabbExtent, mExtraAabbExtent);
	mLocalAabb.mMin -= VuVector3(mExtraAabbExtent, mExtraAabbExtent, mExtraAabbExtent);

	mRootTransform = mpModelMatrices[0];
}

//*****************************************************************************
void VuAnimatedModelInstance::copyPose(const VuAnimatedModelInstance *pFrom)
{
	int boneCount = mpSkeleton->mBoneCount;
	VUASSERT(boneCount == pFrom->getSkeleton()->mBoneCount, "VuAnimatedModelInstance::copyPose() bone count mismatch");

	// copy model/render matrices
	VU_MEMCPY(mpModelMatrices, boneCount*sizeof(VuMatrix), pFrom->mpModelMatrices, boneCount*sizeof(VuMatrix));
	VU_MEMCPY(mpRenderMatrices[mCurSimRenderMatrices], boneCount*sizeof(VuMatrix), pFrom->mpRenderMatrices[pFrom->mCurSimRenderMatrices], boneCount*sizeof(VuMatrix));

	// handle aabb
	mLocalAabb = pFrom->mLocalAabb;

	VuVector3 extraAabbExtent(mExtraAabbExtent, mExtraAabbExtent, mExtraAabbExtent);
	VuVector3 otherExtraAabbExtent(pFrom->mExtraAabbExtent, pFrom->mExtraAabbExtent, pFrom->mExtraAabbExtent);

	mLocalAabb.mMax -= otherExtraAabbExtent;
	mLocalAabb.mMin += otherExtraAabbExtent;

	mLocalAabb.mMax += extraAabbExtent;
	mLocalAabb.mMin -= extraAabbExtent;

	// root transform
	mRootTransform = mpModelMatrices[0];
}

//*****************************************************************************
void VuAnimatedModelInstance::finalizePose()
{
	// swap render matrix
	mCurSimRenderMatrices = !mCurSimRenderMatrices;
	mCurGfxRenderMatrices = !mCurGfxRenderMatrices;
}

//*****************************************************************************
void VuAnimatedModelInstance::draw(const VuMatrix &modelMat, const VuGfxDrawParams &params) const
{
	if ( mpGfxAnimatedScene )
	{
		// draw each mesh
		for ( VuGfxScene::Meshes::iterator itMesh = mpGfxAnimatedScene->mMeshes.begin(); itMesh != mpGfxAnimatedScene->mMeshes.end(); itMesh++ )
		{
			VuGfxSceneMesh *pMesh = *itMesh;

			// calculate depth
			VuVector3 vPos = modelMat.transform(pMesh->getAabb().getCenter());
			float depth = (vPos - params.mCamera.getEyePosition()).mag()/params.mCamera.getFarPlane();
			depth = VuMin(depth, 1.0f);

			for ( VuGfxSceneMesh::Parts::iterator itPart = pMesh->mParts.begin(); itPart != pMesh->mParts.end(); itPart++ )
			{
				VuGfxSceneMeshPart *pPart = *itPart;
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
					pData->mTransform = modelMat;
					pData->mColor = mColor;
					pData->mWaterZ = mWaterZ;
					pData->mpPart = pPart;
					pData->mBoneCount = mpSkeleton->mBoneCount;
					pData->mpMatrixArray = mpRenderMatrices[mCurGfxRenderMatrices];
					pData->mDynamicLightColor = mDynamicLightColor;
					pData->mDynamicLightGroupMask = mDynamicLightGroupMask;
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
					pData->mTransform = modelMat;
					pData->mColor = mColor;
					pData->mWaterZ = mWaterZ;
					pData->mpPart = pPart;
					pData->mBoneCount = mpSkeleton->mBoneCount;
					pData->mpMatrixArray = mpRenderMatrices[mCurGfxRenderMatrices];
					pData->mDynamicLightColor = mDynamicLightColor;
					pData->mDynamicLightGroupMask = mDynamicLightGroupMask;
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
						pData->mTransform = modelMat;
						pData->mpPart = pPart;
						pData->mBoneCount = mpSkeleton->mBoneCount;
						pData->mpMatrixArray = mpRenderMatrices[mCurGfxRenderMatrices];

						VuGfxSort::IF()->setViewportLayer(VuGfxSort::VPL_SSAO);
						VuGfxSort::IF()->submitDrawCommand<false>(VuGfxSort::TRANS_OPAQUE, pMaterialAsset->mpGfxSortSSAODepthMaterial, pChunk->mpGfxSortMesh, ModelInstanceDrawSSAODepthCallback, depth);

						VuGfxSort::IF()->setViewportLayer(prevVPL);
					}
				}
			}
		}
	}
}

//*****************************************************************************
void VuAnimatedModelInstance::drawShadow(const VuMatrix &modelMat, const VuGfxDrawShadowParams &params) const
{
	if ( mpGfxAnimatedScene )
	{
		// draw each mesh
		for ( VuGfxScene::Meshes::iterator iter = mpGfxAnimatedScene->mMeshes.begin(); iter != mpGfxAnimatedScene->mMeshes.end(); iter++ )
		{
			VuGfxSceneMesh *pMesh = *iter;

			// calculate depth
			VuVector3 vPos = modelMat.transform(pMesh->getAabb().getCenter());
			float depth = (vPos - params.mCamera.getEyePosition()).mag()/params.mCamera.getFarPlane();
			depth = VuMin(depth, 1.0f);

			for ( VuGfxSceneMesh::Parts::iterator iter = pMesh->mParts.begin(); iter != pMesh->mParts.end(); iter++ )
			{
				VuGfxSceneMaterial *pMaterial = (*iter)->mpMaterial;
				VuMaterialAsset *pMaterialAsset = pMaterial->mpMaterialAsset;

				if ( pMaterialAsset->mbDoesCastShadows )
				{
					for ( int i = 0; i < params.mShadowVolumeCount; i++ )
					{
						// submit draw shadow commands
						ModelInstanceDrawShadowData *pData = static_cast<ModelInstanceDrawShadowData *>(VuGfxSort::IF()->allocateCommandMemory(sizeof(ModelInstanceDrawShadowData)));
						pData->mTransform = modelMat*params.mpShadowVolumes[i].mCropMatrix;
						pData->mpPart = *iter;
						pData->mBoneCount = mpSkeleton->mBoneCount;
						pData->mpMatrixArray = mpRenderMatrices[mCurGfxRenderMatrices];

						VuGfxSort::IF()->setViewportLayer(VuGfxSort::VPL_SHADOW1 + i);
						VuGfxSort::IF()->submitDrawCommand<false>(VuGfxSort::TRANS_OPAQUE, pMaterialAsset->mpGfxSortShadowMaterial, (*iter)->mpChunk->mpGfxSortMesh, ModelInstanceDrawShadowCallback);
					}
				}
			}
		}
	}
}

//*****************************************************************************
void VuAnimatedModelInstance::drawDropShadow(const VuMatrix &modelMat, const VuGfxDrawShadowParams &params) const
{
	if ( mpGfxAnimatedScene )
	{
		// draw each mesh
		for ( VuGfxScene::Meshes::iterator iter = mpGfxAnimatedScene->mMeshes.begin(); iter != mpGfxAnimatedScene->mMeshes.end(); iter++ )
		{
			VuGfxSceneMesh *pMesh = *iter;

			// calculate depth
			VuVector3 vPos = modelMat.transform(pMesh->getAabb().getCenter());
			float depth = (vPos - params.mCamera.getEyePosition()).mag()/params.mCamera.getFarPlane();
			depth = VuMin(depth, 1.0f);

			for ( VuGfxSceneMesh::Parts::iterator iter = pMesh->mParts.begin(); iter != pMesh->mParts.end(); iter++ )
			{
				VuGfxSceneMaterial *pMaterial = (*iter)->mpMaterial;
				VuMaterialAsset *pMaterialAsset = pMaterial->mpMaterialAsset;

				if ( pMaterialAsset->mbDoesCastShadows )
				{
					for ( int i = 0; i < params.mShadowVolumeCount; i++ )
					{
						// submit draw shadow commands
						ModelInstanceDrawShadowData *pData = static_cast<ModelInstanceDrawShadowData *>(VuGfxSort::IF()->allocateCommandMemory(sizeof(ModelInstanceDrawShadowData)));
						pData->mTransform = modelMat*params.mpShadowVolumes[i].mCropMatrix;
						pData->mpPart = *iter;
						pData->mBoneCount = mpSkeleton->mBoneCount;
						pData->mpMatrixArray = mpRenderMatrices[mCurGfxRenderMatrices];

						VuGfxSort::IF()->setViewportLayer(VuGfxSort::VPL_SHADOW1 + i);
						VuGfxSort::IF()->submitDrawCommand<false>(VuGfxSort::TRANS_OPAQUE, pMaterialAsset->mpGfxSortDropShadowMaterial, (*iter)->mpChunk->mpGfxSortMesh, ModelInstanceDrawDropShadowCallback);
					}
				}
			}
		}
	}
}

//*****************************************************************************
void VuAnimatedModelInstance::drawPrefetch() const
{
	if ( mpGfxAnimatedScene )
	{
		// draw each mesh
		for ( VuGfxScene::Meshes::iterator itMesh = mpGfxAnimatedScene->mMeshes.begin(); itMesh != mpGfxAnimatedScene->mMeshes.end(); itMesh++ )
		{
			VuGfxSceneMesh *pMesh = *itMesh;

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
				pData->mBoneCount = mpSkeleton->mBoneCount;
				pData->mpMatrixArray = mpRenderMatrices[mCurGfxRenderMatrices];
				pData->mDynamicLightColor = mDynamicLightColor;
				pData->mDynamicLightGroupMask = mDynamicLightGroupMask;
				pData->mFlavor = VuMaterialAsset::FLV_OPAQUE;

				VuGfxSort::IF()->submitDrawCommand<false>(VuGfxSort::TRANS_OPAQUE, pGfxSortMaterial, pChunk->mpGfxSortMesh, ModelInstanceDrawCallback, 0.5f);
			}
		}
	}
}

//*****************************************************************************
void VuAnimatedModelInstance::drawInfo(const VuMatrix &modelMat, const VuGfxDrawInfoParams &params) const
{
	// mesh info
	if ( mpGfxAnimatedScene && params.mCamera.isAabbVisible(mpGfxAnimatedScene->mAnimatedInfo.mAabb, modelMat) )
	{
		// mesh info
		for ( VuGfxScene::Meshes::iterator iter = mpGfxAnimatedScene->mMeshes.begin(); iter != mpGfxAnimatedScene->mMeshes.end(); iter++ )
			if ( params.mCamera.isAabbVisible((*iter)->getAabb(), modelMat) )
				drawMeshInfo(*iter, modelMat, params);

		drawBoneInfo(modelMat, params);

		// scene info
		char str[256];
		VU_SPRINTF(str, sizeof(str),
			"%5d Stored Meshes\n"
			"%5d Stored Mesh Parts\n"
			"%5d Stored Verts\n"
			"%5d Stored Tris\n"
			"%5d Materials\n",
			mpGfxAnimatedScene->mInfo.mNumMeshes,
			mpGfxAnimatedScene->mInfo.mNumMeshParts,
			mpGfxAnimatedScene->mInfo.mNumVerts,
			mpGfxAnimatedScene->mInfo.mNumTris,
			mpGfxAnimatedScene->mInfo.mNumMaterials
		);
		drawSceneInfo(modelMat, params, str);
	}
}

//*****************************************************************************
void VuAnimatedModelInstance::drawBoneInfo(const VuMatrix &modelMat, const VuGfxDrawInfoParams &params) const
{
	if ( params.mFlags & (VuGfxDrawInfoParams::BONES|VuGfxDrawInfoParams::BONE_NAMES) )
	{
		for ( int iBone = 0; iBone< mpSkeleton->mBoneCount; iBone++ )
		{
			const VuSkeleton::VuBone &bone = mpSkeleton->mpBones[iBone];
			int parentIndex = mpSkeleton->mpParentIndices[iBone];
			const VuMatrix &boneModelMat = mpModelMatrices[iBone];
			VuMatrix boneWorldMat = boneModelMat*modelMat;

			if ( params.mFlags & VuGfxDrawInfoParams::BONES )
			{
				// draw axes
				VuGfxUtil::IF()->drawLine3d(VuColor(255, 0, 0), boneWorldMat.getTrans(), boneWorldMat.getTrans() + params.mBoneSize*boneWorldMat.getAxisX(), params.mCamera.getViewProjMatrix());
				VuGfxUtil::IF()->drawLine3d(VuColor(0, 255, 0), boneWorldMat.getTrans(), boneWorldMat.getTrans() + params.mBoneSize*boneWorldMat.getAxisY(), params.mCamera.getViewProjMatrix());
				VuGfxUtil::IF()->drawLine3d(VuColor(0, 0, 255), boneWorldMat.getTrans(), boneWorldMat.getTrans() + params.mBoneSize*boneWorldMat.getAxisZ(), params.mCamera.getViewProjMatrix());

				// draw line to parent
				if ( parentIndex >= 0 )
				{
					const VuMatrix &parentBoneModelMat = mpModelMatrices[parentIndex];
					VuMatrix parentBoneWorldMat = parentBoneModelMat*modelMat;

					VuGfxUtil::IF()->drawLine3d(params.mDevLineColor, boneWorldMat.getTrans(), parentBoneWorldMat.getTrans(), params.mCamera.getViewProjMatrix());
				}
			}

			if ( params.mFlags & VuGfxDrawInfoParams::BONE_NAMES )
				drawName(bone.mName, VuAabb::zero(), boneWorldMat, params);
		}
	}
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
	pMaterialAsset->setMatrixArray(pData->mpMatrixArray, pData->mBoneCount);
	pMaterialAsset->setDynamicLightColor(pData->mDynamicLightColor);
	if (pMaterialAsset->mbDynamicLighting)
		pMaterialAsset->setDynamicLights(pData->mTransform, pMeshPart->mAabb, pData->mDynamicLightGroupMask);

	VuGfx::IF()->drawIndexedPrimitive(
		VUGFX_PT_TRIANGLELIST,
		pMeshPart->mMinIndex,
		pMeshPart->mVertCount,
		pMeshPart->mStartIndex,
		pMeshPart->mTriCount
	);
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
	pMaterialAsset->setSSAODepthMatrixArray(pData->mpMatrixArray, pData->mBoneCount);

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
	pMaterialAsset->setShadowMatrixArray(pData->mpMatrixArray, pData->mBoneCount);

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
	pMaterialAsset->setDropShadowMatrixArray(pData->mpMatrixArray, pData->mBoneCount);

	// draw indexed triangles
	VuGfx::IF()->drawIndexedPrimitive(
		VUGFX_PT_TRIANGLELIST,
		pMeshPart->mMinIndex,
		pMeshPart->mVertCount,
		pMeshPart->mStartIndex,
		pMeshPart->mTriCount
	);
}
