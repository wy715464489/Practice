//*****************************************************************************
//
//  Copyright (c) 2009-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  Animated Prop entity
// 
//*****************************************************************************

#include "VuEngine/Entities/VuEntity.h"
#include "VuEngine/Components/Motion/VuMotionComponentIF.h"
#include "VuEngine/Gfx/Model/VuAnimatedModelInstance.h"
#include "VuEngine/Animation/VuAnimationEventIF.h"

class Vu3dLayoutDrawParams;
class Vu3dDrawAnimatedModelComponent;
class Vu3dLayoutComponent;
class VuScriptComponent;
class VuAttachComponent;
class VuMotionComponent;
class VuRigidBodyComponent;
class VuAnimationControl;


class VuAnimatedPropEntity : public VuEntity, public VuAnimationEventIF, public VuMotionComponentIF
{
	DECLARE_RTTI

public:
	VuAnimatedPropEntity();
protected:
	~VuAnimatedPropEntity();

public:
	virtual void		onPostLoad() { transformModified(); }
	virtual void		onGameInitialize();
	virtual void		onGameRelease();

protected:
	// scripting
	VuRetVal			Show(const VuParams &params)	{ show(); return VuRetVal(); }
	VuRetVal			Hide(const VuParams &params)	{ hide(); return VuRetVal(); }
	VuRetVal			PlayAnimation(const VuParams &params);
	VuRetVal			AddAdditiveAnimation(const VuParams &params);
	VuRetVal			RemoveAdditiveAnimation(const VuParams &params);
	VuRetVal			SetAlpha(const VuParams &params);
	VuRetVal			SetAdditiveAlpha(const VuParams &params);

	virtual void		show();
	virtual void		hide();

	virtual void		tickAnim(float fdt);

	void				drawLayout(const Vu3dLayoutDrawParams &params);
	void				transformModified();

	// VuMotionComponentIF interface
	virtual void		onMotionUpdate();
	virtual void		onMotionActivate();
	virtual void		onMotionDeactivate();

	// VuAnimationEventIF
	virtual void		onAnimationEvent(const std::string &type, const VuJsonContainer &params);


	// components
	Vu3dDrawAnimatedModelComponent	*mp3dDrawAnimatedModelComponent;
	Vu3dLayoutComponent				*mp3dLayoutComponent;
	VuScriptComponent				*mpScriptComponent;
	VuAttachComponent				*mpAttachComponent;
	VuMotionComponent				*mpMotionComponent;
	VuRigidBodyComponent			*mpRigidBodyComponent;

	// properties
	bool				mbInitiallyVisible;
	int					mCollisionGroup;
	VUUINT32			mCollisionMask;

	bool				mActive;
	bool				mbBlending;
	float				mBlendRate;

	struct AdditiveAnimation
	{
		AdditiveAnimation() : mBlendRate(0) {}

		float				mBlendRate;
		VuAnimationControl	*mpAnimControl;
	};
	typedef std::map<VUUINT32, AdditiveAnimation> AdditiveAnimations;
	AdditiveAnimations	mAdditiveAnimations;
};
