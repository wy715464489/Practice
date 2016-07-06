//*****************************************************************************
//
//  Copyright (c) 2008-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  Font Asset class
// 
//*****************************************************************************

#pragma once

#include "VuEngine/Assets/VuAsset.h"

class VuFont;


class VuFontAsset : public VuAsset
{
	DECLARE_RTTI

protected:
	~VuFontAsset() { unload(); }
public:
	VuFontAsset() : mpFont(VUNULL) {}

	VuFont				*getFont() const { return mpFont; }

	static void			schema(const VuJsonContainer &creationInfo, VuJsonContainer &schema);
	static bool			bake(const VuJsonContainer &creationInfo, VuAssetBakeParams &bakeParams);

private:
	virtual bool		load(VuBinaryDataReader &reader);
	virtual void		unload();

	VuFont				*mpFont;
};
