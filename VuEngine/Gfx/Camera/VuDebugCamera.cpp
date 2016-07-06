//*****************************************************************************
//
//  Copyright (c) 2007-2008 Ralf Knoesel
//  Copyright (c) 2008-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  DebugCamera class
// 
//*****************************************************************************

#include "VuDebugCamera.h"
#include "VuEngine/Math/VuAabb.h"
#include "VuEngine/Managers/VuInputManager.h"
#include "VuEngine/HAL/Keyboard/VuKeyboard.h"


// constants

static const float MOVEMENT_SPEED = 100.0f;
static const float ROTATION_SPEED = VuDegreesToRadians(360.0f);
static const float KEYBOARD_DAMPING = 10.0f;


//*****************************************************************************
VuDebugCamera::VuDebugCamera():
	mLinControl(0,0,0), mRotControl(0,0)
{
	frame(VuAabb::zero());
}

//*****************************************************************************
void VuDebugCamera::tick(float fdt, int padIndex)
{
	VuVector3 vTgtLinControl(0,0,0);
	VuVector2 vTgtRotControl(0,0);

	// pad
	vTgtLinControl.mX += VuInputManager::IF()->getAxisValue(padIndex, "DebugCameraMoveX");
	vTgtLinControl.mY += VuInputManager::IF()->getAxisValue(padIndex, "DebugCameraMoveY");
	vTgtLinControl.mZ += VuInputManager::IF()->getAxisValue(padIndex, "DebugCameraMoveZ");

	vTgtRotControl.mX += VuInputManager::IF()->getAxisValue(padIndex, "DebugCameraRotateX");
	vTgtRotControl.mY += VuInputManager::IF()->getAxisValue(padIndex, "DebugCameraRotateY");

	// keyboard
	if ( !VuKeyboard::IF()->isKeyDown(VUKEY_SHIFT) && !VuKeyboard::IF()->isKeyDown(VUKEY_ALT) )
	{
		if ( VuKeyboard::IF()->isKeyDown(VUKEY_D, 0) ) vTgtLinControl.mX += 1.0f;
		if ( VuKeyboard::IF()->isKeyDown(VUKEY_A, 0) ) vTgtLinControl.mX -= 1.0f;
		if ( VuKeyboard::IF()->isKeyDown(VUKEY_W, 0) ) vTgtLinControl.mY += 1.0f;
		if ( VuKeyboard::IF()->isKeyDown(VUKEY_S, 0) ) vTgtLinControl.mY -= 1.0f;
		if ( VuKeyboard::IF()->isKeyDown(VUKEY_Q, 0) ) vTgtLinControl.mZ += 1.0f;
		if ( VuKeyboard::IF()->isKeyDown(VUKEY_Z, 0) ) vTgtLinControl.mZ -= 1.0f;

		if ( VuKeyboard::IF()->isKeyDown(VUKEY_RIGHT, 0) ) vTgtRotControl.mX += 1.0f;
		if ( VuKeyboard::IF()->isKeyDown(VUKEY_LEFT,  0) ) vTgtRotControl.mX -= 1.0f;
		if ( VuKeyboard::IF()->isKeyDown(VUKEY_UP,    0) ) vTgtRotControl.mY += 1.0f;
		if ( VuKeyboard::IF()->isKeyDown(VUKEY_DOWN,  0) ) vTgtRotControl.mY -= 1.0f;
	}

	// damping
	float damping = VuMin(KEYBOARD_DAMPING*fdt, 1.0f);
	mLinControl += (vTgtLinControl - mLinControl)*damping;
	mRotControl += (vTgtRotControl - mRotControl)*damping;

	VuVector3 vEye = getEyePosition();
	VuVector3 vTarget = getTargetPosition();

	// movement
	VuVector3 vLinVel = mLinControl;
	vLinVel *= vLinVel*vLinVel;
	vLinVel *= MOVEMENT_SPEED;
	vLinVel = getTransform().transformNormal(vLinVel);

	vEye += fdt*vLinVel;
	vTarget += fdt*vLinVel;

	// rotation
	VuVector3 vRight = VuCross(vTarget - vEye, VuVector3(0,0,1));
	if ( vRight.magSquared() < FLT_MIN )
		vRight = VuVector3(1,0,0);
	else
		vRight.normalize();

	VuVector2 vRotVel = mRotControl;
	vRotVel *= vRotVel*vRotVel;
	vRotVel *= ROTATION_SPEED;
	VuMatrix matRot;
	matRot.loadIdentity();
	matRot.rotateZ(-vRotVel.mX*fdt);
	matRot.rotateAxisLocal(vRight, vRotVel.mY*fdt);
	vTarget = vEye + matRot.transformNormal(vTarget - vEye);

	setViewMatrix(vEye, vTarget, VuVector3(0,0,1));
	if ( fdt > FLT_EPSILON )
		setListenerVelocity(vLinVel);
}

//*****************************************************************************
void VuDebugCamera::frame(const VuAabb &aabb)
{
	VuVector3 vTarget = aabb.getCenter();
	float dist = VuClamp(aabb.getSize().mag(), 5.0f, 25.0f);
	VuVector3 vEye = vTarget + dist*VuVector3(-1, -1, 1).normal();

	setViewMatrix(vEye, vTarget, VuVector3(0,0,1));
}

//*****************************************************************************
void VuDebugCamera::operator = (const VuCamera &other)
{
	(VuCamera &)(*this) = other;
}
