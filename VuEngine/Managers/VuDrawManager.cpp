//*****************************************************************************
//
//  Copyright (c) 2007-2008 Ralf Knoesel
//  Copyright (c) 2008-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  DrawManager class
// 
//*****************************************************************************

#include "VuDrawManager.h"
#include "VuEngine/Math/VuRect.h"
#include "VuEngine/HAL/Gfx/VuGfx.h"
#include "VuEngine/Gfx/VuGfxUtil.h"
#include "VuEngine/Gfx/GfxSort/VuGfxSort.h"
#include "VuEngine/Method/VuMethodUtil.h"
#include "VuEngine/Dev/VuDevMenu.h"
#include "VuEngine/Dev/VuDevStat.h"
#include "VuEngine/Dev/VuDevProfile.h"


// typedefs
typedef VuMethodInterface0<void> Handler;

// internal data
class VuDrawManagerImpl : public VuDrawManager
{
public:
	VuDrawManagerImpl();

	virtual bool			init();
	virtual void			postInit();
	virtual void			draw();

	virtual void			registerHandler(Handler *pHandler);
	virtual void			unregisterHandler(void *pObj);

private:
	typedef std::list<Handler *> Handlers;

	void			drawSafeZone();

	Handlers		mHandlers;
	bool			mbShowSafeZone;
};

// the interface
IMPLEMENT_SYSTEM_COMPONENT(VuDrawManager, VuDrawManagerImpl);


//*****************************************************************************
VuDrawManagerImpl::VuDrawManagerImpl():
	mbShowSafeZone(false)
{
}

//*****************************************************************************
bool VuDrawManagerImpl::init()
{
	return true;
}

//*****************************************************************************
void VuDrawManagerImpl::postInit()
{
	// dev menu
	if ( VuDevMenu::IF() )
		VuDevMenu::IF()->addBool("DrawManager/Show Safe Zone", mbShowSafeZone);

	// dev stats
	if ( VuDevStat::IF() )
		VuDevStat::IF()->addPage("DrawManager", VuRect(50, 10, 40, 80));
}

//*****************************************************************************
void VuDrawManagerImpl::draw()
{
	VU_PROFILE_SIM("Draw");

	// dev stats
	if ( VuDevStat::IF() )
	{
		if ( VuDevStatPage *pPage = VuDevStat::IF()->getCurPage() )
		{
			if ( pPage->getName() == "DrawManager" )
			{
				pPage->clear();
				pPage->printf("%3d handlers", mHandlers.size());
			}
		}
	}

	// draw
	for ( Handlers::iterator iter = mHandlers.begin(); iter != mHandlers.end(); iter++ )
		(*iter)->execute();

	if ( mbShowSafeZone )
		drawSafeZone();
}

//*****************************************************************************
void VuDrawManagerImpl::registerHandler(Handler *pHandler)
{
	mHandlers.push_back(pHandler);
}

//*****************************************************************************
void VuDrawManagerImpl::unregisterHandler(void *pObj)
{
	mHandlers.remove_if(isMethodOfObjectWithDelete(pObj));
}

//*****************************************************************************
void VuDrawManagerImpl::drawSafeZone()
{
	VuGfxSort::IF()->setFullScreenLayer(VuGfxSort::FSL_UI);
	VuGfxSort::IF()->setViewportLayer(VuGfxSort::VPL_UI);

	VuGfxUtil *pGfxUtil = VuGfxUtil::IF();

	// 90% is required (red)
	VuRect rect90 = VuRect(5.0f, 5.0f, 90.0f, 90.0f)/VuVector2(100,100);
	pGfxUtil->drawRectangleOutline2d(0, VuColor(255,64,64,128), rect90);

	// 80% is recommended (yellow)
	VuRect rect80 = VuRect(10.0f, 10.0f, 80.0f, 80.0f)/VuVector2(100,100);
	pGfxUtil->drawRectangleOutline2d(0, VuColor(255,255,64,128), rect80);
}
