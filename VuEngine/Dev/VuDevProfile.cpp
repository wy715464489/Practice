//*****************************************************************************
//
//  Copyright (c) 2008-2008 Ralf Knoesel
//  Copyright (c) 2008-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  Dev Profile
// 
//*****************************************************************************

#include <float.h>
#include "VuDevProfile.h"
#include "VuDev.h"
#include "VuEngine/Math/VuRect.h"
#include "VuEngine/HAL/Gfx/VuGfx.h"
#include "VuEngine/HAL/Keyboard/VuKeyboard.h"
#include "VuEngine/Gfx/VuGfxUtil.h"
#include "VuEngine/Gfx/Font/VuFontDraw.h"
#include "VuEngine/Gfx/GfxSort/VuGfxSort.h"
#include "VuEngine/Util/VuFileUtil.h"
#include "VuEngine/Managers/VuDrawManager.h"
#include "VuEngine/Json/VuJsonContainer.h"
#include "VuEngine/Util/VuHashedString.h"
#include "VuEngine/Dev/VuDevConfig.h"


#if VU_DISABLE_DEV_PROFILE

IMPLEMENT_SYSTEM_COMPONENT(VuDevProfile, VuDevProfile);

#else

// constants

#define MAX_SAMPLE_COUNT 1024
#define TEXT_BUFFER_SIZE (8*1024)
#define MAX_SAMPLE_STRING_SIZE 32
#define SAMPLE_HISTORY_COUNT 10
#define FPS_HISTORY_COUNT 20


class VuDevProfileSampler
{
public:
	VuDevProfileSampler();

	void			setHeader(const char *strHeader)	{ mHeader = strHeader; }
	void			setEnabled(bool enabled)			{ mbEnabledPending = enabled; }
	bool			isEnabled()							{ return mbEnabled; }
	void			reset()								{ mbResetPending = true; }

	void			begin(const char *strSampleName);
	void			end();
	void			update();

	const char		*textBuffer()	{ return mTextBuffer; }

private:
	struct Sample
	{
		char		mstrName[MAX_SAMPLE_STRING_SIZE];
		VUUINT32	mHashedName;
		Sample		*mpParent;
		Sample		*mpFirstChild;
		Sample		*mpNextSibling;
		double		mBeginTime;
		int			mCount;
		float		mAccumulatedTime;
		float		mMinTime;
		float		mMaxTime;
		float		mHistory[SAMPLE_HISTORY_COUNT];
	};

	void			writeSamplesToTextBuffer();

	std::string		mHeader;
	bool			mbEnabled;
	bool			mbEnabledPending;
	bool			mbResetPending;

	Sample			maSamples[MAX_SAMPLE_COUNT];
	int				mSampleCount;
	Sample			*mpCurSample;
	int				mSampleHistoryIndex;

	char			mTextBuffer[TEXT_BUFFER_SIZE];
};


class VuDevProfileImpl : public VuDevProfile, public VuKeyboard::Callback
{
public:
	VuDevProfileImpl();
	~VuDevProfileImpl();

	// external functionality
	virtual bool	init();
	virtual void	beginSim();
	virtual void	endSim();
	virtual void	beginGfx();
	virtual void	endGfx();
	virtual void	synchronizeGfx();
	virtual void	beginDyn(float overlapTime);
	virtual void	endDyn();
	virtual void	synchronizeDyn();
	virtual void	beginWater(float overlapTime);
	virtual void	endWater();
	virtual void	synchronizeWater();

	// internal functionality
	static VuDevProfileImpl *IF()	{ return static_cast<VuDevProfileImpl *>(VuDevProfile::IF()); }

	VuDevProfileSampler	&simSampler()	{ return mSimSampler; }
	VuDevProfileSampler	&gfxSampler()	{ return mGfxSampler; }
	VuDevProfileSampler	&dynSampler()	{ return mDynSampler; }
	VuDevProfileSampler	&waterSampler()	{ return mWaterSampler; }

private:
	void			writeFPSToTextBuffer();
	void			draw();

	// keyboard callback
	virtual void		onKeyDown(VUUINT32 key);

	bool			mbShowFPS;
	float			mFPSHistory[FPS_HISTORY_COUNT];
	int				mFPSHistoryIndex;
	double			mFPSLastFrameTime;
	char			mFPSTextBuffer[32];

