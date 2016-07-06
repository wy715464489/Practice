//*****************************************************************************
//
//  Copyright (c) 2013-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  Config Manager
// 
//*****************************************************************************

#include "VuConfigManager.h"
#include "VuEngine/Method/VuMethodUtil.h"
#include "VuEngine/Assets/VuAssetFactory.h"
#include "VuEngine/Assets/VuDBAsset.h"
#include "VuEngine/Util/VuHash.h"
#include "VuEngine/Dev/VuDevMenu.h"
#include "VuEngine/Dev/VuDevConfig.h"


//*****************************************************************************
bool VuConfigManager::init(std::string deviceType)
{
	VuDBAsset *pDBAsset = VuAssetFactory::IF()->createAsset<VuDBAsset>("ConfigDB");

	// load parameters
	const VuJsonContainer &parameters = pDBAsset->getDB()["Parameters"];
	for ( int i = 0; i < parameters.size(); i++ )
	{
		const VuJsonContainer &parameter = parameters[i];
		const std::string &name = parameter["Name"].asString();
		const std::string &type = parameter["Type"].asString();
		VUUINT32 hashedName = VuHash::fnv32String(name.c_str());

		if ( type == "Bool" )
		{
			BoolExt &entry = mBools[hashedName];

			entry.mValue = entry.mPrevValue = entry.mDefaultValue = parameter["Default"].asBool();

			if ( VuDevMenu::IF() )
			{
				char str[256] = "Config/";
				VU_STRCAT(str, sizeof(str), name.c_str());
				VuDevMenu::IF()->addBool(str, entry.mValue);
			}

			if ( VuDevConfig::IF() )
			{
				VuDevConfig::IF()->getParam(name.c_str()).getValue(entry.mValue);
			}
		}
		else if ( type == "Float" )
		{
			FloatExt &entry = mFloats[hashedName];

			entry.mValue = entry.mPrevValue = entry.mDefaultValue = parameter["Default"].asFloat();
			entry.mMin = parameter["Min"].asFloat();
			entry.mMax = parameter["Max"].asFloat();
			entry.mStep = parameter["Step"].asFloat();

			if ( VuDevMenu::IF() )
			{
				char str[256] = "Config/";
				VU_STRCAT(str, sizeof(str), name.c_str());
				VuDevMenu::IF()->addFloat(str, entry.mValue, entry.mStep, entry.mMin, entry.mMax);
			}

			if ( VuDevConfig::IF() )
			{
				VuDevConfig::IF()->getParam(name.c_str()).getValue(entry.mValue);
			}
		}
		else if ( type == "Int" )
		{
			IntExt &entry = mInts[hashedName];

			entry.mValue = entry.mPrevValue = entry.mDefaultValue = parameter["Default"].asInt();
			entry.mMin = parameter["Min"].asInt();
			entry.mMax = parameter["Max"].asInt();
			entry.mStep = parameter["Step"].asInt();

			if ( VuDevMenu::IF() )
			{
				char str[256] = "Config/";
				VU_STRCAT(str, sizeof(str), name.c_str());
				VuDevMenu::IF()->addInt(str, entry.mValue, entry.mStep, entry.mMin, entry.mMax);
			}

			if ( VuDevConfig::IF() )
			{
				VuDevConfig::IF()->getParam(name.c_str()).getValue(entry.mValue);
			}
		}
	}

	// configure for device type
	if ( VuDevConfig::IF() )
		VuDevConfig::IF()->getParam("DeviceType").getValue(deviceType);

	VUPRINTF("DeviceType = %s\n", deviceType.c_str());

	const VuJsonContainer &deviceData = pDBAsset->getDB()["Devices"][deviceType];
	for ( int i = 0; i < deviceData.numMembers(); i++ )
	{
		const std::string &key = deviceData.getMemberKey(i);
		const VuJsonContainer &entry = deviceData[key];
		VUUINT32 hashedName = VuHash::fnv32String(key.c_str());

		if ( entry.isBool() )
		{
			Bools::iterator iter = mBools.find(hashedName);
			VUASSERT(iter != mBools.end(), "Config key not found!");
			iter->second.mValue = iter->second.mPrevValue = iter->second.mDefaultValue = entry.asBool();
		}
		else if ( entry.isFloat() )
		{
			Floats::iterator iter = mFloats.find(hashedName);
			VUASSERT(iter != mFloats.end(), "Config key not found!");
			iter->second.mValue = iter->second.mPrevValue = iter->second.mDefaultValue = entry.asFloat();
		}
		else if ( entry.isInt() )
		{
			Ints::iterator iter = mInts.find(hashedName);
			VUASSERT(iter != mInts.end(), "Config key not found!");
			iter->second.mValue = iter->second.mPrevValue = iter->second.mDefaultValue = entry.asInt();
		}
		else if ( entry.isObject() )
		{
			const VuJsonContainer &value = entry["Value"];
			if ( value.isFloat() )
			{
				Floats::iterator iter = mFloats.find(hashedName);
				VUASSERT(iter != mFloats.end(), "Config key not found!");
				iter->second.mValue = iter->second.mPrevValue = iter->second.mDefaultValue = value.asFloat();
				entry["Min"].getValue(iter->second.mMin);
				entry["Max"].getValue(iter->second.mMax);
				entry["Step"].getValue(iter->second.mStep);
			}
			else if ( value.isInt() )
			{
				Ints::iterator iter = mInts.find(hashedName);
				VUASSERT(iter != mInts.end(), "Config key not found!");
				iter->second.mValue = iter->second.mPrevValue = iter->second.mDefaultValue = value.asInt();
				entry["Min"].getValue(iter->second.mMin);
				entry["Max"].getValue(iter->second.mMax);
				entry["Step"].getValue(iter->second.mStep);
			}
		}
	}

	VuAssetFactory::IF()->releaseAsset(pDBAsset);

	return true;
}

