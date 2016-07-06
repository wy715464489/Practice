//*****************************************************************************
//
//  Copyright (c) 2008-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  GfxSortDevStat class
// 
//*****************************************************************************

#include "VuGfxSortDevStat.h"
#include "VuGfxSort.h"
#include "VuEngine/Dev/VuDevStat.h"


// static strings used for debug printing

static const char *sFullScreenLayerNames[] =
{
	"Begin     ",	// FSL_BEGIN,
	"Game      ",	// FSL_GAME,
	"Effects   ",	// FSL_EFFECTS,
	"HUD       ",	// FSL_HUD,
	"UI        ",	// FSL_UI,
	"Fade      ",	// FSL_FADE,
	"Dialog    ",	// FSL_DIALOG,
	"MessageBox",	// FSL_MESSAGE_BOX,
	"Toast     ",	// FSL_TOAST,
	"WaterDebug",	// FSL_WATER_DEBUG,
	"DevConsole",	// FSL_DEV_CONSOLE,
	"DevStat   ",	// FSL_DEV_STAT,
	"DevProfile",	// FSL_DEV_PROFILE,
	"DevMenu   ",	// FSL_DEV_MENU,
	"End       "	// Tier
};
VU_COMPILE_TIME_ASSERT(sizeof(sFullScreenLayerNames)/sizeof(sFullScreenLayerNames[0]) == VuGfxSort::FSL_END + 1);

static const char *sViewportNames[] =
{
	"Viewport 0",
	"Viewport 1",
	"Viewport 2",
	"Viewport 3",
	"Viewport 4",
	"Viewport 5",
	"Viewport 6",
	"Viewport 7",
};
VU_COMPILE_TIME_ASSERT(sizeof(sViewportNames)/sizeof(sViewportNames[0]) == 1<<GFX_SORT_VIEWPORT_BITS);

static const char *sReflectionLayerNames[] =
{
	"On ",		// REFLECTION_ON,
	"Off",		// REFLECTION_OFF,
};
VU_COMPILE_TIME_ASSERT(sizeof(sReflectionLayerNames)/sizeof(sReflectionLayerNames[0]) == 1<<GFX_SORT_REFLECTION_LAYER_BITS);

static const char *sViewportLayerNames[] =
{
	"Begin   ",	// VPL_BEGIN,
	"Shadow 1",	// VPL_SHADOW1,
	"Shadow 2",	// VPL_SHADOW2,
	"Shadow 3",	// VPL_SHADOW3,
	"Shadow 4",	// VPL_SHADOW4,
	"SSAO    ",	// VPL_SSAO,
	"Depth   ",	// VPL_DEPTH,
	"SkyBox  ",	// VPL_SKYBOX,
	"World   ",	// VPL_WORLD,
	"HUD     ",	// VPL_HUD
	"UI      ",	// VPL_UI
	"End     ",	// VPL_END
};
VU_COMPILE_TIME_ASSERT(sizeof(sViewportLayerNames)/sizeof(sViewportLayerNames[0]) == VuGfxSort::VPL_END + 1);

static const char *sTranslucencyTypeNames[] =
{
	"Begin             ",	// TRANS_BEGIN,
	"Opaque            ",	// TRANS_OPAQUE,
	"OneBitAlpha       ",	// TRANS_ALPHA_TEST,
	"Foliage           ",	// TRANS_FOLIAGE,
	"Skybox            ",	// TRANS_SKYBOX,
	"TireTrack         ",	// TRANS_TIRE_TRACK,
	"BlobShadow        ",	// TRANS_BLOB_SHADOW,
	"ModulateBelowWater",	// TRANS_MODULATE_BELOW_WATER,
	"AdditiveBelowWater",	// TRANS_ADDITIVE_BELOW_WATER,
	"WaterColor        ",	// TRANS_WATER_COLOR,
	"DepthPass         ",	// TRANS_DEPTH_PASS,
	"ColorPass         ",	// TRANS_COLOR_PASS,
	"ModulateAboveWater",	// TRANS_MODULATE_ABOVE_WATER,
	"AdditiveAboveWater",	// TRANS_ADDITIVE_ABOVE_WATER,
	"WaterDepth        ",	// TRANS_WATER_DEPTH,
	"ModulateClipWater ",	// TRANS_MODULATE_CLIP_WATER,
	"AdditiveClipWater ",	// TRANS_ADDITIVE_CLIP_WATER,
	"UiOpaque          ",	// TRANS_UI_OPAQUE,
	"UiModulate        ",	// TRANS_UI_MODULATE,
	"UiAdditive        ",	// TRANS_UI_ADDITIVE,
	"End               ",	// TRANS_END,
};
VU_COMPILE_TIME_ASSERT(sizeof(sTranslucencyTypeNames)/sizeof(sTranslucencyTypeNames[0]) == VuGfxSort::TRANS_END + 1);

