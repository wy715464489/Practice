//*****************************************************************************
//
//  Copyright (c) 2011-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  Anchor utility.
// 
//*****************************************************************************

#include "VuUIAnchor.h"
#include "VuEngine/UI/VuUI.h"
#include "VuEngine/Math/VuRect.h"
#include "VuEngine/Math/VuMatrix.h"


//*****************************************************************************
VuUIAnchor::VuUIAnchor(eAlign alignH, eAlign alignV, float ratioH, float ratioV):
	mAnchorH(alignH),
	mAnchorV(alignV),
	mRatioH(ratioH),
	mRatioV(ratioV)
{
}

//*****************************************************************************
VuUIAnchor::VuUIAnchor():
	mAnchorH(ANCHOR_NONE),
	mAnchorV(ANCHOR_NONE),
	mRatioH(1.0f),
	mRatioV(1.0f)
{
}

//*****************************************************************************
void VuUIAnchor::apply(const VuRect &in, VuRect &out) const
{
	const VuMatrix &cropMatrix = VuUI::IF()->getCropMatrix();

	out = in;

	if ( mAnchorH == ANCHOR_LEFT )
	{
		out.mX -= mRatioH*cropMatrix.mT.mX/cropMatrix.mX.mX;
	}
	if ( mAnchorH == ANCHOR_RIGHT )
	{
		out.mX += mRatioH*(1.0f - cropMatrix.mT.mX - cropMatrix.mX.mX)/cropMatrix.mX.mX;
	}
	if ( mAnchorH == ANCHOR_LEFT_RIGHT )
	{
		out.mX -= mRatioH*cropMatrix.mT.mX/cropMatrix.mX.mX;
		out.mWidth += mRatioH*(1.0f - cropMatrix.mX.mX)/cropMatrix.mX.mX;
	}

	if ( mAnchorV == ANCHOR_TOP )
	{
		out.mY -= mRatioV*cropMatrix.mT.mY/cropMatrix.mY.mY;
	}
	if ( mAnchorV == ANCHOR_BOTTOM )
	{
		out.mY += mRatioV*(1.0f - cropMatrix.mT.mY - cropMatrix.mY.mY)/cropMatrix.mY.mY;
	}
	if ( mAnchorV == ANCHOR_TOP_BOTTOM )
	{
		out.mY -= mRatioV*cropMatrix.mT.mY/cropMatrix.mY.mY;
		out.mHeight += mRatioV*(1.0f - cropMatrix.mY.mY)/cropMatrix.mY.mY;
	}
}

//*****************************************************************************
void VuUIAnchor::apply(const VuVector2 &in, VuVector2 &out) const
{
	const VuMatrix &cropMatrix = VuUI::IF()->getCropMatrix();

	out = in;

	if ( mAnchorH == ANCHOR_LEFT )
	{
		out.mX -= mRatioH*cropMatrix.mT.mX/cropMatrix.mX.mX;
	}
	if ( mAnchorH == ANCHOR_RIGHT )
	{
		out.mX += mRatioH*(1.0f - cropMatrix.mT.mX - cropMatrix.mX.mX)/cropMatrix.mX.mX;
	}

	if ( mAnchorV == ANCHOR_TOP )
	{
		out.mY -= mRatioV*cropMatrix.mT.mY/cropMatrix.mY.mY;
	}
	if ( mAnchorV == ANCHOR_BOTTOM )
	{
		out.mY += mRatioV*(1.0f - cropMatrix.mT.mY - cropMatrix.mY.mY)/cropMatrix.mY.mY;
	}
}

//*****************************************************************************
void VuUIAnchor::unapply(const VuVector2 &in, VuVector2 &out) const
{
	const VuMatrix &cropMatrix = VuUI::IF()->getCropMatrix();

	out = in;

	if ( mAnchorH == ANCHOR_LEFT )
	{
		out.mX += mRatioH*cropMatrix.mT.mX/cropMatrix.mX.mX;
	}
	if ( mAnchorH == ANCHOR_RIGHT )
	{
		out.mX -= mRatioH*(1.0f - cropMatrix.mT.mX - cropMatrix.mX.mX)/cropMatrix.mX.mX;
	}

	if ( mAnchorV == ANCHOR_TOP )
	{
		out.mY += mRatioV*cropMatrix.mT.mY/cropMatrix.mY.mY;
	}
	if ( mAnchorV == ANCHOR_BOTTOM )
	{
		out.mY -= mRatioV*(1.0f - cropMatrix.mT.mY - cropMatrix.mY.mY)/cropMatrix.mY.mY;
	}
}
