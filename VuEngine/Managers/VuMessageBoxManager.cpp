//*****************************************************************************
//
//  Copyright (c) 2014-2014 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  MessageBoxManager class
// 
//*****************************************************************************

#include "VuMessageBoxManager.h"
#include "VuEngine/Entities/UI/VuUIScreenEntity.h"
#include "VuEngine/Projects/VuProject.h"
#include "VuEngine/Assets/VuAssetFactory.h"
#include "VuEngine/Assets/VuProjectAsset.h"
#include "VuEngine/Assets/VuTextureAsset.h"
#include "VuEngine/Assets/VuDBAsset.h"
#include "VuEngine/Managers/VuTickManager.h"
#include "VuEngine/Managers/VuDrawManager.h"
#include "VuEngine/Gfx/VuGfxUtil.h"
#include "VuEngine/HAL/Audio/VuAudio.h"
#include "VuEngine/UI/VuUI.h"
#include "VuEngine/UI/VuUIUtil.h"


// the interface
IMPLEMENT_SYSTEM_COMPONENT(VuMessageBoxManager, VuMessageBoxManager);

IMPLEMENT_RTTI_BASE(VuMessageBox);


//*****************************************************************************
VuMessageBoxParams::VuMessageBoxParams():
	mType("SimpleA"),
	mPauseGame(false),
	mUserData(0)
{
}

//*****************************************************************************
void VuMessageBox::tick(float fdt)
{
	mpScreen->tick(fdt);
}

//*****************************************************************************
void VuMessageBox::draw()
{
	mpScreen->draw();
}

//*****************************************************************************
VuMessageBoxManager::VuMessageBoxManager():
	mBackgroundColor(0,0,0,90),
	mMinFadeTime(0.3f),
	mFadeValue(0.0f),
	mpActiveMessageBox(VUNULL)
{
	// create FSM
	VuFSM::VuState *pState = mFSM.addState("Inactive");

	pState = mFSM.addState("FadeIn");
	pState->setEnterMethod(this, &VuMessageBoxManager::onFadeInEnter);
	pState->setTickMethod(this, &VuMessageBoxManager::onFadeInTick);

	pState = mFSM.addState("Active");
	pState->setTickMethod(this, &VuMessageBoxManager::onActiveTick);

	pState = mFSM.addState("FadeOut");
	pState->setEnterMethod(this, &VuMessageBoxManager::onFadeOutEnter);
	pState->setTickMethod(this, &VuMessageBoxManager::onFadeOutTick);
	pState->setExitMethod(this, &VuMessageBoxManager::onFadeOutExit);

	// add transitions
	mFSM.addTransition("Inactive", "FadeIn", "MessageBoxQueued");

	mFSM.addTransition("FadeIn", "Active", "FadeInComplete");

	mFSM.addTransition("Active", "FadeOut", "MessageBoxDestroyed");
	mFSM.addTransition("Active", "FadeOut", "MessageBoxClosed");

	mFSM.addTransition("FadeOut", "FadeIn", "FadeOutComplete & MessageBoxQueued");
	mFSM.addTransition("FadeOut", "Inactive", "FadeOutComplete");

	// event handlers
	REG_EVENT_HANDLER(VuMessageBoxManager, OnExitApp);
}

//*****************************************************************************
bool VuMessageBoxManager::init()
{
	mpMessageBoxDBAsset = VuAssetFactory::IF()->createAsset<VuDBAsset>("MessageBoxDB", VuAssetFactory::OPTIONAL_ASSET);

	VuTickManager::IF()->registerHandler(this, &VuMessageBoxManager::tick, "Final");
	VuDrawManager::IF()->registerHandler(this, &VuMessageBoxManager::draw);

	mFSM.begin();

	return true;
}

//*****************************************************************************
void VuMessageBoxManager::preRelease()
{
	mFSM.end();

	releaseActiveMessageBox();

	while ( mMessageBoxQueue.size() )
	{
		mMessageBoxQueue.front()->removeRef();
		mMessageBoxQueue.pop();
	}
}

//*****************************************************************************
void VuMessageBoxManager::release()
{
	VuTickManager::IF()->unregisterHandlers(this);
	VuDrawManager::IF()->unregisterHandler(this);

	VuAssetFactory::IF()->releaseAsset(mpMessageBoxDBAsset);
}

//*****************************************************************************
VuMessageBox *VuMessageBoxManager::create(const VuMessageBoxParams &params)
{
	VuMessageBox *pMessageBox = VUNULL;

	if ( mpMessageBoxDBAsset )
	{
		const std::string &projectAsset = mpMessageBoxDBAsset->getDB()[params.mType]["ProjectAsset"].asString();

		if ( VuAssetFactory::IF()->doesAssetExist<VuProjectAsset>(projectAsset) )
		{
			pMessageBox = new VuMessageBox;
			pMessageBox->addRef();

			pMessageBox->mParams = params;

			mMessageBoxQueue.push(pMessageBox);
		}
	}


	return pMessageBox;
}

//*****************************************************************************
void VuMessageBoxManager::destroy(VuMessageBox *pMessageBox)
{
	pMessageBox->mDestroyed = true;
	pMessageBox->removeRef();
}

