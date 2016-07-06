//*****************************************************************************
//
//  Copyright (c) 2009-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  Skeleton class
// 
//*****************************************************************************

#pragma once

#include "VuEngine/Objects/VuRefObj.h"
#include "VuEngine/Math/VuAabb.h"


class VuJsonContainer;
class VuBinaryDataReader;
class VuBinaryDataWriter;
class VuAnimationTransform;
class VuMatrix;


class VuSkeleton : public VuRefObj
{
public:
	VuSkeleton();
protected:
	~VuSkeleton();

public:
	bool	load(const VuJsonContainer &data);
	void	load(VuBinaryDataReader &reader);
	void	save(VuBinaryDataWriter &writer);

	int		getBoneIndex(const char *name); // return -1 if not found

	struct VuBone
	{
		char	mName[32];
	};

	int						mBoneCount;
	VuBone					*mpBones;
	int						*mpParentIndices;
	VuAnimationTransform	*mpModelPose;
	VuAnimationTransform	*mpLocalPose;
	VuMatrix				*mpInvModelPoseMatrices;
	VuAabb					mSkeletalAabb;

private:
	void	allocateData();
	void	buildDerivedData();
};