	float			mSimStartTime;
	float			mSimTotalTime;
	float			mGfxStartTime;
	float			mGfxTotalTime;
	float			mDynStartTime;
	float			mDynTotalTime;
	float			mDynOverlapTime;
	float			mWaterStartTime;
	float			mWaterTotalTime;
	float			mWaterOverlapTime;
	float			mFrameStartTime;
	float			mFrameTotalTime;

	bool			mbShowPerfBars;
	bool			mbDumpResults;

	char			mGfxTextBufferShadow[TEXT_BUFFER_SIZE];
	char			mDynTextBufferShadow[TEXT_BUFFER_SIZE];
	char			mWaterTextBufferShadow[TEXT_BUFFER_SIZE];

	enum eState { STATE_OFF, STATE_PERF, STATE_SIM, STATE_GFX, STATE_DYN, STATE_WATER, STATE_COUNT };
	eState			mState;

	VuDevProfileSampler	mSimSampler;
	VuDevProfileSampler	mGfxSampler;
	VuDevProfileSampler	mDynSampler;
	VuDevProfileSampler	mWaterSampler;
};


// the interface
IMPLEMENT_SYSTEM_COMPONENT(VuDevProfile, VuDevProfileImpl);


//*****************************************************************************
VuDevProfileImpl::VuDevProfileImpl():
	mbShowFPS(false),
	mFPSHistoryIndex(0),
	mSimStartTime(0.0f),
	mSimTotalTime(0.0f),
	mGfxStartTime(0.0f),
	mGfxTotalTime(0.0f),
	mDynStartTime(0.0f),
	mDynTotalTime(0.0f),
	mDynOverlapTime(0.0f),
	mWaterStartTime(0.0f),
	mWaterTotalTime(0.0f),
	mWaterOverlapTime(0.0f),
	mFrameStartTime(0.0f),
	mFrameTotalTime(0.0f),
	mbShowPerfBars(false),
	mbDumpResults(false),
	mState(STATE_OFF)
{
	mFPSTextBuffer[0] = '\0';

	VuKeyboard::IF()->addCallback(this);

	// start rendering
	VuDrawManager::IF()->registerHandler(this, &VuDevProfileImpl::draw);

	memset(mFPSHistory, 0, sizeof(mFPSHistory));
	mFPSLastFrameTime = VuSys::IF()->getTime();

	mSimSampler.setHeader("Simulation");
	mGfxSampler.setHeader("Graphics");
	mDynSampler.setHeader("Dynamics");
	mWaterSampler.setHeader("Water Renderer");
}

//*****************************************************************************
VuDevProfileImpl::~VuDevProfileImpl()
{
	// stop rendering
	VuDrawManager::IF()->unregisterHandler(this);

	VuKeyboard::IF()->removeCallback(this);
}

//*****************************************************************************
bool VuDevProfileImpl::init()
{
	if ( VuDevConfig::IF() )
		VuDevConfig::IF()->getParam("ShowFPS").getValue(mbShowFPS);

	return true;
}

//*****************************************************************************
void VuDevProfileImpl::beginSim()
{
	mSimStartTime = (float)VuSys::IF()->getTime();
	mSimSampler.begin("Root");
}

//*****************************************************************************
void VuDevProfileImpl::endSim()
{
	// update FPS
	double curTime = VuSys::IF()->getTime();
	mFPSHistory[mFPSHistoryIndex] = (float)(curTime - mFPSLastFrameTime);
	mFPSHistoryIndex = (mFPSHistoryIndex + 1)%FPS_HISTORY_COUNT;
	mFPSLastFrameTime = curTime;

	if ( mbShowFPS )
	{
		// write FPS to text buffer
		writeFPSToTextBuffer();
	}

	mSimTotalTime = (float)curTime - mSimStartTime;

	mSimSampler.end();
	mSimSampler.update();
}

//*****************************************************************************
void VuDevProfileImpl::beginGfx()
{
	mGfxStartTime = (float)VuSys::IF()->getTime();
	mGfxSampler.begin("Root");
}

