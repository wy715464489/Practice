//*****************************************************************************
//
//  Copyright (c) 2005-2008 Ralf Knoesel
//  Copyright (c) 2008-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  Rect class
// 
//*****************************************************************************

#pragma once

#include "VuEngine/Math/VuVector2.h"

class VuRect
{
public:
	VuRect()									{}
	VuRect(float x, float y, float w, float h)	{ mX = x; mY = y; mWidth = w; mHeight = h; }
	VuRect(int x, int y, int w, int h)			{ mX = (float)x; mY = (float)y; mWidth = (float)w; mHeight = (float)h; }
	inline VuRect(const VuVector2 &v0, const VuVector2 &v1);

	inline void							set(float x, float y, float w, float h)	{ mX = x; mY = y; mWidth = w; mHeight = h; }
	inline void							set(int x, int y, int w, int h)			{ mX = (float)x; mY = (float)y; mWidth = (float)w; mHeight = (float)h; }

	bool isValid() const				{ return mWidth >= 0 && mHeight >= 0; }

	float getLeft()	const				{ return mX; }
	float getRight() const				{ return mX + mWidth; }
	float getTop() const				{ return mY; }
	float getBottom() const				{ return mY + mHeight; }
	VuVector2 getTopLeft() const		{ return VuVector2(getLeft(), getTop()); }
	VuVector2 getTopRight() const		{ return VuVector2(getRight(), getTop()); }
	VuVector2 getBottomLeft() const		{ return VuVector2(getLeft(), getBottom()); }
	VuVector2 getBottomRight() const	{ return VuVector2(getRight(), getBottom()); }
	VuVector2 getCenter() const			{ return VuVector2(mX + 0.5f*mWidth, mY + 0.5f*mHeight); }
	VuVector2 getCenterLeft() const		{ return VuVector2(getLeft(), mY + 0.5f*mHeight); }
	VuVector2 getCenterRight() const	{ return VuVector2(getRight(), mY + 0.5f*mHeight); }
	VuVector2 getCenterTop() const		{ return VuVector2(mX + 0.5f*mWidth, getTop()); }
	VuVector2 getCenterBottom() const	{ return VuVector2(mX + 0.5f*mWidth, getBottom()); }
	float getWidth() const				{ return mWidth; }
	float getHeight() const				{ return mHeight; }
	VuVector2 getSize() const			{ return VuVector2(mWidth, mHeight); }
	float getArea() const				{ return mWidth*mHeight; }

	// equality
	bool operator ==(const VuRect &r) const	{ return mX == r.mX && mY == r.mY && mWidth == r.mWidth && mHeight == r.mHeight; }
	bool operator !=(const VuRect &r) const	{ return mX != r.mX || mY != r.mY || mWidth != r.mWidth || mHeight != r.mHeight; }

	// containment
	inline bool contains(const VuVector2 &point) const { return (point.mX >= getLeft()) && (point.mX <= getRight()) && (point.mY >= getTop()) && (point.mY <= getBottom()); }

	// intersection
	inline bool				intersects(const VuRect &rect) const { return (rect.getRight() >= getLeft()) && (rect.getLeft() <= getRight()) && (rect.getBottom() >= getTop()) && (rect.getTop() <= getBottom()); }
	bool					intersects(const VuVector2 &p0, const VuVector2 &p1) const;
	inline static VuRect	intersection(const VuRect &r0, const VuRect &r1);
	inline static VuRect	bounds(const VuRect &r0, const VuRect &r1);

	// translate/scale
	VuRect operator *(const VuVector2 &point) const { return VuRect(mX*point.mX, mY*point.mY, mWidth*point.mX, mHeight*point.mY); }
	VuRect operator /(const VuVector2 &point) const { return VuRect(mX/point.mX, mY/point.mY, mWidth/point.mX, mHeight/point.mY); }
	VuRect operator +(const VuVector2 &point) const { return VuRect(mX + point.mX, mY + point.mY, mWidth, mHeight); }
	VuRect operator -(const VuVector2 &point) const { return VuRect(mX - point.mX, mY - point.mY, mWidth, mHeight); }
	void operator *=(const VuVector2 &point) { mX *= point.mX; mY *= point.mY; mWidth *= point.mX; mHeight *= point.mY; }
	void operator /=(const VuVector2 &point) { mX /= point.mX; mY /= point.mY; mWidth /= point.mX; mHeight /= point.mY; }
	void operator +=(const VuVector2 &point) { mX += point.mX; mY += point.mY; }
	void operator -=(const VuVector2 &point) { mX -= point.mX; mY -= point.mY; }

	inline void		scale(const VuVector2 &origin, float scale);

	inline void		flipX() { mX += mWidth; mWidth *= -1; }
	inline void		flipY() { mY += mHeight; mHeight *= -1; }

	// add
	inline void add(const VuRect &r);
	inline void add(const VuVector2 &v);

	float mX;
	float mY;
	float mWidth;
	float mHeight;
};

#include "VuRect.inl"
