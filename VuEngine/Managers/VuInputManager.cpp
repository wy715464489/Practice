//*****************************************************************************
//
//  Copyright (c) 2011-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  InputManager class
// 
//*****************************************************************************

#include "VuInputManager.h"
#include "VuEngine/Managers/VuTickManager.h"
#include "VuEngine/Managers/VuProfileManager.h"
#include "VuEngine/Assets/VuAssetFactory.h"
#include "VuEngine/Assets/VuDBAsset.h"
#include "VuEngine/HAL/GamePad/VuGamePad.h"
#include "VuEngine/HAL/Keyboard/VuKeyboard.h"
#include "VuEngine/Math/VuMath.h"
#include "VuEngine/Math/VuMathUtil.h"


// implementation
class VuInputManagerImpl : public VuInputManager
{
protected:

	// called by engine
	friend class VuEngine;
	virtual bool	init();
	virtual void	release();

public:
	VuInputManagerImpl();

	virtual float	getAxisValue(int padIndex, const char *name);
	virtual float	getRawAxisValue(int padIndex, const char *name);
	virtual bool	getButtonValue(int padIndex, const char *name);
	virtual bool	getButtonWasPressed(int padIndex, const char *name);
	virtual bool	getButtonWasReleased(int padIndex, const char *name);

	virtual void	setDefaultMapping(int padIndex, eConfig config);

	virtual void	setOnScreenButton(int padIndex, const char *name);
	virtual void	setOnScreenAxis(int padIndex, const char *name, float direction);

	virtual void	setConfig(eConfig config) { mConfig = config; }

private:
	struct AxisDef
	{
		std::string	mName;
		VUUINT32	mHashedName;
		float		mSmoothTime;
	};
	struct ButtonDef
	{
		std::string	mName;
		VUUINT32	mHashedName;
	};
	typedef std::vector<AxisDef> AxisDefs;
	typedef std::vector<ButtonDef> ButtonDefs;

	struct Mapping
	{
		Mapping() : mType(M_INVALID), mIndex(-1) {}
		eMapping	mType;
		int			mIndex;
	};

	struct Axis
	{
		Axis() : mRawValue(0.0f), mValue(0.0f), mVelocity(0.0f), mOnScreenValue(0.0f) {}
		Mapping		mPosMappings[CONFIG_COUNT][MAX_CHANNELS];
		Mapping		mNegMappings[CONFIG_COUNT][MAX_CHANNELS];
		float		mRawValue;
		float		mValue;
		float		mVelocity;
		float		mOnScreenValue;
	};
	struct Button
	{
		Button() : mValue(false), mWasPressed(false), mWasReleased(false), mOnScreenValue(false) {}
		Mapping		mMappings[CONFIG_COUNT][MAX_CHANNELS];
		bool		mValue;
		bool		mWasPressed;
		bool		mWasReleased;
		bool		mOnScreenValue;
	};
	typedef std::vector<Axis> Axes;
	typedef std::vector<Button> Buttons;

	class VuController
	{
	public:
		VuController() : mAxes(0), mButtons(0) {}
		Axes	mAxes;
		Buttons	mButtons;
	};

	void			tick(float fdt);

	Axis			*getAxis(int padIndex, const char *name);
	Button			*getButton(int padIndex, const char *name);
	int				getAxisIndex(const char *name);
	int				getButtonIndex(const char *name);

	void			loadMapping(int padIndex, const VuJsonContainer &data, eConfig config);
	void			loadMapping(const VuJsonContainer &data, Mapping *pMappings);
	void			loadMapping(const VuJsonContainer &data, Mapping &mapping);

	inline float	translateAxis(const AxisDef &axisDef, const VuGamePad::VuController &src, const Mapping *pMappings, int padIndex);
	inline bool		translateButton(const ButtonDef &buttonDef, const VuGamePad::VuController &src, const Mapping *pMappings, int padIndex);

	VuDBAsset		*mpInputDBAsset;
	AxisDefs		mAxisDefs;
	ButtonDefs		mButtonDefs;
	VuController	*mpControllers;
	eConfig			mConfig;
};


// the interface
IMPLEMENT_SYSTEM_COMPONENT(VuInputManager, VuInputManagerImpl);


