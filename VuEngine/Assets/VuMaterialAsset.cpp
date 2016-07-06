//*****************************************************************************
//
//  Copyright (c) 2012-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  Material Asset class
// 
//*****************************************************************************

#include "VuMaterialAsset.h"
#include "VuAssetUtil.h"
#include "VuEngine/VuEngine.h"
#include "VuEngine/HAL/File/VuFile.h"
#include "VuEngine/HAL/Gfx/VuPipelineState.h"
#include "VuEngine/Gfx/VuGfxUtil.h"
#include "VuEngine/Gfx/Light/VuLightManager.h"
#include "VuEngine/Gfx/GfxSort/VuGfxSort.h"
#include "VuEngine/Gfx/GfxComposer/VuGfxComposer.h"
#include "VuEngine/Gfx/GfxScene/VuGfxSceneUtil.h"
#include "VuEngine/Gfx/Shaders/VuShadowShader.h"
#include "VuEngine/Gfx/Shaders/VuDropShadowShader.h"
#include "VuEngine/Gfx/Shaders/VuDepthShader.h"
#include "VuEngine/Assets/VuAssetFactory.h"
#include "VuEngine/Assets/VuAssetBakery.h"
#include "VuEngine/Assets/VuTextureAsset.h"
#include "VuEngine/Assets/VuCubeTextureAsset.h"
#include "VuEngine/Json/VuJsonReader.h"
#include "VuEngine/Json/VuJsonBinaryReader.h"
#include "VuEngine/Json/VuJsonBinaryWriter.h"
#include "VuEngine/Math/VuPackedVector.h"
#include "VuEngine/Util/VuBinaryDataUtil.h"
#include "VuEngine/Util/VuDataUtil.h"


IMPLEMENT_RTTI(VuMaterialAsset, VuAsset);
IMPLEMENT_ASSET_REGISTRATION(VuMaterialAsset);

#define MATERIAL_SHADER_LOD_COUNT 2

//*****************************************************************************
VuMaterialAsset::VuMaterialAsset():
	mHasShaderLODs(false),
	mbAlphaTest(false),
	mbSkinning(false),
	mbDoesCastShadows(false),
	mbDoesReceiveShadows(false),
	mbDoesSSAO(false),
	mbSceneLighting(false),
	mbDynamicLighting(false),
	mbDepthSort(false),
	mTranslucencyType(VuGfxSort::TRANS_OPAQUE),
	mpGfxSortShadowMaterial(VUNULL),
	mpGfxSortDropShadowMaterial(VUNULL),
	mpGfxSortSSAODepthMaterial(VUNULL)
{
	for ( int i = 0; i < NUM_FLAVORS; i++ )
		mpGfxSortMaterials[i] = VUNULL;
}

//*****************************************************************************
void VuMaterialAsset::schema(const VuJsonContainer &creationInfo, VuJsonContainer &schema)
{
	schema["DefaultPath"].putValue("Shaders");

	const std::string &defaultFile = VuAssetBakery::IF()->getCreationInfo(VUPLATFORM, VuAssetFactory::IF()->getSku(), VuSys::IF()->getLanguage(), "VuMaterialAsset", "Default")["File"].asString();
	VuAssetUtil::addFileProperty(schema, "File", "json", defaultFile);

	VuAssetUtil::addBoolProperty(schema, "OptimizeVerts", true);

	// shader file name
	std::string shaderFileName = creationInfo["File"].asString();
	if ( !shaderFileName.empty() )
	{
		// load shader data
		VuJsonContainer shaderData;
		if ( loadShaderData(shaderFileName, shaderData) )
		{
			// recursively build schema, depending on which features are enabled
			VuShaderProgram::Macros macros;
			buildSchema(creationInfo, shaderData, schema);
		}
	}
}

