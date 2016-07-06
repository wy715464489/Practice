//*****************************************************************************
//
//  Copyright (c) 2013-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  Config Manager
// 
//*****************************************************************************

#pragma once

#include <float.h>
#include "VuEngine/VuSystemComponent.h"
#include "VuEngine/Method/VuMethod.h"

class VuEngine;

class VuConfigManager : public VuSystemComponent
{
	DECLARE_SYSTEM_COMPONENT(VuConfigManager)

protected:
	// called by engine
	friend class VuEngine;
	virtual bool	init(std::string deviceType);
	virtual void	release();

	void	tick();

public:
	struct Bool
	{
		bool			mValue;
		bool			mDefaultValue;
	};
	struct Float
	{
		float			mValue;
		float			mDefaultValue;
		float			mMin;
		float			mMax;
		float			mStep;
	};
	struct Int
	{
		int			mValue;
		int			mDefaultValue;
		int			mMin;
		int			mMax;
		int			mStep;
	};

	Bool	*getBool(const char *key);
	Float	*getFloat(const char *key);
	Int		*getInt(const char *key);

	void	setBoolSetting(const char *key, bool value);
	void	setFloatSetting(const char *key, float value);
	void	setIntSetting(const char *key, int value);

	bool	getBoolSetting(const char *key);
	float	getFloatSetting(const char *key);
	int		getIntSetting(const char *key);

	void	pushConfig();
	void	popConfig();
	void	setConfig(const std::string &deviceType);
	int		stackSize() { return (int)mStack.size(); }

	template<class T>
	void	registerBoolHandler(const char *key, T *pObj, void (T::*method)(bool value));

	template<class T>
	void	registerFloatHandler(const char *key, T *pObj, void (T::*method)(float value));

	template<class T>
	void	registerIntHandler(const char *key, T *pObj, void (T::*method)(int value));

	void	unregisterBoolHandler(const char *key, void *pObj);
	void	unregisterFloatHandler(const char *key, void *pObj);
	void	unregisterIntHandler(const char *key, void *pObj);

private:
	typedef VuMethodInterface1<void, bool> BoolHandler;
	typedef VuMethodInterface1<void, float> FloatHandler;
	typedef VuMethodInterface1<void, int> IntHandler;
	typedef std::list<BoolHandler *> BoolHandlers;
	typedef std::list<FloatHandler *> FloatHandlers;
	typedef std::list<IntHandler *> IntHandlers;
	struct BoolExt : Bool
	{
		bool			mPrevValue;
		BoolHandlers	mHandlers;
	};
	struct FloatExt : Float
	{
		float			mPrevValue;
		FloatHandlers	mHandlers;
	};
	struct IntExt : Int
	{
		int			mPrevValue;
		IntHandlers	mHandlers;
	};
	typedef std::map<VUUINT32, BoolExt> Bools;
	typedef std::map<VUUINT32, FloatExt> Floats;
	typedef std::map<VUUINT32, IntExt> Ints;

	void	registerBoolHandler(const char *key, BoolHandler *pHandler);
	void	registerFloatHandler(const char *key, FloatHandler *pHandler);
	void	registerIntHandler(const char *key, IntHandler *pHandler);

	Bools	mBools;
	Floats	mFloats;
	Ints	mInts;

	struct StackEntry
	{
		std::map<VUUINT32, bool> mBools;
		std::map<VUUINT32, float> mFloats;
		std::map<VUUINT32, int> mInts;
	};
	typedef std::deque<StackEntry> Stack;
	Stack	mStack;
};


//*****************************************************************************
template<class T>
void VuConfigManager::registerBoolHandler(const char *key, T *pObj, void (T::*method)(bool value))
{
	// create tick handler
	VuMethodInterface1<void, bool> *pHandler = new VuMethod1<T, void, bool>(pObj, method);

	// register
	registerBoolHandler(key, pHandler);
}

//*****************************************************************************
template<class T>
void VuConfigManager::registerFloatHandler(const char *key, T *pObj, void (T::*method)(float value))
{
	// create tick handler
	VuMethodInterface1<void, float> *pHandler = new VuMethod1<T, void, float>(pObj, method);

	// register
	registerFloatHandler(key, pHandler);
}

//*****************************************************************************
template<class T>
void VuConfigManager::registerIntHandler(const char *key, T *pObj, void (T::*method)(int value))
{
	// create tick handler
	VuMethodInterface1<void, int> *pHandler = new VuMethod1<T, void, int>(pObj, method);

	// register
	registerIntHandler(key, pHandler);
}
