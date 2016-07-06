//*****************************************************************************
//
//  Copyright (c) 2006-2008 Ralf Knoesel
//  Copyright (c) 2008-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  VuEntity class
// 
//*****************************************************************************

#include "VuEntity.h"
#include "VuEntityFactory.h"
#include "VuEntityRepository.h"
#include "VuEntityUtil.h"
#include "VuEngine/Components/VuComponent.h"
#include "VuEngine/Components/Transform/VuTransformComponent.h"
#include "VuEngine/Assets/VuTemplateAsset.h"
#include "VuEngine/Assets/VuAssetFactory.h"
#include "VuEngine/Json/VuJsonContainer.h"


IMPLEMENT_RTTI_BASE(VuEntity);
IMPLEMENT_ENTITY_REGISTRATION(VuEntity);


// static functions
static bool ChildEntityComp(const VuEntity *p1, const VuEntity *p2)
{
	if (  p1->canHaveChildren() && !p2->canHaveChildren() ) return true;
	if ( !p1->canHaveChildren() &&  p2->canHaveChildren() ) return false;

	return p1->getShortName() < p2->getShortName();
}


//*****************************************************************************
VuEntity::VuEntity(VUUINT32 flags):
	mEntityFlags(flags),
	mHashedLongNameFast(VU_FNV32_INIT),
	mpParentEntity(VUNULL),
	mpTemplateAsset(VUNULL)
{
	addComponent(mpTransformComponent = new VuTransformComponent(this));
}

//*****************************************************************************
VuEntity::~VuEntity()
{
	// let parent know
	if ( mpParentEntity )
		mpParentEntity->removeChildEntity(this);

	// release reference to template asset
	if ( mpTemplateAsset )
		VuAssetFactory::IF()->releaseAsset(mpTemplateAsset);

	// release child entities
	clearChildEntities();
}

//*****************************************************************************
void VuEntity::load(const VuJsonContainer &data)
{
	if ( mpTemplateAsset )
	{
		loadTemplated(data);
	}
	else
	{
		loadChildEntities(data["ChildEntities"]);
		mProperties.load(data["Properties"]);
		mComponents.load(data["Components"]);

		onLoad(data);
	}
}

//*****************************************************************************
void VuEntity::postLoad(VUUINT32 parentHash)
{
	// optimized hashed long name
	mHashedLongNameFast = VuHash::fnv32String(getShortName().c_str(), parentHash);

	// handle children
	parentHash = VuHash::fnv32String("/", mHashedLongNameFast);
	for ( int i = 0; i < getChildEntityCount(); i++ )
		getChildEntity(i)->postLoad(parentHash);

	// components
	mComponents.postLoad();

	// hook for derived classes
	onPostLoad();
}

//*****************************************************************************
void VuEntity::save(VuJsonContainer &data) const
{
	if ( mpTemplateAsset )
	{
		saveTemplated(data);
	}
	else
	{
		if ( getChildEntityCount() )
			saveChildEntities(data["ChildEntities"]);

		if ( mProperties.hasProperties() )
			mProperties.save(data["Properties"]);

		if ( mComponents.hasComponents() )
			mComponents.save(data["Components"]);

		onSave(data);
	}
}

//*****************************************************************************
void VuEntity::bake()
{
	for ( int i = 0; i < getChildEntityCount(); i++ )
		getChildEntity(i)->bake();

	mComponents.bake();

	onBake();
}

//*****************************************************************************
void VuEntity::clearBaked()
{
	for ( int i = 0; i < getChildEntityCount(); i++ )
		getChildEntity(i)->clearBaked();

	mComponents.clearBaked();

	onClearBaked();
}

//*****************************************************************************
void VuEntity::gameInitialize()
{
	if ( !isGameInitialized() )
	{
		if ( !(mEntityFlags & IGNORE_REPOSITORY) )
			VuEntityRepository::IF()->addEntity(this);

		mComponents.gameInitialize();

		for ( int i = 0; i < getChildEntityCount(); i++ )
			getChildEntity(i)->gameInitialize();

		onGameInitialize();

		mEntityFlags |= GAME_INITIALIZED;
	}
}

