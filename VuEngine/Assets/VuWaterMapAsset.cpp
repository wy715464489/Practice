//*****************************************************************************
//
//  Copyright (c) 2011-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  Water Map Asset class
// 
//*****************************************************************************

#include "VuWaterMapAsset.h"
#include "VuAssetUtil.h"
#include "VuEngine/HAL/File/VuFile.h"
#include "VuEngine/Util/VuTgaUtil.h"
#include "VuEngine/Util/VuImageUtil.h"
#include "VuEngine/Util/VuDataUtil.h"
#include "VuEngine/Util/VuBinaryDataUtil.h"


IMPLEMENT_RTTI(VuWaterMapAsset, VuAsset);
IMPLEMENT_ASSET_REGISTRATION(VuWaterMapAsset);


class VuWaterMapAsset::VuClipLevel
{
public:
	VuClipLevel() : mData(0) {};

	void				load(VuBinaryDataReader &reader);
	void				save(VuBinaryDataWriter &writer);

	int					mWidth;
	int					mHeight;
	VuArray<VUUINT8>	mData;
};


//*****************************************************************************
VuWaterMapAsset::VuWaterMapAsset():
	mSFD16(0),
	mClipLevels(0)
{
}

//*****************************************************************************
bool VuWaterMapAsset::isVisible(int level, int x, int y)
{
	if ( level < mClipLevels.size() )
	{
		VuClipLevel *pClipLevel = mClipLevels[level];
		int index = y*pClipLevel->mWidth + x;
		if ( index < pClipLevel->mData.size() )
		{
			return pClipLevel->mData[y*pClipLevel->mWidth + x] > 0;
		}
	}

	return true;
}

//*****************************************************************************
void VuWaterMapAsset::schema(const VuJsonContainer &creationInfo, VuJsonContainer &schema)
{
	schema["DefaultPath"].putValue("WaterMaps");

	VuAssetUtil::addFileProperty(schema, "File", "tga");
}

//*****************************************************************************
bool VuWaterMapAsset::bake(const VuJsonContainer &creationInfo, VuAssetBakeParams &bakeParams)
{
	VuBinaryDataWriter &writer = bakeParams.mWriter;

	const std::string &fileName = creationInfo["File"].asString();

	VuTgaLoader tgaLoader;
	if ( tgaLoader.load(VuFile::IF()->getRootPath() + fileName) != VuTgaLoader::OK )
		return false;

	// convert to rgba
	VuArray<VUBYTE> rgba;
	if ( !VuImageUtil::convertToRGBA(tgaLoader, rgba) )
		return false;

	int width = tgaLoader.getWidth();
	int height = tgaLoader.getHeight();

	writer.writeValue(width);
	writer.writeValue(height);

	// extract 16-bit rgb
	VuArray<VUBYTE> rgb565;
	rgb565.resize(width*height*2);
	VuImageUtil::convertRGBAto565(&rgba[0], width, height, &rgb565[0]);

	// make sure dimensions are a power of 2 + 1
	if ( VuBitCount(width - 1) != 1 || VuBitCount(height - 1) != 1 )
		return VUWARNING("Water maps must be powers of 2 + 1");

	// write SFD (shadow/foam/decal)
	writer.writeData(&rgb565[0], rgb565.size());

	// clip levels (in alpha channel)
	ClipLevels clipLevels;
	if ( tgaLoader.getBPP() == 32 )
	{
		// highest clip level
		VuClipLevel *pClipLevel = new VuClipLevel;
		pClipLevel->mWidth = width - 1;
		pClipLevel->mHeight = height - 1;
		pClipLevel->mData.resize(pClipLevel->mWidth*pClipLevel->mHeight);
		{
			VUUINT8 *pDst = &pClipLevel->mData.begin();
			const VUUINT8 *pSrc = &rgba[0];
			for ( int y = 0; y < pClipLevel->mHeight; y++ )
			{
				for ( int x = 0; x < pClipLevel->mWidth; x++ )
				{
					*pDst = 0;
					if ( pSrc[3] >= 128 && pSrc[4 + 3] >= 128 && pSrc[width*4 + 3] >= 128 && pSrc[(width + 1)*4 + 3] >= 128 )
						*pDst = 0xff;

					pDst++;
					pSrc += 4;
				}
				pSrc += 4;
			}
		}

		clipLevels.push_back(pClipLevel);

		// build lower clip levels
		const VuClipLevel *pPrevLevel = clipLevels.back();
		while ( pPrevLevel->mWidth > 1 && pPrevLevel->mHeight > 1 )
		{
			VuClipLevel *pCurLevel = new VuClipLevel;
			pCurLevel->mWidth = pPrevLevel->mWidth >> 1;
			pCurLevel->mHeight = pPrevLevel->mHeight >> 1;
			pCurLevel->mData.resize(pCurLevel->mWidth*pCurLevel->mHeight);

			VUUINT8 *pDst = &pCurLevel->mData.begin();
			const VUUINT8 *pSrc = &pPrevLevel->mData.begin();
			for ( int y = 0; y < pCurLevel->mHeight; y++ )
			{
				for ( int x = 0; x < pCurLevel->mWidth; x++ )
				{
					*pDst = 0;
					if ( pSrc[0] || pSrc[1] || pSrc[pPrevLevel->mWidth] || pSrc[pPrevLevel->mWidth + 1] )
						*pDst = 0xff;

					pDst++;
					pSrc += 2;
				}
				pSrc += pPrevLevel->mWidth;
			}

			clipLevels.push_back(pCurLevel);
			pPrevLevel = pCurLevel;
		}
	}

	// write out in reverse order
	writer.writeValue(clipLevels.size());
	for ( int i = clipLevels.size() - 1; i >= 0; i-- )
		clipLevels[i]->save(writer);

	for ( int i = 0; i < clipLevels.size(); i++ )
		delete clipLevels[i];

	return true;
}

//*****************************************************************************
bool VuWaterMapAsset::load(VuBinaryDataReader &reader)
{
	reader.readValue(mWidth);
	reader.readValue(mHeight);

	mSFD16.resize(mWidth*mHeight*2);
	reader.readData(&mSFD16[0], mSFD16.size());

	int clipCount;
	reader.readValue(clipCount);
	mClipLevels.resize(clipCount);
	for ( int i = 0; i < clipCount; i++ )
	{
		mClipLevels[i] = new VuClipLevel;
		mClipLevels[i]->load(reader);
	}

	return true;
}

//*****************************************************************************
void VuWaterMapAsset::unload()
{
	mWidth = 0;
	mHeight = 0;
	mSFD16.deallocate();
	for ( int i = 0; i < mClipLevels.size(); i++ )
		delete mClipLevels[i];
	mClipLevels.deallocate();
}

//*****************************************************************************
void VuWaterMapAsset::VuClipLevel::load(VuBinaryDataReader &reader)
{
	reader.readValue(mWidth);
	reader.readValue(mHeight);
	reader.readArray(mData);
}

//*****************************************************************************
void VuWaterMapAsset::VuClipLevel::save(VuBinaryDataWriter &writer)
{
	writer.writeValue(mWidth);
	writer.writeValue(mHeight);
	writer.writeArray(mData);
}
