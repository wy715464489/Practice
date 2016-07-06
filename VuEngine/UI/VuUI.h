//*****************************************************************************
//
//  Copyright (c) 2008-2008 Ralf Knoesel
//  Copyright (c) 2008-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  Interface class to UI library.
// 
//*****************************************************************************

#pragma once

#include "VuEngine/VuSystemComponent.h"
#include "VuEngine/Math/VuMatrix.h"

class VuMatrix;
class VuFont;
class VuFontAsset;
class VuEntity;


class VuUI : public VuSystemComponent
{
	DECLARE_SYSTEM_COMPONENT(VuUI)

	friend class VuEngine;
	virtual bool init();
	virtual void release();

public:
	VuUI();

	// authoring aspect ratio
	void			setAuthoringScreenScale(float scaleX, float scaleY);
	const VuVector2	&getAuthoringScreenScale()	{ return mAuthoringScreenScale; }
	float			getAuthoringAspectRatio()	{ return mAuthoringAspectRatio; }

	// events
	void			registerEvent(const char *strName);
	int				getEventCount() { return (int)mEvents.size(); }
	std::string		&getEvent(int index) { return mEvents[index]; }

	// matrix override
	void			setCropMatrix(const VuMatrix &mat);
	const VuMatrix	&getCropMatrix()		{ return mCropMatrix; }
	const VuMatrix	&getInvCropMatrix()		{ return mInvCropMatrix; }

	// text scale override
	void			setTextScale(float scale)	{ mTextScale = scale; }
	float			getTextScale()				{ return mTextScale; }

	// default font
	VuFont			*getDefaultFont();

	// focus
	VuEntity		*getFocus() { return mpFocusEntity; }
	void			setFocus(VuEntity *pEntity);
	void			pushFocus();
	void			popFocus();
	int				getFocusStackSize() { return (int)mFocusStack.size(); }

private:
	void			tickInput(float fdt);

	typedef std::vector<std::string> Events;
	typedef std::stack<VuEntity *> FocusStack;

	Events		mEvents;
	VuMatrix	mCropMatrix;
	VuMatrix	mInvCropMatrix;
	float		mTextScale;
	VuVector2	mAuthoringScreenScale;
	float		mAuthoringAspectRatio;
	VuFontAsset	*mpDefaultFontAsset;
	VuEntity	*mpFocusEntity;
	VuEntity	*mpFocusPendingEntity;
	FocusStack	mFocusStack;
};

