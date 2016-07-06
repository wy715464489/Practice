//*****************************************************************************
//
//  Copyright (c) 2011-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  Anchor utility.
// 
//*****************************************************************************

#pragma once

class VuRect;
class VuVector2;


class VuUIAnchor
{
public:
	enum eAlign { ANCHOR_NONE, ANCHOR_LEFT, ANCHOR_RIGHT, ANCHOR_LEFT_RIGHT, ANCHOR_TOP, ANCHOR_BOTTOM, ANCHOR_TOP_BOTTOM };

	VuUIAnchor(eAlign alignH, eAlign alignV, float ratioH = 1.0f, float ratioV = 1.0f);
	VuUIAnchor();

	void	apply(const VuRect &in, VuRect &out) const;
	void	apply(const VuVector2 &in, VuVector2 &out) const;
	void	unapply(const VuVector2 &in, VuVector2 &out) const;

	int		mAnchorH;	// ANCHOR_NONE, ANCHOR_LEFT, ANCHOR_RIGHT
	int		mAnchorV;	// ANCHOR_NONE, ANCHOR_TOP, ANCHOR_BOTTOM
	float	mRatioH;
	float	mRatioV;
};
