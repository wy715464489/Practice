//*****************************************************************************
//
//  Copyright (c) 2008-2008 Ralf Knoesel
//  Copyright (c) 2008-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  Water directional flow wave implementation
// 
//*****************************************************************************

#include <float.h>
#include "VuWaterDirectionalFlowWave.h"
#include "VuWater.h"
#include "VuEngine/Gfx/Camera/VuCamera.h"
#include "VuEngine/HAL/Gfx/VuGfx.h"
#include "VuEngine/Gfx/Shaders/VuBasicShaders.h"
#include "VuEngine/Gfx/VuGfxUtil.h"
#include "VuEngine/Dev/VuDevProfile.h"
#include "VuEngine/Math/VuAabb.h"


IMPLEMENT_RTTI(VuWaterDirectionalFlowWave, VuWaterWave);


//*****************************************************************************
VuWaterDirectionalFlowWave::VuWaterDirectionalFlowWave(const VuWaterDirectionalFlowWaveDesc &desc)
{
	setDesc(desc);

	updateBounds();
}

//*****************************************************************************
void VuWaterDirectionalFlowWave::modify(const VuWaterDirectionalFlowWaveDesc &desc)
{
	setDesc(desc);

	updateBounds();
	VuWater::IF()->rebinWave(this);
}

//*****************************************************************************
bool VuWaterDirectionalFlowWave::tick(float fdt)
{
	if ( refCount() == 1 )
		return false; // done

	return true; // not done
}

//*****************************************************************************
void VuWaterDirectionalFlowWave::getSurfaceData(VuWaterSurfaceDataParams &params)
{
	if ( params.mVertexType == VuWaterSurfaceDataParams::VT_PHYSICS )
	{
		if ( params.mClipType == VuWaterSurfaceDataParams::CT_NOCLIP )
			getSurfaceData<VuWaterSurfaceDataParams::CT_NOCLIP>(params);
		else
			getSurfaceData<VuWaterSurfaceDataParams::CT_CLIP>(params);
	}
}

//*****************************************************************************
void VuWaterDirectionalFlowWave::debugDraw2d()
{
	VuGfxUtil *pGfxUtil = VuGfxUtil::IF();

	// set up matrix
	VuMatrix mat = pGfxUtil->getMatrix();
	mat.translateLocal(VuVector3(mDesc.mPos.mX, mDesc.mPos.mY, 0));
	mat.rotateZLocal(mDesc.mRotZ);
	mat.scaleLocal(VuVector3(mDesc.mSizeX, mDesc.mSizeY, 1));

	pGfxUtil->pushMatrix(mat);
	{
		pGfxUtil->drawRectangleOutline2d(0, VuColor(64,255,64,128), VuRect(-0.5f, -0.5f, 1.0f, 1.0f));
	}
	pGfxUtil->popMatrix();
}

//*****************************************************************************
void VuWaterDirectionalFlowWave::setDesc(const VuWaterDirectionalFlowWaveDesc &desc)
{
	mDesc = desc;

	// recalculate matricies
	mLocalToWorldMatrix.loadIdentity();
	mLocalToWorldMatrix.scale(VuVector3(0.5f*mDesc.mSizeX, 0.5f*mDesc.mSizeY, 1.0f));
	mLocalToWorldMatrix.rotateZ(mDesc.mRotZ);
	mLocalToWorldMatrix.translate(mDesc.mPos);

	mWorldToLocalMatrix = mLocalToWorldMatrix;
	mWorldToLocalMatrix.invert();
}

//*****************************************************************************
void VuWaterDirectionalFlowWave::updateBounds()
{
	// aabb
	VuVector2 aabbMin = VuVector2(FLT_MAX, FLT_MAX);
	VuVector2 aabbMax = VuVector2(-FLT_MAX, -FLT_MAX);

	float sin, cos;
	VuSinCos(mDesc.mRotZ, sin, cos);

	VuVector2 vCenter(mDesc.mPos.mX, mDesc.mPos.mY);
	VuVector2 vExtents(0.5f*mDesc.mSizeX, 0.5f*mDesc.mSizeY);

	VuVector2 vX = vExtents.mX*VuVector2(cos, -sin);
	VuVector2 vY = vExtents.mY*VuVector2(sin, cos);

	VuMinMax(vCenter - vX - vY, aabbMin, aabbMax);
	VuMinMax(vCenter + vX - vY, aabbMin, aabbMax);
	VuMinMax(vCenter + vX + vY, aabbMin, aabbMax);
	VuMinMax(vCenter - vX + vY, aabbMin, aabbMax);

	mBoundingAabb.mMin = VuVector3(aabbMin.mX, aabbMin.mY, mDesc.mPos.mZ);
	mBoundingAabb.mMax = VuVector3(aabbMax.mX, aabbMax.mY, mDesc.mPos.mZ);

	// disk
	mBoundingDiskCenter = vCenter;
	mBoundingDiskRadius = vExtents.mag();
}

//*****************************************************************************
template<int CLIP_TYPE>
void VuWaterDirectionalFlowWave::getSurfaceData(VuWaterSurfaceDataParams &params)
{
	VuWaterVertex vert;
	vert.configure(params.mpPhysicsVertex);

	for ( int iVert = 0; iVert < params.mVertCount; iVert++ )
	{
		if ( CLIP_TYPE == VuWaterSurfaceDataParams::CT_NOCLIP || params.mppWaterSurface[iVert] == params.mpWaterClipSurface )
		{
			float worldX = vert.mpPosition->mX;
			float worldY = vert.mpPosition->mY;

			// transform position into local space
			float localX = worldX*mWorldToLocalMatrix.mX.mX + worldY*mWorldToLocalMatrix.mY.mX + mWorldToLocalMatrix.mT.mX;
			float localY = worldX*mWorldToLocalMatrix.mX.mY + worldY*mWorldToLocalMatrix.mY.mY + mWorldToLocalMatrix.mT.mY;

			float absLocalX = VuAbs(localX);
			float absLocalY = VuAbs(localY);
			if ( VuMax(absLocalX, absLocalY) < 1 )
			{
				float weight = 1.0f;
				if ( absLocalX > mDesc.mLateralDecayRatio )
					weight *= (absLocalX - 1.0f)/(mDesc.mLateralDecayRatio - 1.0f);
				if ( absLocalY > mDesc.mLongitudinalDecayRatio )
					weight *= (absLocalY - 1.0f)/(mDesc.mLongitudinalDecayRatio - 1.0f);

				vert.mpDxyzDt->mX += weight*mDesc.mFlowVelocity.mX;
				vert.mpDxyzDt->mY += weight*mDesc.mFlowVelocity.mY;
				vert.mpDxyzDt->mZ += weight*mDesc.mFlowVelocity.mZ;
			}
		}

		// next vert
		vert.mpPosition = (VuPackedVector3 *)((VUBYTE *)vert.mpPosition + params.mStride);
		vert.mpDxyzDt = (VuPackedVector3 *)((VUBYTE *)vert.mpDxyzDt + params.mStride);
	}
}
