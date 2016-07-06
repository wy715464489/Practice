//*****************************************************************************
//
//  Copyright (c) 2007-2008 Ralf Knoesel
//  Copyright (c) 2008-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  EntityRepository class
// 
//*****************************************************************************

#include "VuEntityRepository.h"
#include "VuEngine/Projects/VuProject.h"
#include "VuEngine/Entities/VuEntity.h"
#include "VuEngine/Entities/VuEntityUtil.h"
#include "VuEngine/Managers/VuTickManager.h"
#include "VuEngine/Util/VuHash.h"
#include "VuEngine/Dev/VuDevStat.h"


// the interface
IMPLEMENT_SYSTEM_COMPONENT(VuEntityRepository, VuEntityRepository);


//*****************************************************************************
VuEntityRepository::VuEntityRepository()
{
	if ( VuDevStat::IF() )
		VuDevStat::IF()->addPage("EntityRepository", VuRect(50, 10, 40, 80));
}

//*****************************************************************************
VuEntityRepository::~VuEntityRepository()
{
	// make sure that all entities have been released
	VUASSERT(mEntities.size() == 0, "VuEntityRepository::~VuEntityRepository() entity leak" );
	VUASSERT(mProjects.size() == 0, "VuEntityRepository::~VuEntityRepository() project leak" );
}

//*****************************************************************************
bool VuEntityRepository::init()
{
	VuTickManager::IF()->registerHandler(this, &VuEntityRepository::tick, "PostDecision");

	return true;
}

//*****************************************************************************
void VuEntityRepository::preRelease()
{
	resetManagedEntities();
}

//*****************************************************************************
void VuEntityRepository::release()
{
	VuTickManager::IF()->unregisterHandlers(this);
}

//*****************************************************************************
void VuEntityRepository::addProject(VuProject *pProject)
{
	// verify namespace doesn't exist already
	if ( mProjects.find(pProject->getName()) != mProjects.end() )
	{
		VUPRINTF("Duplicate projects: '%s'\n", pProject->getName().c_str());
		VUASSERT(0, "VuEntityRepository::addProject() duplicate projects");
	}

	// verify namespace doesn't exist already
	VUASSERT(mProjects.find(pProject->getName()) == mProjects.end(), "VuEntityRepository::addProject() duplicate projects");

	// add project
	mProjects.insert(pProject->getName());
}

//*****************************************************************************
void VuEntityRepository::removeProject(VuProject *pProject)
{
	// find project
	Projects::iterator itProject = mProjects.find(pProject->getName());

	// verify namespace exists
	VUASSERT(itProject != mProjects.end(), "VuEntityRepository::removeProject() project not found");

	// erase project
	mProjects.erase(itProject);
}

//*****************************************************************************
void VuEntityRepository::addEntity(VuEntity *pEntity)
{
	if ( VuEntity *pFoundEntity = findEntity(pEntity->getHashedLongNameFast()) )
	{
		VUPRINTF("Name collision: '%s' and '%s'\n", pEntity->getLongName().c_str(), pFoundEntity->getLongName().c_str());
		VUASSERT(0, "VuEntityRepository::addEntity entity already registered");
	}

	mEntities[pEntity->getHashedLongNameFast()] = pEntity;
}

//*****************************************************************************
void VuEntityRepository::removeEntity(VuEntity *pEntity)
{
	Entities::iterator iter = mEntities.find(pEntity->getHashedLongNameFast());
	VUASSERT(iter != mEntities.end(), "VuEntityRepository::removeEntity entity not registered");

	mEntities.erase(iter);
}

//*****************************************************************************
void VuEntityRepository::addManagedEntity(VuEntity *pEntity)
{
	VUUINT64 perfCounter = VuSys::IF()->getPerfCounter();

	char strShortName[64];
	VU_SPRINTF(strShortName, sizeof(strShortName), "ManagedpEntity_%08x%08x", VUUINT32(perfCounter>>32), VUUINT32(perfCounter&0xffffffff));
	pEntity->setShortName(strShortName);
	pEntity->load(VuJsonContainer::null);
	pEntity->postLoad();
	pEntity->gameInitialize();

	mManagedEntities.push_back(ManagedEntity(pEntity));
}

//*****************************************************************************
void VuEntityRepository::removeManagedEntity(VuEntity *pEntity)
{
	for ( ManagedEntities::iterator iter = mManagedEntities.begin(); iter != mManagedEntities.end(); iter++ )
		if ( iter->mpEntity == pEntity )
			iter->mRemove = true;
}

//*****************************************************************************
void VuEntityRepository::resetManagedEntities()
{
	for ( ManagedEntities::iterator iter = mManagedEntities.begin(); iter != mManagedEntities.end(); iter++ )
	{
		iter->mpEntity->gameRelease();
		iter->mpEntity->removeRef();
	}
	mManagedEntities.clear();
}

//*****************************************************************************
VuEntity *VuEntityRepository::findEntity(const char *strEntityLongName)
{
	Entities::iterator iter = mEntities.find(VuHash::fnv32String(strEntityLongName));
	if ( iter != mEntities.end() )
		return iter->second;

	return VUNULL;
}

//*****************************************************************************
VuEntity *VuEntityRepository::findEntity(VUUINT32 hashedLongName)
{
	Entities::iterator iter = mEntities.find(hashedLongName);
	if ( iter != mEntities.end() )
		return iter->second;

	return VUNULL;
}

//*****************************************************************************
bool VuEntityRepository::isProjectLoaded(const char *strProjectName)
{
	return mProjects.find(strProjectName) != mProjects.end();
}

//*****************************************************************************
void VuEntityRepository::getEntityData(VuJsonContainer &data) const
{
	for ( const auto &entry : mEntities )
	{
		VuEntity *pEntity = entry.second;

		std::string strPartialName = pEntity->getLongName();
		VuJsonContainer *pSubData = &data;
		while ( strPartialName != pEntity->getShortName() )
		{
			std::string strPath = VuEntityUtil::getRoot(strPartialName);
			pSubData = &(*pSubData)[strPath];
			strPartialName = VuEntityUtil::subtractRoot(strPartialName);
		}
		(*pSubData)[strPartialName];
	}
}

//*****************************************************************************
void VuEntityRepository::tick(float fdt)
{
	mManagedEntities.remove_if(ManagedEntity::remove);

	if ( VuDevStat::IF() )
		updateDevStats(fdt);
}

//*****************************************************************************
void VuEntityRepository::updateDevStats(float fdt)
{
	// dev stats
	if ( VuDevStatPage *pPage = VuDevStat::IF()->getCurPage() )
	{
		if ( pPage->getName() == "EntityRepository" )
		{
			typedef std::map<const char *, int> Counts;
			Counts counts;
			for (ManagedEntities::const_iterator iter = mManagedEntities.begin(); iter != mManagedEntities.end(); iter++)
				counts[iter->mpEntity->getType()]++;

			pPage->clear();

			for (Counts::const_iterator iter = counts.begin(); iter != counts.end(); iter++)
				pPage->printf("%d %s\n", iter->second, iter->first);

			pPage->printf("\nTotal Entities: %d", mManagedEntities.size());
		}
	}
}

//*****************************************************************************
bool VuEntityRepository::ManagedEntity::remove(const ManagedEntity &managedEntity)
{
	if ( managedEntity.mRemove )
	{
		managedEntity.mpEntity->gameRelease();
		managedEntity.mpEntity->removeRef();
	}

	return managedEntity.mRemove;
}
