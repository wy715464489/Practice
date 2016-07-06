//*****************************************************************************
//
//  Copyright (c) 2007-2008 Ralf Knoesel
//  Copyright (c) 2008-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  ScriptComponent class
// 
//*****************************************************************************

#include "VuScriptComponent.h"
#include "VuEngine/Entities/VuEntity.h"
#include "VuEngine/Properties/VuBasicProperty.h"
#include "VuEngine/Util/VuDataUtil.h"
#include "VuEngine/Json/VuJsonContainer.h"


IMPLEMENT_RTTI(VuScriptComponent, VuComponent);


//*****************************************************************************
VuScriptComponent::VuScriptComponent(VuEntity *pOwner, int defaultWidth, bool defaultEnabled) : VuComponent(pOwner),
	mTemplatedRefConnectionCount(0),
	mTemplatedPosition(0,0),
	mEnabled(defaultEnabled),
	mScriptTrace(true),
	mWidth(defaultWidth),
	mPosition(0,0),
	mDepth(0)
{
	// add properties
	addProperty(new VuBoolProperty("Enable", mEnabled));
	addProperty(new VuBoolProperty("Script Trace", mScriptTrace));
	addProperty(new VuIntProperty("Width", mWidth));
}

//*****************************************************************************
VuScriptComponent::~VuScriptComponent()
{
	// release plugs
	for ( int i = 0; i < (int)mPlugs.size(); i++ )
		mPlugs[i]->removeRef();

	// release refs
	for ( int i = 0; i < (int)mRefs.size(); i++ )
		mRefs[i]->removeRef();

	// release ref connections
	while ( mRefConnections.size() )
		mRefConnections[0]->disconnect();
}

//*****************************************************************************
void VuScriptComponent::onLoad(const VuJsonContainer &data)
{
	VuDataUtil::getValue(data["Width"], mWidth);
	VuDataUtil::getValue(data["Position"], mPosition);

	// plugs (optional)
	for ( int i = 0; i < (int)mPlugs.size(); i++ )
		mPlugs[i]->load(data["Plugs"]);

	// refs (optional)
	for ( int i = 0; i < (int)mRefs.size(); i++ )
		mRefs[i]->load(data["Refs"]);

	// ref connections
	loadRefConnections(data["RefConnections"]);
}

//*****************************************************************************
void VuScriptComponent::onSave(VuJsonContainer &data) const
{
	if ( VuDist(mPosition, mTemplatedPosition) > FLT_EPSILON )
		VuDataUtil::putValue(data["Position"], mPosition);

	// save plugs
	if ( mPlugs.size() )
		for ( int i = 0; i < (int)mPlugs.size(); i++ )
			mPlugs[i]->save(data["Plugs"]);

	// save refs
	if ( mRefs.size() )
		for ( int i = 0; i < (int)mRefs.size(); i++ )
			mRefs[i]->save(data["Refs"]);

	// save ref connections
	if ( mRefConnections.size() )
		saveRefConnections(data["RefConnections"]);
}

//*****************************************************************************
void VuScriptComponent::onApplyTemplate()
{
	// plugs
	for ( int i = 0; i < (int)mPlugs.size(); i++ )
		mPlugs[i]->applyTemplate();

	// refs
	for ( int i = 0; i < (int)mRefs.size(); i++ )
		mRefs[i]->applyTemplate();

	// ref connections
	mTemplatedRefConnectionCount = (int)mRefConnections.size();
	mTemplatedPosition = mPosition;
}

//*****************************************************************************
VuScriptPlug *VuScriptComponent::addPlug(VuScriptPlug *pPlug)
{
	pPlug->setOwnerScriptComponent(this);
	mPlugs.push_back(pPlug);

	return pPlug;
}

//*****************************************************************************
void VuScriptComponent::removePlug(const VuScriptPlug *pPlug)
{
	removePlug(getPlugIndex(pPlug));
}

//*****************************************************************************
void VuScriptComponent::removePlug(int index)
{
	if ( index >= 0 && index < (int)mPlugs.size())
	{
		mPlugs[index]->removeRef();
		mPlugs.erase(mPlugs.begin() + index);
	}
}

