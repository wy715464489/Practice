//*****************************************************************************
//
//  Copyright (c) 2009-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  Trigger entity
// 
//*****************************************************************************

#pragma once

#include "VuEngine/Entities/VuEntity.h"

class Vu3dLayoutComponent;
class VuScriptComponent;
class VuTransformComponent;
class Vu3dLayoutDrawParams;


class VuTriggerEntity : public VuEntity
{
	DECLARE_RTTI

public:
	VuTriggerEntity();

	virtual void		onPostLoad() { modified(); }
	virtual void		onGameInitialize();
	virtual void		onGameRelease();


protected:
	// called by trigger manager
	friend class VuTriggerManager;
	virtual void		update() = 0;
	virtual void		draw() = 0;

protected:

	void				doTrigger(VuEntity *pEntity, bool bEnter);

	virtual void		drawLayout(const Vu3dLayoutDrawParams &params) = 0;

	void				modified();

	// scripting
	VuRetVal			Activate(const VuParams &params = VuParams());
	VuRetVal			Deactivate(const VuParams &params = VuParams());

	// components
	Vu3dLayoutComponent		*mp3dLayoutComponent;
	VuScriptComponent		*mpScriptComponent;

	// properties
	bool				mbInitiallyActive;
	std::string			mTriggerType;

	VUUINT32			mTriggerMask;
	bool				mbActive;

	bool				mbFirstTrigger;
	VUINT32				mTriggerTimestamp;
};
