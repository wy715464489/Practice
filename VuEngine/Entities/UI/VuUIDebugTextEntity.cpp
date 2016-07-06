//*****************************************************************************
//
//  Copyright (c) 2010-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  Debug text class
// 
//*****************************************************************************

#include "VuEngine/Entities/UI/VuUITextBaseEntity.h"
#include "VuEngine/Properties/VuBasicProperty.h"


class VuUIDebugTextEntity : public VuUITextBaseEntity
{
	DECLARE_RTTI

public:
	VuUIDebugTextEntity();

	virtual const char		*getText() { return mText.c_str(); }

protected:
	std::string	mText;
};


IMPLEMENT_RTTI(VuUIDebugTextEntity, VuUITextBaseEntity);
IMPLEMENT_ENTITY_REGISTRATION(VuUIDebugTextEntity);


//*****************************************************************************
VuUIDebugTextEntity::VuUIDebugTextEntity()
{
	addProperty(new VuStringProperty("Text", mText));
}
