//*****************************************************************************
//
//  Copyright (c) 2009-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  Keyframe Motion entity
// 
//*****************************************************************************

#include "VuMotionEntity.h"
#include "VuKeyframeEntity.h"
#include "VuEngine/Components/3dLayout/Vu3dLayoutComponent.h"
#include "VuEngine/Properties/VuBasicProperty.h"
#include "VuEngine/Properties/VuEnumProperty.h"
#include "VuEngine/Components/Motion/VuMotionComponent.h"
#include "VuEngine/Components/Script/VuScriptComponent.h"
#include "VuEngine/Gfx/VuGfxUtil.h"
#include "VuEngine/Gfx/Camera/VuCamera.h"
#include "VuEngine/Math/VuMatrix.h"
#include "VuEngine/Math/VuQuaternion.h"
#include "VuEngine/Math/VuCubicCurve.h"
#include "VuEngine/Math/VuPosSpline.h"
#include "VuEngine/Math/VuRotSpline.h"
#include "VuEngine/Math/VuMathUtil.h"


class VuKeyframeMotionEntity : public VuMotionEntity
{
	DECLARE_RTTI

public:
	VuKeyframeMotionEntity();

	virtual void	onGameInitialize();
	virtual void	onGameRelease();

	struct Keyframe
	{
		float			mTime;
		VuVector3		mPosition;
		VuQuaternion	mRotation;
	};

private:
	typedef VuArray<Keyframe> Keyframes;

	void			gatherKeyframes();
	void			buildCubicPosCurve();
	void			buildCubicEulerCurve();
	void			buildSpline();

	void			drawLayout(const Vu3dLayoutDrawParams &params);
	void			modified();

	virtual void	onActivate();
	virtual void	onDeactivate();
	virtual void	onUpdate(float fdt);

	void			setStaticKeyframe(int iFrame);
	void			setDynamicKeyframe(int iFrame0, int iFrame1, float fTime, float fTime_dt);
	inline void		interpolateKeyframe(int iFrame0, int iFrame1, float fTime, VuMatrix &out);

	VuVector3		calcLinearVel(int iFrame0, int iFrame1);
	VuVector3		calcAngularVel(int iFrame0, int iFrame1);

	void			update(VuMatrix &transform, VuVector3 &linVel, VuVector3 &angVel);

	enum eCurveType { CT_LINEAR, CT_CUBIC, CT_CUBIC_EULER, CT_SPLINE };

	// properties
	int				mCurveType;
	bool			mbLoop;
	bool			mbEaseInOut;
	bool			mbAffectPosition;
	bool			mbAffectRotation;

	// components
	Vu3dLayoutComponent	*mp3dLayoutComponent;

	Keyframes		mKeyframes;
	float			mTotalTime;
	float			mCurTime;

	VuCubicPosCurve	mCubicPosCurve;
	VuCubicPosCurve	mCubicEulerCurve;
	VuPosSpline		mPosSpline;
	VuRotSpline		mRotSpline;
};

IMPLEMENT_RTTI(VuKeyframeMotionEntity, VuMotionEntity);
IMPLEMENT_ENTITY_REGISTRATION(VuKeyframeMotionEntity);


// constants
#define CUBIC_DRAW_DT (0.1f)
#define SPLINE_DRAW_COUNT 100

//*****************************************************************************
static int CompareKeyframes(const void *p1, const void *p2)
{
	const VuKeyframeMotionEntity::Keyframe *pKeyframe1 = (const VuKeyframeMotionEntity::Keyframe *)p1;
	const VuKeyframeMotionEntity::Keyframe *pKeyframe2 = (const VuKeyframeMotionEntity::Keyframe *)p2;

	if ( pKeyframe1->mTime < pKeyframe2->mTime ) return -1;
	if ( pKeyframe1->mTime > pKeyframe2->mTime ) return 1;

	return 0;
}