//*****************************************************************************
bool VuMaterialAsset::bake(const VuJsonContainer &creationInfo, VuAssetBakeParams &bakeParams)
{
	VuBinaryDataWriter &writer = bakeParams.mWriter;

	const VuJsonContainer &materialData = creationInfo; 

	// shader file name
	std::string shaderFileName = VuAssetBakery::IF()->getCreationInfo(bakeParams.mPlatform, bakeParams.mSku, bakeParams.mLanguage, "VuMaterialAsset", "Default")["File"].asString();
	VuDataUtil::getValue(materialData["File"], shaderFileName);

	// load shader data
	VuJsonContainer shaderData;
	if ( !loadShaderData(shaderFileName, shaderData) )
		return false;

	// recursively build parameter macros, depending on which features are enabled
	VuShaderProgram::Macros macros;
	buildParameterMacros(materialData, shaderData, macros);

	if ( VuEngine::IF()->editorMode() )
		macros["EditorMode"] = "";

	// are there LODs?
	VuJsonContainer lodData;
	lodData[0] = shaderData["LOD1"];
	lodData[1] = shaderData["LOD2"];

	// bake shader program
	VuArray<VUBYTE> shaderProgramData;
	{
		VuBinaryDataWriter shaderProgramWriter(shaderProgramData);
		if ( !VuShaderProgram::bake(bakeParams.mPlatform, shaderData, VuJsonContainer::null, &macros, shaderProgramWriter) )
			return VUWARNING("Unable to bake shader '%s'.", shaderFileName.c_str());
	}

	// bake LODs
	VuArray<VUBYTE> lodProgramData[MATERIAL_SHADER_LOD_COUNT];
	for ( int i = 0; i < MATERIAL_SHADER_LOD_COUNT; i++ )
	{
		if ( lodData[i].isObject() )
		{
			const std::string lodFileName = lodData[i]["Shader"].asString();

			VuJsonContainer lodShaderData;
			if ( !loadShaderData(lodFileName, lodShaderData) )
				return false;

			VuShaderProgram::Macros lodMacros = macros;
			for ( int j = 0; j < lodData[i]["Macros"].size(); j++ )
				lodMacros[lodData[i]["Macros"][j].asString()] = "";

			VuBinaryDataWriter shaderProgramWriter(lodProgramData[i]);
			if ( !VuShaderProgram::bake(bakeParams.mPlatform, lodShaderData, lodData[i], &lodMacros, shaderProgramWriter) )
				return VUWARNING("Unable to bake shader '%s'.", lodFileName.c_str());

			bakeParams.mDependencies.addFile(lodFileName);
		}
	}

	// write shader data
	writer.writeArray(shaderProgramData);
	for ( int i = 0; i < MATERIAL_SHADER_LOD_COUNT; i++ )
		writer.writeArray(lodProgramData[i]);

	// do we cast/receive shadows?
	bool bDoesCastShadows = macros.find("CastShadows") != macros.end();
	bool bDoesReceiveShadows = macros.find("ReceiveShadows") != macros.end();
	bool bDoesSSAO = macros.find("SSAO") != macros.end();
	bool bSceneLighting = macros.find("SceneLighting") != macros.end();
	bool bDynamicLighting = macros.find("DynamicLighting") != macros.end();

	// create material desc(recursively build material description, depending on which features are enabled)
	VuGfxSortMaterialDesc materialDesc;
	std::string errors;
	buildMaterialDesc(materialData, shaderData, materialDesc, errors);

	// do we have a texture used for alpha testing?
	bool bAlphaTest = false;
	const VuGfxSortMaterialDesc::VuTextureArrayEntry *pATT = VUNULL;
	{
		VuShaderProgram::Macros::const_iterator iter = macros.find("AlphaTesting");
		if ( iter != macros.end() )
		{
			pATT = materialDesc.getTextureEntry(iter->second.c_str());
			bAlphaTest = true;
		}
	}

	// create shadow material
	VuGfxSortMaterialDesc shadowMaterialDesc;
	if ( pATT )
	{
		shadowMaterialDesc.addConstantBool("gAlphaTestEnabled", true);
		shadowMaterialDesc.addTexture("OneBitAlphaTexture", pATT->mType, pATT->mAssetName);
	}
	else
	{
		shadowMaterialDesc.addConstantBool("gAlphaTestEnabled", false);
	}

	// translucency type
	bool bDepthSort;
	VUUINT32 translucencyType;
	{
		bool bModulate = false;
		bool bAdditive = false;
		VuShaderProgram::Macros::const_iterator iter = macros.find("TranslucencyType");
		if ( iter != macros.end() )
		{
			bModulate = iter->second == "Modulate";
			bAdditive = iter->second == "Additive";
		}

		bool bAboveWater = true;
		bool bBelowWater = false;
		bool bClipWater = false;
		iter = macros.find("WaterInteraction");
		if ( iter != macros.end() )
		{
			bAboveWater = iter->second == "Above";
			bBelowWater = iter->second == "Below";
			bClipWater = iter->second == "Clip";
		}

		bDepthSort = bModulate;
		translucencyType = VuGfxSort::TRANS_OPAQUE;
		if ( bAlphaTest )
			translucencyType = VuGfxSort::TRANS_ALPHA_TEST;
		if ( bModulate || bAdditive )
		{
			if ( bAboveWater )
				translucencyType = bAdditive ? VuGfxSort::TRANS_ADDITIVE_ABOVE_WATER : VuGfxSort::TRANS_MODULATE_ABOVE_WATER;
			if ( bBelowWater )
				translucencyType = bAdditive ? VuGfxSort::TRANS_ADDITIVE_BELOW_WATER : VuGfxSort::TRANS_MODULATE_BELOW_WATER;
			if ( bClipWater )
				translucencyType = bAdditive ? VuGfxSort::TRANS_ADDITIVE_CLIP_WATER : VuGfxSort::TRANS_MODULATE_CLIP_WATER;
		}
	}

	// are we skinning?
	bool bSkinning = macros.find("Skinning") != macros.end();

	// vertex declaration
	{
		// optimize vertex declaration?
		bool optimizeVerts = true;
		VuDataUtil::getValue(creationInfo["OptimizeVerts"], optimizeVerts);

		VuVertexDeclarationElements elements;
		elements.load(shaderData["VertexDeclaration"]);

		if ( optimizeVerts )
		{
			VuVertexDeclarationElements optElements;
			VuGfxSceneUtil::optimizeVertexDeclaration(bakeParams.mPlatform, bSkinning, elements, optElements);
			optElements.save(writer);
		}
		else
		{
			elements.save(writer);
		}
	}

	// properties
	writer.writeValue(bAlphaTest);
	writer.writeValue(bSkinning);
	writer.writeValue(bDoesCastShadows);
	writer.writeValue(bDoesReceiveShadows);
	writer.writeValue(bDoesSSAO);
	writer.writeValue(bSceneLighting);
	writer.writeValue(bDynamicLighting);
	writer.writeValue(bDepthSort);
	writer.writeValue(translucencyType);
	writer.writeString(errors);

	// materials
	materialDesc.saveParams(writer);
	shadowMaterialDesc.saveParams(writer);

	return true;
}

