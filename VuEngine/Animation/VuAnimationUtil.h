//*****************************************************************************
//
//  Copyright (c) 2009-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  Animation utility functionality
// 
//*****************************************************************************

#pragma once

class VuAnimationTransform;
class VuMatrix;
class VuAabb;


namespace VuAnimationUtil
{
	void	transformLocalPoseToModelPose(int boneCount, const int *pParentIndices, const VuAnimationTransform *pLocalPose, VuAnimationTransform *pModelPose, VuMatrix *pModelMatrices);
	void	transformModelPoseToLocalPose(int boneCount, const int *pParentIndices, const VuAnimationTransform *pModelPose, VuAnimationTransform *pLocalPose);
	void	blendPoses(int boneCount, const VuAnimationTransform *pPoseA, const VuAnimationTransform *pPoseB, float ratio, VuAnimationTransform *pPose);
	VuAabb	calculateModelPoseAabb(int boneCount, const VuAnimationTransform *pModelPose);
	VuAabb	calculateModelPoseLocalAabb(int boneCount, const VuAnimationTransform *pModelPose);

	void	accumPoseNormal(int boneCount, const VuAnimationTransform *pPose, float weight, VuAnimationTransform *pAccumPose);
	void	accumPoseAdditive(int boneCount, const VuAnimationTransform *pPose, float weight, VuAnimationTransform *pAccumPose);
}
