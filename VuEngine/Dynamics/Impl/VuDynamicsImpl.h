//*****************************************************************************
//
//  Copyright (c) 2008-2008 Ralf Knoesel
//  Copyright (c) 2008-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  Dynamics implementation
// 
//*****************************************************************************

#pragma once

#include "VuEngine/Dynamics/VuDynamics.h"

class VuDynamicsWorldImpl;
class VuDynamicsContactManagerImpl;
class VuDynamicsDebugDrawerImpl;
class btGhostPairCallback;


class VuDynamicsImpl : public VuDynamics, public VuDynamicsStepCallback
{
public:
	VuDynamicsImpl();
	~VuDynamicsImpl();

	virtual bool		init(bool asynchronous, int maxSubSteps, float fFixedTimeStep);
	virtual void		release();

	// external interface

	virtual void		setGravity(const VuVector3 &vGravity);
	virtual VuVector3	getGravity();

	virtual float		getLocalTime();

	virtual void		registerRigidBodyCallback(VuRigidBodyCallback *pCallback)	{ mRigidBodyCallbacks.push_back(pCallback); }
	virtual void		unregisterRigidBodyCallback(VuRigidBodyCallback *pCallback)	{ mRigidBodyCallbacks.remove(pCallback); }

	virtual void		registerStepCallback(VuDynamicsStepCallback *pCallback)		{ flush(); mStepCallbacks.push_back(pCallback); }
	virtual void		unregisterStepCallback(VuDynamicsStepCallback *pCallback)	{ flush(); mStepCallbacks.remove(pCallback); }

	virtual void		registerContactCallback(VuGlobalContactCallback *pCallback);
	virtual void		unregisterContactCallback(VuGlobalContactCallback *pCallback);

	virtual void		addRigidBody(VuRigidBody *pRigidBody);
	virtual void		removeRigidBody(VuRigidBody *pRigidBody);
	virtual void		addConstraint(btTypedConstraint *pConstraint, bool disableCollisionsBetweenLinkedBodies);
	virtual void		removeConstraint(btTypedConstraint *pConstraint);

	virtual const SurfaceTypes	&getSurfaceTypes();
	virtual int					getSurfaceTypeCount();
	virtual VUUINT8				getSurfaceTypeID(const char *strName);
	virtual float				getSurfaceFriction(VUUINT8 surfaceTypeID);
	virtual const VuColor		&getSurfaceColor(VUUINT8 surfaceTypeID);
	virtual const std::string	&getSurfaceTypeName(VUUINT8 surfaceTypeID);

	virtual bool		isPotentiallyBusy();
	virtual bool		isBusy();
	virtual void		flush();

	virtual void		drawCollision(const VuCamera &camera);

	virtual btDynamicsWorld	*getDynamicsWorld();

	// internal interface

	static VuDynamicsImpl *IF() { return static_cast<VuDynamicsImpl *>(VuDynamics::IF()); }

	virtual VuDynamicsContactManagerImpl	*getContactManager()	{ return mpContactManager; }

private:
	typedef std::list<VuRigidBodyCallback *> RigidBodyCallbacks;
	typedef std::list<VuDynamicsStepCallback *> StepCallbacks;

	static void			threadProc(void *pParam) { static_cast<VuDynamicsImpl *>(pParam)->threadProc(); }
	void				threadProc();

	void				tickDynamicsSync(float fdt);
	void				tickDynamicsKick(float fdt);
	void				updateDevStats(float fdt);

	void				draw();

	// VuDynamicsWorldImpl::Callback
	virtual void		onDynamicsAdvanceEnvironment(float fdt, bool bSimStep);
	virtual void		onDynamicsApplyForces(float fdt);

	void				profileRecursive(CProfileIterator *iter, int level);


	// synchronization
	VUHANDLE			mhThread;
	bool				mbAsynchronousDynamics;
	bool				mbWorkerThreadActive;
	bool				mbTerminateThread;
	bool				mbAsyncPhaseActive;
	float				mDynamicsStepTime;
	float				mDynamicsOverlapTime;
	VUHANDLE			mWorkAvailableEvent;
	VUHANDLE			mWorkCompletedEvent;

	btDefaultCollisionConfiguration	*mpCollisionConfiguration;
	btCollisionDispatcher			*mpDispatcher;
	btBroadphaseInterface			*mpBroadphase;
	btConstraintSolver				*mpConstraintSolver;
	btGhostPairCallback				*mpGhostPairCallback;
	VuDynamicsWorldImpl				*mpDynamicsWorld;
	VuDynamicsContactManagerImpl	*mpContactManager;
	VuDynamicsDebugDrawerImpl		*mpDebugDrawer;

	RigidBodyCallbacks				mRigidBodyCallbacks;
	StepCallbacks					mStepCallbacks;

	int								mMaxSubSteps;
	float							mFixedTimeStep;
};
