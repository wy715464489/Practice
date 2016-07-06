//*****************************************************************************
//
//  Copyright (c) 2007-2008 Ralf Knoesel
//  Copyright (c) 2008-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  Texture Asset class
// 
//*****************************************************************************

#include "VuTextureAsset.h"
#include "VuAssetUtil.h"
#include "VuEngine/Assets/VuAssetFactory.h"
#include "VuEngine/Assets/VuAssetBakery.h"
#include "VuEngine/HAL/Gfx/VuGfx.h"
#include "VuEngine/HAL/Gfx/VuTexture.h"
#include "VuEngine/HAL/File/VuFile.h"
#include "VuEngine/Gfx/VuGfxUtil.h"
#include "VuEngine/Util/VuDataUtil.h"
#include "VuEngine/Util/VuBinaryDataUtil.h"


IMPLEMENT_RTTI(VuTextureAsset, VuAsset);
IMPLEMENT_ASSET_REGISTRATION(VuTextureAsset);


//*****************************************************************************
VuBaseTexture *VuTextureAsset::getBaseTexture() const
{
	return mpTexture;
}

//*****************************************************************************
bool VuTextureAsset::substitute(const VuAsset *pSubstAsset)
{
	// make sure type matches
	if ( !pSubstAsset->isDerivedFrom(msRTTI) )
		return false;

	const VuTextureAsset *pSubstTextureAsset = static_cast<const VuTextureAsset *>(pSubstAsset);

	VuGfxSort::IF()->flush();

	unload();

	if ( (mpTexture = pSubstTextureAsset->getTexture()) != VUNULL)
		mpTexture->addRef();

	return true;
}

//*****************************************************************************
void VuTextureAsset::schema(const VuJsonContainer &creationInfo, VuJsonContainer &schema)
{
	schema["DefaultPath"].putValue("Textures");

	VuAssetUtil::addFileProperty(schema, "File", "tga");

	// Type
	{
		const char *choices[] = { "DEFAULT", "BUMP", "SDF", VUNULL };
		VuAssetUtil::addEnumProperty(schema, "Type", choices, "DEFAULT", "DEFAULT - best compression/fastest speed\nBUMP - intended for normal maps\nSDF - signed distance field");
	}

	// Compression
	{
		const char *dxChoices[] = { "32BIT", "S3TC", VUNULL };
		VuAssetUtil::addEnumProperty(schema, "Format DX", dxChoices, "S3TC", "DirectX Format:\n32BIT - uncompressed\nS3TC - DXT1 for 24 bit, DXT5 for 32 bit");

		const char *iosChoices[] = { "32BIT", "S3TC", "PVRTC", VUNULL };
		VuAssetUtil::addEnumProperty(schema, "Format IOS", iosChoices, "PVRTC", "iOS Format:\n32BIT - uncompressed\nS3TC - DXT1 for 24 bit, DXT5 for 32 bit, decompressed at run-time\nPVRTC - PowerVR texture compression");

		const char *oglesChoices[] = { "32BIT", "ETC1/DXT5", VUNULL };
		VuAssetUtil::addEnumProperty(schema, "Format OGLES", oglesChoices, "ETC1/DXT5", "OpenGL ES Format:\n32BIT - uncompressed\nETC1/DXT5 - ETC1 for 24 bit, DXT5 for 32 bit");

		const char *etcChoices[] = { "LOW", "MEDIUM", "HIGH", VUNULL };
		VuAssetUtil::addEnumProperty(schema, "ETC Quality", etcChoices, "LOW", "LOW - lowest quality, fastest speed\nMEDIUM - medium quality, medium speed\nHIGH - high quality, slow speed");

		VuAssetUtil::addBoolProperty(schema, "ETC Dithering", false, "Enable dithering for ETC compression");
		VuAssetUtil::addBoolProperty(schema, "Scale Low Spec", true, "Scale down on low spec devices");
	}

	// Address U/V Wrapping
	{
		const char *choices[] = { "WRAP", "CLAMP", VUNULL };
		VuAssetUtil::addEnumProperty(schema, "AddressU", choices, "WRAP", "WRAP - tile at every integer junction\nCLAMP - texture coordinates outside the range [0.0, 1.0] are set to the texture color at 0.0 or 1.0, respectively");
		VuAssetUtil::addEnumProperty(schema, "AddressV", choices, "WRAP", "WRAP - tile at every integer junction\nCLAMP - texture coordinates outside the range [0.0, 1.0] are set to the texture color at 0.0 or 1.0, respectively");
	}

	// Mag/Min filter
	{
		const char *choices[] = { "POINT", "LINEAR", "ANISOTROPIC", VUNULL };
		VuAssetUtil::addEnumProperty(schema, "MagFilter", choices, "LINEAR", "POINT - point filtering (nearest texel)\nLINEAR - bilinear interpolation filtering\nANISOTROPIC - anisotropic filtering");
		VuAssetUtil::addEnumProperty(schema, "MinFilter", choices, "LINEAR", "POINT - point filtering (nearest texel)\nLINEAR - bilinear interpolation filtering\nANISOTROPIC - anisotropic filtering");
	}

	// Mip filter
	{
		const char *choices[] = { "NONE", "POINT", "LINEAR", VUNULL };
		VuAssetUtil::addEnumProperty(schema, "MipFilter", choices, "POINT", "NONE - mipmapping disabled\nPOINT - nearest point mipmap filtering\nLINEAR - linear mipmap interpolation");
	}
}

