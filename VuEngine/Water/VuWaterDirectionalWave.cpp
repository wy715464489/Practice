//*****************************************************************************
//
//  Copyright (c) 2008-2008 Ralf Knoesel
//  Copyright (c) 2008-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  Water directional wave implementation
// 
//*****************************************************************************

#include <float.h>
#include "VuWaterDirectionalWave.h"
#include "VuWater.h"
#include "VuEngine/Gfx/Camera/VuCamera.h"
#include "VuEngine/HAL/Gfx/VuGfx.h"
#include "VuEngine/Gfx/Shaders/VuBasicShaders.h"
#include "VuEngine/Gfx/VuGfxUtil.h"
#include "VuEngine/Dev/VuDevProfile.h"
#include "VuEngine/Math/VuAabb.h"


IMPLEMENT_RTTI(VuWaterDirectionalWave, VuWaterWave);


//*****************************************************************************
VuWaterDirectionalWave::VuWaterDirectionalWave(const VuWaterDirectionalWaveDesc &desc):
	mAge(0)
{
	setDesc(desc);

	updateBounds();
}

//*****************************************************************************
void VuWaterDirectionalWave::modify(const VuWaterDirectionalWaveDesc &desc)
{
	setDesc(desc);

	updateBounds();
	VuWater::IF()->rebinWave(this);
}

//*****************************************************************************
bool VuWaterDirectionalWave::tick(float fdt)
{
	mAge += fdt;

	if ( refCount() == 1 )
		return false; // done

	return true; // not done
}

//*****************************************************************************
void VuWaterDirectionalWave::getSurfaceData(VuWaterSurfaceDataParams &params)
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
void VuWaterDirectionalWave::debugDraw2d()
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
void VuWaterDirectionalWave::setDesc(const VuWaterDirectionalWaveDesc &desc)
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
void VuWaterDirectionalWave::updateBounds()
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
void VuWaterDirectionalWave::getSurfaceData(VuWaterSurfaceDataParams &params)
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
				float u = (mDesc.mFrequency*localY - mDesc.mSpeed*mAge)*2.0f*VU_PI;

				float sin_u, cos_u;
				VuSinCosEst(VuModAngle(u), sin_u, cos_u);

				float height = mDesc.mMaxHeight*sin_u;

				float lateral_damping = 1;
				float sin_v=0.0f, cos_v=0.0f;
				if ( absLocalX > mDesc.mLateralDecayRatio )
				{
					float v = (absLocalX - mDesc.mLateralDecayRatio)/(1 - mDesc.mLateralDecayRatio)*VU_PI;
					VuSinCosEst(VuModAngle(v), sin_v, cos_v);
					lateral_damping = 0.5f*(1.0f + cos_v);
				}
				height = height*lateral_damping;

				float longitudinal_damping = 1;
				float sin_w=0.0f, cos_w=0.0f;
				if ( absLocalY > mDesc.mLongitudinalDecayRatio )
				{
					float w = (absLocalY - mDesc.mLongitudinalDecayRatio)/(1 - mDesc.mLongitudinalDecayRatio)*VU_PI;
					VuSinCosEst(VuModAngle(w), sin_w, cos_w);
					longitudinal_damping = 0.5f*(1.0f + cos_w);
				}
				height = height*longitudinal_damping;

				*vert.mpHeight += height;

				if ( VERTEX_TYPE == VuWaterSurfaceDataParams::VT_PHYSICS )
				{
					//float u = (mDesc.mFrequency*localY - mDesc.mSpeed*mAge)*2.0f*VU_PI;
					float du_dt = -mDesc.mSpeed*2.0f*VU_PI;

					//float height = mDesc.mMaxHeight*sin_u;
					float dheight_dt = mDesc.mMaxHeight*cos_u*du_dt;

					//height = height*lateral_damping;
					dheight_dt *= lateral_damping;

					//height = height*longitudinal_damping;
					dheight_dt *= longitudinal_damping;

					vert.mpDxyzDt->mZ += dheight_dt;
				}
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

					//float absLocalY = VuAbs(localY);
					float dabsLocalY_dx = VuSelect(localY, dlocalY_dx, -dlocalY_dx);
					float dabsLocalY_dy = VuSelect(localY, dlocalY_dy, -dlocalY_dy);

					//float u = (mDesc.mFrequency*localY - mDesc.mSpeed*mAge)*2.0f*VU_PI;
					float du_dx = mDesc.mFrequency*dlocalY_dx*2.0f*VU_PI;
					float du_dy = mDesc.mFrequency*dlocalY_dy*2.0f*VU_PI;

					//float height = mDesc.mMaxHeight*sin_u;
					float dheight_dx = mDesc.mMaxHeight*cos_u*du_dx;
					float dheight_dy = mDesc.mMaxHeight*cos_u*du_dy;

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

					//float longitudinal_damping = 1;
					float dlongitudinal_damping_dx = 0;
					float dlongitudinal_damping_dy = 0;
					if ( absLocalY > mDesc.mLongitudinalDecayRatio )
					{
						//float w = (absLocalY - mDesc.mLongitudinalDecayRatio)/(1 - mDesc.mLongitudinalDecayRatio)*VU_PI;
						float dw_dx = dabsLocalY_dx/(1 - mDesc.mLongitudinalDecayRatio)*VU_PI;
						float dw_dy = dabsLocalY_dy/(1 - mDesc.mLongitudinalDecayRatio)*VU_PI;

						//longitudinal_damping = 0.5f*(1.0f + cos_w);
						dlongitudinal_damping_dx = -0.5f*sin_w*dw_dx;
						dlongitudinal_damping_dy = -0.5f*sin_w*dw_dy;
					}

					//height = height*longitudinal_damping;
					dheight_dx = height*dlongitudinal_damping_dx + dheight_dx*longitudinal_damping;
					dheight_dy = height*dlongitudinal_damping_dy + dheight_dy*longitudinal_damping;

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
		if ( VERTEX_TYPE == VuWaterSurfaceDataParams::VT_PHYSICS )
			vert.mpDxyzDt = (VuPackedVector3 *)((VUBYTE *)vert.mpDxyzDt + params.mStride);
	}
}
