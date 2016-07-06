//*****************************************************************************
//
//  Copyright (c) 2010-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  TGA utility.
// 
//*****************************************************************************

#include "VuTgaUtil.h"
#include "VuEngine/HAL/File/VuFile.h"
#include "VuEngine/Util/VuEndianUtil.h"
#include "VuEngine/Util/VuFileUtil.h"


//*****************************************************************************
VuTgaLoader::VuTgaLoader():
	mWidth(0), mHeight(0), mBPP(0), mImageSize(0), mEnc(0),
	mpImage(VUNULL), mpPalette(VUNULL), mpData(VUNULL)
{
}

//*****************************************************************************
VuTgaLoader::~VuTgaLoader()
{
	delete mpImage;
	delete mpPalette;
}

//*****************************************************************************
VuTgaLoader::eResult VuTgaLoader::load(const std::string &fileName)
{
	// load file
	VuArray<VUBYTE> fileData;
	if ( !VuFileUtil::loadFile(fileName, fileData) )
		return ERR_NO_FILE;

	return load(&fileData[0], fileData.size());
}

//*****************************************************************************
VuTgaLoader::eResult VuTgaLoader::load(const VUBYTE *pData, int dataSize)
{
	eResult res;

	// Clear out any existing image and palette
	delete mpImage;
	mpImage = VUNULL;
	delete mpPalette;
	mpPalette = VUNULL;
	mpData = pData;

	// Process the header
	res = readHeader();
	if ( res != OK )
		return res;

	switch(mEnc)
	{
		case 1: // Raw Indexed
		{
			// Check filesize against header values
			if ( (mImageSize + 18 + mpData[0] + 768) > dataSize )
				return ERR_BAD_FORMAT;

			// Double check image type field
			if ( mpData[1] != 1 )
				return ERR_BAD_FORMAT;

			// Load image data
			res = loadRawData();
			if ( res != OK )
				return res;

			// Load palette
			res = loadTgaPalette();
			if ( res != OK )
				return res;

			break;
		}

	case 2: // Raw RGB
		{
			// Check filesize against header values
			if ( (mImageSize + 18 + mpData[0]) > dataSize )
				return ERR_BAD_FORMAT;

			// Double check image type field
			if ( mpData[1] != 0 )
				return ERR_BAD_FORMAT;

			// Load image data
			res = loadRawData();
			if ( res != OK )
				return res;

			convertBGRtoRGB(); // Convert to RGB

			break;
		}

	case 9: // RLE Indexed
		{
			// Double check image type field
			if ( mpData[1] != 1 )
				return ERR_BAD_FORMAT;

			// Load image data
			res = loadTgaRLEData();
			if ( res != OK )
				return res;

			// Load palette
			res = loadTgaPalette();
			if ( res != OK )
				return res;

			break;
		}

	case 10: // RLE RGB
		{
			// Double check image type field
			if ( mpData[1] != 0 )
				return ERR_BAD_FORMAT;

			// Load image data
			res = loadTgaRLEData();
			if ( res != OK )
				return res;

			convertBGRtoRGB(); // Convert to RGB

			break;
		}

	default:
		return ERR_UNSUPPORTED;
	}

	// Check flip bit
	if ( mpData[17] & 0x20)
		flipImg();

	// Clean up
	mpData = VUNULL;

	return OK;
}

//*****************************************************************************
// Flips the image vertically (Why store images upside down?)
//*****************************************************************************
void VuTgaLoader::flipImg()
{
	int lineLen = mWidth*(mBPP/8);

	VUBYTE *pLine1 = mpImage;
	VUBYTE *pLine2 = &mpImage[lineLen * (mHeight - 1)];

	for( ; pLine1 < pLine2; pLine2 -= (lineLen*2) )
	{
		for ( int index = 0; index < lineLen; pLine1++, pLine2++, index++ )
			VuSwap(*pLine1, *pLine2);
	} 
}

//*****************************************************************************
// Examine the header and populate our class attributes
//*****************************************************************************
VuTgaLoader::eResult VuTgaLoader::readHeader()
{
	if ( mpData[1] > 1 )    // 0 (RGB) and 1 (Indexed) are the only types we know about
	{
		return ERR_UNSUPPORTED;
	}

	mEnc = mpData[2];
	// Encoding flag  1 = Raw indexed image
	//                2 = Raw RGB
	//                3 = Raw greyscale
	//                9 = RLE indexed
	//               10 = RLE RGB
	//               11 = RLE greyscale
	//               32 & 33 Other compression, indexed

	if ( mEnc > 11 ) // We don't want 32 or 33
	{
		return ERR_UNSUPPORTED;
	}

	// Get palette info
	VUINT16 colMapStart, colMapLen;
	VU_MEMCPY(&colMapStart, sizeof(colMapStart), &mpData[3], 2);
	VU_MEMCPY(&colMapLen, sizeof(colMapLen), &mpData[5], 2);
	#if VU_BIG_ENDIAN
		colMapStart = VuEndianUtil::swap(colMapStart);
		colMapLen = VuEndianUtil::swap(colMapLen);
	#endif

	// Reject indexed images if not a VGA palette (256 entries with 24 bits per entry)
	if ( mpData[1] == 1 ) // Indexed
	{
		if ( colMapStart != 0 || colMapLen != 256 || mpData[7] != 24 )
		{
			return ERR_UNSUPPORTED;
		}
	}

	// Get image window and produce width & height values
	VUINT16 x1,y1,x2,y2;
	VU_MEMCPY(&x1, sizeof(x1), &mpData[8], 2);
	VU_MEMCPY(&y1, sizeof(y1), &mpData[10], 2);
	VU_MEMCPY(&x2, sizeof(x2), &mpData[12], 2);
	VU_MEMCPY(&y2, sizeof(y2), &mpData[14], 2);
	#if VU_BIG_ENDIAN
		x1 = VuEndianUtil::swap(x1);
		y1 = VuEndianUtil::swap(y1);
		x2 = VuEndianUtil::swap(x2);
		y2 = VuEndianUtil::swap(y2);
	#endif

	mWidth= x2 - x1;
	mHeight= y2 - y1;

	if ( mWidth < 1 || mHeight < 1 )
		return ERR_BAD_FORMAT;

	// Bits per Pixel
	mBPP = mpData[16];

	// Check flip / interleave byte
	if ( mpData[17] > 32) // Interleaved data
	{
		return ERR_UNSUPPORTED;
	}

	// Calculate image size
	mImageSize = mWidth*mHeight*(mBPP/8);

	return OK;
}