//*****************************************************************************
bool VuMaterialAsset::loadShaderData(const std::string &shaderFileName, VuJsonContainer &shaderData)
{
	VuJsonReader reader;
	if ( !reader.loadFromFile(shaderData, VuFile::IF()->getRootPath() + shaderFileName) )
		return VUWARNING("Unable to load shader data for shader '%s': %s", shaderFileName.c_str(), reader.getLastError().c_str());

	return true;
}

//*****************************************************************************
void VuMaterialAsset::buildSchema(const VuJsonContainer &materialData, const VuJsonContainer &defParent, VuJsonContainer &schema)
{
	const VuJsonContainer &defParams = defParent["Parameters"];

	// definition defines shader
	for ( int iParam = 0; iParam < defParams.size(); iParam++ )
	{
		const VuJsonContainer &defParam = defParams[iParam];
		const std::string &strType = defParam["Type"].asString();
		const std::string &strName = defParam["Name"].asString();

		const VuJsonContainer &instValue = materialData[strName];

		if ( strType == "Feature" )
		{
			VuAssetUtil::addBoolProperty(schema, strName, defParam["Default"].asBool());

			// if feature is enabled, recurse
			if ( instValue.asBool() )
			{
				// load children
				buildSchema(materialData, defParam, schema);
			}
		}
		else if ( strType == "BoolParameter" )
		{
			VuAssetUtil::addBoolProperty(schema, strName, defParam["Default"].asBool());
		}
		else if ( strType == "EnumParameter" )
		{
			VuAssetUtil::addEnumProperty(schema, strName, defParam["Choices"], defParam["Default"].asString());
		}
		else if ( strType == "FloatParameter" )
		{
			VuAssetUtil::addFloatProperty(schema, strName, defParam["Default"].asFloat());
		}
		else if ( strType == "ColorParameter" )
		{
			VuColor defaultColor(255,255,255);
			VuDataUtil::getValue(defParam["Default"], defaultColor);
			VuAssetUtil::addColorProperty(schema, strName, defaultColor);
		}
		else if ( strType == "Color4Parameter" )
		{
			VuColor defaultColor(255,255,255);
			VuDataUtil::getValue(defParam["Default"], defaultColor);
			VuAssetUtil::addColorProperty(schema, strName, defaultColor);
		}
		else if ( strType == "Texture" )
		{
			VuAssetUtil::addAssetProperty(schema, strName, "VuTextureAsset", "");
		}
		else if ( strType == "CubeTexture" )
		{
			VuAssetUtil::addAssetProperty(schema, strName, "VuCubeTextureAsset", "");
		}
	}
}

