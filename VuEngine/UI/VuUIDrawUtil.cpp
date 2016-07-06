//*****************************************************************************
//
//  Copyright (c) 2008-2008 Ralf Knoesel
//  Copyright (c) 2008-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  UI draw utility functionality.
// 
//*****************************************************************************

#include "VuEngine/UI/VuUI.h"
#include "VuEngine/Components/Transform/VuTransformComponent.h"
#include "VuEngine/Entities/VuEntity.h"
#include "VuEngine/Math/VuMatrix.h"
#include "VuEngine/Gfx/Font/VuFontDraw.h"
#include "VuUIDrawUtil.h"

//*****************************************************************************
void VuUIDrawUtil::getParams(VuEntity *pEntity, VuUIDrawParams &params)
{
	VuTransformComponent *pTC = pEntity->getTransformComponent();

	const VuVector3 &pos = pTC->getWorldPosition();
	const VuVector3 &scale = pTC->getWorldScale();

	const VuVector2 &authScale = VuUI::IF()->getAuthoringScreenScale();

	params.mPosition.mX = pos.mX;
	params.mPosition.mY = pos.mY;
	params.mLocalScale.mX = scale.mX;
	params.mLocalScale.mY = scale.mY;
	params.mAuthScale.mX = authScale.mX;
	params.mAuthScale.mY = authScale.mY;
	params.mInvAuthScale.mX = 1.0f/authScale.mX;
	params.mInvAuthScale.mY = 1.0f/authScale.mY;
	params.mDepth = 0.5f + pos.mZ/200.0f;
}

//*****************************************************************************
bool VuUIDrawUtil::isVisible(const VuRect &rect)
{
	const VuMatrix &invCropMat = VuUI::IF()->getInvCropMatrix();
	VuRect screen(invCropMat.mT.mX, invCropMat.mT.mY, invCropMat.mX.mX, invCropMat.mY.mY);
	return rect.intersects(screen);
}

//*****************************************************************************
void VuUIDrawUtil::shrinkToFit(const char* pText, const VuUIDrawParams& uiParams, const VuFontDB::VuEntry &fontEntry, VuFontDrawParams& fdParams, VuRect& rect)
{
	float origSize = fdParams.mSize;
	float width = VuFontDraw::measureStringWidth(fontEntry.font(), pText, fdParams, VuUI::IF()->getAuthoringAspectRatio());

	// We're trying to fit to a rect, so if the rect doesn't have a width defined, we have nothing to fit into. Ignore.
	if (rect.mWidth > 0)
	{
		// > 1.0 means the text won't fit in the rect
		float fitRatio = width / rect.mWidth;

		if (fitRatio > 1.0f)
		{
			// Shrink the text by the amount required to get it inside the box
			float shrinkRatio = rect.mWidth / width;

			// If getting it inside the box over-shrunk it (made it teensy), clamp
			// the shrinking and use font size scaling the rest of the way
			//
			if (shrinkRatio <= 0.75f)
			{
				// Approximate font size scaling based on the unscaled amount
				// when scaling down very small
				fdParams.mSize -= fdParams.mSize * (0.75f - shrinkRatio);

				// Never scale the font size down more than 75%
				if (fdParams.mSize < (origSize * 0.75f))
				{
					fdParams.mSize = origSize * 0.75f;
				}

				shrinkRatio = 0.75f;
			}

			fdParams.mStretch = shrinkRatio;
		}
	}
}