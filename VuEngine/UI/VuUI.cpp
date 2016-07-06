//*****************************************************************************
//
//  Copyright (c) 2008-2008 Ralf Knoesel
//  Copyright (c) 2008-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  Interface class to UI library.
// 
//*****************************************************************************

#include "VuUI.h"
#include "VuEngine/Assets/VuAssetFactory.h"
#include "VuEngine/Assets/VuFontAsset.h"
#include "VuEngine/Managers/VuTickManager.h"


// the interface
IMPLEMENT_SYSTEM_COMPONENT(VuUI, VuUI);


//*****************************************************************************
VuUI::VuUI():
	mpDefaultFontAsset(VUNULL)
{
	mCropMatrix.loadIdentity();
	mInvCropMatrix.loadIdentity();
	mTextScale = 1.0f;
	setAuthoringScreenScale(1280.0f, 720.0f);
}

//*****************************************************************************
bool VuUI::init()
{
	// basic events
	registerEvent("ScreenEnter");
	registerEvent("ScreenExit");

	registerEvent("GamePadInput");

	if ( VuAssetFactory::IF()->doesAssetExist<VuFontAsset>("Dev") )
		mpDefaultFontAsset = VuAssetFactory::IF()->createAsset<VuFontAsset>("Dev");

	VuTickManager::IF()->registerHandler(this, &VuUI::tickInput, "Input");

	return true;
}

//*****************************************************************************
void VuUI::release()
{
	VuAssetFactory::IF()->releaseAsset(mpDefaultFontAsset);

	mEvents.clear();

	VuTickManager::IF()->unregisterHandlers(this);
}

//*****************************************************************************
void VuUI::setAuthoringScreenScale(float scaleX, float scaleY)
{
	mAuthoringScreenScale.mX = scaleX;
	mAuthoringScreenScale.mY = scaleY;
	mAuthoringAspectRatio = scaleX/scaleY;
}

//*****************************************************************************
void VuUI::registerEvent(const char *strName)
{
	mEvents.push_back(strName);
}

//*****************************************************************************
void VuUI::setCropMatrix(const VuMatrix &mat)
{
	mCropMatrix = mat;
	mInvCropMatrix = mat;
	mInvCropMatrix.invert();
}

//*****************************************************************************
VuFont *VuUI::getDefaultFont()
{
	if ( mpDefaultFontAsset )
		return mpDefaultFontAsset->getFont();

	return VUNULL;
}

//*****************************************************************************
void VuUI::setFocus(VuEntity *pEntity)
{
	mpFocusEntity = VUNULL;
	mpFocusPendingEntity = pEntity;
}

//*****************************************************************************
void VuUI::pushFocus()
{
	mFocusStack.push(mpFocusEntity);
	mpFocusEntity = VUNULL;
}

//*****************************************************************************
void VuUI::popFocus()
{
	if ( mFocusStack.size() )
	{
		mpFocusEntity = mFocusStack.top();
		mFocusStack.pop();
	}
	else
	{
		mpFocusEntity = VUNULL;
	}
}

//*****************************************************************************
void VuUI::tickInput(float fdt)
{
	if ( mpFocusPendingEntity )
	{
		mpFocusEntity = mpFocusPendingEntity;
		mpFocusPendingEntity = VUNULL;
	}
}