//*****************************************************************************
bool VuMaterialAsset::load(VuBinaryDataReader &reader)
{
	// shader data
	int shaderSize;
	reader.readValue(shaderSize);
	const void *shaderData = reader.cur();
	reader.skip(shaderSize);

	// handle LODs
	mHasShaderLODs = false;
	for ( int i = 0; i < MATERIAL_SHADER_LOD_COUNT; i++ )
	{
		int lodSize;
		reader.readValue(lodSize);
		const void *lodData = reader.cur();
		reader.skip(lodSize);

		if ( lodSize && VuGfxUtil::IF()->getShaderLOD() > i )
		{
			shaderSize = lodSize;
			shaderData = lodData;
		}

		if ( lodSize )
			mHasShaderLODs = true;
	}

	// load vertex declaration elements
	VuVertexDeclarationParams vdParams;
	vdParams.mElements.load(reader);
	vdParams.mStreams.push_back(vdParams.mElements.calcVertexSize(0));

	// properties
	reader.readValue(mbAlphaTest);
	reader.readValue(mbSkinning);
	reader.readValue(mbDoesCastShadows);
	reader.readValue(mbDoesReceiveShadows);
	reader.readValue(mbDoesSSAO);
	reader.readValue(mbSceneLighting);
	reader.readValue(mbDynamicLighting);
	reader.readValue(mbDepthSort);
	reader.readValue(mTranslucencyType);
	const char *errors = reader.readString();

	// check for missing textures
	#if !defined VURETAIL
	if ( VuEngine::IF()->gameMode() && errors[0] )
		return VUWARNING(errors);
	#endif

	if ( mbSceneLighting )
	{
		vdParams.mElements.push_back(VuVertexDeclarationElement(1, 0, VUGFX_DECL_TYPE_UBYTE4N, VUGFX_DECL_USAGE_COLOR, 1));
		vdParams.mStreams.push_back(vdParams.mElements.calcVertexSize(1));
	}

	// create materials
	{
		VuBinaryDataReader shaderReader(shaderData, shaderSize);
		mpShaderProgram = VuGfx::IF()->loadShaderProgram(shaderReader);

		VuVertexDeclaration *pVD = VuGfx::IF()->createVertexDeclaration(vdParams, mpShaderProgram);

		VuGfxSortMaterialDesc desc;
		desc.loadParams(reader);

		// opaque material
		{
			VuPipelineStateParams psParams;
			VuPipelineState *pPS = VuGfx::IF()->createPipelineState(mpShaderProgram, pVD, psParams);

			mpGfxSortMaterials[FLV_OPAQUE] = VuGfxSort::IF()->createMaterial(pPS, desc);
			#ifndef VURETAIL
				mpGfxSortMaterials[FLV_OPAQUE]->mDebugName = getAssetName();
			#endif

			pPS->removeRef();
		}

		// modulated material
		{
			VuPipelineStateParams psParams;
			psParams.mAlphaBlendEnabled = true;
			VuPipelineState *pPS = VuGfx::IF()->createPipelineState(mpShaderProgram, pVD, psParams);

			mpGfxSortMaterials[FLV_MODULATED] = VuGfxSort::IF()->createMaterial(pPS, desc);
			#ifndef VURETAIL
				mpGfxSortMaterials[FLV_MODULATED]->mDebugName = getAssetName();
			#endif

			pPS->removeRef();
		}

		// additive material
		{
			VuPipelineStateParams psParams;
			psParams.mAlphaBlendEnabled = true;
			psParams.mSrcBlendMode = VUGFX_BLEND_SRCALPHA;
			psParams.mDstBlendMode = VUGFX_BLEND_ONE;
			VuPipelineState *pPS = VuGfx::IF()->createPipelineState(mpShaderProgram, pVD, psParams);

			mpGfxSortMaterials[FLV_ADDITIVE] = VuGfxSort::IF()->createMaterial(pPS, desc);
			#ifndef VURETAIL
				mpGfxSortMaterials[FLV_ADDITIVE]->mDebugName = getAssetName();
			#endif

			pPS->removeRef();
		}

		// depth pass material
		{
			VuPipelineStateParams psParams;
			psParams.mColorWriteEnabled = false;
			VuPipelineState *pPS = VuGfx::IF()->createPipelineState(mpShaderProgram, pVD, psParams);

			mpGfxSortMaterials[FLV_DEPTH] = VuGfxSort::IF()->createMaterial(pPS, desc);
			#ifndef VURETAIL
				mpGfxSortMaterials[FLV_DEPTH]->mDebugName = getAssetName();
			#endif

			pPS->removeRef();
		}

		pVD->removeRef();
	}

	// create shadow materials
	{
		VuGfxSortMaterialDesc desc;
		desc.loadParams(reader);

		// standard
		{
			mpShadowShaderProgram = VuGfxUtil::IF()->shadowShader()->getShaderProgram(mbSkinning, mbAlphaTest);
			mpShadowShaderProgram->addRef();

			VuVertexDeclaration *pVD = VuGfx::IF()->createVertexDeclaration(vdParams, mpShadowShaderProgram);
			
			VuPipelineStateParams psParams;
			psParams.mDepthRenderingHint = true;
			VuPipelineState *pPS = VuGfx::IF()->createPipelineState(mpShadowShaderProgram, pVD, psParams);

			mpGfxSortShadowMaterial = VuGfxSort::IF()->createMaterial(pPS, desc);
			#ifndef VURETAIL
				mpGfxSortShadowMaterial->mDebugName = getAssetName() + " standard shadow";
			#endif

			pPS->removeRef();
			pVD->removeRef();
		}

		// drop
		{
			mpDropShadowShaderProgram = VuGfxUtil::IF()->dropShadowShader()->getShaderProgram(mbSkinning, mbAlphaTest);
			mpDropShadowShaderProgram->addRef();

			VuVertexDeclaration *pVD = VuGfx::IF()->createVertexDeclaration(vdParams, mpDropShadowShaderProgram);
			VuPipelineState *pPS = VuGfx::IF()->createPipelineState(mpDropShadowShaderProgram, pVD, VuPipelineStateParams());

			mpGfxSortDropShadowMaterial = VuGfxSort::IF()->createMaterial(pPS, desc);
			#ifndef VURETAIL
				mpGfxSortDropShadowMaterial->mDebugName = getAssetName() + " drop shadow";
			#endif

			pPS->removeRef();
			pVD->removeRef();
		}

		// ssao depth
		{
			mpSSAODepthShaderProgram = VuGfxUtil::IF()->depthShader()->getShaderProgram(mbSkinning, mbAlphaTest);
			mpSSAODepthShaderProgram->addRef();

			VuVertexDeclaration *pVD = VuGfx::IF()->createVertexDeclaration(vdParams, mpSSAODepthShaderProgram);
			VuPipelineState *pPS = VuGfx::IF()->createPipelineState(mpSSAODepthShaderProgram, pVD, VuPipelineStateParams());

			mpGfxSortSSAODepthMaterial = VuGfxSort::IF()->createMaterial(pPS, desc);
			#ifndef VURETAIL
				mpGfxSortSSAODepthMaterial->mDebugName = getAssetName() + " drop shadow";
			#endif

			pPS->removeRef();
			pVD->removeRef();
		}
	}

	// resolve constants
	resolveConstants();

	return true;
}