//*****************************************************************************
void VuEntity::gameRelease()
{
	if ( isGameInitialized() )
	{
		mComponents.gameRelease();

		for ( int i = 0; i < getChildEntityCount(); i++ )
			getChildEntity(i)->gameRelease();

		onGameRelease();

		if ( !(mEntityFlags & IGNORE_REPOSITORY) )
			VuEntityRepository::IF()->removeEntity(this);

		mEntityFlags &= ~GAME_INITIALIZED;
	}
}

//*****************************************************************************
void VuEntity::gameReset()
{
	if ( !isGameInitialized() )
	{
		mProperties.reset();

		mComponents.gameReset();

		for ( int i = 0; i < getChildEntityCount(); i++ )
			getChildEntity(i)->gameReset();

		onGameReset();
	}
}

//*****************************************************************************
std::string VuEntity::getCreationType() const
{
	if ( isTemplateRoot() )
		return std::string("#") + mpTemplateAsset->getAssetName();

	return getType();
}

//*****************************************************************************
void VuEntity::setShortName(const std::string &strShortName)
{
	mstrShortName = strShortName;
}

//*****************************************************************************
std::string VuEntity::getLongName() const
{
	std::string longName = getShortName();

	for ( VuEntity *pParent = getParentEntity(); pParent; pParent = pParent->getParentEntity() )
		longName = pParent->getShortName() + "/" + longName;

	return longName;
}

//*****************************************************************************
VUUINT32 VuEntity::getHashedLongName() const
{
	return VuHash::fnv32String(getLongName().c_str());
}

//*****************************************************************************
void VuEntity::setParentEntity(VuEntity *pParentEntity)
{
	mpParentEntity = pParentEntity;
}

//*****************************************************************************
bool VuEntity::isParentOf(const VuEntity *pChildEntity)
{
	while ( (pChildEntity = pChildEntity->getParentEntity()) != VUNULL)
	{
		if ( pChildEntity == this )
			return true;
	}

	return false;
}

//*****************************************************************************
void VuEntity::clearChildEntities()
{
	for ( int i = 0; i < getChildEntityCount(); i++ )
	{
		getChildEntity(i)->setParentEntity(VUNULL);
		getChildEntity(i)->removeRef();
	}
	mChildEntities.clear();
}

//*****************************************************************************
void VuEntity::addChildEntity(VuEntity *pEntity)
{
	mChildEntities.push_back(pEntity);
	pEntity->setParentEntity(this);

	sortChildEntities();
}

//*****************************************************************************
// This method is not very efficient.  This is ok because it is only used
// by the editor.
//*****************************************************************************
bool VuEntity::removeChildEntity(VuEntity *pEntity)
{
	// find entity
	ChildEntities::iterator iter = std::find(mChildEntities.begin(), mChildEntities.end(), pEntity);
	if ( iter != mChildEntities.end() )
	{
		pEntity->setParentEntity(VUNULL);
		mChildEntities.erase(iter);
		return true;
	}

	return false;
}

//*****************************************************************************
void VuEntity::sortChildEntities()
{
	std::sort(mChildEntities.begin(), mChildEntities.end(), ChildEntityComp);
}

//*****************************************************************************
VuEntity *VuEntity::getChildEntity(const std::string &strShortName) const
{
	for ( int i = 0; i < getChildEntityCount(); i++ )
		if ( getChildEntity(i)->getShortName() == strShortName )
			return getChildEntity(i);

	return VUNULL;
}

//*****************************************************************************
VuEntity *VuEntity::getRootEntity()
{
	VuEntity *pEntity = this;

	while ( pEntity->getParentEntity() )
		pEntity = pEntity->getParentEntity();

	return pEntity;
}

//*****************************************************************************
VuEntity *VuEntity::findEntity(const std::string &strLongName)
{
	VuEntity *pEntity = getRootEntity();
	std::string strPartialName = strLongName;

	while ( strPartialName != pEntity->getShortName() )
	{
		strPartialName = VuEntityUtil::subtractRoot(strPartialName);
		pEntity = pEntity->getChildEntity(VuEntityUtil::getRoot(strPartialName));
		if ( !pEntity )
			return VUNULL;
	}

	return pEntity;
}

//*****************************************************************************
VuProperty *VuEntity::getProperty(const std::string &strName) const
{
	// find entity property
	VuProperty *pProperty = mProperties.get(strName);

	// if not found, try components
	if ( !pProperty )
	{
		std::string path = VuEntityUtil::getPath(strName);
		std::string name = VuEntityUtil::getName(strName);
		for ( VuComponent *pComponent = mComponents.getFirst(); pComponent; pComponent = pComponent->getNextComponent() )
		{
			if ( path == pComponent->getShortComponentType() )
			{
				pProperty = pComponent->getProperties().get(name);
				break;
			}
		}
	}

	return pProperty;
}

