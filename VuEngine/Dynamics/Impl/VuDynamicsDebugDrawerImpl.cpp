//*****************************************************************************
//
//  Copyright (c) 2007-2008 Ralf Knoesel
//  Copyright (c) 2008-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  DynamicsDebugDrawer class
// 
//*****************************************************************************

#include "VuDynamicsDebugDrawerImpl.h"
#include "VuEngine/Util/VuColor.h"
#include "VuEngine/HAL/Gfx/VuGfx.h"
#include "VuEngine/Dev/VuDev.h"
#include "VuEngine/Dev/VuDevMenu.h"


//*****************************************************************************
VuDynamicsDebugDrawerImpl::VuDynamicsDebugDrawerImpl():
	mDebugMode(0),
	mbEnabled(false)
{
	// populate debug menu
	#define ADD_DEBUG_MODE(mode) addDebugMode("VuDynamics/" #mode, btIDebugDraw::DBG_##mode)
	{
		ADD_DEBUG_MODE(DrawWireframe);
		ADD_DEBUG_MODE(DrawAabb);
		ADD_DEBUG_MODE(DrawFeaturesText);
		ADD_DEBUG_MODE(DrawContactPoints);
		ADD_DEBUG_MODE(NoDeactivation);
		ADD_DEBUG_MODE(NoHelpText);
		ADD_DEBUG_MODE(DrawText);
		ADD_DEBUG_MODE(ProfileTimings);
		ADD_DEBUG_MODE(EnableSatComparison);
		ADD_DEBUG_MODE(DisableBulletLCP);
		ADD_DEBUG_MODE(EnableCCD);
		ADD_DEBUG_MODE(DrawConstraints);
		ADD_DEBUG_MODE(DrawConstraintLimits);
		ADD_DEBUG_MODE(FastWireframe);
	}
	#undef ADD_DEBUG_MODE
}

//*****************************************************************************
VuDynamicsDebugDrawerImpl::~VuDynamicsDebugDrawerImpl()
{
}

//*****************************************************************************
void VuDynamicsDebugDrawerImpl::drawLine(const btVector3& from, const btVector3& to, const btVector3& color)
{
	if ( mbEnabled && getDebugMode() )
	{
		VuColor col((VUUINT8)(color.getX()*255.0f), (VUUINT8)(color.getY()*255.0f), (VUUINT8)(color.getZ()*255.0f));
		VuDev::IF()->drawLine(VuDynamicsUtil::toVuVector3(from), VuDynamicsUtil::toVuVector3(to), col);
	}
}

//*****************************************************************************
void VuDynamicsDebugDrawerImpl::drawContactPoint(const btVector3& pointOnB, const btVector3& normalOnB, btScalar distance, int lifeTime, const btVector3& color)
{
	if ( mbEnabled && (mDebugMode & btIDebugDraw::DBG_DrawContactPoints) )
	{
		drawLine(pointOnB, pointOnB + normalOnB*distance, color);

		char buf[12];
		VU_SPRINTF(buf,sizeof(buf)," %d",lifeTime);
		draw3dText(pointOnB, buf);
	}
}

//*****************************************************************************
void VuDynamicsDebugDrawerImpl::reportErrorWarning(const char* warningString)
{
	VUPRINTF(warningString);
}

//*****************************************************************************
void VuDynamicsDebugDrawerImpl::draw3dText(const btVector3& location, const char* textString)
{
	if ( mbEnabled )
		VuDev::IF()->printf(VuDynamicsUtil::toVuVector3(location), 0, VuColor(255,255,255), textString);
}

//*****************************************************************************
void VuDynamicsDebugDrawerImpl::addDebugMode(const char *strMode, btIDebugDraw::DebugDrawModes mode)
{
	mDebugModes.push_back(DebugMode(mode));
	if ( VuDevMenu::IF() )
		VuDevMenu::IF()->addBool(strMode, mDebugModes.back().mbValue);
}

//*****************************************************************************
void VuDynamicsDebugDrawerImpl::update()
{
	// update debug mode
	int debugMode = 0;
	for ( DebugModes::iterator iter = mDebugModes.begin(); iter != mDebugModes.end(); iter++ )
		if ( iter->mbValue )
			debugMode |= iter->mMode;
	setDebugMode(debugMode);
}
