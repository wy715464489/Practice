//*****************************************************************************
//
//  Copyright (c) 2007-2008 Ralf Knoesel
//  Copyright (c) 2008-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  Base Texture Asset class
// 
//*****************************************************************************

#pragma once

#include "VuEngine/Assets/VuAsset.h"

class VuBaseTexture;

class VuBaseTextureAsset : public VuAsset
{
public:
	virtual VuBaseTexture *getBaseTexture() const = 0;
};
