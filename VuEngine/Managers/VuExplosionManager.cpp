//*****************************************************************************
//
//  Copyright (c) 2009-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  Explosion manager
// 
//*****************************************************************************

#include "VuExplosionManager.h"
#include "VuEngine/Entities/VuEntity.h"
#include "VuEngine/Assets/VuAssetFactory.h"
#include "VuEngine/Assets/VuDBAsset.h"
#include "VuEngine/Pfx/VuPfx.h"
#include "VuEngine/Pfx/VuPfxManager.h"
#include "VuEngine/Pfx/VuPfxEntity.h"
#include "VuEngine/Dynamics/VuDynamics.h"
#include "VuEngine/Math/VuMathUtil.h"
#include "VuEngine/Math/VuUnitConversion.h"
#include "VuEngine/Util/VuAudioUtil.h"


// the interface
IMPLEMENT_SYSTEM_COMPONENT(VuExplosionManager, VuExplosionManager);


//*****************************************************************************
bool VuExplosionManager::init()
{
	mpExplosionDB = VuAssetFactory::IF()->createAsset<VuDBAsset>("ExplosionDB", VuAssetFactory::OPTIONAL_ASSET);

	return true;
}

//*****************************************************************************
void VuExplosionManager::release()
{
	VuAssetFactory::IF()->releaseAsset(mpExplosionDB);
}

//*****************************************************************************
void VuExplosionManager::createExplosion(const VuVector3 &pos, const char *type, VuEntity *pOriginator)
{
	if ( mpExplosionDB )
	{
		const VuJsonContainer &data = mpExplosionDB->getDB()[type];
		if ( data.isObject() )
		{
			createExplosion(pos, data, pOriginator);
		}
	}
}

//*****************************************************************************
void VuExplosionManager::createExplosion(const VuVector3 &pos, const VuJsonContainer &data, VuEntity *pOriginator)
{
	struct VuCallback : btBroadphaseAabbCallback
	{
		virtual bool process(const btBroadphaseProxy* proxy)
		{
			btCollisionObject *pColObj = static_cast<btCollisionObject *>(proxy->m_clientObject);
			if ( pColObj->getInternalType() == btCollisionObject::CO_RIGID_BODY )
			{
				VuRigidBody *pRigidBody = static_cast<VuRigidBody *>(pColObj);
				if ( pRigidBody->getExtendedFlags() & (EXT_COL_ENGINE_DETECT_EXPLOSIONS|EXT_COL_ENGINE_REACT_TO_EXPLOSIONS) )
				{
					// get aabb
					btTransform ident;
					ident.setIdentity();
					btVector3 aabbMin, aabbMax;
					pColObj->getCollisionShape()->getAabb(ident, aabbMin, aabbMax);

					// cal distance from explosion origin to box
					VuAabb aabb;
					aabb.mMin = VuDynamicsUtil::toVuVector3(aabbMin);
					aabb.mMax = VuDynamicsUtil::toVuVector3(aabbMax);

					VuMatrix transform = VuDynamicsUtil::toVuMatrix(pColObj->getWorldTransform());

					VuVector3 closestPoint;
					float dist = VuMathUtil::distPointBox(mPos, aabb, transform,closestPoint);

					if ( dist < mRadius )
					{
						if ( pRigidBody->getExtendedFlags() & EXT_COL_ENGINE_DETECT_EXPLOSIONS )
						{
							// send message
							VuParams params;
							params.addVector3(mPos);
							params.addFloat(dist);
							params.addPointer(mpData);
							params.addEntity(mpOriginator);
							pRigidBody->getEntity()->handleEvent("OnExplosion", params);
						}

						if ( (pRigidBody->getExtendedFlags() & EXT_COL_ENGINE_REACT_TO_EXPLOSIONS) && (pRigidBody->getInvMass() > FLT_EPSILON) )
						{
							// calculate fun impulse
							const VuJsonContainer &impulseData = (*mpData)["Impulse"];
							if ( impulseData.isObject() )
							{
								float distRatio = 1.0f - VuLinStep(impulseData["InnerRadius"].asFloat(), impulseData["OuterRadius"].asFloat(), dist);
								float massRatio = VuLinStep(impulseData["MinMass"].asFloat(), impulseData["MaxMass"].asFloat(), 1.0f/pRigidBody->getInvMass());
								float impulse = VuLerp(impulseData["MinMassImpulse"].asFloat(), impulseData["MaxMassImpulse"].asFloat(), massRatio);
								impulse = VuMphToMetersPerSecond(impulse);
								impulse *= distRatio;

								VuVector3 linVel = pRigidBody->getVuCenterOfMassPosition() - mPos;
								linVel.mZ = VuMax(VuAbs(linVel.mZ), linVel.mag2d());
								linVel.normalize();

								linVel *= impulse;

								linVel += pRigidBody->getVuLinearVelocity();
								pRigidBody->setVuLinearVelocity(linVel);

								pRigidBody->activate();
							}
						}
					}
				}
			}

			return true;
		}
		VuVector3				mPos;
		float					mRadius;
		const VuJsonContainer	*mpData;
		VuEntity				*mpOriginator;
	};

	float radius = data["OuterRadius"].asFloat();

	btVector3 aabbMin(pos.mX - radius, pos.mY - radius, pos.mZ - radius);
	btVector3 aabbMax(pos.mX + radius, pos.mY + radius, pos.mZ + radius);

	VuCallback callback;
	callback.mPos = pos;
	callback.mRadius = radius;
	callback.mpData = &data;
	callback.mpOriginator = pOriginator;
	VuDynamics::IF()->getDynamicsWorld()->getBroadphase()->aabbTest(aabbMin, aabbMax, callback);

	// pfx
	if ( VUUINT32 hPfx = VuPfxManager::IF()->createEntity(data["PfxSystem"].asCString(), true) )
	{
		if ( VuPfxEntity *pPfxEntity = VuPfxManager::IF()->getEntity(hPfx) )
		{
			pPfxEntity->getSystemInstance()->setPosition(pos);
			pPfxEntity->getSystemInstance()->start();
		}
	}

	// sfx
	VuAudioUtil::playSfx(data["AudioEvent"].asCString(), pos);
}
