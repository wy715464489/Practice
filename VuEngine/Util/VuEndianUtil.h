//*****************************************************************************
//
//  Copyright (c) 2007-2008 Ralf Knoesel
//  Copyright (c) 2008-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  Utility functionality to deal with endianness.
// 
//*****************************************************************************

#pragma once


namespace VuEndianUtil
{
	//*****************************************************************************
	// swap
	//*****************************************************************************

	template <class T>
	void swap(const T &srcVal, T &dstVal)
	{
		const VUBYTE *src = (VUBYTE *)&srcVal;
		VUBYTE *dst = (VUBYTE *)&dstVal;

		if ( sizeof(T) == 1 )
		{
			dst[0] = src[0];
		}
		else if ( sizeof(T) == 2 )
		{
			dst[0] = src[1];
			dst[1] = src[0];
		}
		else if ( sizeof(T) == 4 )
		{
			dst[0] = src[3];
			dst[1] = src[2];
			dst[2] = src[1];
			dst[3] = src[0];
		}
		else
		{
			src += sizeof(T) - 1;
			for ( unsigned int i = 0; i < sizeof(T); i++ )
				*dst++ = *src--;
		}
	}

	template <class T>
	T swap(const T &val)
	{
		T tmp;
		swap<T>(val, tmp);
		return tmp;
	}

	template <class T>
	void swapInPlace(T &val)
	{
		T tmp;
		swap<T>(val, tmp);
		val = tmp;
	}


	//*****************************************************************************
	// copy
	//*****************************************************************************

	template <class T>
	void copy(const T &srcVal, T &dstVal)
	{
		const VUBYTE *src = (VUBYTE *)&srcVal;
		VUBYTE *dst = (VUBYTE *)&dstVal;

		for ( int i = 0; i < sizeof(T); i++ )
			*dst++ = *src++;
	}


	//*****************************************************************************
	// conditional swap
	//*****************************************************************************

	template <class T>
	void swapIfLittle(const T &srcVal, T &dstVal)
	{
		#if VU_LITTLE_ENDIAN
			swap(srcVal, dstVal);
		#elif VU_BIG_ENDIAN
			copy(srcVal, dstVal);
		#else
			#error Endianness not defined!
		#endif
	}

	template <class T>
	T swapIfLittle(const T &val)
	{
		T tmp;
		swapIfLittle<T>(val, tmp);
		return tmp;
	}

	template <class T>
	void swapInPlaceIfLittle(T &val)
	{
		T tmp;
		swapIfLittle<T>(val, tmp);
		val = tmp;
	}
}