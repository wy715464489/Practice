//*****************************************************************************
//
//  Copyright (c) 2010-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  TGA utility.
// 
//*****************************************************************************

#pragma once

#include "VuEngine/Containers/VuArray.h"


#define IMG_OK              0x1
#define IMG_ERR_NO_FILE     0x2
#define IMG_ERR_MEM_FAIL    0x4
#define IMG_ERR_BAD_FORMAT  0x8
#define IMG_ERR_UNSUPPORTED 0x40
 
class VuTgaLoader
{
public:
	VuTgaLoader();
	~VuTgaLoader();

	enum eResult { OK, ERR_NO_FILE, ERR_MEM_FAIL, ERR_BAD_FORMAT, ERR_UNSUPPORTED };
	eResult	load(const std::string &fileName);
	eResult	load(const VUBYTE *pData, int dataSize);

	int				getBPP() const		{ return mBPP; }
	int				getWidth() const	{ return mWidth; }
	int				getHeight() const	{ return mHeight; }
	VUBYTE			*getImage() const	{ return mpImage; }
	VUBYTE			*getPalette() const	{ return mpPalette; }
	void			flipImg();
 
private:
	int				mWidth;
	int				mHeight;
	int				mBPP;
	int				mImageSize;
	int				mEnc;
	VUBYTE			*mpImage;
	VUBYTE			*mpPalette;
	const VUBYTE	*mpData;
   
	eResult			readHeader();
	eResult			loadRawData();
	eResult			loadTgaRLEData();
	eResult			loadTgaPalette();
	void			convertBGRtoRGB();
};


namespace VuTgaUtil
{

	void createHeader(int bpp, int width, int height, bool flipY, VuArray<VUBYTE> &header);

} // namespace VuTgaUtil