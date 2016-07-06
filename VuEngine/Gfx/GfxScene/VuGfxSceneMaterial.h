//*****************************************************************************
//
//  Copyright (c) 2012-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  SceneMaterial class
// 
//*****************************************************************************

#pragma once

#include "VuEngine/Objects/VuRefObj.h"
#include "VuEngine/Assets/VuMaterialAsset.h"

class VuJsonContainer;
class VuGfxSceneBakeState;
class VuBinaryDataReader;
class VuBinaryDataWriter;
class VuAssetDependencies;

class VuGfxSceneMaterial : public VuRefObj
{
protected:
	~VuGfxSceneMaterial();
public:
	VuGfxSceneMaterial(int index);

	bool					load(VuBinaryDataReader &reader);
	static bool				bake(const VuJsonContainer &data, VuBinaryDataWriter &writer, VuAssetDependencies &dependencies);

	int						mIndex;
	VuMaterialAsset			*mpMaterialAsset;
};
