//*****************************************************************************
//
//  Copyright (c) 2011-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  Image utility
// 
//*****************************************************************************

#include "VuImageUtil.h"
#include "VuTgaUtil.h"
#include "VuEndianUtil.h"
#include "VuEngine/Gfx/Dxt/VuDxt.h"
#include "VuEngine/Math/VuMath.h"
#include "VuEngine/Memory/VuScratchPad.h"


//*****************************************************************************
static bool VerifyTga(const VuTgaLoader &tgaLoader)
{
	if ( tgaLoader.getPalette() )
		return VUWARNING("Paletted texture loading not supported.");

	if ( tgaLoader.getBPP() != 8 && tgaLoader.getBPP() != 24 && tgaLoader.getBPP() != 32 )
		return VUWARNING("Source BPP not supported.");

	return true;
}

//*****************************************************************************
bool VuImageUtil::convertToRGBA(const VuTgaLoader &tgaLoader, VuArray<VUBYTE> &image)
{
	if ( !VerifyTga(tgaLoader) )
		return false;

	int width = tgaLoader.getWidth();
	int height = tgaLoader.getHeight();
	const VUBYTE *src = tgaLoader.getImage();
	image.resize(width*height*4);

	if ( tgaLoader.getBPP() == 8 )
		convertRtoRGBA(src, width, height, &image[0]);
	else if ( tgaLoader.getBPP() == 24 )
		convertRGBtoRGBA(src, width, height, &image[0]);
	else if ( tgaLoader.getBPP() == 32 )
		VU_MEMCPY(&image[0], image.size(), src, image.size());

	return true;
}

//*****************************************************************************
bool VuImageUtil::convertToBGRA(const VuTgaLoader &tgaLoader, VuArray<VUBYTE> &image)
{
	if ( !VerifyTga(tgaLoader) )
		return false;

	int width = tgaLoader.getWidth();
	int height = tgaLoader.getHeight();
	const VUBYTE *src = tgaLoader.getImage();
	image.resize(width*height*4);

	if ( tgaLoader.getBPP() == 8 )
		convertRtoBGRA(src, width, height, &image[0]);
	else if ( tgaLoader.getBPP() == 24 )
		convertRGBtoBGRA(src, width, height, &image[0]);
	else if ( tgaLoader.getBPP() == 32 )
		convertRGBAtoBGRA(src, width, height, &image[0]);

	return true;
}

//*****************************************************************************
bool VuImageUtil::convertToARGB(const VuTgaLoader &tgaLoader, VuArray<VUBYTE> &image)
{
	if ( !VerifyTga(tgaLoader) )
		return false;

	int width = tgaLoader.getWidth();
	int height = tgaLoader.getHeight();
	const VUBYTE *src = tgaLoader.getImage();
	image.resize(width*height*4);

	if ( tgaLoader.getBPP() == 8 )
		convertRtoARGB(src, width, height, &image[0]);
	else if ( tgaLoader.getBPP() == 24 )
		convertRGBtoARGB(src, width, height, &image[0]);
	else if ( tgaLoader.getBPP() == 32 )
		convertRGBAtoARGB(src, width, height, &image[0]);

	return true;
}

//*****************************************************************************
bool VuImageUtil::convertToRGB(const VuTgaLoader &tgaLoader, VuArray<VUBYTE> &image)
{
	if ( !VerifyTga(tgaLoader) )
		return false;

	int width = tgaLoader.getWidth();
	int height = tgaLoader.getHeight();
	const VUBYTE *src = tgaLoader.getImage();
	image.resize(width*height*3);

	if ( tgaLoader.getBPP() == 8 )
		convertRtoRGB(src, width, height, &image[0]);
	else if ( tgaLoader.getBPP() == 24 )
		VU_MEMCPY(&image[0], image.size(), src, image.size());
	else if ( tgaLoader.getBPP() == 32 )
		convertRGBAtoRGB(src, width, height, &image[0]);
	
	return true;
}

