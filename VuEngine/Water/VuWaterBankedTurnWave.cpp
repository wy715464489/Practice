//*****************************************************************************
//
//  Copyright (c) 2009-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  Water banked turn wave implementation
// 
//*****************************************************************************

#include <float.h>
#include "VuWaterBankedTurnWave.h"
#include "VuWater.h"
#include "VuEngine/Gfx/Camera/VuCamera.h"
#include "VuEngine/HAL/Gfx/VuGfx.h"
#include "VuEngine/Gfx/Shaders/VuBasicShaders.h"
#include "VuEngine/Gfx/VuGfxUtil.h"
#include "VuEngine/Dev/VuDevProfile.h"
#include "VuEngine/Math/VuAabb.h"


IMPLEMENT_RTTI(VuWaterBankedTurnWave, VuWaterWave);


//*****************************************************************************
VuWaterBankedTurnWave::VuWaterBankedTurnWave(const VuWaterBankedTurnWaveDesc &desc):
	mFwd(0,1),
	mHalfAngularSize(VU_PI)
{
	setDesc(desc);

	updateBounds();
}

//*****************************************************************************
void VuWaterBankedTurnWave::modify(const VuWaterBankedTurnWaveDesc &desc)
{
	setDesc(desc);

	updateBounds();
	VuWater::IF()->rebinWave(this);
}

//*****************************************************************************
bool VuWaterBankedTurnWave::tick(float fdt)
{
	if ( refCount() == 1 )
		return false; // done

	return true; // not done
}

//*****************************************************************************
void VuWaterBankedTurnWave::getSurfaceData(VuWaterSurfaceDataParams &params)
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
void VuWaterBankedTurnWave::debugDraw3d(const VuCamera &camera)
{
	VuGfxUtil *pGfxUtil = VuGfxUtil::IF();

	VuMatrix matMVP;
	matMVP.loadIdentity();
	matMVP.translate(mDesc.mPos);
	matMVP *= camera.getViewProjMatrix();

	VuColor col(255,255,0);

		int numSegments = 16;
		for ( int iSide = 0; iSide <= numSegments; iSide++ )
		{
			float fCurAngle = -0.5f*mDesc.mAngularSize + mDesc.mAngularSize*iSide/numSegments;
			float fNextAngle = -0.5f*mDesc.mAngularSize + mDesc.mAngularSize*(iSide + 1)/numSegments;
			VuVector3 v0 = VuVector3(-VuSin(fCurAngle), VuCos(fCurAngle), 0);
			VuVector3 v1 = VuVector3(-VuSin(fNextAngle), VuCos(fNextAngle), 0);

			VuVector3 vi0 = mDesc.mInnerRadius*v0;
			VuVector3 vi1 = mDesc.mInnerRadius*v1;
			VuVector3 vo0 = mDesc.mOuterRadius*v0;
			VuVector3 vo1 = mDesc.mOuterRadius*v1;
			VuVector3 vc0 = 0.5f*(vi0 + vo0) + VuVector3(0, 0, mDesc.mHeight);
			VuVector3 vc1 = 0.5f*(vi1 + vo1) + VuVector3(0, 0, mDesc.mHeight);

			pGfxUtil->drawLine3d(col, vi0, vc0, matMVP);
			pGfxUtil->drawLine3d(col, vc0, vo0, matMVP);

			if ( iSide < numSegments )
			{
				pGfxUtil->drawLine3d(col, vi0, vi1, matMVP);
				pGfxUtil->drawLine3d(col, vc0, vc1, matMVP);
				pGfxUtil->drawLine3d(col, vo0, vo1, matMVP);
			}
		}
}

//*****************************************************************************
void VuWaterBankedTurnWave::debugDraw2d()
{
	VuGfxUtil *pGfxUtil = VuGfxUtil::IF();

	// set up matrix
	VuMatrix mat = pGfxUtil->getMatrix();
	mat.translateLocal(VuVector3(mDesc.mPos.mX, mDesc.mPos.mY, 0));

	pGfxUtil->pushMatrix(mat);
	{
		VuVector2 vExtents(mDesc.mOuterRadius, mDesc.mOuterRadius);
		pGfxUtil->drawEllipseOutline2d(0, VuColor(64,255,64,128), VuRect(-vExtents, vExtents), 16);
	}
	pGfxUtil->popMatrix();
}

//*****************************************************************************
void VuWaterBankedTurnWave::setDesc(const VuWaterBankedTurnWaveDesc &desc)
{
	mDesc = desc;

	mFwd.mX = -VuSin(desc.mRotZ);
	mFwd.mY =  VuCos(desc.mRotZ);

	mHalfAngularSize = 0.5f*mDesc.mAngularSize;
}

//*****************************************************************************
void VuWaterBankedTurnWave::updateBounds()
{
	// aabb
	VuVector2 vCenter(mDesc.mPos.mX, mDesc.mPos.mY);
	VuVector2 vExtents(mDesc.mOuterRadius, mDesc.mOuterRadius);

	mBoundingAabb.mMin = mDesc.mPos - VuVector3(vExtents.mX, vExtents.mY, 0.0f);
	mBoundingAabb.mMax = mDesc.mPos + VuVector3(vExtents.mX, vExtents.mY, mDesc.mHeight);

	// disk
	mBoundingDiskCenter = vCenter;
	mBoundingDiskRadius = mDesc.mOuterRadius;
}

