//*****************************************************************************
//
//  Copyright (c) 2011-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  3dDrawBreakableModelComponent class
// 
//*****************************************************************************

#pragma once

#include "Vu3dDrawComponent.h"
#include "VuEngine/Gfx/Model/VuBreakableModelInstance.h"


class Vu3dDrawBreakableModelComponent : public Vu3dDrawComponent
{
	DECLARE_SHORT_COMPONENT_TYPE(BreakableModel)
	DECLARE_RTTI

public:
	Vu3dDrawBreakableModelComponent(VuEntity *pOwnerEntity);
	~Vu3dDrawBreakableModelComponent();

	virtual void				onPostLoad() { modified(); }
	virtual void				onGameInitialize();
	virtual void				onGameRelease();

	void						startBreak(const VuMatrix &mat, const VuVector3 &vel, const VuColor &color);

	void						tickDecision(float fdt);
	void						tickBuild(float fdt);

	bool						isWhole() { return mState == STATE_WHOLE; }
	bool						isBreaking() { return mState == STATE_BREAKING; }

protected:
	void						draw(const VuGfxDrawParams &params);

	void						modified();

	enum eState { STATE_WHOLE, STATE_BREAKING, STATE_DONE };

	// properties
	std::string					mModelAssetName;
	float						mDrawDist;
	float						mRejectionScaleModifier;
	float						mFadeDelay;
	float						mFadeTime;

	VuBreakableModelInstance	mBreakableModelInstance;
	eState						mState;
	float						mTimer;
	VuColor						mColor;
};
