//*****************************************************************************
//
//  Copyright (c) 2011-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  Touchscreen hardware abstration layer
// 
//*****************************************************************************

#pragma once

#include "VuEngine/VuSystemComponent.h"

class VuEngine;
class VuVector2;


class VuTouch : public VuSystemComponent
{
	DECLARE_SYSTEM_COMPONENT(VuTouch)

protected:
	// called by engine
	friend class VuEngine;
	virtual bool init() { return true; }

public:
	VuTouch();

	enum eSpecial { SPECIAL_BACK_PRESSED };

	// registration of callbacks
	class Callback
	{
	public:
		virtual void onTouchDown(const VuVector2 &touch) {}
		virtual void onTouchUp(const VuVector2 &touch) {}
		virtual void onTouchMove() {}
		virtual void onTouchSpecial(eSpecial special) {}
	};
	void			addCallback(Callback *pCB);
	void			removeCallback(Callback *pCB);
	void			setCallbackPriority(Callback *pCB, VUUINT32 priority);

	void			addLowLevelCallback(Callback *pCB);
	void			removeLowLevelCallback(Callback *pCB);

	bool			hasFocus(Callback *pCB);
	VUUINT32		getFocusPriority() { return mFocusPriority; }

	virtual int		getTouchCount(VUUINT32 priority = 0xffffffff) = 0;
	virtual void	getTouchRaw(int index, VuVector2 &touch) = 0;
	void			getTouch(int index, VuVector2 &touch);

protected:
	class VuCallbackEntry
	{
	public:
		VuCallbackEntry(Callback *pCallback) : mpCallback(pCallback), mPriority(0) {}
		Callback	*mpCallback;
		VUUINT32	mPriority;
	};
	typedef std::vector<VuCallbackEntry> Callbacks;

	void			onTouchDownInternal(const VuVector2 &touch);
	void			onTouchUpInternal(const VuVector2 &touch);
	void			onTouchMoveInternal();
	void			onTouchSpecialInternal(eSpecial special);
	void			recalculateFocusPriority();

	Callbacks		mCallbacks;
	Callbacks		mLowLevelCallbacks;
	VUUINT32		mFocusPriority;
};