//*****************************************************************************
//
//  Copyright (c) 2009-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  zlib utility functionality
// 
//*****************************************************************************

#include <zlib.h>

#include "VuZLibUtil.h"
#include "VuEngine/HAL/File/VuFile.h"
#include "VuEngine/Math/VuMath.h"


#define VU_ZLIB_CHUNK_SIZE (128*1024)

namespace VuZLibUtil
{
	int gzipUncompress(Bytef *dest, uLongf *destLen, const Bytef *source, uLong sourceLen);
}

//*****************************************************************************
VUUINT32 VuZLibUtil::calcCompressBound(VUUINT32 sourceLen)
{
	return ::compressBound(sourceLen);
}

//*****************************************************************************
bool VuZLibUtil::compressToMemory(void *dest, VUUINT32 *destLen, const void *source, VUUINT32 sourceLen)
{
	uLongf dl = *destLen;
	bool success = ::compress(static_cast<VUBYTE *>(dest), &dl, static_cast<const VUBYTE *>(source), sourceLen) == Z_OK;
	*destLen = dl;

	return success;
}

//*****************************************************************************
bool VuZLibUtil::uncompressFromMemory(void *dest, VUUINT32 *destLen, const void *source, VUUINT32 sourceLen)
{
	uLongf dl = *destLen;
	bool success = ::uncompress(static_cast<VUBYTE *>(dest), &dl, static_cast<const VUBYTE *>(source), sourceLen) == Z_OK;
	*destLen = dl;

	return success;
}

//*****************************************************************************
bool VuZLibUtil::compressToFile(VUHANDLE hFile, const void *source, VUUINT32 sourceLen)
{
	z_stream strm;
	memset(&strm, 0, sizeof(strm));
	if ( deflateInit(&strm, Z_DEFAULT_COMPRESSION) != Z_OK )
		return false;

	strm.next_in = (Bytef *)source;
	strm.avail_in = sourceLen;

	VUBYTE chunk[VU_ZLIB_CHUNK_SIZE];
	{
		int result = Z_OK;
		while ( result == Z_OK )
		{
			strm.next_out = (Bytef *)chunk;
			strm.avail_out = VU_ZLIB_CHUNK_SIZE;

			result = deflate(&strm, Z_FINISH);

			int have = VU_ZLIB_CHUNK_SIZE - strm.avail_out;
			if ( VuFile::IF()->write(hFile, chunk, have) != have )
			{
				deflateEnd(&strm);
				return false;
			}
		}

		if ( deflateEnd(&strm) != Z_OK )
			return false;

		if ( result != Z_STREAM_END || strm.avail_in != 0 )
			return false;
	}

	return true;
}

//*****************************************************************************
bool VuZLibUtil::uncompressFromFile(VUHANDLE hFile, VUUINT32 sourceLen, void *dest, VUUINT32 *destLen)
{
	z_stream strm;
	memset(&strm, 0, sizeof(strm));
	if ( inflateInit(&strm) != Z_OK )
		return false;

	strm.next_out = (Bytef *)dest;
	strm.avail_out = *destLen;

	VUBYTE chunk[VU_ZLIB_CHUNK_SIZE];
	{
		int result = Z_OK;
		while ( result == Z_OK )
		{
			strm.next_in = (Bytef *)chunk;
			strm.avail_in = VuFile::IF()->read(hFile, chunk, VuMin(sourceLen, (VUUINT32)VU_ZLIB_CHUNK_SIZE));

			sourceLen -= strm.avail_in;

			result = inflate(&strm, Z_NO_FLUSH);
		}

		if ( inflateEnd(&strm) != Z_OK )
			return false;

		if ( result != Z_STREAM_END )
			return false;
	}

	*destLen -= strm.avail_out;

	return true;
}

//*****************************************************************************
bool VuZLibUtil::gzipUncompressFromMemory(void *dest, VUUINT32 *destLen, const void *source, VUUINT32 sourceLen)
{
	uLongf dl = *destLen;
	bool success = gzipUncompress(static_cast<VUBYTE *>(dest), &dl, static_cast<const VUBYTE *>(source), sourceLen) == Z_OK;
	*destLen = dl;

	return success;
}

//*****************************************************************************
int VuZLibUtil::gzipUncompress (Bytef *dest, uLongf *destLen, const Bytef *source, uLong sourceLen)
{
    z_stream stream;
    int err;

    stream.next_in = (Bytef*)source;
    stream.avail_in = (uInt)sourceLen;
    /* Check for source > 64K on 16-bit machine: */
    if ((uLong)stream.avail_in != sourceLen) return Z_BUF_ERROR;

    stream.next_out = dest;
    stream.avail_out = (uInt)*destLen;
    if ((uLong)stream.avail_out != *destLen) return Z_BUF_ERROR;

    stream.zalloc = (alloc_func)0;
    stream.zfree = (free_func)0;

	err = inflateInit2(&stream, 32+MAX_WBITS);
    if (err != Z_OK) return err;

    err = inflate(&stream, Z_FINISH);
    if (err != Z_STREAM_END) {
        inflateEnd(&stream);
        if (err == Z_NEED_DICT || (err == Z_BUF_ERROR && stream.avail_in == 0))
            return Z_DATA_ERROR;
        return err;
    }
    *destLen = stream.total_out;

    err = inflateEnd(&stream);
    return err;
}