//*****************************************************************************
void VuMessageBoxManager::tick(float fdt)
{
	// use real dt
	fdt = VuTickManager::IF()->getRealDeltaTime();

	// remove destroyed message boxes from front of queue
	while ( mMessageBoxQueue.size() && mMessageBoxQueue.front()->mDestroyed )
	{
		mMessageBoxQueue.front()->removeRef();
		mMessageBoxQueue.pop();
	}

	// look for new message box in queue
	if ( mMessageBoxQueue.size() )
		mFSM.pulseCondition("MessageBoxQueued");

	mFSM.evaluate();
	mFSM.tick(fdt);
}

//*****************************************************************************
void VuMessageBoxManager::draw()
{
	// draw message box entity
	if ( mpActiveMessageBox )
	{
		mpActiveMessageBox->draw();

		VuColor color = mBackgroundColor;
		color.mA = VuRound(color.mA*mFadeValue);
		VuGfxUtil::IF()->drawFilledRectangle2d(1.0f, color);
	}
}

//*****************************************************************************
void VuMessageBoxManager::releaseActiveMessageBox()
{
	if ( mpActiveMessageBox )
	{
		if ( mpActiveMessageBox->mParams.mPauseGame )
		{
			VuTickManager::IF()->popPauseRequest();
			VuAudio::IF()->popCategoryPause("game");
		}

		VuAssetFactory::IF()->releaseAsset(mpActiveMessageBox->mpImage);
		mpActiveMessageBox->mpProject->gameRelease();
		mpActiveMessageBox->mpProject->removeRef();
		mpActiveMessageBox->removeRef();
		mpActiveMessageBox = VUNULL;
	}
}

//*****************************************************************************
void VuMessageBoxManager::onFadeInEnter()
{
	mpActiveMessageBox = mMessageBoxQueue.front();
	mMessageBoxQueue.pop();

	if ( mpActiveMessageBox->mParams.mPauseGame )
	{
		VuTickManager::IF()->pushPauseRequest();
		VuAudio::IF()->pushCategoryPause("game");
	}

	const std::string &projectAsset = mpMessageBoxDBAsset->getDB()[mpActiveMessageBox->mParams.mType]["ProjectAsset"].asString();
	VuProjectAsset *pProjectAsset = VuAssetFactory::IF()->createAsset<VuProjectAsset>(projectAsset);

	VuProject *pProject = new VuProject;
	if ( pProject->load(pProjectAsset) )
	{
		if ( pProject->getRootEntity()->isDerivedFrom(VuUIScreenEntity::msRTTI) )
		{
			mpActiveMessageBox->mpProject = pProject;
			mpActiveMessageBox->mpScreen = static_cast<VuUIScreenEntity *>(pProject->getRootEntity());
			mpActiveMessageBox->mpScreen->setFullScreenLayer(VuGfxSort::FSL_MESSAGE_BOX);
			mpActiveMessageBox->mpScreen->setPriority(32768);
			if ( mpActiveMessageBox->mParams.mImage.length() )
				mpActiveMessageBox->mpImage = VuAssetFactory::IF()->createAsset<VuTextureAsset>(mpActiveMessageBox->mParams.mImage, VuAssetFactory::OPTIONAL_ASSET);
		}
	}

	VuAssetFactory::IF()->releaseAsset(pProjectAsset);

	VUASSERT(mpActiveMessageBox->mpScreen, "Unable to create MessageBox.");

	VuUI::IF()->pushFocus();
	mpActiveMessageBox->mpProject->gameInitialize();

	VuUIUtil::startTransitionIn(mpActiveMessageBox->mpScreen);
}

//*****************************************************************************
void VuMessageBoxManager::onFadeInTick(float fdt)
{
	float fadeValue = VuMin(mFSM.getTimeInState()/mMinFadeTime, 1.0f);
	mFadeValue = VuMax(mFadeValue, fadeValue);

	if ( VuUIUtil::tickTransition(mpActiveMessageBox->mpScreen, fdt) && mFadeValue >= 1.0f )
		mFSM.pulseCondition("FadeInComplete");
}

//*****************************************************************************
void VuMessageBoxManager::onActiveTick(float fdt)
{
	// tick message box
	mpActiveMessageBox->tick(fdt);

	if ( mpActiveMessageBox->mDestroyed )
		mFSM.pulseCondition("MessageBoxDestroyed");

	if ( mpActiveMessageBox->mClosed )
		mFSM.pulseCondition("MessageBoxClosed");
}

//*****************************************************************************
void VuMessageBoxManager::onFadeOutEnter()
{
	VuUI::IF()->popFocus();

	if ( !mpActiveMessageBox->mDestroyed )
	{
		if ( mpActiveMessageBox->mpCallback )
		{
			mpActiveMessageBox->mpCallback->onMessageBoxClosed(mpActiveMessageBox);
		}
	}

	VuUIUtil::startTransitionOut(mpActiveMessageBox->mpScreen);
}

//*****************************************************************************
void VuMessageBoxManager::onFadeOutTick(float fdt)
{
	if ( mMessageBoxQueue.empty() )
	{
		mFadeValue = VuMax(1.0f - mFSM.getTimeInState()/mMinFadeTime, 0.0f);
		if ( VuUIUtil::tickTransition(mpActiveMessageBox->mpScreen, fdt) && mFadeValue <= 0.0f )
			mFSM.pulseCondition("FadeOutComplete");
	}
	else
	{
		if ( VuUIUtil::tickTransition(mpActiveMessageBox->mpScreen, fdt) )
			mFSM.pulseCondition("FadeOutComplete");
	}
}

//*****************************************************************************
void VuMessageBoxManager::onFadeOutExit()
{
	releaseActiveMessageBox();
}