//*****************************************************************************
bool VuImageUtil::convertToR(const VuTgaLoader &tgaLoader, VuArray<VUBYTE> &image)
{
	if ( !VerifyTga(tgaLoader) )
		return false;

	int width = tgaLoader.getWidth();
	int height = tgaLoader.getHeight();
	const VUBYTE *src = tgaLoader.getImage();
	image.resize(width*height);

	if ( tgaLoader.getBPP() == 8 )
		VU_MEMCPY(&image[0], image.size(), src, image.size());
	else if ( tgaLoader.getBPP() == 24 )
		convertRGBtoR(src, width, height, &image[0]);
	else if ( tgaLoader.getBPP() == 32 )
		convertRGBAtoR(src, width, height, &image[0]);
	
	return true;
}

//*****************************************************************************
bool VuImageUtil::convertToRGBA4444(const VuTgaLoader &tgaLoader, VuArray<VUBYTE> &image)
{
	VuArray<VUBYTE> source;
	if ( !convertToRGBA(tgaLoader, source) )
		return false;

	int imageSize = tgaLoader.getWidth()*tgaLoader.getHeight();
	image.resize(imageSize*2);
	const VUBYTE *pSrc = &source[0];
	VUUINT16 *pDst = reinterpret_cast<VUUINT16 *>(&image[0]);
	for ( int i = 0; i < imageSize; i++ )
	{
		int r = pSrc[0]>>4;
		int g = pSrc[1]>>4;
		int b = pSrc[2]>>4;
		int a = pSrc[3]>>4;
		pDst[0] = (VUUINT16)((r<<12) | (g<<8) | (b<<4) | (a));
		pSrc += 4;
		pDst += 1;
	}

	return true;
}

//*****************************************************************************
bool VuImageUtil::convertToRGBA5551(const VuTgaLoader &tgaLoader, VuArray<VUBYTE> &image)
{
	VuArray<VUBYTE> source;
	if ( !convertToRGBA(tgaLoader, source) )
		return false;

	int imageSize = tgaLoader.getWidth()*tgaLoader.getHeight();
	image.resize(imageSize*2);
	const VUBYTE *pSrc = &source[0];
	VUUINT16 *pDst = reinterpret_cast<VUUINT16 *>(&image[0]);
	for ( int i = 0; i < imageSize; i++ )
	{
		int r = pSrc[0]>>3;
		int g = pSrc[1]>>3;
		int b = pSrc[2]>>3;
		int a = pSrc[3]>>7;
		pDst[0] = (VUUINT16)((r<<11) | (g<<6) | (b<<1) | (a));
		pSrc += 4;
		pDst += 1;
	}

	return true;
}

//*****************************************************************************
bool VuImageUtil::convertToRGB565(const VuTgaLoader &tgaLoader, VuArray<VUBYTE> &image)
{
	if ( !VerifyTga(tgaLoader) )
		return false;

	int width = tgaLoader.getWidth();
	int height = tgaLoader.getHeight();
	const VUBYTE *src = tgaLoader.getImage();
	image.resize(width*height*2);

	if ( tgaLoader.getBPP() == 8 )
		convertRto565(src, width, height, &image[0]);
	else if ( tgaLoader.getBPP() == 24 )
		convertRGBto565(src, width, height, &image[0]);
	else if ( tgaLoader.getBPP() == 32 )
		convertRGBAto565(src, width, height, &image[0]);

	return true;
}

//*****************************************************************************
bool VuImageUtil::convertToDXT1(const VuTgaLoader &tgaLoader, VuArray<VUBYTE> &image)
{
	VuArray<VUBYTE> source;
	if ( !convertToRGBA(tgaLoader, source) )
		return false;

	image.resize(VuDxt::getStorageRequirements(tgaLoader.getWidth(), tgaLoader.getHeight(), VuDxt::DXT1));
	VuDxt::compressImage(&source[0], tgaLoader.getWidth(), tgaLoader.getHeight(), &image[0], VuDxt::DXT1);

	return true;
}

//*****************************************************************************
bool VuImageUtil::convertToDXT3(const VuTgaLoader &tgaLoader, VuArray<VUBYTE> &image)
{
	VuArray<VUBYTE> source;
	if ( !convertToRGBA(tgaLoader, source) )
		return false;

	image.resize(VuDxt::getStorageRequirements(tgaLoader.getWidth(), tgaLoader.getHeight(), VuDxt::DXT3));
	VuDxt::compressImage(&source[0], tgaLoader.getWidth(), tgaLoader.getHeight(), &image[0], VuDxt::DXT3);

	return true;
}

