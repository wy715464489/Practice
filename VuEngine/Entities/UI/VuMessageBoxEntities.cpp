//*****************************************************************************
//
//  Copyright (c) 2013-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  MessageBox entities
// 
//*****************************************************************************

#include "VuMessageBoxEntities.h"
#include "VuEngine/Components/Script/VuScriptComponent.h"
#include "VuEngine/Properties/VuBasicProperty.h"
#include "VuEngine/Properties/VuAssetProperty.h"
#include "VuEngine/Properties/VuDBEntryProperty.h"
#include "VuEngine/Assets/VuProjectAsset.h"
#include "VuEngine/Assets/VuTextureAsset.h"
#include "VuEngine/Managers/VuMessageBoxManager.h"


//*****************************************************************************
// MessageBox
//*****************************************************************************

IMPLEMENT_RTTI(VuMessageBoxEntity, VuEntity);
IMPLEMENT_ENTITY_REGISTRATION(VuMessageBoxEntity);

//*****************************************************************************
VuMessageBoxEntity::VuMessageBoxEntity():
	mpMessageBox(VUNULL)
{
	// components
	addComponent(mpScriptComponent = new VuScriptComponent(this));

	// properties
	addProperty(mpTypeProperty = new VuDBEntryProperty("Type", mMessageBoxParams.mType, "MessageBoxDB"));
	addProperty(new VuStringProperty("Heading String ID", mMessageBoxParams.mHeading));
	addProperty(new VuStringProperty("String ID", mMessageBoxParams.mBody));
	addProperty(new VuStringProperty("Text A", mMessageBoxParams.mTextA));
	addProperty(new VuStringProperty("Text B", mMessageBoxParams.mTextB));
	addProperty(new VuAssetNameProperty(VuTextureAsset::msRTTI.mstrType, "Image", mMessageBoxParams.mImage));
	addProperty(new VuBoolProperty("Pause Game", mMessageBoxParams.mPauseGame));

	mpTypeProperty->setNotifyOnLoad();
	mpTypeProperty->setWatcher(this, &VuMessageBoxEntity::modified);

	// scripting
	ADD_SCRIPT_INPUT_NOARGS(mpScriptComponent, VuMessageBoxEntity, Create);
	ADD_SCRIPT_OUTPUT_NOARGS(mpScriptComponent, OnClosed);

	modified();
}

//*****************************************************************************
VuMessageBoxEntity::~VuMessageBoxEntity()
{
	if ( mpMessageBox )
	{
		mpMessageBox->setCallback(VUNULL);
		VuMessageBoxManager::IF()->destroy(mpMessageBox);
		mpMessageBox = VUNULL;
	}
}

//*****************************************************************************
VuRetVal VuMessageBoxEntity::Create(const VuParams &params)
{
	mpMessageBox = VuMessageBoxManager::IF()->create(mMessageBoxParams);
	if ( mpMessageBox )
	{
		mpMessageBox->setCallback(this);

		onOpened();
	}

	return VuRetVal();
}

//*****************************************************************************
void VuMessageBoxEntity::onMessageBoxClosed(VuMessageBox *pMessageBox)
{
	onClosed();

	if ( VuScriptPlug *pResultPlug = mpScriptComponent->getPlug(mpMessageBox->getResult()) )
		pResultPlug->execute();

	mpMessageBox->removeRef();
	mpMessageBox = VUNULL;

	mpScriptComponent->getPlug("OnClosed")->execute();
}

//*****************************************************************************
void VuMessageBoxEntity::modified()
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
// CloseMessageBox
//*****************************************************************************

class VuCloseMessageBoxEntity : public VuEntity
{
	DECLARE_RTTI

public:
	VuCloseMessageBoxEntity();

protected:
	// scripting
	VuRetVal			Trigger(const VuParams &params);

	// components
	VuScriptComponent	*mpScriptComponent;

	// properties
	std::string			mMessageBoxResult;
};

IMPLEMENT_RTTI(VuCloseMessageBoxEntity, VuEntity);
IMPLEMENT_ENTITY_REGISTRATION(VuCloseMessageBoxEntity);


//*****************************************************************************
VuCloseMessageBoxEntity::VuCloseMessageBoxEntity()
{
	// components
	addComponent(mpScriptComponent = new VuScriptComponent(this));

	// properties
	addProperty(new VuStringProperty("MessageBox Result", mMessageBoxResult));

	// scripting
	ADD_SCRIPT_INPUT_NOARGS(mpScriptComponent, VuCloseMessageBoxEntity, Trigger);
}

//*****************************************************************************
VuRetVal VuCloseMessageBoxEntity::Trigger(const VuParams &params)
{
	if ( VuMessageBox *pMessageBox = VuMessageBoxManager::IF()->getActiveMessageBox() )
	{
		pMessageBox->setResult(mMessageBoxResult.c_str());
		pMessageBox->close();
	}

	return VuRetVal();
}
