//*****************************************************************************
//
//  Copyright (c) 2009-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  Static Model Asset class
// 
//*****************************************************************************

#pragma once

#include "VuAsset.h"

class VuGfxStaticScene;


class VuStaticModelAsset : public VuAsset
{
	DECLARE_RTTI

protected:
	~VuStaticModelAsset() { unload(); }
public:
	VuStaticModelAsset() : mpGfxStaticScene(VUNULL) {}

	VuGfxStaticScene	*getGfxStaticScene() const { return mpGfxStaticScene; }

	virtual bool		substitute(const VuAsset *pSubstAsset);

	static void			schema(const VuJsonContainer &creationInfo, VuJsonContainer &schema);
	static bool			bake(const VuJsonContainer &creationInfo, VuAssetBakeParams &bakeParams);

private:
	virtual bool		load(VuBinaryDataReader &reader);
	virtual void		unload();
	virtual void		editorReload();

	VuGfxStaticScene	*mpGfxStaticScene;
};
