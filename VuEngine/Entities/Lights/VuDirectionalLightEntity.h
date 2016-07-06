//*****************************************************************************
//
//  Copyright (c) 2008-2008 Ralf Knoesel
//  Copyright (c) 2008-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  Directional light entity
// 
//*****************************************************************************

#include "VuEngine/Entities/VuEntity.h"
#include "VuEngine/Components/Motion/VuMotionComponent.h"
#include "VuEngine/Util/VuColor.h"

class Vu3dLayoutDrawParams;
class VuScriptComponent;
class Vu3dLayoutComponent;


class VuDirectionalLightEntity : public VuEntity, public VuMotionComponentIF
{
	DECLARE_RTTI

public:
	VuDirectionalLightEntity();

	bool				isDefaultLight() const { return mDefaultLight; }
	const VuColor		&frontColor() const { return mFrontColor; }

private:
	// event handlers
	void				OnEditorProjectSelected(const VuParams &params) { if (mDefaultLight) apply(); }

	VuRetVal			Trigger(const VuParams &params);

	void				apply();

	void				drawLayout(const Vu3dLayoutDrawParams &params);

	// VuMotionComponentIF interface
	virtual void		onMotionUpdate();

	// components
	VuScriptComponent	*mpScriptComponent;
	Vu3dLayoutComponent	*mp3dLayoutComponent;
	VuMotionComponent	*mpMotionComponent;

	// properties
	bool				mDefaultLight;
	VuColor				mFrontColor;
	VuColor				mBackColor;
	VuColor				mSpecularColor;
	VuColor				mFoliageColor;
};
