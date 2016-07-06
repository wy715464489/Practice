//*****************************************************************************
//
//  Copyright (c) 2007-2008 Ralf Knoesel
//  Copyright (c) 2008-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  EntityRepository class
// 
//*****************************************************************************

#pragma once

#include "VuEngine/VuSystemComponent.h"
#include "VuEngine/Method/VuMethod.h"


class VuEntity;
class VuProject;
class VuJsonContainer;


class VuEntityRepository : public VuSystemComponent
{
	DECLARE_SYSTEM_COMPONENT(VuEntityRepository)

public:
	VuEntityRepository();
	~VuEntityRepository();

	virtual bool	init();
	virtual void	preRelease();
	virtual void	release();

	void			addProject(VuProject *pProject);
	void			removeProject(VuProject *pProject);

	void			addEntity(VuEntity *pEntity);
	void			removeEntity(VuEntity *pEntity);

	void			addManagedEntity(VuEntity *pEntity);
	void			removeManagedEntity(VuEntity *pEntity);
	void			resetManagedEntities();

	VuEntity		*findEntity(const char *strEntityLongName);
	VuEntity		*findEntity(VUUINT32 hashedLongName);
	bool			isProjectLoaded(const char *strProjectName);

	// these are slow methods... not to be used frequently
	void			getEntityData(VuJsonContainer &data) const;

private:
	void			tick(float fdt);
	void			updateDevStats(float fdt);

	typedef std::set<std::string> Projects;
	Projects		mProjects;

	typedef std::hash_map<VUUINT32, VuEntity *> Entities;
	Entities		mEntities;

	struct ManagedEntity
	{
		ManagedEntity(VuEntity *pEntity) : mpEntity(pEntity), mRemove(false) {}
		static bool	remove(const ManagedEntity &managedEntity);
		VuEntity	*mpEntity;
		bool		mRemove;
	};
	typedef std::list<ManagedEntity> ManagedEntities;
	ManagedEntities	mManagedEntities;
};
