//*****************************************************************************
//
//  Copyright (c) 2008-2008 Ralf Knoesel
//  Copyright (c) 2008-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  UI event enum property class
// 
//*****************************************************************************

#include "VuUIEventEnumProperty.h"
#include "VuEngine/UI/VuUI.h"
#include "VuEngine/Util/VuHashedString.h"


//*****************************************************************************
int VuUIEventEnumProperty::getChoiceCount() const
{
	return VuUI::IF()->getEventCount();
}

//*****************************************************************************
const char *VuUIEventEnumProperty::getChoice(int index) const
{
	return VuUI::IF()->getEvent(index).c_str();
}
