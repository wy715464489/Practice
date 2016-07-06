//*****************************************************************************
//
//  Copyright (c) 2007-2008 Ralf Knoesel
//  Copyright (c) 2008-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  TransformComponent class
// 
//*****************************************************************************

#pragma once

#include "VuEngine/Components/VuComponent.h"
#include "VuEngine/Math/VuMatrix.h"
#include "VuEngine/Method/VuMethod.h"


class VuTransformComponent : public VuComponent
{
	DECLARE_SHORT_COMPONENT_TYPE(Transform)
	DECLARE_RTTI

public:
	VuTransformComponent(VuEntity *pOwner);
	~VuTransformComponent();

	virtual void		onPostLoad();
	virtual void		onGameReset();

	void				setLocalTransform(const VuMatrix &xform, bool notify = true);
	void				setLocalTransform(const VuVector3 &pos, const VuVector3 &rot, bool notify = true);
	void				setLocalPosition(const VuVector3 &pos, bool notify = true);
	void				setLocalRotation(const VuVector3 &rot, bool notify = true);
	void				setLocalScale(const VuVector3 &scale, bool notify = true);

	void				setWorldTransform(const VuMatrix &xform, bool notify = true);
	void				setWorldTransform(const VuVector3 &pos, const VuVector3 &rot, bool notify = true);
	void				setWorldPosition(const VuVector3 &pos, bool notify = true);
	void				setWorldRotation(const VuVector3 &rot, bool notify = true);
	void				setWorldScale(const VuVector3 &scale, bool notify = true);

	const VuMatrix		&getLocalTransform() const	{ return mLocalTransform; }
	const VuVector3		&getLocalPosition() const	{ return mLocalTransform.getTrans(); }
	const VuVector3		&getLocalRotation() const	{ return mLocalRotation; }
	const VuVector3		&getLocalScale()	const	{ return mLocalScale; }

	const VuMatrix		&getWorldTransform() const	{ return mWorldTransform; }
	const VuVector3		&getWorldPosition() const	{ return mWorldTransform.getTrans(); }
	const VuVector3		&getWorldRotation() const	{ return mWorldRotation; }
	const VuVector3		&getWorldScale()	const	{ return mWorldScale; }

	template<class T>
	void				setWatcher(void (T::*method)());

	// manipulation mask
	enum {
		TRANS_X	= (1<<0),
		TRANS_Y	= (1<<1),
		TRANS_Z	= (1<<2),
		ROT_X	= (1<<3),
		ROT_Y	= (1<<4),
		ROT_Z	= (1<<5),
		SCALE_X	= (1<<6),
		SCALE_Y	= (1<<7),
		SCALE_Z	= (1<<8),

		TRANS	= TRANS_X|TRANS_Y|TRANS_Z,
		ROT		= ROT_X|ROT_Y|ROT_Z,
		SCALE	= SCALE_X|SCALE_Y|SCALE_Z,
	};
	void		setMask(VUUINT32 mask) { mMask = mask; }
	VUUINT32	getMask() const        { return mMask; }

private:
	void						addProperties();
	void						propertiesModified();
	void						notifyWatcher();
	void						recalcLocalTransform();
	void						recalcWorldTransform();
	void						recalcLocalScale();
	void						recalcWorldScale();
	void						updateChildren(bool notify);
	static void					calcTransformFromEulerPos(VuMatrix &transform, const VuVector3 &euler, VuVector3 pos);

	VuMatrix					mLocalTransform;
	VuMatrix					mWorldTransform;
	VuVector3					mLocalRotation;
	VuVector3					mWorldRotation;
	VuVector3					mLocalScale;
	VuVector3					mWorldScale;
	VuMethodInterface0<void>	*mpWatcher;
	VUUINT32					mMask;
};

#include "VuTransformComponent.inl"
