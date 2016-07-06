//*****************************************************************************
//
//  Copyright (c) 2006-2008 Ralf Knoesel
//  Copyright (c) 2008-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  Interface class to GamePad library.
// 
//*****************************************************************************

#include "VuGamePad.h"
#include "VuEngine/Util/VuHash.h"


//*****************************************************************************
bool VuGamePad::init()
{
	for ( int i = 0; i < MAX_NUM_PADS; i++ )
		getController(i).init();

	return true;
}

//*****************************************************************************
int VuGamePad::getAxisIndex(const char *name)
{
	VUUINT32 hashedName = VuHash::fnv32String(name);

	for ( int i = 0; i < mAxisDefs.size(); i++ )
		if ( hashedName == mAxisDefs[i].mHashedName )
			return i;

	return -1;
}

//*****************************************************************************
int VuGamePad::getButtonIndex(const char *name)
{
	VUUINT32 hashedName = VuHash::fnv32String(name);

	for ( int i = 0; i < mButtonDefs.size(); i++ )
		if ( hashedName == mButtonDefs[i].mHashedName )
			return i;

	return -1;
}

//*****************************************************************************
VuGamePad::VuController::VuController():
	mIsConnected(false),
	mDeviceType(DEVICE_UNKNOWN),
	mButtons(0)
{
}

//*****************************************************************************
void VuGamePad::VuController::init()
{
	mAxes.resize(VuGamePad::IF()->getAxisCount());
	zero();
}

//*****************************************************************************
void VuGamePad::VuController::zero()
{
	mButtons = 0;
	for ( int i = 0; i < mAxes.size(); i++ )
		mAxes[i] = 0.0f;
}

//*****************************************************************************
void VuGamePad::addAxis(const char *name, float minVal, float maxVal)
{
	AxisDef def;

	def.mName = name;
	def.mHashedName = VuHash::fnv32String(name);
	def.mMinVal = minVal;
	def.mMaxVal = maxVal;

	mAxisDefs.push_back(def);
}

//*****************************************************************************
void VuGamePad::addButton(const char *name)
{
	ButtonDef def;

	def.mName = name;
	def.mHashedName = VuHash::fnv32String(name);

	mButtonDefs.push_back(def);
}