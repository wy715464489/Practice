//*****************************************************************************
//
//  Copyright (c) 2007-2008 Ralf Knoesel
//  Copyright (c) 2008-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  UI Image class
// 
//*****************************************************************************

#pragma once

#include "VuEngine/UI/VuUIPropertyUtil.h"
#include "VuEngine/Entities/UI/VuUIImageBaseEntity.h"
#include "VuEngine/Properties/VuAssetProperty.h"

class VuTextureAsset;


class VuUIImageEntity : public VuUIImageBaseEntity
{
	DECLARE_RTTI

public:
	VuUIImageEntity();

protected:

	VuTexture *getTexture() const;

	// properties
	std::string						mTextureAssetName;

	// property references
	VuAssetProperty<VuTextureAsset>	*mpTextureAssetProperty;
};
