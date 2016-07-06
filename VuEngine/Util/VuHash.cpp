//*****************************************************************************
//
//  Copyright (c) 2006-2008 Ralf Knoesel
//  Copyright (c) 2008-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  Hash library
// 
//*****************************************************************************

#include "VuHash.h"



static struct VuCrc32Data
{
	VuCrc32Data() : mpTable(0) {}
	~VuCrc32Data();

	void		init();

	VUUINT32	*mpTable;
} sCrc32Data;


inline void calcCrc32(const VUBYTE byte, VUUINT32 &crc32)
{
	crc32 = ((crc32) >> 8) ^ sCrc32Data.mpTable[(byte) ^ ((crc32) & 0x000000FF)];
}


//*****************************************************************************
void VuCrc32Data::init()
{
	if ( mpTable )
		return;

	// This is the official polynomial used by CRC32 in PKZip.
	// Often times the polynomial shown reversed as 0x04C11DB7.
	VUUINT32 dwPolynomial = 0xEDB88320;
	int i, j;

	mpTable = new VUUINT32[256];

	VUUINT32 crc;
	for(i = 0; i < 256; i++)
	{
		crc = i;
		for(j = 8; j > 0; j--)
		{
			if(crc & 1)
				crc = (crc >> 1) ^ dwPolynomial;
			else
				crc >>= 1;
		}
		mpTable[i] = crc;
	}
}

//*****************************************************************************
VuCrc32Data::~VuCrc32Data()
{
	delete[] mpTable;
}

//*****************************************************************************
VUUINT32 VuHash::crc32(const void *pBytes, VUINT nBytes, VUUINT32 crc32)
{
	sCrc32Data.init();

	crc32 = ~crc32;

	for ( int i = 0; i < nBytes; i++ )
		calcCrc32(((VUBYTE *)pBytes)[i], crc32);

	return ~crc32;
}

//*****************************************************************************
VUUINT32 VuHash::crc32String(const char *str, VUUINT32 crc32)
{
	sCrc32Data.init();

	crc32 = ~crc32;

	while ( *str )
	{
		calcCrc32(*str, crc32);
		str++;
	}

	return ~crc32;
}

