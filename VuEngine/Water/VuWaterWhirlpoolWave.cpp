//*****************************************************************************
//
//  Copyright (c) 2008-2008 Ralf Knoesel
//  Copyright (c) 2008-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  Water whirlpool wave implementation
// 
//*****************************************************************************

#include <float.h>
#include "VuWaterWhirlpoolWave.h"
#include "VuWater.h"
#include "VuEngine/Gfx/Camera/VuCamera.h"
#include "VuEngine/HAL/Gfx/VuGfx.h"
#include "VuEngine/Gfx/Shaders/VuBasicShaders.h"
#include "VuEngine/Gfx/VuGfxUtil.h"
#include "VuEngine/Dev/VuDevProfile.h"
#include "VuEngine/Math/VuAabb.h"


IMPLEMENT_RTTI(VuWaterWhirlpoolWave, VuWaterWave);


//*****************************************************************************
VuWaterWhirlpoolWave::VuWaterWhirlpoolWave(const VuWaterWhirlpoolWaveDesc &desc):
	mAge(0)
{
	setDesc(desc);

	updateBounds();
}

//*****************************************************************************
void VuWaterWhirlpoolWave::modify(const VuWaterWhirlpoolWaveDesc &desc)
{
	setDesc(desc);

	updateBounds();
	VuWater::IF()->rebinWave(this);
}

//*****************************************************************************
bool VuWaterWhirlpoolWave::tick(float fdt)
{
	mAge += fdt;

	if ( refCount() == 1 )
		return false; // done

	return true; // not done
}

//*****************************************************************************
void VuWaterWhirlpoolWave::getSurfaceData(VuWaterSurfaceDataParams &params)
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
void VuWaterWhirlpoolWave::debugDraw3d(const VuCamera &camera)
{
	VuGfxUtil *pGfxUtil = VuGfxUtil::IF();

	VuMatrix matMVP;
	matMVP.loadIdentity();
	matMVP.translate(mDesc.mPos);
	matMVP *= camera.getViewProjMatrix();

	VuColor col(255,255,0);

	int numSides = 16;
	float fStep = 2.0f*VU_PI/numSides;
	float fCurAngle = 0.0f;
	float fNextAngle = fStep;
	for ( int iSide = 0; iSide < numSides; iSide++ )
	{
		VuVector3 v0 = VuVector3(VuCos(fCurAngle), VuSin(fCurAngle), 0);
		VuVector3 v1 = VuVector3(VuCos(fNextAngle), VuSin(fNextAngle), 0);

		VuVector3 vi0 = mDesc.mInnerRadius*v0 + VuVector3(0, 0, -mDesc.mDepth);
		VuVector3 vi1 = mDesc.mInnerRadius*v1 + VuVector3(0, 0, -mDesc.mDepth);
		VuVector3 vo0 = mDesc.mOuterRadius*v0;
		VuVector3 vo1 = mDesc.mOuterRadius*v1;

		pGfxUtil->drawLine3d(col, vi0, vo0, matMVP);
		pGfxUtil->drawLine3d(col, vi0, vi1, matMVP);
		pGfxUtil->drawLine3d(col, vo0, vo1, matMVP);

		fCurAngle = fNextAngle;
		fNextAngle += fStep;
	}
}

//*****************************************************************************
void VuWaterWhirlpoolWave::debugDraw2d()
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
void VuWaterWhirlpoolWave::setDesc(const VuWaterWhirlpoolWaveDesc &desc)
{
	mDesc = desc;
}

//*****************************************************************************
void VuWaterWhirlpoolWave::updateBounds()
{
	// aabb
	VuVector2 vCenter(mDesc.mPos.mX, mDesc.mPos.mY);
	VuVector2 vExtents(mDesc.mOuterRadius, mDesc.mOuterRadius);

	mBoundingAabb.mMin = mDesc.mPos - VuVector3(vExtents.mX, vExtents.mY, mDesc.mDepth);
	mBoundingAabb.mMax = mDesc.mPos + VuVector3(vExtents.mX, vExtents.mY, 0);

	// disk
	mBoundingDiskCenter = vCenter;
	mBoundingDiskRadius = mDesc.mOuterRadius;
}

