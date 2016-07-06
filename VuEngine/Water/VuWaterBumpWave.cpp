//*****************************************************************************
//
//  Copyright (c) 2009-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  Water bump wave implementation
// 
//*****************************************************************************

#include <float.h>
#include "VuWaterBumpWave.h"
#include "VuWater.h"
#include "VuEngine/Gfx/Camera/VuCamera.h"
#include "VuEngine/HAL/Gfx/VuGfx.h"
#include "VuEngine/Gfx/Shaders/VuBasicShaders.h"
#include "VuEngine/Gfx/VuGfxUtil.h"
#include "VuEngine/Dev/VuDevProfile.h"
#include "VuEngine/Math/VuAabb.h"


IMPLEMENT_RTTI(VuWaterBumpWave, VuWaterWave);


//*****************************************************************************
VuWaterBumpWave::VuWaterBumpWave(const VuWaterBumpWaveDesc &desc)
{
	setDesc(desc);

	updateBounds();
}

//*****************************************************************************
void VuWaterBumpWave::modify(const VuWaterBumpWaveDesc &desc)
{
	setDesc(desc);

	updateBounds();
	VuWater::IF()->rebinWave(this);
}

//*****************************************************************************
bool VuWaterBumpWave::tick(float fdt)
{
	if ( refCount() == 1 )
		return false; // done

	return true; // not done
}

//*****************************************************************************
void VuWaterBumpWave::getSurfaceData(VuWaterSurfaceDataParams &params)
{
	if ( params.mVertexType == VuWaterSurfaceDataParams::VT_PHYSICS )
	{
		if ( params.mClipType == VuWaterSurfaceDataParams::CT_NOCLIP )
			getSurfaceData<VuWaterSurfaceDataParams::VT_PHYSICS, VuWaterSurfaceDataParams::CT_NOCLIP>(params);
		else
			getSurfaceData<VuWaterSurfaceDataParams::VT_PHYSICS, VuWaterSurfaceDataParams::CT_CLIP>(params);
	}
	else
	{
		if ( params.mClipType == VuWaterSurfaceDataParams::CT_NOCLIP )
			getSurfaceData<VuWaterSurfaceDataParams::VT_RENDER, VuWaterSurfaceDataParams::CT_NOCLIP>(params);
		else
			getSurfaceData<VuWaterSurfaceDataParams::VT_RENDER, VuWaterSurfaceDataParams::CT_CLIP>(params);
	}
}

//*****************************************************************************
void VuWaterBumpWave::debugDraw2d()
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
void VuWaterBumpWave::setDesc(const VuWaterBumpWaveDesc &desc)
{
	mDesc = desc;

	// recalculate matricies
	mLocalToWorldMatrix.loadIdentity();
	mLocalToWorldMatrix.scale(VuVector3(0.5f*mDesc.mSizeX, 0.5f*mDesc.mSizeY, mDesc.mMaxHeight));
	mLocalToWorldMatrix.rotateZ(mDesc.mRotZ);
	mLocalToWorldMatrix.translate(mDesc.mPos);

	mWorldToLocalMatrix = mLocalToWorldMatrix;
	mWorldToLocalMatrix.invert();
}

//*****************************************************************************
void VuWaterBumpWave::updateBounds()
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

	mBoundingAabb.mMin = VuVector3(aabbMin.mX, aabbMin.mY, mDesc.mPos.mZ - mDesc.mMaxHeight);
	mBoundingAabb.mMax = VuVector3(aabbMax.mX, aabbMax.mY, mDesc.mPos.mZ + mDesc.mMaxHeight);

	// disk
	mBoundingDiskCenter = vCenter;
	mBoundingDiskRadius = vExtents.mag();
}

