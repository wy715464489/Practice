//*****************************************************************************
//
//  Copyright (c) 2006-2008 Ralf Knoesel
//  Copyright (c) 2008-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  Texture interface class.
// 
//*****************************************************************************

#pragma once

#include "VuGfxTypes.h"
#include "VuEngine/Objects/VuRefObj.h"
#include "VuEngine/Objects/VuRTTI.h"
#include "VuEngine/Containers/VuArray.h"
#include "VuEngine/Gfx/Etc/VuEtc.h"

class VuBinaryDataReader;
class VuBinaryDataWriter;
class VuTgaLoader;
class VuTextureState;
class VuTextureCompression;
struct VuOglesParams;


class VuBaseTexture : public VuRefObj
{
	DECLARE_RTTI
};

class VuTexture : public VuBaseTexture
{
	DECLARE_RTTI

public:
	VuTexture(int width, int height, int levelCount);

	int					getWidth() const		{ return mWidth; }
	int					getHeight() const		{ return mHeight; }
	int					getLevelCount() const	{ return mLevelCount; }

	virtual void		setData(int level, const void *pData, int size) = 0;
	virtual bool		reload(VuBinaryDataReader &reader, int skipLevels) { return false; }

	static bool			bake(const std::string &platform, const std::string &strFileName, VuGfxTextureType type, const VuTextureCompression &compression, const VuTextureState &state, VuBinaryDataWriter &writer);

protected:
	static bool			bakeOgles(const std::string &platform, const VuTgaLoader &tgaLoader, VuGfxTextureType type, const VuTextureCompression &compression, const VuTextureState &state, VuBinaryDataWriter &writer);
	static bool			bakeOgles(const VuTgaLoader &tgaLoader, const VuOglesParams &params, const VuTextureCompression &compression, const VuTextureState &state, VuBinaryDataWriter &writer);
	static bool			bakeD3d11(const VuTgaLoader &tgaLoader, VuGfxTextureType type, const VuTextureCompression &compression, const VuTextureState &state, VuBinaryDataWriter &writer);
	//static bool			bakePs4(const VuTgaLoader &tgaLoader, VuGfxTextureType type, const VuTextureCompression &compression, const VuTextureState &state, VuBinaryDataWriter &writer);

	int					mWidth;
	int					mHeight;
	int					mLevelCount;
};

class VuCubeTexture : public VuBaseTexture
{
	DECLARE_RTTI

public:
	VuCubeTexture(int edgeLength, int levelCount);

	int					getEdgeLength() const	{ return mEdgeLength; }
	int					getLevelCount() const	{ return mLevelCount; }

	virtual bool		reload(VuBinaryDataReader &reader, int skipLevels) { return false; }

	static bool			bake(const std::string &platform, const std::string &strFileName, VuGfxTextureType type, const VuTextureCompression &compression, const VuTextureState &state, VuBinaryDataWriter &writer);

protected:
	static bool			bakeOgles(const std::string &platform, const VuTgaLoader &tgaLoader, VuGfxTextureType type, const VuTextureCompression &compression, const VuTextureState &state, VuBinaryDataWriter &writer);
	static bool			bakeOgles(const VuTgaLoader &tgaLoader, const VuOglesParams &params, const VuTextureCompression &compression, const VuTextureState &state, VuBinaryDataWriter &writer);
	static bool			bakeD3d11(const VuTgaLoader &tgaLoader, VuGfxTextureType type, const VuTextureCompression &compression, const VuTextureState &state, VuBinaryDataWriter &writer);
	//static bool			bakePs4(const VuTgaLoader &tgaLoader, VuGfxTextureType type, const VuTextureCompression &compression, const VuTextureState &state, VuBinaryDataWriter &writer);

	int					mEdgeLength;
	int					mLevelCount;
};


class VuTextureState
{
public:
	VuTextureState():
		mAddressU(VUGFX_ADDRESS_WRAP),
		mAddressV(VUGFX_ADDRESS_WRAP),
		mMagFilter(VUGFX_TEXF_LINEAR),
		mMinFilter(VUGFX_TEXF_LINEAR),
		mMipFilter(VUGFX_TEXF_POINT)
	{}

	VuGfxTextureAddress		mAddressU;
	VuGfxTextureAddress		mAddressV;
	VuGfxTextureFilterType	mMagFilter;
	VuGfxTextureFilterType	mMinFilter;
	VuGfxTextureFilterType	mMipFilter;

	void	serialize(VuBinaryDataWriter &writer) const;
	void	deserialize(VuBinaryDataReader &reader);
};

class VuTextureCompression
{
public:
	VuTextureCompression():
		mFormatDX(VUGFX_FORMAT_DX_S3TC),
		mFormatIOS(VUGFX_FORMAT_IOS_PVRTC),
		mFormatOGLES(VUGFX_FORMAT_OGLES_ETC1_DXT5)
	{}

	VuGfxFormatDX		mFormatDX;
	VuGfxFormatIOS		mFormatIOS;
	VuGfxFormatOGLES	mFormatOGLES;
	VuEtc::VuPackParams	mEtcParams;
};