static const char *sCommandNames[] =
{
	"Command",
	"Draw   ",
};
VU_COMPILE_TIME_ASSERT(sizeof(sCommandNames)/sizeof(sCommandNames[0]) == 2);


//*****************************************************************************
VuGfxSortDevStat::VuGfxSortDevStat():
	mbEnabled(false),
	mCurTier(FSL)
{
	// built tiers
	mTiers.reserve(6);
	mTiers.push_back(Tier(       VuGfxSort::FSL_END,   GFX_SORT_FULLSCREEN_LAYER_SHIFT,   GFX_SORT_FULLSCREEN_LAYER_MASK,  sFullScreenLayerNames));
	mTiers.push_back(Tier(                        3,           GFX_SORT_VIEWPORT_SHIFT,           GFX_SORT_VIEWPORT_MASK,         sViewportNames));
	mTiers.push_back(Tier(VuGfxSort::REFLECTION_OFF,   GFX_SORT_REFLECTION_LAYER_SHIFT,   GFX_SORT_REFLECTION_LAYER_MASK,  sReflectionLayerNames));
	mTiers.push_back(Tier(       VuGfxSort::VPL_END,     GFX_SORT_VIEWPORT_LAYER_SHIFT,     GFX_SORT_VIEWPORT_LAYER_MASK,    sViewportLayerNames));
	mTiers.push_back(Tier(     VuGfxSort::TRANS_END, GFX_SORT_TRANSLUCENCY_LAYER_SHIFT, GFX_SORT_TRANSLUCENCY_LAYER_MASK, sTranslucencyTypeNames));
	mTiers.push_back(Tier(                        1,            GFX_SORT_COMMAND_SHIFT,            GFX_SORT_COMMAND_MASK,          sCommandNames));

	// reserve choice memory
	mChoices.reserve(32);

	rebuildChoices();

	VuKeyboard::IF()->addCallback(this);
}

//*****************************************************************************
VuGfxSortDevStat::~VuGfxSortDevStat()
{
	VuKeyboard::IF()->removeCallback(this);
}

//*****************************************************************************
void VuGfxSortDevStat::onKeyDown(VUUINT32 key)
{
	if ( !mbEnabled )
		return;

	if ( VuKeyboard::IF()->isKeyDown(VUKEY_SHIFT) )
	{
		if ( key == VUKEY_LEFT )
			mCurTier--;
		else if ( key == VUKEY_RIGHT )
			mCurTier++;
		else if ( key == VUKEY_UP )
			mTiers[mCurTier].mCurSelection--;
		else if ( key == VUKEY_DOWN )
			mTiers[mCurTier].mCurSelection++;

		mCurTier = VuClamp(mCurTier, 0, TIER_COUNT - 1);

		Tier *pCurTier = &mTiers[mCurTier];
		pCurTier->mCurSelection = VuClamp(pCurTier->mCurSelection, 0, pCurTier->mMaxSelection);


		// rebuild choices
		rebuildChoices();
	}
}

//*****************************************************************************
void VuGfxSortDevStat::print(VuDevStatPage *pPage, const VuArray<VUUINT64> &sortedKeys)
{
	pPage->printf("\n");
	pPage->printf(mHeaderText);

	for ( int i = 0; i < mChoices.size(); i++ )
	{
		const Choice &choice = mChoices[i];

		VUUINT64 filter = choice.mFilter;
		VUUINT64 mask = choice.mMask;

		// count # of commands matching this filter/mask
		int count = 0;
		for ( const VUUINT64 *pKey = &sortedKeys.begin(); pKey != &sortedKeys.end(); pKey++ )
			if ( ((*pKey) & mask) == filter )
				count++;

		Tier *pCurTier = &mTiers[mCurTier];
		if ( i == pCurTier->mCurSelection )
			pPage->printf("{[255,255,0]}");
		else if ( i == 0 )
			pPage->printf("{[128,128,128]}");

		pPage->printf("%s%6d\n", choice.mText, count);
		pPage->printf("{[]}");
	}
}