//*****************************************************************************
template<int VERTEX_TYPE, int CLIP_TYPE>
void VuWaterBumpWave::getSurfaceData(VuWaterSurfaceDataParams &params)
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
			float localX = worldX*mWorldToLocalMatrix.mX.mX + worldY*mWorldToLocalMatrix.mY.mX + mWorldToLocalMatrix.mT.mX;
			float localY = worldX*mWorldToLocalMatrix.mX.mY + worldY*mWorldToLocalMatrix.mY.mY + mWorldToLocalMatrix.mT.mY;

			float absLocalX = VuAbs(localX);
			float absLocalY = VuAbs(localY);
			if ( VuMax(absLocalX, absLocalY) < 1 )
			{
				float u = localY*VU_PI;

				float sin_u, cos_u;
				VuSinCosEst(VuModAngle(u), sin_u, cos_u);

				float height = mDesc.mMaxHeight*0.5f*(1.0f + cos_u);

				float lateral_damping = 1;
				float sin_v=0.0f, cos_v=0.0f;
				if ( absLocalX > mDesc.mLateralDecayRatio )
				{
					float v = (absLocalX - mDesc.mLateralDecayRatio)/(1 - mDesc.mLateralDecayRatio)*VU_PI;
					VuSinCosEst(VuModAngle(v), sin_v, cos_v);
					lateral_damping = 0.5f*(1.0f + cos_v);
				}
				height = height*lateral_damping;

				*vert.mpHeight += height;

				if ( VERTEX_TYPE == VuWaterSurfaceDataParams::VT_RENDER )
				{
					//float localX = worldX*mWorldToLocalMatrix.mX.mX + worldY*mWorldToLocalMatrix.mY.mX + mWorldToLocalMatrix.mT.mX;
					float dlocalX_dx = mWorldToLocalMatrix.mX.mX;
					float dlocalX_dy = mWorldToLocalMatrix.mY.mX;

					//float localY = worldX*mWorldToLocalMatrix.mX.mY + worldY*mWorldToLocalMatrix.mY.mY + mWorldToLocalMatrix.mT.mY;
					float dlocalY_dx = mWorldToLocalMatrix.mX.mY;
					float dlocalY_dy = mWorldToLocalMatrix.mY.mY;

					//float absLocalX = VuAbs(localX);
					float dabsLocalX_dx = VuSelect(localX, dlocalX_dx, -dlocalX_dx);
					float dabsLocalX_dy = VuSelect(localX, dlocalX_dy, -dlocalX_dy);

					//float u = localY*VU_PI;
					float du_dx = dlocalY_dx*VU_PI;
					float du_dy = dlocalY_dy*VU_PI;

					//float height = mDesc.mMaxHeight*0.5f*(1.0f + cos_u);
					float dheight_dx = -0.5f*mDesc.mMaxHeight*sin_u*du_dx;
					float dheight_dy = -0.5f*mDesc.mMaxHeight*sin_u*du_dy;

					//float lateral_damping = 1;
					float dlateral_damping_dx = 0;
					float dlateral_damping_dy = 0;

					if ( absLocalX > mDesc.mLateralDecayRatio )
					{
						//float v = (absLocalX - mDesc.mLateralDecayRatio)/(1 - mDesc.mLateralDecayRatio)*VU_PI;
						float dv_dx = dabsLocalX_dx/(1 - mDesc.mLateralDecayRatio)*VU_PI;
						float dv_dy = dabsLocalX_dy/(1 - mDesc.mLateralDecayRatio)*VU_PI;

						//lateral_damping = 0.5f*(1.0f + cos_v);
						dlateral_damping_dx = -0.5f*sin_v*dv_dx;
						dlateral_damping_dy = -0.5f*sin_v*dv_dy;
					}

					//height = height*lateral_damping;
					dheight_dx = height*dlateral_damping_dx + dheight_dx*lateral_damping;
					dheight_dy = height*dlateral_damping_dy + dheight_dy*lateral_damping;

					vert.mpDzDxy->mX += dheight_dx;
					vert.mpDzDxy->mY += dheight_dy;
				}
			}
		}

		// next vert
		vert.mpPosition = (VuPackedVector3 *)((VUBYTE *)vert.mpPosition + params.mStride);
		vert.mpHeight = (float *)((VUBYTE *)vert.mpHeight + params.mStride);
		if ( VERTEX_TYPE == VuWaterSurfaceDataParams::VT_RENDER )
			vert.mpDzDxy = (VuPackedVector2 *)((VUBYTE *)vert.mpDzDxy + params.mStride);
	}
}
