//*****************************************************************************
//
//  Copyright (c) 2009-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  Skeleton class
// 
//*****************************************************************************

#include "VuSkeleton.h"
#include "VuAnimationTransform.h"
#include "VuAnimationUtil.h"
#include "VuEngine/Math/VuMatrix.h"
#include "VuEngine/Json/VuJsonContainer.h"
#include "VuEngine/Util/VuDataUtil.h"
#include "VuEngine/Util/VuBinaryDataUtil.h"


//*****************************************************************************
VuSkeleton::VuSkeleton():
	mBoneCount(0),
	mpBones(VUNULL),
	mpParentIndices(VUNULL),
	mpModelPose(VUNULL),
	mpLocalPose(VUNULL),
	mpInvModelPoseMatrices(VUNULL)
{
}

//*****************************************************************************
VuSkeleton::~VuSkeleton()
{
	delete[] mpBones;
	delete[] mpParentIndices;
	delete[] mpModelPose;
	delete[] mpLocalPose;
	delete[] mpInvModelPoseMatrices;
}

//*****************************************************************************
bool VuSkeleton::load(const VuJsonContainer &data)
{
	mBoneCount = data.size();

	allocateData();

	// load bones (model pose)
	for ( int iBone = 0; iBone < mBoneCount; iBone++ )
	{
		const VuJsonContainer &boneData = data[iBone];
		VuBone &bone = mpBones[iBone];

		// name
		memset(bone.mName, 0, sizeof(bone.mName));
		const char *name = boneData["Name"].asCString();
		if ( strlen(name) + 1 > sizeof(bone.mName) )
			return VUERROR("Bone name '%s' exceeds max size of %d characters.", name, sizeof(bone.mName) - 1);
		VU_STRCPY(bone.mName, sizeof(bone.mName), name);

		// parent index
		mpParentIndices[iBone] = -1;
		boneData["ParentIndex"].getValue(mpParentIndices[iBone]);

		// get model transform
		VuDataUtil::getValue(boneData["Transform"], mpModelPose[iBone]);
	}

	mSkeletalAabb = VuAnimationUtil::calculateModelPoseAabb(mBoneCount, mpModelPose);

	buildDerivedData();

	return true;
}

//*****************************************************************************
void VuSkeleton::load(VuBinaryDataReader &reader)
{
	reader.readValue(mBoneCount);

	allocateData();

	reader.readData(mpBones, mBoneCount*sizeof(mpBones[0]));
	reader.readData(mpParentIndices, mBoneCount*sizeof(mpParentIndices[0]));
	for ( int i = 0; i < mBoneCount; i++ )
		mpModelPose[i].deserialize(reader);
	reader.readValue(mSkeletalAabb);

	buildDerivedData();
}

//*****************************************************************************
void VuSkeleton::save(VuBinaryDataWriter &writer)
{
	writer.writeValue(mBoneCount);
	writer.writeData(mpBones, mBoneCount*sizeof(mpBones[0]));
	for ( int i = 0; i < mBoneCount; i++ )
		writer.writeValue(mpParentIndices[i]);
	for ( int i = 0; i < mBoneCount; i++ )
		mpModelPose[i].serialize(writer);
	writer.writeValue(mSkeletalAabb);
}

//*****************************************************************************
int VuSkeleton::getBoneIndex(const char *name)
{
	for ( int iBone = 0; iBone < mBoneCount; iBone++ )
		if ( strcmp(mpBones[iBone].mName, name) == 0 )
			return iBone;

	return -1;
}

//*****************************************************************************
void VuSkeleton::allocateData()
{
	mpBones = new VuBone[mBoneCount];
	mpParentIndices = new int[mBoneCount];
	mpModelPose = new VuAnimationTransform[mBoneCount];
	mpLocalPose = new VuAnimationTransform[mBoneCount];
	mpInvModelPoseMatrices = new VuMatrix[mBoneCount];
}

//*****************************************************************************
void VuSkeleton::buildDerivedData()
{
	// build local pose
	VuAnimationUtil::transformModelPoseToLocalPose(mBoneCount, mpParentIndices, mpModelPose, mpLocalPose);

	// calculate inverse model pose matrices
	for ( int iBone = 0; iBone < mBoneCount; iBone++ )
	{
		mpModelPose[iBone].toMatrix(mpInvModelPoseMatrices[iBone]);
		mpInvModelPoseMatrices[iBone].invert();
	}
}
