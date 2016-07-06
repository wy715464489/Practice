//*****************************************************************************
//
//  Copyright (c) 2009-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  Explosion manager
// 
//*****************************************************************************

#pragma once

#include "VuEngine/VuSystemComponent.h"

class VuVector3;
class VuJsonContainer;
class VuDBAsset;
class VuEntity;


class VuExplosionManager : public VuSystemComponent
{
	DECLARE_SYSTEM_COMPONENT(VuExplosionManager)

protected:
	// called by engine
	friend class VuEngine;
	virtual bool init();
	virtual void release();

public:
	void	createExplosion(const VuVector3 &pos, const char *type, VuEntity *pOriginator);
	void	createExplosion(const VuVector3 &pos, const VuJsonContainer &data, VuEntity *pOriginator);

protected:
	VuDBAsset	*mpExplosionDB;
};
