//*****************************************************************************
//
//  Copyright (c) 2014-2014 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  MessageBoxManager class
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
class VuTextureAsset;
class VuDBAsset;


class VuMessageBoxParams
{
public:
	VuMessageBoxParams();

	std::string	mType;
	std::string	mHeading;
	std::string	mBody;
	std::string	mTextA;
	std::string	mTextB;
	std::string	mImage;
	bool		mPauseGame;
	VUUINT32	mUserData;
};

class VuMessageBox : public VuRefObj
{
	DECLARE_RTTI

public:
	VuMessageBox() : mpProject(VUNULL), mpScreen(VUNULL), mpImage(VUNULL), mDestroyed(false), mpCallback(VUNULL), mClosed(false) {}
	virtual ~VuMessageBox() {}

	// callback
	class Callback { public: virtual void onMessageBoxClosed(VuMessageBox *pMessageBox) = 0; };
	void				setCallback(Callback *pCB)	{ mpCallback = pCB; }

	// actions
	void				close() { mClosed = true; }

	// result
	void				setResult(const char *result) { mResult = result; }
	const std::string	&getResult() const { return mResult; }

	// params
	const VuMessageBoxParams	&params() const { return mParams; }

protected:
	friend class VuMessageBoxManager;

	void				tick(float fdt);
	void				draw();

	VuMessageBoxParams	mParams;
	VuProject			*mpProject;
	VuUIScreenEntity	*mpScreen;
	VuTextureAsset		*mpImage;
	bool				mDestroyed;
	Callback			*mpCallback;
	bool				mClosed;
	std::string			mResult;
};


class VuMessageBoxManager : public VuSystemComponent
{
	DECLARE_SYSTEM_COMPONENT(VuMessageBoxManager)
	DECLARE_EVENT_MAP

protected:
	// called by game
	friend class VuEngine;
	virtual bool init();
	virtual void preRelease();
	virtual void release();

public:
	VuMessageBoxManager();

	VuMessageBox		*create(const VuMessageBoxParams &params);
	void				destroy(VuMessageBox *pMessageBox);

	VuMessageBox		*getActiveMessageBox() { return mpActiveMessageBox; }
	float				getFadeValue() { return mFadeValue; }

	void				setBackgroundColor(const VuColor &color) { mBackgroundColor = color; }
	void				setMinFadeTime(float minFadeTime) { mMinFadeTime = minFadeTime; }

protected:
	// event handlers
	void				OnExitApp(const VuParams &params) { preRelease(); }

	void				tick(float fdt);
	void				draw();

	void				releaseActiveMessageBox();

	// FSM
	void				onFadeInEnter();
	void				onFadeInTick(float fdt);
	void				onActiveEnter();
	void				onActiveTick(float fdt);
	void				onActiveExit();
	void				onFadeOutEnter();
	void				onFadeOutTick(float fdt);
	void				onFadeOutExit();

	typedef std::queue<VuMessageBox *> MessageBoxQueue;
	MessageBoxQueue		mMessageBoxQueue;

	VuDBAsset			*mpMessageBoxDBAsset;
	VuFSM				mFSM;
	VuColor				mBackgroundColor;
	float				mMinFadeTime;
	float				mFadeValue;
	VuMessageBox		*mpActiveMessageBox;
};
