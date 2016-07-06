//*****************************************************************************
//
//  Copyright (c) 2011-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  ProfileManager class
// 
//*****************************************************************************

#pragma once

#include "VuEngine/VuSystemComponent.h"
#include "VuEngine/Json/VuJsonContainer.h"

#define PROFILE_VERSION 1
#define PROFILE_FILE_NAME "profile"
#define PROFILE_BACKUP_FILE_NAME "profileback"

struct ProfileHeader
{
	VUUINT32	mMagic;
	VUUINT32	mVersion;
	VUUINT32	mDataSize;
	VUUINT32	mDataHash;
};

static const VUUINT32 scProfileMagic = ('V'<<24)|('U'<<16)|('P'<<8)|('R');

class VuProfileManager : public VuSystemComponent
{
	DECLARE_SYSTEM_COMPONENT(VuProfileManager)

protected:
	// created by engine
	friend class VuEngine;
	virtual bool init(const std::string &gameName);

public:
	VuProfileManager();

	const VuJsonContainer	&dataRead()		{ return mData; }
	VuJsonContainer			&dataWrite()	{ return mData; }

	const VuJsonContainer	&tempDataRead()		{ return mTempData; }
	VuJsonContainer			&tempDataWrite()	{ return mTempData; }

	void					save();

	virtual void			getPath(std::string &path) = 0;

protected:
	virtual void			loadInternal();
	virtual void			saveInternal();

	enum eResult { RESULT_OK, RESULT_NOT_FOUND, RESULT_CORRUPTED };
	static eResult			loadInternal(const std::string &fileName, VuJsonContainer &data);
	static bool				saveInternal(const std::string &fileName, const VuJsonContainer &data);

	VuJsonContainer			mData;
	VuJsonContainer			mTempData;
};
