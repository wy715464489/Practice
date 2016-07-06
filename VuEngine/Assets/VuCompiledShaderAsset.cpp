//*****************************************************************************
//
//  Copyright (c) 2012-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  Shader Asset class
// 
//*****************************************************************************

#include "VuCompiledShaderAsset.h"
#include "VuAssetUtil.h"
#include "VuEngine/HAL/Gfx/VuGfx.h"
#include "VuEngine/HAL/Gfx/VuShaderProgram.h"
#include "VuEngine/HAL/Gfx/VuVertexDeclaration.h"
#include "VuEngine/HAL/File/VuFile.h"
#include "VuEngine/Json/VuJsonReader.h"
#include "VuEngine/Util/VuBinaryDataUtil.h"


IMPLEMENT_RTTI(VuCompiledShaderAsset, VuAsset);
IMPLEMENT_ASSET_REGISTRATION(VuCompiledShaderAsset);


//*****************************************************************************
void VuCompiledShaderAsset::schema(const VuJsonContainer &creationInfo, VuJsonContainer &schema)
{
	schema["DefaultPath"].putValue("Shaders");

	VuAssetUtil::addFileProperty(schema, "File", "json");
	VuAssetUtil::addStringProperty(schema, "Macros", "");
}

//*****************************************************************************
bool VuCompiledShaderAsset::bake(const VuJsonContainer &creationInfo, VuAssetBakeParams &bakeParams)
{
	VuBinaryDataWriter &writer = bakeParams.mWriter;

	const std::string &fileName = creationInfo["File"].asString();
	const std::string &macroString = creationInfo["Macros"].asString();

	VuJsonReader reader;

	VuJsonContainer shaderData;
	if ( !reader.loadFromFile(shaderData, VuFile::IF()->getRootPath() + fileName) )
		return VUWARNING("Unable to load compiled shader asset %s: %s", fileName.c_str(), reader.getLastError().c_str());

	VuJsonContainer macroData;
	if ( !reader.loadFromString(macroData, macroString) )
		return VUWARNING("Unable to parse macros '%s': %s", macroString.c_str(), reader.getLastError().c_str());

	VuShaderProgram::Macros macros;
	for ( int i = 0; i < macroData.numMembers(); i++ )
	{
		const std::string &key = macroData.getMemberKey(i);
		macros[key] = macroData[key].asString();
	}

	// bake shader program
	if ( !VuShaderProgram::bake(bakeParams.mPlatform, shaderData, VuJsonContainer::null, &macros, writer) )
		return VUWARNING("Unable to bake shader.");

	return true;
}

//*****************************************************************************
bool VuCompiledShaderAsset::load(VuBinaryDataReader &reader)
{
	mpShaderProgram = VuGfx::IF()->loadShaderProgram(reader);

	return true;
}

//*****************************************************************************
void VuCompiledShaderAsset::unload()
{
	if ( mpShaderProgram )
	{
		mpShaderProgram->removeRef();
		mpShaderProgram = VUNULL;
	}
}