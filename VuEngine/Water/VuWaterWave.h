//*****************************************************************************
//
//  Copyright (c) 2008-2008 Ralf Knoesel
//  Copyright (c) 2008-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  Water wave class
// 
//*****************************************************************************

#pragma once

#include "VuWaterVertex.h"
#include "VuEngine/Objects/VuRefObj.h"
#include "VuEngine/Objects/VuRTTI.h"
#include "VuEngine/Containers/VuPointerList.h"
#include "VuEngine/Math/VuPackedVector.h"
#include "VuEngine/Math/VuVector2.h"
#include "VuEngine/Math/VuAabb.h"

class VuWaterSurfaceDataParams;
class VuWaterTreeNode;
class VuWaterSurface;
class VuCamera;
class VuDbrtNode;


//*****************************************************************************
// Water wave interface
//*****************************************************************************
class VuWaterWave : public VuRefObj
{
	DECLARE_RTTI

public:
	VuWaterWave(VUUINT32 flags = 0);

	// flags
	enum
	{
		POINT_SURFACE_BINNING	= 1<<0,
	};

	void				setTimeFactor(float timeFactor) { mTimeFactor = timeFactor; }

	virtual	bool		tick(float fdt) = 0;
	virtual void		getSurfaceData(VuWaterSurfaceDataParams &params) = 0;
	virtual void		debugDraw3d(const VuCamera &camera) {}
	virtual void		debugDraw2d() {}

	typedef VuPointerList<VuDbrtNode> DbrtNodes;

	VuAabb				mBoundingAabb;
	VuVector2			mBoundingDiskCenter;
	float				mBoundingDiskRadius;
	VuWaterTreeNode		*mpWaterTreeNode;
	VuWaterWave			*mpNextWaveInNode;
	DbrtNodes			mDbrtNodes;
	VUUINT32			mFlags;
	float				mTimeFactor;

protected:
	virtual ~VuWaterWave();
};


//*****************************************************************************
// Water vertex used during water composition
//*****************************************************************************
class VuWaterVertex
{
public:
	inline void configure(VuWaterPhysicsVertex *pVerts)
	{
		mpPosition = (const VuPackedVector3 *)&pVerts->mPosition;
		mpHeight = &pVerts->mHeight;
		mpDxyzDt = (VuPackedVector3 *)&pVerts->mDxyzDt;
	}
	inline void configure(VuWaterRenderVertex *pVerts)
	{
		mpPosition = &pVerts->mPosition;
		mpHeight = &pVerts->mPosition.mZ;
		mpDzDxy = &pVerts->mDzDxy;
		mpFoam = &pVerts->mFoam;
	}

	const VuPackedVector3	*mpPosition;
	float					*mpHeight;
	VuPackedVector2			*mpDzDxy;
	VuPackedVector3			*mpDxyzDt;
	float					*mpFoam;
};
