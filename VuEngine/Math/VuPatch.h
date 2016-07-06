//*****************************************************************************
//
//  Copyright (c) 2008-2008 Ralf Knoesel
//  Copyright (c) 2008-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  Patch class
// 
//*****************************************************************************

#pragma once


//    o5 o4
// o6 i3 i2 o3
// o7 i0 i1 o2
//    o0 o1

template<class T>
class VuPatch
{
public:
	void	set(const T &i0, const T &i1, const T &i2, const T &i3,
				const T &o0, const T &o1, const T &o2, const T &o3,
				const T &o4, const T &o5, const T &o6, const T &o7);

	T		interpolate(float u, float v) const;
	T		interpolate(float u, float v, T &gradU, T &gradV) const;

private:
	T		mVal[4];
	T		mVec[8];
	T		mInt[4];
};


#include "VuPatch.inl"
