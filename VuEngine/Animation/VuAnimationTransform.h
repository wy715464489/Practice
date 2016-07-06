//*****************************************************************************
//
//  Copyright (c) 2009-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  Animation Transform class
// 
//*****************************************************************************

#pragma once

#include "VuEngine/Math/VuVector3.h"
#include "VuEngine/Math/VuQuaternion.h"

class VuBinaryDataWriter;
class VuBinaryDataReader;


class VuAnimationTransform
{
public:
	VuAnimationTransform()	{}

	void			loadIdentity();

	void			fromMatrix(const VuMatrix &mat);
	void			toMatrix(VuMatrix &mat) const;

	inline void		setZero();
	inline void		blendAddMul(const VuAnimationTransform &other, float weight = 1.0f);
	inline void		normalize(float weight = 1.0f);

	void			serialize(VuBinaryDataWriter &writer);
	void			deserialize(VuBinaryDataReader &reader);

	VuVector3		mTranslation;
	VuQuaternion	mRotation;
	VuVector3		mScale;
};

#include "VuAnimationTransform.inl"
