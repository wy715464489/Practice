//*****************************************************************************
//
//  Copyright (c) 2008-2008 Ralf Knoesel
//  Copyright (c) 2008-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  Dynamics implementation
// 
//*****************************************************************************

#include "BulletCollision/CollisionDispatch/btGhostObject.h"
#include "VuDynamicsImpl.h"
#include "VuDynamicsWorldImpl.h"
#include "VuDynamicsContactManagerImpl.H"
#include "VuDynamicsDebugDrawerImpl.h"
#include "VuDynamicsDrawCollisionImpl.h"
#include "VuEngine/Managers/VuTickManager.h"
#include "VuEngine/Managers/VuDrawManager.h"
#include "VuEngine/Method/VuMethodUtil.h"
#include "VuEngine/HAL/Thread/VuThread.h"
#include "VuEngine/Dev/VuDevStat.h"
#include "VuEngine/Dev/VuDevProfile.h"


// the interface
IMPLEMENT_SYSTEM_COMPONENT(VuDynamics, VuDynamicsImpl);


//*****************************************************************************
VuDynamicsImpl::VuDynamicsImpl():
	mhThread(VUNULL),
	mbAsynchronousDynamics(false),
	mbWorkerThreadActive(false),
	mbTerminateThread(false),
	mbAsyncPhaseActive(false),
	mDynamicsStepTime(0.0f),
	mpCollisionConfiguration(VUNULL),
	mpDispatcher(VUNULL),
	mpBroadphase(VUNULL),
	mpConstraintSolver(VUNULL),
	mpDynamicsWorld(VUNULL)
{
	// set up dev menu/stats
	if ( VuDevStat::IF() )
	{
		VuDevStat::IF()->addPage("DynamicsInfo", VuRect(50, 10, 40, 80));
		VuDevStat::IF()->addPage("DynamicsProfile", VuRect(5, 5, 90, 90));

		// register phased tick
		VuTickManager::IF()->registerHandler(this, &VuDynamicsImpl::updateDevStats, "Final");
	}

	mWorkAvailableEvent = VuThread::IF()->createEvent();
	mWorkCompletedEvent = VuThread::IF()->createEvent();
}

//*****************************************************************************
VuDynamicsImpl::~VuDynamicsImpl()
{
	// unregister phased tick
	VuTickManager::IF()->unregisterHandlers(this);

	VuThread::IF()->destroyEvent(mWorkAvailableEvent);
	VuThread::IF()->destroyEvent(mWorkCompletedEvent);
}

//*****************************************************************************
bool VuDynamicsImpl::init(bool asynchronous, int maxSubSteps, float fFixedTimeStep)
{
	mbAsynchronousDynamics = asynchronous;

	// create world
	mpCollisionConfiguration = new btDefaultCollisionConfiguration;
	mpDispatcher = new btCollisionDispatcher(mpCollisionConfiguration);
	mpBroadphase = new btDbvtBroadphase;
	mpConstraintSolver = new btSequentialImpulseConstraintSolver;
	mpGhostPairCallback = new btGhostPairCallback;
	mpDynamicsWorld = new VuDynamicsWorldImpl(mpDispatcher, mpBroadphase, mpConstraintSolver, mpCollisionConfiguration);

	// use split impulse
	mpDynamicsWorld->getSolverInfo().m_splitImpulse = true;

	// register ghost pair callback
	mpBroadphase->getOverlappingPairCache()->setInternalGhostPairCallback(mpGhostPairCallback);

	// set up dynamics world callback
	mpDynamicsWorld->setCallback(this);

	// set up contact manager
	mpContactManager = new VuDynamicsContactManagerImpl(mpDynamicsWorld);

	// set up debug drawer
	mpDebugDrawer = new VuDynamicsDebugDrawerImpl;
	mpDynamicsWorld->setDebugDrawer(mpDebugDrawer);

	// register phased tick
	VuTickManager::IF()->registerHandler(this, &VuDynamicsImpl::tickDynamicsSync, "DynamicsSync");
	VuTickManager::IF()->registerHandler(this, &VuDynamicsImpl::tickDynamicsKick, "DynamicsKick");

	// register draw
	VuDrawManager::IF()->registerHandler(this, &VuDynamicsImpl::draw);

	mMaxSubSteps = maxSubSteps;
	mFixedTimeStep = fFixedTimeStep;

	mhThread = VuThread::IF()->createThread(threadProc, this);

	return true;
}

