//*****************************************************************************
//
//  Copyright (c) 2008-2008 Ralf Knoesel
//  Copyright (c) 2008-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  Dev Stats
// 
//*****************************************************************************

#pragma once

#include "VuEngine/VuSystemComponent.h"
#include "VuEngine/Math/VuRect.h"

class VuDevStatPage;
class VuEngine;


#if VU_DISABLE_DEV_STAT

	class VuDevStat : public VuSystemComponent
	{
		DECLARE_SYSTEM_COMPONENT(VuDevStat)
	protected:
		friend class VuEngine;
		virtual bool init() { return true; }
	public:
		virtual void			addPage(const char *strPage, const VuRect &rect) {};
		virtual VuDevStatPage	*getPage(const char *strPage) { return VUNULL; }
		virtual VuDevStatPage	*getCurPage() { return VUNULL; }
	};

	class VuDevStatPage
	{
	public:
		virtual const std::string	&getName() const { return mEmptyString; }
		virtual void				clear() {}
		virtual void				printf(const char *fmt, ...) {}
		virtual const std::string	&getText() const { return mEmptyString; }
		std::string					mEmptyString;
	};

#else

	class VuDevStat : public VuSystemComponent
	{
		DECLARE_SYSTEM_COMPONENT(VuDevStat)

	protected:
		// called by engine
		friend class VuEngine;
		virtual bool init() = 0;

	public:
		virtual void			addPage(const char *strPage, const VuRect &rect) = 0;
		virtual VuDevStatPage	*getPage(const char *strPage) = 0;
		virtual VuDevStatPage	*getCurPage() = 0;
	};

	class VuDevStatPage
	{
	public:
		virtual ~VuDevStatPage() {}		// some compilers warn about deleting base class objects with a non-virtual destructor

		virtual const std::string	&getName() const = 0;
		virtual void				clear() = 0;
		virtual void				printf(const char *fmt, ...) = 0;
		virtual const std::string	&getText() const = 0;
	};

#endif // VU_DISABLE_DEV_STAT
