//*****************************************************************************
//
//  Copyright (c) 2009-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  Bit Field property class
// 
//*****************************************************************************

#include "VuBitFieldProperty.h"
#include "VuEngine/Json/VuJsonContainer.h"
#include "VuEngine/Json/VuFastContainer.h"
#include "VuEngine/Util/VuDataUtil.h"
#include "VuEngine/Util/VuFastDataUtil.h"


//*****************************************************************************
VuBitFieldProperty::VuBitFieldProperty(const char *strName, VUUINT32 &value, VUUINT32 mask):
	VuProperty(strName),
	mDefaultValue(false),
	mInitialValue(false),
	mValue(value),
	mMask(mask)
{
	if ( value & mMask )
	{
		mDefaultValue = true;
		mInitialValue = true;
	}
}

//*****************************************************************************
void VuBitFieldProperty::load(const VuJsonContainer &data)
{
	writeBitField(mDefaultValue);
	setCurrent(data[mstrName], mbNotifyOnLoad);
	mInitialValue = readBitField();
}

//*****************************************************************************
void VuBitFieldProperty::save(VuJsonContainer &data) const
{
	if ( readBitField() != mDefaultValue )
		getCurrent(data[mstrName]);
}

//*****************************************************************************
void VuBitFieldProperty::load(const VuFastContainer &data)
{
	writeBitField(mDefaultValue);

	bool value;
	if ( VuFastDataUtil::getValue(data[mstrName], value) )
	{
		if ( readBitField() != value )
		{
			writeBitField(value);
			if ( mbNotifyOnLoad )
				notifyWatcher();
		}
	}

	mInitialValue = readBitField();
}

//*****************************************************************************
void VuBitFieldProperty::setCurrent(const VuJsonContainer &data, bool notify)
{
	bool value;
	if ( VuDataUtil::getValue(data, value) )
	{
		if ( readBitField() != value )
		{
			writeBitField(value);
			if ( notify )
				notifyWatcher();
		}
	}
}

//*****************************************************************************
void VuBitFieldProperty::getCurrent(VuJsonContainer &data) const
{
	VuDataUtil::putValue(data, readBitField());
}

//*****************************************************************************
void VuBitFieldProperty::getDefault(VuJsonContainer &data) const
{
	VuDataUtil::putValue(data, mDefaultValue);
}

//*****************************************************************************
bool VuBitFieldProperty::readBitField() const
{
	return (mValue & mMask) ? true : false;
}

//*****************************************************************************
void VuBitFieldProperty::writeBitField(bool value)
{
	if ( value )
		mValue |= mMask;
	else
		mValue &= ~mMask;
}
