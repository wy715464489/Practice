//*****************************************************************************
//
//  Copyright (c) 2013-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  Blob property class
// 
//*****************************************************************************

#include "VuBlobProperty.h"
#include "VuEngine/Json/VuJsonContainer.h"
#include "VuEngine/Json/VuFastContainer.h"
#include "VuEngine/Util/VuDataUtil.h"
#include "VuEngine/Util/VuFastDataUtil.h"


//*****************************************************************************
VuBlobProperty::VuBlobProperty(const char *strName, VuArray<VUBYTE> &value):
	VuProperty(strName),
	mValue(value)
{
}

//*****************************************************************************
void VuBlobProperty::load(const VuJsonContainer &data)
{
	setCurrent(data[mstrName], mbNotifyOnLoad);
}

//*****************************************************************************
void VuBlobProperty::save(VuJsonContainer &data) const
{
	getCurrent(data[mstrName]);
}

//*****************************************************************************
void VuBlobProperty::load(const VuFastContainer &data)
{
	VuFastDataUtil::getValue(data[mstrName], mValue);
	if ( mbNotifyOnLoad )
		notifyWatcher();
}

//*****************************************************************************
void VuBlobProperty::setCurrent(const VuJsonContainer &data, bool notify)
{
	VuDataUtil::getValue(data, mValue);
	if ( notify )
		notifyWatcher();
}

//*****************************************************************************
void VuBlobProperty::getCurrent(VuJsonContainer &data) const
{
	VuDataUtil::putValue(data, mValue);
}
