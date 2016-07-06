//*****************************************************************************
//
//  Copyright (c) 2012-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  Cloud Manager
// 
//*****************************************************************************

#pragma once

#include "VuEngine/VuSystemComponent.h"
#include "VuEngine/Events/VuEventMap.h"
#include "VuEngine/Json/VuJsonContainer.h"
#include "VuEngine/Containers/VuArray.h"
#include "VuEngine/Util/VuFSM.h"


class VuCloudManager : public VuSystemComponent
{
	DECLARE_SYSTEM_COMPONENT(VuCloudManager)
	DECLARE_EVENT_MAP

protected:
	// called by game
	friend class VuEngine;
	virtual bool init();
	virtual void postInit();
	virtual void release();

public:
	VuCloudManager();

	bool			isNewsAvailable();
	bool			isNewsInterstitial();
	bool			isNewsUnread() { return mNewsUnread; }
	void			getNewsData(VuArray<VUBYTE> &data);

	void			onNewsLaunched();

private:
	// event handlers
	void			OnSaveProfile(const VuParams &params) { saveToProfile(); }

	void			loadFromProfile();
	void			saveToProfile();

	void			tick(float fdt);

	// FSM
	void			onIdentityEnter();
	void			onIdentityExit();
	void			onIdentityTick(float fdt);
	void			onNewsEnter();
	void			onNewsExit();
	void			onNewsTick(float fdt);
	void			onNewsDataEnter();
	void			onNewsDataExit();
	void			onNewsDataTick(float fdt);
	void			onNewsTrackEnter();
	void			onNewsTrackExit();
	void			onNewsTrackTick(float fdt);

	bool			validateNewsData(const VuArray<VUBYTE> &newsData);

	VuFSM			mFSM;

	VUHANDLE		mHttpRequest;

	VuJsonContainer	mNewsResponse;
	bool			mIdentitySaved;
	bool			mNewsUnread;
};
