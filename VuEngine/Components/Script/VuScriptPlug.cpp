//*****************************************************************************
//
//  Copyright (c) 2007-2008 Ralf Knoesel
//  Copyright (c) 2008-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  ScriptPlug class
// 
//*****************************************************************************

#include "VuScriptPlug.h"
#include "VuScriptComponent.h"
#include "VuEngine/Entities/VuEntity.h"
#include "VuEngine/Json/VuJsonContainer.h"
#include "VuEngine/Dev/VuDevConfig.h"
#include "VuEngine/Dev/VuDevMenu.h"
#include "VuEngine/Dev/VuDevUtil.h"


bool VuScriptPlug::smScriptTrace = false;


//*****************************************************************************
VuScriptPlug::VuScriptPlug(const char *strName, VuRetVal::eType retType, const VuParamDecl &paramDecl):
	mstrName(strName),
	mRetType(retType),
	mParamDecl(paramDecl),
	mpOwnerScriptComponent(VUNULL),
	mTemplatedConnectionCount(0)
{
	static VuDevBoolOnce sOnce;

	if ( sOnce )
	{
		if ( VuDevConfig::IF() )
		{
			VuDevConfig::IF()->getParam("ScriptTrace").getValue(smScriptTrace);
		}
		if ( VuDevMenu::IF() )
		{
			VuDevMenu::IF()->addBool("Script/Trace", smScriptTrace);
		}
	}
}

//*****************************************************************************
VuScriptPlug::~VuScriptPlug()
{
	// disconnect myself from anything I'm connected to
	while ( mConnections.size() )
		mConnections[0]->disconnect(*this);
}

//*****************************************************************************
void VuScriptPlug::load(const VuJsonContainer &data)
{
	// connections
	loadConnections(data[getName()]["Connections"]);
}

//*****************************************************************************
void VuScriptPlug::save(VuJsonContainer &data) const
{
	// save connections
	if ( mConnections.size() )
		saveConnections(data[getName()]["Connections"]);
}

//*****************************************************************************
bool VuScriptPlug::areCompatible(const VuScriptPlug &a, const VuScriptPlug &b)
{
	// make sure that we have one input and one output
	if ( a.isInput() == b.isInput() )
		return false;

	// determine input (callee) and output (caller) plug
	const VuScriptPlug &in = a.isInput() ? a : b;
	const VuScriptPlug &out = a.isInput() ? b : a;

	// does the caller expect a return value?
	if ( out.getRetType() != VuRetVal::Void )
	{
		// make sure return types match
		if ( out.getRetType() != in.getRetType() )
			return false;

		// if expecting a return value, only a single connection is allowed for caller
		if ( out.getNumConnections() )
			return false;
	}

	// make sure that caller (output) has at least as many arguments as callee (input)
	if ( out.getParamDecl().mNumParams < in.getParamDecl().mNumParams )
		return false;

	// make sure that any callee (input) arguments are matched by caller (output)
	for ( int i = 0; i < in.getParamDecl().mNumParams; i++ )
		if ( in.getParamDecl().maParamTypes[i] != out.getParamDecl().maParamTypes[i] )
			return false;

	// check if already connected
	for ( int i = 0; i < in.getNumConnections(); i++ )
		if ( in.getConnection(i) == &out )
			return false;

	for ( int i = 0; i < out.getNumConnections(); i++ )
		if ( out.getConnection(i) == &in )
			return false;

	return true;
}

//*****************************************************************************
void VuScriptPlug::connect(VuScriptPlug &plug)
{
	// check for compatibility
	if ( !areCompatible(*this, plug) )
		return;

	mConnections.push_back(&plug);
	plug.mConnections.push_back(this);
}

//*****************************************************************************
void VuScriptPlug::disconnect(VuScriptPlug &plug)
{
	for ( int i = 0; i < (int)mConnections.size(); i++ )
	{
		if ( mConnections[i] == &plug )
		{
			mConnections.erase(mConnections.begin() + i);
			plug.disconnect(*this);
			break;
		}
	}
}

//*****************************************************************************
VuRetVal VuScriptPlug::execConnections(const VuParams &params) const
{
	VuRetVal retVal;

	// script tracing
	if ( smScriptTrace && mpOwnerScriptComponent->scriptTrace() && mConnections.size() )
	{
		VUPRINTF("SCRIPT TRACE: %s (%s)\n", getName().c_str(), mpOwnerScriptComponent->getOwnerEntity()->getLongName().c_str());
		for ( int i = 0; i < (int)mConnections.size(); i++  )
			VUPRINTF("           -> %s (%s)\n", mConnections[i]->getName().c_str(), mConnections[i]->getOwnerScriptComponent()->getOwnerEntity()->getLongName().c_str());
	}

	if ( mRetType == VuRetVal::Void )
	{
		for ( int i = 0; i < (int)mConnections.size(); i++  )
			mConnections[i]->execute(params);
	}
	else if ( mConnections.size() )
	{
		// if return type is not void, only a single connection is allowed
		VUASSERT(mConnections.size() == 1, "VuScriptPlug::execConnections() multiple connections with return param");

		retVal = mConnections[0]->execute(params);
	}

	return retVal;
}

//*****************************************************************************
void VuScriptPlug::loadConnections(const VuJsonContainer &data)
{
	// load
	for ( int i = 0; i < data.size(); i++ )
	{
		std::string strEntity = data[i]["EntityName"].asString();
		std::string strPlug = data[i]["PlugName"].asString();

		// find entity
		if ( VuEntity *pEntity = getOwnerScriptComponent()->getOwnerEntity()->findEntity(strEntity.c_str()) )
		{
			// get script component
			if ( VuScriptComponent *pScriptComponent = pEntity->getComponent<VuScriptComponent>() )
			{
				// find plug
				if ( VuScriptPlug *pDstPlug = pScriptComponent->getPlug(strPlug) )
				{
					// success
					connect(*pDstPlug);
				}
			}
		}
	}
}

//*****************************************************************************
void VuScriptPlug::saveConnections(VuJsonContainer &data) const
{
	for ( int i = mTemplatedConnectionCount; i < (int)mConnections.size(); i++ )
	{
		VuJsonContainer &dst = data.append();

		dst["EntityName"].putValue(mConnections[i]->getOwnerScriptComponent()->getOwnerEntity()->getLongName());
		dst["PlugName"].putValue(mConnections[i]->getName());
	}

	// sort connections
	for ( int i = 0; i < data.size(); i++ )
	{
		for ( int j = i + 1; j < data.size(); j++ )
		{
			int entityComp = strcmp(data[i]["EntityName"].asCString(), data[j]["EntityName"].asCString());
			int plugComp = strcmp(data[i]["PlugName"].asCString(), data[j]["PlugName"].asCString());
			if ( entityComp > 0 || (entityComp == 0 && plugComp > 0) )
				VuSwap(data[i], data[j]);
		}
	}
}

//*****************************************************************************
VuRetVal VuScriptOutputPlug::execute(const VuParams &params) const
{
	return execConnections(params);
}
