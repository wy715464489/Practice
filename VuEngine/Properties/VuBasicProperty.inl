//*****************************************************************************
//
//  Copyright (c) 2007-2008 Ralf Knoesel
//  Copyright (c) 2008-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  Basic property inline implementation
// 
//*****************************************************************************

#include "VuEngine/Json/VuJsonContainer.h"
#include "VuEngine/Json/VuFastContainer.h"
#include "VuEngine/Util/VuDataUtil.h"
#include "VuEngine/Util/VuFastDataUtil.h"


//*****************************************************************************
template <class T, VuProperty::eType type>
VuBasicProperty<T, type>::VuBasicProperty(const char *strName, T &value):
	VuProperty(strName),
	mDefaultValue(value),
	mInitialValue(value),
	mValue(value)
{
}

//*****************************************************************************
template <class T, VuProperty::eType type>
void VuBasicProperty<T, type>::load(const VuJsonContainer &data)
{
	mValue = mDefaultValue;
	setCurrent(data[mstrName], mbNotifyOnLoad);
	mInitialValue = mValue;
}

//*****************************************************************************
template <class T, VuProperty::eType type>
void VuBasicProperty<T, type>::save(VuJsonContainer &data) const
{
	if ( mValue != mDefaultValue )
		getCurrent(data[mstrName]);
}

//*****************************************************************************
template <class T, VuProperty::eType type>
void VuBasicProperty<T, type>::load(const VuFastContainer &data)
{
	mValue = mDefaultValue;

	T value;
	if ( VuFastDataUtil::getValue(data[mstrName], value) )
	{
		value = transformToNative(value);
		if ( mValue != value )
		{
			mValue = value;
			if ( mbNotifyOnLoad )
				notifyWatcher();
		}
	}

	mInitialValue = mValue;
}

//*****************************************************************************
template <class T, VuProperty::eType type>
void VuBasicProperty<T, type>::setCurrent(const VuJsonContainer &data, bool notify)
{
	T value;
	if ( VuDataUtil::getValue(data, value) )
	{
		value = transformToNative(value);
		if ( mValue != value )
		{
			mValue = value;
			if ( notify )
				notifyWatcher();
		}
	}
}

//*****************************************************************************
template <class T, VuProperty::eType type>
void VuBasicProperty<T, type>::getCurrent(VuJsonContainer &data) const
{
	VuDataUtil::putValue(data, transformFromNative(mValue));
}

//*****************************************************************************
template <class T, VuProperty::eType type>
void VuBasicProperty<T, type>::getDefault(VuJsonContainer &data) const
{
	VuDataUtil::putValue(data, transformFromNative(mDefaultValue));
}