//*****************************************************************************
void VuConfigManager::release()
{
	for ( Bools::iterator iter = mBools.begin(); iter != mBools.end(); iter++ )
		VUASSERT(iter->second.mHandlers.size() == 0, "Bool config leak");

	for ( Floats::iterator iter = mFloats.begin(); iter != mFloats.end(); iter++ )
		VUASSERT(iter->second.mHandlers.size() == 0, "Float config leak");

	for ( Ints::iterator iter = mInts.begin(); iter != mInts.end(); iter++ )
		VUASSERT(iter->second.mHandlers.size() == 0, "Int config leak");

	VUASSERT(mStack.size() == 0, "Config Manager push/pop mismatch!");
}

//*****************************************************************************
void VuConfigManager::tick()
{
	for ( Bools::iterator itBools = mBools.begin(); itBools != mBools.end(); itBools++ )
	{
		BoolExt &entry = itBools->second;
		if ( entry.mValue != entry.mPrevValue )
		{
			for ( BoolHandlers::iterator itHandlers = entry.mHandlers.begin(); itHandlers != entry.mHandlers.end(); itHandlers++ )
				(*itHandlers)->execute(entry.mValue);

			entry.mPrevValue = entry.mValue;
		}
	}

	for ( Floats::iterator itFloats = mFloats.begin(); itFloats != mFloats.end(); itFloats++ )
	{
		FloatExt &entry = itFloats->second;
		if ( entry.mValue != entry.mPrevValue )
		{
			for ( FloatHandlers::iterator itHandlers = entry.mHandlers.begin(); itHandlers != entry.mHandlers.end(); itHandlers++ )
				(*itHandlers)->execute(entry.mValue);

			entry.mPrevValue = entry.mValue;
		}
	}

	for ( Ints::iterator itInts = mInts.begin(); itInts != mInts.end(); itInts++ )
	{
		IntExt &entry = itInts->second;
		if ( entry.mValue != entry.mPrevValue )
		{
			for ( IntHandlers::iterator itHandlers = entry.mHandlers.begin(); itHandlers != entry.mHandlers.end(); itHandlers++ )
				(*itHandlers)->execute(entry.mValue);

			entry.mPrevValue = entry.mValue;
		}
	}
}

//*****************************************************************************
VuConfigManager::Bool *VuConfigManager::getBool(const char *key)
{
	Bools::iterator iter = mBools.find(VuHash::fnv32String(key));
	if ( iter != mBools.end() )
		return &iter->second;
	return VUNULL;
}

