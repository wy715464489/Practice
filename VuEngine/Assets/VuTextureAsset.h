//*****************************************************************************
//
//  Copyright (c) 2007-2008 Ralf Knoesel
//  Copyright (c) 2008-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  Texture Asset class
// 
//*****************************************************************************

#pragma once

#include "VuBaseTextureAsset.h"

class VuJsonContainer;
class VuTexture;


class VuTextureAsset : public VuBaseTextureAsset
{
	DECLARE_RTTI

protected:
	~VuTextureAsset() { unload(); }
public:
	VuTextureAsset() : mpTexture(VUNULL) {}

	VuTexture			*getTexture() const { return mpTexture; }
	VuBaseTexture		*getBaseTexture() const;
	bool				getScaleLowSpec() const { return mScaleLowSpec; }

	virtual bool		substitute(const VuAsset *pSubstAsset);

	static void			schema(const VuJsonContainer &creationInfo, VuJsonContainer &schema);
	static bool			bake(const VuJsonContainer &creationInfo, VuAssetBakeParams &bakeParams);

private:
	virtual bool		load(VuBinaryDataReader &reader);
	virtual void		unload();
	virtual void		editorReload();

	VuTexture			*mpTexture;
	bool				mScaleLowSpec;
};
