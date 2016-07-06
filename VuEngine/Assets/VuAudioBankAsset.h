//*****************************************************************************
//
//  Copyright (c) 2011-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  Audio Bank Asset class
// 
//*****************************************************************************

#pragma once

#include "VuAsset.h"

namespace FMOD { class Sound; }


class VuAudioBankAsset : public VuAsset
{
	DECLARE_RTTI

protected:
	~VuAudioBankAsset() { unload(); }

public:
	VuAudioBankAsset();

	static void			schema(const VuJsonContainer &creationInfo, VuJsonContainer &schema);
	static bool			bake(const VuJsonContainer &creationInfo, VuAssetBakeParams &bakeParams);

private:
	virtual bool		load(VuBinaryDataReader &reader);
	virtual void		unload();

	int					mInstanceCount;
	bool				mDecompress;

	std::string			mBankName;
	FMOD::Sound			*mpSound;
};