//*****************************************************************************
void VuDynamicsImpl::release()
{
	flush();

	VUPRINTF("Terminating VuDynamicsImpl thread...\n");
	mbTerminateThread = true;
	VuThread::IF()->setEvent(mWorkAvailableEvent);
	VuThread::IF()->joinThread(mhThread);

	VUASSERT(mpDynamicsWorld != VUNULL, "VuDynamicsImpl::createWorld() not created");

	// unregister phased tick
	VuTickManager::IF()->unregisterHandlers(this);

	// unregister draw
	VuDrawManager::IF()->unregisterHandler(this);

	delete mpDebugDrawer;				mpDebugDrawer = VUNULL;
	delete mpContactManager;			mpContactManager = VUNULL;
	delete mpDynamicsWorld;				mpDynamicsWorld = VUNULL;
	delete mpGhostPairCallback;			mpGhostPairCallback = VUNULL;
	delete mpConstraintSolver;			mpConstraintSolver = VUNULL;
	delete mpBroadphase;				mpBroadphase = VUNULL;
	delete mpDispatcher;				mpDispatcher = VUNULL;
	delete mpCollisionConfiguration;	mpCollisionConfiguration = VUNULL;
}

//*****************************************************************************
void VuDynamicsImpl::setGravity(const VuVector3 &vGravity)
{
	mpDynamicsWorld->setGravity(VuDynamicsUtil::toBtVector3(vGravity));
}

//*****************************************************************************
VuVector3 VuDynamicsImpl::getGravity()
{
	return VuDynamicsUtil::toVuVector3(mpDynamicsWorld->getGravity());
}

//*****************************************************************************
float VuDynamicsImpl::getLocalTime()
{
	return mpDynamicsWorld->getLocalTime();
}

//*****************************************************************************
void VuDynamicsImpl::registerContactCallback(VuGlobalContactCallback *pCallback)
{
	mpContactManager->registerContactCallback(pCallback);
}

//*****************************************************************************
void VuDynamicsImpl::unregisterContactCallback(VuGlobalContactCallback *pCallback)
{
	mpContactManager->unregisterContactCallback(pCallback);
}

//*****************************************************************************
void VuDynamicsImpl::addRigidBody(VuRigidBody *pRigidBody)
{
	flush();

	// enables contact point callbacks
	pRigidBody->setCollisionFlags(pRigidBody->getCollisionFlags() | btCollisionObject::CF_CUSTOM_MATERIAL_CALLBACK);

	mpDynamicsWorld->addRigidBody(pRigidBody, pRigidBody->getCollisionGroup(), pRigidBody->getCollisionMask());

	for ( RigidBodyCallbacks::iterator iter = mRigidBodyCallbacks.begin(); iter != mRigidBodyCallbacks.end(); iter++ )
		(*iter)->onRigidBodyAdded(pRigidBody);
}

//*****************************************************************************
void VuDynamicsImpl::removeRigidBody(VuRigidBody *pRigidBody)
{
	flush();

	// activate contacting bodies
	int numManifolds = mpDynamicsWorld->getDispatcher()->getNumManifolds();
	for ( int i = 0; i < numManifolds; i++ )
	{
		btPersistentManifold *contactManifold = mpDynamicsWorld->getDispatcher()->getManifoldByIndexInternal(i);
		const btCollisionObject *pObjA = static_cast<const btCollisionObject*>(contactManifold->getBody0());
		const btCollisionObject *pObjB = static_cast<const btCollisionObject*>(contactManifold->getBody1());
		if ( pObjA == pRigidBody )
			pObjB->activate(true);
		if ( pObjB == pRigidBody )
			pObjA->activate(true);
	}

	mpDynamicsWorld->removeRigidBody(pRigidBody);

	for ( RigidBodyCallbacks::iterator iter = mRigidBodyCallbacks.begin(); iter != mRigidBodyCallbacks.end(); iter++ )
		(*iter)->onRigidBodyRemoved(pRigidBody);
}