//*****************************************************************************
void VuMaterialAsset::unload()
{
	for ( int i = 0; i < NUM_FLAVORS; i++ )
	{
		if ( mpGfxSortMaterials[i] )
		{
			VuGfxSort::IF()->releaseMaterial(mpGfxSortMaterials[i]);
			mpGfxSortMaterials[i] = VUNULL;
		}
	}

	if ( mpGfxSortShadowMaterial )
	{
		VuGfxSort::IF()->releaseMaterial(mpGfxSortShadowMaterial);
		mpGfxSortShadowMaterial = VUNULL;
	}

	if ( mpGfxSortDropShadowMaterial )
	{
		VuGfxSort::IF()->releaseMaterial(mpGfxSortDropShadowMaterial);
		mpGfxSortDropShadowMaterial = VUNULL;
	}

	if ( mpGfxSortSSAODepthMaterial )
	{
		VuGfxSort::IF()->releaseMaterial(mpGfxSortSSAODepthMaterial);
		mpGfxSortSSAODepthMaterial = VUNULL;
	}

	VU_SAFE_RELEASE(mpShaderProgram);
	VU_SAFE_RELEASE(mpShadowShaderProgram);
	VU_SAFE_RELEASE(mpDropShadowShaderProgram);
	VU_SAFE_RELEASE(mpSSAODepthShaderProgram);
}

