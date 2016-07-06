//*****************************************************************************
//
//  Copyright (c) 2011-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  Draw collision implementation
// 
//*****************************************************************************

#pragma once

class VuCamera;
class VuColor;
class VuMatrix;
class btCollisionShape;


namespace VuDynamicsDrawCollisionImpl
{
	void	drawCollision(const VuCamera &camera);
	void	drawCollisionObject(const VuCamera &camera, const VuMatrix &worldTransform, const btCollisionShape *shape, const VuColor &color);
}
