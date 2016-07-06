//*****************************************************************************
//
//  Copyright (c) 2008-2008 Ralf Knoesel
//  Copyright (c) 2008-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  DynamicsContactManager class
// 
//*****************************************************************************

#pragma once

#include "VuEngine/Dynamics/VuDynamics.h"
#include "VuEngine/Containers/VuList.h"
#include "VuEngine/Util/VuColor.h"


class VuDynamicsContactManagerImpl
{
public:
	VuDynamicsContactManagerImpl(btCollisionWorld *pCollisionWorld);
	~VuDynamicsContactManagerImpl();

	void				registerContactCallback(VuGlobalContactCallback *pCallback)		{ mContactCallbacks.push_back(pCallback); }
	void				unregisterContactCallback(VuGlobalContactCallback *pCallback)	{ mContactCallbacks.remove(pCallback); }

	bool				contactAdded(btManifoldPoint &cp, const btCollisionObject *colObj0, int partId0, int index0, const btCollisionObject *colObj1, int partId1, int index1);

	typedef VuDynamics::SurfaceTypes SurfaceNames;
	const SurfaceNames	&getSurfaceNames() const { return mSurfaceNames; }
	int					getSurfaceTypeCount() const;
	VUUINT8				getSurfaceTypeID(const char *strName) const;

	// surface types
	struct SurfaceType
	{
		SurfaceType() : mFriction(0), mColor(255,255,255), mHashedName(0) {}
		float		mFriction;
		VuColor		mColor;
		std::string	mName;
		VUUINT32	mHashedName;
	};
	const SurfaceType	&getSurfaceType(VUUINT8 surfaceTypeID) const { return mSurfaceTypes[surfaceTypeID]; }

private:
	// contacts
	typedef std::list<VuGlobalContactCallback *> ContactCallbacks;

	ContactCallbacks			mContactCallbacks;

	typedef std::vector<SurfaceType> SurfaceTypes;

	SurfaceTypes		mSurfaceTypes;
	SurfaceNames		mSurfaceNames;
};