//*****************************************************************************
// Load uncompressed image data
//*****************************************************************************
VuTgaLoader::eResult VuTgaLoader::loadRawData()
{
	mpImage = new VUBYTE[mImageSize];

	if ( mpImage == VUNULL )
		return ERR_MEM_FAIL;

	int offset = mpData[0] + 18; // Add header to ident field size

	if ( mpData[1] ==1 ) // Indexed images
		offset += 768;  // Add palette offset

	VU_MEMCPY(mpImage, mImageSize, &mpData[offset], mImageSize);

	return OK;
}

//*****************************************************************************
// Load RLE compressed image data
//*****************************************************************************
VuTgaLoader::eResult VuTgaLoader::loadTgaRLEData()
{
	// Calculate offset to image data
	int offset = mpData[0] + 18;

	// Add palette offset for indexed images
	if ( mpData[1] == 1 )
		offset+=768; 

	// Get pixel size in bytes
	int pixelSize = mBPP/8;

	// Set our pointer to the beginning of the image data
	const VUBYTE *pCur = &mpData[offset];

	// Allocate space for the image data
	mpImage = new VUBYTE[mImageSize];
	if ( mpImage == VUNULL )
		return ERR_MEM_FAIL;

	// Decode
	int index = 0;
	int length;
	while ( index < mImageSize )
	{
		if ( *pCur & 0x80 ) // Run length chunk (High bit = 1)
		{
			length = *pCur - 127; // Get run length
			pCur++; // Move to pixel data  

			// Repeat the next pixel bLength times
			for ( int loop = 0; loop < length; loop++, index += pixelSize )
				VU_MEMCPY(&mpImage[index], mImageSize - index, pCur, pixelSize);

			pCur += pixelSize; // Move to the next descriptor chunk
		}
		else // Raw chunk
		{
			length = *pCur + 1; // Get run length
			pCur++; // Move to pixel data

			// Write the next bLength pixels directly
			for ( int loop = 0; loop < length; loop++, index += pixelSize, pCur += pixelSize )
				VU_MEMCPY(&mpImage[index], mImageSize - index, pCur, pixelSize);
		}
	}

	return OK;
}

//*****************************************************************************
// Load a 256 color palette
//*****************************************************************************
VuTgaLoader::eResult VuTgaLoader::loadTgaPalette()
{
	// Create space for new palette
	mpPalette = new VUBYTE[768];
	if ( mpPalette == VUNULL )
		return ERR_MEM_FAIL;

	// VGA palette is the 768 bytes following the header
	VU_MEMCPY(mpPalette, 768, &mpData[mpData[0] + 18], 768);

	// Palette entries are BGR ordered so we have to convert to RGB
	for ( int index = 0; index < 768; index += 3 )
		VuSwap(mpPalette[index], mpPalette[index + 2]);

	return OK;
}

//*****************************************************************************
// Convert BGR to RGB (or back again)
//*****************************************************************************
void VuTgaLoader::convertBGRtoRGB()
{
	// Set ptr to start of image
	VUBYTE *pCur = mpImage;

	// Calc number of pixels
	int pixelCount = mWidth*mHeight;

	// Get pixel size in bytes
	int pixelSize = mBPP/8;

	// For each pixel
	for ( int index = 0; index < pixelCount; index++ )
	{
		VuSwap(*pCur, *(pCur + 2));
		pCur += pixelSize;
	}
}

//*****************************************************************************
// Create TGA header
//*****************************************************************************
void VuTgaUtil::createHeader(int bpp, int width, int height, bool flipY, VuArray<VUBYTE> &header)
{
	// create header
	VUUINT16 x2 = (VUUINT16)width;
	VUUINT16 y2 = (VUUINT16)height;

	#if VU_BIG_ENDIAN
		x2 = VuEndianUtil::swap(x2);
		y2 = VuEndianUtil::swap(y2);
	#endif

	header.resize(18);
	memset(&header[0], 0, header.size());
	header[2] = 2;
	VU_MEMCPY(&header[12], 18 - 12, &x2, 2);
	VU_MEMCPY(&header[14], 18 - 14, &y2, 2);
	header[16] = (VUBYTE)bpp;

	if ( flipY )
		header[17] = 0x20;
}