//*****************************************************************************
void VuDynamicsImpl::addConstraint(btTypedConstraint *pConstraint, bool disableCollisionsBetweenLinkedBodies)
{
	flush();

	mpDynamicsWorld->addConstraint(pConstraint, disableCollisionsBetweenLinkedBodies);
}

//*****************************************************************************
void VuDynamicsImpl::removeConstraint(btTypedConstraint *pConstraint)
{
	flush();

	mpDynamicsWorld->removeConstraint(pConstraint);
}

//*****************************************************************************
const VuDynamics::SurfaceTypes &VuDynamicsImpl::getSurfaceTypes()
{
	return mpContactManager->getSurfaceNames();
}

//*****************************************************************************
int VuDynamicsImpl::getSurfaceTypeCount()
{
	return mpContactManager->getSurfaceTypeCount();
}

//*****************************************************************************
VUUINT8 VuDynamicsImpl::getSurfaceTypeID(const char *strName)
{
	return mpContactManager->getSurfaceTypeID(strName);
}

//*****************************************************************************
float VuDynamicsImpl::getSurfaceFriction(VUUINT8 surfaceTypeID)
{
	return mpContactManager->getSurfaceType(surfaceTypeID).mFriction;
}

//*****************************************************************************
const VuColor &VuDynamicsImpl::getSurfaceColor(VUUINT8 surfaceTypeID)
{
	return mpContactManager->getSurfaceType(surfaceTypeID).mColor;
}

//*****************************************************************************
const std::string &VuDynamicsImpl::getSurfaceTypeName(VUUINT8 surfaceTypeID)
{
	return mpContactManager->getSurfaceType(surfaceTypeID).mName;
}

//*****************************************************************************
bool VuDynamicsImpl::isPotentiallyBusy()
{
	return mbAsyncPhaseActive;
}

//*****************************************************************************
bool VuDynamicsImpl::isBusy()
{
	return mbWorkerThreadActive;
}

//*****************************************************************************
void VuDynamicsImpl::flush()
{
	if ( mbWorkerThreadActive )
	{
		VuThread::IF()->waitForSingleObject(mWorkCompletedEvent, 0xffffffff);
		mbWorkerThreadActive = false;
	}
}

//*****************************************************************************
void VuDynamicsImpl::drawCollision(const VuCamera &camera)
{
	flush();
	VuDynamicsDrawCollisionImpl::drawCollision(camera);
}

//*****************************************************************************
btDynamicsWorld *VuDynamicsImpl::getDynamicsWorld()
{
	return mpDynamicsWorld;
}

//*****************************************************************************
void VuDynamicsImpl::threadProc()
{
	VuThread::IF()->setThreadProcessor(4);

	VUPRINTF("VuDynamics thread starting...\n");

	for (;;)
	{
		if ( !VuThread::IF()->waitForSingleObject(mWorkAvailableEvent) )
		{
			VUPRINTF("VuDynamicsImpl::threadProc() wait error!\n");
			break;
		}
		if ( mbTerminateThread )
		{
			VUPRINTF("VuDynamicsImpl thread exiting...\n");
			break;
		}

		// do work
		{
			if ( VuDevProfile::IF() )
				VuDevProfile::IF()->beginDyn(mDynamicsOverlapTime);

			if ( mDynamicsStepTime > 0 )
				mpDynamicsWorld->stepSimulation(mDynamicsStepTime, mMaxSubSteps, mFixedTimeStep);

			if ( VuDevProfile::IF() )
				VuDevProfile::IF()->endDyn();
		}

		VuThread::IF()->setEvent(mWorkCompletedEvent);
	}

	VuThread::IF()->endThread();
}

//*****************************************************************************
void VuDynamicsImpl::tickDynamicsSync(float fdt)
{
	flush();

	mDynamicsOverlapTime = (float)VuSys::IF()->getTime();

	// synchronize other systems
	if ( VuDevProfile::IF() )
		VuDevProfile::IF()->synchronizeDyn();

	if ( fdt > 0 )
	{
		// synchronize motion states to interpolated time
		mpDynamicsWorld->synchronizeMotionStates();

		mpDebugDrawer->update();
	}

	mbAsyncPhaseActive = false;
}

