//*****************************************************************************
//
//  Copyright (c) 2013-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  lzma functionality
// 
//*****************************************************************************

#include "VuLzma.h"
#include "VuEngine/Libs/lzma/LzmaLib.h"
#include "VuEngine/HAL/File/VuFile.h"


#define LZMA_LEVEL 9

//*****************************************************************************
VUUINT32 VuLzma::calcCompressBound(VUUINT32 sourceLen)
{
	return 32*1024 + sourceLen;
}

//*****************************************************************************
bool VuLzma::compressToMemory(void *dest, VUUINT32 *destLen, const void *source, VUUINT32 sourceLen)
{
	VUBYTE *destBytes = (VUBYTE *)dest;
	size_t propsSize = LZMA_PROPS_SIZE;

	size_t dstLen = *destLen - LZMA_PROPS_SIZE;
	if ( LzmaCompress(destBytes + LZMA_PROPS_SIZE, &dstLen, (const VUBYTE *)source, sourceLen, destBytes, &propsSize, LZMA_LEVEL, 0, -1, -1, -1, -1, -1) == SZ_OK )
	{
		*destLen = (VUUINT32)dstLen + LZMA_PROPS_SIZE;
		return true;
	}

	return false;
}

//*****************************************************************************
bool VuLzma::uncompressFromMemory(void *dest, VUUINT32 *destLen, const void *source, VUUINT32 sourceLen)
{
	const VUBYTE *sourceBytes = (const VUBYTE *)source;
	size_t srcLen = sourceLen - LZMA_PROPS_SIZE;
	size_t dstLen = *destLen;
	if ( LzmaUncompress((VUBYTE *)dest, &dstLen, sourceBytes + LZMA_PROPS_SIZE, &srcLen, sourceBytes, LZMA_PROPS_SIZE) == SZ_OK )
	{
		*destLen = (VUUINT32)dstLen;
		return true;
	}

	return false;
}

//*****************************************************************************
bool VuLzma::compressToFile(VUHANDLE hFile, const void *source, VUUINT32 sourceLen)
{
	bool success = false;

	VUUINT32 destLen = calcCompressBound(sourceLen);
	VUBYTE *dest = (VUBYTE *)malloc(destLen);

	if ( compressToMemory(dest, &destLen, source, sourceLen) )
	{
		if ( VuFile::IF()->write(hFile, dest, destLen) == (int)destLen )
			success = true;
	}

	free(dest);

	return success;
}

//*****************************************************************************
bool VuLzma::uncompressFromFile(VUHANDLE hFile, VUUINT32 sourceLen, void *dest, VUUINT32 *destLen)
{
	bool success = false;

	if ( VUBYTE *fileData = (VUBYTE *)malloc(sourceLen) )
	{
		if ( VuFile::IF()->read(hFile, fileData, sourceLen) == (int)sourceLen )
		{
			if ( uncompressFromMemory(dest, destLen, fileData, sourceLen) )
				success = true;
		}

		free(fileData);
	}

	return success;
}
