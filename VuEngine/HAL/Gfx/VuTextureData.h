//*****************************************************************************
//
//  Copyright (c) 2010-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  Texture data container
// 
//*****************************************************************************

#pragma once

#include "VuEngine/Containers/VuArray.h"
#include "VuEngine/Gfx/Etc/VuEtc.h"

class VuTgaLoader;
class VuBinaryDataReader;
class VuBinaryDataWriter;


class VuTextureData
{
public:
	VuTextureData();

	enum eFormat { FORMAT_INVALID, FORMAT_RGBA, FORMAT_ARGB, FORMAT_RGB, FORMAT_RG, FORMAT_R, FORMAT_ETC1, FORMAT_DXT1, FORMAT_DXT5, FORMAT_PVRTC_RGB, FORMAT_PVRTC_RGBA, FORMAT_565, FORMAT_5551, FORMAT_4444, FORMAT_VU, FORMAT_UV };
	enum eHints { HINT_ASSUME_TILES = 1<<0, HINT_NORMAL_MAP = 1<<1, };

	struct VuBuildParams
	{
		VuBuildParams() : mHints(0) {}
		VUUINT32			mHints;
		VuEtc::VuPackParams	mEtcParams;
	};

	void	create(int width, int height, eFormat format, bool createMipMaps);
	bool	build(const VuTgaLoader &tgaLoader, eFormat format, bool createMipMaps, const VuBuildParams &buildParams);
	bool	build(const VUBYTE *rgba, int width, int height, eFormat format, bool createMipMaps, const VuBuildParams &buildParams);

	void	buildMipLevels();
	void	flipEndianness();

	void	load(VuBinaryDataReader &reader, int skipMipLevels = 0);
	void	save(VuBinaryDataWriter &writer);

	eFormat	getFormat()		{ return mFormat; }
	int		getWidth()		{ return mWidth; }
	int		getHeight()		{ return mHeight; }
	int		getLevelCount()	{ return mLevelCount; }

	VUBYTE	*getLevelData(int level);
	int		getLevelWidth(int level);
	int		getLevelHeight(int level);
	int		getLevelBlockCount(int level);
	int		getLevelSize(int level);
	int		getLevelPitch(int level);
	int		getTotalSize();

	const VuArray<VUBYTE> &getData() { return mData; }

	// debug
	static void	enableVisualizeMipLevels(bool enable)	{ smVisualizeMipLevels = enable; }
	void		visualizeMipLevels();
	
private:
	bool	buildLevel(int level, const VUBYTE *src, const VuBuildParams &buildParams);
	bool	buildMipLevelRecursive(int level, const float *src, const VuBuildParams &buildParams);

	eFormat			mFormat;
	int				mWidth;
	int				mHeight;
	int				mLevelCount;
	VuArray<VUBYTE>	mData;

	// debug
	static bool		smVisualizeMipLevels;
};