//*****************************************************************************
void VuDevProfileImpl::endGfx()
{
	mGfxTotalTime = (float)VuSys::IF()->getTime() - mGfxStartTime;

	mGfxSampler.end();
	mGfxSampler.update();
}

//*****************************************************************************
void VuDevProfileImpl::synchronizeGfx()
{
	VU_MEMCPY(mGfxTextBufferShadow, TEXT_BUFFER_SIZE, mGfxSampler.textBuffer(), TEXT_BUFFER_SIZE);

	float curTime = (float)VuSys::IF()->getTime();
	mFrameTotalTime = curTime - mFrameStartTime;
	mFrameStartTime = curTime;
}

//*****************************************************************************
void VuDevProfileImpl::beginDyn(float overlapTime)
{
	mDynOverlapTime = overlapTime;
	mDynStartTime = (float)VuSys::IF()->getTime();
	mDynSampler.begin("Root");
}

//*****************************************************************************
void VuDevProfileImpl::endDyn()
{
	mDynTotalTime = (float)VuSys::IF()->getTime() - mDynStartTime;

	mDynSampler.end();
	mDynSampler.update();
}

//*****************************************************************************
void VuDevProfileImpl::synchronizeDyn()
{
	VU_MEMCPY(mDynTextBufferShadow, TEXT_BUFFER_SIZE, mDynSampler.textBuffer(), TEXT_BUFFER_SIZE);
}

//*****************************************************************************
void VuDevProfileImpl::beginWater(float overlapTime)
{
	mWaterOverlapTime = overlapTime;
	mWaterStartTime = (float)VuSys::IF()->getTime();
	mWaterSampler.begin("Root");
}

//*****************************************************************************
void VuDevProfileImpl::endWater()
{
	mWaterTotalTime = (float)VuSys::IF()->getTime() - mWaterStartTime;

	mWaterSampler.end();
	mWaterSampler.update();
}

//*****************************************************************************
void VuDevProfileImpl::synchronizeWater()
{
	VU_MEMCPY(mWaterTextBufferShadow, TEXT_BUFFER_SIZE, mWaterSampler.textBuffer(), TEXT_BUFFER_SIZE);
}