//*****************************************************************************
template<int VERTEX_TYPE, int CLIP_TYPE>
void VuWaterBankedTurnWave::getSurfaceData(VuWaterSurfaceDataParams &params)
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
			VuVector2 vPos = VuVector2(vert.mpPosition->mX - mDesc.mPos.mX, vert.mpPosition->mY - mDesc.mPos.mY);

			float dist_squared = vPos.magSquared();
			if ( dist_squared < mDesc.mOuterRadius*mDesc.mOuterRadius )
			{
				if ( dist_squared > mDesc.mInnerRadius*mDesc.mInnerRadius )
				{
					float dist = VuSqrt(dist_squared);

					// angular check
					float cosAng = VuDot(vPos, mFwd);
					cosAng = cosAng/dist;
					cosAng = VuClamp(cosAng, -1.0f, 1.0f);
					float ang = VuACos(cosAng);
					if ( ang < mHalfAngularSize )
					{
						float u = (dist - mDesc.mInnerRadius)/(mDesc.mOuterRadius - mDesc.mInnerRadius);
						u = -VU_PI + u*VU_2PI;

						float sin_u, cos_u;
						VuSinCosEst(u, sin_u, cos_u);

						float magnitude = 0.5f*(1.0f + cos_u);

						float height = magnitude*mDesc.mHeight;

						// apply angular damping
						float angRatio = ang/mHalfAngularSize;
						float angular_damping = 1;
						float sin_v=0.0f, cos_v=0.0f;
						if ( angRatio > mDesc.mAngularDecayRatio )
						{
							float v = (angRatio - mDesc.mAngularDecayRatio)/(1 - mDesc.mAngularDecayRatio)*VU_PI;
							VuSinCosEst(VuModAngle(v), sin_v, cos_v);
							angular_damping = 0.5f*(1.0f + cos_v);
						}
						height = height*angular_damping;

						*vert.mpHeight += height;

						if ( VERTEX_TYPE == VuWaterSurfaceDataParams::VT_RENDER )
						{
							//float dist_squared = vPos.magSquared();
							//float dist = VuSqrt(dist_squared);
							float ddist_dx = vPos.mX/dist;
							float ddist_dy = vPos.mY/dist;

							//float cosAng = dot(vPos, mFwd);
							float dcosAng_dx = mFwd.mX;
							float dcosAng_dy = mFwd.mY;

							//cosAng = cosAng/dist;
							dcosAng_dx = (dcosAng_dx*dist - cosAng*ddist_dx)/dist_squared;
							dcosAng_dy = (dcosAng_dy*dist - cosAng*ddist_dy)/dist_squared;

							//float ang = VuACos(cosAng);
							float temp = -1/VuSqrt(1 - cosAng*cosAng);
							float dang_dx = temp*dcosAng_dx;
							float dang_dy = temp*dcosAng_dy;

							//float u = (dist - mDesc.mInnerRadius)/(mDesc.mOuterRadius - mDesc.mInnerRadius);
							float du_dx = ddist_dx/(mDesc.mOuterRadius - mDesc.mInnerRadius);
							float du_dy = ddist_dy/(mDesc.mOuterRadius - mDesc.mInnerRadius);

							//u = -VU_PI + u*VU_2PI;
							du_dx = du_dx*VU_2PI;
							du_dy = du_dy*VU_2PI;

							//float magnitude = 0.5f*(1.0f + cos_u);
							float dmagnitude_dx = -0.5f*sin_u*du_dx;
							float dmagnitude_dy = -0.5f*sin_u*du_dy;

							//height = height + magnitude*mDesc.mHeight;
							float dheight_dx = dmagnitude_dx*mDesc.mHeight;
							float dheight_dy = dmagnitude_dy*mDesc.mHeight;

							//float angRatio = ang/mHalfAngularSize;
							float dangRatio_dx = dang_dx/mHalfAngularSize;
							float dangRatio_dy = dang_dy/mHalfAngularSize;

							//float angular_damping = 1;
							float dangular_damping_dx = 0;
							float dangular_damping_dy = 0;

							if ( angRatio > mDesc.mAngularDecayRatio )
							{
								//float v = (angRatio - mDesc.mAngularDecayRatio)/(1 - mDesc.mAngularDecayRatio)*VU_PI;
								float dv_dx = dangRatio_dx/(1 - mDesc.mAngularDecayRatio)*VU_PI;
								float dv_dy = dangRatio_dy/(1 - mDesc.mAngularDecayRatio)*VU_PI;

								//angular_damping = 0.5f*(1.0f + cos_v);
								dangular_damping_dx = -0.5f*sin_v*dv_dx;
								dangular_damping_dy = -0.5f*sin_v*dv_dy;
							}

							//height = height*angular_damping;
							dheight_dx = height*dangular_damping_dx + dheight_dx*angular_damping;
							dheight_dy = height*dangular_damping_dy + dheight_dy*angular_damping;

							// normal
							vert.mpDzDxy->mX += dheight_dx;
							vert.mpDzDxy->mY += dheight_dy;
						}
					}
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
