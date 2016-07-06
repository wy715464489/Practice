//*****************************************************************************
//
//  Copyright (c) 2012-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  DepthFogComponent class
// 
//*****************************************************************************

#pragma once

#include "VuEngine/Components/VuComponent.h"

class VuStaticModelInstance;
class VuVector3;


class VuDepthFogComponent : public VuComponent
{
	DECLARE_SHORT_COMPONENT_TYPE(DepthFog)
	DECLARE_RTTI

public:
	VuDepthFogComponent(VuEntity *pOwner);

	virtual void	onLoad(const VuJsonContainer &data) { loadDepthFog(data); }
	virtual void	onSave(VuJsonContainer &data) const { saveDepthFog(data); }

	virtual void	onBake();

protected:
	void		collideRay(VuEntity *pEntity, const VuVector3 &v0, VuVector3 &v1);

	void		loadDepthFog(const VuJsonContainer &data);
	void		saveDepthFog(VuJsonContainer &data) const;

	enum eLocation { LOC_PIVOT, LOC_AABB, LOC_MANUAL };

	// properties
	int			mLocation;
	float		mManualWaterZ;
};