//*****************************************************************************
void VuDevProfileImpl::draw()
{
	VuGfxSort::IF()->setFullScreenLayer(VuGfxSort::FSL_DEV_PROFILE);

	VuGfxUtil *pGfxUtil = VuGfxUtil::IF();
	VuFontDraw *pFontDraw = pGfxUtil->fontDraw();
	VuFont *pFont = VuDev::IF()->getFont();

	VuFontDrawParams params;
	float textHeight = params.mSize/FONT_DRAW_SCALE_Y;

	if ( mbShowFPS )
	{
		VuRect rect(0.05f, 0.05f, 0.0f, 0.0f);

		VuFontDrawParams fpsParams;
		fpsParams.mSize = 36;
		fpsParams.mWeight = 120;
		fpsParams.mOutlineWeight = 5;

		pFontDraw->drawString(0, pFont, mFPSTextBuffer, fpsParams, rect, 0);
	}
	if ( mbShowPerfBars )
	{
		VuRect rect(0.05f, 0.05f + textHeight, 0.9f, 0.12f);

		// draw background
		pGfxUtil->drawFilledRectangle2d(GFX_SORT_DEPTH_STEP, VuColor(0, 0, 0, 128), rect);

		rect.mX += 0.06f;
		rect.mY += 0.01f;
		rect.mWidth -= 0.07f;
		rect.mHeight -= 0.02f;

		// draw bars
		{
			VuRect totalBar(rect.mX, rect.mY + 0.01f, rect.mWidth*15.0f*mFrameTotalTime, 0.01f);
			pFontDraw->drawString(0, pFont, "Total", params, totalBar - VuVector2(0.05f, 0.0f), VUGFX_TEXT_DRAW_VCENTER);
			pGfxUtil->drawFilledRectangle2d(0, VuColor(255,128,128), totalBar);

			VuRect simBar(rect.mX, rect.mY + 0.03f, rect.mWidth*15.0f*mSimTotalTime, 0.01f);
			pFontDraw->drawString(0, pFont, "Main", params, simBar - VuVector2(0.05f, 0.0f), VUGFX_TEXT_DRAW_VCENTER);
			pGfxUtil->drawFilledRectangle2d(0, VuColor(128,255,128), simBar);

			VuRect gfxBar(rect.mX, rect.mY + 0.05f, rect.mWidth*15.0f*mGfxTotalTime, 0.01f);
			pFontDraw->drawString(0, pFont, "Gfx", params, gfxBar - VuVector2(0.05f, 0.0f), VUGFX_TEXT_DRAW_VCENTER);
			pGfxUtil->drawFilledRectangle2d(0, VuColor(128,128,255), gfxBar);

			VuRect dynBar(rect.mX, rect.mY + 0.07f, rect.mWidth*15.0f*mDynTotalTime, 0.01f);
			pFontDraw->drawString(0, pFont, "Dyn", params, dynBar - VuVector2(0.05f, 0.0f), VUGFX_TEXT_DRAW_VCENTER);
			dynBar.mX += rect.mWidth*15.0f*mDynOverlapTime;
			pGfxUtil->drawFilledRectangle2d(0, VuColor(255,255,128), dynBar);

			VuRect waterBar(rect.mX, rect.mY + 0.09f, rect.mWidth*15.0f*mWaterTotalTime, 0.01f);
			pFontDraw->drawString(0, pFont, "Water", params, waterBar - VuVector2(0.05f, 0.0f), VUGFX_TEXT_DRAW_VCENTER);
			waterBar.mX += rect.mWidth*15.0f*mWaterOverlapTime;
			pGfxUtil->drawFilledRectangle2d(0, VuColor(128,255,255), waterBar);
		}

		// draw tick marks
		{
			VuVector2 verts[10];
			for ( int i = 0; i < 5; i++ )
			{
				float x = rect.getLeft() + i*rect.getWidth()/4;
				verts[i*2+0] = VuVector2(x, rect.getTop());
				verts[i*2+1] = VuVector2(x, rect.getBottom());
			}
			pGfxUtil->drawLines2d(0, VUGFX_PT_LINELIST, VuColor(128,128,128), verts, 10);
		}

	}
	if ( mSimSampler.isEnabled() || mGfxSampler.isEnabled() || mDynSampler.isEnabled() || mWaterSampler.isEnabled() )
	{
		VuRect rect(0.05f, 0.05f + textHeight, 0.9f, 0.9f - textHeight);

		// draw background
		pGfxUtil->drawFilledRectangle2d(GFX_SORT_DEPTH_STEP, VuColor(0, 0, 0, 128), rect);

		const char *strText = "";
		if ( mSimSampler.isEnabled() )
			strText = mSimSampler.textBuffer();
		if ( mGfxSampler.isEnabled() )
			strText = mGfxTextBufferShadow;
		if ( mDynSampler.isEnabled() )
			strText = mDynTextBufferShadow;
		if ( mWaterSampler.isEnabled() )
			strText = mWaterTextBufferShadow;

		// draw text buffer
		pFontDraw->drawString(0, pFont, strText, params, rect, 0);

		if ( mbDumpResults )
		{
			VUPRINT(strText);
			mbDumpResults = false;
		}
	}
}

//*****************************************************************************
void VuDevProfileImpl::writeFPSToTextBuffer()
{
	// calculate average
	float fAvgTime = 0;
	float fHigh = 0;
	float fLow = FLT_MAX;
	for ( int i = 0; i < FPS_HISTORY_COUNT; i++ )
	{
		VuMinMax(mFPSHistory[i], fLow, fHigh);
		fAvgTime += mFPSHistory[i];
	}
	fAvgTime -= fLow;
	fAvgTime -= fHigh;
	fAvgTime /= (FPS_HISTORY_COUNT - 2);

	VU_SPRINTF(mFPSTextBuffer, sizeof(mFPSTextBuffer), "%3d FPS\n", VuRound(1.0f/fAvgTime));
}

