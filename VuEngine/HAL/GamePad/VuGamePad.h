//*****************************************************************************
//
//  Copyright (c) 2006-2008 Ralf Knoesel
//  Copyright (c) 2008-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  Interface class to GamePad library.
// 
//*****************************************************************************

#pragma once

#include "VuEngine/VuSystemComponent.h"
#include "VuEngine/Containers/VuArray.h"

class VuEngine;
class VuJsonContainer;


class VuGamePad : public VuSystemComponent
{
	DECLARE_SYSTEM_COMPONENT(VuGamePad)

protected:
	// called by engine
	friend class VuEngine;
	virtual bool init();

public:

	// axis/button enumeration
	int				getAxisCount()			{ return mAxisDefs.size(); }
	const char		*getAxisName(int axis)	{ return mAxisDefs[axis].mName; }
	float			getAxisMin(int axis)	{ return mAxisDefs[axis].mMinVal; }
	float			getAxisMax(int axis)	{ return mAxisDefs[axis].mMaxVal; }
	int				getAxisIndex(const char *name);

	int				getButtonCount()			{ return mButtonDefs.size(); }
	const char		*getButtonName(int button)	{ return mButtonDefs[button].mName; }
	int				getButtonIndex(const char *name);

	// Pad interface
	class VuController;
	enum { MAX_NUM_PADS = 6 };
	virtual VuController	&getController(int index) = 0;

	enum eDeviceType
	{
		DEVICE_UNKNOWN,
		DEVICE_GAMEPAD,
		DEVICE_SIMPLE_GAMEPAD,
		DEVICE_REMOTE_CONTROL,
		DEVICE_STEERING_WHEEL,
		DEVICE_MOBILE_CONTROLLER,

		NUM_DEVICE_TYPES
	};
	class VuController
	{
	public:
		VuController();
		void			init();
		void			zero();

		enum eEffect
		{
			EFFECT_COLLISION_SMALL,
			EFFECT_COLLISION_MEDIUM,
			EFFECT_COLLISION_LARGE,
			EFFECT_SPLASH_SMALL,
			EFFECT_SPLASH_MEDIUM,
			EFFECT_SPLASH_LARGE,

			NUM_EFFECTS
		};
		virtual void	playVibrationEffect(int effect) = 0;

		typedef VuArray<float> Axes;

		bool			mIsConnected;
		eDeviceType		mDeviceType;
		VUUINT32		mButtons;
		Axes			mAxes;
		std::string		mEndpointId;
	};

protected:
	void	addAxis(const char *name, float minVal, float maxVal);
	void	addButton(const char *name);

	struct AxisDef
	{
		const char	*mName;
		VUUINT32	mHashedName;
		float		mMinVal;
		float		mMaxVal;
	};
	struct ButtonDef
	{
		const char	*mName;
		VUUINT32	mHashedName;
	};
	typedef VuArray<AxisDef> AxisDefs;
	typedef VuArray<ButtonDef> ButtonDefs;

	AxisDefs	mAxisDefs;
	ButtonDefs	mButtonDefs;
};