//*****************************************************************************
VuKeyframeMotionEntity::VuKeyframeMotionEntity():
	VuMotionEntity(CAN_HAVE_CHILDREN),
	mCurveType(CT_LINEAR),
	mbLoop(false),
	mbEaseInOut(true),
	mbAffectPosition(true),
	mbAffectRotation(true),
	mTotalTime(0),
	mCurTime(0)
{
	// components
	addComponent(mp3dLayoutComponent = new Vu3dLayoutComponent(this));
	mp3dLayoutComponent->setDrawMethod(this, &VuKeyframeMotionEntity::drawLayout);
	mp3dLayoutComponent->setLocalBounds(VuAabb(VuVector3(-1e9), VuVector3(1e9)));

	// properties
	addProperty(new VuBoolProperty("Loop", mbLoop))->setWatcher(this, &VuKeyframeMotionEntity::modified);
	addProperty(new VuBoolProperty("Ease In/Out", mbEaseInOut))->setWatcher(this, &VuKeyframeMotionEntity::modified);
	addProperty(new VuBoolProperty("Affect Position", mbAffectPosition));
	addProperty(new VuBoolProperty("Affect Rotation", mbAffectRotation));

	static VuStaticIntEnumProperty::Choice curveTypeChoices[] =
	{
		{ "Linear", CT_LINEAR },
		{ "Cubic", CT_CUBIC },
		{ "Cubic Euler", CT_CUBIC_EULER },
		{ "Spline", CT_SPLINE },
		{ VUNULL }
	};
	addProperty(new VuStaticIntEnumProperty("Curve Type", mCurveType, curveTypeChoices))
		->setWatcher(this, &VuKeyframeMotionEntity::modified);

	// scripting
	ADD_SCRIPT_OUTPUT_NOARGS(mpScriptComponent, OnLoop);
	ADD_SCRIPT_OUTPUT_NOARGS(mpScriptComponent, OnDone);
}

//*****************************************************************************
void VuKeyframeMotionEntity::onGameInitialize()
{
	// gather keyframes
	gatherKeyframes();

	modified();

	VuMotionEntity::onGameInitialize();
}

//*****************************************************************************
void VuKeyframeMotionEntity::onGameRelease()
{
	VuMotionEntity::onGameRelease();

	mKeyframes.clear();
	mCubicPosCurve.clear();
	mCubicEulerCurve.clear();
	mPosSpline.clear();
	mRotSpline.clear();
}

//*****************************************************************************
void VuKeyframeMotionEntity::gatherKeyframes()
{
	mKeyframes.clear();

	for ( int i = 0; i < getChildEntityCount(); i++ )
	{
		if ( getChildEntity(i)->isDerivedFrom(VuKeyframeEntity::msRTTI) )
		{
			VuKeyframeEntity *pKeyframeEntity = static_cast<VuKeyframeEntity *>(getChildEntity(i));
			Keyframe keyframe;
			keyframe.mTime = pKeyframeEntity->getTime();
			keyframe.mPosition = pKeyframeEntity->getTransformComponent()->getWorldPosition();
			keyframe.mRotation = VuQuaternion(pKeyframeEntity->getTransformComponent()->getWorldTransform());
			mKeyframes.push_back(keyframe);
		}
	}

	if ( mKeyframes.size() )
	{
		// sort keyframes
		qsort(&mKeyframes[0], mKeyframes.size(), sizeof(mKeyframes[0]), CompareKeyframes);

		// normalize times
		float fStartTime = mKeyframes.begin().mTime;
		for ( int i = 0; i < mKeyframes.size(); i++ )
			mKeyframes[i].mTime -= fStartTime;
		mTotalTime = mKeyframes.back().mTime;
	}
}

