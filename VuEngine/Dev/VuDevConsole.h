//*****************************************************************************
//
//  Copyright (c) 2008-2008 Ralf Knoesel
//  Copyright (c) 2008-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  Dev Console
// 
//*****************************************************************************

#pragma once

#include "VuEngine/VuSystemComponent.h"

class VuEngine;


#if VU_DISABLE_DEV_CONSOLE

	class VuDevConsole : public VuSystemComponent
	{
		DECLARE_SYSTEM_COMPONENT(VuDevConsole)
	protected:
		friend class VuEngine;
		virtual bool init()	{ return true; }
	public:
		virtual void	show(bool bShow) {}
	};

#else

	class VuDevConsole : public VuSystemComponent
	{
		DECLARE_SYSTEM_COMPONENT(VuDevConsole)
	protected:
		// called by engine
		friend class VuEngine;
		virtual bool init()	{ return true; }

	public:
		virtual void	show(bool bShow) = 0;
	};

#endif // VU_DISABLE_DEV_CONSOLE
