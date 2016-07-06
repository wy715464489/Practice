//*****************************************************************************
//
//  Copyright (c) 2008-2008 Ralf Knoesel
//  Copyright (c) 2008-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  Dev Stats
// 
//*****************************************************************************

#include <stdarg.h>
#include "VuDevStat.h"
#include "VuDev.h"
#include "VuDevConfig.h"
#include "VuEngine/Math/VuRect.h"
#include "VuEngine/HAL/Gfx/VuGfx.h"
#include "VuEngine/HAL/Keyboard/VuKeyboard.h"
#include "VuEngine/Gfx/VuGfxUtil.h"
#include "VuEngine/Gfx/Font/VuFontDraw.h"
#include "VuEngine/Gfx/GfxSort/VuGfxSort.h"
#include "VuEngine/Managers/VuDrawManager.h"
#include "VuEngine/Util/VuDataUtil.h"


#if VU_DISABLE_DEV_STAT

IMPLEMENT_SYSTEM_COMPONENT(VuDevStat, VuDevStat);

#else

class VuDevStatPageImpl : public VuDevStatPage
{
public:
	VuDevStatPageImpl(const std::string &name, const VuRect &rect) : mName(name), mRect(rect) {}

	// external interface
	virtual const std::string	&getName() const	{ return mName; }
	virtual void				clear()				{ mText.clear(); }
	virtual void				printf(const char *fmt, ...);
	virtual const std::string	&getText() const	{ return mText; }

	// internal interface
	void					draw();

private:
	void					cullText(int maxLineCount);

	std::string				mName;
	VuRect					mRect;
	std::string				mText;
};

class VuDevStatImpl : public VuDevStat, public VuKeyboard::Callback
{
public:
	VuDevStatImpl();
	~VuDevStatImpl();

	virtual bool init() { return true; }

	virtual void					addPage(const char *strPage, const VuRect &rect);
	virtual VuDevStatPageImpl		*getPage(const char *strPage);
	virtual VuDevStatPageImpl		*getCurPage();

private:
	typedef std::map<std::string, VuDevStatPageImpl *> Pages;

	void					draw();

	// keyboard callback
	virtual void			onKeyDown(VUUINT32 key);

	bool					mbVisible;
	std::string				mCurPage;
	Pages					mPages;
};

// the interface
IMPLEMENT_SYSTEM_COMPONENT(VuDevStat, VuDevStatImpl);


//*****************************************************************************
VuDevStatImpl::VuDevStatImpl():
	mbVisible(false)
{
	VuKeyboard::IF()->addCallback(this);

	// start rendering
	VuDrawManager::IF()->registerHandler(this, &VuDevStatImpl::draw);
}

//*****************************************************************************
VuDevStatImpl::~VuDevStatImpl()
{
	// stop rendering
	VuDrawManager::IF()->unregisterHandler(this);

	VuKeyboard::IF()->removeCallback(this);

	for ( Pages::iterator iter = mPages.begin(); iter != mPages.end(); iter++ )
		delete iter->second;
}

//*****************************************************************************
VuDevStatPageImpl *VuDevStatImpl::getPage(const char *strPage)
{
	Pages::iterator iter = mPages.find(strPage);
	if ( iter != mPages.end() )
		return iter->second;

	return VUNULL;
}

//*****************************************************************************
VuDevStatPageImpl *VuDevStatImpl::getCurPage()
{
	if ( mbVisible )
		return getPage(mCurPage.c_str());

	return VUNULL;
}

//*****************************************************************************
void VuDevStatImpl::addPage(const char *strPage, const VuRect &rect)
{
	VUASSERT(mPages.find(strPage) == mPages.end(), "VuDevStatImpl::addPage page already exists");

	std::string name = strPage;

	mPages[strPage] = new VuDevStatPageImpl(name, rect);

	std::string startPageName;
	
	if(VuDevConfig::IF() && VuDataUtil::getValue(VuDevConfig::IF()->getParam("DevStatPage"), startPageName))
	{
		if(startPageName == name)
		{
			mCurPage = name;
		}
	}

	if ( mCurPage.size() == 0 )
		mCurPage = mPages.begin()->first;
}