//*****************************************************************************
void VuGfxSortDevStat::rebuildChoices()
{
	mChoices.clear();

	Tier *pCurTier = &mTiers[mCurTier];

	// add choice for 'all'
	{
		Choice choice;
		VU_STRCPY(choice.mText, sizeof(choice.mText), mCurTier ? mTiers[mCurTier-1].mCurText : "");
		VU_STRCAT(choice.mText, sizeof(choice.mText), " All");
		int spaces = (int)strlen(pCurTier->mpTextOptions[0]) - 3;
		for ( int i = 0; i < spaces; i++ )
			VU_STRCAT(choice.mText, sizeof(choice.mText), " ");
		VU_STRCAT(choice.mText, sizeof(choice.mText), " |");
		choice.mMask = mCurTier ? mTiers[mCurTier-1].mCurMask : 0;
		choice.mFilter = mCurTier ? mTiers[mCurTier-1].mCurFilter : 0;
		mChoices.push_back(choice);
	}

	for ( int i = 0; i < mTiers[mCurTier].mMaxSelection; i++ )
	{
		Choice choice;

		// build string
		VU_STRCPY(choice.mText, sizeof(choice.mText), mCurTier ? mTiers[mCurTier-1].mCurText : "");

		const char *str = pCurTier->mpTextOptions[i];
		VU_STRCAT(choice.mText, sizeof(choice.mText), " ");
		VU_STRCAT(choice.mText, sizeof(choice.mText), str);
		VU_STRCAT(choice.mText, sizeof(choice.mText), " |");

		// rebuild mask
		choice.mMask = mCurTier ? mTiers[mCurTier-1].mCurMask : 0;
		choice.mMask |= pCurTier->mMask;

		// rebuild filter
		choice.mFilter = mCurTier ? mTiers[mCurTier-1].mCurFilter : 0;
		choice.mFilter |= (VUUINT64)i << pCurTier->mShift;

		// add choice
		mChoices.push_back(choice);
	}

	pCurTier->mCurFilter = mChoices[pCurTier->mCurSelection].mFilter;
	pCurTier->mCurMask = mChoices[pCurTier->mCurSelection].mMask;
	VU_STRCPY(pCurTier->mCurText, sizeof(pCurTier->mCurText), mChoices[pCurTier->mCurSelection].mText);
	
	// rebuild header text
	VU_STRCPY(mHeaderText, sizeof(mHeaderText), "\n FSL        |");
	if ( mCurTier >= VP )	VU_STRCAT(mHeaderText, sizeof(mHeaderText), " Viewport   |");
	if ( mCurTier >= RL )	VU_STRCAT(mHeaderText, sizeof(mHeaderText), " REF |");
	if ( mCurTier >= VPL )	VU_STRCAT(mHeaderText, sizeof(mHeaderText), " VPL      |");
	if ( mCurTier >= TT )	VU_STRCAT(mHeaderText, sizeof(mHeaderText), " Translucency       |");
	if ( mCurTier >= CMD )	VU_STRCAT(mHeaderText, sizeof(mHeaderText), " TYP     |");
	VU_STRCAT(mHeaderText, sizeof(mHeaderText), " Count");
	VU_STRCAT(mHeaderText, sizeof(mHeaderText), "\n------------+");
	if ( mCurTier >= VP )	VU_STRCAT(mHeaderText, sizeof(mHeaderText), "------------+");
	if ( mCurTier >= RL )	VU_STRCAT(mHeaderText, sizeof(mHeaderText), "-----+");
	if ( mCurTier >= VPL )	VU_STRCAT(mHeaderText, sizeof(mHeaderText), "----------+");
	if ( mCurTier >= TT )	VU_STRCAT(mHeaderText, sizeof(mHeaderText), "--------------------+");
	if ( mCurTier >= CMD )	VU_STRCAT(mHeaderText, sizeof(mHeaderText), "---------+");
	VU_STRCAT(mHeaderText, sizeof(mHeaderText), "------\n");
}
