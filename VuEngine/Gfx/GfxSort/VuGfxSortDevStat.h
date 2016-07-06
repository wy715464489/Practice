//*****************************************************************************
//
//  Copyright (c) 2008-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  GfxSortDevStat class
// 
//*****************************************************************************

#pragma once

#include "VuEngine/HAL/Keyboard/VuKeyboard.h"
#include "VuEngine/Containers/VuArray.h"

class VuDevStatPage;


class VuGfxSortDevStat : public VuKeyboard::Callback
{
public:
	VuGfxSortDevStat();
	virtual ~VuGfxSortDevStat();	// some compilers warn about deleting base class objects with a non-virtual destructor

	// keyboard callback
	virtual void	onKeyDown(VUUINT32 key);

	void			enable(bool bEnable)	{ mbEnabled = bEnable; }
	void			print(VuDevStatPage *pPage, const VuArray<VUUINT64> &sortedKeys);

private:
	void			rebuildChoices();

	enum eTier { FSL, VP, RL, VPL, TT, CMD, TIER_COUNT };

	struct Tier
	{
		Tier(int maxSelection, int shift, VUUINT64 mask, const char **pTextOptions) :
			mCurFilter(0), mCurMask(0), mCurSelection(0), mMaxSelection(maxSelection + 1), mShift(shift), mMask(mask), mpTextOptions(pTextOptions)
			{ mCurText[0] = '\0'; }

		VUUINT64	mCurFilter;
		VUUINT64	mCurMask;
		char		mCurText[256];
		int			mCurSelection;
		int			mMaxSelection;
		int			mShift;
		VUUINT64	mMask;
		const char	**mpTextOptions;
	};
	typedef VuArray<Tier> Tiers;

	struct Choice
	{
		VUUINT64	mFilter;
		VUUINT64	mMask;
		char		mText[256];
	};
	typedef VuArray<Choice> Choices;

	bool			mbEnabled;
	int				mCurTier;
	Tiers			mTiers;
	Choices			mChoices;
	char			mHeaderText[256];
};
