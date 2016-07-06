//*****************************************************************************
//
//  Copyright (c) 2005-2008 Ralf Knoesel
//  Copyright (c) 2008-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  Common functionality.
// 
//*****************************************************************************

#pragma once

// define configuration-specific defines
#if defined(VUDEBUG)
	#include "VuDebugSettings.h"
#elif defined(VURELEASE)
	#include "VuReleaseSettings.h"
#elif defined(VURETAIL)
	#include "VuRetailSettings.h"
#else
	#error Configuration not defined!
#endif

// platform-specific defines
#if defined(VUWIN32)
	#include "VuWin32.h"
#elif defined(VUXBOX360)
	#include "VuXbox360.h"
#elif defined(VUANDROID)
    #include "VuAndroid.h"
#elif defined(VUIOS)
    #include "VuIos.h"
#elif defined(VUWINSTORE) || defined(VUWINPHONE)
	#include "VuWindows.h"
#elif defined(VUBB10)
	#include "VuBB10.h"
/*
#elif defined(VUPS4)
	#include "VuPs4.h"*/
#elif defined(VUXB1)
	#include "VuXb1.h"
#else
	#error Platform not defined!
#endif


// null
#define VUNULL 0

// alignment macro
inline VUUINT VuAlign(VUUINT value, VUUINT alignment)
{
	VUUINT result = (value + alignment - 1)/alignment;
	result *= alignment;

	return result;
}

// swap template
template<class T>
inline void VuSwap(T &a, T &b)
{
	T tmp = a;
	a = b;
	b = tmp;
}

// test for alpha-numeric character
inline bool VuIsAlphaNum(char c)
{
	if ( ( c >= '0' && c <= '9' ) ||
		 ( c >= 'A' && c <= 'Z' ) ||
		 ( c >= 'a' && c <= 'z' ) )
		return true;
	return false;
}

// count the number of 'on' bits in n
inline int VuBitCount(VUUINT32 n)
{
	n = n - ((n >> 1) & 0x55555555);
	n = (n & 0x33333333) + ((n >> 2) & 0x33333333);
	n = (((n + (n >> 4)) & 0xf0f0f0f) * 0x1010101) >> 24;

	return n;
}

// compute the next highest power of 2
inline VUUINT32 VuNextHighestPower2(VUUINT32 n)
{
	n--;
	n |= n >> 1;
	n |= n >> 2;
	n |= n >> 4;
	n |= n >> 8;
	n |= n >> 16;
	n++;

	return n;
}

// compute the high bit
inline VUUINT32 VuHighBit(VUUINT32 n)
{
	VUUINT32 result = 0;
	while ( n >>= 1)
		result++;

	return result;
}

// Lock-Free algorithm support
template<typename T> struct VuLockFreeNode
{
	VuLockFreeNode() : mpNext(VUNULL) {}
	VuLockFreeNode(T v) : mValue(v), mpNext(VUNULL) {}

	T								mValue;
	VuLockFreeNode<T> * volatile	mpNext;
};
template<typename T>
inline bool VuCompareAndSwap(VuLockFreeNode<T> * volatile *_ptr, VuLockFreeNode<T> *oldVal, VuLockFreeNode<T> *newVal);
template<typename T>
inline bool VuCompareAndSwap2(VuLockFreeNode<T> * volatile *_ptr, VuLockFreeNode<T> *old1, VUUINT32 old2, VuLockFreeNode<T> *new1, VUUINT32 new2);


// include system interface
#include "VuEngine/HAL/Sys/VuSys.h"
