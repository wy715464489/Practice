//*****************************************************************************
//
//  Copyright (c) 2007-2008 Ralf Knoesel
//  Copyright (c) 2008-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  Prop entity
// 
//*****************************************************************************

#pragma once

#include "VuEngine/Entities/VuEntity.h"
#include "VuEngine/Components/Motion/VuMotionComponentIF.h"

class Vu3dLayoutComponent;
class VuTransformComponent;
class VuScriptComponent;
class VuMotionComponent;
class VuAttachComponent;
class Vu3dDrawStaticModelComponent;
class VuRigidBodyComponent;
class Vu3dLayoutDrawParams;
class VuGfxDrawParams;
class VuGfxDrawShadowParams;


class VuPropEntity : public VuEntity, public VuMotionComponentIF
{
	DECLARE_RTTI

public:
	VuPropEntity();

	virtual void			onPostLoad() { transformModified(); }
	virtual void			onGameInitialize();
	virtual void			onGameRelease();

protected:
	// scripting
	VuRetVal				Show(const VuParams &params)	{ show(); return VuRetVal(); }
	VuRetVal				Hide(const VuParams &params)	{ hide(); return VuRetVal(); }

	void					show();
	void					hide();

	void					drawLayout(const Vu3dLayoutDrawParams &params);
	bool					collideLayout(const VuVector3 &v0, VuVector3 &v1);
	void					transformModified();

	// VuMotionComponentIF interface
	virtual void			onMotionUpdate();
	virtual void			onMotionActivate();
	virtual void			onMotionDeactivate();

	// components
	Vu3dLayoutComponent				*mp3dLayoutComponent;
	VuScriptComponent				*mpScriptComponent;
	VuMotionComponent				*mpMotionComponent;
	VuAttachComponent				*mpAttachComponent;
	Vu3dDrawStaticModelComponent	*mp3dDrawStaticModelComponent;
	VuRigidBodyComponent			*mpRigidBodyComponent;

	// properties
	bool							mbInitiallyVisible;

	bool							mbVisible;
};