//*****************************************************************************
//
//  Copyright (c) 2007-2008 Ralf Knoesel
//  Copyright (c) 2008-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  Enum property class
// 
//*****************************************************************************

#include "VuEnumProperty.h"


//*****************************************************************************
void VuIntEnumProperty::load(const VuFastContainer &data)
{
	mValue = mDefaultValue;

	const char *strValue = data[mstrName].asCString();
	for ( int i = 0; i < getChoiceCount(); i++ )
	{
		if ( strcmp(strValue, getChoiceName(i)) == 0 )
		{
			int value = getChoiceValue(i);
			if ( mValue != value )
			{
				mValue = value;
				if ( mbNotifyOnLoad )
					notifyWatcher();
			}

			break;
		}
	}

	mInitialValue = mValue;
}

//*****************************************************************************
void VuIntEnumProperty::setCurrent(const VuJsonContainer &data, bool notify)
{
	VuIntProperty::setCurrent(translateChoice(data), notify);
}

//*****************************************************************************
void VuIntEnumProperty::getCurrent(VuJsonContainer &data) const
{
	VuIntProperty::getCurrent(data);
	data = translateChoice(data);
}

//*****************************************************************************
void VuIntEnumProperty::getDefault(VuJsonContainer &data) const
{
	VuIntProperty::getDefault(data);
	data = translateChoice(data);
}

//*****************************************************************************
VuJsonContainer VuIntEnumProperty::translateChoice(const VuJsonContainer &value) const
{
	VuJsonContainer result;

	if ( value.isInt() )
	{
		result.putValue("");
		for ( int i = 0; i < getChoiceCount(); i++ )
		{
			if ( value.asInt() == getChoiceValue(i) )
			{
				result.putValue(getChoiceName(i));
				break;
			}
		}
	}
	else if ( value.isString() )
	{
		result.putValue(0);
		for ( int i = 0; i < getChoiceCount(); i++ )
		{
			if ( value.asString() == getChoiceName(i) )
			{
				result.putValue(getChoiceValue(i));
				break;
			}
		}
	}

	return result;
}

//*****************************************************************************
const char *VuIntEnumProperty::getCurChoiceName() const
{
	for ( int i = 0; i < getChoiceCount(); i++ )
		if ( mValue == getChoiceValue(i) )
			return getChoiceName(i);

	return "";
}

//*****************************************************************************
VuStaticIntEnumProperty::VuStaticIntEnumProperty(const char *strName, int &pValue, const Choice *choices) : VuIntEnumProperty(strName, pValue),
	mpChoices(choices)
{
}

//*****************************************************************************
int VuStaticIntEnumProperty::getChoiceCount() const
{
	int count = 0;
	while ( mpChoices[count].mpName )
		count++;
	return count;
}

//*****************************************************************************
VuStaticStringEnumProperty::VuStaticStringEnumProperty(const char *strName, std::string &pValue, const Choice *choices) : VuStringEnumProperty(strName, pValue),
	mpChoices(choices)
{
}

//*****************************************************************************
int VuStaticStringEnumProperty::getChoiceCount() const
{
	int count = 0;
	while ( mpChoices[count].mpName )
		count++;
	return count;
}
