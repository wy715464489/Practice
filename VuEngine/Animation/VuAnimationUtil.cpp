//*****************************************************************************
//
//  Copyright (c) 2009-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  Animation utility functionality
// 
//*****************************************************************************

#include "VuAnimationUtil.h"
#include "VuAnimationTransform.h"
#include "VuSkeleton.h"
#include "VuEngine/Math/VuMatrix.h"
#include "VuEngine/Memory/VuScratchPad.h"


//*****************************************************************************
void VuAnimationUtil::transformLocalPoseToModelPose(int boneCount, const int *pParentIndices, const VuAnimationTransform *pLocalPose, VuAnimationTransform *pModelPose, VuMatrix *pModelMatrices)
{
	// safety first
	if ( boneCount <= 0 )
		return;

	// first bone is root
	pModelPose[0] = pLocalPose[0];
	pModelPose[0].toMatrix(pModelMatrices[0]);
	for ( int iBone = 1; iBone < boneCount; iBone++ )
	{
		int parentIndex = pParentIndices[iBone];

		// handle translation
		pModelPose[iBone].mTranslation = pModelMatrices[parentIndex].transform(pLocalPose[iBone].mTranslation);

		// concatenate rotation
		pModelPose[iBone].mRotation = pModelPose[parentIndex].mRotation*pLocalPose[iBone].mRotation;

		// copy scale
		pModelPose[iBone].mScale = pLocalPose[iBone].mScale;

		// calculate model-space matrix
		pModelPose[iBone].toMatrix(pModelMatrices[iBone]);
	}
}

//*****************************************************************************
void VuAnimationUtil::transformModelPoseToLocalPose(int boneCount, const int *pParentIndices, const VuAnimationTransform *pModelPose, VuAnimationTransform *pLocalPose)
{
	// safety first
	if ( boneCount <= 0 )
		return;

	// first bone is root
	pLocalPose[0] = pModelPose[0];
	for ( int iBone = 1; iBone < boneCount; iBone++ )
	{
		int parentIndex = pParentIndices[iBone];

		// handle translation
		VuMatrix invMatParent;
		pModelPose[parentIndex].toMatrix(invMatParent);
		invMatParent.invert();
		pLocalPose[iBone].mTranslation = invMatParent.transform(pModelPose[iBone].mTranslation);

		// handle rotation
		VuQuaternion invParentRotation = pModelPose[parentIndex].mRotation;
		invParentRotation.invert();
		pLocalPose[iBone].mRotation = invParentRotation*pModelPose[iBone].mRotation;

		// copy scale
		pLocalPose[iBone].mScale = pModelPose[iBone].mScale;
	}
}

//*****************************************************************************
void VuAnimationUtil::blendPoses(int boneCount, const VuAnimationTransform *pPoseA, const VuAnimationTransform *pPoseB, float ratio, VuAnimationTransform *pPose)
{
	float weightA = 1.0f - ratio;
	float weightB = ratio;

	// prepare for blending
	memset(pPose, 0, boneCount*sizeof(VuAnimationTransform));

	for ( int iBone = 0; iBone < boneCount; iBone++ )
	{
		pPose->blendAddMul(*pPoseA, weightA);
		pPose->blendAddMul(*pPoseB, weightB);
		pPose->normalize();

		pPoseA++;
		pPoseB++;
		pPose++;
	}
}

//*****************************************************************************
VuAabb VuAnimationUtil::calculateModelPoseAabb(int boneCount, const VuAnimationTransform *pModelPose)
{
	if ( boneCount <= 0 )
		return VuAabb::zero();

	VuAabb aabb;
	for ( int i = 0; i < boneCount; i++ )
		aabb.addPoint(pModelPose[i].mTranslation);

	return aabb;
}

//*****************************************************************************
VuAabb VuAnimationUtil::calculateModelPoseLocalAabb(int boneCount, const VuAnimationTransform *pModelPose)
{
	if ( boneCount <= 0 )
		return VuAabb::zero();

	VuMatrix invRootMat;
	pModelPose[0].toMatrix(invRootMat);
	invRootMat.invert();

	VuAabb aabb;
	for ( int i = 0; i < boneCount; i++ )
		aabb.addPoint(invRootMat.transform(pModelPose[i].mTranslation));

	return aabb;
}

//*****************************************************************************
void VuAnimationUtil::accumPoseNormal(int boneCount, const VuAnimationTransform *pPose, float weight, VuAnimationTransform *pAccumPose)
{
	for ( int iBone = 0; iBone < boneCount; iBone++ )
	{
		pAccumPose->mTranslation += weight*pPose->mTranslation;
		pAccumPose->mScale += weight*pPose->mScale;

		float signedWeight = VuSelect(VuDot(pAccumPose->mRotation.mVec, pPose->mRotation.mVec), weight, -weight);
		pAccumPose->mRotation.mVec += signedWeight*pPose->mRotation.mVec;

		pPose++;
		pAccumPose++;
	}
}

//*****************************************************************************
void VuAnimationUtil::accumPoseAdditive(int boneCount, const VuAnimationTransform *pPose, float weight, VuAnimationTransform *pAccumPose)
{
	VuAnimationTransform product;
	for ( int iBone = 0; iBone < boneCount; iBone++ )
	{
		pAccumPose->mTranslation = (1.0f - weight)*pAccumPose->mTranslation + weight*(pPose->mTranslation + pAccumPose->mTranslation);
		pAccumPose->mRotation = (1.0f - weight)*pAccumPose->mRotation + weight*(pPose->mRotation*pAccumPose->mRotation);
		pAccumPose->mScale = (1.0f - weight)*pAccumPose->mScale + weight*(pPose->mScale*pAccumPose->mScale);

		pAccumPose->mRotation.normalize();
		
		pPose++;
		pAccumPose++;
	}
}