//*****************************************************************************
VuInputManagerImpl::VuInputManagerImpl():
	mConfig(CONFIG_GAMEPAD)
{
}

//*****************************************************************************
bool VuInputManagerImpl::init()
{
	VuTickManager::IF()->registerHandler(this, &VuInputManagerImpl::tick, "Input");

	// load input DB
	mpInputDBAsset = VuAssetFactory::IF()->createAsset<VuDBAsset>("InputDB");

	// create axis definitions
	const VuJsonContainer &axisData = mpInputDBAsset->getDB()["Axes"];
	for ( int i = 0; i < axisData.size(); i++ )
	{
		AxisDef axisDef;

		axisDef.mName = axisData[i]["Name"].asString();
		axisDef.mHashedName = VuHash::fnv32String(axisDef.mName.c_str());
		axisDef.mSmoothTime = axisData[i]["SmoothTime"].asFloat();

		mAxisDefs.push_back(axisDef);
	}

	// create button definitions
	const VuJsonContainer &buttonData = mpInputDBAsset->getDB()["Buttons"];
	for ( int i = 0; i < buttonData.size(); i++ )
	{
		ButtonDef buttonDef;

		buttonDef.mName = buttonData[i]["Name"].asString();
		buttonDef.mHashedName = VuHash::fnv32String(buttonDef.mName.c_str());

		mButtonDefs.push_back(buttonDef);
	}

	// create controllers
	mpControllers = new VuController[VuGamePad::MAX_NUM_PADS];
	for ( int iController = 0; iController < VuGamePad::MAX_NUM_PADS; iController++ )
	{
		VuController &controller = mpControllers[iController];

		controller.mAxes.resize(mAxisDefs.size());
		controller.mButtons.resize(mButtonDefs.size());

		// start with default mapping
		setDefaultMapping(iController, CONFIG_GAMEPAD);
		setDefaultMapping(iController, CONFIG_KEYBOARD);
	}

	return true;
}

//*****************************************************************************
void VuInputManagerImpl::release()
{
	VuTickManager::IF()->unregisterHandlers(this);
	VuAssetFactory::IF()->releaseAsset(mpInputDBAsset);

	delete[] mpControllers;
}

//*****************************************************************************
float VuInputManagerImpl::getAxisValue(int padIndex, const char *name)
{
	if ( const Axis *pAxis = getAxis(padIndex, name) )
		return pAxis->mValue;

	return 0.0f;
}

//*****************************************************************************
float VuInputManagerImpl::getRawAxisValue(int padIndex, const char *name)
{
	if ( const Axis *pAxis = getAxis(padIndex, name) )
		return pAxis->mRawValue;

	return 0.0f;
}

//*****************************************************************************
bool VuInputManagerImpl::getButtonValue(int padIndex, const char *name)
{
	if ( const Button *pButton = getButton(padIndex, name) )
		return pButton->mValue;

	return false;
}

//*****************************************************************************
bool VuInputManagerImpl::getButtonWasPressed(int padIndex, const char *name)
{
	if ( const Button *pButton = getButton(padIndex, name) )
		return pButton->mWasPressed;

	return false;
}

//*****************************************************************************
bool VuInputManagerImpl::getButtonWasReleased(int padIndex, const char *name)
{
	if ( const Button *pButton = getButton(padIndex, name) )
		return pButton->mWasReleased;

	return false;
}

//*****************************************************************************
void VuInputManagerImpl::setDefaultMapping(int padIndex, eConfig config)
{
	const VuJsonContainer &defaultMapping = mpInputDBAsset->getDB()["DefaultMapping"][VUPLATFORM];
	const std::string &sku = VuAssetFactory::IF()->getSku();

	const VuJsonContainer &defaultData = defaultMapping.hasMember(sku) ? defaultMapping[sku] : defaultMapping;

	if ( config == CONFIG_GAMEPAD )
		loadMapping(padIndex, defaultData["GamePad"], config);
	else if ( config == CONFIG_KEYBOARD )
		loadMapping(padIndex, defaultData["Keyboard"], config);
}

//*****************************************************************************
void VuInputManagerImpl::setOnScreenButton(int padIndex, const char *name)
{
	if ( Button *pButton = getButton(padIndex, name) )
		pButton->mOnScreenValue = true;
}

