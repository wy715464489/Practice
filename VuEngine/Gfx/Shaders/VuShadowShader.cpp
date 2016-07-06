//*****************************************************************************
//
//  Copyright (c) 2008-2008 Ralf Knoesel
//  Copyright (c) 2008-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  SceneShadowShader class
// 
//*****************************************************************************

#include "VuShadowShader.h"
#include "VuEngine/HAL/Gfx/VuGfx.h"
#include "VuEngine/HAL/Gfx/VuShaderProgram.h"
#include "VuEngine/Assets/VuCompiledShaderAsset.h"
#include "VuEngine/Assets/VuAssetFactory.h"


// flavors
enum { FLAVOR_STATIC, FLAVOR_STATIC_ALPHA_TEST, FLAVOR_ANIMATED, FLAVOR_ANIMATED_ALPHA_TEST, FLAVOR_COUNT };

class VuShadowShaderFlavor
{
public:
	VuShadowShaderFlavor();
	~VuShadowShaderFlavor();

	bool					create(const char *shaderName, bool bAnimated);

	VuCompiledShaderAsset	*mpCompiledShaderAsset;
};


//*****************************************************************************
VuShadowShader::VuShadowShader():
	mpFlavors(VUNULL)
{
}

//*****************************************************************************
VuShadowShader::~VuShadowShader()
{
}

//*****************************************************************************
bool VuShadowShader::init()
{
	// create flavors
	mpFlavors = new VuShadowShaderFlavor[FLAVOR_COUNT];

	if ( !mpFlavors[FLAVOR_STATIC].create("Shadow/Static", false) )
		return false;

	if ( !mpFlavors[FLAVOR_STATIC_ALPHA_TEST].create("Shadow/StaticAlphaTest", false) )
		return false;

	if ( !mpFlavors[FLAVOR_ANIMATED].create("Shadow/Animated", true) )
		return false;

	if ( !mpFlavors[FLAVOR_ANIMATED_ALPHA_TEST].create("Shadow/AnimatedAlphaTest", false) )
		return false;

	return true;
}

//*****************************************************************************
void VuShadowShader::release()
{
	delete[] mpFlavors;
	mpFlavors = VUNULL;
}

//*****************************************************************************
VuShaderProgram *VuShadowShader::getShaderProgram(bool bAnimated, bool bAlphaTest) const
{
	return mpFlavors[bAnimated*2 + bAlphaTest].mpCompiledShaderAsset->getShaderProgram();
}

//*****************************************************************************
VuShadowShaderFlavor::VuShadowShaderFlavor():
	mpCompiledShaderAsset(VUNULL)
{
}

//*****************************************************************************
VuShadowShaderFlavor::~VuShadowShaderFlavor()
{
	VuAssetFactory::IF()->releaseAsset(mpCompiledShaderAsset);
}

//*****************************************************************************
bool VuShadowShaderFlavor::create(const char *shaderName, bool bAnimated)
{
	mpCompiledShaderAsset = VuAssetFactory::IF()->createAsset<VuCompiledShaderAsset>(shaderName);

	return true;
}