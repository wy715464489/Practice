//*****************************************************************************
//
//  Copyright (c) 2008-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  VuFont class
// 
//*****************************************************************************

#include "VuFont.h"
#include "VuEngine/Json/VuJsonContainer.h"
#include "VuEngine/HAL/Gfx/VuGfx.h"
#include "VuEngine/HAL/Gfx/VuTexture.h"
#include "VuEngine/Containers/VuArray.h"
#include "VuEngine/Assets/VuAssetFactory.h"
#include "VuEngine/Assets/VuTextureAsset.h"
#include "VuEngine/Util/VuDataUtil.h"
#include "VuEngine/Util/VuBinaryDataUtil.h"
#include "VuEngine/Util/VuImageUtil.h"
#include "VuEngine/Util/VuUtf8.h"

//*****************************************************************************
VuFont::VuFont():
	mpTexture(VUNULL),
	mAscender(0),
	mDescender(0),
	mMaxRadius(0),
	mUnknownCharacter(0xffff)
{
}

//*****************************************************************************
VuFont::~VuFont()
{
	if ( mpTexture )
		mpTexture->removeRef();

	for ( ImageEntry *pIE = &mImages.begin(); pIE != &mImages.end(); pIE++ )
		VuAssetFactory::IF()->releaseAsset(pIE->mpTextureAsset);
}

//*****************************************************************************
bool VuFont::load(const VuJsonContainer &data)
{
	VuArray<VUBYTE> bakedData;
	VuBinaryDataWriter writer(bakedData);
	if ( !bake(data, writer) )
		return false;

	VuBinaryDataReader reader(bakedData);
	return load(reader);
}

//*****************************************************************************
bool VuFont::bake(const VuJsonContainer &data, VuBinaryDataWriter &writer)
{
	// metrics
	const VuJsonContainer &metricsData = data["Metrics"];
	{
		float ascender = metricsData["Ascender"].asFloat();
		float descender = metricsData["Descender"].asFloat();
		float maxRadius = metricsData["MaxRadius"].asFloat();
		writer.writeValue(ascender);
		writer.writeValue(descender);
		writer.writeValue(maxRadius);
	}

	// characters
	const VuJsonContainer &charactersData = data["Characters"];
	{
		int count = charactersData.size();
		writer.writeValue(count);

		for ( int i = 0; i < count; i++ )
		{
			CharEntry entry;
			entry.load(charactersData[i]);
			entry.serialize(writer);
		}
	}

	// texture
	const VuJsonContainer &textureData = data["Texture"];
	{
		// get data
		int width = textureData["Width"].asInt();
		int height = textureData["Height"].asInt();
		writer.writeValue(width);
		writer.writeValue(height);

		VuArray<VUBYTE> bytes;
		VuDataUtil::getValue(textureData["Data"], bytes);
		writer.writeData(&bytes[0], bytes.size());
	}

	// images
	const VuJsonContainer &imagesData = data["Images"];
	{
		int count = imagesData.size();
		writer.writeValue(count);

		for ( int i = 0; i < count; i++ )
		{
			const std::string &textureAssetName = imagesData[i]["TextureAsset"].asString();
			writer.writeString(textureAssetName);
		}
	}

	return true;
}

//*****************************************************************************
bool VuFont::load(VuBinaryDataReader &reader)
{
	// metrics
	{
		reader.readValue(mAscender);
		reader.readValue(mDescender);
		reader.readValue(mMaxRadius);
	}

	// characters
	{
		reader.readArray(mCharacters);
	}
	
	// texture
	{
		// read size
		int width, height;
		reader.readValue(width);
		reader.readValue(height);

		// read data
		VuArray<VUBYTE> bytes;
		bytes.resize(width*height);
		reader.readData(&bytes[0], bytes.size());

		// create texture
		VuTextureState state;
		mpTexture = VuGfx::IF()->createTexture(width, height,  0, VUGFX_FORMAT_LIN_R8, state);
		if ( !mpTexture )
			return VUWARNING("Unable to create font texture.");

		// copy pixels
		mpTexture->setData(0, &bytes.begin(), bytes.size());

		// build mip chain
		for ( int i = 1; i < mpTexture->getLevelCount(); i++ )
		{
			VuImageUtil::generateMipLevelR(width, height, &bytes.begin(), &bytes.begin());
			width = VuMax(width>>1, 1);
			height = VuMax(height>>1, 1);
			mpTexture->setData(i, &bytes.begin(), width*height);
		}
	}

	// images
	{
		int imagesSize;
		reader.readValue(imagesSize);
		mImages.resize(imagesSize);
		for ( ImageEntry *pIE = &mImages.begin(); pIE != &mImages.end(); pIE++ )
		{
			const char *textureAssetName = reader.readString();
			pIE->mpTextureAsset = VuAssetFactory::IF()->createAsset<VuTextureAsset>(textureAssetName);
			if ( !pIE->mpTextureAsset )
				return VUWARNING("Unable to find font image texture asset '%s'.", textureAssetName);
		}
	}

	// lookup tables
	buildLookupTables();

	return true;
}

//*****************************************************************************
void VuFont::buildLookupTables()
{
	// build character lookup table
	for ( int i = 0; i < (int)mCharacters.size(); i++ )
	{
		VUUINT32 code = mCharacters[i].mCode;
		mCharLookupTable[code] = (VUUINT16)i;
		if ( code == '?' )
			mUnknownCharacter = (VUUINT16)i;
	}
}

//*****************************************************************************
void VuFont::CharEntry::load(const VuJsonContainer &data)
{
	mCode = data["Code"].asInt();

	mSrcL = data["SrcL"].asFloat();
	mSrcR = data["SrcR"].asFloat();
	mSrcT = data["SrcT"].asFloat();
	mSrcB = data["SrcB"].asFloat();

	mDstL = data["DstL"].asFloat();
	mDstR = data["DstR"].asFloat();
	mDstT = data["DstT"].asFloat();
	mDstB = data["DstB"].asFloat();

	mAdvance = data["Advance"].asFloat();

	mImageIndex = (VUUINT16)data["ImageIndex"].asInt();
	mIsImage = data["IsImage"].asBool();

	memset(mPad, 0, sizeof(mPad));
}

//*****************************************************************************
void VuFont::CharEntry::serialize(VuBinaryDataWriter &writer)
{
	writer.writeValue(mCode);
	writer.writeValue(mSrcL);
	writer.writeValue(mSrcR);
	writer.writeValue(mSrcT);
	writer.writeValue(mSrcB);
	writer.writeValue(mDstL);
	writer.writeValue(mDstR);
	writer.writeValue(mDstT);
	writer.writeValue(mDstB);
	writer.writeValue(mAdvance);
	writer.writeValue(mImageIndex);
	writer.writeValue(mIsImage);
	writer.writeData(mPad, sizeof(mPad));
}