//*****************************************************************************
void VuInputManagerImpl::setOnScreenAxis(int padIndex, const char *name, float direction)
{
	if ( Axis *pAxis = getAxis(padIndex, name) )
		pAxis->mOnScreenValue += direction;
}

//*****************************************************************************
void VuInputManagerImpl::tick(float fdt)
{
	for ( int iController = 0; iController < VuGamePad::MAX_NUM_PADS; iController++ )
	{
		VuController &dst = mpControllers[iController];
		const VuGamePad::VuController &src = VuGamePad::IF()->getController(iController);

		// process axes
		for ( int iAxis = 0; iAxis < (int)mAxisDefs.size(); iAxis++ )
		{
			const AxisDef &axisDef = mAxisDefs[iAxis];
			Axis &axis = dst.mAxes[iAxis];

			float posValue = translateAxis(axisDef, src, axis.mPosMappings[mConfig], iController);
			float negValue = translateAxis(axisDef, src, axis.mNegMappings[mConfig], iController);

			axis.mRawValue = posValue - negValue + axis.mOnScreenValue;
			axis.mRawValue = VuClamp(axis.mRawValue, -1.0f, 1.0f);
			axis.mOnScreenValue = 0.0f;

			if ( axisDef.mSmoothTime > 0.0f )
			{
				axis.mValue = VuMathUtil::smoothCD(axis.mValue, axis.mRawValue, axis.mVelocity, axisDef.mSmoothTime, fdt);
			}
			else
			{
				axis.mValue = axis.mRawValue;
			}
		}

		// process buttons
		for ( int i = 0; i < (int)mButtonDefs.size(); i++ )
		{
			const ButtonDef &buttonDef = mButtonDefs[i];
			Button &button = dst.mButtons[i];

			bool value = translateButton(buttonDef, src, button.mMappings[mConfig], iController);

			bool prevValue = button.mValue;
			button.mValue = value | button.mOnScreenValue;
			button.mWasPressed = button.mValue && !prevValue;
			button.mWasReleased = !button.mValue && prevValue;
			button.mOnScreenValue = false;
		}
	}
}

//*****************************************************************************
VuInputManagerImpl::Axis *VuInputManagerImpl::getAxis(int padIndex, const char *name)
{
	VUUINT32 hashedName = VuHash::fnv32String(name);

	for ( int i = 0; i < (int)mAxisDefs.size(); i++ )
		if ( hashedName == mAxisDefs[i].mHashedName )
			return &mpControllers[padIndex].mAxes[i];

	return VUNULL;
}

//*****************************************************************************
VuInputManagerImpl::Button *VuInputManagerImpl::getButton(int padIndex, const char *name)
{
	VUUINT32 hashedName = VuHash::fnv32String(name);

	for ( int i = 0; i < (int)mButtonDefs.size(); i++ )
		if ( hashedName == mButtonDefs[i].mHashedName )
			return &mpControllers[padIndex].mButtons[i];

	return VUNULL;
}

//*****************************************************************************
int VuInputManagerImpl::getAxisIndex(const char *name)
{
	VUUINT32 hashedName = VuHash::fnv32String(name);

	for ( int i = 0; i < (int)mAxisDefs.size(); i++ )
		if ( hashedName == mAxisDefs[i].mHashedName )
			return i;

	return -1;
}

//*****************************************************************************
int VuInputManagerImpl::getButtonIndex(const char *name)
{
	VUUINT32 hashedName = VuHash::fnv32String(name);

	for ( int i = 0; i < (int)mButtonDefs.size(); i++ )
		if ( hashedName == mButtonDefs[i].mHashedName )
			return i;

	return -1;
}

//*****************************************************************************
void VuInputManagerImpl::loadMapping(int padIndex, const VuJsonContainer &data, eConfig config)
{
	// load axis mapping
	const VuJsonContainer &axisData = data["Axes"];
	for ( int i = 0; i < axisData.numMembers(); i++ )
	{
		const std::string &axisName = axisData.getMemberKey(i);
		if ( Axis *pAxis = getAxis(padIndex, axisName.c_str()) )
		{
			loadMapping(axisData[axisName]["+"], pAxis->mPosMappings[config]);
			loadMapping(axisData[axisName]["-"], pAxis->mNegMappings[config]);
		}
	}

	// load button mapping
	const VuJsonContainer &buttonData = data["Buttons"];
	for ( int i = 0; i < buttonData.numMembers(); i++ )
	{
		const std::string &buttonName = buttonData.getMemberKey(i);
		if ( Button *pButton = getButton(padIndex, buttonName.c_str()) )
		{
			loadMapping(buttonData[buttonName], pButton->mMappings[config]);
		}
	}
}

