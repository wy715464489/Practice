//*****************************************************************************
//
//  Copyright (c) 2012-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  Asset Util
// 
//*****************************************************************************

#pragma once

class VuJsonContainer;
class VuColor;


namespace VuAssetUtil
{
	void		addFileProperty(VuJsonContainer &schema, const std::string &name, const std::string &extension, const std::string &toolTip = "");
	void		addBoolProperty(VuJsonContainer &schema, const std::string &name, bool defaultValue, const std::string &toolTip = "");
	void		addIntProperty(VuJsonContainer &schema, const std::string &name, int defaultValue, const std::string &toolTip = "");
	void		addFloatProperty(VuJsonContainer &schema, const std::string &name, float defaultValue, const std::string &toolTip = "");
	void		addStringProperty(VuJsonContainer &schema, const std::string &name, const std::string &defaultValue, const std::string &toolTip = "");
	void		addColorProperty(VuJsonContainer &schema, const std::string &name, const VuColor &defaultValue, const std::string &toolTip = "");
	void		addEnumProperty(VuJsonContainer &schema, const std::string &name, const char **choices, const std::string &defaultValue, const std::string &toolTip = "");
	void		addEnumProperty(VuJsonContainer &schema, const std::string &name, const VuJsonContainer &choices, const std::string &defaultValue, const std::string &toolTip = "");
	void		addAssetProperty(VuJsonContainer &schema, const std::string &name, const std::string &assetType, const std::string &defaultValue, const std::string &toolTip = "");

	void		addMaterialAssignments(VuJsonContainer &schema, const std::string &fileName, const std::string &defaultMaterial);
}
