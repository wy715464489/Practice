//*****************************************************************************
//
//  Copyright (c) 2008-2008 Ralf Knoesel
//  Copyright (c) 2008-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  DynamicsContactManager class
// 
//*****************************************************************************

#include "VuDynamicsContactManagerImpl.h"
#include "VuDynamicsImpl.h"
#include "VuEngine/Assets/VuAssetFactory.h"
#include "VuEngine/Assets/VuDBAsset.h"
#include "VuEngine/Assets/VuCollisionMeshAsset.h"
#include "VuEngine/Util/VuDataUtil.h"
#include "VuEngine/Dev/VuDevUtil.h"


//*****************************************************************************
bool ContactAddedWrapper(btManifoldPoint &cp, const btCollisionObjectWrapper *colObj0Wrap, int partId0, int index0, const btCollisionObjectWrapper *colObj1Wrap, int partId1, int index1)
{
	return VuDynamicsImpl::IF()->getContactManager()->contactAdded(cp, colObj0Wrap->m_collisionObject, partId0, index0, colObj1Wrap->m_collisionObject, partId1, index1);
}

//*****************************************************************************
VuDynamicsContactManagerImpl::VuDynamicsContactManagerImpl(btCollisionWorld *pCollisionWorld)
{
	// hook into Bullet callbacks
	gContactAddedCallback = ContactAddedWrapper;

	// load surface DB
	if ( VuAssetFactory::IF()->doesAssetExist<VuDBAsset>("SurfaceDB") )
	{
		VuDBAsset *pSurfaceDBAsset = VuAssetFactory::IF()->createAsset<VuDBAsset>("SurfaceDB");

		const VuJsonContainer &db = pSurfaceDBAsset->getDB();
		mSurfaceTypes.resize(db.size());

		// surface types
		for ( int i = 0; i < db.size(); i++ )
		{
			const VuJsonContainer &entry = db[i];

			// add entry
			VuDataUtil::getValue(entry["Friction"], mSurfaceTypes[i].mFriction);
			VuDataUtil::getValue(entry["Color"], mSurfaceTypes[i].mColor);
			VuDataUtil::getValue(entry["Name"], mSurfaceTypes[i].mName);
			mSurfaceTypes[i].mHashedName = VuHash::fnv32String(mSurfaceTypes[i].mName.c_str());

			mSurfaceNames.push_back(entry["Name"].asString());
		}

		VuAssetFactory::IF()->releaseAsset(pSurfaceDBAsset);
	}

	// make sure there is at least a default surface type
	mSurfaceTypes.resize(VuMax((VUUINT)mSurfaceTypes.size(), 1u));
}

//*****************************************************************************
VuDynamicsContactManagerImpl::~VuDynamicsContactManagerImpl()
{
	gContactAddedCallback = VUNULL;
}

//*****************************************************************************
bool VuDynamicsContactManagerImpl::contactAdded(btManifoldPoint &cp, const btCollisionObject *colObj0, int partId0, int index0, const btCollisionObject *colObj1, int partId1, int index1)
{
	// non-rigid bodies not supported yet
	if ( colObj0->getInternalType() != btCollisionObject::CO_RIGID_BODY )
		return true;
	if ( colObj1->getInternalType() != btCollisionObject::CO_RIGID_BODY )
		return true;

	bool respond = true;

	VUASSERT(colObj0->getCollisionShape()->getUserPointer() == VUNULL, "VuDynamicsContactManagerImpl::contactAdded() bad assumption");

	// translate contact point
	VuContactPoint akcp;
	akcp.mpBody0 = static_cast<const VuRigidBody *>(colObj0);
	akcp.mpBody1 = static_cast<const VuRigidBody *>(colObj1);
	akcp.mSurfaceType0 = akcp.mpBody0->getSurfaceType();
	akcp.mSurfaceType1 = akcp.mpBody1->getSurfaceType();
	if ( VuCollisionMeshAsset *pAsset = static_cast<VuCollisionMeshAsset *>(colObj1->getCollisionShape()->getUserPointer()) )
	{
		const VuCollisionMeshAsset::VuMaterial &material = pAsset->getTriangleMaterial(index1);
		akcp.mSurfaceType1 = material.mSurfaceTypeID;
		pAsset->adjustInternalEdgeContacts(cp, colObj1, index1);
	}
	akcp.mpOtherBody = VUNULL;
	akcp.mPosWorld = VuDynamicsUtil::toVuVector3(0.5f*(cp.m_positionWorldOnA + cp.m_positionWorldOnB));
	akcp.mNorWorld = VuDynamicsUtil::toVuVector3(cp.m_normalWorldOnB);

	// determine combined friction
	akcp.mCombinedFriction = getSurfaceType(akcp.mSurfaceType0).mFriction*getSurfaceType(akcp.mSurfaceType1).mFriction;

	// contact added callback
	for ( ContactCallbacks::iterator iter = mContactCallbacks.begin(); iter != mContactCallbacks.end(); iter++ )
		(*iter)->onGlobalContactAdded(akcp);

	if ( akcp.mpBody0->getContactCallback() )
	{
		akcp.mpOtherBody = akcp.mpBody1;
		respond &= akcp.mpBody0->getContactCallback()->onRigidBodyContactAdded(akcp);
	}

	// flip normal for body 1 callback
	akcp.mNorWorld = -akcp.mNorWorld;

	if ( akcp.mpBody1->getContactCallback() )
	{
		akcp.mpOtherBody = akcp.mpBody0;
		respond &= akcp.mpBody1->getContactCallback()->onRigidBodyContactAdded(akcp);
	}

	// update combined friction
	cp.m_combinedFriction = akcp.mCombinedFriction;

	return respond;
}

//*****************************************************************************
int VuDynamicsContactManagerImpl::getSurfaceTypeCount() const
{
	return (int)mSurfaceTypes.size();
}

//*****************************************************************************
VUUINT8 VuDynamicsContactManagerImpl::getSurfaceTypeID(const char *strName) const
{
	VUUINT32 hashedName = VuHash::fnv32String(strName);

	VUUINT8 type = 0;
	for ( SurfaceTypes::const_iterator iter = mSurfaceTypes.begin(); iter != mSurfaceTypes.end(); iter++ )
	{
		if ( iter->mHashedName == hashedName )
			return type;
		type++;
	}

	return 0;
}