//*****************************************************************************
bool VuImageUtil::convertToDXT5(const VuTgaLoader &tgaLoader, VuArray<VUBYTE> &image)
{
	VuArray<VUBYTE> source;
	if ( !convertToRGBA(tgaLoader, source) )
		return false;

	image.resize(VuDxt::getStorageRequirements(tgaLoader.getWidth(), tgaLoader.getHeight(), VuDxt::DXT5));
	VuDxt::compressImage(&source[0], tgaLoader.getWidth(), tgaLoader.getHeight(), &image[0], VuDxt::DXT5);

	return true;
}

//*****************************************************************************
void buildMipChainRGBA(VuArray<VUBYTE> &image)
{
}

//*****************************************************************************
void VuImageUtil::convertRGBtoRGBA(const VUBYTE *src, int width, int height, VUBYTE *dst)
{
	int imageSize = width*height;
	for ( int i = 0; i < imageSize; i++ )
	{
		dst[0] = src[0];
		dst[1] = src[1];
		dst[2] = src[2];
		dst[3] = 255;
		src += 3;
		dst += 4;
	}
}

//*****************************************************************************
void VuImageUtil::convertRtoRGBA(const VUBYTE *src, int width, int height, VUBYTE *dst)
{
	int imageSize = width*height;
	for ( int i = 0; i < imageSize; i++ )
	{
		dst[0] = src[0];
		dst[1] = src[0];
		dst[2] = src[0];
		dst[3] = 255;
		src += 1;
		dst += 4;
	}
}

//*****************************************************************************
void VuImageUtil::convertARGBtoRGBA(const VUBYTE *src, int width, int height, VUBYTE *dst)
{
	VUBYTE r, g, b, a;

	int imageSize = width*height;
	for ( int i = 0; i < imageSize; i++ )
	{
		r = src[1];
		g = src[2];
		b = src[3];
		a = src[0];

		dst[0] = r;
		dst[1] = g;
		dst[2] = b;
		dst[3] = a;

		src += 4;
		dst += 4;
	}
}

//*****************************************************************************
void VuImageUtil::convertBGRAtoRGBA(const VUBYTE *src, int width, int height, VUBYTE *dst)
{
	VUBYTE r, g, b, a;

	int imageSize = width*height;
	for ( int i = 0; i < imageSize; i++ )
	{
		r = src[2];
		g = src[1];
		b = src[0];
		a = src[3];

		dst[0] = r;
		dst[1] = g;
		dst[2] = b;
		dst[3] = a;

		src += 4;
		dst += 4;
	}
}

//*****************************************************************************
void VuImageUtil::convertRGBtoARGB(const VUBYTE *src, int width, int height, VUBYTE *dst)
{
	int imageSize = width*height;
	for ( int i = 0; i < imageSize; i++ )
	{
		dst[0] = 255;
		dst[1] = src[0];
		dst[2] = src[1];
		dst[3] = src[2];
		src += 3;
		dst += 4;
	}
}

//*****************************************************************************
void VuImageUtil::convertRtoARGB(const VUBYTE *src, int width, int height, VUBYTE *dst)
{
	int imageSize = width*height;
	for ( int i = 0; i < imageSize; i++ )
	{
		dst[0] = 255;
		dst[1] = src[0];
		dst[2] = src[0];
		dst[3] = src[0];
		src += 1;
		dst += 4;
	}
}

//*****************************************************************************
void VuImageUtil::convertRGBAtoBGRA(const VUBYTE *src, int width, int height, VUBYTE *dst)
{
	int imageSize = width*height;
	for ( int i = 0; i < imageSize; i++ )
	{
		dst[0] = src[2];
		dst[1] = src[1];
		dst[2] = src[0];
		dst[3] = src[3];
		src += 4;
		dst += 4;
	}
}

//*****************************************************************************
void VuImageUtil::convertRGBtoBGRA(const VUBYTE *src, int width, int height, VUBYTE *dst)
{
	int imageSize = width*height;
	for ( int i = 0; i < imageSize; i++ )
	{
		dst[0] = src[2];
		dst[1] = src[1];
		dst[2] = src[0];
		dst[3] = 255;
		src += 3;
		dst += 4;
	}
}

