/*
 * 32 bit magic FNV-1a prime
 */
#include "Hash.h"

#define FNV_32_PRIME ((UINT32)0x01000193)
#define FNV_64_PRIME ((UINT64)0x100000001b3ULL)


//*****************************************************************************
UINT32 Hash::fnv32(const void *pBytes, INT nBytes, UINT32 fnv32)
{
	const unsigned char *bp = (unsigned char *)pBytes; /* start of buffer */
	const unsigned char *be = bp + nBytes;             /* beyond end of buffer */

	/*
	* FNV-1a hash each octet in the buffer
	*/
	while (bp < be)
	{
		/* xor the bottom with the current octet */
		fnv32 ^= (UINT32)*bp++;

		/* multiply by the 32 bit FNV magic prime mod 2^32 */
		fnv32 *= FNV_32_PRIME;
	}

	/* return our new hash value */
	return fnv32;
}

//*****************************************************************************
UINT32 Hash::fnv32String(const char *str, UINT32 fnv32)
{
	const unsigned char *s = (const unsigned char *)str;	/* unsigned string */

	/*
	* FNV-1a hash each octet in the buffer
	*/
	while (*s)
	{
		/* xor the bottom with the current octet */
		fnv32 ^= (UINT32)*s++;

		/* multiply by the 32 bit FNV magic prime mod 2^32 */
		fnv32 *= FNV_32_PRIME;
	}

	/* return our new hash value */
	return fnv32;
}

//*****************************************************************************
UINT64 Hash::fnv64(const void *pBytes, INT nBytes, UINT64 fnv64)
{
	const unsigned char *bp = (unsigned char *)pBytes; /* start of buffer */
	const unsigned char *be = bp + nBytes;             /* beyond end of buffer */

	/*
	* FNV-1a hash each octet in the buffer
	*/
	while (bp < be)
	{
		/* xor the bottom with the current octet */
		fnv64 ^= (UINT64)*bp++;

		/* multiply by the 64 bit FNV magic prime mod 2^64 */
		fnv64 *= FNV_64_PRIME;
	}

	/* return our new hash value */
	return fnv64;
}

//*****************************************************************************
UINT64 Hash::fnv64String(const char *str, UINT64 fnv64)
{
	const unsigned char *s = (const unsigned char *)str;	/* unsigned string */

	/*
	* FNV-1a hash each octet in the buffer
	*/
	while (*s)
	{
		/* xor the bottom with the current octet */
		fnv64 ^= (UINT64)*s++;

		/* multiply by the 64 bit FNV magic prime mod 2^64 */
		fnv64 *= FNV_64_PRIME;
	}

	/* return our new hash value */
	return fnv64;
}