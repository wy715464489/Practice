//*****************************************************************************
//
//  Copyright (c) 2013-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  ToastManager class
// 
//*****************************************************************************

#pragma once

#include "VuEngine/VuSystemComponent.h"
#include "VuEngine/Objects/VuRTTI.h"

class VuProjectAsset;
class VuProject;
class VuUIScreenEntity;
class VuTextureAsset;


class VuToast
{
	DECLARE_RTTI

public:
	VuToast() : mpImage(VUNULL), mActiveTime(3.0f), mpProject(VUNULL), mpScreen(VUNULL), mAge(0.0f), mState(STATE_BEGIN) {}
	virtual ~VuToast();

	virtual bool		tick(float fdt); // return true when done
	virtual void		draw();

	std::string			mText;
	VuTextureAsset		*mpImage;
	float				mActiveTime;
	VuProject			*mpProject;
	VuUIScreenEntity	*mpScreen;
	float				mAge;

	enum eState { STATE_BEGIN, STATE_FADE_IN, STATE_ACTIVE, STATE_FADE_OUT };
	eState				mState;
};


class VuToastManager : public VuSystemComponent
{
	DECLARE_SYSTEM_COMPONENT(VuToastManager)

protected:
	// called by game
	friend class VuEngine;
	virtual bool init();
	virtual void preRelease();
	virtual void release();

public:
	VuToastManager();

	void				registerToastType(const VuRTTI &rtti, const char *projectAsset);

	bool				showToast(VuToast *pToast);
	VuToast				*getActiveToast() { return mpActiveToast; }

protected:
	void				tick(float fdt);
	void				draw();

	struct ToastType
	{
		ToastType() : mpProjectAsset(VUNULL), mpProject(VUNULL), mpScreen(VUNULL) {}
		VuProjectAsset		*mpProjectAsset;
		VuProject			*mpProject;
		VuUIScreenEntity	*mpScreen;
	};
	typedef std::map<std::string, ToastType> ToastTypes;
	ToastTypes			mToastTypes;

	typedef std::queue<VuToast *> ToastQueue;
	ToastQueue			mToastQueue;

	VuToast				*mpActiveToast;
};
