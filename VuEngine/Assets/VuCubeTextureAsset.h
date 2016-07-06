//*****************************************************************************
//
//  Copyright (c) 2008-2008 Ralf Knoesel
//  Copyright (c) 2008-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  Cube Texture Asset class
// 
//*****************************************************************************

#pragma once

#include "VuBaseTextureAsset.h"

class VuJsonContainer;
class VuCubeTexture;


class VuCubeTextureAsset : public VuBaseTextureAsset
{
	DECLARE_RTTI

protected:
	~VuCubeTextureAsset() { unload(); }
public:
	VuCubeTextureAsset() : mpCubeTexture(VUNULL) {}

	VuCubeTexture		*getTexture() const { return mpCubeTexture; }
	VuBaseTexture		*getBaseTexture() const;
	bool				getScaleLowSpec() const { return mScaleLowSpec; }

	virtual bool		substitute(const VuAsset *pSubstAsset);

	static void			schema(const VuJsonContainer &creationInfo, VuJsonContainer &schema);
	static bool			bake(const VuJsonContainer &creationInfo, VuAssetBakeParams &bakeParams);

private:
	virtual bool		load(VuBinaryDataReader &reader);
	virtual void		unload();
	virtual void		editorReload();

	VuCubeTexture		*mpCubeTexture;
	bool				mScaleLowSpec;
};