//*****************************************************************************
template<int VERTEX_TYPE, int CLIP_TYPE>
void VuWaterWhirlpoolWave::getSurfaceData(VuWaterSurfaceDataParams &params)
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
				float height = -mDesc.mDepth;
				if ( dist_squared > mDesc.mInnerRadius*mDesc.mInnerRadius )
				{
					float dist = VuSqrt(dist_squared) + FLT_EPSILON;
					float u = (dist - mDesc.mInnerRadius)/(mDesc.mOuterRadius - mDesc.mInnerRadius);
					float magnitude = VuSqrt(u);
					height = height + magnitude*mDesc.mDepth;

					if ( VERTEX_TYPE == VuWaterSurfaceDataParams::VT_RENDER )
					{
						//float dist_squared = vPos.magSquared();
						//float dist = VuSqrt(dist_squared);
						float ddist_dx = vPos.mX/dist;
						float ddist_dy = vPos.mY/dist;

						//float u = (dist - mDesc.mInnerRadius)/(mDesc.mOuterRadius - mDesc.mInnerRadius);
						float du_dx = ddist_dx/(mDesc.mOuterRadius - mDesc.mInnerRadius);
						float du_dy = ddist_dy/(mDesc.mOuterRadius - mDesc.mInnerRadius);

						//float magnitude = VuSqrt(u);
						float dmagnitude_dx = 0.5f*du_dx/magnitude;
						float dmagnitude_dy = 0.5f*du_dy/magnitude;

						//height = height + magnitude*mDesc.mDepth;
						float dheight_dx = dmagnitude_dx*mDesc.mDepth;
						float dheight_dy = dmagnitude_dy*mDesc.mDepth;

						// normal
						vert.mpDzDxy->mX += dheight_dx;
						vert.mpDzDxy->mY += dheight_dy;

						// foam
						*vert.mpFoam += (1 - u)*mDesc.mFoaminess;
					}
					if ( VERTEX_TYPE == VuWaterSurfaceDataParams::VT_PHYSICS )
					{
						VuVector2 vNormalizedPos = vPos/dist;

						// angular component
						VuVector3 vAngVel = mDesc.mAngularSpeed*VuVector3(-vNormalizedPos.mY, vNormalizedPos.mX, 0);

						// linear component
						//float u = (dist - mDesc.mInnerRadius)/(mDesc.mOuterRadius - mDesc.mInnerRadius);
						float du_ddist = 1/(mDesc.mOuterRadius - mDesc.mInnerRadius);
						//float magnitude = VuSqrt(u);
						float dmagnitude_ddist = 0.5f*du_ddist/magnitude;
						//height = height + magnitude*mDesc.mDepth;
						float dheight_ddist = dmagnitude_ddist*mDesc.mDepth;
						VuVector3 vLinComp = VuVector3(-vNormalizedPos.mX, -vNormalizedPos.mY, -dheight_ddist).normal();
						VuVector3 vLinVel = mDesc.mLinearSpeed*vLinComp;

						float vel_magnitude = 1 - magnitude;
						VuVector3 vVel = vel_magnitude*(vAngVel + vLinVel);

						vert.mpDxyzDt->mX += vVel.mX;
						vert.mpDxyzDt->mY += vVel.mY;
						vert.mpDxyzDt->mZ += vVel.mZ;
					}
				}
				else
				{
					if ( VERTEX_TYPE == VuWaterSurfaceDataParams::VT_RENDER )
					{
						*vert.mpFoam += mDesc.mFoaminess;
					}
				}

				*vert.mpHeight += height;
			}
		}

		// next vert
		vert.mpPosition = (VuPackedVector3 *)((VUBYTE *)vert.mpPosition + params.mStride);
		vert.mpHeight = (float *)((VUBYTE *)vert.mpHeight + params.mStride);
		if ( VERTEX_TYPE == VuWaterSurfaceDataParams::VT_PHYSICS )
		{
			vert.mpDxyzDt = (VuPackedVector3 *)((VUBYTE *)vert.mpDxyzDt + params.mStride);
		}
		else
		{
			vert.mpDzDxy = (VuPackedVector2 *)((VUBYTE *)vert.mpDzDxy + params.mStride);
			vert.mpFoam = (float *)((VUBYTE *)vert.mpFoam + params.mStride);
		}
	}
}