//*****************************************************************************
void VuDevProfileImpl::onKeyDown(VUUINT32 key)
{
	if ( key == VUKEY_BACK )
	{
		mState = (eState)((mState + 1)%STATE_COUNT);
		mSimSampler.setEnabled(mState == STATE_SIM);
		mGfxSampler.setEnabled(mState == STATE_GFX);
		mDynSampler.setEnabled(mState == STATE_DYN);
		mWaterSampler.setEnabled(mState == STATE_WATER);
		mbShowPerfBars = (mState == STATE_PERF);
	}
	else if ( key == VUKEY_TAB )
	{
		mbShowFPS = !mbShowFPS;
	}
	else if ( key == VUKEY_R )
	{
		mSimSampler.reset();
		mGfxSampler.reset();
		mDynSampler.reset();
		mWaterSampler.reset();
	}
	else if ( key == VUKEY_M )
	{
		mbDumpResults = true;
	}
}

//*****************************************************************************
VuDevProfileSampler::VuDevProfileSampler():
	mbEnabled(false),
	mbEnabledPending(false),
	mbResetPending(false),
	mSampleCount(0),
	mpCurSample(VUNULL),
	mSampleHistoryIndex(0)
{
}

//*****************************************************************************
void VuDevProfileSampler::begin(const char *strSampleName)
{
	if ( !mbEnabled )
	{
		mSampleCount = 0;
		return;
	}

	VUUINT32 hashedName = VuHash::fnv32String(strSampleName);

	// try to find sample
	Sample *pSample = VUNULL;
	Sample *pPrevSample = VUNULL;
	if ( mpCurSample )
	{
		// search children of current sample
		for ( pSample = mpCurSample->mpFirstChild; pSample; pSample = pSample->mpNextSibling )
		{
			if ( pSample->mHashedName == hashedName )
				break;
			pPrevSample = pSample;
		}
	}
	else if ( mSampleCount )
	{
		pSample = maSamples;
	}

	if ( pSample == VUNULL )
	{
		// new sample
		VUASSERT(mSampleCount < MAX_SAMPLE_COUNT, "VuDevProfileImpl::begin() out of samples");
		pSample = &maSamples[mSampleCount++];
		memset(pSample, 0, sizeof(*pSample));
		VU_STRNCPY(pSample->mstrName, MAX_SAMPLE_STRING_SIZE, strSampleName, MAX_SAMPLE_STRING_SIZE - 1);
		pSample->mHashedName = hashedName;
		pSample->mpParent = mpCurSample;
		pSample->mMinTime = FLT_MAX;

		if ( mpCurSample )
		{
			if ( pPrevSample )
				pPrevSample->mpNextSibling = pSample;
			else
				mpCurSample->mpFirstChild = pSample;
		}
	}

	pSample->mBeginTime = VuSys::IF()->getTime();

	mpCurSample = pSample;
}

//*****************************************************************************
void VuDevProfileSampler::end()
{
	if ( !mbEnabled )
		return;

	VUASSERT(mpCurSample, "VuDevProfileImpl::end() begin/end mismatch");

	// accumulate
	mpCurSample->mAccumulatedTime += (float)(VuSys::IF()->getTime() - mpCurSample->mBeginTime);
	mpCurSample->mCount++;

	mpCurSample = mpCurSample->mpParent;
}

//*****************************************************************************
void VuDevProfileSampler::update()
{
	if ( mbEnabled && mpCurSample == VUNULL )
	{
		// update min/max/history
		for ( int i = 0; i < mSampleCount; i++ )
		{
			maSamples[i].mMinTime = VuMin(maSamples[i].mMinTime, maSamples[i].mAccumulatedTime);
			maSamples[i].mMaxTime = VuMax(maSamples[i].mMaxTime, maSamples[i].mAccumulatedTime);
			maSamples[i].mHistory[mSampleHistoryIndex] = maSamples[i].mAccumulatedTime;
		}

		writeSamplesToTextBuffer();

		// reset samples
		for ( int i = 0; i < mSampleCount; i++ )
		{
			maSamples[i].mCount = 0;
			maSamples[i].mAccumulatedTime = 0;
		}

		mSampleHistoryIndex = (mSampleHistoryIndex + 1)%SAMPLE_HISTORY_COUNT;
	}

	mbEnabled = mbEnabledPending;

	if ( mbResetPending )
	{
		mSampleCount = 0;
		mbResetPending = false;
	}
}

