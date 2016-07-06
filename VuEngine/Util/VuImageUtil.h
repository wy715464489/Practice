//*****************************************************************************
//
//  Copyright (c) 2011-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  Image utility
// 
//*****************************************************************************

#pragma once

#include "VuEngine/Containers/VuArray.h"

class VuTgaLoader;


namespace VuImageUtil
{
	bool convertToRGBA(const VuTgaLoader &tgaLoader, VuArray<VUBYTE> &image);
	bool convertToBGRA(const VuTgaLoader &tgaLoader, VuArray<VUBYTE> &image);
	bool convertToARGB(const VuTgaLoader &tgaLoader, VuArray<VUBYTE> &image);
	bool convertToRGB(const VuTgaLoader &tgaLoader, VuArray<VUBYTE> &image);
	bool convertToR(const VuTgaLoader &tgaLoader, VuArray<VUBYTE> &image);
	bool convertToRGBA4444(const VuTgaLoader &tgaLoader, VuArray<VUBYTE> &image);
	bool convertToRGBA5551(const VuTgaLoader &tgaLoader, VuArray<VUBYTE> &image);
	bool convertToRGB565(const VuTgaLoader &tgaLoader, VuArray<VUBYTE> &image);
	bool convertToDXT1(const VuTgaLoader &tgaLoader, VuArray<VUBYTE> &image);
	bool convertToDXT3(const VuTgaLoader &tgaLoader, VuArray<VUBYTE> &image);
	bool convertToDXT5(const VuTgaLoader &tgaLoader, VuArray<VUBYTE> &image);

	void convertRGBtoRGBA(const VUBYTE *src, int width, int height, VUBYTE *dst);
	void convertRtoRGBA(const VUBYTE *src, int width, int height, VUBYTE *dst);
	void convertARGBtoRGBA(const VUBYTE *src, int width, int height, VUBYTE *dst);
	void convertBGRAtoRGBA(const VUBYTE *src, int width, int height, VUBYTE *dst);

	void convertRGBtoARGB(const VUBYTE *src, int width, int height, VUBYTE *dst);
	void convertRtoARGB(const VUBYTE *src, int width, int height, VUBYTE *dst);

	void convertRGBAtoBGRA(const VUBYTE *src, int width, int height, VUBYTE *dst);
	void convertRGBtoBGRA(const VUBYTE *src, int width, int height, VUBYTE *dst);
	void convertRtoBGRA(const VUBYTE *src, int width, int height, VUBYTE *dst);
	void convertRGBtoBGR(const VUBYTE *src, int width, int height, VUBYTE *dst);

	void convertRGBAtoRGB(const VUBYTE *src, int width, int height, VUBYTE *dst);
	void convertRtoRGB(const VUBYTE *src, int width, int height, VUBYTE *dst);

	void convertRGBAtoRG(const VUBYTE *src, int width, int height, VUBYTE *dst);

	void convertRGBAtoR(const VUBYTE *src, int width, int height, VUBYTE *dst);
	void convertRGBtoR(const VUBYTE *src, int width, int height, VUBYTE *dst);

	void convertRGBAtoA(const VUBYTE *src, int width, int height, VUBYTE *dst);

	void convertRto565(const VUBYTE *src, int width, int height, VUBYTE *dst);
	void convertRGBto565(const VUBYTE *src, int width, int height, VUBYTE *dst);
	void convertRGBAto565(const VUBYTE *src, int width, int height, VUBYTE *dst);
	void convertRGBAto5551(const VUBYTE *src, int width, int height, VUBYTE *dst);
	void convertRGBAto4444(const VUBYTE *src, int width, int height, VUBYTE *dst);

	void convertRGBAtoUV(const VUBYTE *src, int width, int height, VUBYTE *dst);
	void convertRGBAtoVU(const VUBYTE *src, int width, int height, VUBYTE *dst);

	void convertRGBAtoARGB(const VUBYTE *src, int width, int height, VUBYTE *dst);

	void generateMipLevelRGBA(int srcWidth, int srcHeight, const VUBYTE *src, VUBYTE *dst);
	void generateMipLevelRGB(int srcWidth, int srcHeight, const VUBYTE *src, VUBYTE *dst);
	void generateMipLevelRG(int srcWidth, int srcHeight, const VUBYTE *src, VUBYTE *dst);
	void generateMipLevelR(int srcWidth, int srcHeight, const VUBYTE *src, VUBYTE *dst);
	void generateMipLevelFRGBA(int srcWidth, int srcHeight, const float *src, float *dst);

	void endianFlip4(VUBYTE *data, int width, int height);
	void endianFlip2(VUBYTE *data, int width, int height);

	void makeSquare4(const VUBYTE *src, int &width, int &height, VuArray<VUBYTE> &dstImage);

	void flipVert(VuArray<VUBYTE> &image, int width, int height);
	void flipVert(VUBYTE *image, int width, int height, int bpp);
	void swapRB(VUBYTE *image, int pixelCount, int pixelSize);

	void convertRGBAtoFRGBA(const VUBYTE *src, int width, int height, float *dst);
	void convertFRGBAtoRGBA(const float *src, int width, int height, VUBYTE *dst);
}

// color conversion macros
#define VU_EXTRACT_R_FROM_565(packedColor) ((packedColor>>8)&0xf8)
#define VU_EXTRACT_G_FROM_565(packedColor) ((packedColor>>3)&0xfc)
#define VU_EXTRACT_B_FROM_565(packedColor) ((packedColor<<3)&0xf8)
