//*****************************************************************************
//
//  Copyright (c) 2007-2008 Ralf Knoesel
//  Copyright (c) 2008-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  3dLayoutComponent class
// 
//*****************************************************************************

#pragma once

#include "VuEngine/Components/VuComponent.h"
#include "VuEngine/Method/VuMethod.h"
#include "VuEngine/Math/VuVector3.h"
#include "VuEngine/Math/VuAabb.h"

class VuCamera;
class VuMatrix;
class Vu3dLayoutDrawParams;


class Vu3dLayoutComponent : public VuComponent
{
	DECLARE_SHORT_COMPONENT_TYPE(3dLayout)
	DECLARE_RTTI

public:
	Vu3dLayoutComponent(VuEntity *pOwner);
	~Vu3dLayoutComponent();

	// drawing
	template<class T>
	void			setDrawMethod(T *pObj, void (T::*method)(const Vu3dLayoutDrawParams &params));
	void			draw(const Vu3dLayoutDrawParams &params) const;

	// colliding
	template<class T>
	void			setCollideMethod(T *pOwner, bool (T::*collideMethod)(const VuVector3 &, VuVector3 &));
	bool			collideRay(const VuVector3 &v0, VuVector3 &v1) const;

	// bounding info
	void			setLocalBounds(const VuAabb &bounds)	{ mBounds = bounds; }
	const VuAabb	&getLocalBounds() const;

	void			setForceVisible(bool force) { mForceVisible = force; }
	bool			getForceVisible() { return mForceVisible; }

private:
	typedef VuMethodInterface1<void, const Vu3dLayoutDrawParams &> DrawMethod;
	typedef VuMethodInterface2<bool, const VuVector3 &, VuVector3 &> CollideMethod;

	DrawMethod		*mpDrawMethod;
	CollideMethod	*mpCollideMethod;
	VuAabb			mBounds;
	bool			mForceVisible;
};

class Vu3dLayoutDrawParams
{
public:
	explicit Vu3dLayoutDrawParams(const VuCamera &camera);

	const VuCamera	&mCamera;
	bool			mbSelected;
	bool			mbDrawCollision;
	bool			mbDrawFluids;
	bool			mbForceHighLOD;
};

#include "Vu3dLayoutComponent.inl"
