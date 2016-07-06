//*****************************************************************************
//
//  Copyright (c) 2008-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  Pfx Node
// 
//*****************************************************************************

#include "VuPfxNode.h"
#include "VuPfxGroup.h"
#include "VuPfxSystem.h"
#include "VuPfxPattern.h"
#include "VuPfxProcess.h"
#include "VuPfx.h"
#include "VuPfxRegistry.h"
#include "VuEngine/Json/VuJsonContainer.h"
#include "VuEngine/Json/VuFastContainer.h"
#include "VuEngine/Util/VuBinaryDataUtil.h"


IMPLEMENT_RTTI_BASE(VuPfxNode);


//*****************************************************************************
VuPfxNode::~VuPfxNode()
{
	for ( ChildNodes::iterator iter = mChildNodes.begin(); iter != mChildNodes.end(); iter++ )
		iter->second->removeRef();
}

//*****************************************************************************
void VuPfxNode::load(const VuJsonContainer &data)
{
	mProperties.load(data["Properties"]);

	loadChildNodes(data["ChildNodes"]);

	onLoad();
}

//*****************************************************************************
void VuPfxNode::save(VuJsonContainer &data) const
{
	data["Type"].putValue(getType());

	if ( isDerivedFrom(VuPfxGroup::msRTTI) )		data["BaseType"].putValue("group");
	else if ( isDerivedFrom(VuPfxSystem::msRTTI) )	data["BaseType"].putValue("system");
	else if ( isDerivedFrom(VuPfxPattern::msRTTI) )	data["BaseType"].putValue("pattern");
	else if ( isDerivedFrom(VuPfxProcess::msRTTI) )	data["BaseType"].putValue("process");

	if ( mProperties.hasProperties() )
		mProperties.save(data["Properties"]);

	if ( mChildNodes.size() )
		saveChildNodes(data["ChildNodes"]);
}

//*****************************************************************************
void VuPfxNode::load(const VuFastContainer &data)
{
	mProperties.load(data["Properties"]);

	loadChildNodes(data["ChildNodes"]);

	onLoad();
}

//*****************************************************************************
void VuPfxNode::loadChildNodes(const VuJsonContainer &data)
{
	for ( int i = 0; i < data.numMembers(); i++ )
	{
		const std::string &name = data.getMemberKey(i);
		const std::string &type = data[name]["Type"].asString();
		const std::string &baseType = data[name]["BaseType"].asString();

		VuPfxNode *pChildNode = VUNULL;
		if ( baseType == "group" )
			pChildNode = new VuPfxGroup;
		else if ( baseType == "system" )
			pChildNode = new VuPfxSystem;
		else if ( baseType == "pattern" )
			pChildNode = VuPfx::IF()->registry()->createPattern(type.c_str());
		else if ( baseType == "process" )
			pChildNode = VuPfx::IF()->registry()->createProcess(getType(), type.c_str());

		if ( pChildNode )
		{
			pChildNode->mName = name;
			pChildNode->load(data[name]);
			mChildNodes[name] = pChildNode;
		}
	}
}

//*****************************************************************************
void VuPfxNode::saveChildNodes(VuJsonContainer &data) const
{
	// save nodes
	for ( ChildNodes::const_iterator iter = mChildNodes.begin(); iter != mChildNodes.end(); iter++ )
		iter->second->save(data[iter->first]);
}

//*****************************************************************************
void VuPfxNode::loadChildNodes(const VuFastContainer &data)
{
	static VUUINT32 sHashedBaseTypeGroup = VuHash::fnv32String("group");
	static VUUINT32 sHashedBaseTypeSystem = VuHash::fnv32String("system");
	static VUUINT32 sHashedBaseTypePattern = VuHash::fnv32String("pattern");
	static VUUINT32 sHashedBaseTypeProcess = VuHash::fnv32String("process");

	for ( int i = 0; i < data.numMembers(); i++ )
	{
		const VuFastContainer &childData = data.getMember(i);

		const char *name = data.getMemberKey(i);
		const char *type = childData["Type"].asCString();
		const char *baseType = childData["BaseType"].asCString();

		VUUINT32 hashedBaseType = VuHash::fnv32String(baseType);

		VuPfxNode *pChildNode = VUNULL;
		if ( hashedBaseType == sHashedBaseTypeGroup )
			pChildNode = new VuPfxGroup;
		else if ( hashedBaseType == sHashedBaseTypeSystem )
			pChildNode = new VuPfxSystem;
		else if ( hashedBaseType == sHashedBaseTypePattern )
			pChildNode = VuPfx::IF()->registry()->createPattern(type);
		else if ( hashedBaseType == sHashedBaseTypeProcess )
			pChildNode = VuPfx::IF()->registry()->createProcess(getType(), type);

		if ( pChildNode )
		{
			pChildNode->mName = name;
			pChildNode->load(childData);
			mChildNodes[name] = pChildNode;
		}
	}
}
