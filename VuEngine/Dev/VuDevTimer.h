//*****************************************************************************
//
//  Copyright (c) 2009-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  DevTimer class
// 
//*****************************************************************************

#pragma once

#if VU_DISABLE_DEV_TIMER

	class VuDevTimer : public VuSystemComponent
	{
		DECLARE_SYSTEM_COMPONENT(VuDevTimer)
	protected:
		friend class VuEngine;
		virtual bool init() { return true; }
	public:
		virtual void show() {}
		virtual void hide() {}
		virtual void pause() {}
		virtual void unpause() {}
		virtual void reset() {}
	};

#else

	class VuDevTimer : public VuSystemComponent
	{
		DECLARE_SYSTEM_COMPONENT(VuDevTimer)

	protected:
		// called by engine
		friend class VuEngine;
		virtual bool init() = 0;

	public:

		virtual void show() = 0;
		virtual void hide() = 0;
		virtual void pause() = 0;
		virtual void unpause() = 0;
		virtual void reset() = 0;
	};

#endif // VU_DISABLE_DEV_TIMER