//*****************************************************************************
void VuImageUtil::convertRtoBGRA(const VUBYTE *src, int width, int height, VUBYTE *dst)
{
	int imageSize = width*height;
	for ( int i = 0; i < imageSize; i++ )
	{
		dst[0] = src[0];
		dst[1] = src[0];
		dst[2] = src[0];
		dst[3] = 255;
		src += 1;
		dst += 4;
	}
}

//*****************************************************************************
void VuImageUtil::convertRGBtoBGR(const VUBYTE *src, int width, int height, VUBYTE *dst)
{
	int imageSize = width*height;
	for ( int i = 0; i < imageSize; i++ )
	{
		dst[0] = src[2];
		dst[1] = src[1];
		dst[2] = src[0];
		src += 3;
		dst += 3;
	}
}

//*****************************************************************************
void VuImageUtil::convertRGBAtoRGB(const VUBYTE *src, int width, int height, VUBYTE *dst)
{
	int imageSize = width*height;
	for ( int i = 0; i < imageSize; i++ )
	{
		dst[0] = src[0];
		dst[1] = src[1];
		dst[2] = src[2];
		src += 4;
		dst += 3;
	}
}

//*****************************************************************************
void VuImageUtil::convertRtoRGB(const VUBYTE *src, int width, int height, VUBYTE *dst)
{
	int imageSize = width*height;
	for ( int i = 0; i < imageSize; i++ )
	{
		dst[0] = src[0];
		dst[1] = src[0];
		dst[2] = src[0];
		src += 1;
		dst += 3;
	}
}

//*****************************************************************************
void VuImageUtil::convertRGBAtoRG(const VUBYTE *src, int width, int height, VUBYTE *dst)
{
	int imageSize = width*height;
	for ( int i = 0; i < imageSize; i++ )
	{
		dst[0] = src[0];
		dst[1] = src[1];
		src += 4;
		dst += 2;
	}
}

//*****************************************************************************
void VuImageUtil::convertRGBAtoR(const VUBYTE *src, int width, int height, VUBYTE *dst)
{
	int imageSize = width*height;
	for ( int i = 0; i < imageSize; i++ )
	{
		dst[0] = src[0];
		src += 4;
		dst += 1;
	}
}

//*****************************************************************************
void VuImageUtil::convertRGBtoR(const VUBYTE *src, int width, int height, VUBYTE *dst)
{
	int imageSize = width*height;
	for ( int i = 0; i < imageSize; i++ )
	{
		dst[0] = src[0];
		src += 3;
		dst += 1;
	}
}

//*****************************************************************************
void VuImageUtil::convertRGBAtoA(const VUBYTE *src, int width, int height, VUBYTE *dst)
{
	int imageSize = width*height;
	for ( int i = 0; i < imageSize; i++ )
	{
		dst[0] = src[3];
		src += 4;
		dst += 1;
	}
}

//*****************************************************************************
void VuImageUtil::convertRto565(const VUBYTE *src, int width, int height, VUBYTE *dst)
{
	int size = width*height;
	for ( int i = 0; i < size; i++ )
	{
		int r = src[0];
		int g = 0;
		int b = 0;

		VUUINT16 data = ((r&0xf8) << 8) | ((g&0xfc) << 3) | ((b&0xf8) >> 3);

		dst[0] = (data&0xff);
		dst[1] = (data&0xff00) >> 8;

		src += 1;
		dst += 2;
	}
}

//*****************************************************************************
void VuImageUtil::convertRGBto565(const VUBYTE *src, int width, int height, VUBYTE *dst)
{
	int size = width*height;
	for ( int i = 0; i < size; i++ )
	{
		int r = src[0];
		int g = src[1];
		int b = src[2];

		VUUINT16 data = ((r&0xf8) << 8) | ((g&0xfc) << 3) | ((b&0xf8) >> 3);

		dst[0] = (data&0xff);
		dst[1] = (data&0xff00) >> 8;

		src += 3;
		dst += 2;
	}
}

