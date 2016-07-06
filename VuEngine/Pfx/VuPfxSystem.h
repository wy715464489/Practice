//*****************************************************************************
//
//  Copyright (c) 2008-2008 Ralf Knoesel
//  Copyright (c) 2008-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  Pfx System
// 
//*****************************************************************************

#pragma once

#include "VuPfxNode.h"
#include "VuEngine/Containers/VuList.h"
#include "VuEngine/Math/VuMatrix.h"
#include "VuEngine/Math/VuAabb.h"

class VuPfxPattern;
class VuPfxPatternInstance;
class VuGfxDrawParams;
class VuGfxDrawShadowParams;


class VuPfxSystem : public VuPfxNode
{
	DECLARE_RTTI

protected:
	~VuPfxSystem() {}

public:
	VuPfxSystem();

	// properties
	float					mDuration;
};

class VuPfxSystemInstance : public VuListElement<VuPfxSystemInstance>
{
public:
	VuPfxSystemInstance();

	bool					create();
	void					destroy();


	inline void				setMatrix(const VuMatrix &mat);
	inline void				setPosition(const VuVector3 &pos);
	inline void				setRotation(const VuVector3 &rot);
	void					setLinearVelocity(const VuVector3 &linVel)	{ mLinearVelocity = linVel; }
	void					setScale(float scale)						{ mScale = scale; }
	void					setColor(const VuVector4 &color)			{ mColor = color; }

	enum eState { STATE_STOPPED, STATE_ALIVE, STATE_STOPPING };
	eState					getState()		{ return mState; }
	const VuAabb			&getAabb()		{ return mAabb; }
	int						particleCount()	{ return mParticleCount; }
	float					currentTime()	{ return mCurrentTime; }
	float					duration()		{ return mpParams->mDuration; }

	void					start();
	void					stop(bool bHardKill = false);

	void					tick(float fdt, bool ui);
	void					draw(const VuGfxDrawParams &params);
	void					drawShadow(const VuGfxDrawShadowParams &params);


	typedef VuList<VuPfxPatternInstance> Patterns;

	VuPfxSystem				*mpParams;
	Patterns				mPatterns;

	VuMatrix				mMatrix;
	VuVector3				mLinearVelocity;
	VuVector3				mRotation;
	eState					mState;
	VuAabb					mAabb;
	int						mParticleCount;
	float					mCurrentTime;
	float					mScale;
	VuVector4				mColor;
};


//*****************************************************************************
inline void VuPfxSystemInstance::setMatrix(const VuMatrix &mat)
{
	mMatrix = mat;
	mRotation = mMatrix.getEulerAngles();	// recalculate euler angles
}

//*****************************************************************************
inline void VuPfxSystemInstance::setPosition(const VuVector3 &pos)
{
	mMatrix.setTrans(pos);
}

//*****************************************************************************
inline void VuPfxSystemInstance::setRotation(const VuVector3 &rot)
{
	mRotation = rot;
	VuVector3 pos = mMatrix.getTrans();
	mMatrix = VuMatrix::rotationXYZ(mRotation);
	mMatrix.setTrans(pos);
}