//*****************************************************************************
void VuKeyframeMotionEntity::buildCubicPosCurve()
{
	mCubicPosCurve.clear();

	if ( mKeyframes.size() >= 2 )
	{
		mCubicPosCurve.reserve(mKeyframes.size());
		for ( int i = 0; i < mKeyframes.size(); i++ )
			mCubicPosCurve.addControlPoint(mKeyframes[i].mPosition, mKeyframes[i].mTime);

		VuVector3 startLinVel(0,0,0);
		VuVector3 endLinVel(0,0,0);
		if ( !mbEaseInOut )
		{
			startLinVel = calcLinearVel(0, 1);
			endLinVel = calcLinearVel(mKeyframes.size() - 2, mKeyframes.size() - 1);
			if (mbLoop)
			{
				VuVector3 avgLinVel = 0.5f*(startLinVel + endLinVel);
				startLinVel = avgLinVel;
				endLinVel = avgLinVel;
			}
		}
		mCubicPosCurve.build(startLinVel, endLinVel);
	}
}

//*****************************************************************************
void VuKeyframeMotionEntity::buildCubicEulerCurve()
{
	mCubicEulerCurve.clear();

	if ( mKeyframes.size() >= 2 )
	{
		VuArray<VuVector3> euler(0);
		euler.resize(mKeyframes.size());
		for ( int i = 0; i < euler.size(); i++ )
			mKeyframes[i].mRotation.toEulerAngles(euler[i]);

		for ( int i = 1; i < euler.size(); i++ )
			euler[i] = euler[i-1] + VuMathUtil::rotationDelta(euler[i-1], euler[i]);

		mCubicEulerCurve.reserve(mKeyframes.size());
		for ( int i = 0; i < mKeyframes.size(); i++ )
			mCubicEulerCurve.addControlPoint(euler[i], mKeyframes[i].mTime);

		VuVector3 startLinVel(0,0,0);
		VuVector3 endLinVel(0,0,0);
		if ( !mbEaseInOut )
		{
			startLinVel = calcAngularVel(0, 1);
			endLinVel = calcAngularVel(mKeyframes.size() - 2, mKeyframes.size() - 1);
			if (mbLoop)
			{
				VuVector3 avgLinVel = 0.5f*(startLinVel + endLinVel);
				startLinVel = avgLinVel;
				endLinVel = avgLinVel;
			}
		}
		mCubicEulerCurve.build(startLinVel, endLinVel);
	}
}

//*****************************************************************************
void VuKeyframeMotionEntity::buildSpline()
{
	mPosSpline.clear();
	mRotSpline.clear();

	if ( mKeyframes.size() >= 4 )
	{
		Keyframe nextKeyframe, prevKeyframe;

		if (mbLoop)
		{
			float totalTime = mKeyframes.back().mTime - mKeyframes.begin().mTime;

			prevKeyframe = mKeyframes[mKeyframes.size() - 2];
			prevKeyframe.mTime -= totalTime;

			nextKeyframe = mKeyframes[1];
			nextKeyframe.mTime += totalTime;
		}
		else
		{
			prevKeyframe = mKeyframes.begin();
			prevKeyframe.mPosition += mKeyframes[0].mPosition - mKeyframes[1].mPosition;
			prevKeyframe.mTime += mKeyframes[0].mTime - mKeyframes[1].mTime;

			nextKeyframe = mKeyframes.back();
			nextKeyframe.mPosition += mKeyframes.back().mPosition - mKeyframes[mKeyframes.size() - 2].mPosition;
			nextKeyframe.mTime += mKeyframes.back().mTime - mKeyframes[mKeyframes.size() - 2].mTime;
		}

		// positions
		{
			VuArray<VuPosSpline::Key> keys(0);
			keys.resize(mKeyframes.size() + 2);
			keys.begin().mPos = prevKeyframe.mPosition;
			keys.begin().mTime = prevKeyframe.mTime;
			for (int i = 0; i < mKeyframes.size(); i++)
			{
				keys[i + 1].mPos = mKeyframes[i].mPosition;
				keys[i + 1].mTime = mKeyframes[i].mTime;
			}
			keys.back().mPos = nextKeyframe.mPosition;
			keys.back().mTime = nextKeyframe.mTime;

			mPosSpline.build(&keys.begin(), keys.size());
		}

		// rotations
		{
			VuArray<VuRotSpline::Key> keys(0);
			keys.resize(mKeyframes.size() + 2);
			keys.begin().mRot = prevKeyframe.mRotation;
			keys.begin().mTime = prevKeyframe.mTime;
			for (int i = 0; i < mKeyframes.size(); i++)
			{
				keys[i + 1].mRot = mKeyframes[i].mRotation;
				keys[i + 1].mTime = mKeyframes[i].mTime;
			}
			keys.back().mRot = nextKeyframe.mRotation;
			keys.back().mTime = nextKeyframe.mTime;

			mRotSpline.build(&keys.begin(), keys.size());
		}
	}
}

