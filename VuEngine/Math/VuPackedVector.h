//*****************************************************************************
//
//  Copyright (c) 2008-2008 Ralf Knoesel
//  Copyright (c) 2008-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  Packed Vector classes
// 
//*****************************************************************************

#pragma once


class VuPackedVector2
{
public:
	VuPackedVector2()								{}
	VuPackedVector2(float x, float y)				{ mX = x; mY = y; }

	bool operator ==(const VuPackedVector2 &v) const	{ return mX == v.mX && mY == v.mY; }
	bool operator !=(const VuPackedVector2 &v) const	{ return mX != v.mX || mY != v.mY; }

	float mX;
	float mY;
};

class VuPackedVector3
{
public:
	VuPackedVector3()								{}
	VuPackedVector3(float x, float y, float z)		{ mX = x; mY = y; mZ = z; }

	bool operator ==(const VuPackedVector3 &v) const	{ return mX == v.mX && mY == v.mY && mZ == v.mZ; }
	bool operator !=(const VuPackedVector3 &v) const	{ return mX != v.mX || mY != v.mY || mZ != v.mZ; }

	float mX;
	float mY;
	float mZ;
};

class VuPackedVector4
{
public:
	VuPackedVector4()									{}
	VuPackedVector4(float x, float y, float z, float w)	{ mX = x; mY = y; mZ = z; mW = w; }

	bool operator ==(const VuPackedVector4 &v) const	{ return mX == v.mX && mY == v.mY && mZ == v.mZ && mW == v.mW; }
	bool operator !=(const VuPackedVector4 &v) const	{ return mX != v.mX || mY != v.mY || mZ != v.mZ || mW != v.mW; }

	float mX;
	float mY;
	float mZ;
	float mW;
};
