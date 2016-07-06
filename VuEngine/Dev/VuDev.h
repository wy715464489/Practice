//*****************************************************************************
//
//  Copyright (c) 2005-2008 Ralf Knoesel
//  Copyright (c) 2008-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  Interface class to Debug library.
// 
//*****************************************************************************

#pragma once

#include "VuEngine/VuSystemComponent.h"

class VuEngine;
class VuColor;
class VuFont;
class VuVector3;
class VuMatrix;
class VuJsonContainer;

#if VU_DISABLE_DEV
	class VuDev : public VuSystemComponent
	{
		DECLARE_SYSTEM_COMPONENT(VuDev)
	protected:
		friend class VuEngine;
		bool init() { return true; }
	public:
		VuFont				*getFont()	{ return VUNULL; }
		void				printf(const VuVector3 &pos, int flags, const VuColor &color, const char *fmt, ...) {}
		void				drawLine(const VuVector3 &v0, const VuVector3 &v1, const VuColor &color) {}
		void				drawLine(const VuVector3 &v0, const VuColor &c0, const VuVector3 &v1, const VuColor &c1) {}
		void				drawBox(const VuMatrix &mat, const VuColor &color) {}
		void				drawSphere(const VuVector3 &vPos, float fRadius, const VuColor &color, int axisSubdivCount, int heightSubdivCount)	{}
		void				drawCylinder(const VuColor &color, float fHeight, float fRadius, int numSides, const VuMatrix &matModel) {}
		void				drawArrow(const VuColor &color, float fLength, float fHeadLength, float fHeadWidth, const VuMatrix &matModel) {}
		void				drawArc(const VuVector3 &pos, const VuVector3 &axis, const VuVector3 &right, float angle, float radius, int segmentCount, const VuColor &color) {}
		void				printJsonContainer(const VuJsonContainer &container) const {}

		const std::string	&getBuildNumber() const { return mBuildNumber; }
		void				setBuildNumber(int buildNumber) {}

	private:
		std::string		mBuildNumber;
	};

#else

	class VuDev : public VuSystemComponent
	{
		DECLARE_SYSTEM_COMPONENT(VuDev)

	protected:

		// called by engine
		friend class VuEngine;
		virtual bool init() = 0;

	public:

		// dev font
		virtual VuFont					*getFont() = 0;

		// printf to screen
		virtual void					printf(const VuVector3 &pos, int flags, const VuColor &color, const char *fmt, ...) = 0;

		// draw debug line
		virtual void					drawLine(const VuVector3 &v0, const VuVector3 &v1, const VuColor &color) = 0;
		virtual void					drawLine(const VuVector3 &v0, const VuColor &c0, const VuVector3 &v1, const VuColor &c1) = 0;

		// draw debug box (use matrix scaling for size)
		virtual void					drawBox(const VuMatrix &mat, const VuColor &color) = 0;

		// draw debug sphere
		virtual void					drawSphere(const VuVector3 &vPos,
												float fRadius,
												const VuColor &color,
												int axisSubdivCount,
												int heightSubdivCount) = 0;

		// draw debug cylinder
		virtual void					drawCylinder(const VuColor &color, float fHeight, float fRadius, int numSides, const VuMatrix &matModel) = 0;

		// draw debug arrow
		virtual void					drawArrow(const VuColor &color, float fLength, float fHeadLength, float fHeadWidth, const VuMatrix &matModel) = 0;

		// draw debug arc
		virtual void					drawArc(const VuVector3 &pos, 
												const VuVector3 &axis, 
												const VuVector3 &right, 
												float angle, 
												float radius, 
												int segmentCount, 
												const VuColor &color) = 0;

		// for debugging the contents of a Json container
		virtual void					printJsonContainer(const VuJsonContainer &container) const = 0;

		virtual const std::string		&getBuildNumber() const = 0;
		virtual void					setBuildNumber(int buildNumber) = 0;
	};
#endif // VU_DISABLE_DEV
