//*****************************************************************************
//
//  Copyright (c) 2008-2008 Ralf Knoesel
//  Copyright (c) 2008-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  3dDrawComponent class
// 
//*****************************************************************************

#pragma once

#include "Vu3dDrawManager.h"
#include "VuEngine/Components/VuComponent.h"
#include "VuEngine/Method/VuMethod.h"
#include "VuEngine/Math/VuVector3.h"
#include "VuEngine/Math/VuAabb.h"
#include "VuEngine/Math/VuMatrix.h"

class VuDbvtNode;


class Vu3dDrawComponent : public VuComponent
{
	DECLARE_SHORT_COMPONENT_TYPE(3dDraw)
	DECLARE_RTTI

public:
	Vu3dDrawComponent(VuEntity *pOwnerEntity, bool bReflectDefault = false);
	~Vu3dDrawComponent();

	// method specification
	template<class T>
	void			setDrawMethod(T *pObj, void (T::*method)(const VuGfxDrawParams &params));
	template<class T>
	void			setDrawShadowMethod(T *pObj, void (T::*method)(const VuGfxDrawShadowParams &params));
	template<class T>
	void			setDrawPrefetchMethod(T *pObj, void (T::*method)());

	// capability
	void			enableReflection(bool enable) { mbReflecting = enable; }
	void			enableShadow(bool enable) { mbShadowing = enable; }

	// adding/removing from scene
	void			show();
	void			hide();
	bool			isShown() { return mbRegistered; }

	// visibility
	void			updateVisibility(const VuAabb &aabb);
	void			updateVisibility(const VuAabb &aabb, const VuMatrix &transform);
	const VuAabb	&getAabb()		{ return mAabb; }

	// zones
	void			setZoneBits(VUUINT32 zoneBits) { mZoneBits = zoneBits; }
	VUUINT32		getZoneBits() { return mZoneBits; }

protected:
	friend class Vu3dDrawManager;

	typedef VuMethodInterface1<void, const VuGfxDrawParams &> DrawMethod;
	typedef VuMethodInterface1<void, const VuGfxDrawShadowParams &> DrawShadowMethod;
	typedef VuMethodInterface0<void> DrawPrefetchMethod;

	DrawMethod				*mpDrawMethod;
	DrawShadowMethod		*mpDrawShadowMethod;
	DrawPrefetchMethod		*mpDrawPrefetchMethod;
	bool					mbDrawing;
	bool					mbReflecting;
	bool					mbShadowing;
	bool					mbRegistered;
	VuDbvtNode				*mpDbvtNode;
	VuAabb					mAabb;
	VUUINT32				mZoneBits;
};

#include "Vu3dDrawComponent.inl"
