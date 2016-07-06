//*****************************************************************************
//
//  Copyright (c) 2009-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  Motion entity
// 
//*****************************************************************************

#pragma once

#include "VuEngine/Entities/VuEntity.h"

class VuScriptComponent;
class VuMotionComponent;
class VuScriptRef;


class VuMotionEntity : public VuEntity
{
	DECLARE_RTTI

public:
	VuMotionEntity(VUUINT32 flags = 0);

	virtual void		onGameInitialize();
	virtual void		onGameRelease();
	virtual void		onGameReset();

protected:
	// scripting
	VuRetVal			Activate(const VuParams &params = VuParams());
	VuRetVal			Deactivate(const VuParams &params = VuParams());

	void				tickMotion(float fdt);

	// implemented by derived classes
	virtual void		onActivate() = 0;
	virtual void		onDeactivate() = 0;
	virtual void		onUpdate(float fdt) = 0;

	// components
	VuScriptComponent	*mpScriptComponent;

	// references
	VuScriptRef			*mpEntityRef;

	// properties
	bool				mbInitiallyActive;
	bool				mbOneShot;
	
	VuMotionComponent	*mpTargetMotionComponent;
	bool				mbActive;
	bool				mbIsShot;
};