//*****************************************************************************
void VuDynamicsImpl::tickDynamicsKick(float fdt)
{
	mbAsyncPhaseActive = true;

	// calculate time spent overlapping w/ main thread
	mDynamicsOverlapTime = (float)VuSys::IF()->getTime() - mDynamicsOverlapTime;

	// kick off next frame
	mbWorkerThreadActive = true;
	mDynamicsStepTime = fdt;
	VuThread::IF()->setEvent(mWorkAvailableEvent);

	// if processing synchronously, flush now
	if ( !mbAsynchronousDynamics )
		flush();
}

//*****************************************************************************
void VuDynamicsImpl::updateDevStats(float fdt)
{
	// dev stats
	if ( VuDevStatPage *pPage = VuDevStat::IF()->getCurPage() )
	{
		if ( pPage->getName() == "DynamicsInfo" )
		{
			pPage->clear();
		}
		if ( pPage->getName() == "DynamicsProfile" )
		{
			pPage->clear();
			CProfileIterator *iter = CProfileManager::Get_Iterator();
			profileRecursive(iter, 0);
			CProfileManager::Release_Iterator(iter);
		}
	}
}

//*****************************************************************************
void VuDynamicsImpl::draw()
{
	if ( mpDebugDrawer->getDebugMode() )
	{
		flush();
		mpDebugDrawer->enable(true);
		mpDynamicsWorld->debugDrawWorld();
		mpDebugDrawer->enable(false);
	}
}

//*****************************************************************************
void VuDynamicsImpl::onDynamicsAdvanceEnvironment(float fdt, bool bSimStep)
{
	VU_PROFILE_DYN("AdvanceEnvironment");

	for ( StepCallbacks::iterator iter = mStepCallbacks.begin(); iter != mStepCallbacks.end(); iter++ )
		(*iter)->onDynamicsAdvanceEnvironment(fdt, bSimStep);
}

//*****************************************************************************
void VuDynamicsImpl::onDynamicsApplyForces(float fdt)
{
	VU_PROFILE_DYN("ApplyForces");

	for ( StepCallbacks::iterator iter = mStepCallbacks.begin(); iter != mStepCallbacks.end(); iter++ )
		(*iter)->onDynamicsApplyForces(fdt);
}

//*****************************************************************************
void VuDynamicsImpl::profileRecursive(CProfileIterator *iter, int level)
{
	iter->First();
	if ( iter->Is_Done() )
		return;

	VuDevStatPage *pPage = VuDevStat::IF()->getCurPage();

	float accumulated_time = 0;
	float parent_time = iter->Is_Root() ? CProfileManager::Get_Time_Since_Reset() : iter->Get_Current_Parent_Total_Time();
	int frames_since_reset = CProfileManager::Get_Frame_Count_Since_Reset();
	for (int i = 0; i < level; i++)	pPage->printf(".");
		pPage->printf("----------------------------------\n");
	for (int i = 0; i < level; i++)	pPage->printf(".");
		pPage->printf("Profiling: %s (total running time: %.3f ms) ---\n",	iter->Get_Current_Parent_Name(), parent_time );
	float totalTime = 0.f;

	int numChildren = 0;
	
	for ( int i = 0; !iter->Is_Done(); i++, iter->Next())
	{
		numChildren++;
		float current_total_time = iter->Get_Current_Total_Time();
		accumulated_time += current_total_time;
		float fraction = parent_time > SIMD_EPSILON ? (current_total_time / parent_time) * 100 : 0.f;
		{
			for (int j = 0; j < level; j++)	pPage->printf(".");
		}
		pPage->printf("%d -- %s (%.2f %%) :: %.3f ms / frame (%d calls)\n",i, iter->Get_Current_Name(), fraction,(current_total_time / (double)frames_since_reset),iter->Get_Current_Total_Calls());
		totalTime += current_total_time;
		//recurse into children
	}

	if (parent_time < accumulated_time)
	{
		pPage->printf("what's wrong\n");
	}
	for (int i = 0; i < level; i++)	pPage->printf(".");
	pPage->printf("%s (%.3f %%) :: %.3f ms\n", "Unaccounted:",parent_time > SIMD_EPSILON ? ((parent_time - accumulated_time) / parent_time) * 100 : 0.f, parent_time - accumulated_time);
	
	for (int i = 0; i < numChildren; i++)
	{
		iter->Enter_Child(i);
		profileRecursive(iter, level + 3);
		iter->Enter_Parent();
	}
}