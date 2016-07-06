//*****************************************************************************
//
//  Copyright (c) 2008-2008 Ralf Knoesel
//  Copyright (c) 2008-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  Ambient light entity
// 
//*****************************************************************************

#include "VuEngine/Entities/VuEntity.h"
#include "VuEngine/Util/VuColor.h"

class VuScriptComponent;


class VuAmbientLightEntity : public VuEntity
{
	DECLARE_RTTI

public:
	VuAmbientLightEntity();

	bool				isDefaultLight() const { return mDefaultLight; }
	const VuColor		&color() const { return mColor; }

private:
	// event handlers
	void				OnEditorProjectSelected(const VuParams &params) { if (mDefaultLight) apply(); }

	VuRetVal			Trigger(const VuParams &params);

	void				apply();

	// components
	VuScriptComponent	*mpScriptComponent;

	// properties
	bool				mDefaultLight;
	VuColor				mColor;
	VuColor				mFoliageColor;
};