//*****************************************************************************
void VuMaterialAsset::resolveConstants()
{
	// get shader constants (all materials use same shader program)
	{
		VuShaderProgram *pSP = mpGfxSortMaterials[0]->mpShaderProgram;

		// handle to model matrix constant
		mConstants.mhModelMatrix = pSP->getConstantByName("gModelMatrix");

		// handle to bone count and matrix array
		mConstants.mhMatrixArray = pSP->getConstantByName("gMatrixArray");

		// handle to color constant
		mConstants.mhColor = pSP->getConstantByName("gColor");

		// handle to water z
		mConstants.mhWaterZ = pSP->getConstantByName("gWaterZ");

		// handle to dynamic light constants
		mConstants.mhDynamicLightColor = pSP->getConstantByName("gDynamicLightColor");
		mConstants.mhDynamicLightDirections = pSP->getConstantByName("gDynamicLightDirections");
		mConstants.mhDynamicLightDiffuseColors = pSP->getConstantByName("gDynamicLightDiffuseColors");
	}

	// shadow
	{
		VuShaderProgram *pSP = mpGfxSortShadowMaterial->mpShaderProgram;
		mShadowConstants.mhMatrix = pSP->getConstantByName("gMatrix");
		mShadowConstants.mhMatrixArray = pSP->getConstantByName("gMatrixArray");
	}

	// drop shadow
	{
		VuShaderProgram *pSP = mpGfxSortDropShadowMaterial->mpShaderProgram;
		mDropShadowConstants.mhMatrix = pSP->getConstantByName("gMatrix");
		mDropShadowConstants.mhMatrixArray = pSP->getConstantByName("gMatrixArray");
	}

	// ssao depth
	{
		VuShaderProgram *pSP = mpGfxSortSSAODepthMaterial->mpShaderProgram;
		mSSAODepthConstants.mhModelMatrix = pSP->getConstantByName("gModelMatrix");
		mSSAODepthConstants.mhMatrixArray = pSP->getConstantByName("gMatrixArray");
	}
}

//*****************************************************************************
void VuMaterialAsset::buildParameterMacros(const VuJsonContainer &materialData, const VuJsonContainer &defParent, VuShaderProgram::Macros &macros)
{
	const VuJsonContainer &defParams = defParent["Parameters"];

	// definition defines shader
	for ( int iParam = 0; iParam < defParams.size(); iParam++ )
	{
		const VuJsonContainer &defParam = defParams[iParam];
		const std::string &strType = defParam["Type"].asString();
		const std::string &strName = defParam["Name"].asString();

		const VuJsonContainer &instValue = materialData[strName];

		if ( strType == "Feature" )
		{
			// if feature is enabled, recurse
			if ( instValue.asBool() )
			{
				// append macro
				macros[strName] = "";

				// load children
				buildParameterMacros(materialData, defParam, macros);
			}
		}
		else if ( strType == "BoolParameter" )
		{
			// determine value from definition, then instance
			bool bValue = false;
			defParam["Default"].getValue(bValue);
			instValue.getValue(bValue);

			// if true, create macro
			if ( bValue )
				macros[strName] = "";
		}
		else if ( strType == "EnumParameter" )
		{
			// determine value from definition, then instance
			std::string strValue;
			defParam["Default"].getValue(strValue);
			instValue.getValue(strValue);

			// convert value from user to internal representation
			strValue = defParam["Choices"][strValue].asString();

			// create macro
			if ( strValue.size() )
				macros[strName] = strValue;
		}
	}
}

