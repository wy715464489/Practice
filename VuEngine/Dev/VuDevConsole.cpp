//*****************************************************************************
//
//  Copyright (c) 2008-2008 Ralf Knoesel
//  Copyright (c) 2008-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  Dev Console
// 
//*****************************************************************************

#include "VuDevConsole.h"
#include "VuEngine/HAL/Keyboard/VuKeyboard.h"
#include "VuEngine/HAL/Gfx/VuGfx.h"
#include "VuEngine/Gfx/VuGfxUtil.h"
#include "VuEngine/Gfx/Font/VuFontDraw.h"
#include "VuEngine/Gfx/GfxSort/VuGfxSort.h"
#include "VuEngine/Managers/VuDrawManager.h"
#include "VuEngine/Dev/VuDev.h"


#if VU_DISABLE_DEV_CONSOLE

IMPLEMENT_SYSTEM_COMPONENT(VuDevConsole, VuDevConsole);

#else

#define MAX_TEXT_SIZE (16*1024)
#define DEV_CONSOLE_FONT_HEIGHT 16 // % screen height

//*****************************************************************************
class VuDevConsoleImpl : public VuDevConsole, VuSys::LogCallback, VuKeyboard::Callback
{
public:
	VuDevConsoleImpl();
	~VuDevConsoleImpl();

	virtual void	show(bool bShow);

private:
	// VuSys::LogCallback
	virtual void	append(const char *str);

	// VuKeyboard::Callback
	virtual void	onKeyDown(VUUINT32 key);

	void			draw();

	bool			mbVisible;
	char			mText[MAX_TEXT_SIZE];
	int				mCurPos;
	float			mVertOffset;
};

// the interface
IMPLEMENT_SYSTEM_COMPONENT(VuDevConsole, VuDevConsoleImpl);


//*****************************************************************************
VuDevConsoleImpl::VuDevConsoleImpl():
	mbVisible(false),
	mCurPos(0),
	mVertOffset(0)
{
	memset(mText, 0, sizeof(mText));

	VuSys::IF()->addLogCallback(this);
	VuKeyboard::IF()->addCallback(this);
	VuDrawManager::IF()->registerHandler(this, &VuDevConsoleImpl::draw);

	printf("DevConsole Initialized...\n");
}

//*****************************************************************************
VuDevConsoleImpl::~VuDevConsoleImpl()
{
	VuDrawManager::IF()->unregisterHandler(this);
	VuKeyboard::IF()->removeCallback(this);
	VuSys::IF()->removeLogCallback(this);
}

//*****************************************************************************
void VuDevConsoleImpl::show(bool bShow)
{
	mbVisible = bShow;
}

//*****************************************************************************
void VuDevConsoleImpl::append(const char *str)
{
	while ( *str == '\b' )
	{
		if ( mCurPos == 0 )
			break;

		mCurPos--;
		mText[mCurPos] = '\0';

		str++;
	}

	int size = (int)strlen(str) + 1;
	if ( mCurPos + size >= MAX_TEXT_SIZE )
	{
		memmove(mText, mText + MAX_TEXT_SIZE/2, MAX_TEXT_SIZE/2);
		memset(mText + MAX_TEXT_SIZE/2, 0, MAX_TEXT_SIZE/2);
		mCurPos -= MAX_TEXT_SIZE/2;
	}

	VU_MEMCPY(&mText[mCurPos], MAX_TEXT_SIZE - mCurPos, str, size);
	mCurPos += size - 1;
}

//*****************************************************************************
void VuDevConsoleImpl::onKeyDown(VUUINT32 key)
{
	if ( key == VUKEY_ENTER )
	{
		mbVisible = !mbVisible;
		VuKeyboard::IF()->setCallbackPriority(this, mbVisible ? 65534 : 0);
	}

	if ( mbVisible )
	{
		float scroll = VuKeyboard::IF()->isKeyDown(VUKEY_SHIFT) ? 0.9f : DEV_CONSOLE_FONT_HEIGHT/FONT_DRAW_SCALE_Y;

		if ( key == VUKEY_UP )
		{
			mVertOffset += scroll;
		}
		else if ( key == VUKEY_DOWN )
		{
			mVertOffset -= scroll;
		}

		mVertOffset = VuMax(mVertOffset, 0.0f);
	}
}

//*****************************************************************************
void VuDevConsoleImpl::draw()
{
	if ( !mbVisible )
		return;

	VuGfxSort::IF()->setFullScreenLayer(VuGfxSort::FSL_DEV_CONSOLE);

	VuRect rect(0.05f, 0.05f, 0.9f, 0.9f);

	// draw background
	VuGfxUtil::IF()->drawFilledRectangle2d(GFX_SORT_DEPTH_STEP, VuColor(0, 0, 0, 192), rect);

	// draw text
	VuFontDrawParams params;
	params.mSize = DEV_CONSOLE_FONT_HEIGHT;
	VuVector2 offset(0.0f, mVertOffset);
	VuGfxUtil::IF()->fontDraw()->drawString(0, VuDev::IF()->getFont(), mText, params, rect, VUGFX_TEXT_DRAW_BOTTOM|VUGFX_TEXT_DRAW_WORDBREAK|VUGFX_TEXT_DRAW_CLIP, 1.0f, offset);
}

#endif // VU_DISABLE_DEV_CONSOLE
