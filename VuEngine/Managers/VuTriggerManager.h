//*****************************************************************************
//
//  Copyright (c) 2009-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  TriggerManager class
// 
//*****************************************************************************

#pragma once

#include "VuEngine/VuSystemComponent.h"
#include "VuEngine/Math/VuVector3.h"
#include "VuEngine/Containers/VuArray.h"

class VuInstigatorComponent;
class VuTransformComponent;
class VuTriggerEntity;


class VuTriggerManager : VuSystemComponent
{
	DECLARE_SYSTEM_COMPONENT(VuTriggerManager)

public:
	VuTriggerManager();
	~VuTriggerManager();

protected:
	// called by engine
	friend class VuEngine;
	virtual bool		init();
	virtual void		release();

public:
	typedef std::vector<std::string> Types;

	struct VuInstigatorEntry
	{
		VuInstigatorComponent	*mpInstigatorComponent;
		VuTransformComponent	*mpTransformComponent;
		VUUINT32				mMask;
		VuVector3				mPrevPos;
		float					mPrevRadius;
		VuVector3				mCurPos;
		float					mCurRadius;
	};
	typedef VuArray<VuInstigatorEntry> Instigators;

	// types/masks
	static void			addType(const std::string &strType)	{ smTypes.push_back(strType); }
	static const Types	&getTypes()							{ return smTypes; }
	static VUUINT32		getTypeMask(const char *strType);

	// instigator access
	const Instigators	&getInstigators()	{ return mInstigators; }

protected:
	// called by instigator component
	friend class VuInstigatorComponent;
	void			addInstigator(VuInstigatorComponent *pInstigatorComponent);
	void			removeInstigator(VuInstigatorComponent *pInstigatorComponent);
	void			snapInstigator(VuInstigatorComponent *pInstigatorComponent);

	// called by trigger entity
	friend class VuTriggerEntity;
	void			addTriggerEntity(VuTriggerEntity *pTriggerEntity);
	void			removeTriggerEntity(VuTriggerEntity *pTriggerEntity);

private:
	void			tick(float fdt);
	void			draw();

	typedef VuArray<VuTriggerEntity *> TriggerEntities;

	static Types	smTypes;
	Instigators		mInstigators;
	TriggerEntities	mTriggerEntities;

	bool			mBusy;
	TriggerEntities	mAddedTriggers;
	TriggerEntities	mRemovedTriggers;
};
