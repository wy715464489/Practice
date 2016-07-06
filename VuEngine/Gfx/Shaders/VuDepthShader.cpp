//*****************************************************************************
//
//  Copyright (c) 2008-2008 Ralf Knoesel
//  Copyright (c) 2008-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  SceneDepthShader class
// 
//*****************************************************************************

#include "VuDepthShader.h"
#include "VuEngine/Gfx/Camera/VuCamera.h"
#include "VuEngine/HAL/Gfx/VuGfx.h"
#include "VuEngine/HAL/Gfx/VuShaderProgram.h"
#include "VuEngine/Assets/VuCompiledShaderAsset.h"
#include "VuEngine/Assets/VuAssetFactory.h"


// flavors
enum { FLAVOR_STATIC, FLAVOR_STATIC_ALPHA_TEST, FLAVOR_ANIMATED, FLAVOR_ANIMATED_ALPHA_TEST, FLAVOR_COUNT };

class VuDepthShaderFlavor
{
public:
	VuDepthShaderFlavor();
	~VuDepthShaderFlavor();

	bool					create(const char *shaderName, bool bAnimated);

	VuCompiledShaderAsset	*mpCompiledShaderAsset;
	VUHANDLE				mhSpConstModelMatrix;
	VUHANDLE				mhSpConstMatrixArray;
};


//*****************************************************************************
VuDepthShader::VuDepthShader():
	mpFlavors(VUNULL)
{
}

//*****************************************************************************
VuDepthShader::~VuDepthShader()
{
}

//*****************************************************************************
bool VuDepthShader::init()
{
	// create flavors
	mpFlavors = new VuDepthShaderFlavor[FLAVOR_COUNT];

	if ( !mpFlavors[FLAVOR_STATIC].create("Depth/Static", false) )
		return false;

	if ( !mpFlavors[FLAVOR_STATIC_ALPHA_TEST].create("Depth/StaticAlphaTest", false) )
		return false;

	if ( !mpFlavors[FLAVOR_ANIMATED].create("Depth/Animated", true) )
		return false;

	if ( !mpFlavors[FLAVOR_ANIMATED_ALPHA_TEST].create("Depth/AnimatedAlphaTest", false) )
		return false;

	return true;
}

//*****************************************************************************
void VuDepthShader::release()
{
	delete[] mpFlavors;
	mpFlavors = VUNULL;
}

//*****************************************************************************
VuShaderProgram *VuDepthShader::getShaderProgram(bool bAnimated, bool bAlphaTest) const
{
	return mpFlavors[bAnimated*2 + bAlphaTest].mpCompiledShaderAsset->getShaderProgram();
}

//*****************************************************************************
VuDepthShaderFlavor::VuDepthShaderFlavor():
	mpCompiledShaderAsset(VUNULL),
	mhSpConstModelMatrix(VUNULL),
	mhSpConstMatrixArray(VUNULL)
{
}

//*****************************************************************************
VuDepthShaderFlavor::~VuDepthShaderFlavor()
{
	VuAssetFactory::IF()->releaseAsset(mpCompiledShaderAsset);
}

//*****************************************************************************
bool VuDepthShaderFlavor::create(const char *shaderName, bool bAnimated)
{
	mpCompiledShaderAsset = VuAssetFactory::IF()->createAsset<VuCompiledShaderAsset>(shaderName);
	VuShaderProgram *pSP = mpCompiledShaderAsset->getShaderProgram();

	// get shader constants
	mhSpConstModelMatrix = pSP->getConstantByName("gModelMatrix");
	mhSpConstMatrixArray = pSP->getConstantByName("gMatrixArray");

	return true;
}