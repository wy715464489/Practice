//*****************************************************************************
//
//  Copyright (c) 2008-2008 Ralf Knoesel
//  Copyright (c) 2008-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  Dev Profile
// 
//*****************************************************************************

#pragma once

#include "VuEngine/VuSystemComponent.h"

class VuEngine;


#if VU_DISABLE_DEV_PROFILE

	class VuDevProfile : VuSystemComponent
	{
		DECLARE_SYSTEM_COMPONENT(VuDevProfile)
	protected:
		friend class VuEngine;
		virtual bool	init() { return true; }
	public:
		virtual void	beginSim() {}
		virtual void	endSim() {}
		virtual void	beginGfx() {}
		virtual void	endGfx() {}
		virtual void	synchronizeGfx() {}
		virtual void	beginDyn(float overlapTime) {}
		virtual void	endDyn() {}
		virtual void	synchronizeDyn() {}
		virtual void	beginWater(float overlapTime) {}
		virtual void	endWater() {}
		virtual void	synchronizeWater() {}
	};

	#define VU_PROFILE_SIM(name)
	#define VU_PROFILE_GFX(name)
	#define VU_PROFILE_DYN(name)
	#define VU_PROFILE_WATER(name)

#else

	class VuDevProfile : VuSystemComponent
	{
		DECLARE_SYSTEM_COMPONENT(VuDevProfile)

	protected:
		// called by engine
		friend class VuEngine;
		virtual bool	init() { return true; }

	public:
		virtual void	beginSim() = 0;
		virtual void	endSim() = 0;
		virtual void	beginGfx() = 0;
		virtual void	endGfx() = 0;
		virtual void	synchronizeGfx() = 0;
		virtual void	beginDyn(float overlapTime) = 0;
		virtual void	endDyn() = 0;
		virtual void	synchronizeDyn() = 0;
		virtual void	beginWater(float overlapTime) = 0;
		virtual void	endWater() = 0;
		virtual void	synchronizeWater() = 0;
	};


	//*****************************************************************************
	// Profile sample
	//*****************************************************************************
	class VuDevProfileSimSample
	{
	public:
		VuDevProfileSimSample(const char *strName);
		~VuDevProfileSimSample();
	};
	class VuDevProfileGfxSample
	{
	public:
		VuDevProfileGfxSample(const char *strName);
		~VuDevProfileGfxSample();
	};
	class VuDevProfileDynSample
	{
	public:
		VuDevProfileDynSample(const char *strName);
		~VuDevProfileDynSample();
	};
	class VuDevProfileWaterSample
	{
	public:
		VuDevProfileWaterSample(const char *strName);
		~VuDevProfileWaterSample();
	};

	#define VU_PROFILE_SIM(name) VuDevProfileSimSample profileSample(name)
	#define VU_PROFILE_GFX(name) VuDevProfileGfxSample profileSample(name)
	#define VU_PROFILE_DYN(name) VuDevProfileDynSample profileSample(name)
	#define VU_PROFILE_WATER(name) VuDevProfileWaterSample profileSample(name)

#endif // VU_DISABLE_DEV_PROFILE