//*****************************************************************************
void VuImageUtil::convertRGBAto565(const VUBYTE *src, int width, int height, VUBYTE *dst)
{
	int size = width*height;
	for ( int i = 0; i < size; i++ )
	{
		int r = src[0];
		int g = src[1];
		int b = src[2];

		VUUINT16 data = ((r&0xf8) << 8) | ((g&0xfc) << 3) | ((b&0xf8) >> 3);

		dst[0] = (data&0xff);
		dst[1] = (data&0xff00) >> 8;

		src += 4;
		dst += 2;
	}
}

//*****************************************************************************
void VuImageUtil::convertRGBAto5551(const VUBYTE *src, int width, int height, VUBYTE *dst)
{
	int size = width*height;
	for ( int i = 0; i < size; i++ )
	{
		int r = src[0];
		int g = src[1];
		int b = src[2];
		int a = src[3];

		VUUINT16 data = ((r&0xf8) << 8) | ((g&0xf8) << 3) | ((b&0xf8) >> 2) | ((a&0x80) >> 7);

		dst[0] = (data&0xff);
		dst[1] = (data&0xff00) >> 8;

		src += 4;
		dst += 2;
	}
}

//*****************************************************************************
void VuImageUtil::convertRGBAto4444(const VUBYTE *src, int width, int height, VUBYTE *dst)
{
	int size = width*height;
	for ( int i = 0; i < size; i++ )
	{
		int r = src[0];
		int g = src[1];
		int b = src[2];
		int a = src[3];

		VUUINT16 data = ((r&0xf0) << 8) | ((g&0xf0) << 4) | (b&0xf0) | ((a&0xf0) >> 4);

		dst[0] = (data&0xff);
		dst[1] = (data&0xff00) >> 8;

		src += 4;
		dst += 2;
	}
}

//*****************************************************************************
void VuImageUtil::convertRGBAtoUV(const VUBYTE *src, int width, int height, VUBYTE *dst)
{
	VUINT8 *dstSigned = (VUINT8 *)dst;

	int imageSize = width*height;
	for ( int i = 0; i < imageSize; i++ )
	{
		dstSigned[0] = (int)src[0] - 128; // U = R
		dstSigned[1] = (int)src[1] - 128; // V = G
		src += 4;
		dstSigned += 2;
	}
}

//*****************************************************************************
void VuImageUtil::convertRGBAtoVU(const VUBYTE *src, int width, int height, VUBYTE *dst)
{
	VUINT8 *dstSigned = (VUINT8 *)dst;

	int imageSize = width*height;
	for ( int i = 0; i < imageSize; i++ )
	{
		dstSigned[0] = (int)src[1] - 128; // V = G
		dstSigned[1] = (int)src[0] - 128; // U = R
		src += 4;
		dstSigned += 2;
	}
}

//*****************************************************************************
void VuImageUtil::convertRGBAtoARGB(const VUBYTE *src, int width, int height, VUBYTE *dst)
{
	int imageSize = width*height;
	for ( int i = 0; i < imageSize; i++ )
	{
		dst[0] = src[3];
		dst[1] = src[0];
		dst[2] = src[1];
		dst[3] = src[2];
		src += 4;
		dst += 4;
	}
}

