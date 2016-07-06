//*****************************************************************************
//
//  Copyright (c) 2007-2008 Ralf Knoesel
//  Copyright (c) 2008-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  Base64 library
// 
//*****************************************************************************

#include "VuBase64.h"


static struct VuBase64Data
{
	VuBase64Data() : mpDecodeTable(0) {}
	~VuBase64Data();

	void		init();

	VUINT8		*mpDecodeTable; // size = 256
} sBase64Data;

static const char sEncodeTable[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";


static inline VUINT8 decode64(char c)
{
	return sBase64Data.mpDecodeTable[(VUUINT8)c];
}

static inline bool isBase64(char c)
{
	return decode64(c) >= 0;
}


//*****************************************************************************
void VuBase64Data::init()
{
	if ( mpDecodeTable )
		return;

	mpDecodeTable = new VUINT8[256];

	for ( int i = 0; i < 256; i++ )
	{
		mpDecodeTable[i] = -1;
		for ( int j = 0; j < 64; j++ )
		{
			if ( sEncodeTable[j] == i )
			{
				mpDecodeTable[i] = (VUUINT8)j;
				break;
			}
		}
	}
}

//*****************************************************************************
VuBase64Data::~VuBase64Data()
{
	delete[] mpDecodeTable;
}

//*****************************************************************************
void VuBase64::encode(const VUBYTE *bytes, int len, std::string &str)
{
	sBase64Data.init();

	VUINT inSize = len;
	const VUUINT8 *pIn = bytes;

	// calc output size
	VUINT outSize = (((inSize) + 2) / 3) * 4;

	// allocate output
	str.clear();
	str.resize(outSize);

	VUINT index = 0;
	while ( inSize )
	{
		str[index++] = sEncodeTable[(pIn[0] >> 2) & 0x3f];
		str[index++] = sEncodeTable[((pIn[0] << 4) + (--inSize ? pIn[1] >> 4 : 0)) & 0x3f];
		str[index++] = inSize ? sEncodeTable[((pIn[1] << 2) + (--inSize ? pIn[2] >> 6 : 0)) & 0x3f] : '=';
		str[index++] = inSize ? sEncodeTable[pIn[2] & 0x3f] : '=';

		if ( inSize )
			inSize--;

		if ( inSize )
			pIn += 3;
	}

	VUASSERT(index == outSize, "VuBase64::encode() output size mismatch");
}

//*****************************************************************************
void VuBase64::encode(const VuArray<VUBYTE> &bytes, std::string &str)
{
	encode(&bytes.begin(), bytes.size(), str);
}

//*****************************************************************************
bool VuBase64::decode(const std::string &str, VuArray<VUBYTE> &bytes)
{
	sBase64Data.init();

	VUINT inSize = (int)str.size();
	const char *pIn = str.c_str();

	// calc output size
	VUINT outSize = 3 * (inSize / 4);

	if ( str.size() > 0 && str[str.size() - 1] == '=' ) outSize--;
	if ( str.size() > 1 && str[str.size() - 2] == '=' ) outSize--;

	// allocate output
	bytes.resize(outSize);

	VUUINT8 *pOut = &bytes.begin();

	while ( inSize >= 2 )
	{
		if ( !isBase64(pIn[0]) || !isBase64(pIn[1]) )
			break;

		*pOut++ = (decode64(pIn[0]) << 2) | (decode64(pIn[1]) >> 4);

		if ( inSize == 2 )
			break;

		if ( pIn[2] == '=' )
		{
			if ( inSize != 4 )
				break;

			if ( pIn[3] != '=' )
				break;
		}
		else
		{
			if ( !isBase64(pIn[2]) )
				break;

			*pOut++ = ((decode64(pIn[1]) << 4) & 0xf0) | (decode64(pIn[2]) >> 2);

			if ( inSize == 3 )
				break;

			if ( pIn[3] == '=' )
			{
				if ( inSize != 4 )
					break;
			}
			else
			{
				if ( !isBase64(pIn[3]) )
					break;

				*pOut++ = ((decode64(pIn[2]) << 6) & 0xc0) | decode64(pIn[3]);
			}
		}

		pIn += 4;
		inSize -= 4;
	}

	if ( pOut - &bytes.begin() != bytes.size() )
	{
		bytes.resize(0);
		return false;
	}

	return true;
}
