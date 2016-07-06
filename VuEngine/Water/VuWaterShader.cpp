//*****************************************************************************
//
//  Copyright (c) 2008-2008 Ralf Knoesel
//  Copyright (c) 2008-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  Water shader implementation
// 
//*****************************************************************************

#include "VuWaterShader.h"
#include "VuWaterRenderer.h"
#include "VuWaterTexture.h"
#include "VuWater.h"
#include "VuEngine/Gfx/GfxComposer/VuGfxComposer.h"
#include "VuEngine/Gfx/Camera/VuCamera.h"
#include "VuEngine/Gfx/Light/VuLightManager.h"
#include "VuEngine/Assets/VuAssetFactory.h"
#include "VuEngine/Assets/VuCompiledShaderAsset.h"
#include "VuEngine/Assets/VuTextureAsset.h"
#include "VuEngine/Assets/VuCubeTextureAsset.h"
#include "VuEngine/Gfx/GfxSort/VuGfxSort.h"
#include "VuEngine/Gfx/VuGfxSettings.h"
#include "VuEngine/Gfx/VuGfxUtil.h"
#include "VuEngine/HAL/Gfx/VuGfx.h"
#include "VuEngine/HAL/Gfx/VuShaderProgram.h"
#include "VuEngine/HAL/Gfx/VuVertexDeclaration.h"
#include "VuEngine/HAL/Gfx/VuPipelineState.h"
#include "VuEngine/HAL/Gfx/VuTexture.h"
#include "VuEngine/Math/VuMatrix.h"
#include "VuEngine/Util/VuEndianUtil.h"
#include "VuEngine/Dev/VuDevConfig.h"


// constants

#define NORMAL_TEXTURE_POW 7
#define FRESNEL_TEXTURE_SIZE 128
#define FOAM_TEXTURE_POW 8


// static functions

//*****************************************************************************
template<class T>
T &bufferVal(VuArray<T> &buffer, int pow, int x, int y)
{
	int size = 1 << pow;
	int mask = size - 1;

	return buffer[(((y & mask) << pow) + (x & mask))];
}