//*****************************************************************************
void VuImageUtil::generateMipLevelRGBA(int srcWidth, int srcHeight, const VUBYTE *src, VUBYTE *dst)
{
	int dstWidth = VuMax(srcWidth>>1, 1);
	int dstHeight = VuMax(srcHeight>>1, 1);

	#define READ_R(x, y) ((int)src[((y*srcWidth + x)*4) + 0])
	#define READ_G(x, y) ((int)src[((y*srcWidth + x)*4) + 1])
	#define READ_B(x, y) ((int)src[((y*srcWidth + x)*4) + 2])
	#define READ_A(x, y) ((int)src[((y*srcWidth + x)*4) + 3])

	for ( int dsty = 0; dsty < dstHeight; dsty++ )
	{
		int srcy = dsty*2;
		int srcy0 = VuMin(srcy + 0, srcHeight - 1);
		int srcy1 = VuMin(srcy + 1, srcHeight - 1);

		for ( int dstx = 0; dstx < dstWidth; dstx++ )
		{
			int srcx = dstx*2;
			int srcx0 = VuMin(srcx + 0, srcWidth - 1);
			int srcx1 = VuMin(srcx + 1, srcWidth - 1);

			int r = (READ_R(srcx0, srcy0) + READ_R(srcx1, srcy0) + READ_R(srcx0, srcy1) + READ_R(srcx1, srcy1))>>2;
			int g = (READ_G(srcx0, srcy0) + READ_G(srcx1, srcy0) + READ_G(srcx0, srcy1) + READ_G(srcx1, srcy1))>>2;
			int b = (READ_B(srcx0, srcy0) + READ_B(srcx1, srcy0) + READ_B(srcx0, srcy1) + READ_B(srcx1, srcy1))>>2;
			int a = (READ_A(srcx0, srcy0) + READ_A(srcx1, srcy0) + READ_A(srcx0, srcy1) + READ_A(srcx1, srcy1))>>2;

			dst[0] = (VUBYTE)r;
			dst[1] = (VUBYTE)g;
			dst[2] = (VUBYTE)b;
			dst[3] = (VUBYTE)a;
			dst += 4;
		}
	}

	#undef READ_R
	#undef READ_G
	#undef READ_B
	#undef READ_A
}

//*****************************************************************************
void VuImageUtil::generateMipLevelRGB(int srcWidth, int srcHeight, const VUBYTE *src, VUBYTE *dst)
{
	int dstWidth = VuMax(srcWidth>>1, 1);
	int dstHeight = VuMax(srcHeight>>1, 1);

	#define READ_R(x, y) ((int)src[((y*srcWidth + x)*3) + 0])
	#define READ_G(x, y) ((int)src[((y*srcWidth + x)*3) + 1])
	#define READ_B(x, y) ((int)src[((y*srcWidth + x)*3) + 2])

	for ( int dsty = 0; dsty < dstHeight; dsty++ )
	{
		int srcy = dsty*2;
		int srcy0 = VuMin(srcy + 0, srcHeight - 1);
		int srcy1 = VuMin(srcy + 1, srcHeight - 1);

		for ( int dstx = 0; dstx < dstWidth; dstx++ )
		{
			int srcx = dstx*2;
			int srcx0 = VuMin(srcx + 0, srcWidth - 1);
			int srcx1 = VuMin(srcx + 1, srcWidth - 1);

			int r = (READ_R(srcx0, srcy0) + READ_R(srcx1, srcy0) + READ_R(srcx0, srcy1) + READ_R(srcx1, srcy1))>>2;
			int g = (READ_G(srcx0, srcy0) + READ_G(srcx1, srcy0) + READ_G(srcx0, srcy1) + READ_G(srcx1, srcy1))>>2;
			int b = (READ_B(srcx0, srcy0) + READ_B(srcx1, srcy0) + READ_B(srcx0, srcy1) + READ_B(srcx1, srcy1))>>2;

			dst[0] = (VUBYTE)r;
			dst[1] = (VUBYTE)g;
			dst[2] = (VUBYTE)b;
			dst += 3;
		}
	}

	#undef READ_R
	#undef READ_G
	#undef READ_B
}

//*****************************************************************************
void VuImageUtil::generateMipLevelRG(int srcWidth, int srcHeight, const VUBYTE *src, VUBYTE *dst)
{
	int dstWidth = VuMax(srcWidth>>1, 1);
	int dstHeight = VuMax(srcHeight>>1, 1);

	#define READ_R(x, y) ((int)src[((y*srcWidth + x)*2) + 0])
	#define READ_G(x, y) ((int)src[((y*srcWidth + x)*2) + 1])

	for ( int dsty = 0; dsty < dstHeight; dsty++ )
	{
		int srcy = dsty*2;
		int srcy0 = VuMin(srcy + 0, srcHeight - 1);
		int srcy1 = VuMin(srcy + 1, srcHeight - 1);

		for ( int dstx = 0; dstx < dstWidth; dstx++ )
		{
			int srcx = dstx*2;
			int srcx0 = VuMin(srcx + 0, srcWidth - 1);
			int srcx1 = VuMin(srcx + 1, srcWidth - 1);

			int r = (READ_R(srcx0, srcy0) + READ_R(srcx1, srcy0) + READ_R(srcx0, srcy1) + READ_R(srcx1, srcy1))>>2;
			int g = (READ_G(srcx0, srcy0) + READ_G(srcx1, srcy0) + READ_G(srcx0, srcy1) + READ_G(srcx1, srcy1))>>2;

			dst[0] = (VUBYTE)r;
			dst[1] = (VUBYTE)g;
			dst += 2;
		}
	}

	#undef READ_R
	#undef READ_G
}

