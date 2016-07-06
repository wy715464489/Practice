//*****************************************************************************
//
//  Copyright (c) 2009-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  DevTimer class
// 
//*****************************************************************************

#include "VuDevTimer.h"
#include "VuEngine/Dev/VuDev.h"
#include "VuEngine/Dev/VuDevConfig.h"
#include "VuEngine/Dev/VuDevMenu.h"
#include "VuEngine/Gfx/VuGfxUtil.h"
#include "VuEngine/Gfx/Font/VuFontDraw.h"
#include "VuEngine/Gfx/GfxSort/VuGfxSort.h"
#include "VuEngine/HAL/Gfx/VuGfx.h"
#include "VuEngine/HAL/Keyboard/VuKeyboard.h"
#include "VuEngine/Json/VuJsonContainer.h"
#include "VuEngine/Managers/VuDrawManager.h"
#include "VuEngine/Managers/VuTickManager.h"
#include "VuEngine/Util/VuStringUtil.h"


#if VU_DISABLE_DEV_TIMER

IMPLEMENT_SYSTEM_COMPONENT(VuDevTimer, VuDevTimer);

#else

class VuDevTimerImpl : public VuDevTimer, public VuKeyboard::Callback
{
protected:

	bool init();

public:

	VuDevTimerImpl();
	~VuDevTimerImpl();

	// VuDevTimer
	void show();
	void hide();
	void pause();
	void unpause();
	void reset();

	void tick(float fdt);

	void draw();

	void release();

	// VuKeyboard::Callback
	void onKeyDown(VUUINT32 key);

private:

	bool mbShowing;
	bool mbPaused;
	float mTimeElapsed;

	float mTextSize;
	float mX;
	float mY;
};

//*****************************************************************************
IMPLEMENT_SYSTEM_COMPONENT(VuDevTimer, VuDevTimerImpl);

//*****************************************************************************
bool VuDevTimerImpl::init()
{
	VuTickManager::IF()->registerHandler(this, &VuDevTimerImpl::tick, "Decision");
	VuDrawManager::IF()->registerHandler(this, &VuDevTimerImpl::draw);
	VuKeyboard::IF()->addCallback(this);

	const VuJsonContainer &config = VuDevConfig::IF()->getParam("DevTimer");

	config["Showing"].getValue(mbShowing);
	config["Size"].getValue(mTextSize);
	config["X"].getValue(mX);
	config["Y"].getValue(mY);

	VuDevMenu::IF()->addBool("Dev/Timer", mbShowing);

	return true;
}

//*****************************************************************************
VuDevTimerImpl::VuDevTimerImpl()
: mbShowing(false)
, mbPaused(true)
, mTimeElapsed(0.0f)
, mTextSize(4.0f)
, mX(0.0f)
, mY(0.0f)
{

}

//*****************************************************************************
VuDevTimerImpl::~VuDevTimerImpl()
{

}

//*****************************************************************************
void VuDevTimerImpl::show()
{
	mbShowing = true;
}

//*****************************************************************************
void VuDevTimerImpl::hide()
{
	mbShowing = false;
}

//*****************************************************************************
void VuDevTimerImpl::pause()
{
	mbPaused = true;
}

//*****************************************************************************
void VuDevTimerImpl::unpause()
{
	mbPaused = false;
}

//*****************************************************************************
void VuDevTimerImpl::reset()
{
	mTimeElapsed = 0.0f;
	pause();
}

//*****************************************************************************
void VuDevTimerImpl::tick(float fdt)
{
	if(!mbPaused)
	{
		mTimeElapsed += fdt;
	}
}

//*****************************************************************************
void VuDevTimerImpl::draw()
{
	if(!mbShowing)
	{
		return;
	}

	VuGfxSort::IF()->setFullScreenLayer(VuGfxSort::FSL_HUD);

	VuGfxUtil *pGfxUtil = VuGfxUtil::IF();
	VuFontDraw *pFontDraw = VuGfxUtil::IF()->fontDraw();
	VuFont *pFont = VuDev::IF()->getFont();


	float posX = 0.05f + 0.9f*mX;
	float posY = 0.05f + 0.9f*mY;

	pGfxUtil->pushTextScale(1.0f);
	{
		{
			VuColor gray(217,230,247);
			VuColor white(255, 255, 255);
			VuColor yellow(255, 229, 82);

			VuFontDrawParams params;

			params.mColor = gray;
			params.mSize = mTextSize;
			params.mWeight = 130;
			params.mOutlineWeight = 2;
			params.mOutlineSoftness = 2;

			char timeStr[256];

			VuStringUtil::timeFormatSeconds(VuStringUtil::MM_SS_HH, mTimeElapsed, timeStr, 256);
			
			pFontDraw->drawString(0, pFont, timeStr, params, VuRect(posX, posY, 0.0f, 0.0f), VUGFX_TEXT_DRAW_TOP|VUGFX_TEXT_DRAW_LEFT);
		}
	}
	pGfxUtil->popTextScale();
}

//*****************************************************************************
void VuDevTimerImpl::release()
{
	VuKeyboard::IF()->removeCallback(this);
	VuDrawManager::IF()->unregisterHandler(this);
	VuTickManager::IF()->unregisterHandler(this, "Decision");
}

//*****************************************************************************
void VuDevTimerImpl::onKeyDown(VUUINT32 key)
{
	if ( key == VUKEY_PERIOD )
	{
		mbShowing = true;

		if ( VuKeyboard::IF()->isKeyDown(VUKEY_SHIFT) )
		{
			reset();
		}
		else
		{
			if ( mbPaused )
			{
				unpause();
			}
			else
			{
				pause();
			}
		}
	}
}

#endif // VU_DISABLE_DEV_TIMER
