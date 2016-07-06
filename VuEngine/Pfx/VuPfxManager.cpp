//*****************************************************************************
//
//  Copyright (c) 2008-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  High-level Pfx Manager class
// 
//*****************************************************************************

#include "VuPfxManager.h"
#include "VuPfxEntity.h"
#include "VuPfx.h"
#include "VuEngine/Entities/VuEntityFactory.h"
#include "VuEngine/Components/3dDraw/Vu3dDrawComponent.h"
#include "VuEngine/Managers/VuTickManager.h"


// the interface
IMPLEMENT_SYSTEM_COMPONENT(VuPfxManager, VuPfxManager);


//*****************************************************************************
VuPfxManagerConfig::VuPfxManagerConfig():
	mMaxCount(64)
{
}

//*****************************************************************************
VuPfxManager::VuPfxManager():
	mppEntityHandleTable(VUNULL),
	mHandleMask(0),
	mHandleShift(0),
	mHandleMaxCount(0)
{
}

//*****************************************************************************
bool VuPfxManager::init()
{
	// configure w/ default settings
	configure(VuPfxManagerConfig());

	// register phased tick
	VuTickManager::IF()->registerHandler(this, &VuPfxManager::tickFinal, "Final");

	return true;
}

//*****************************************************************************
void VuPfxManager::release()
{
	// unregister phased tick
	VuTickManager::IF()->unregisterHandlers(this);

	killAllEntities();

	while ( mFreeList.size() )
		mFreeList.pop_back()->removeRef();

	delete mppEntityHandleTable;
}

//*****************************************************************************
void VuPfxManager::configure(const VuPfxManagerConfig &config)
{
	mConfig = config;

	VUASSERT(mActiveList.size() == 0, "VuPfxManager::configure() active pfx entities");

	// increase
	while ( mFreeList.size() < (int)mConfig.mMaxCount )
		mFreeList.push_back(new VuPfxEntity);

	// decrease
	while ( mFreeList.size() > (int)mConfig.mMaxCount )
		mFreeList.pop_back()->removeRef();

	// rebuild handle table
	{
		mHandleMask = VuNextHighestPower2(mConfig.mMaxCount) - 1;
		mHandleShift = VuBitCount(mHandleMask);
		mHandleMaxCount = 0xffffffff >> mHandleShift;

		delete mppEntityHandleTable;
		mppEntityHandleTable = new VuPfxEntity *[mConfig.mMaxCount];
		VuPfxEntity *pEntity = mFreeList.front();
		for ( int i = 0; i < mFreeList.size(); i++ )
		{
			mppEntityHandleTable[i] = pEntity;
			pEntity->mHandleSlot = i;
			pEntity->mHandleCount = 1;
			pEntity = pEntity->next();
		}
	}
}

//*****************************************************************************
VUUINT32 VuPfxManager::createEntity(const char *strPath, bool bOneShot)
{
	if ( VuPfxEntity *pEntity = create(strPath) )
	{
		if ( ++pEntity->mHandleCount > mHandleMaxCount )
			pEntity->mHandleCount = 1;

		VUUINT32 handle = (pEntity->mHandleCount << mHandleShift) + pEntity->mHandleSlot;

		// one-shot warning
		if ( bOneShot && pEntity->mpPfxSystemInstance->duration() <= 0 )
			VUWARNING("One-shot PFX created with infinite duration: %s", strPath);

		return handle;
	}

	return 0;
}

//*****************************************************************************
void VuPfxManager::releaseEntity(VUUINT32 handle, bool bHardKill)
{
	if ( VuPfxEntity *pEntity = getEntity(handle) )
	{
		pEntity->getSystemInstance()->stop(bHardKill);
		if ( bHardKill )
			destroy(pEntity);
	}
}

//*****************************************************************************
VuPfxEntity *VuPfxManager::getEntity(VUUINT32 handle)
{
	VUUINT32 slot = handle & mHandleMask;
	VUUINT32 count = handle >> mHandleShift;

	if ( slot < mConfig.mMaxCount )
	{
		VuPfxEntity *pEntity = mppEntityHandleTable[slot];
		if ( pEntity->isGameInitialized() && pEntity->mHandleCount == count )
			return pEntity;
	}

	return VUNULL;
}

//*****************************************************************************
void VuPfxManager::killAllEntities()
{
	while ( mActiveList.size() )
		destroy(mActiveList.back());
}

//*****************************************************************************
void VuPfxManager::tickFinal(float fdt)
{
	VuPfxEntity *pEntity = mActiveList.front();
	while ( pEntity )
	{
		VuPfxEntity *pNextEntity = pEntity->next();

		tick(pEntity, fdt);

		// remove?
		if ( pEntity->getSystemInstance()->getState() == VuPfxSystemInstance::STATE_STOPPED )
			destroy(pEntity);

		pEntity = pNextEntity;
	}
}

//*****************************************************************************
void VuPfxManager::tick(VuPfxEntity *pEntity, float fdt)
{
	VuPfxSystemInstance *pSysInstance = pEntity->getSystemInstance();
	Vu3dDrawComponent *pDrawComp = pEntity->mp3dDrawComponent;

	pSysInstance->tick(fdt, false);

	if ( pSysInstance->particleCount() )
	{
		pDrawComp->show();
		pDrawComp->updateVisibility(pSysInstance->getAabb());
	}
	else
	{
		pDrawComp->hide();
	}
}

//*****************************************************************************
VuPfxEntity *VuPfxManager::create(const char *strPath)
{
	if ( mFreeList.size() )
	{
		if ( VuPfxSystemInstance *pSystemInstance = VuPfx::IF()->createSystemInstance(strPath) )
		{
			VuPfxEntity *pEntity = mFreeList.pop_back();
			pEntity->mpPfxSystemInstance = pSystemInstance;
			pEntity->gameInitialize();
			mActiveList.push_back(pEntity);

			return pEntity;
		}
	}

	return VUNULL;
}

//*****************************************************************************
void VuPfxManager::destroy(VuPfxEntity *pEntity)
{
	mActiveList.remove(pEntity);
	pEntity->gameRelease();
	VuPfx::IF()->releaseSystemInstance(pEntity->getSystemInstance());
	mFreeList.push_back(pEntity);
}
