//*****************************************************************************
//
//  Copyright (c) 2013-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  DialogManager class
// 
//*****************************************************************************

#include "VuDialogManager.h"
#include "VuEngine/Entities/UI/VuUIScreenEntity.h"
#include "VuEngine/Projects/VuProject.h"
#include "VuEngine/Assets/VuAssetFactory.h"
#include "VuEngine/Assets/VuProjectAsset.h"
#include "VuEngine/Managers/VuTickManager.h"
#include "VuEngine/Managers/VuDrawManager.h"
#include "VuEngine/Gfx/VuGfxUtil.h"
#include "VuEngine/HAL/Audio/VuAudio.h"
#include "VuEngine/UI/VuUI.h"
#include "VuEngine/UI/VuUIUtil.h"


// the interface
IMPLEMENT_SYSTEM_COMPONENT(VuDialogManager, VuDialogManager);

IMPLEMENT_RTTI_BASE(VuDialog);


//*****************************************************************************
void VuDialog::tick(float fdt)
{
	mpScreen->tick(fdt);
}

//*****************************************************************************
void VuDialog::draw()
{
	mpScreen->draw();
}

//*****************************************************************************
VuDialogManager::VuDialogManager():
	mBackgroundColor(0,0,0,90),
	mMinFadeTime(0.3f),
	mFadeValue(0.0f),
	mpActiveDialog(VUNULL),
	mPauseRequestCount(0)
{
	// create FSM
	VuFSM::VuState *pState = mFSM.addState("Inactive");

	pState = mFSM.addState("FadeIn");
	pState->setEnterMethod(this, &VuDialogManager::onFadeInEnter);
	pState->setTickMethod(this, &VuDialogManager::onFadeInTick);

	pState = mFSM.addState("Active");
	pState->setTickMethod(this, &VuDialogManager::onActiveTick);

	pState = mFSM.addState("FadeOut");
	pState->setEnterMethod(this, &VuDialogManager::onFadeOutEnter);
	pState->setTickMethod(this, &VuDialogManager::onFadeOutTick);
	pState->setExitMethod(this, &VuDialogManager::onFadeOutExit);

	// add transitions
	mFSM.addTransition("Inactive", "FadeIn", "DialogQueued");

	mFSM.addTransition("FadeIn", "Active", "FadeInComplete");

	mFSM.addTransition("Active", "FadeOut", "DialogDestroyed");
	mFSM.addTransition("Active", "FadeOut", "DialogClosed");

	mFSM.addTransition("FadeOut", "FadeIn", "FadeOutComplete & DialogQueued");
	mFSM.addTransition("FadeOut", "Inactive", "FadeOutComplete");

	// event handlers
	REG_EVENT_HANDLER(VuDialogManager, OnExitApp);
}

//*****************************************************************************
bool VuDialogManager::init()
{
	VuTickManager::IF()->registerHandler(this, &VuDialogManager::tick, "Final");
	VuDrawManager::IF()->registerHandler(this, &VuDialogManager::draw);

	mFSM.begin();

	return true;
}

//*****************************************************************************
void VuDialogManager::preRelease()
{
	mFSM.end();

	releaseActiveDialog();

	while ( mDialogQueue.size() )
	{
		mDialogQueue.front()->removeRef();
		mDialogQueue.pop();
	}
}

//*****************************************************************************
void VuDialogManager::release()
{
	VuTickManager::IF()->unregisterHandlers(this);
	VuDrawManager::IF()->unregisterHandler(this);
}

//*****************************************************************************
VuDialog *VuDialogManager::create(const char *projectAsset)
{
	VuDialog *pDialog = VUNULL;

	if ( VuAssetFactory::IF()->doesAssetExist<VuProjectAsset>(projectAsset) )
	{
		pDialog = new VuDialog;
		pDialog->addRef();

		pDialog->mProjectAsset = projectAsset;

		mDialogQueue.push(pDialog);
	}

	return pDialog;
}

//*****************************************************************************
void VuDialogManager::destroy(VuDialog *pDialog)
{
	pDialog->mDestroyed = true;
	pDialog->removeRef();
}

//*****************************************************************************
void VuDialogManager::tick(float fdt)
{
	// use real dt
	fdt = VuTickManager::IF()->getRealDeltaTime();

	// remove destroyed message boxes from front of queue
	while ( mDialogQueue.size() && mDialogQueue.front()->mDestroyed )
	{
		mDialogQueue.front()->removeRef();
		mDialogQueue.pop();
	}

	// look for new message box in queue
	if ( mDialogQueue.size() )
		mFSM.pulseCondition("DialogQueued");

	mFSM.evaluate();
	mFSM.tick(fdt);
}

