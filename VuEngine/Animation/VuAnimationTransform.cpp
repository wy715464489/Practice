//*****************************************************************************
//
//  Copyright (c) 2009-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  Animation Transform class
// 
//*****************************************************************************

#include "VuAnimationTransform.h"
#include "VuEngine/Math/VuMatrix.h"
#include "VuEngine/Util/VuBinaryDataUtil.h"


//*****************************************************************************
void VuAnimationTransform::loadIdentity()
{
	mTranslation = VuVector3(0,0,0);
	mRotation.loadIdentity();
	mScale = VuVector3(1,1,1);
}

//*****************************************************************************
void VuAnimationTransform::fromMatrix(const VuMatrix &mat)
{
	// get translation
	mTranslation = mat.getTrans();

	// get scale
	mScale.mX = mat.mX.mag3d();
	mScale.mY = mat.mY.mag3d();
	mScale.mZ = mat.mZ.mag3d();

	// normalize axes
	VuMatrix normalizedMatrix = mat;
	normalizedMatrix.mX /= mScale.mX;
	normalizedMatrix.mY /= mScale.mY;
	normalizedMatrix.mZ /= mScale.mZ;

	// get rotation
	mRotation.fromRotationMatrix(normalizedMatrix);
}

//*****************************************************************************
void VuAnimationTransform::toMatrix(VuMatrix &mat) const
{
	mRotation.toRotationMatrix(mat);
	mat.scaleLocal(mScale);
	mat.setTrans(mTranslation);
}

//*****************************************************************************
void VuAnimationTransform::serialize(VuBinaryDataWriter &writer)
{
	if ( VuAbs(mTranslation.mX) < 0.001f ) mTranslation.mX = 0.0f;
	if ( VuAbs(mTranslation.mY) < 0.001f ) mTranslation.mY = 0.0f;
	if ( VuAbs(mTranslation.mZ) < 0.001f ) mTranslation.mZ = 0.0f;
	writer.writeValue(mTranslation.mX);
	writer.writeValue(mTranslation.mY);
	writer.writeValue(mTranslation.mZ);

	VUINT16 quatX = (VUINT16)VuRound(mRotation.mVec.mX*32767);
	VUINT16 quatY = (VUINT16)VuRound(mRotation.mVec.mY*32767);
	VUINT16 quatZ = (VUINT16)VuRound(mRotation.mVec.mZ*32767);
	VUINT16 quatW = (VUINT16)VuRound(mRotation.mVec.mW*32767);
	writer.writeValue(quatX);
	writer.writeValue(quatY);
	writer.writeValue(quatZ);
	writer.writeValue(quatW);

	if ( VuAbs(mScale.mX - 1) < 0.001f ) mScale.mX = 1.0f;
	if ( VuAbs(mScale.mY - 1) < 0.001f ) mScale.mY = 1.0f;
	if ( VuAbs(mScale.mZ - 1) < 0.001f ) mScale.mZ = 1.0f;
	writer.writeValue(mScale.mX);
	writer.writeValue(mScale.mY);
	writer.writeValue(mScale.mZ);
}

//*****************************************************************************
void VuAnimationTransform::deserialize(VuBinaryDataReader &reader)
{
	reader.readValue(mTranslation.mX);
	reader.readValue(mTranslation.mY);
	reader.readValue(mTranslation.mZ);

	VUINT16 quatX, quatY, quatZ, quatW;
	reader.readValue(quatX);
	reader.readValue(quatY);
	reader.readValue(quatZ);
	reader.readValue(quatW);
	mRotation.mVec.mX = quatX*(1.0f/32767);
	mRotation.mVec.mY = quatY*(1.0f/32767);
	mRotation.mVec.mZ = quatZ*(1.0f/32767);
	mRotation.mVec.mW = quatW*(1.0f/32767);

	reader.readValue(mScale.mX);
	reader.readValue(mScale.mY);
	reader.readValue(mScale.mZ);
}
