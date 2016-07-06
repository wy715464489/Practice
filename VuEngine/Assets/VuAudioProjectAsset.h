//*****************************************************************************
//
//  Copyright (c) 2011-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  Audio Project Asset class
// 
//*****************************************************************************

#pragma once

#include "VuAsset.h"

namespace FMOD { class EventProject; }


class VuAudioProjectAsset : public VuAsset
{
	DECLARE_RTTI

protected:
	~VuAudioProjectAsset() { unload(); }

public:
	VuAudioProjectAsset();

	static void			schema(const VuJsonContainer &creationInfo, VuJsonContainer &schema);
	static bool			bake(const VuJsonContainer &creationInfo, VuAssetBakeParams &bakeParams);

private:
	virtual bool		load(VuBinaryDataReader &reader);
	virtual void		unload();

	FMOD::EventProject	*mpProject;
};
