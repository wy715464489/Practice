//*****************************************************************************
//
//  Copyright (c) 2009-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  MotionComponent interface class
// 
//*****************************************************************************

#pragma once


class VuMotionComponentIF
{
public:
	virtual void	onMotionUpdate()		{}
	virtual void	onMotionActivate()		{}
	virtual void	onMotionDeactivate()	{}
};
