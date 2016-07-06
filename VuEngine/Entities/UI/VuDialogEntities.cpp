//*****************************************************************************
//
//  Copyright (c) 2013-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  Dialog entities
// 
//*****************************************************************************

#include "VuDialogEntities.h"
#include "VuEngine/Components/Script/VuScriptComponent.h"
#include "VuEngine/Properties/VuBasicProperty.h"
#include "VuEngine/Properties/VuAssetProperty.h"
#include "VuEngine/Properties/VuDBEntryProperty.h"
#include "VuEngine/Assets/VuProjectAsset.h"
#include "VuEngine/Managers/VuDialogManager.h"


//*****************************************************************************
// Dialog
//*****************************************************************************

IMPLEMENT_RTTI(VuDialogEntity, VuEntity);
IMPLEMENT_ENTITY_REGISTRATION(VuDialogEntity);

//*****************************************************************************
VuDialogEntity::VuDialogEntity():
	mPauseGame(false),
	mpDialog(VUNULL)
{
	// components
	addComponent(mpScriptComponent = new VuScriptComponent(this));

	// properties
	addProperty(mpTypeProperty = new VuDBEntryProperty("Type", mDialogType, "DialogDB"));
	addProperty(new VuBoolProperty("Pause Game", mPauseGame));

	mpTypeProperty->setNotifyOnLoad();
	mpTypeProperty->setWatcher(this, &VuDialogEntity::modified);

	// scripting
	ADD_SCRIPT_INPUT_NOARGS(mpScriptComponent, VuDialogEntity, Show);
	ADD_SCRIPT_OUTPUT_NOARGS(mpScriptComponent, OnClosed);
}

//*****************************************************************************
VuDialogEntity::~VuDialogEntity()
{
	if ( mpDialog )
	{
		mpDialog->setCallback(VUNULL);
		VuDialogManager::IF()->destroy(mpDialog);
		mpDialog = VUNULL;
	}
}

//*****************************************************************************
VuRetVal VuDialogEntity::Show(const VuParams &params)
{
	if ( mpDialog == VUNULL )
	{
		mpDialog = VuDialogManager::IF()->create(mpTypeProperty->getEntryData()["ProjectAsset"].asCString());

		if ( mpDialog)
		{
			mpDialog->setCallback(this);
			mpDialog->setPauseGame(mPauseGame);

			onOpened();
		}
	}

	return VuRetVal();
}

//*****************************************************************************
void VuDialogEntity::onDialogClosed(VuDialog *pDialog)
{
	onClosed();

	if ( VuScriptPlug *pResultPlug = mpScriptComponent->getPlug(mpDialog->getResult()) )
		pResultPlug->execute();

	mpDialog->removeRef();
	mpDialog = VUNULL;

	mpScriptComponent->getPlug("OnClosed")->execute();
}

//*****************************************************************************
void VuDialogEntity::modified()
{
	for ( ResultPlugs::iterator iter = mResultPlugs.begin(); iter != mResultPlugs.end(); iter++ )
		mpScriptComponent->removePlug(*iter);

	mResultPlugs.clear();
	const VuJsonContainer &results = mpTypeProperty->getEntryData()["Results"];
	for ( int i = 0; i < results.size(); i++ )
		mResultPlugs.push_back(new VuScriptOutputPlug(results[i].asCString()));

	for ( ResultPlugs::iterator iter = mResultPlugs.begin(); iter != mResultPlugs.end(); iter++ )
		mpScriptComponent->addPlug(*iter);
}


//*****************************************************************************
// CloseDialog
//*****************************************************************************

class VuCloseDialogEntity : public VuEntity
{
	DECLARE_RTTI

public:
	VuCloseDialogEntity();

protected:
	// scripting
	VuRetVal			Trigger(const VuParams &params);

	// components
	VuScriptComponent	*mpScriptComponent;

	// properties
	std::string			mDialogResult;
};

IMPLEMENT_RTTI(VuCloseDialogEntity, VuEntity);
IMPLEMENT_ENTITY_REGISTRATION(VuCloseDialogEntity);


//*****************************************************************************
VuCloseDialogEntity::VuCloseDialogEntity()
{
	// components
	addComponent(mpScriptComponent = new VuScriptComponent(this));

	// properties
	addProperty(new VuStringProperty("Dialog Result", mDialogResult));

	// scripting
	ADD_SCRIPT_INPUT_NOARGS(mpScriptComponent, VuCloseDialogEntity, Trigger);
}

//*****************************************************************************
VuRetVal VuCloseDialogEntity::Trigger(const VuParams &params)
{
	if ( VuDialog *pDialog = VuDialogManager::IF()->getActiveDialog() )
	{
		pDialog->setResult(mDialogResult.c_str());
		pDialog->close();
	}

	return VuRetVal();
}
