//*****************************************************************************
//
//  Copyright (c) 2013-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  DropShadowShader class
// 
//*****************************************************************************

#include "VuDropShadowShader.h"
#include "VuEngine/HAL/Gfx/VuGfx.h"
#include "VuEngine/HAL/Gfx/VuShaderProgram.h"
#include "VuEngine/Assets/VuCompiledShaderAsset.h"
#include "VuEngine/Assets/VuAssetFactory.h"


// flavors
enum { FLAVOR_STATIC, FLAVOR_STATIC_ALPHA_TEST, FLAVOR_ANIMATED, FLAVOR_ANIMATED_ALPHA_TEST, FLAVOR_COUNT };

class VuDropShadowShaderFlavor
{
public:
	VuDropShadowShaderFlavor();
	~VuDropShadowShaderFlavor();

	bool					create(const char *shaderName, bool bAnimated);

	VuCompiledShaderAsset	*mpCompiledShaderAsset;
};


//*****************************************************************************
VuDropShadowShader::VuDropShadowShader():
	mpFlavors(VUNULL)
{
}

//*****************************************************************************
VuDropShadowShader::~VuDropShadowShader()
{
}

//*****************************************************************************
bool VuDropShadowShader::init()
{
	// create flavors
	mpFlavors = new VuDropShadowShaderFlavor[FLAVOR_COUNT];

	if ( !mpFlavors[FLAVOR_STATIC].create("DropShadow/Static", false) )
		return false;

	if ( !mpFlavors[FLAVOR_STATIC_ALPHA_TEST].create("DropShadow/StaticAlphaTest", false) )
		return false;

	if ( !mpFlavors[FLAVOR_ANIMATED].create("DropShadow/Animated", true) )
		return false;

	if ( !mpFlavors[FLAVOR_ANIMATED_ALPHA_TEST].create("DropShadow/AnimatedAlphaTest", false) )
		return false;

	return true;
}

//*****************************************************************************
void VuDropShadowShader::release()
{
	delete[] mpFlavors;
	mpFlavors = VUNULL;
}

//*****************************************************************************
VuShaderProgram *VuDropShadowShader::getShaderProgram(bool bAnimated, bool bAlphaTest) const
{
	return mpFlavors[bAnimated*2 + bAlphaTest].mpCompiledShaderAsset->getShaderProgram();
}

//*****************************************************************************
VuDropShadowShaderFlavor::VuDropShadowShaderFlavor():
	mpCompiledShaderAsset(VUNULL)
{
}

//*****************************************************************************
VuDropShadowShaderFlavor::~VuDropShadowShaderFlavor()
{
	VuAssetFactory::IF()->releaseAsset(mpCompiledShaderAsset);
}

//*****************************************************************************
bool VuDropShadowShaderFlavor::create(const char *shaderName, bool bAnimated)
{
	mpCompiledShaderAsset = VuAssetFactory::IF()->createAsset<VuCompiledShaderAsset>(shaderName);

	return true;
}