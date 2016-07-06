//*****************************************************************************
//
//  Copyright (c) 2007-2008 Ralf Knoesel
//  Copyright (c) 2008-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  TickManager class
// 
//*****************************************************************************

#include "VuTickManager.h"
#include "VuEngine/Containers/VuArray.h"
#include "VuEngine/HAL/Keyboard/VuKeyboard.h"
#include "VuEngine/Method/VuMethodUtil.h"
#include "VuEngine/Dev/VuDevMenu.h"
#include "VuEngine/Dev/VuDevStat.h"
#include "VuEngine/Dev/VuDevProfile.h"


#define DELTATIME_HISTORY_SIZE 10

// typedefs
typedef VuMethodInterface1<void, float> Handler;

// tick phase
class VuTickPhase
{
public:
	typedef std::list<Handler *> Handlers;
	typedef std::vector<void *> Objects;

	VuTickPhase(const char *strName, bool useLastDeltaTime) : mstrName(strName), mUseLastDeltaTime(useLastDeltaTime), mbEnabled(true), mpCurObj(VUNULL) {}

	void		add(Handler *pHandler)	{ mHandlers.push_back(pHandler); }
	void		remove(void *pObj)
	{
		if ( pObj == mpCurObj )
			mPendingRemovals.push_back(pObj);
		else
			mHandlers.remove_if(isMethodOfObjectWithDelete(pObj));
	}
	void		tick(float fdt, float fdtLast);

	std::string	mstrName;
	Handlers	mHandlers;
	bool		mUseLastDeltaTime;
	bool		mbEnabled;
	Objects		mPendingRemovals;
	void		*mpCurObj;
};

// internal data
class VuTickManagerImpl : public VuTickManager, public VuKeyboard::Callback
{
public:
	VuTickManagerImpl();

	virtual bool	init();
	virtual void	postInit();
	virtual void	preRelease();
	virtual void	tick();

	virtual void	onKeyDown(VUUINT32 key);

	virtual void	setMaxClockDelta(float fdt);
	virtual void	registerHandler(Handler *pHandler, const char *strPhase);
	virtual void	unregisterHandler(void *pObj, const char *strPhase);
	virtual void	unregisterHandlers(void *pObj);

	virtual void	pushPauseRequest()	{ mPauseRequestCount++; }
	virtual void	popPauseRequest()	{ mPauseRequestCount--; VUASSERT(mPauseRequestCount >= 0, "VuTickManagerImpl::popPauseRequest() push/pop mismatch"); }
	virtual void	resetPauseRequests(){ mPauseRequestCount = 0; }
	virtual bool	isPaused()			{ return mbPaused || mPauseRequestCount > 0; }

	virtual float	getRealDeltaTime()			{ return mRealDeltaTime; }
	virtual float	getUnclampedRealDeltaTime()	{ return mUnclampedRealDeltaTime; }

	virtual double	getGameTime() { return mGameTime; }


private:
	typedef std::list<VuTickPhase> Phases;

	void		addPhase(const char *strPhase, bool useLastDeltaTime);
	VuTickPhase	*getPhase(const char *strPhase);

	float		mMaxClockDelta;
	Phases		mPhases;

	double		mLastTime;
	bool		mbPaused;
	bool		mbSlowed;
	bool		mbStep;
	int			mPauseRequestCount;
	float		mRealDeltaTime;
	float		mUnclampedRealDeltaTime;
	float		mLastRealDeltaTime;
	double		mGameTime;
};

// the interface
IMPLEMENT_SYSTEM_COMPONENT(VuTickManager, VuTickManagerImpl);


//*****************************************************************************
VuTickManagerImpl::VuTickManagerImpl():
	mMaxClockDelta(FLT_MAX),
	mLastTime(0.0),
	mbPaused(false),
	mbSlowed(false),
	mbStep(false),
	mPauseRequestCount(0),
	mRealDeltaTime(0),
	mUnclampedRealDeltaTime(0),
	mLastRealDeltaTime(0),
	mGameTime(0)
{
}

//*****************************************************************************
bool VuTickManagerImpl::init()
{
	// add standard phases
	addPhase("DynamicsSync", false);
	addPhase("Network", true);
	addPhase("Input", true);
	addPhase("Services", true);
	addPhase("PreDecision", true);
	addPhase("Decision", true);
	addPhase("PostDecision", true);
	addPhase("Triggers", true);
	addPhase("Viewports", true);
	addPhase("Motion", true);
	addPhase("Water", false);
	addPhase("Anim", true);
	addPhase("Corona", true);
	addPhase("DynamicsKick", false);
	addPhase("Build", true);
	addPhase("PostBuild", true);
	addPhase("Final", true);
	addPhase("Audio", true);

	return true;
}

