//*****************************************************************************
//
//  Copyright (c) 2007-2008 Ralf Knoesel
//  Copyright (c) 2008-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  Dynamic Prop entity
// 
//*****************************************************************************

#pragma once

#include "btBulletDynamicsCommon.h"
#include "VuEngine/Entities/VuEntity.h"

class Vu3dLayoutComponent;
class VuTransformComponent;
class VuScriptComponent;
class VuAttachComponent;
class Vu3dDrawStaticModelComponent;
class VuRigidBodyComponent;
class Vu3dLayoutDrawParams;
class VuGfxDrawParams;
class VuGfxDrawShadowParams;


class VuDynamicPropEntity : public VuEntity, btMotionState
{
	DECLARE_RTTI

public:
	VuDynamicPropEntity();

	virtual void			onPostLoad();
	virtual void			onGameInitialize();
	virtual void			onGameRelease();

protected:
	// btMotionState
	virtual void			getWorldTransform(btTransform& worldTrans) const;
	virtual void			setWorldTransform(const btTransform& worldTrans);

	// scripting
	VuRetVal				Show(const VuParams &params)	{ show(); return VuRetVal(); }
	VuRetVal				Hide(const VuParams &params)	{ hide(); return VuRetVal(); }

	virtual void			show();
	virtual void			hide();

	virtual void			drawLayout(const Vu3dLayoutDrawParams &params);
	bool					collideLayout(const VuVector3 &v0, VuVector3 &v1);
	void					transformModified();
	void					massModified();

	virtual void			tickBuild(float fdt);

	// components
	Vu3dLayoutComponent				*mp3dLayoutComponent;
	VuScriptComponent				*mpScriptComponent;
	VuAttachComponent				*mpAttachComponent;
	Vu3dDrawStaticModelComponent	*mp3dDrawStaticModelComponent;
	VuRigidBodyComponent			*mpRigidBodyComponent;

	// properties
	bool							mbInitiallyVisible;
	float							mMass;
	VuVector3						mCenterOfMass;
	bool							mCollideWithStaticProps;

	bool							mbVisible;
};