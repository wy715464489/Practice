//*****************************************************************************
//
//  Copyright (c) 2008-2008 Ralf Knoesel
//  Copyright (c) 2008-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  Dev Menu
// 
//*****************************************************************************

#pragma once

#include <float.h>
#include "VuEngine/VuSystemComponent.h"

class VuEngine;
class VuColor;


#if VU_DISABLE_DEV_MENU

	class VuDevMenu : public VuSystemComponent
	{
		DECLARE_SYSTEM_COMPONENT(VuDevMenu)

	protected:
		friend class VuEngine;
		virtual bool init() { return true; }

	public:
		struct IntEnumChoice
		{
			const char	*mpName;
			int			mValue;
		};

		class Callback { public: virtual void onDevMenu(int param) = 0; };

		virtual void	addBool(const char *strPath, bool &value) {}
		virtual void	addInt(const char *strPath, int &value, int valStep = 1, int valMin = INT_MIN, int valMax = INT_MAX) {}
		virtual void	addFloat(const char *strPath, float &value, float valStep = 1.0f, float valMin = -FLT_MAX, float valMax = FLT_MAX) {}
		virtual void	addUInt8(const char *strPath, VUUINT8 &value, int valStep = 1, int valMin = 0, int valMax = 255) {}
		virtual void	addColor3(const char *strPath, VuColor &value) {}
		virtual void	addColor4(const char *strPath, VuColor &value) {}
		virtual void	addIntEnum(const char *strPath, int &value, const IntEnumChoice *choices) {}
		virtual void	addCallback(const char *strPath, Callback *pCB, int param) {}

		virtual void	removeNode(const char *strPath) {};
	};

#else

	class VuDevMenu : public VuSystemComponent
	{
		DECLARE_SYSTEM_COMPONENT(VuDevMenu)

	protected:
		// called by engine
		friend class VuEngine;
		virtual bool init() = 0;

	public:
		struct IntEnumChoice
		{
			const char	*mpName;
			int			mValue;
		};

		class Callback { public: virtual void onDevMenu(int param) = 0; };

		virtual void	addBool(const char *strPath, bool &value) = 0;
		virtual void	addInt(const char *strPath, int &value, int valStep = 1, int valMin = INT_MIN, int valMax = INT_MAX) = 0;
		virtual void	addFloat(const char *strPath, float &value, float valStep = 1.0f, float valMin = -FLT_MAX, float valMax = FLT_MAX) = 0;
		virtual void	addUInt8(const char *strPath, VUUINT8 &value, int valStep = 1, int valMin = 0, int valMax = 255) = 0;
		virtual void	addColor3(const char *strPath, VuColor &value) = 0;
		virtual void	addColor4(const char *strPath, VuColor &value) = 0;
		virtual void	addIntEnum(const char *strPath, int &value, const IntEnumChoice *choices) = 0;
		virtual void	addCallback(const char *strPath, Callback *pCB, int param) = 0;

		virtual void	removeNode(const char *strPath) = 0;
	};

#endif //VU_DISABLE_DEV_MENU
