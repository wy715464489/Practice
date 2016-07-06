//*****************************************************************************
//
//  Copyright (c) 2009-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  Animation class
// 
//*****************************************************************************

#include "VuAnimation.h"
#include "VuSkeleton.h"
#include "VuAnimationTransform.h"
#include "VuAnimationUtil.h"
#include "VuEngine/Json/VuJsonContainer.h"
#include "VuEngine/Util/VuDataUtil.h"
#include "VuEngine/Util/VuBinaryDataUtil.h"
#include "VuEngine/Util/VuEndianUtil.h"
#include "VuEngine/Math/VuMatrix.h"


// constants
#define ANIMATION_FRAMES_PER_SECOND 30.0f


//*****************************************************************************
VuAnimation::VuAnimation():
	mBoneCount(0),
	mFrameCount(0),
	mpData(VUNULL),
	mIsAdditive(false)
{
}

//*****************************************************************************
VuAnimation::~VuAnimation()
{
	delete[] mpData;
}

//*****************************************************************************
bool VuAnimation::load(const VuJsonContainer &data, bool bAdditive)
{
	// load skeleton
	VuSkeleton *pSkeleton = new VuSkeleton;
	if ( !pSkeleton->load(data["Skeleton"]) )
	{
		pSkeleton->removeRef();
		return false;
	}

	mBoneCount = pSkeleton->mBoneCount;
	data["FrameCount"].getValue(mFrameCount);

	// allocate
	mpData = new VuAnimationTransform[mFrameCount*mBoneCount];

	// load frames
	VuArray<VUBYTE> bytes;
	bytes.reserve(mBoneCount*sizeof(VuAnimationTransform));
	for ( int iFrame = 0; iFrame < mFrameCount; iFrame++ )
	{
		VuAnimationTransform *pPose = &mpData[iFrame*mBoneCount];

		VuDataUtil::getValue(data["Frames"][iFrame], bytes);
		VUASSERT(bytes.size() == mBoneCount*sizeof(VuAnimationTransform), "VuAnimation::load() byte size mismatch");
		VU_MEMCPY(pPose, mBoneCount*sizeof(VuAnimationTransform), &bytes.begin(), bytes.size());
	}

	// fix endianness
	#if VU_BIG_ENDIAN
		endianSwap();
	#endif

	// build skeletal local aabb using model-space frames
	mSkeletalLocalAabb.reset();
	for ( int iFrame = 0; iFrame < mFrameCount; iFrame++ )
	{
		VuAnimationTransform *pPose = &mpData[iFrame*mBoneCount];
		VuAabb frameAabb = VuAnimationUtil::calculateModelPoseLocalAabb(mBoneCount, pPose);
		mSkeletalLocalAabb.addAabb(frameAabb);
	}

	// transform data from model space to local space
	for ( int iFrame = 0; iFrame < mFrameCount; iFrame++ )
	{
		VuAnimationTransform *pPose = &mpData[iFrame*mBoneCount];

		VuAnimationTransform tempPose[VUGFX_MAX_BONE_COUNT];
		VuAnimationUtil::transformModelPoseToLocalPose(mBoneCount, pSkeleton->mpParentIndices, pPose, tempPose);
		VU_MEMCPY(pPose, mBoneCount*sizeof(VuAnimationTransform), tempPose, mBoneCount*sizeof(VuAnimationTransform));
	}

	// handle additive animation data
	mIsAdditive = bAdditive;
	if ( bAdditive )
	{
		for ( int iFrame = 0; iFrame < mFrameCount; iFrame++ )
		{
			VuAnimationTransform *pPose = &mpData[iFrame*mBoneCount];
		
			// additive animation data is difference between bind pose local pose and local pose
			for ( int iBone = 0; iBone < mBoneCount; iBone++ )
			{
				VuAnimationTransform bind = pSkeleton->mpLocalPose[iBone];
				VuAnimationTransform local = pPose[iBone];

				pPose[iBone].mTranslation = local.mTranslation - bind.mTranslation;
				pPose[iBone].mRotation = local.mRotation/bind.mRotation;
				pPose[iBone].mScale = local.mScale/bind.mScale;
			}
		}
	}

	// clean up
	pSkeleton->removeRef();

	buildDerivedData();

	return true;
}

//*****************************************************************************
void VuAnimation::load(VuBinaryDataReader &reader)
{
	reader.readValue(mBoneCount);
	reader.readValue(mFrameCount);
	mpData = new VuAnimationTransform[mFrameCount*mBoneCount];
	for ( int i = 0; i < mFrameCount*mBoneCount; i++ )
		mpData[i].deserialize(reader);
	reader.readValue(mSkeletalLocalAabb);
	reader.readValue(mIsAdditive);

	buildDerivedData();
}

//*****************************************************************************
void VuAnimation::save(VuBinaryDataWriter &writer)
{
	writer.writeValue(mBoneCount);
	writer.writeValue(mFrameCount);
	for ( int i = 0; i < mFrameCount*mBoneCount; i++ )
		mpData[i].serialize(writer);
	writer.writeValue(mSkeletalLocalAabb);
	writer.writeValue(mIsAdditive);
}

//*****************************************************************************
void VuAnimation::sample(float localTime, VuAnimationTransform *pPose) const
{
	float frame = mFrameCount*localTime/mTotalTime;
	int frameA = VuFloorInt(frame);
	int frameB = (frameA + 1);
	float blendFactor = frame - frameA;

	frameA %= mFrameCount;
	frameB %= mFrameCount;

	VuAnimationTransform *pPoseA = &mpData[frameA*mBoneCount];
	VuAnimationTransform *pPoseB = &mpData[frameB*mBoneCount];

	VuAnimationUtil::blendPoses(mBoneCount, pPoseA, pPoseB, blendFactor, pPose);
}

//*****************************************************************************
void VuAnimation::endianSwap()
{
	for ( int i = 0; i < mFrameCount*mBoneCount; i++ )
	{
		VuAnimationTransform &transform = mpData[i];

		VuEndianUtil::swapInPlace(transform.mTranslation.mX);
		VuEndianUtil::swapInPlace(transform.mTranslation.mY);
		VuEndianUtil::swapInPlace(transform.mTranslation.mZ);
		VuEndianUtil::swapInPlace(transform.mRotation.mVec.mX);
		VuEndianUtil::swapInPlace(transform.mRotation.mVec.mY);
		VuEndianUtil::swapInPlace(transform.mRotation.mVec.mZ);
		VuEndianUtil::swapInPlace(transform.mRotation.mVec.mW);
		VuEndianUtil::swapInPlace(transform.mScale.mX);
		VuEndianUtil::swapInPlace(transform.mScale.mY);
		VuEndianUtil::swapInPlace(transform.mScale.mZ);
	}
}

//*****************************************************************************
void VuAnimation::buildDerivedData()
{
	// calculate end and total time
	mEndTime = (mFrameCount - 1)/ANIMATION_FRAMES_PER_SECOND;
	mTotalTime = mFrameCount/ANIMATION_FRAMES_PER_SECOND;
}