//*****************************************************************************
//
//  Copyright (c) 2012-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  Compiled Shader Asset class
// 
//*****************************************************************************

#pragma once

#include "VuAsset.h"

class VuShaderProgram;


class VuCompiledShaderAsset : public VuAsset
{
	DECLARE_RTTI

protected:
	~VuCompiledShaderAsset() { unload(); }

public:
	VuShaderProgram		*getShaderProgram()		{ return mpShaderProgram; }

	static void			schema(const VuJsonContainer &creationInfo, VuJsonContainer &schema);
	static bool			bake(const VuJsonContainer &creationInfo, VuAssetBakeParams &bakeParams);

private:
	virtual bool		load(VuBinaryDataReader &reader);
	virtual void		unload();

	VuShaderProgram		*mpShaderProgram;
};
