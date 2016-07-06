//*****************************************************************************
//
//  Copyright (c) 2006-2008 Ralf Knoesel
//  Copyright (c) 2008-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  Hash library
// 
//*****************************************************************************

#pragma once

#define VU_FNV32_INIT 0x811c9dc5
#define VU_FNV64_INIT 0xcbf29ce484222325ULL

namespace VuHash
{
	VUUINT32 crc32(const void *pBytes, VUINT nBytes, VUUINT32 crc32 = 0);
	VUUINT32 crc32String(const char *str, VUUINT32 crc32 = 0);

	inline VUUINT32 fnv32(const void *pBytes, VUINT nBytes, VUUINT32 fnv32 = VU_FNV32_INIT);
	inline VUUINT32 fnv32String(const char *str, VUUINT32 fnv32 = VU_FNV32_INIT);

	inline VUUINT64 fnv64(const void *pBytes, VUINT nBytes, VUUINT64 fnv64 = VU_FNV64_INIT);
	inline VUUINT64 fnv64String(const char *str, VUUINT64 fnv64 = VU_FNV64_INIT);
}

#include "VuHash.inl"
