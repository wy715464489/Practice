//*****************************************************************************
//
//  Copyright (c) 2011-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  Xb1-specific Profile Manager
//
//*****************************************************************************
#pragma once

#include <ppltasks.h>
#include <collection.h>
#include <robuffer.h>

#include "VuEngine/Util/VuHash.h"
#include "VuEngine/Managers/VuProfileManager.h"
#include "VuEngine/Json/VuJsonContainer.h"
#include "VuEngine/Json/VuJsonBinaryReader.h"
#include "VuEngine/Json/VuJsonBinaryWriter.h"

using namespace Concurrency;
using namespace Windows::Xbox::Storage;
using namespace Windows::Xbox::System;
using namespace Windows::Foundation;
using namespace Windows::Storage;
using namespace Windows::Foundation::Collections;

class VuXb1ProfileManager : public VuProfileManager
{
public:
	VuXb1ProfileManager() : mUser(nullptr) {}

	virtual bool			init(const std::string &gameName);

	// Call setUserAndLoad() whenever the logged in user changes
	//
	void					setUserAndLoad(User^ userId);
	
	// To access the userId
	User^					getUser() { critical_section::scoped_lock lock(mLock); return mUser; }
	void					setUser(User^ user) { critical_section::scoped_lock lock(mLock); mUser = user; }

	static VuXb1ProfileManager *IF() { return static_cast<VuXb1ProfileManager *>(VuProfileManager::IF()); }

	static bool				isSameUserId(const wchar_t* user1, const wchar_t* user2);
	static bool				isSameUserId(const wchar_t* user1, IUser^ user2);
	static bool				isSameUserId(IUser^ user1, const wchar_t* user2);
	static bool				isSameUserId(IUser^ user1, IUser^ user2);

protected:
	virtual void			loadInternal();
	virtual void			saveInternal();

private:
	virtual void			getPath(std::string &path) {}
	byte*					getBufferPointer(Streams::IBuffer^ buffer);

	User^					mUser;
	ConnectedStorageSpace^	mConnectedStorageSpace;
    critical_section		mLock;
};

