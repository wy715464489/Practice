//*****************************************************************************
//
//  Copyright (c) 2013-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  ToastManager class
// 
//*****************************************************************************

#include "VuToastManager.h"
#include "VuEngine/Entities/UI/VuUIScreenEntity.h"
#include "VuEngine/Projects/VuProject.h"
#include "VuEngine/Assets/VuAssetFactory.h"
#include "VuEngine/Assets/VuProjectAsset.h"
#include "VuEngine/Assets/VuTextureAsset.h"
#include "VuEngine/Managers/VuTickManager.h"
#include "VuEngine/Managers/VuDrawManager.h"
#include "VuEngine/Gfx/GfxSort/VuGfxSort.h"
#include "VuEngine/UI/VuUIUtil.h"


// the interface
IMPLEMENT_SYSTEM_COMPONENT(VuToastManager, VuToastManager);

IMPLEMENT_RTTI_BASE(VuToast);


//*****************************************************************************
VuToast::~VuToast()
{
	VuAssetFactory::IF()->releaseAsset(mpImage);
}

//*****************************************************************************
bool VuToast::tick(float fdt)
{
	if ( mState == STATE_BEGIN )
	{
		VuUIUtil::startTransitionIn(mpScreen);
		mState = STATE_FADE_IN;
	}
	else if ( mState == STATE_FADE_IN )
	{
		if ( VuUIUtil::tickTransition(mpScreen, fdt) )
			mState = STATE_ACTIVE;
	}
	else if ( mState == STATE_ACTIVE )
	{
		mpScreen->tick(fdt, 0);
		mAge += fdt;
		if ( mAge >= mActiveTime )
		{
			VuUIUtil::startTransitionOut(mpScreen);
			mState = STATE_FADE_OUT;
		}
	}
	else if ( mState == STATE_FADE_OUT )
	{
		if ( VuUIUtil::tickTransition(mpScreen, fdt) )
			return true;
	}

	return false;
}

//*****************************************************************************
void VuToast::draw()
{
	mpScreen->draw();
}

//*****************************************************************************
VuToastManager::VuToastManager():
	mpActiveToast(VUNULL)
{
}

//*****************************************************************************
bool VuToastManager::init()
{
	VuTickManager::IF()->registerHandler(this, &VuToastManager::tick, "Final");
	VuDrawManager::IF()->registerHandler(this, &VuToastManager::draw);

	return true;
}

//*****************************************************************************
void VuToastManager::preRelease()
{
	delete mpActiveToast;
	mpActiveToast = VUNULL;
	while ( mToastQueue.size() )
	{
		delete mToastQueue.front();
		mToastQueue.pop();
	}

	for ( ToastTypes::iterator iter = mToastTypes.begin(); iter != mToastTypes.end(); iter++ )
	{
		if ( iter->second.mpProject )
		{
			iter->second.mpProject->gameRelease();
			iter->second.mpProject->removeRef();
		}
		VuAssetFactory::IF()->releaseAsset(iter->second.mpProjectAsset);
	}
	mToastTypes.clear();
}

//*****************************************************************************
void VuToastManager::release()
{
	VuTickManager::IF()->unregisterHandlers(this);
	VuDrawManager::IF()->unregisterHandler(this);
}

//*****************************************************************************
void VuToastManager::registerToastType(const VuRTTI &rtti, const char *projectAsset)
{
	// already registered?
	if ( mToastTypes.find(rtti.mstrType) != mToastTypes.end() )
		return;

	ToastType &toastType = mToastTypes[rtti.mstrType];

	if ( VuAssetFactory::IF()->doesAssetExist<VuProjectAsset>(projectAsset) )
	{
		toastType.mpProjectAsset = VuAssetFactory::IF()->createAsset<VuProjectAsset>(projectAsset);
		toastType.mpProject = new VuProject;
		if ( toastType.mpProject->load(toastType.mpProjectAsset) )
		{
			if ( toastType.mpProject->getRootEntity()->isDerivedFrom(VuUIScreenEntity::msRTTI) )
			{
				toastType.mpScreen = static_cast<VuUIScreenEntity *>(toastType.mpProject->getRootEntity());
				toastType.mpScreen->setFullScreenLayer(VuGfxSort::FSL_TOAST);
			}
		}
	}
}

//*****************************************************************************
bool VuToastManager::showToast(VuToast *pToast)
{
	ToastTypes::iterator iter = mToastTypes.find(pToast->getType());
	if ( iter != mToastTypes.end() && iter->second.mpScreen )
	{
		pToast->mpProject = iter->second.mpProject;
		pToast->mpScreen = iter->second.mpScreen;
		mToastQueue.push(pToast);
		return true;
	}

	delete pToast;
	return false;
}

//*****************************************************************************
void VuToastManager::tick(float fdt)
{
	fdt = VuTickManager::IF()->getRealDeltaTime();

	if ( mpActiveToast )
	{
		if ( mpActiveToast->tick(fdt) )
		{
			mpActiveToast->mpProject->gameRelease();
			delete mpActiveToast;
			mpActiveToast = VUNULL;
		}
	}
	else if ( mToastQueue.size() )
	{
		mpActiveToast = mToastQueue.front();
		mToastQueue.pop();

		mpActiveToast->mpProject->gameInitialize();
	}
}

//*****************************************************************************
void VuToastManager::draw()
{
	if ( mpActiveToast )
		mpActiveToast->draw();
}