//*****************************************************************************
void VuKeyframeMotionEntity::drawLayout(const Vu3dLayoutDrawParams &params)
{
	if ( params.mbSelected )
	{
		gatherKeyframes();

		if ( mCurveType == CT_LINEAR )
		{
			for ( int i = 0; i < mKeyframes.size() - 1; i++ )
				VuGfxUtil::IF()->drawLine3d(VuColor(128,255,128), mKeyframes[i].mPosition, mKeyframes[i+1].mPosition, params.mCamera.getViewProjMatrix());
		}
		else if ( mCurveType == CT_CUBIC || mCurveType == CT_CUBIC_EULER )
		{
			buildCubicPosCurve();
			if ( mCubicPosCurve.isBuilt() )
			{
				VuVector3 vPos;
				mCubicPosCurve.getPointAtTime(0.0f, vPos);
				for ( float fTime = CUBIC_DRAW_DT; fTime < mTotalTime; fTime += CUBIC_DRAW_DT )
				{
					VuVector3 vPrevPos = vPos;
					mCubicPosCurve.getPointAtTime(fTime, vPos);
					VuGfxUtil::IF()->drawLine3d(VuColor(128,255,128), vPrevPos, vPos, params.mCamera.getViewProjMatrix());
				}
				VuVector3 vPrevPos = vPos;
				mCubicPosCurve.getPointAtTime(mTotalTime, vPos);
				VuGfxUtil::IF()->drawLine3d(VuColor(128,255,128), vPrevPos, vPos, params.mCamera.getViewProjMatrix());
			}
		}
		else if ( mCurveType == CT_SPLINE )
		{
			buildSpline();
			if ( mPosSpline.isBuilt() )
			{
				VuVector3 vPos = mPosSpline.getPositionAtLength(0);
				for ( int i = 1; i < SPLINE_DRAW_COUNT; i++ )
				{
					VuVector3 vPrevPos = vPos;
					vPos = mPosSpline.getPositionAtLength(mPosSpline.getTotalLength()*i/SPLINE_DRAW_COUNT);
					VuGfxUtil::IF()->drawLine3d(VuColor(128,255,128), vPrevPos, vPos, params.mCamera.getViewProjMatrix());
				}
				VuVector3 vPrevPos = vPos;
				vPos = mPosSpline.getPositionAtLength(mPosSpline.getTotalLength());
				VuGfxUtil::IF()->drawLine3d(VuColor(128,255,128), vPrevPos, vPos, params.mCamera.getViewProjMatrix());
			}
		}
	}
}

//*****************************************************************************
void VuKeyframeMotionEntity::modified()
{
	if (mCurveType == CT_CUBIC)
	{
		buildCubicPosCurve();
	}
	if ( mCurveType == CT_CUBIC_EULER )
	{
		buildCubicPosCurve();
		buildCubicEulerCurve();
	}
	else if ( mCurveType == CT_SPLINE )
	{
		buildSpline();
	}
}

//*****************************************************************************
void VuKeyframeMotionEntity::onActivate()
{
	mCurTime = 0;
}

