//*****************************************************************************
//
//  Copyright (c) 2011-2015 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  Type conversion from VuGfx to Ps4.
// 
//*****************************************************************************

#pragma once

#include <gnm.h>
#include "VuEngine/HAL/Gfx/VuGfxTypes.h"

using namespace sce;


namespace VuPs4GfxTypes
{

	Gnm::DataFormat					convert(VuGfxFormat format);
	Gnm::CompareFunc				convert(VuGfxCompFunc compFunc);
	Gnm::BlendMultiplier			convert(VuGfxBlendMode blendMode);
	Gnm::PrimitiveSetupCullFaceMode	convert(VuGfxCullMode cullMode);
	Gnm::PrimitiveType			 	convert(VuGfxPrimitiveType primitiveType);

} // namespace VuPs4GfxTypes
