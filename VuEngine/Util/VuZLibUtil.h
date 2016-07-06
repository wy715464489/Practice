//*****************************************************************************
//
//  Copyright (c) 2009-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  zlib utility functionality
// 
//*****************************************************************************

#pragma once


namespace VuZLibUtil
{
	VUUINT32	calcCompressBound(VUUINT32 sourceLen);
	bool		compressToMemory(void *dest, VUUINT32 *destLen, const void *source, VUUINT32 sourceLen);
	bool		uncompressFromMemory(void *dest, VUUINT32 *destLen, const void *source, VUUINT32 sourceLen);

	bool		compressToFile(VUHANDLE hFile, const void *source, VUUINT32 sourceLen);
	bool		uncompressFromFile(VUHANDLE hFile, VUUINT32 sourceLen, void *dest, VUUINT32 *destLen);

	bool		gzipUncompressFromMemory(void *dest, VUUINT32 *destLen, const void *source, VUUINT32 sourceLen);
}
