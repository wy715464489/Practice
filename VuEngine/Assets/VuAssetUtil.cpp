//*****************************************************************************
//
//  Copyright (c) 2012-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  Asset Util
// 
//*****************************************************************************

#include "VuAssetUtil.h"
#include "VuEngine/Assets/VuAssetFactory.h"
#include "VuEngine/Gfx/GfxScene/VuGfxSceneUtil.h"
#include "VuEngine/Json/VuJsonContainer.h"
#include "VuEngine/Json/VuJsonReader.h"
#include "VuEngine/Util/VuDataUtil.h"


//*****************************************************************************
void VuAssetUtil::addFileProperty(VuJsonContainer &schema, const std::string &name, const std::string &extension, const std::string &toolTip)
{
	VuJsonContainer &prop = schema["Properties"].append();

	prop["Type"].putValue("File");
	prop["Name"].putValue(name);
	prop["Extension"].putValue(extension);
	prop["ToolTip"].putValue(toolTip);
}

//*****************************************************************************
void VuAssetUtil::addBoolProperty(VuJsonContainer &schema, const std::string &name, bool defaultValue, const std::string &toolTip)
{
	VuJsonContainer &prop = schema["Properties"].append();

	prop["Type"].putValue("Bool");
	prop["Name"].putValue(name);
	prop["Default"].putValue(defaultValue);
	prop["ToolTip"].putValue(toolTip);
}

//*****************************************************************************
void VuAssetUtil::addIntProperty(VuJsonContainer &schema, const std::string &name, int defaultValue, const std::string &toolTip)
{
	VuJsonContainer &prop = schema["Properties"].append();

	prop["Type"].putValue("Int");
	prop["Name"].putValue(name);
	prop["Default"].putValue(defaultValue);
	prop["ToolTip"].putValue(toolTip);
}

//*****************************************************************************
void VuAssetUtil::addFloatProperty(VuJsonContainer &schema, const std::string &name, float defaultValue, const std::string &toolTip)
{
	VuJsonContainer &prop = schema["Properties"].append();

	prop["Type"].putValue("Float");
	prop["Name"].putValue(name);
	prop["Default"].putValue(defaultValue);
	prop["ToolTip"].putValue(toolTip);
}

//*****************************************************************************
void VuAssetUtil::addStringProperty(VuJsonContainer &schema, const std::string &name, const std::string &defaultValue, const std::string &toolTip)
{
	VuJsonContainer &prop = schema["Properties"].append();

	prop["Type"].putValue("String");
	prop["Name"].putValue(name);
	prop["Default"].putValue(defaultValue);
	prop["ToolTip"].putValue(toolTip);
}

//*****************************************************************************
void VuAssetUtil::addColorProperty(VuJsonContainer &schema, const std::string &name, const VuColor &defaultValue, const std::string &toolTip)
{
	VuJsonContainer &prop = schema["Properties"].append();

	prop["Type"].putValue("Color");
	prop["Name"].putValue(name);
	VuDataUtil::putValue(prop["Default"], defaultValue);
	prop["ToolTip"].putValue(toolTip);
}

//*****************************************************************************
void VuAssetUtil::addEnumProperty(VuJsonContainer &schema, const std::string &name, const char **choices, const std::string &defaultValue, const std::string &toolTip)
{
	VuJsonContainer &prop = schema["Properties"].append();

	prop["Type"].putValue("Enum");
	prop["Name"].putValue(name);
	prop["Default"].putValue(defaultValue);
	prop["ToolTip"].putValue(toolTip);

	for ( int i = 0; choices[i]; i++ )
		prop["Choices"].append().putValue(choices[i]);
}

//*****************************************************************************
void VuAssetUtil::addEnumProperty(VuJsonContainer &schema, const std::string &name, const VuJsonContainer &choices, const std::string &defaultValue, const std::string &toolTip)
{
	VuJsonContainer &prop = schema["Properties"].append();

	prop["Type"].putValue("Enum");
	prop["Name"].putValue(name);
	prop["Default"].putValue(defaultValue);
	prop["ToolTip"].putValue(toolTip);

	if ( choices.isArray() )
	{
		for ( int i = 0; i < choices.size(); i++ )
			prop["Choices"].append().putValue(choices[i].asString());
	}
	else if ( choices.isObject() )
	{
		for ( int i = 0; i < choices.numMembers(); i++ )
			prop["Choices"].append().putValue(choices.getMemberKey(i));
	}
}

//*****************************************************************************
void VuAssetUtil::addAssetProperty(VuJsonContainer &schema, const std::string &name, const std::string &assetType, const std::string &defaultValue, const std::string &toolTip)
{
	VuJsonContainer &prop = schema["Properties"].append();

	prop["Type"].putValue("Asset");
	prop["Name"].putValue(name);
	prop["Default"].putValue(defaultValue);
	prop["ToolTip"].putValue(toolTip);

	const VuAssetFactory::AssetNames &assetNames = VuAssetFactory::IF()->getAssetNames(assetType);
	for ( int i = 0; i < (int)assetNames.size(); i++ )
		prop["Choices"].append().putValue(assetNames[i]);
}

//*****************************************************************************
void VuAssetUtil::addMaterialAssignments(VuJsonContainer &schema, const std::string &fileName, const std::string &defaultMaterial)
{
	VuJsonContainer data;
	VuJsonReader reader;
	if ( reader.loadFromFile(data, fileName) )
	{
		const VuJsonContainer *pScene = &VuJsonContainer::null;

		if ( data.hasMember("VuGfxScene") )
			pScene = &data["VuGfxScene"];
		else if ( data.hasMember("VuAnimatedModel") )
			pScene = &data["VuAnimatedModel"];

		VuJsonContainer materials;
		VuGfxSceneUtil::cleanUpMaterials(VuJsonContainer::null, *pScene, materials, defaultMaterial);
		for ( int i = 0; i < materials.size(); i++ )
		{
			const VuJsonContainer &material = materials[i];

			addAssetProperty(schema, material["Name"].asString(), "VuMaterialAsset", defaultMaterial);
		}
	}
}
