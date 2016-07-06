//*****************************************************************************
//
//  Copyright (c) 2008-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  High-level Pfx Manager class
// 
//*****************************************************************************

#pragma once

#include "VuEngine/VuSystemComponent.h"
#include "VuEngine/Containers/VuList.h"

class VuEngine;
class VuPfxEntity;


class VuPfxManagerConfig
{
public:
	VuPfxManagerConfig();

	VUUINT32	mMaxCount;
};

class VuPfxManager : public VuSystemComponent
{
	DECLARE_SYSTEM_COMPONENT(VuPfxManager)

protected:
	// called by engine
	friend class VuEngine;
	bool	init();
	void	release();

public:
	VuPfxManager();

	// configuration
	void			configure(const VuPfxManagerConfig &config);

	// entity management
	VUUINT32		createEntity(const char *strPath, bool bOneShot);
	void			releaseEntity(VUUINT32 handle, bool bHardKill = false);
	VuPfxEntity		*getEntity(VUUINT32 handle);
	void			killAllEntities();

	int				getActiveCount()	{ return mActiveList.size(); }
	int				getFreeCount()		{ return mFreeList.size(); }

private:
	void			tickFinal(float fdt);
	void			tick(VuPfxEntity *pEntity, float fdt);
	VuPfxEntity		*create(const char *strPath);
	void			destroy(VuPfxEntity *pEntity);


	typedef VuList<VuPfxEntity> PfxEntityList;

	VuPfxManagerConfig	mConfig;
	VuPfxEntity			**mppEntityHandleTable;
	VUUINT32			mHandleMask;
	VUUINT32			mHandleShift;
	VUUINT32			mHandleMaxCount;
	PfxEntityList		mFreeList;
	PfxEntityList		mActiveList;
};
