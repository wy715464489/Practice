//*****************************************************************************
//
//  Copyright (c) 2007-2008 Ralf Knoesel
//  Copyright (c) 2008-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  LocationMotionState class
// 
//*****************************************************************************

#pragma once

#include "VuDynamics.h"
#include "VuEngine/Components/Transform/VuTransformComponent.h"



class VuLocationMotionState : public btMotionState
{
public:
	VuLocationMotionState(VuTransformComponent *pTransform) : mpTransform(pTransform) {}

	virtual void	getWorldTransform(btTransform& worldTrans) const	{ worldTrans = VuDynamicsUtil::toBtTransform(mpTransform->getMatrix()); }
	virtual void	setWorldTransform(const btTransform& worldTrans)	{ mpTransform->setMatrix(VuDynamicsUtil::toVuMatrix(worldTrans), false); }

private:
	VuTransformComponent	*mpTransform;
};