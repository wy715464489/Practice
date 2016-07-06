//*****************************************************************************
//
//  Copyright (c) 2006-2008 Ralf Knoesel
//  Copyright (c) 2008-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  Hash library
// 
//*****************************************************************************


/*
 * 32 bit magic FNV-1a prime
 */
#define FNV_32_PRIME ((VUUINT32)0x01000193)
#define FNV_64_PRIME ((VUUINT64)0x100000001b3ULL)


//*****************************************************************************
VUUINT32 VuHash::fnv32(const void *pBytes, VUINT nBytes, VUUINT32 fnv32)
{
	const unsigned char *bp = (unsigned char *)pBytes; /* start of buffer */
	const unsigned char *be = bp + nBytes;             /* beyond end of buffer */

	/*
	* FNV-1a hash each octet in the buffer
	*/
	while (bp < be)
	{
		/* xor the bottom with the current octet */
		fnv32 ^= (VUUINT32)*bp++;

		/* multiply by the 32 bit FNV magic prime mod 2^32 */
		fnv32 *= FNV_32_PRIME;
	}

	/* return our new hash value */
	return fnv32;
}

//*****************************************************************************
VUUINT32 VuHash::fnv32String(const char *str, VUUINT32 fnv32)
{
	const unsigned char *s = (const unsigned char *)str;	/* unsigned string */

	/*
	* FNV-1a hash each octet in the buffer
	*/
	while (*s)
	{
		/* xor the bottom with the current octet */
		fnv32 ^= (VUUINT32)*s++;

		/* multiply by the 32 bit FNV magic prime mod 2^32 */
		fnv32 *= FNV_32_PRIME;
	}

	/* return our new hash value */
	return fnv32;
}

//*****************************************************************************
VUUINT64 VuHash::fnv64(const void *pBytes, VUINT nBytes, VUUINT64 fnv64)
{
	const unsigned char *bp = (unsigned char *)pBytes; /* start of buffer */
	const unsigned char *be = bp + nBytes;             /* beyond end of buffer */

	/*
	* FNV-1a hash each octet in the buffer
	*/
	while (bp < be)
	{
		/* xor the bottom with the current octet */
		fnv64 ^= (VUUINT64)*bp++;

		/* multiply by the 64 bit FNV magic prime mod 2^64 */
		fnv64 *= FNV_64_PRIME;
	}

	/* return our new hash value */
	return fnv64;
}

//*****************************************************************************
VUUINT64 VuHash::fnv64String(const char *str, VUUINT64 fnv64)
{
	const unsigned char *s = (const unsigned char *)str;	/* unsigned string */

	/*
	* FNV-1a hash each octet in the buffer
	*/
	while (*s)
	{
		/* xor the bottom with the current octet */
		fnv64 ^= (VUUINT64)*s++;

		/* multiply by the 64 bit FNV magic prime mod 2^64 */
		fnv64 *= FNV_64_PRIME;
	}

	/* return our new hash value */
	return fnv64;
}