//*****************************************************************************
VuConfigManager::Float *VuConfigManager::getFloat(const char *key)
{
	Floats::iterator iter = mFloats.find(VuHash::fnv32String(key));
	if ( iter != mFloats.end() )
		return &iter->second;
	return VUNULL;
}

//*****************************************************************************
VuConfigManager::Int *VuConfigManager::getInt(const char *key)
{
	Ints::iterator iter = mInts.find(VuHash::fnv32String(key));
	if ( iter != mInts.end() )
		return &iter->second;
	return VUNULL;
}

//*****************************************************************************
void VuConfigManager::setBoolSetting(const char *key, bool value)
{
	if ( mStack.size() )
	{
		std::map<VUUINT32, bool>::iterator iter = mStack.front().mBools.find(VuHash::fnv32String(key));
		VUASSERT(iter != mStack.front().mBools.end(), "Config key not found!");
		iter->second = value;
	}
	else
	{
		getBool(key)->mValue = value;
	}
}

//*****************************************************************************
void VuConfigManager::setFloatSetting(const char *key, float value)
{
	if ( mStack.size() )
	{
		std::map<VUUINT32, float>::iterator iter = mStack.front().mFloats.find(VuHash::fnv32String(key));
		VUASSERT(iter != mStack.front().mFloats.end(), "Config key not found!");
		iter->second = value;
	}
	else
	{
		getFloat(key)->mValue = value;
	}
}

//*****************************************************************************
void VuConfigManager::setIntSetting(const char *key, int value)
{
	if ( mStack.size() )
	{
		std::map<VUUINT32, int>::iterator iter = mStack.front().mInts.find(VuHash::fnv32String(key));
		VUASSERT(iter != mStack.front().mInts.end(), "Config key not found!");
		iter->second = value;
	}
	else
	{
		getInt(key)->mValue = value;
	}
}

//*****************************************************************************
bool VuConfigManager::getBoolSetting(const char *key)
{
	if ( mStack.size() )
	{
		std::map<VUUINT32, bool>::iterator iter = mStack.front().mBools.find(VuHash::fnv32String(key));
		VUASSERT(iter != mStack.front().mBools.end(), "Config key not found!");
		return iter->second;
	}

	return getBool(key)->mValue;
}

//*****************************************************************************
float VuConfigManager::getFloatSetting(const char *key)
{
	if ( mStack.size() )
	{
		std::map<VUUINT32, float>::iterator iter = mStack.front().mFloats.find(VuHash::fnv32String(key));
		VUASSERT(iter != mStack.front().mFloats.end(), "Config key not found!");
		return iter->second;
	}

	return getFloat(key)->mValue;
}

//*****************************************************************************
int VuConfigManager::getIntSetting(const char *key)
{
	if ( mStack.size() )
	{
		std::map<VUUINT32, int>::iterator iter = mStack.front().mInts.find(VuHash::fnv32String(key));
		VUASSERT(iter != mStack.front().mInts.end(), "Config key not found!");
		return iter->second;
	}

	return getInt(key)->mValue;
}

//*****************************************************************************
void VuConfigManager::pushConfig()
{
	mStack.push_back(StackEntry());
	StackEntry &stackEntry = mStack.back();

	for ( Bools::iterator itBools = mBools.begin(); itBools != mBools.end(); itBools++ )
		stackEntry.mBools[itBools->first] = itBools->second.mValue;

	for ( Floats::iterator itFloats = mFloats.begin(); itFloats != mFloats.end(); itFloats++ )
		stackEntry.mFloats[itFloats->first] = itFloats->second.mValue;

	for ( Ints::iterator itInts = mInts.begin(); itInts != mInts.end(); itInts++ )
		stackEntry.mInts[itInts->first] = itInts->second.mValue;
}

//*****************************************************************************
void VuConfigManager::popConfig()
{
	VUASSERT(mStack.size(), "Config Manager push/pop mismatch!");

	StackEntry &stackEntry = mStack.back();

	for ( Bools::iterator itBools = mBools.begin(); itBools != mBools.end(); itBools++ )
		itBools->second.mValue = stackEntry.mBools[itBools->first];

	for ( Floats::iterator itFloats = mFloats.begin(); itFloats != mFloats.end(); itFloats++ )
		itFloats->second.mValue = stackEntry.mFloats[itFloats->first];

	for ( Ints::iterator itInts = mInts.begin(); itInts != mInts.end(); itInts++ )
		itInts->second.mValue = stackEntry.mInts[itInts->first];

	mStack.pop_back();

	tick();
}

