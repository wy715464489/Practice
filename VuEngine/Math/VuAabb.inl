//*****************************************************************************
//
//  Copyright (c) 2011-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  VuAabb inline functionality
// 
//*****************************************************************************


//*****************************************************************************
inline bool VuAabb::contains(const VuAabb &bounds) const
{
	return mMin.mX <= bounds.mMin.mX && mMin.mY <= bounds.mMin.mY && mMin.mZ <= bounds.mMin.mZ &&
		   mMax.mX >= bounds.mMax.mX && mMax.mY >= bounds.mMax.mY && mMax.mZ >= bounds.mMax.mZ;
}

//*****************************************************************************
inline bool VuAabb::contains(const VuVector3 &point) const
{
	return mMin.mX <= point.mX && mMin.mY <= point.mY && mMin.mZ <= point.mZ &&
		   mMax.mX >= point.mX && mMax.mY >= point.mY && mMax.mZ >= point.mZ;
}

//*****************************************************************************
inline bool VuAabb::intersects(const VuAabb &bounds) const
{
	return mMin.mX <= bounds.mMax.mX && mMax.mX >= bounds.mMin.mX &&
		   mMin.mY <= bounds.mMax.mY && mMax.mY >= bounds.mMin.mY &&
		   mMin.mZ <= bounds.mMax.mZ && mMax.mZ >= bounds.mMin.mZ;
}