//*****************************************************************************
void VuMaterialAsset::buildMaterialDesc(const VuJsonContainer &materialData, const VuJsonContainer &defParent, VuGfxSortMaterialDesc &desc, std::string &errors)
{
	const VuJsonContainer &defParams = defParent["Parameters"];

	// definition defines shader
	for ( int iParam = 0; iParam < defParams.size(); iParam++ )
	{
		const VuJsonContainer &defParam = defParams[iParam];
		const std::string &strType = defParam["Type"].asString();
		const std::string &strName = defParam["Name"].asString();

		const VuJsonContainer &instValue = materialData[strName];

		if ( strType == "Feature" )
		{
			// if feature is enabled, recurse
			if ( instValue.asBool() )
			{
				// build children
				buildMaterialDesc(materialData, defParam, desc, errors);
			}
		}
		else if ( strType == "FloatParameter" )
		{
			// determine value from definition, then instance
			float val = 0;
			defParam["Default"].getValue(val);
			instValue.getValue(val);

			// add constant
			desc.addConstantFloat(strName.c_str(), val);
		}
		else if ( strType == "ColorParameter" )
		{
			// determine value from definition, then instance
			VuColor val(0,0,0);
			VuDataUtil::getValue(defParam["Default"], val);
			VuDataUtil::getValue(instValue, val);

			// add constant
			desc.addConstantColor3(strName.c_str(), val);
		}
		else if ( strType == "Color4Parameter" )
		{
			// determine value from definition, then instance
			VuColor val(0,0,0);
			VuDataUtil::getValue(defParam["Default"], val);
			VuDataUtil::getValue(instValue, val);

			// add constant
			desc.addConstantColor4(strName.c_str(), val);
		}
		else if ( strType == "Texture" || strType == "CubeTexture" )
		{
			// texture asset name
			std::string strTextureAssetName;
			instValue.getValue(strTextureAssetName);

			// create texture asset
			if ( strType == "Texture" )
			{
				if ( VuAssetFactory::IF()->doesAssetExist<VuTextureAsset>(strTextureAssetName) )
				{
					desc.addTexture(strName.c_str(), VuGfxSortMaterialDesc::TEXTURE, strTextureAssetName.c_str());
				}
				else
				{
					char str[256];
					VU_SPRINTF(str, sizeof(str), "Missing Texture: %s\n", strName.c_str());
					errors += str;
				}
			}
			else if ( strType == "CubeTexture" )
			{
				if ( VuAssetFactory::IF()->doesAssetExist<VuCubeTextureAsset>(strTextureAssetName) )
				{
					desc.addTexture(strName.c_str(), VuGfxSortMaterialDesc::CUBE_TEXTURE, strTextureAssetName.c_str());
				}
				else
				{
					char str[256];
					VU_SPRINTF(str, sizeof(str), "Missing Texture: %s\n", strName.c_str());
					errors += str;
				}
			}
		}
	}
}

//*****************************************************************************
void VuMaterialAsset::setModelMatrix(const VuMatrix &mat) const
{
	if ( mConstants.mhModelMatrix )
	{
		mpShaderProgram->setConstantMatrix(mConstants.mhModelMatrix, mat);
	}
}

//*****************************************************************************
void VuMaterialAsset::setMatrixArray(const VuMatrix *pMatrixArray, int boneCount) const
{
	VUASSERT(boneCount <= VUGFX_MAX_BONE_COUNT, "Max bone count exceeded");

	if ( mConstants.mhMatrixArray )
	{
		mpShaderProgram->setConstantMatrixArray(mConstants.mhMatrixArray, pMatrixArray, boneCount, true);
	}
}

//*****************************************************************************
void VuMaterialAsset::setColor(const VuColor &color) const
{
	if ( mConstants.mhColor )
	{
		mpShaderProgram->setConstantColor4(mConstants.mhColor, color);
	}
}

//*****************************************************************************
void VuMaterialAsset::setWaterZ(float waterZ) const
{
	if ( mConstants.mhWaterZ )
	{
		mpShaderProgram->setConstantFloat(mConstants.mhWaterZ, waterZ);
	}
}

//*****************************************************************************
void VuMaterialAsset::setDynamicLightColor(const VuColor &color) const
{
	if ( mConstants.mhDynamicLightColor )
	{
		mpShaderProgram->setConstantColor3(mConstants.mhDynamicLightColor, color);
	}
}

//*****************************************************************************
void VuMaterialAsset::setDynamicLights(const VuMatrix &transform, const VuAabb &aabb, VUUINT32 groupMask) const
{
	// lights
	if ( mConstants.mhDynamicLightDirections && mConstants.mhDynamicLightDiffuseColors )
	{
		VuShaderLights shaderLights;
		VuVector3 point = transform.transform(aabb.getCenter());
		VuLightManager::IF()->getShaderLights(point, groupMask, shaderLights);

		mpShaderProgram->setConstantVector4Array(mConstants.mhDynamicLightDirections, shaderLights.mDirections, VuShaderLights::MAX_DYNAMIC_LIGHT_COUNT);
		mpShaderProgram->setConstantVector4Array(mConstants.mhDynamicLightDiffuseColors, shaderLights.mDiffuseColors, VuShaderLights::MAX_DYNAMIC_LIGHT_COUNT);
	}
}

//*****************************************************************************
void VuMaterialAsset::setShadowMatrix(const VuMatrix &mat) const
{
	mpShadowShaderProgram->setConstantMatrix(mShadowConstants.mhMatrix, mat);
}

