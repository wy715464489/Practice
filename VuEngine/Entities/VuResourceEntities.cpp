//*****************************************************************************
//
//  Copyright (c) 2009-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  Resource entities
// 
//*****************************************************************************

#include "VuEngine/Entities/VuEntity.h"
#include "VuEngine/Properties/VuAssetProperty.h"
#include "VuEngine/Assets/VuAssetFactory.h"
#include "VuEngine/Assets/VuPfxAsset.h"
#include "VuEngine/Assets/VuAudioProjectAsset.h"
#include "VuEngine/Assets/VuAudioBankAsset.h"


//*****************************************************************************
// Pfx Project
//*****************************************************************************
class VuPfxProjectEntity : public VuEntity
{
	DECLARE_RTTI

public:
	VuPfxProjectEntity();

private:
	// properties
	std::string	mAssetName;
};

IMPLEMENT_RTTI(VuPfxProjectEntity, VuEntity);
IMPLEMENT_ENTITY_REGISTRATION(VuPfxProjectEntity);


//*****************************************************************************
VuPfxProjectEntity::VuPfxProjectEntity()
{
	addProperty(new VuAssetProperty<VuPfxAsset>("Pfx Asset", mAssetName));
}


//*****************************************************************************
// Audio Bank
//*****************************************************************************
class VuAudioBankEntity : public VuEntity
{
	DECLARE_RTTI

public:
	VuAudioBankEntity();

private:
	// properties
	std::string	mAssetName;
};

IMPLEMENT_RTTI(VuAudioBankEntity, VuEntity);
IMPLEMENT_ENTITY_REGISTRATION(VuAudioBankEntity);


//*****************************************************************************
VuAudioBankEntity::VuAudioBankEntity()
{
	addProperty(new VuAssetProperty<VuAudioBankAsset>("Audio Bank", mAssetName));
}


//*****************************************************************************
// Audio Project
//*****************************************************************************
class VuAudioProjectEntity : public VuEntity
{
	DECLARE_RTTI

public:
	VuAudioProjectEntity();

private:
	// properties
	std::string	mAssetName;
};

IMPLEMENT_RTTI(VuAudioProjectEntity, VuEntity);
IMPLEMENT_ENTITY_REGISTRATION(VuAudioProjectEntity);


//*****************************************************************************
VuAudioProjectEntity::VuAudioProjectEntity()
{
	addProperty(new VuAssetProperty<VuAudioProjectAsset>("Audio Project", mAssetName));
}