//*****************************************************************************
void VuEntity::handleEvent(VUUINT32 key, const VuParams &params) const
{
	mEventMap.handle(key, params);
}

//*****************************************************************************
void VuEntity::handleEventRecursive(VUUINT32 key, const VuParams &params) const
{
	mEventMap.handle(key, params);

	// recurse
	for ( int i = 0; i < getChildEntityCount(); i++ )
		getChildEntity(i)->handleEventRecursive(key, params);
}

//*****************************************************************************
bool VuEntity::isTemplateChild() const
{
	for ( VuEntity *pParent = getParentEntity(); pParent; pParent = pParent->getParentEntity() )
		if ( pParent->isTemplateRoot() )
			return true;

	return false;
}

//*****************************************************************************
void VuEntity::applyTemplate(VuTemplateAsset *pTemplateAsset)
{
	// load from template
	load(pTemplateAsset->getTemplate());

	mpTemplateAsset = pTemplateAsset;

	// recurse
	applyTemplateRecursive();
}

//*****************************************************************************
void VuEntity::loadChildEntities(const VuJsonContainer &data)
{
	std::map<std::string, const VuJsonContainer *> entityData;

	// create entities
	for ( int iEntity = 0; iEntity < data.size(); iEntity++ )
	{
		const VuJsonContainer &entity = data[iEntity];

		std::string strType, strName;
		if ( entity["type"].getValue(strType) && entity["name"].getValue(strName) )
		{
			entityData[strName] = &entity["data"];

			// check if entity already exists
			if ( !getChildEntity(strName) )
			{
				// create entity
				VuEntity *pEntity = VuEntityFactory::IF()->createEntity(strType);
				if ( !pEntity )
				{
					VUWARNING("Unable to create entity of type '%s'.", strType.c_str());
				}
				else
				{
					pEntity->setShortName(strName);
					addChildEntity(pEntity);
				}
			}
		}
	}

	// load entities
	for ( int iEntity = 0; iEntity < getChildEntityCount(); iEntity++ )
	{
		VuEntity *pEntity = getChildEntity(iEntity);

		pEntity->load(*entityData[pEntity->getShortName()]);
	}

	sortChildEntities();
}

//*****************************************************************************
void VuEntity::saveChildEntities(VuJsonContainer &data) const
{
	// save entities
	for ( int i = 0; i < getChildEntityCount(); i++ )
	{
		const VuEntity *pEntity = getChildEntity(i);

		data[i]["name"].putValue(pEntity->getShortName());
		data[i]["type"].putValue(pEntity->getCreationType());
		pEntity->save(data[i]["data"]);
	}
}

//*****************************************************************************
void VuEntity::loadTemplated(const VuJsonContainer &data)
{
	// children
	for ( int i = 0; i < getChildEntityCount(); i++ )
		getChildEntity(i)->loadTemplated(data["ChildEntities"][getChildEntity(i)->getShortName()]);

	// load properties which are different from template
	// (because defaults were changed when template was applied)
	mProperties.load(data["Properties"]);

	// load templated components
	mComponents.load(data["Components"]);

	onLoad(data);
}

//*****************************************************************************
void VuEntity::saveTemplated(VuJsonContainer &data) const
{
	// children
	for ( int i = 0; i < getChildEntityCount(); i++ )
		getChildEntity(i)->saveTemplated(data["ChildEntities"][getChildEntity(i)->getShortName()]);

	// save properties which are different from template
	// (because defaults were changed when template was applied)
	if ( mProperties.hasProperties() )
		mProperties.save(data["Properties"]);

	// save templated components
	if ( mComponents.hasComponents() )
		mComponents.save(data["Components"]);

	onSave(data);
}

//*****************************************************************************
void VuEntity::applyTemplateRecursive()
{
	if ( !isTemplateRoot() )
		mEntityFlags |= LOCKED_CHILD;

	mProperties.updateDefaults();

	mComponents.applyTemplate();

	for ( int i = 0; i < getChildEntityCount(); i++ )
		getChildEntity(i)->applyTemplateRecursive();
}