//*****************************************************************************
void VuDevProfileSampler::writeSamplesToTextBuffer()
{
	// clear text buffer
	mTextBuffer[0] = '\0';

	// header
	VU_STRCAT(mTextBuffer, TEXT_BUFFER_SIZE, mHeader.c_str());
	VU_STRCAT(mTextBuffer, TEXT_BUFFER_SIZE, "\n");
	VU_STRCAT(mTextBuffer, TEXT_BUFFER_SIZE, "Cur      | Min      | Max          | Avg      | Cnt  | Name\n");
	VU_STRCAT(mTextBuffer, TEXT_BUFFER_SIZE, "---------+----------+--------------+----------+------+--------------------\n");

	Sample *pSample = maSamples;
	int depth = 0;

	while ( pSample )
	{
		// calculate average
		float fAvgTime = 0;
		float fHigh = 0;
		float fLow = FLT_MAX;
		for ( int i = 0; i < SAMPLE_HISTORY_COUNT; i++ )
		{
			VuMinMax(pSample->mHistory[i], fLow, fHigh);
			fAvgTime += pSample->mHistory[i];
		}
		fAvgTime -= fLow;
		fAvgTime -= fHigh;
		fAvgTime /= (SAMPLE_HISTORY_COUNT - 2);

		char str[256];
		VU_SPRINTF(str, sizeof(str), "%8.5f | %8.5f | %12.5f | %8.5f | %4d | ", pSample->mAccumulatedTime*1000.0f, pSample->mMinTime*1000.0f, pSample->mMaxTime*1000.0f, fAvgTime*1000.0f, pSample->mCount);
		for ( int i = 0; i < depth; i++ )
			VU_STRCAT(str, sizeof(str), " ");
		VU_STRCAT(str, sizeof(str), pSample->mstrName);
		VU_STRCAT(str, sizeof(str), "\n");

		VU_STRCAT(mTextBuffer, TEXT_BUFFER_SIZE, str);

		Sample *pNextSample = VUNULL;
		if ( pSample->mpFirstChild )
		{
			pNextSample = pSample->mpFirstChild;
			depth++;
		}
		else if ( pSample->mpNextSibling )
		{
			pNextSample = pSample->mpNextSibling;
		}
		else
		{
			for ( pSample = pSample->mpParent; pSample && !pNextSample; pSample = pSample->mpParent )
			{
				pNextSample = pSample->mpNextSibling;
				depth--;
			}
		}

		pSample = pNextSample;
	}
}

//*****************************************************************************
VuDevProfileSimSample::VuDevProfileSimSample(const char *strName)
{
	if ( VuDevProfileImpl::IF() )
		VuDevProfileImpl::IF()->simSampler().begin(strName);
}

//*****************************************************************************
VuDevProfileSimSample::~VuDevProfileSimSample()
{
	if ( VuDevProfileImpl::IF() )
		VuDevProfileImpl::IF()->simSampler().end();
}

//*****************************************************************************
VuDevProfileGfxSample::VuDevProfileGfxSample(const char *strName)
{
	if ( VuDevProfileImpl::IF() )
		VuDevProfileImpl::IF()->gfxSampler().begin(strName);
}

//*****************************************************************************
VuDevProfileGfxSample::~VuDevProfileGfxSample()
{
	if ( VuDevProfileImpl::IF() )
		VuDevProfileImpl::IF()->gfxSampler().end();
}

//*****************************************************************************
VuDevProfileDynSample::VuDevProfileDynSample(const char *strName)
{
	if ( VuDevProfileImpl::IF() )
		VuDevProfileImpl::IF()->dynSampler().begin(strName);
}

//*****************************************************************************
VuDevProfileDynSample::~VuDevProfileDynSample()
{
	if ( VuDevProfileImpl::IF() )
		VuDevProfileImpl::IF()->dynSampler().end();
}

//*****************************************************************************
VuDevProfileWaterSample::VuDevProfileWaterSample(const char *strName)
{
	if ( VuDevProfileImpl::IF() )
		VuDevProfileImpl::IF()->waterSampler().begin(strName);
}

//*****************************************************************************
VuDevProfileWaterSample::~VuDevProfileWaterSample()
{
	if ( VuDevProfileImpl::IF() )
		VuDevProfileImpl::IF()->waterSampler().end();
}

#endif  // VU_DISABLE_DEV_PROFILE
