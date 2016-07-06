//*****************************************************************************
//
//  Copyright (c) 2009-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  Water ramp wave implementation
// 
//*****************************************************************************

#include <float.h>
#include "VuWaterRampWave.h"
#include "VuWaterSurface.h"
#include "VuWater.h"
#include "VuEngine/Gfx/Camera/VuCamera.h"
#include "VuEngine/HAL/Gfx/VuGfx.h"
#include "VuEngine/Gfx/Shaders/VuBasicShaders.h"
#include "VuEngine/Gfx/VuGfxUtil.h"
#include "VuEngine/Dev/VuDevProfile.h"
#include "VuEngine/Math/VuAabb.h"


IMPLEMENT_RTTI(VuWaterRampWave, VuWaterWave);


//*****************************************************************************
VuWaterRampWave::VuWaterRampWave(const VuWaterRampWaveDesc &desc):
	VuWaterWave(POINT_SURFACE_BINNING)
{
	setDesc(desc);

	updateBounds();
}

//*****************************************************************************
void VuWaterRampWave::modify(const VuWaterRampWaveDesc &desc)
{
	setDesc(desc);

	updateBounds();
	VuWater::IF()->rebinWave(this);
}

//*****************************************************************************
bool VuWaterRampWave::tick(float fdt)
{
	if ( refCount() == 1 )
		return false; // done

	return true; // not done
}

//*****************************************************************************
void VuWaterRampWave::getSurfaceData(VuWaterSurfaceDataParams &params)
{
	if ( params.mVertexType == VuWaterSurfaceDataParams::VT_PHYSICS )
		if ( params.mClipType == VuWaterSurfaceDataParams::CT_NOCLIP )
			getSurfaceData<VuWaterSurfaceDataParams::VT_PHYSICS, VuWaterSurfaceDataParams::CT_NOCLIP>(params);
		else
			getSurfaceData<VuWaterSurfaceDataParams::VT_PHYSICS, VuWaterSurfaceDataParams::CT_CLIP>(params);
	else
		if ( params.mClipType == VuWaterSurfaceDataParams::CT_NOCLIP )
			getSurfaceData<VuWaterSurfaceDataParams::VT_RENDER, VuWaterSurfaceDataParams::CT_NOCLIP>(params);
		else
			getSurfaceData<VuWaterSurfaceDataParams::VT_RENDER, VuWaterSurfaceDataParams::CT_CLIP>(params);
}

//*****************************************************************************
void VuWaterRampWave::debugDraw2d()
{
	VuGfxUtil *pGfxUtil = VuGfxUtil::IF();

	// set up matrix
	VuMatrix mat = pGfxUtil->getMatrix();
	mat.translateLocal(VuVector3(mDesc.mPos.mX, mDesc.mPos.mY, 0));
	mat.rotateZLocal(mDesc.mRotZ);
	mat.scaleLocal(mDesc.mSize);

	pGfxUtil->pushMatrix(mat);
	{
		pGfxUtil->drawRectangleOutline2d(0, VuColor(64,255,64,128), VuRect(-0.5f, -0.5f, 1.0f, 1.0f));
	}
	pGfxUtil->popMatrix();
}

//*****************************************************************************
void VuWaterRampWave::setDesc(const VuWaterRampWaveDesc &desc)
{
	mDesc = desc;

	// calculate matricies
	mLocalToWorldMatrix.loadIdentity();
	mLocalToWorldMatrix.scale(0.5f*mDesc.mSize);
	mLocalToWorldMatrix.rotateZ(mDesc.mRotZ);
	mLocalToWorldMatrix.translate(mDesc.mPos);

	mWorldToLocalMatrix = mLocalToWorldMatrix;
	mWorldToLocalMatrix.invert();

	// calculate derived quantities
	mFactor = mDesc.mTransitionRatio > 0 ? 1.0f/(mDesc.mTransitionRatio*(2.0f - mDesc.mTransitionRatio)) : 0;
	mSlope = 2.0f*mDesc.mTransitionRatio*mFactor;
	mSlope *= 0.5f*mDesc.mSize.mZ;

	mNormal.mX = mSlope*mWorldToLocalMatrix.mX.mY;
	mNormal.mY = mSlope*mWorldToLocalMatrix.mY.mY;

	mAxisX = mLocalToWorldMatrix.getAxisX().normal();
	mSlopedFlow = desc.mFlowSpeed*VuCross(VuVector3(-mNormal.mX, -mNormal.mY, 1), -mAxisX);
	mStraightFlow = -desc.mFlowSpeed*mLocalToWorldMatrix.getAxisY().normal();
}

//*****************************************************************************
void VuWaterRampWave::updateBounds()
{
	// aabb
	VuVector2 aabbMin = VuVector2(FLT_MAX, FLT_MAX);
	VuVector2 aabbMax = VuVector2(-FLT_MAX, -FLT_MAX);

	float sin, cos;
	VuSinCos(mDesc.mRotZ, sin, cos);

	VuVector2 vCenter(mDesc.mPos.mX, mDesc.mPos.mY);
	VuVector2 vExtents(0.5f*mDesc.mSize.mX, 0.5f*mDesc.mSize.mY);

	VuVector2 vX = vExtents.mX*VuVector2(cos, -sin);
	VuVector2 vY = vExtents.mY*VuVector2(sin, cos);

	VuMinMax(vCenter - vX - vY, aabbMin, aabbMax);
	VuMinMax(vCenter + vX - vY, aabbMin, aabbMax);
	VuMinMax(vCenter + vX + vY, aabbMin, aabbMax);
	VuMinMax(vCenter - vX + vY, aabbMin, aabbMax);

	mBoundingAabb.mMin = VuVector3(aabbMin.mX, aabbMin.mY, mDesc.mPos.mZ - 0.5f*mDesc.mSize.mZ);
	mBoundingAabb.mMax = VuVector3(aabbMax.mX, aabbMax.mY, mDesc.mPos.mZ + 0.5f*mDesc.mSize.mZ);

	// disk
	mBoundingDiskCenter = vCenter;
	mBoundingDiskRadius = vExtents.mag();
}

