//*****************************************************************************
//
//  Copyright (c) 2009-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  Animated Skeleton class
// 
//*****************************************************************************

#include "VuAnimatedSkeleton.h"
#include "VuSkeleton.h"
#include "VuAnimation.h"
#include "VuAnimationTransform.h"
#include "VuAnimationControl.h"
#include "VuAnimationUtil.h"
#include "VuEngine/HAL/Gfx/VuGfxTypes.h"
#include "VuEngine/Util/VuColor.h"
#include "VuEngine/Dev/VuDev.h"
#include "VuEngine/Dev/VuDevProfile.h"


//*****************************************************************************
VuAnimatedSkeleton::VuAnimatedSkeleton(VuSkeleton *pSkeleton):
	mpSkeleton(pSkeleton),
	mSkeletalLocalAabb(VuAabb::zero())
{
	mpSkeleton->addRef();

	mpLocalPose = new VuAnimationTransform[pSkeleton->mBoneCount];

	// start with bind pose
	int copySize = mpSkeleton->mBoneCount*sizeof(VuAnimationTransform);
	VU_MEMCPY(mpLocalPose, copySize, mpSkeleton->mpLocalPose, copySize);
}

//*****************************************************************************
VuAnimatedSkeleton::~VuAnimatedSkeleton()
{
	clearAnimationControls();

	mpSkeleton->removeRef();

	delete[] mpLocalPose;
}

//*****************************************************************************
void VuAnimatedSkeleton::advance(float fdt)
{
	for ( int i = 0; i < mAnimationControls.size(); i++ )
		mAnimationControls[i]->advance(fdt);
}

//*****************************************************************************
void VuAnimatedSkeleton::build()
{
	VU_PROFILE_SIM("AnimBuild");

	// use scratch pad
	VuAnimationTransform tempPose[VUGFX_MAX_BONE_COUNT];

	int boneCount = mpSkeleton->mBoneCount;

	// calculate total weight
	float totalWeight = 0;
	for ( int i = 0; i < mBlendAnimationControls.size(); i++ )
		totalWeight += mBlendAnimationControls[i]->getWeight();

	if ( totalWeight > FLT_EPSILON )
	{
		// prepare for blending
		memset(mpLocalPose, 0, boneCount*sizeof(VuAnimationTransform));
		mSkeletalLocalAabb.reset();

		// blend in all animations
		for ( int iAnim = 0; iAnim < mBlendAnimationControls.size(); iAnim++ )
		{
			VuAnimationControl *pAC = mBlendAnimationControls[iAnim];
			if ( pAC->getWeight() > 0 )
			{
				// sample animation into scratch pad
				pAC->getAnimation()->sample(pAC->getLocalTime(), tempPose);

				// do normal blending
				VuAnimationUtil::accumPoseNormal(boneCount, tempPose, pAC->getWeight(), mpLocalPose);

				// update aabb
				mSkeletalLocalAabb.addAabb(pAC->getAnimation()->getSkeletalLocalAabb());
			}
		}

		// renormalize
		for ( int iBone = 0; iBone < boneCount; iBone++ )
			mpLocalPose[iBone].normalize(totalWeight);
	}
	else
	{
		// use bind pose
		int copySize = mpSkeleton->mBoneCount*sizeof(VuAnimationTransform);
		VU_MEMCPY(mpLocalPose, copySize, mpSkeleton->mpLocalPose, copySize);
		mSkeletalLocalAabb = mpSkeleton->mSkeletalAabb;
	}

	// handle additive animation
	if ( mAdditiveAnimationControls.size() )
	{
		for ( int iAnim = 0; iAnim < mAdditiveAnimationControls.size(); iAnim++ )
		{
			VuAnimationControl *pAC = mAdditiveAnimationControls[iAnim];
			if ( pAC->getWeight() > 0 )
			{
				// sample animation into scratch pad
				pAC->getAnimation()->sample(pAC->getLocalTime(), tempPose);

				// do additive blending
				VuAnimationUtil::accumPoseAdditive(boneCount, tempPose, pAC->getWeight(), mpLocalPose);

				// update aabb
				mSkeletalLocalAabb.addAabb(pAC->getAnimation()->getSkeletalLocalAabb());
			}
		}
	}
}

//*****************************************************************************
void VuAnimatedSkeleton::addAnimationControl(VuAnimationControl *pAnimationControl)
{
	pAnimationControl->addRef();
	mAnimationControls.push_back(pAnimationControl);
	if ( pAnimationControl->getAnimation()->isAdditive() )
		mAdditiveAnimationControls.push_back(pAnimationControl);
	else
		mBlendAnimationControls.push_back(pAnimationControl);
}

//*****************************************************************************
void VuAnimatedSkeleton::removeAnimationControl(VuAnimationControl *pAnimationControl)
{
	mAnimationControls.remove(pAnimationControl);
	mBlendAnimationControls.remove(pAnimationControl);
	mAdditiveAnimationControls.remove(pAnimationControl);
	pAnimationControl->removeRef();
}

//*****************************************************************************
void VuAnimatedSkeleton::clearAnimationControls()
{
	for ( int i = 0; i < mAnimationControls.size(); i++ )
		mAnimationControls[i]->removeRef();
	mAnimationControls.clear();
	mBlendAnimationControls.clear();
	mAdditiveAnimationControls.clear();
}

//*****************************************************************************
void VuAnimatedSkeleton::clearBlendAnimationControls()
{
	while ( mBlendAnimationControls.size() )
		removeAnimationControl(mBlendAnimationControls[0]);
}

//*****************************************************************************
void VuAnimatedSkeleton::clearAdditiveAnimationControls()
{
	while ( mAdditiveAnimationControls.size() )
		removeAnimationControl(mAdditiveAnimationControls[0]);
}

//*****************************************************************************
void VuAnimatedSkeleton::drawInfo(const VuVector3 &pos) const
{
	char strInfo[256] = "";

	for ( int i = 0; i < mAnimationControls.size(); i++ )
	{
		VuAnimationControl *pAnimControl = mAnimationControls[i];

		VU_SPRINTF(strInfo, sizeof(strInfo), "slot %d: time = %.2f, weight = %.2f\n", i, pAnimControl->getLocalTime(), pAnimControl->getWeight());
	}

	VuDev::IF()->printf(pos, VUGFX_TEXT_DRAW_HCENTER|VUGFX_TEXT_DRAW_VCENTER, VuColor(255,255,255), strInfo);
}
