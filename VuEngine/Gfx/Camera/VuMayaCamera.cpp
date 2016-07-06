//*****************************************************************************
//
//  Copyright (c) 2007-2008 Ralf Knoesel
//  Copyright (c) 2008-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  MayaCamera class
// 
//*****************************************************************************

#include "VuMayaCamera.h"


//*****************************************************************************
VuMayaCamera::VuMayaCamera():
	mDisplaySize(1000.0f, 1000.0f),
	mTarget(0,0,0),
	mDist(10.0f),
	mRotX(45.0f),
	mRotZ(-45.0f),
//	mButtons(0),
	mLastMouseX(0),
	mLastMouseY(0),
	mMovement(0)
{
	update();
}

//*****************************************************************************
bool VuMayaCamera::onMouseMove(VUINT x, VUINT y, VUINT dw, VUUINT32 buttons, VUUINT32 modifiers)
{
	// calc delta, update last pos
	VUINT dx = x - mLastMouseX;
	VUINT dy = y - mLastMouseY;
	mLastMouseX = x;
	mLastMouseY = y;

	// mouse wheel behaves like alt-right-button y axis
	if ( dw != 0 )
	{
		modifiers = MODIFIER_ALT;
		buttons = BUTTON_RIGHT;
		dy = -dw/10;
	}

	// calc orientation matrix
	VuMatrix mat;
	mat.loadIdentity();
	mat.rotateZLocal(VuDegreesToRadians(-mRotZ));
	mat.rotateXLocal(VuDegreesToRadians(-mRotX));

	bool bUpdate = false;

	if ( modifiers & MODIFIER_ALT )
	{
		if ( buttons & BUTTON_LEFT )
		{
			mRotZ += (float)dx;
			mRotX += (float)dy;
			bUpdate = true;
		}
		else if ( buttons & BUTTON_RIGHT )
		{
			float fScale = ((float)dy + 100.0f)/100.0f;
			fScale = VuMin(VuMax(fScale, -0.5f), 2.0f);
			
			mDist *= fScale;
			mDist = VuMax(mDist, 0.1f);
			bUpdate = true;
		}
		else if ( buttons & BUTTON_MIDDLE )
		{
			float fTransX = 1.4142f*mDist*(float)dx/mDisplaySize.mX;
			float fTransZ = 1.4142f*mDist*(float)dy/mDisplaySize.mY;
			
			mTarget -= mat.getAxisX()*fTransX;
			mTarget += mat.getAxisZ()*fTransZ;

			bUpdate = true;
		}
	}
	else if ( modifiers == MODIFIER_NONE )
	{
		if ( buttons & BUTTON_MIDDLE )
		{
			mRotZ += 0.5f*dx;
			mRotX += 0.5f*dy;
			VuVector3 prevEyePos = getEyePosition();
			update();
			VuVector3 newEyePos = getEyePosition();
			mTarget += prevEyePos - newEyePos;
			bUpdate = true;
		}
	}

	if ( bUpdate )
		update();

	return bUpdate;
}

//*****************************************************************************
void VuMayaCamera::startMovement(VUUINT32 dir)
{
	VUUINT32 lastMovement = mMovement;

	mMovement |= dir;

	if ( mMovement && !lastMovement )
		mLastTime = VuSys::IF()->getTime();
}

//*****************************************************************************
void VuMayaCamera::stopMovement(VUUINT32 dir)
{
	mMovement &= ~dir;
}

//*****************************************************************************
bool VuMayaCamera::tick()
{
	if ( mMovement )
	{
		// calculate dt
		double curTime = VuSys::IF()->getTime();
		float fDeltaSeconds = (float)(curTime - mLastTime);
		mLastTime = curTime;

		if ( fDeltaSeconds > 0.0f )
		{
			// calc orientation matrix
			VuMatrix mat;
			mat.loadIdentity();
			mat.rotateZLocal(VuDegreesToRadians(-mRotZ));
			mat.rotateXLocal(VuDegreesToRadians(-mRotX));

			if ( mMovement & MOVE_POS_X ) mTarget += mat.getAxisX()*mDist*fDeltaSeconds;
			if ( mMovement & MOVE_NEG_X ) mTarget -= mat.getAxisX()*mDist*fDeltaSeconds;
			if ( mMovement & MOVE_POS_Y ) mTarget += mat.getAxisY()*mDist*fDeltaSeconds;
			if ( mMovement & MOVE_NEG_Y ) mTarget -= mat.getAxisY()*mDist*fDeltaSeconds;
			if ( mMovement & MOVE_POS_Z ) mTarget += mat.getAxisZ()*mDist*fDeltaSeconds;
			if ( mMovement & MOVE_NEG_Z ) mTarget -= mat.getAxisZ()*mDist*fDeltaSeconds;

			update();
		}

		return true;
	}

	return false;
}

//*****************************************************************************
void VuMayaCamera::find(const VuVector3 &vTargetPos, float fTargetSize)
{
	// target
	mTarget = vTargetPos;
	mDist = VuMax(fTargetSize, 1.0f);
	
	// update
	update();
}

//*****************************************************************************
void VuMayaCamera::update()
{
	// normalize angles
	while ( mRotX <= -180.0f )	mRotX += 360.0f;
	while ( mRotX > 180.0f )	mRotX -= 360.0f;
	while ( mRotZ <= -180.0f )	mRotZ += 360.0f;
	while ( mRotZ > 180.0f )	mRotZ -= 360.0f;

	// build rotation matrix
	VuMatrix mat;
	mat.loadIdentity();
	mat.rotateZLocal(VuDegreesToRadians(-mRotZ));
	mat.rotateXLocal(VuDegreesToRadians(-mRotX));

	VuVector3 vPos = mat.transform(VuVector3(0, -mDist, 0));
	vPos += mTarget;

	// determine up vector
	VuVector3 vUp = mat.getAxisZ();

	setViewMatrix(vPos, mTarget, vUp);
}