//*****************************************************************************
void VuDialogManager::draw()
{
	// draw message box entity
	if ( mpActiveDialog )
	{
		mpActiveDialog->draw();

		VuColor color = mBackgroundColor;
		color.mA = VuRound(color.mA*mFadeValue);
		VuGfxUtil::IF()->drawFilledRectangle2d(1.0f, color);
	}
}

//*****************************************************************************
void VuDialogManager::releaseActiveDialog()
{
	if ( mpActiveDialog )
	{
		if ( mpActiveDialog->mPauseGame )
		{
			VuTickManager::IF()->popPauseRequest();
			VuAudio::IF()->popCategoryPause("game");
		}

		mpActiveDialog->mpProject->gameRelease();
		mpActiveDialog->mpProject->removeRef();
		mpActiveDialog->removeRef();
		mpActiveDialog = VUNULL;
	}
}

//*****************************************************************************
void VuDialogManager::onFadeInEnter()
{
	mpActiveDialog = mDialogQueue.front();
	mDialogQueue.pop();

	if ( mpActiveDialog->mPauseGame )
	{
		VuTickManager::IF()->pushPauseRequest();
		VuAudio::IF()->pushCategoryPause("game");
	}

	VuProjectAsset *pProjectAsset = VuAssetFactory::IF()->createAsset<VuProjectAsset>(mpActiveDialog->mProjectAsset);

	VuProject *pProject = new VuProject;
	if ( pProject->load(pProjectAsset) )
	{
		if ( pProject->getRootEntity()->isDerivedFrom(VuUIScreenEntity::msRTTI) )
		{
			mpActiveDialog->mpProject = pProject;
			mpActiveDialog->mpScreen = static_cast<VuUIScreenEntity *>(pProject->getRootEntity());
			mpActiveDialog->mpScreen->setFullScreenLayer(VuGfxSort::FSL_DIALOG);
			mpActiveDialog->mpScreen->setPriority(16384);
		}
	}

	VuAssetFactory::IF()->releaseAsset(pProjectAsset);

	VUASSERT(mpActiveDialog->mpScreen, "Unable to create dialog.");

	VuUI::IF()->pushFocus();
	mpActiveDialog->mpProject->gameInitialize();

	VuUIUtil::startTransitionIn(mpActiveDialog->mpScreen);
}

//*****************************************************************************
void VuDialogManager::onFadeInTick(float fdt)
{
	float fadeValue = VuMin(mFSM.getTimeInState()/mMinFadeTime, 1.0f);
	mFadeValue = VuMax(mFadeValue, fadeValue);

	if ( VuUIUtil::tickTransition(mpActiveDialog->mpScreen, fdt) && mFadeValue >= 1.0f )
		mFSM.pulseCondition("FadeInComplete");
}

//*****************************************************************************
void VuDialogManager::onActiveTick(float fdt)
{
	// tick dialog
	if ( mPauseRequestCount == 0 )
		mpActiveDialog->tick(fdt);

	if ( mpActiveDialog->mDestroyed )
		mFSM.pulseCondition("DialogDestroyed");

	if ( mpActiveDialog->mClosed )
		mFSM.pulseCondition("DialogClosed");
}

//*****************************************************************************
void VuDialogManager::onFadeOutEnter()
{
	VuUI::IF()->popFocus();

	if ( !mpActiveDialog->mDestroyed )
	{
		if ( mpActiveDialog->mpCallback )
		{
			mpActiveDialog->mpCallback->onDialogClosed(mpActiveDialog);
		}
	}

	VuUIUtil::startTransitionOut(mpActiveDialog->mpScreen);
}

//*****************************************************************************
void VuDialogManager::onFadeOutTick(float fdt)
{
	if ( mDialogQueue.empty() )
	{
		mFadeValue = VuMax(1.0f - mFSM.getTimeInState()/mMinFadeTime, 0.0f);
		if ( VuUIUtil::tickTransition(mpActiveDialog->mpScreen, fdt) && mFadeValue <= 0.0f )
			mFSM.pulseCondition("FadeOutComplete");
	}
	else
	{
		if ( VuUIUtil::tickTransition(mpActiveDialog->mpScreen, fdt) )
			mFSM.pulseCondition("FadeOutComplete");
	}
}

//*****************************************************************************
void VuDialogManager::onFadeOutExit()
{
	releaseActiveDialog();
}
