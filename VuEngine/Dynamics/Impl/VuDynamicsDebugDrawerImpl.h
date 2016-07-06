//*****************************************************************************
//
//  Copyright (c) 2007-2008 Ralf Knoesel
//  Copyright (c) 2008-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  DynamicsDebugDrawer class
// 
//*****************************************************************************

#pragma once

#include "VuEngine/Dynamics/VuDynamics.h"


class VuDynamicsDebugDrawerImpl : public btIDebugDraw
{
public:
	VuDynamicsDebugDrawerImpl();
	~VuDynamicsDebugDrawerImpl();

	// btIDebugDraw interface
	virtual void	drawLine(const btVector3& from, const btVector3& to, const btVector3& color);
	virtual void	drawContactPoint(const btVector3& pointOnB, const btVector3& normalOnB, btScalar distance, int lifeTime, const btVector3& color);
	virtual void	reportErrorWarning(const char* warningString);
	virtual void	draw3dText(const btVector3& location, const char* textString);

	virtual void	setDebugMode(int debugMode)	{ mDebugMode = debugMode; }
	virtual int		getDebugMode() const		{ return mDebugMode; }

	void			update();

	void			enable(bool enable)	{ mbEnabled = enable; }

private:
	struct DebugMode
	{
		DebugMode(btIDebugDraw::DebugDrawModes mode) : mMode(mode), mbValue(false) {}
		btIDebugDraw::DebugDrawModes	mMode;
		bool							mbValue;
	};
	typedef std::list<DebugMode> DebugModes;

	void		addDebugMode(const char *strMode, btIDebugDraw::DebugDrawModes mode);

	int			mDebugMode;
	DebugModes	mDebugModes;
	bool		mbEnabled;
};