//*****************************************************************************
VuScriptRef *VuScriptComponent::addRef(VuScriptRef *pRef)
{
	mRefs.push_back(pRef);

	return pRef;
}

//*****************************************************************************
void VuScriptComponent::removeRef(const VuScriptRef *pRef)
{
	removeRef(getRefIndex(pRef));
}

//*****************************************************************************
void VuScriptComponent::removeRef(int index)
{
	if ( index >= 0 && index < (int)mRefs.size())
	{
		mRefs[index]->removeRef();
		mRefs.erase(mRefs.begin() + index);
	}
}

//*****************************************************************************
VuScriptPlug *VuScriptComponent::getPlug(const char *strName) const
{
	for ( int i = 0; i < getNumPlugs(); i++ )
		if ( getPlug(i)->getName() == strName )
			return getPlug(i);

	return VUNULL;
}

//*****************************************************************************
VuScriptPlug *VuScriptComponent::getPlug(const std::string &strName) const
{
	for ( int i = 0; i < getNumPlugs(); i++ )
		if ( getPlug(i)->getName() == strName )
			return getPlug(i);

	return VUNULL;
}

//*****************************************************************************
int VuScriptComponent::getPlugIndex(const VuScriptPlug *pPlug) const
{
	for ( int i = 0; i < (int)mPlugs.size(); i++ )
		if ( mPlugs[i] == pPlug )
			return i;

	return -1;
}

//*****************************************************************************
int VuScriptComponent::getNumPlugsOfType(bool bInput) const
{
	int count = 0;
	for ( int i = 0; i < getNumPlugs(); i++ )
		if ( getPlug(i)->isInput() == bInput )
			count++;

	return count;
}

//*****************************************************************************
VuScriptRef *VuScriptComponent::getRef(const char *strName) const
{
	for ( int i = 0; i < getNumRefs(); i++ )
		if ( strcmp(getRef(i)->getName(), strName) == 0 )
			return getRef(i);

	return VUNULL;
}

//*****************************************************************************
int VuScriptComponent::getRefIndex(const VuScriptRef *pRef) const
{
	for ( int i = 0; i < (int)mRefs.size(); i++ )
		if ( mRefs[i] == pRef )
			return i;

	return -1;
}

//*****************************************************************************
void VuScriptComponent::addRefConnection(VuScriptRef *pRef)
{
	mRefConnections.push_back(pRef);
}

//*****************************************************************************
void VuScriptComponent::removeRefConnection(VuScriptRef *pRef)
{
	for ( int i = 0; i < (int)mRefConnections.size(); i++ )
	{
		if ( mRefConnections[i] == pRef )
		{
			mRefConnections.erase(mRefConnections.begin() + i);
			break;
		}
	}
}

//*****************************************************************************
bool VuScriptComponent::isConnectedWith(const VuScriptRef *pRef) const
{
	return std::find(mRefConnections.begin(), mRefConnections.end(), pRef) != mRefConnections.end();
}

//*****************************************************************************
void VuScriptComponent::loadRefConnections(const VuJsonContainer &data)
{
	for ( int i = 0; i < data.size(); i++ )
	{
		std::string strEntity = data[i]["EntityName"].asString();
		std::string strRef = data[i]["RefName"].asString();

		// find entity
		if ( VuEntity *pEntity = getOwnerEntity()->findEntity(strEntity.c_str()) )
		{
			// get script component
			if ( VuScriptComponent *pScriptComponent = pEntity->getComponent<VuScriptComponent>() )
			{
				// find ref
				if ( VuScriptRef *pRef = pScriptComponent->getRef(strRef) )
				{
					// confirm compatibility
					if ( pRef->isCompatibleWith(getOwnerEntity()) )
					{
						// success
						pRef->connect(*this);
					}
				}
			}
		}
	}
}

//*****************************************************************************
void VuScriptComponent::saveRefConnections(VuJsonContainer &data) const
{
	for ( int i = mTemplatedRefConnectionCount; i < (int)mRefConnections.size(); i++ )
	{
		VuJsonContainer &dst = data.append();

		dst["EntityName"].putValue(mRefConnections[i]->getOwnerScriptComponent()->getOwnerEntity()->getLongName());
		dst["RefName"].putValue(mRefConnections[i]->getName());
	}
}
