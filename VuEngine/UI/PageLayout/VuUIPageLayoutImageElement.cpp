//*****************************************************************************
//
//  Copyright (c) 2009-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  UI Page Layout Image Element.
// 
//*****************************************************************************

#include "VuUIPageLayoutImageElement.h"
#include "VuEngine/Assets/VuAssetFactory.h"
#include "VuEngine/Assets/VuTextureAsset.h"
#include "VuEngine/Gfx/VuGfxUtil.h"
#include "VuEngine/HAL/Gfx/VuTexture.h"
#include "VuEngine/Math/VuVector2.h"
#include "VuEngine/Json/VuJsonContainer.h"


//*****************************************************************************
VuUIPageLayoutImageElement::VuUIPageLayoutImageElement(const VuJsonContainer &data):
	mpTextureAsset(VUNULL)
{
	const std::string &textureAssetName = data["Texture"].asString();
	if ( VuAssetFactory::IF()->doesAssetExist<VuTextureAsset>(textureAssetName) )
	{
		mpTextureAsset = VuAssetFactory::IF()->createAsset<VuTextureAsset>(textureAssetName);
	}

	mAlignment = data["Align"].asString();
}

//*****************************************************************************
VuUIPageLayoutImageElement::~VuUIPageLayoutImageElement()
{
	VuAssetFactory::IF()->releaseAsset(mpTextureAsset);
}

//*****************************************************************************
float VuUIPageLayoutImageElement::measureHeight(float width, const VuVector2 &scale)
{
	return mpTextureAsset->getTexture()->getHeight()*scale.mY;
}

//*****************************************************************************
void VuUIPageLayoutImageElement::draw(float depth, const VuRect &rect, float offsetY, float alpha, const VuVector2 &scale)
{
	if ( mpTextureAsset )
	{
		VuVector2 size = VuVector2(mpTextureAsset->getTexture()->getWidth(), mpTextureAsset->getTexture()->getHeight())*scale;

		// calculate unclipped destination rectangle
		VuRect dstRect(rect.mX, rect.mY + offsetY, size.mX, size.mY);

		// handle alignment
		if ( mAlignment == "Right" )
			dstRect.mX = rect.getRight() - size.mX;
		else if ( mAlignment == "Center" )
			dstRect.mX = rect.getCenter().mX - 0.5f*size.mX;

		// clip
		VuRect dstRectOrig = dstRect;
		dstRect = VuRect::intersection(rect, dstRect);

		VuRect srcRect(0,0,1,1);
		srcRect.mWidth = dstRect.mWidth/dstRectOrig.mWidth;
		srcRect.mHeight = dstRect.mHeight/dstRectOrig.mHeight;

		if ( dstRectOrig.getLeft() < rect.getLeft() )
			srcRect.mX = 1.0f - srcRect.mWidth;

		if ( dstRectOrig.getTop() < rect.getTop() )
			srcRect.mY = 1.0f - srcRect.mHeight;

		VuColor color(255,255,255);
		color.mA = (VUUINT8)VuRound(color.mA*alpha);
		VuGfxUtil::IF()->drawTexture2d(depth, mpTextureAsset->getTexture(), color, dstRect, srcRect);
	}
}
