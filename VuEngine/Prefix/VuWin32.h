//*****************************************************************************
//
//  Copyright (c) 2005-2008 Ralf Knoesel
//  Copyright (c) 2008-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  Defines useful types.
// 
//*****************************************************************************

#pragma once


#include <assert.h>
#include <stdio.h>
#include <ctime>


#define VUPLATFORM "Win32"


// undefine some breaking windows defines

#undef min
#undef max


// standard types

typedef   int   VUINT;
typedef __int8  VUINT8;
typedef __int16 VUINT16;
typedef __int32 VUINT32;
typedef __int64 VUINT64;

typedef unsigned   int   VUUINT;
typedef unsigned __int8  VUUINT8;
typedef unsigned __int16 VUUINT16;
typedef unsigned __int32 VUUINT32;
typedef unsigned __int64 VUUINT64;

typedef void *VUHANDLE;

typedef unsigned char VUBYTE;


// endianness

#define VU_LITTLE_ENDIAN 1
#define VU_BIG_ENDIAN 0


// assertion

#if VU_DISABLE_ASSERTS
	#define	VUASSERT(exp, msg)
#else
	extern "C" {
		_CRTIMP void __cdecl _assert(_In_z_ const char * _Message, _In_z_ const char *_File, _In_ unsigned _Line);
	}
	//#define VUASSERT(exp, msg) ((void)((exp) || (_assert(#msg "(" #exp ")", __FILE__, __LINE__), 0)))
	#define VUASSERT(exp, msg) \
	{ \
		if ( !(exp) ) \
		{ \
			if ( VuSys::IF() ) \
			{ \
				VuSys::IF()->print("ASSERT: " #msg "(" #exp ")\n"); \
			} \
			_assert(#msg "(" #exp ")", __FILE__, __LINE__); \
		} \
	}
#endif


// compile-time assertion

#define VU_COMPILE_TIME_ASSERT(cond) extern char assertion[(cond) ? 1 : -1]


// alignment

//#define VUALIGN(bytes) __declspec(align(bytes))
#define VUALIGN(bytes)


// aligned malloc (todo: refactor memory allocations)

#define VU_ALIGNED_ALLOC(size, alignment) _aligned_malloc(size, alignment)
#define VU_ALIGNED_FREE(ptr)              _aligned_free(ptr)


// stdio stuff

#define VU_SAFE_STDIO 1

#ifdef VUDEBUG
	extern void VuDebugMemCpy(void*, size_t, const void*, size_t);
	#define VU_MEMCPY(dstPtr, dstSize, srcPtr, count) VuDebugMemCpy(dstPtr, dstSize, srcPtr, count)
#else
	#define VU_MEMCPY(dstPtr, dstSize, srcPtr, count) memcpy_s(dstPtr, dstSize, srcPtr, count)
#endif

#define VU_SPRINTF(strDst, dstSize, format, ...) sprintf_s(strDst, dstSize, format, __VA_ARGS__) 
#define VU_SNPRINTF(strDst, dstSize, count, format, ...) _snprintf_s(strDst, dstSize, count, format, __VA_ARGS__)
#define VU_VSNPRINTF(strDst, dstSize, count, format, args) vsnprintf_s(strDst, dstSize, count, format, args)
#define VU_SSCANF(buffer, format, ...) sscanf_s(buffer, format, __VA_ARGS__)
#define VU_STRCPY(strDst, dstSize, strSrc) strcpy_s(strDst, dstSize, strSrc)
#define VU_STRCAT(strDst, dstSize, strSrc) strcat_s(strDst, dstSize, strSrc)
#define VU_STRNCPY(strDst, dstSize, strSrc, count) strncpy_s(strDst, dstSize, strSrc, count)
#define VU_STRNCAT(strDst, dstSize, strSrc, count) strncat_s(strDst, dstSize, strSrc, count)
#define VU_STRTOK(strToken, strDelimit, context) strtok_s(strToken, strDelimit, context)
#define VU_STRNICMP(str1, str2, count) _strnicmp(str1, str2, count)


// time

#define VU_LOCALTIME(pTime, pTS) localtime_s(pTS, pTime)


// misc

#define VERIFY_PACKED_ASSET_HASH 1