//*****************************************************************************
void VuImageUtil::generateMipLevelR(int srcWidth, int srcHeight, const VUBYTE *src, VUBYTE *dst)
{
	int dstWidth = VuMax(srcWidth>>1, 1);
	int dstHeight = VuMax(srcHeight>>1, 1);

	#define READ_R(x, y) ((int)src[((y*srcWidth + x)*1) + 0])

	for ( int dsty = 0; dsty < dstHeight; dsty++ )
	{
		int srcy = dsty*2;
		int srcy0 = VuMin(srcy + 0, srcHeight - 1);
		int srcy1 = VuMin(srcy + 1, srcHeight - 1);

		for ( int dstx = 0; dstx < dstWidth; dstx++ )
		{
			int srcx = dstx*2;
			int srcx0 = VuMin(srcx + 0, srcWidth - 1);
			int srcx1 = VuMin(srcx + 1, srcWidth - 1);

			int r = (READ_R(srcx0, srcy0) + READ_R(srcx1, srcy0) + READ_R(srcx0, srcy1) + READ_R(srcx1, srcy1))>>2;

			dst[0] = (VUBYTE)r;
			dst += 1;
		}
	}

	#undef READ_R
}

//*****************************************************************************
void VuImageUtil::generateMipLevelFRGBA(int srcWidth, int srcHeight, const float *src, float *dst)
{
	int dstWidth = VuMax(srcWidth>>1, 1);
	int dstHeight = VuMax(srcHeight>>1, 1);

	#define READ_R(x, y) (src[((y*srcWidth + x)*4) + 0])
	#define READ_G(x, y) (src[((y*srcWidth + x)*4) + 1])
	#define READ_B(x, y) (src[((y*srcWidth + x)*4) + 2])
	#define READ_A(x, y) (src[((y*srcWidth + x)*4) + 3])

	for ( int dsty = 0; dsty < dstHeight; dsty++ )
	{
		int srcy = dsty*2;
		int srcy0 = VuMin(srcy + 0, srcHeight - 1);
		int srcy1 = VuMin(srcy + 1, srcHeight - 1);

		for ( int dstx = 0; dstx < dstWidth; dstx++ )
		{
			int srcx = dstx*2;
			int srcx0 = VuMin(srcx + 0, srcWidth - 1);
			int srcx1 = VuMin(srcx + 1, srcWidth - 1);

			float r = (READ_R(srcx0, srcy0) + READ_R(srcx1, srcy0) + READ_R(srcx0, srcy1) + READ_R(srcx1, srcy1))*0.25f;
			float g = (READ_G(srcx0, srcy0) + READ_G(srcx1, srcy0) + READ_G(srcx0, srcy1) + READ_G(srcx1, srcy1))*0.25f;
			float b = (READ_B(srcx0, srcy0) + READ_B(srcx1, srcy0) + READ_B(srcx0, srcy1) + READ_B(srcx1, srcy1))*0.25f;
			float a = (READ_A(srcx0, srcy0) + READ_A(srcx1, srcy0) + READ_A(srcx0, srcy1) + READ_A(srcx1, srcy1))*0.25f;

			dst[0] = r;
			dst[1] = g;
			dst[2] = b;
			dst[3] = a;
			dst += 4;
		}
	}

	#undef READ_R
	#undef READ_G
	#undef READ_B
	#undef READ_A
}

//*****************************************************************************
void VuImageUtil::endianFlip4(VUBYTE *data, int width, int height)
{
	VUUINT32 *p = (VUUINT32 *)data;
	int imageSize = width*height;
	for ( int i = 0; i < imageSize; i++ )
	{
		VuEndianUtil::swapInPlace(*p);
		p++;
	}
}