//*****************************************************************************
VuWaterShader::VuWaterShader(const VuWaterShaderDesc &desc):
	mDesc(desc),
	mpCompiledShaderAsset(VUNULL),
	mpFoamTextureAsset(VUNULL),
	mpReflectionCubeTextureAsset(VUNULL),
	mpDecalTextureAsset(VUNULL),
	mpMaterial(VUNULL),
	mhSpConstAmbientColor(VUNULL),
	mhSpConstDiffuseColor(VUNULL),
	mhSpConstFoamAmbientColor(VUNULL),
	mhSpConstFoamDiffuseColor(VUNULL),
	mhSpConstFoamTextureSize(VUNULL),
	mhSpConstFoamCenter(VUNULL),
	mhSpConstWaterZ(VUNULL),
	mhSpConstReflectionMapOffset(VUNULL),
	mhSpConstReflectionMapScale(VUNULL),
	mhSpConstFoamToNormalTextureScale(VUNULL),
	mhSpConstDecalAmbientColor(VUNULL),
	mhSpConstDecalDiffuseColor(VUNULL),
	miSampFresnel(-1),
	miSampFoam(-1),
	miSampReflection(-1),
	miSampDecal(-1),
	miSampNormal(-1)
{
	std::string shaderName = mDesc.mShaderNameOverride;
	if ( shaderName.empty() )
	{
		// get water shader asset
		if ( mDesc.mNormalMapEnabled )
		{
			if ( mDesc.mReceiveShadows )
				shaderName = "Water/Shadow/";
			else
				shaderName = "Water/Complex/";
		}
		else
		{
			shaderName = "Water/Simple/";
		}

		if ( mDesc.mFogEnabled )
			shaderName += "Fog";
		if ( mDesc.mProceduralReflection )
			shaderName += "Reflect";
		if ( mDesc.mDecalTextureAsset.length() )
			shaderName += "Decal";

		std::string::iterator iter = shaderName.end() - 1;
		if ( *iter == '/' )
			shaderName += "Basic";
	}

	mpCompiledShaderAsset = VuAssetFactory::IF()->createAsset<VuCompiledShaderAsset>(shaderName.c_str());

	// create vertex declaration
	VuVertexDeclarationParams vdParams;
	{
		VUUINT16 offset = 0;

		vdParams.mElements.push_back(VuVertexDeclarationElement(0, offset, VUGFX_DECL_TYPE_FLOAT3, VUGFX_DECL_USAGE_POSITION, 0));
		offset += 12;
		vdParams.mElements.push_back(VuVertexDeclarationElement(0, offset, VUGFX_DECL_TYPE_SHORT2N, VUGFX_DECL_USAGE_NORMAL, 0));
		offset += 4;
		vdParams.mElements.push_back(VuVertexDeclarationElement(0, offset, VUGFX_DECL_TYPE_UBYTE4N, VUGFX_DECL_USAGE_COLOR, 0));
		offset += 4;
		vdParams.mElements.push_back(VuVertexDeclarationElement(0, offset, VUGFX_DECL_TYPE_UBYTE4N, VUGFX_DECL_USAGE_COLOR, 1));
		offset += 4;
		vdParams.mStreams.push_back(VuVertexDeclarationStream(offset));
	}
	VuVertexDeclaration *pVD = VuGfx::IF()->createVertexDeclaration(vdParams, mpCompiledShaderAsset->getShaderProgram());

	VuPipelineStateParams psParams;
	if ( mDesc.mDiffuseColor.mA < 255 )
		psParams.mAlphaBlendEnabled = true;
	VuPipelineState *pPS = VuGfx::IF()->createPipelineState(mpCompiledShaderAsset->getShaderProgram(), pVD, psParams);

	// create material
	VuGfxSortMaterialDesc materialDesc;
	mpMaterial = VuGfxSort::IF()->createMaterial(pPS, materialDesc);

	// create fresnel texture
	mpFresnelTexture = createFresnelTexture();

	// load foam texture
	if ( VuAssetFactory::IF()->doesAssetExist<VuTextureAsset>(mDesc.mFoamTextureAsset) )
		mpFoamTextureAsset = VuAssetFactory::IF()->createAsset<VuTextureAsset>(mDesc.mFoamTextureAsset);

	// load reflection cube texture
	if ( !mDesc.mProceduralReflection )
		if ( VuAssetFactory::IF()->doesAssetExist<VuCubeTextureAsset>(mDesc.mReflectionCubeTextureAsset) )
			mpReflectionCubeTextureAsset = VuAssetFactory::IF()->createAsset<VuCubeTextureAsset>(mDesc.mReflectionCubeTextureAsset);

	// load decal texture
	if ( VuAssetFactory::IF()->doesAssetExist<VuTextureAsset>(mDesc.mDecalTextureAsset) )
		mpDecalTextureAsset = VuAssetFactory::IF()->createAsset<VuTextureAsset>(mDesc.mDecalTextureAsset);

	// constants/samplers
	{
		VuShaderProgram *pSP = mpMaterial->mpShaderProgram;

		mhSpConstAmbientColor				= pSP->getConstantByName("gAmbientColor");
		mhSpConstDiffuseColor				= pSP->getConstantByName("gDiffuseColor");
		mhSpConstFoamAmbientColor			= pSP->getConstantByName("gFoamAmbientColor");
		mhSpConstFoamDiffuseColor			= pSP->getConstantByName("gFoamDiffuseColor");
		mhSpConstFoamTextureSize			= pSP->getConstantByName("gFoamTextureSize");
		mhSpConstFoamCenter					= pSP->getConstantByName("gFoamCenter");
		mhSpConstWaterZ						= pSP->getConstantByName("gWaterZ");
		mhSpConstReflectionMapOffset		= pSP->getConstantByName("gReflectionMapOffset");
		mhSpConstReflectionMapScale			= pSP->getConstantByName("gReflectionMapScale");
		mhSpConstFoamToNormalTextureScale	= pSP->getConstantByName("gFoamToNormalTextureScale");
		mhSpConstDecalAmbientColor			= pSP->getConstantByName("gDecalAmbientColor");
		mhSpConstDecalDiffuseColor			= pSP->getConstantByName("gDecalDiffuseColor");

		miSampFresnel		= pSP->getSamplerIndexByName("gFresnelTexture");
		miSampFoam			= pSP->getSamplerIndexByName("gFoamTexture");
		miSampReflection	= pSP->getSamplerIndexByName("ReflectionTexture");
		miSampDecal			= pSP->getSamplerIndexByName("DecalTexture");
		miSampNormal		= pSP->getSamplerIndexByName("NormalTexture");
	}

	// clean up
	pPS->removeRef();
	pVD->removeRef();
}