//*****************************************************************************
template<int VERTEX_TYPE, int CLIP_TYPE>
void VuWaterRampWave::getSurfaceData(VuWaterSurfaceDataParams &params)
{
	VuWaterVertex vert;
	if ( VERTEX_TYPE == VuWaterSurfaceDataParams::VT_PHYSICS )
		vert.configure(params.mpPhysicsVertex);
	else
		vert.configure(params.mpRenderVertex);

	for ( int iVert = 0; iVert < params.mVertCount; iVert++ )
	{
		if ( CLIP_TYPE == VuWaterSurfaceDataParams::CT_NOCLIP || params.mppWaterSurface[iVert] == params.mpWaterClipSurface )
		{
			float worldX = vert.mpPosition->mX;
			float worldY = vert.mpPosition->mY;

			// transform position into local space
			float localY = worldX*mWorldToLocalMatrix.mX.mY + worldY*mWorldToLocalMatrix.mY.mY + mWorldToLocalMatrix.mT.mY;

			float absLocalY = VuAbs(localY);
			if ( absLocalY > (1.0f - mDesc.mTransitionRatio) )
			{
				//float localY = worldX*mWorldToLocalMatrix.mX.mY + worldY*mWorldToLocalMatrix.mY.mY + mWorldToLocalMatrix.mT.mY;
				float dlocalY_dx = mWorldToLocalMatrix.mX.mY;
				float dlocalY_dy = mWorldToLocalMatrix.mY.mY;

				float height = 0;
				float dheight_dx = 0;
				float dheight_dy = 0;
				float slopeAmt = 0;
				if ( localY < 0 )
				{
					height = -1 + (localY + 1)*(localY + 1)*mFactor;
					if ( VERTEX_TYPE == VuWaterSurfaceDataParams::VT_RENDER )
					{
						dheight_dx = 2*(localY + 1)*mFactor*dlocalY_dx;
						dheight_dy = 2*(localY + 1)*mFactor*dlocalY_dy;
					}
				}
				else
				{
					height = 1 - (1 - localY)*(1 - localY)*mFactor;
					if ( VERTEX_TYPE == VuWaterSurfaceDataParams::VT_RENDER )
					{
						dheight_dx = 2*(1 - localY)*mFactor*dlocalY_dx;
						dheight_dy = 2*(1 - localY)*mFactor*dlocalY_dy;
					}
				}

				height *= 0.5f*mDesc.mSize.mZ;

				*vert.mpHeight += height;

				if ( VERTEX_TYPE == VuWaterSurfaceDataParams::VT_RENDER )
				{
					dheight_dx *= 0.5f*mDesc.mSize.mZ;
					dheight_dy *= 0.5f*mDesc.mSize.mZ;

					vert.mpDzDxy->mX += dheight_dx;
					vert.mpDzDxy->mY += dheight_dy;
				}
				if ( VERTEX_TYPE == VuWaterSurfaceDataParams::VT_PHYSICS )
				{
					slopeAmt = (1.0f - absLocalY)/mDesc.mTransitionRatio;
					vert.mpDxyzDt->mX += VuLerp(mStraightFlow.mX, mSlopedFlow.mX, slopeAmt);
					vert.mpDxyzDt->mY += VuLerp(mStraightFlow.mY, mSlopedFlow.mY, slopeAmt);
					vert.mpDxyzDt->mZ += VuLerp(mStraightFlow.mZ, mSlopedFlow.mZ, slopeAmt);
				}
			}
			else
			{
				*vert.mpHeight += localY*mSlope;

				if ( VERTEX_TYPE == VuWaterSurfaceDataParams::VT_RENDER )
				{
					vert.mpDzDxy->mX += mNormal.mX;
					vert.mpDzDxy->mY += mNormal.mY;
				}
				if ( VERTEX_TYPE == VuWaterSurfaceDataParams::VT_PHYSICS )
				{
					vert.mpDxyzDt->mX += mSlopedFlow.mX;
					vert.mpDxyzDt->mY += mSlopedFlow.mY;
					vert.mpDxyzDt->mZ += mSlopedFlow.mZ;
				}
			}
		}

		// next vert
		vert.mpPosition = (VuPackedVector3 *)((VUBYTE *)vert.mpPosition + params.mStride);
		vert.mpHeight = (float *)((VUBYTE *)vert.mpHeight + params.mStride);
		if ( VERTEX_TYPE == VuWaterSurfaceDataParams::VT_PHYSICS )
			vert.mpDxyzDt = (VuPackedVector3 *)((VUBYTE *)vert.mpDxyzDt + params.mStride);
		if ( VERTEX_TYPE == VuWaterSurfaceDataParams::VT_RENDER )
			vert.mpDzDxy = (VuPackedVector2 *)((VUBYTE *)vert.mpDzDxy + params.mStride);
	}
}