//*****************************************************************************
void VuImageUtil::endianFlip2(VUBYTE *data, int width, int height)
{
	VUUINT16 *p = (VUUINT16 *)data;
	int imageSize = width*height;
	for ( int i = 0; i < imageSize; i++ )
	{
		VuEndianUtil::swapInPlace(*p);
		p++;
	}
}

//*****************************************************************************
void VuImageUtil::makeSquare4(const VUBYTE *src, int &width, int &height, VuArray<VUBYTE> &dstImage)
{
	VUASSERT(VuBitCount(width) == 1 && VuBitCount(height) == 1, "VuImageUtil::makeSquare() only supports power-of-2 image sizes.");

	if ( width == height )
	{
		dstImage.resize(width*height);
		VU_MEMCPY(&dstImage[0], dstImage.size(), src, width*height);
	}
	else
	{
		int srcWidth = width;
		int srcHeight = height;
		int dstLen = VuMin(width, height);
		int scale = VuMax(width, height)/dstLen;
	
		dstImage.resize(dstLen*dstLen*4);
		VUBYTE *dst = &dstImage[0];

		for ( int y = 0; y < dstLen; y++ )
		{
			for ( int x = 0; x < dstLen; x++ )
			{
				int c0 = 0, c1 = 0, c2 = 0, c3 = 0;
				for ( int i = 0; i < scale; i++ )
				{
					const VUBYTE *texel;
					if ( srcWidth > srcHeight )
						texel = &src[(srcWidth*y + (x*scale + i))*4];
					else
						texel = &src[(srcWidth*(y*scale + i) + x)*4];
					c0 += texel[0];
					c1 += texel[1];
					c2 += texel[2];
					c3 += texel[3];
				}
				dst[0] = (VUBYTE)((c0 + (scale>>1))/scale);
				dst[1] = (VUBYTE)((c1 + (scale>>1))/scale);
				dst[2] = (VUBYTE)((c2 + (scale>>1))/scale);
				dst[3] = (VUBYTE)((c3 + (scale>>1))/scale);
				dst += 4;
			}
		}

		width = dstLen;
		height = dstLen;
	}
}

//*****************************************************************************
void VuImageUtil::flipVert(VuArray<VUBYTE> &image, int width, int height)
{
	int bpp = image.size()/(height*width);
	flipVert(&image[0], width, height, bpp);
}

//*****************************************************************************
void VuImageUtil::flipVert(VUBYTE *image, int width, int height, int bpp)
{
	int pitch = width*bpp;

	for ( int y = 0; y < height/2; y++ )
	{
		VU_MEMCPY(VuScratchPad::get(), VuScratchPad::SIZE, &image[y*pitch], pitch);
		VU_MEMCPY(&image[y*pitch], pitch, &image[(height - y - 1)*pitch], pitch);
		VU_MEMCPY(&image[(height - y - 1)*pitch], pitch, VuScratchPad::get(), pitch);
	}
}

//*****************************************************************************
void VuImageUtil::swapRB(VUBYTE *image, int pixelCount, int pixelSize)
{
	for ( int i = 0; i < pixelCount; i++ )
	{
		VuSwap(*image, *(image + 2));
		image += pixelSize;
	}
}

//*****************************************************************************
void VuImageUtil::convertRGBAtoFRGBA(const VUBYTE *src, int width, int height, float *dst)
{
	int imageSize = width*height;
	for ( int i = 0; i < imageSize; i++ )
	{
		dst[0] = src[0]*(1.0f/255.0f);
		dst[1] = src[1]*(1.0f/255.0f);
		dst[2] = src[2]*(1.0f/255.0f);
		dst[3] = src[3]*(1.0f/255.0f);
		src += 4;
		dst += 4;
	}
}

//*****************************************************************************
void VuImageUtil::convertFRGBAtoRGBA(const float *src, int width, int height, VUBYTE *dst)
{
	int imageSize = width*height;
	for ( int i = 0; i < imageSize; i++ )
	{
		dst[0] = (VUBYTE)VuRound(src[0]*255.0f);
		dst[1] = (VUBYTE)VuRound(src[1]*255.0f);
		dst[2] = (VUBYTE)VuRound(src[2]*255.0f);
		dst[3] = (VUBYTE)VuRound(src[3]*255.0f);
		src += 4;
		dst += 4;
	}
}