//*****************************************************************************
bool VuTextureAsset::bake(const VuJsonContainer &creationInfo, VuAssetBakeParams &bakeParams)
{
	VuBinaryDataWriter &writer = bakeParams.mWriter;

	const std::string &fileName = creationInfo["File"].asString();

	// determine type
	VuGfxTextureType type = VUGFX_TEXTURE_TYPE_DEFAULT;
	VuDataUtil::getValue(creationInfo["Type"], type);

	// compression
	VuTextureCompression compression;
	VuDataUtil::getValue(creationInfo["Format DX"], compression.mFormatDX);
	VuDataUtil::getValue(creationInfo["Format IOS"], compression.mFormatIOS);
	VuDataUtil::getValue(creationInfo["Format OGLES"], compression.mFormatOGLES);
	VuDataUtil::getValue(creationInfo["ETC Quality"], compression.mEtcParams.mQuality);
	VuDataUtil::getValue(creationInfo["ETC Dithering"], compression.mEtcParams.mDithering);

	bool scaleLowSpec = true;
	VuDataUtil::getValue(creationInfo["Scale Low Spec"], scaleLowSpec);
	writer.writeValue(scaleLowSpec);

	// texture state
	VuTextureState state;
	VuDataUtil::getValue(creationInfo["AddressU"], state.mAddressU);
	VuDataUtil::getValue(creationInfo["AddressV"], state.mAddressV);
	VuDataUtil::getValue(creationInfo["MagFilter"], state.mMagFilter);
	VuDataUtil::getValue(creationInfo["MinFilter"], state.mMinFilter);
	VuDataUtil::getValue(creationInfo["MipFilter"], state.mMipFilter);

	// bake texture
	if ( !VuTexture::bake(bakeParams.mPlatform, VuFile::IF()->getRootPath() + fileName, type, compression, state, writer) )
		return VUWARNING("Unable to bake 2d texture %s.", fileName.c_str());

	return true;
}

//*****************************************************************************
bool VuTextureAsset::load(VuBinaryDataReader &reader)
{
	reader.readValue(mScaleLowSpec);

	int skipLevels = mScaleLowSpec && VuGfxUtil::IF()->getLowTextureLOD() ? 1 : 0;

	mpTexture = VuGfx::IF()->loadTexture(reader, skipLevels);
	if ( !mpTexture )
		return VUWARNING("Unable to load baked 2d texture %s.", getAssetName().c_str());

	return true;
}

//*****************************************************************************
void VuTextureAsset::unload()
{
	if ( mpTexture )
		mpTexture->removeRef();
	mpTexture = VUNULL;
}

//*****************************************************************************
void VuTextureAsset::editorReload()
{
	const VuJsonContainer &creationInfo = VuAssetBakery::IF()->getCreationInfo(VUPLATFORM, VuAssetFactory::IF()->getSku(), VuSys::IF()->getLanguage(), getType(), getAssetName());
	VuAssetBakeParams bakeParams(VUPLATFORM, VuAssetFactory::IF()->getSku(), VuSys::IF()->getLanguage());
	if ( bake(creationInfo, bakeParams) )
	{
		VuBinaryDataReader reader(bakeParams.mData);

		bool scaleLowSpec;
		reader.readValue(scaleLowSpec);

		int skipLevels = scaleLowSpec && VuGfxUtil::IF()->getLowTextureLOD() ? 1 : 0;

		if ( !mpTexture->reload(reader, skipLevels) )
			unload();
	}
}