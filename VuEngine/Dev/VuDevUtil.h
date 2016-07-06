//*****************************************************************************
//
//  Copyright (c) 2008-2008 Ralf Knoesel
//  Copyright (c) 2008-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  DevUtil functionality
// 
//*****************************************************************************

#pragma once


#if VU_DISABLE_DEV_UTIL

	class VuDevBoolOnce
	{
	public:
		inline operator bool () { return false; }
	};

#else

	//*****************************************************************************
	// DevBoolOnce may be used statically to execute some code only once.
	//*****************************************************************************
	class VuDevBoolOnce
	{
	public:
		VuDevBoolOnce() : mbBool(true) {}
		inline operator bool ();

	private:
		bool	mbBool;
	};

	inline VuDevBoolOnce::operator bool ()
	{
		if ( mbBool )
		{
			mbBool = false;
			return true;
		}

		return false;
	}

#endif // VU_DISABLE_DEV_UTIL
