//*****************************************************************************
//
//  Copyright (c) 2005-2008 Ralf Knoesel
//  Copyright (c) 2008-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  Abstract interface to system components.
// 
//*****************************************************************************

#pragma once


class VuSystemComponent
{
public:
	virtual ~VuSystemComponent() {}

	virtual void	postInit()		{};
	virtual void	preRelease()	{};
	virtual void	release()		{};

	virtual void	resetIF() = 0;
};


//*****************************************************************************
// Macro used to declare a system component.
#define DECLARE_SYSTEM_COMPONENT(System)					\
public:														\
	static System	*IF()		{ return mpInterface; }		\
	virtual void	resetIF()	{ mpInterface = VUNULL; }	\
	static System	*mpInterface;							\
private:


//*****************************************************************************
// Macro used to implement a system component.
#define IMPLEMENT_SYSTEM_COMPONENT(System, PlatformSystem)	\
System *System::mpInterface = VUNULL;						\
System *Create##System##Interface()							\
{															\
	System *pIF = new PlatformSystem;						\
	System::mpInterface = pIF;								\
	return pIF;												\
}
