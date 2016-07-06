//*****************************************************************************
//
//  Copyright (c) 2008-2008 Ralf Knoesel
//  Copyright (c) 2008-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  Rect inline functionality
// 
//*****************************************************************************


//*****************************************************************************
VuRect::VuRect(const VuVector2 &v0, const VuVector2 &v1)
{
	mX = VuMin(v0.mX, v1.mX);
	mY = VuMin(v0.mY, v1.mY);
	mWidth = VuMax(v0.mX, v1.mX) - mX;
	mHeight = VuMax(v0.mY, v1.mY) - mY;
}

//*****************************************************************************
VuRect VuRect::intersection(const VuRect &r0, const VuRect &r1)
{
	float x0 = VuMax(r0.getLeft(), r1.getLeft());
	float x1 = VuMin(r0.getRight(), r1.getRight());
	float y0 = VuMax(r0.getTop(), r1.getTop());
	float y1 = VuMin(r0.getBottom(), r1.getBottom());

	return VuRect(x0, y0, x1 - x0, y1 - y0);
}

//*****************************************************************************
VuRect VuRect::bounds(const VuRect &r0, const VuRect &r1)
{
	float x0 = VuMin(r0.getLeft(), r1.getLeft());
	float x1 = VuMax(r0.getRight(), r1.getRight());
	float y0 = VuMin(r0.getTop(), r1.getTop());
	float y1 = VuMax(r0.getBottom(), r1.getBottom());

	return VuRect(x0, y0, x1 - x0, y1 - y0);
}

//*****************************************************************************
void VuRect::scale(const VuVector2 &origin, float scale)
{
	mX = origin.mX + scale*(mX - origin.mX);
	mY = origin.mY + scale*(mY - origin.mY);
	mWidth *= scale;
	mHeight *= scale;
}

//*****************************************************************************
void VuRect::add(const VuRect &r)
{
	float x0 = VuMin(getLeft(), r.getLeft());
	float x1 = VuMax(getRight(), r.getRight());
	float y0 = VuMin(getTop(), r.getTop());
	float y1 = VuMax(getBottom(), r.getBottom());

	mX = x0; mY = y0; mWidth = x1 - x0; mHeight = y1 - y0;
}

//*****************************************************************************
void VuRect::add(const VuVector2 &v)
{
	float x0 = VuMin(getLeft(), v.mX);
	float x1 = VuMax(getRight(), v.mX);
	float y0 = VuMin(getTop(), v.mY);
	float y1 = VuMax(getBottom(), v.mY);

	mX = x0; mY = y0; mWidth = x1 - x0; mHeight = y1 - y0;
}

//*****************************************************************************
inline VuRect VuLerp(const VuRect &a, const VuRect &b, float factor)
{
	VuRect rect;

	rect.mX = (1.0f - factor)*a.mX + factor*b.mX;
	rect.mY = (1.0f - factor)*a.mY + factor*b.mY;
	rect.mWidth = (1.0f - factor)*a.mWidth + factor*b.mWidth;
	rect.mHeight = (1.0f - factor)*a.mHeight + factor*b.mHeight;

	return rect;
}

