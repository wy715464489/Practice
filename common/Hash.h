#pragma once

#include "Common.h"

#define VU_FNV32_INIT 0x811c9dc5
#define VU_FNV64_INIT 0xcbf29ce484222325ULL

namespace Hash
{
	// VUUINT32 crc32(const void *pBytes, VUINT nBytes, VUUINT32 crc32 = 0);
	// VUUINT32 crc32String(const char *str, VUUINT32 crc32 = 0);

	UINT32 fnv32(const void *pBytes, INT nBytes, UINT32 fnv32 = VU_FNV32_INIT);
	UINT32 fnv32String(const char *str, UINT32 fnv32 = VU_FNV32_INIT);

	UINT64 fnv64(const void *pBytes, INT nBytes, UINT64 fnv64 = VU_FNV64_INIT);
	UINT64 fnv64String(const char *str, UINT64 fnv64 = VU_FNV64_INIT);
}

// #include "VuHash.inl"