//*****************************************************************************
void VuKeyframeMotionEntity::onDeactivate()
{
}

//*****************************************************************************
void VuKeyframeMotionEntity::onUpdate(float fdt)
{
	if ( mKeyframes.size() == 0 )
	{
		Deactivate();
		return;
	}

	if ( mKeyframes.size() == 1 || mTotalTime < FLT_EPSILON )
	{
		setStaticKeyframe(0);
		Deactivate();
		return;
	}

	bool bTriggerLoopOutput = false;

	// have we reached the end?
	if ( mCurTime >= mTotalTime )
	{
		if ( mbLoop )
		{
			float fFrac = mCurTime/mTotalTime;
			fFrac -= VuFloor(fFrac);
			mCurTime = fFrac*mTotalTime;
			bTriggerLoopOutput = true;
		}
		else
		{
			setStaticKeyframe(mKeyframes.size() - 1);
			Deactivate();
			mpScriptComponent->getPlug("OnDone")->execute();
			return;
		}
	}

	// apply ease-in/ease-out
	float fTime = mCurTime;
	float fTime_dt = 1;
	if ( mbEaseInOut && (mCurveType == CT_LINEAR || mCurveType == CT_SPLINE) )
	{
		fTime = fTime/mTotalTime;
		fTime_dt = fTime_dt/mTotalTime;
		fTime = 3*fTime*fTime - 2*fTime*fTime*fTime;
		fTime_dt = 6*fTime*fTime_dt - 6*fTime*fTime*fTime_dt;
		fTime = fTime*mTotalTime;
		fTime_dt = fTime_dt*mTotalTime;
	}

	// find frames
	int iFrame0=0;
	for ( int i = 0; i < mKeyframes.size() - 1; i++ )
		if ( fTime >= mKeyframes[i].mTime )
			iFrame0 = i;
	int iFrame1 = iFrame0 + 1;

	setDynamicKeyframe(iFrame0, iFrame1, fTime, fTime_dt);

	// update time
	mCurTime += fdt;

	// trigger loop output after everything is said and done
	if ( bTriggerLoopOutput )
	{
		mpScriptComponent->getPlug("OnLoop")->execute();
	}
}

//*****************************************************************************
void VuKeyframeMotionEntity::setStaticKeyframe(int iFrame)
{
	VuMatrix transform;
	mKeyframes[iFrame].mRotation.toRotationMatrix(transform);
	transform.setTrans(mKeyframes[iFrame].mPosition);

	VuVector3 linVel(0,0,0);
	VuVector3 angVel(0,0,0);
	update(transform, linVel, angVel);
}

//*****************************************************************************
void VuKeyframeMotionEntity::setDynamicKeyframe(int iFrame0, int iFrame1, float fTime, float fTime_dt)
{
	#define STEP_DELTA 0.001f // seconds

	// interpolate keyframe at time t
	VuMatrix transform;
	interpolateKeyframe(iFrame0, iFrame1, fTime, transform);

	// interpolate keyframe at time t + STEP_DELTA
	VuMatrix transformDelta;
	interpolateKeyframe(iFrame0, iFrame1, fTime + fTime_dt*STEP_DELTA, transformDelta);

	// calculate/set linear velocity
	VuVector3 vLinVel = (transformDelta.getTrans() - transform.getTrans())/STEP_DELTA;

	// calculate/set angular velocity
	VuVector3 vAngVel = (transformDelta.getEulerAngles() - transform.getEulerAngles())/STEP_DELTA;

	update(transform, vLinVel, vAngVel);
}

