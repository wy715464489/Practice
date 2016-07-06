//*****************************************************************************
//
//  Copyright (c) 2008-2008 Ralf Knoesel
//  Copyright (c) 2008-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  UI draw utility functionality.
// 
//*****************************************************************************

#pragma once

#include "VuEngine/Math/VuVector2.h"
#include "VuEngine/Math/VuRect.h"
#include "VuEngine/DB/VuFontDB.h"

class VuEntity;
class VuRect;

struct VuUIDrawParams
{
	VuVector2	mPosition;
	VuVector2	mLocalScale;
	VuVector2	mAuthScale;
	VuVector2	mInvAuthScale;
	float		mDepth;

	inline VuRect transform(const VuRect &rect) const {
		return (rect*mLocalScale + mPosition)*mInvAuthScale;
	}
	inline VuVector2 transform(const VuVector2 &pos) const {
		return (pos*mLocalScale + mPosition)*mInvAuthScale;
	}
	inline VuRect transformInv(const VuRect &rect) const {
		return (rect*mAuthScale - mPosition)/mLocalScale;
	}
	inline VuVector2 transformInv(const VuVector2 &pos) const {
		return (pos*mAuthScale - mPosition)/mLocalScale;
	}
};

namespace VuUIDrawUtil
{
	void getParams(VuEntity *pEntity, VuUIDrawParams &params);
	bool isVisible(const VuRect &rect);
	void shrinkToFit(const char* pText, const VuUIDrawParams& uiParams, const VuFontDB::VuEntry& fontEntry, VuFontDrawParams& fdParams, VuRect& rect);
}
