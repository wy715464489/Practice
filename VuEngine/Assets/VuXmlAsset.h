//*****************************************************************************
//
//  Copyright (c) 2011-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  Xml Asset class
// 
//*****************************************************************************

#pragma once

#include "VuEngine/Assets/VuAsset.h"
#include "VuEngine/Libs/tinyxml/tinyxml.h"


class VuXmlAsset : public VuAsset
{
	DECLARE_RTTI

protected:
	~VuXmlAsset() { unload(); }
public:

	const TiXmlDocument		&getXmlDocument() const { return mXmlDocument; }

	static bool				bake(const VuJsonContainer &creationInfo, VuAssetBakeParams &bakeParams);

protected:
	virtual bool			load(VuBinaryDataReader &reader);
	virtual void			unload();

	TiXmlDocument			mXmlDocument;
};