//*****************************************************************************
void VuMaterialAsset::setShadowMatrixArray(const VuMatrix *pMatrixArray, int boneCount) const
{
	VUASSERT(boneCount <= VUGFX_MAX_BONE_COUNT, "Max bone count exceeded");

	mpShadowShaderProgram->setConstantMatrixArray(mShadowConstants.mhMatrixArray, pMatrixArray, boneCount, true);
}

//*****************************************************************************
void VuMaterialAsset::setDropShadowMatrix(const VuMatrix &mat) const
{
	mpDropShadowShaderProgram->setConstantMatrix(mDropShadowConstants.mhMatrix, mat);
}

//*****************************************************************************
void VuMaterialAsset::setDropShadowMatrixArray(const VuMatrix *pMatrixArray, int boneCount) const
{
	VUASSERT(boneCount <= VUGFX_MAX_BONE_COUNT, "Max bone count exceeded");

	mpDropShadowShaderProgram->setConstantMatrixArray(mDropShadowConstants.mhMatrixArray, pMatrixArray, boneCount, true);
}

//*****************************************************************************
void VuMaterialAsset::setSSAODepthModelMatrix(const VuMatrix &mat) const
{
	mpSSAODepthShaderProgram->setConstantMatrix(mSSAODepthConstants.mhModelMatrix, mat);
}

//*****************************************************************************
void VuMaterialAsset::setSSAODepthMatrixArray(const VuMatrix *pMatrixArray, int boneCount) const
{
	VUASSERT(boneCount <= VUGFX_MAX_BONE_COUNT, "Max bone count exceeded");

	mpSSAODepthShaderProgram->setConstantMatrixArray(mSSAODepthConstants.mhMatrixArray, pMatrixArray, boneCount, true);
}

//*****************************************************************************
VuMaterialAsset::eFlavor VuMaterialAsset::getFlavor(int translucencyType)
{
	static VuMaterialAsset::eFlavor sMaterialFlavorLookup[] =
	{
		VuMaterialAsset::FLV_OPAQUE,    // TRANS_BEGIN,
		VuMaterialAsset::FLV_OPAQUE,    // TRANS_OPAQUE,
		VuMaterialAsset::FLV_OPAQUE,    // TRANS_ALPHA_TEST,
		VuMaterialAsset::FLV_OPAQUE,    // TRANS_FOLIAGE,
		VuMaterialAsset::FLV_OPAQUE,    // TRANS_SKYBOX,
		VuMaterialAsset::FLV_MODULATED, // TRANS_TIRE_TRACK,
		VuMaterialAsset::FLV_MODULATED, // TRANS_BLOB_SHADOW,
		VuMaterialAsset::FLV_MODULATED, // TRANS_MODULATE_BELOW_WATER,
		VuMaterialAsset::FLV_ADDITIVE,  // TRANS_ADDITIVE_BELOW_WATER,
		VuMaterialAsset::FLV_MODULATED, // TRANS_WATER_COLOR,
		VuMaterialAsset::FLV_DEPTH,     // TRANS_DEPTH_PASS,
		VuMaterialAsset::FLV_MODULATED, // TRANS_COLOR_PASS,
		VuMaterialAsset::FLV_MODULATED, // TRANS_MODULATE_ABOVE_WATER,
		VuMaterialAsset::FLV_ADDITIVE,  // TRANS_ADDITIVE_ABOVE_WATER,
		VuMaterialAsset::FLV_OPAQUE,    // TRANS_WATER_DEPTH,
		VuMaterialAsset::FLV_MODULATED, // TRANS_MODULATE_CLIP_WATER,
		VuMaterialAsset::FLV_ADDITIVE,  // TRANS_ADDITIVE_CLIP_WATER,
		VuMaterialAsset::FLV_OPAQUE,    // TRANS_UI_OPAQUE,
		VuMaterialAsset::FLV_MODULATED, // TRANS_UI_MODULATE,
		VuMaterialAsset::FLV_ADDITIVE,  // TRANS_UI_ADDITIVE,
		VuMaterialAsset::FLV_OPAQUE,    // TRANS_END,
	};
	VU_COMPILE_TIME_ASSERT(sizeof(sMaterialFlavorLookup)/sizeof(sMaterialFlavorLookup[0]) == VuGfxSort::TRANS_END + 1);

	return sMaterialFlavorLookup[translucencyType];
}