//*****************************************************************************
void VuInputManagerImpl::loadMapping(const VuJsonContainer &data, Mapping *pMappings)
{
	if ( data.isArray() )
	{
		int count = VuMin(data.size(), MAX_CHANNELS);
		for ( int iChannel = 0; iChannel < count; iChannel++ )
			loadMapping(data[iChannel], pMappings[iChannel]);
	}
	else if ( data.isString() )
	{
		loadMapping(data, pMappings[0]);
	}
}

//*****************************************************************************
void VuInputManagerImpl::loadMapping(const VuJsonContainer &data, Mapping &mapping)
{
	const char *strType = data.asCString();
	const char *strIndex = strstr(strType, "/");
	if ( strIndex )
	{
		strIndex++;

		if ( strncmp(strType, "+Axis", 5) == 0 )
		{
			mapping.mIndex = VuGamePad::IF()->getAxisIndex(strIndex);
			if ( mapping.mIndex >= 0 )
				mapping.mType = M_AXIS_POS;
		}
		else if ( strncmp(strType, "-Axis", 5) == 0 )
		{
			mapping.mIndex = VuGamePad::IF()->getAxisIndex(strIndex);
			if ( mapping.mIndex >= 0 )
				mapping.mType = M_AXIS_NEG;
		}
		else if ( strncmp(strType, "Button", 6) == 0 )
		{
			mapping.mIndex = VuGamePad::IF()->getButtonIndex(strIndex);
			if ( mapping.mIndex >= 0 )
				mapping.mType = M_BUTTON;
		}
		else if ( strncmp(strType, "Key", 3) == 0 )
		{
			mapping.mIndex = VuKeyboard::IF()->getKeyIndex(strIndex);
			if ( mapping.mIndex >= 0 )
				mapping.mType = M_KEY;
		}
	}
}

//*****************************************************************************
float VuInputManagerImpl::translateAxis(const AxisDef &axisDef, const VuGamePad::VuController &src, const Mapping *pMappings, int padIndex)
{
	float value = 0.0f;

	for ( int i = 0; i < MAX_CHANNELS; i++ )
	{
		const Mapping &mapping = pMappings[i];

		switch ( mapping.mType )
		{
			case M_AXIS_POS:
			{
				value += VuMax(src.mAxes[mapping.mIndex], 0.0f);
				break;
			}
			case M_AXIS_NEG:
			{
				value += -VuMin(src.mAxes[mapping.mIndex], 0.0f);
				break;
			}
			case M_BUTTON:
			{
				value += src.mButtons & (1<<mapping.mIndex) ? 1.0f : 0.0f;
				break;
			}
			case M_KEY:
			{
				value += (padIndex == 0) && VuKeyboard::IF()->isKeyDown(mapping.mIndex) ? 1.0f : 0.0f;
				break;
			}
			default:
				break;
		}
	}

	return value;
}

//*****************************************************************************
bool VuInputManagerImpl::translateButton(const ButtonDef &buttonDef, const VuGamePad::VuController &src, const Mapping *pMappings, int padIndex)
{
	bool value = false;

	for ( int i = 0; i < MAX_CHANNELS; i++ )
	{
		const Mapping &mapping = pMappings[i];

		switch ( mapping.mType )
		{
			case M_AXIS_POS:
			{
				value |= src.mAxes[mapping.mIndex] > 0.5f;
				break;
			}
			case M_AXIS_NEG:
			{
				value |= src.mAxes[mapping.mIndex] < -0.5f;
				break;
			}
			case M_BUTTON:
			{
				value |= src.mButtons & (1<<mapping.mIndex) ? true : false;
				break;
			}
			case M_KEY:
			{
				value |= (padIndex == 0) && VuKeyboard::IF()->isKeyDown(mapping.mIndex);
				break;
			}
			default:
				break;
		}
	}

	return value;
}
