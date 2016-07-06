//*****************************************************************************
//
//  Copyright (c) 2010-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  Defines useful types.
// 
//*****************************************************************************

#pragma once


#include <assert.h>
#include <stdio.h>
#include <ctime>


#define VUPLATFORM "Ios"


// standard types

typedef int       VUINT;
typedef char      VUINT8;
typedef short     VUINT16;
typedef int       VUINT32;
typedef long long VUINT64;

typedef unsigned int       VUUINT;
typedef unsigned char      VUUINT8;
typedef unsigned short     VUUINT16;
typedef unsigned int       VUUINT32;
typedef unsigned long long VUUINT64;

typedef void *VUHANDLE;

typedef unsigned char VUBYTE;


// endianness

#define VU_LITTLE_ENDIAN 1
#define VU_BIG_ENDIAN 0


// assertion

#if VU_DISABLE_ASSERTS
#define	VUASSERT(exp, msg)
#else
#define	VUASSERT(exp, msg)													\
{																			\
    if ( !(exp) )															\
    {																		\
        VUPRINTF("ASSERT %s (%s) %s %d\n", #msg, #exp, __FILE__, __LINE__);	\
        assert(#exp " " #msg);												\
    }																		\
}
#endif


// compile-time assertion

#define VU_COMPILE_TIME_ASSERT(cond) extern char assertion[(cond) ? 1 : -1]


// alignment

//#define VUALIGN(bytes) __attribute__((aligned(bytes)))
#define VUALIGN(bytes)


// aligned malloc (todo: refactor memory allocations)

//#define VU_ALIGNED_ALLOC(size, alignment) memalign(size, alignment)
#define VU_ALIGNED_ALLOC(size, alignment) malloc(size)
#define VU_ALIGNED_FREE(ptr)              free(ptr)


// stdio stuff

#define VU_SAFE_STDIO 0

#define VU_MEMCPY(dstPtr, dstSize, srcPtr, count) memcpy(dstPtr, srcPtr, count)
#define VU_SPRINTF(strDst, dstSize, format, ...) sprintf(strDst, format, __VA_ARGS__)
#define VU_SNPRINTF(strDst, dstSize, count, format, ...) snprintf(strDst, count, format, __VA_ARGS__)
#define VU_VSNPRINTF(strDst, dstSize, count, format, args) vsnprintf(strDst, count, format, args)
#define VU_SSCANF(buffer, format, ...) sscanf(buffer, format, __VA_ARGS__)
#define VU_STRCPY(strDst, dstSize, strSrc) strcpy(strDst, strSrc)
#define VU_STRCAT(strDst, dstSize, strSrc) strcat(strDst, strSrc)
#define VU_STRNCPY(strDst, dstSize, strSrc, count) strncpy(strDst, strSrc, count)
#define VU_STRNCAT(strDst, dstSize, strSrc, count) strncat(strDst, strSrc, count)
#define VU_STRTOK(strToken, strDelimit, context) strtok(strToken, strDelimit)
#define VU_STRNICMP(str1, str2, count) strncasecmp(str1, str2, count)

int fopen_s(FILE **fp, const char *filename, const char *mode);
char *itoa(int val, char *str, int base);


// time

#define VU_LOCALTIME(pTime, pTS) localtime_r(pTime, pTS)


// no keyboard support

#define VU_DISABLE_KEYBOARD 1


