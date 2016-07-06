//*****************************************************************************
//
//  Copyright (c) 2013-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  UI News Image class
// 
//*****************************************************************************

#include "VuEngine/Entities/UI/VuUIImageBaseEntity.h"
#include "VuEngine/Properties/VuBlobProperty.h"
#include "VuEngine/Gfx/GfxSort/VuGfxSort.h"
#include "VuEngine/HAL/Gfx/VuGfx.h"
#include "VuEngine/HAL/Gfx/VuTexture.h"
#include "VuEngine/Gfx/Jpeg/VuJpeg.h"
#include "VuEngine/Util/VuTgaUtil.h"
#include "VuEngine/Util/VuImageUtil.h"


class VuUINewsImageEntity : public VuUIImageBaseEntity
{
	DECLARE_RTTI

public:
	VuUINewsImageEntity();
	~VuUINewsImageEntity();

	virtual void		onPostLoad() { modified(); }

protected:

	VuTexture *getTexture() const { return mpTexture; }
	
	void				clear();
	void				modified();
	bool				loadJpeg();
	bool				loadTga();

	// properties
	VuArray<VUBYTE>		mData;

	VuTexture			*mpTexture;
};

IMPLEMENT_RTTI(VuUINewsImageEntity, VuEntity);
IMPLEMENT_ENTITY_REGISTRATION(VuUINewsImageEntity);


//*****************************************************************************
VuUINewsImageEntity::VuUINewsImageEntity():
	mpTexture(VUNULL)
{
	// properties
	addProperty(new VuBlobProperty("Image File", mData)) -> setWatcher(this, &VuUINewsImageEntity::modified);
}

//*****************************************************************************
VuUINewsImageEntity::~VuUINewsImageEntity()
{
	clear();
}

//*****************************************************************************
void VuUINewsImageEntity::clear()
{
	VuGfxSort::IF()->flush();

	if ( mpTexture )
	{
		mpTexture->removeRef();
		mpTexture = VUNULL;
	}
}

//*****************************************************************************
void VuUINewsImageEntity::modified()
{
	clear();

	if ( mData.size() )
	{
		bool success = false;
		if ( !success ) success = loadJpeg();
		if ( !success ) success = loadTga();
	}
}

//*****************************************************************************
bool VuUINewsImageEntity::loadJpeg()
{
	VuArray<VUBYTE> rgb;
	int width, height;
	if ( !VuJpeg::decompress(mData, rgb, width, height) )
		return false;

	#if defined VUANDROID || defined VUIOS || defined VUBB10
		VuImageUtil::flipVert(rgb, width, height);
	#endif

	VuArray<VUBYTE> data;
	data.resize(width*height*4);

	// rgb -> rgba/argb (hacky... todo: fix for VectorEngine4)
	#if defined VUXBOX360
		VuImageUtil::convertRGBtoBGRA(&rgb.begin(), width, height, &data.begin());
	#else
		VuImageUtil::convertRGBtoRGBA(&rgb.begin(), width, height, &data.begin());
	#endif

	// create texture
	VuTextureState state;
	state.mAddressU = VUGFX_ADDRESS_CLAMP;
	state.mAddressV = VUGFX_ADDRESS_CLAMP;
	state.mMipFilter = VUGFX_TEXF_NONE;
	mpTexture = VuGfx::IF()->createTexture(width, height,  0, VUGFX_FORMAT_A8R8G8B8, state);

	mpTexture->setData(0, &data.begin(), data.size());

	return true;
}

//*****************************************************************************
bool VuUINewsImageEntity::loadTga()
{
	VuTgaLoader tgaLoader;
	if ( tgaLoader.load(&mData[0], mData.size()) != VuTgaLoader::OK )
		return false;

	// convert tga to rgba
	VuArray<VUBYTE> rgba;
	// rgb -> rgba/argb (hacky... todo: fix for VectorEngine4)
	#if defined VUWIN32 || defined VUWINSTORE || defined VUWINPHONE
		if ( !VuImageUtil::convertToBGRA(tgaLoader, rgba) )
			return false;
	#else
		if ( !VuImageUtil::convertToRGBA(tgaLoader, rgba) )
			return false;
	#endif

	#if defined VUWIN32 || defined VUWINSTORE || defined VUWINPHONE
		VuImageUtil::flipVert(rgba, tgaLoader.getWidth(), tgaLoader.getHeight());
	#endif

	// create texture
	VuTextureState state;
	state.mAddressU = VUGFX_ADDRESS_CLAMP;
	state.mAddressV = VUGFX_ADDRESS_CLAMP;
	state.mMipFilter = VUGFX_TEXF_NONE;
	mpTexture = VuGfx::IF()->createTexture(tgaLoader.getWidth(), tgaLoader.getHeight(),  0, VUGFX_FORMAT_A8R8G8B8, state);

	mpTexture->setData(0, &rgba.begin(), rgba.size());

	return true;
}