//*****************************************************************************
inline void VuKeyframeMotionEntity::interpolateKeyframe(int iFrame0, int iFrame1, float fTime, VuMatrix &out)
{
	const Keyframe &kf0 = mKeyframes[iFrame0];
	const Keyframe &kf1 = mKeyframes[iFrame1];

	float fRatio = (fTime - kf0.mTime)/(kf1.mTime - kf0.mTime);

	out.loadIdentity();

	if ( mCurveType == CT_LINEAR )
	{
		VuVector3 vPos = VuLerp(kf0.mPosition, kf1.mPosition, fRatio);
		VuQuaternion qRot = VuSlerp(kf0.mRotation, kf1.mRotation, fRatio);
		qRot.toRotationMatrix(out);
		out.setTrans(vPos);
	}
	else if ( mCurveType == CT_CUBIC )
	{
		VuVector3 vPos;
		mCubicPosCurve.interpolate(iFrame0, iFrame1, fRatio, vPos);
		VuQuaternion qRot = VuSlerp(kf0.mRotation, kf1.mRotation, fRatio);
		qRot.toRotationMatrix(out);
		out.setTrans(vPos);
	}
	else if ( mCurveType == CT_CUBIC_EULER )
	{
		VuVector3 vPos, vRot;
		mCubicPosCurve.interpolate(iFrame0, iFrame1, fRatio, vPos);
		mCubicEulerCurve.interpolate(iFrame0, iFrame1, fRatio, vRot);
		out.setEulerAngles(vRot);
		out.setTrans(vPos);
	}
	else if ( mCurveType == CT_SPLINE )
	{
		VUASSERT(mPosSpline.isBuilt() && mRotSpline.isBuilt(), "Splines needs at least 4 keyframes!");

		VuVector3 vPos = mPosSpline.getPositionAtTime(fTime);
		VuQuaternion qRot = mRotSpline.getRotationAtTime(fTime);
		qRot.toRotationMatrix(out);
		out.setTrans(vPos);
	}
}

//*****************************************************************************
VuVector3 VuKeyframeMotionEntity::calcLinearVel(int iFrame0, int iFrame1)
{
	if ( iFrame0 >= 0 && iFrame1 < mKeyframes.size() )
	{
		const Keyframe &kf0 = mKeyframes[iFrame0];
		const Keyframe &kf1 = mKeyframes[iFrame1];

		if ( kf1.mTime > kf0.mTime )
			return (kf1.mPosition - kf0.mPosition)/(kf1.mTime - kf0.mTime);
	}

	return VuVector3(0,0,0);
}

//*****************************************************************************
VuVector3 VuKeyframeMotionEntity::calcAngularVel(int iFrame0, int iFrame1)
{
	if ( iFrame0 >= 0 && iFrame1 < mKeyframes.size() )
	{
		const Keyframe &kf0 = mKeyframes[iFrame0];
		const Keyframe &kf1 = mKeyframes[iFrame1];

		if ( kf1.mTime > kf0.mTime )
		{
			VuVector3 rot0, rot1;
			kf0.mRotation.toEulerAngles(rot0);
			kf1.mRotation.toEulerAngles(rot1);
			return (rot0 - rot1)/(kf1.mTime - kf0.mTime);
		}
	}

	return VuVector3(0,0,0);
}

//*****************************************************************************
void VuKeyframeMotionEntity::update(VuMatrix &transform, VuVector3 &linVel, VuVector3 &angVel)
{
	if ( VuTransformComponent *pTC = mpTargetMotionComponent->getOwnerEntity()->getTransformComponent() )
	{
		if ( !mbAffectPosition )
		{
			transform.setTrans(pTC->getWorldPosition());
			linVel = VuVector3(0,0,0);
		}
		
		if ( !mbAffectRotation )
		{
			VuVector3 vPos = transform.getTrans();
			transform = pTC->getWorldTransform();
			transform.setTrans(vPos);
			angVel = VuVector3(0,0,0);
		}
	}

	mpTargetMotionComponent->setWorldTransform(transform);
	mpTargetMotionComponent->setWorldLinearVelocity(linVel);
	mpTargetMotionComponent->setWorldAngularVelocity(angVel);
	mpTargetMotionComponent->update();
}