//*****************************************************************************
void VuTickManagerImpl::postInit()
{
	// add keyboard callback
	if ( VuKeyboard::IF() )
		VuKeyboard::IF()->addCallback(this);

	// add phases to dev menu
	if ( VuDevMenu::IF() )
		for ( Phases::iterator iter = mPhases.begin(); iter != mPhases.end(); iter++ )
			VuDevMenu::IF()->addBool((std::string("TickManager/") + iter->mstrName).c_str(), iter->mbEnabled);

	// dev stats
	if ( VuDevStat::IF() )
		VuDevStat::IF()->addPage("TickManager", VuRect(50, 10, 40, 80));
}

//*****************************************************************************
void VuTickManagerImpl::preRelease()
{
	// remove keyboard callback
	if ( VuKeyboard::IF() )
		VuKeyboard::IF()->removeCallback(this);
}

//*****************************************************************************
void VuTickManagerImpl::tick()
{
	VU_PROFILE_SIM("Tick");

	// update time
	double currentTime = VuSys::IF()->getTime();
	float fdt = (float)(currentTime - mLastTime);
	mLastTime = currentTime;

	if ( fdt < 0.0f )
		VUPRINTF("Bad system clock behavior: negative dt!\n");

	// store unclamped real delta
	mUnclampedRealDeltaTime = fdt;

	// clamp
	fdt = VuClamp(fdt, 0.0f, mMaxClockDelta);

	// store real delta
	mRealDeltaTime = fdt;

	float fdtLast = mLastRealDeltaTime;

	// apply pause/slow
	if ( mbPaused && mbStep )
	{
		mbStep = false;
	}
	else if ( mbPaused )
	{
		fdt = 0;
		fdtLast = 0;
	}
	if ( mbSlowed )
	{
		fdt *= 0.1f;
		fdtLast *= 0.1f;
	}

	// apply pause request(s)
	if ( mPauseRequestCount )
	{
		fdt = 0;
		fdtLast = 0;
	}

	// tick phases
	for ( Phases::iterator iter = mPhases.begin(); iter != mPhases.end(); iter++ )
		if ( iter->mbEnabled )
			iter->tick(fdt, fdtLast);

	mLastRealDeltaTime = mRealDeltaTime;
	mGameTime += fdt;

	// dev stats
	if ( VuDevStat::IF() )
	{
		if ( VuDevStatPage *pPage = VuDevStat::IF()->getCurPage() )
		{
			if ( pPage->getName() == "TickManager" )
			{
				pPage->clear();
				for ( Phases::iterator iter = mPhases.begin(); iter != mPhases.end(); iter++ )
					pPage->printf("%8s: %3d handlers\n", iter->mstrName.c_str(), iter->mHandlers.size());
			}
		}
	}
}

//*****************************************************************************
void VuTickManagerImpl::onKeyDown(VUUINT32 key)
{
	if ( key == VUKEY_P )
	{
		mbPaused = !mbPaused;
	}
	else if ( key == VUKEY_O )
	{
		if ( mbPaused )
			mbStep = true;
		else
			mbSlowed = !mbSlowed;
	}
}

//*****************************************************************************
void VuTickManagerImpl::setMaxClockDelta(float fdt)
{
	mMaxClockDelta = fdt;
}

//*****************************************************************************
void VuTickManagerImpl::registerHandler(Handler *pHandler, const char *strPhase)
{
	if ( VuTickPhase *pPhase = getPhase(strPhase) )
		pPhase->add(pHandler);
	else
		VUASSERT(0, "VuTickManagerImpl::registerHandler() phase not found");
}

//*****************************************************************************
void VuTickManagerImpl::unregisterHandler(void *pObj, const char *strPhase)
{
	if ( VuTickPhase *pPhase = getPhase(strPhase) )
		pPhase->remove(pObj);
	else
		VUASSERT(0, "VuTickManagerImpl::unregisterHandler() phase not found");
}

//*****************************************************************************
void VuTickManagerImpl::unregisterHandlers(void *pObj)
{
	for ( Phases::iterator iter = mPhases.begin(); iter != mPhases.end(); iter++ )
		iter->remove(pObj);
}

//*****************************************************************************
void VuTickManagerImpl::addPhase(const char *strPhase, bool useLastDeltaTime)
{
	mPhases.push_back(VuTickPhase(strPhase, useLastDeltaTime));
}

//*****************************************************************************
VuTickPhase *VuTickManagerImpl::getPhase(const char *strPhase)
{
	for ( Phases::iterator iter = mPhases.begin(); iter != mPhases.end(); iter++ )
		if ( iter->mstrName == strPhase )
			return &(*iter);

	return VUNULL;
}

//*****************************************************************************
void VuTickPhase::tick(float fdt, float fdtLast)
{
	VU_PROFILE_SIM(mstrName.c_str());

	if ( mUseLastDeltaTime )
		fdt = fdtLast;

	// execute handlers
	for ( Handlers::iterator iter = mHandlers.begin(); iter != mHandlers.end(); iter++ )
	{
		mpCurObj = (*iter)->getObj();
		(*iter)->execute(fdt);
	}
	mpCurObj = VUNULL;

	// handle pending removals
	for ( Objects::iterator iter = mPendingRemovals.begin(); iter != mPendingRemovals.end(); iter++ )
		remove(*iter);
	mPendingRemovals.clear();
}
