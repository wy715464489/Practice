//*****************************************************************************
//
//  Copyright (c) 2013-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  UI News Text class
// 
//*****************************************************************************

#include "VuEngine/Entities/UI/VuUITextBaseEntity.h"


class VuUINewsTextEntity : public VuUITextBaseEntity
{
	DECLARE_RTTI

public:
	VuUINewsTextEntity();

	const char	*getText() { return mText.c_str(); }

protected:
	std::string	mText;
};

IMPLEMENT_RTTI(VuUINewsTextEntity, VuEntity);
IMPLEMENT_ENTITY_REGISTRATION(VuUINewsTextEntity);


//*****************************************************************************
VuUINewsTextEntity::VuUINewsTextEntity()
{
	// properties
	addProperty(new VuStringProperty("Text", mText));
}