//*****************************************************************************
void VuDevStatImpl::draw()
{
	if ( VuDevStatPageImpl *pPage = getCurPage() )
		pPage->draw();
}

//*****************************************************************************
void VuDevStatImpl::onKeyDown(VUUINT32 key)
{
	if ( !mPages.size() )
		return;

	if ( mbVisible )
	{
		Pages::iterator iter = mPages.find(mCurPage);
		if ( key == VUKEY_LEFT_BRACKET )
		{
			if ( iter == mPages.begin() )
				iter = mPages.end();
			iter--;
		}
		else if ( key == VUKEY_RIGHT_BRACKET )
		{
			iter++;
			if ( iter == mPages.end() )
				iter = mPages.begin();
		}
		else if ( key == VUKEY_M )
		{
			if ( VuDevStatPageImpl *pPage = getCurPage() )
			{
				VUPRINTF("\nDEV STAT PAGE DUMP: %s\n\n", pPage->getName().c_str());
				VUPRINTF("%s\n\n", pPage->getText().c_str());
			}
		}
		mCurPage = iter->first;
	}

	if ( key == VUKEY_BACKSLASH && mPages.size() )
		mbVisible = !mbVisible;
}

//*****************************************************************************
void VuDevStatPageImpl::printf(const char *fmt, ...)
{
	char str[256];

	va_list args;
	va_start(args, fmt);
	VU_VSNPRINTF(str, sizeof(str), sizeof(str) - 1, fmt, args);
	va_end(args);
	str[sizeof(str)-1] = '\0';

	mText.append(str);
}

//*****************************************************************************
void VuDevStatPageImpl::draw()
{
	VuGfxSort::IF()->setFullScreenLayer(VuGfxSort::FSL_DEV_STAT);

	VuGfxUtil *pGfxUtil = VuGfxUtil::IF();
	VuFontDraw *pFontDraw = pGfxUtil->fontDraw();
	VuFontDrawParams params;
	float textHeight = params.mSize/FONT_DRAW_SCALE_Y;

	VuRect screenRect = mRect/VuVector2(100, 100);

	// draw background
	pGfxUtil->drawFilledRectangle2d(GFX_SORT_DEPTH_STEP, VuColor(0, 0, 0, 192), screenRect);

	// draw outline
	pGfxUtil->drawRectangleOutline2d(0, VuColor(128,128,128), screenRect);

	// draw heading
	params.mColor = VuColor(255,255,0);
	pFontDraw->drawString(0, VuDev::IF()->getFont(), mName.c_str(), params, screenRect, VUGFX_TEXT_DRAW_CLIP);

	// calculate text rect
	VuRect rect = screenRect;
	rect.mY += 2*textHeight;
	rect.mHeight -= 2*textHeight;

	// cull text
	cullText((int)(rect.mHeight/textHeight));

	// draw text
	params.mColor = VuColor(255,255,255);
	pFontDraw->drawString(0, VuDev::IF()->getFont(), mText.c_str(), params, rect, VUGFX_TEXT_DRAW_CLIP|VUGFX_TEXT_DRAW_WORDBREAK);
}

//*****************************************************************************
void VuDevStatPageImpl::cullText(int maxLineCount)
{
	if ( maxLineCount <= 0 )
	{
		mText.clear();
		return;
	}

	// count lines of text (backwards)
	int lineCount = 1;
	for ( int iChar = (int)mText.length() - 2; iChar >= 0; iChar-- )
	{
		if ( mText[iChar] == '\n' )
		{
			if ( lineCount == maxLineCount )
			{
				mText = mText.substr(iChar + 1);
				return;
			}
			lineCount++;
		}
	}
}

#endif // VU_DISABLE_DEV_STAT
