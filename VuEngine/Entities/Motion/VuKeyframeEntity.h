//*****************************************************************************
//
//  Copyright (c) 2009-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  Keyframe entity
// 
//*****************************************************************************

#include "VuEngine/Entities/VuEntity.h"
#include "VuEngine/Components/Transform/VuTransformComponent.h"

class Vu3dLayoutComponent;
class Vu3dLayoutDrawParams;


class VuKeyframeEntity : public VuEntity
{
	DECLARE_RTTI

public:
	VuKeyframeEntity();

	float				getTime() const			{ return mTime; }

private:
	void				drawLayout(const Vu3dLayoutDrawParams &params);

	// components
	Vu3dLayoutComponent		*mp3dLayoutComponent;

	// properties
	float				mTime;
};
