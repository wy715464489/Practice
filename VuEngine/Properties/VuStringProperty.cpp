//*****************************************************************************
//
//  Copyright (c) 2014-2014 Ralf Knoesel
//  Confidential Trade Secrets
// 
//  String property class
// 
//*****************************************************************************

#include "VuStringProperty.h"
#include "VuEngine/Json/VuJsonContainer.h"
#include "VuEngine/Json/VuFastContainer.h"


//*****************************************************************************
void VuStringProperty::load(const VuJsonContainer &data)
{
	mValue = mDefaultValue;
	setCurrent(data[mstrName], mbNotifyOnLoad);
	mInitialValue = mValue;
}

//*****************************************************************************
void VuStringProperty::save(VuJsonContainer &data) const
{
	if ( mValue != mDefaultValue )
		getCurrent(data[mstrName]);
}

//*****************************************************************************
void VuStringProperty::load(const VuFastContainer &data)
{
	mValue = mDefaultValue;

	const VuFastContainer &field = data[mstrName];
	if ( field.isString() )
	{
		const char *value = field.asCString();
		if ( mValue != value )
		{
			mValue = value;
			onValueChanged();
			if ( mbNotifyOnLoad )
				notifyWatcher();
		}
	}

	mInitialValue = mValue;
}

//*****************************************************************************
void VuStringProperty::setCurrent(const VuJsonContainer &data, bool notify)
{
	if ( data.isString() )
	{
		const char *value = data.asCString();
		if ( mValue != value )
		{
			mValue = value;
			onValueChanged();
			if ( notify )
				notifyWatcher();
		}
	}
}

//*****************************************************************************
void VuStringProperty::getCurrent(VuJsonContainer &data) const
{
	data.putValue(mValue);
}

//*****************************************************************************
void VuStringProperty::getDefault(VuJsonContainer &data) const
{
	data.putValue(mDefaultValue);
}