//*****************************************************************************
VuWaterShader::~VuWaterShader()
{
	VuWater::IF()->removeShader(this);

	VuGfxSort::IF()->releaseMaterial(mpMaterial);
	VuAssetFactory::IF()->releaseAsset(mpCompiledShaderAsset);
	VuAssetFactory::IF()->releaseAsset(mpFoamTextureAsset);
	VuAssetFactory::IF()->releaseAsset(mpReflectionCubeTextureAsset);
	VuAssetFactory::IF()->releaseAsset(mpDecalTextureAsset);
	mpFresnelTexture->removeRef();
}

//*****************************************************************************
void VuWaterShader::use(float fWaterZ, const VuMatrix &transform, const VuAabb &aabb) const
{
	//const VuCamera &camera = VuGfxSort::IF()->getRenderCamera();

	VuShaderProgram *pSP = mpMaterial->mpShaderProgram;

	VuWaterTexture *pWaterTexture = VuWater::IF()->renderer()->getWaterTexture();

	if ( mDesc.mOverrideGlobalSettings )
	{
		if ( mhSpConstAmbientColor )		pSP->setConstantColor4(mhSpConstAmbientColor, mDesc.mAmbientColor);
		if ( mhSpConstDiffuseColor )		pSP->setConstantColor4(mhSpConstDiffuseColor, mDesc.mDiffuseColor);
		if ( mhSpConstFoamAmbientColor )	pSP->setConstantColor3(mhSpConstFoamAmbientColor, mDesc.mFoamAmbientColor);
		if ( mhSpConstFoamDiffuseColor )	pSP->setConstantColor3(mhSpConstFoamDiffuseColor, mDesc.mFoamDiffuseColor);
		if ( mhSpConstFoamTextureSize )		pSP->setConstantFloat(mhSpConstFoamTextureSize, mDesc.mFoamTextureSize);
		if ( mhSpConstDecalAmbientColor )	pSP->setConstantColor3(mhSpConstDecalAmbientColor, mDesc.mDecalAmbientColor);
		if ( mhSpConstDecalDiffuseColor )	pSP->setConstantColor3(mhSpConstDecalDiffuseColor, mDesc.mDecalDiffuseColor);
	}
	else
	{
		const VuGfxSettings &settings = VuGfxSort::IF()->getRenderGfxSettings();
		if ( mhSpConstAmbientColor )		pSP->setConstantColor4(mhSpConstAmbientColor, settings.mWaterAmbientColor);
		if ( mhSpConstDiffuseColor )		pSP->setConstantColor4(mhSpConstDiffuseColor, settings.mWaterDiffuseColor);
		if ( mhSpConstFoamAmbientColor )	pSP->setConstantColor3(mhSpConstFoamAmbientColor, settings.mWaterFoamAmbientColor);
		if ( mhSpConstFoamDiffuseColor )	pSP->setConstantColor3(mhSpConstFoamDiffuseColor, settings.mWaterFoamDiffuseColor);
		if ( mhSpConstFoamTextureSize )		pSP->setConstantFloat(mhSpConstFoamTextureSize, settings.mWaterFoamTextureSize);
		if ( mhSpConstDecalAmbientColor )	pSP->setConstantColor3(mhSpConstDecalAmbientColor, settings.mWaterDecalAmbientColor);
		if ( mhSpConstDecalDiffuseColor )	pSP->setConstantColor3(mhSpConstDecalDiffuseColor, settings.mWaterDecalDiffuseColor);
	}

	if ( miSampFresnel >= 0 )
		VuGfx::IF()->setTexture(miSampFresnel, mpFresnelTexture);
	if ( miSampFoam >= 0 )
		VuGfx::IF()->setTexture(miSampFoam, mpFoamTextureAsset ? mpFoamTextureAsset->getTexture() : VUNULL);
	if ( miSampDecal >= 0 )
		VuGfx::IF()->setTexture(miSampDecal, mpDecalTextureAsset ? mpDecalTextureAsset->getTexture() : VUNULL);
	if ( miSampNormal >= 0 )
		VuGfx::IF()->setTexture(miSampNormal, pWaterTexture->getActiveTexture());

	// normal texture scale
	if ( mhSpConstFoamToNormalTextureScale )
		pSP->setConstantFloat(mhSpConstFoamToNormalTextureScale, 1.0f/pWaterTexture->getGfxDesc().mNormalTextureScale);

	// determine foam tex coord extents
	if ( mhSpConstFoamCenter )
	{
		float foamTextureSize = mDesc.mOverrideGlobalSettings ? mDesc.mFoamTextureSize : VuGfxSort::IF()->getRenderGfxSettings().mWaterFoamTextureSize;
		VuVector2 foamCenter(transform.getTrans().mX, transform.getTrans().mY);
		foamCenter /= foamTextureSize;
		foamCenter.mX = VuFloor(foamCenter.mX);
		foamCenter.mY = VuFloor(foamCenter.mY);
		pSP->setConstantVector2(mhSpConstFoamCenter, foamCenter);
	}

	if ( mDesc.mProceduralReflection )
	{
		VuTexture *pReflectionTexture = VuGfxComposer::IF()->getWaterReflectionTexture(VuGfxSort::IF()->getRenderViewport());

		if ( mhSpConstWaterZ )				pSP->setConstantFloat(mhSpConstWaterZ, fWaterZ);
		if ( mhSpConstReflectionMapOffset )	pSP->setConstantVector2(mhSpConstReflectionMapOffset, VuGfxComposer::IF()->getWaterReflectionMapOffset());
		if ( mhSpConstReflectionMapScale )	pSP->setConstantVector2(mhSpConstReflectionMapScale, VuGfxComposer::IF()->getWaterReflectionMapScale());

		VuGfx::IF()->setTexture(miSampReflection, pReflectionTexture);
	}
	else if ( mpReflectionCubeTextureAsset )
	{
		VuCubeTexture *pReflectionCubeTexture = mpReflectionCubeTextureAsset->getTexture();

		VuGfx::IF()->setTexture(miSampReflection, pReflectionCubeTexture);
	}
}

