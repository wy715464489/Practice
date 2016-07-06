//*****************************************************************************
//
//  Copyright (c) 2009-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  TriggerManager class
// 
//*****************************************************************************

#include "VuTriggerManager.h"
#include "VuTickManager.h"
#include "VuEngine/Entities/VuEntity.h"
#include "VuEngine/Entities/Triggers/VuTriggerEntity.h"
#include "VuEngine/Components/Instigator/VuInstigatorComponent.h"
#include "VuEngine/Components/Transform/VuTransformComponent.h"
#include "VuEngine/Util/VuColor.h"
#include "VuEngine/Dev/VuDev.h"
#include "VuEngine/Dev/VuDevMenu.h"


// the interface
IMPLEMENT_SYSTEM_COMPONENT(VuTriggerManager, VuTriggerManager);

// static variables
VuTriggerManager::Types VuTriggerManager::smTypes;
static bool sbDebugDrawInstigators = false;
static bool sbDebugDrawTriggerEntities = false;


//*****************************************************************************
VuTriggerManager::VuTriggerManager():
	mBusy(false)
{
}

//*****************************************************************************
VuTriggerManager::~VuTriggerManager()
{
}

//*****************************************************************************
bool VuTriggerManager::init()
{
	VuTickManager::IF()->registerHandler(this, &VuTriggerManager::tick, "Triggers");

	VuDevMenu::IF()->addBool("TriggerManager/Draw Instigators", sbDebugDrawInstigators);
	VuDevMenu::IF()->addBool("TriggerManager/Draw Trigger Entities", sbDebugDrawTriggerEntities);

	return true;
}

//*****************************************************************************
void VuTriggerManager::release()
{
	VuTickManager::IF()->unregisterHandlers(this);

	VUASSERT(mInstigators.size() == 0, "VuTriggerManager::release() dangling instigators");
	VUASSERT(mTriggerEntities.size() == 0, "VuTriggerManager::release() dangling trigger entities");
}

//*****************************************************************************
VUUINT32 VuTriggerManager::getTypeMask(const char *strType)
{
	for ( int i = 0; i < (int)smTypes.size(); i++ )
		if ( smTypes[i] == strType )
			return (1<<i);

	return 0;
}

//*****************************************************************************
void VuTriggerManager::addInstigator(VuInstigatorComponent *pInstigatorComponent)
{
	VuTransformComponent *pTransformComponent = pInstigatorComponent->getOwnerEntity()->getTransformComponent();
	VUASSERT(pTransformComponent, "VuTriggerManager::addInstigator() transform component required");

	VuInstigatorEntry entry;
	entry.mpInstigatorComponent = pInstigatorComponent;
	entry.mpTransformComponent = pTransformComponent;
	entry.mMask = pInstigatorComponent->getMask();
	entry.mPrevPos = entry.mCurPos = pTransformComponent->getWorldTransform().transform(pInstigatorComponent->getOffset());
	entry.mPrevRadius = entry.mCurRadius = pInstigatorComponent->getRadius();

	mInstigators.push_back(entry);
}

//*****************************************************************************
void VuTriggerManager::removeInstigator(VuInstigatorComponent *pInstigatorComponent)
{
	for ( VuInstigatorEntry *pi = &mInstigators.begin(); pi != &mInstigators.end(); pi++ )
	{
		if ( pi->mpInstigatorComponent == pInstigatorComponent )
		{
			*pi = mInstigators.back();
			mInstigators.resize(mInstigators.size() - 1);
			break;
		}
	}
}

//*****************************************************************************
void VuTriggerManager::snapInstigator(VuInstigatorComponent *pInstigatorComponent)
{
	for ( VuInstigatorEntry *pi = &mInstigators.begin(); pi != &mInstigators.end(); pi++ )
	{
		if ( pi->mpInstigatorComponent == pInstigatorComponent )
		{
			pi->mCurPos = pi->mpTransformComponent->getWorldTransform().transform(pi->mpInstigatorComponent->getOffset());
			pi->mPrevPos = pi->mCurPos;
		}
	}
}

//*****************************************************************************
void VuTriggerManager::addTriggerEntity(VuTriggerEntity *pTriggerEntity)
{
	if ( mBusy )
		mAddedTriggers.push_back(pTriggerEntity);
	else
		mTriggerEntities.push_back(pTriggerEntity);
}

//*****************************************************************************
void VuTriggerManager::removeTriggerEntity(VuTriggerEntity *pTriggerEntity)
{
	if ( mBusy )
		mRemovedTriggers.push_back(pTriggerEntity);
	else
		mTriggerEntities.remove(pTriggerEntity);
}

//*****************************************************************************
void VuTriggerManager::tick(float fdt)
{
	// update instigators
	for ( VuInstigatorEntry *pi = &mInstigators.begin(); pi != &mInstigators.end(); pi++ )
	{
		pi->mPrevPos = pi->mCurPos;
		pi->mPrevRadius = pi->mCurRadius;
		pi->mCurPos = pi->mpTransformComponent->getWorldTransform().transform(pi->mpInstigatorComponent->getOffset());
		pi->mCurRadius = pi->mpInstigatorComponent->getRadius();
	}

	// update triggers
	mBusy = true;
	{
		for ( VuTriggerEntity **ppti = &mTriggerEntities.begin(); ppti != &mTriggerEntities.end(); ppti++ )
			(*ppti)->update();
	}
	mBusy = false;

	for ( int i = 0; i < mAddedTriggers.size(); i++ )
		mTriggerEntities.push_back(mAddedTriggers[i]);
	mAddedTriggers.clear();

	for ( int i = 0; i < mRemovedTriggers.size(); i++ )
		mTriggerEntities.remove(mRemovedTriggers[i]);
	mRemovedTriggers.clear();

	// debug draw instigators
	if ( sbDebugDrawInstigators )
		for ( VuInstigatorEntry *pi = &mInstigators.begin(); pi != &mInstigators.end(); pi++ )
			VuDev::IF()->drawSphere(pi->mCurPos, pi->mCurRadius, VuColor(0,255,255), 4, 4);

	// debug draw triggers
	if ( sbDebugDrawTriggerEntities )
		for ( VuTriggerEntity **ppti = &mTriggerEntities.begin(); ppti != &mTriggerEntities.end(); ppti++ )
			(*ppti)->draw();
}