//*****************************************************************************
void VuConfigManager::setConfig(const std::string &deviceType)
{
	VuDBAsset *pDBAsset = VuAssetFactory::IF()->createAsset<VuDBAsset>("ConfigDB");

	const VuJsonContainer &deviceData = pDBAsset->getDB()["Devices"][deviceType];
	for ( int i = 0; i < deviceData.numMembers(); i++ )
	{
		const std::string &key = deviceData.getMemberKey(i);
		const VuJsonContainer &entry = deviceData[key];
		VUUINT32 hashedName = VuHash::fnv32String(key.c_str());

		if ( entry.isBool() )
		{
			Bools::iterator iter = mBools.find(hashedName);
			VUASSERT(iter != mBools.end(), "Config key not found!");
			iter->second.mValue = entry.asBool();
		}
		else if ( entry.isFloat() )
		{
			Floats::iterator iter = mFloats.find(hashedName);
			VUASSERT(iter != mFloats.end(), "Config key not found!");
			iter->second.mValue = entry.asFloat();
		}
		else if ( entry.isInt() )
		{
			Ints::iterator iter = mInts.find(hashedName);
			VUASSERT(iter != mInts.end(), "Config key not found!");
			iter->second.mValue = entry.asInt();
		}
		else if ( entry.isObject() )
		{
			const VuJsonContainer &value = entry["Value"];
			if ( value.isFloat() )
			{
				Floats::iterator iter = mFloats.find(hashedName);
				VUASSERT(iter != mFloats.end(), "Config key not found!");
				iter->second.mValue = value.asFloat();
			}
			else if ( value.isInt() )
			{
				Ints::iterator iter = mInts.find(hashedName);
				VUASSERT(iter != mInts.end(), "Config key not found!");
				iter->second.mValue = value.asInt();
			}
		}
	}

	VuAssetFactory::IF()->releaseAsset(pDBAsset);

	tick();
}

//*****************************************************************************
void VuConfigManager::unregisterBoolHandler(const char *key, void *pObj)
{
	Bools::iterator iter = mBools.find(VuHash::fnv32String(key));
	VUASSERT(iter != mBools.end(), "Config key not found!");
	iter->second.mHandlers.remove_if(isMethodOfObjectWithDelete(pObj));
}

//*****************************************************************************
void VuConfigManager::unregisterFloatHandler(const char *key, void *pObj)
{
	Floats::iterator iter = mFloats.find(VuHash::fnv32String(key));
	VUASSERT(iter != mFloats.end(), "Config key not found!");
	iter->second.mHandlers.remove_if(isMethodOfObjectWithDelete(pObj));
}

//*****************************************************************************
void VuConfigManager::unregisterIntHandler(const char *key, void *pObj)
{
	Ints::iterator iter = mInts.find(VuHash::fnv32String(key));
	VUASSERT(iter != mInts.end(), "Config key not found!");
	iter->second.mHandlers.remove_if(isMethodOfObjectWithDelete(pObj));
}

//*****************************************************************************
void VuConfigManager::registerBoolHandler(const char *key, VuMethodInterface1<void, bool> *pHandler)
{
	Bools::iterator iter = mBools.find(VuHash::fnv32String(key));
	VUASSERT(iter != mBools.end(), "Config key not found!");
	iter->second.mHandlers.push_back(pHandler);
}

//*****************************************************************************
void VuConfigManager::registerFloatHandler(const char *key, VuMethodInterface1<void, float> *pHandler)
{
	Floats::iterator iter = mFloats.find(VuHash::fnv32String(key));
	VUASSERT(iter != mFloats.end(), "Config key not found!");
	iter->second.mHandlers.push_back(pHandler);
}

//*****************************************************************************
void VuConfigManager::registerIntHandler(const char *key, VuMethodInterface1<void, int> *pHandler)
{
	Ints::iterator iter = mInts.find(VuHash::fnv32String(key));
	VUASSERT(iter != mInts.end(), "Config key not found!");
	iter->second.mHandlers.push_back(pHandler);
}