//*****************************************************************************
VuCubeTexture *VuWaterShader::getReflectionCubeTexture() const
{
	if ( mpReflectionCubeTextureAsset )
		return mpReflectionCubeTextureAsset->getTexture();

	return VUNULL;
}

//*****************************************************************************
VuTexture *VuWaterShader::createFresnelTexture()
{
	VuTextureState state;
	state.mAddressU = VUGFX_ADDRESS_CLAMP;
	state.mAddressV = VUGFX_ADDRESS_CLAMP;
	state.mMipFilter = VUGFX_TEXF_NONE;
	VuTexture *pTexture = VuGfx::IF()->createTexture(FRESNEL_TEXTURE_SIZE, 1, 0, VUGFX_FORMAT_LIN_R8, state);

	// build Fresnel lookup table
	float fresnelFactor = mDesc.mFresnelFactor;
	float fresnelMin = mDesc.mFresnelMin;
	float fresnelMax = mDesc.mFresnelMax;

	VUUINT8 aTexture[FRESNEL_TEXTURE_SIZE];
	VUUINT8 *pDst = aTexture;
	for ( int i = 0; i < FRESNEL_TEXTURE_SIZE; i++ )
	{
		float fCosIncidentAngle = (float)i/(FRESNEL_TEXTURE_SIZE-1);
		float fIncidentAngle = VuACos(fCosIncidentAngle);
		fIncidentAngle = VuClamp(fIncidentAngle, 0.0f, VU_PI);
		float fTransmittedAngle = VuASin(VuSin(fIncidentAngle)/1.33f);
		float fSum = fTransmittedAngle + fIncidentAngle;
		float fDiff = fTransmittedAngle - fIncidentAngle;
		float fReflectionCoefficient = 0.5f*(VuSquare(VuSin(fDiff)/VuSin(fSum)) + VuSquare(VuTan(fDiff)/VuTan(fSum)));

		fReflectionCoefficient = (fReflectionCoefficient - fresnelMin)/(fresnelMax - fresnelMin);
		fReflectionCoefficient = VuClamp(fReflectionCoefficient, 0.0f, 1.0f);
		pDst[i] = (VUUINT8)VuRound(fReflectionCoefficient*fresnelFactor*255.0f);
	}

	pTexture->setData(0, aTexture, sizeof(aTexture));

	return pTexture;
}
