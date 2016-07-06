//*****************************************************************************
//
//  Copyright (c) 2009-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  Animation Transform inline functionality
// 
//*****************************************************************************

//*****************************************************************************
void VuAnimationTransform::setZero()
{
	mTranslation = VuVector3(0,0,0);
	mRotation.mVec = VuVector4(0,0,0,0);
	mScale = VuVector3(0,0,0);
}

//*****************************************************************************
void VuAnimationTransform::blendAddMul(const VuAnimationTransform &other, float weight)
{
	mTranslation += weight*other.mTranslation;
	mScale += weight*other.mScale;

	float signedWeight = VuDot(mRotation.mVec, other.mRotation.mVec) < 0.0f ? -weight : weight;
	mRotation.mVec += signedWeight*other.mRotation.mVec;
}

//*****************************************************************************
void VuAnimationTransform::normalize(float weight)
{
	const float invWeight = 1.0f/weight;

	mTranslation *= invWeight;
	mScale *= invWeight;
	mRotation.normalize();
}
