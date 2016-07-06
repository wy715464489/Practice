//*****************************************************************************
//
//  Copyright (c) 2013-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  DialogManager class
// 
//*****************************************************************************

#pragma once

#include "VuEngine/VuSystemComponent.h"
#include "VuEngine/Objects/VuRTTI.h"
#include "VuEngine/Objects/VuRefObj.h"
#include "VuEngine/Events/VuEventMap.h"
#include "VuEngine/Util/VuFSM.h"
#include "VuEngine/Util/VuColor.h"

class VuProjectAsset;
class VuProject;
class VuUIScreenEntity;


class VuDialog : public VuRefObj
{
	DECLARE_RTTI

public:
	VuDialog() : mpProject(VUNULL), mpScreen(VUNULL), mDestroyed(false), mpCallback(VUNULL), mPauseGame(false), mClosed(false) {}
	virtual ~VuDialog() {}

	// callback
	class Callback { public: virtual void onDialogClosed(VuDialog *pDialog) = 0; };
	void				setCallback(Callback *pCB)	{ mpCallback = pCB; }

	// configuration
	void				setPauseGame(bool pause) { mPauseGame = pause; }

	// actions
	void				close() { mClosed = true; }

	// result
	void				setResult(const char *result) { mResult = result; }
	const std::string	&getResult() const { return mResult; }

protected:
	friend class VuDialogManager;

	void				tick(float fdt);
	void				draw();

	std::string			mProjectAsset;
	VuProject			*mpProject;
	VuUIScreenEntity	*mpScreen;
	bool				mDestroyed;
	Callback			*mpCallback;
	bool				mPauseGame;
	bool				mClosed;
	std::string			mResult;
};


class VuDialogManager : public VuSystemComponent
{
	DECLARE_SYSTEM_COMPONENT(VuDialogManager)
	DECLARE_EVENT_MAP

protected:
	// called by game
	friend class VuEngine;
	virtual bool init();
	virtual void preRelease();
	virtual void release();

public:
	VuDialogManager();

	VuDialog			*create(const char *projectAsset);
	void				destroy(VuDialog *pDialog);

	VuDialog			*getActiveDialog() { return mpActiveDialog; }
	float				getFadeValue() { return mFadeValue; }

	void				pushPauseRequest()	{ mPauseRequestCount++; }
	void				popPauseRequest()	{ mPauseRequestCount--; VUASSERT(mPauseRequestCount >= 0, "VuDialogManager::popPauseRequest() push/pop mismatch"); }

	void				setBackgroundColor(const VuColor &color) { mBackgroundColor = color; }
	void				setMinFadeTime(float minFadeTime) { mMinFadeTime = minFadeTime; }

protected:
	// event handlers
	void				OnExitApp(const VuParams &params) { preRelease(); }

	void				tick(float fdt);
	void				draw();

	void				releaseActiveDialog();

	// FSM
	void				onFadeInEnter();
	void				onFadeInTick(float fdt);
	void				onActiveEnter();
	void				onActiveTick(float fdt);
	void				onActiveExit();
	void				onFadeOutEnter();
	void				onFadeOutTick(float fdt);
	void				onFadeOutExit();

	typedef std::queue<VuDialog *> DialogQueue;
	DialogQueue			mDialogQueue;

	VuFSM				mFSM;
	VuColor				mBackgroundColor;
	float				mMinFadeTime;
	float				mFadeValue;
	VuDialog			*mpActiveDialog;
	int					mPauseRequestCount;
};
