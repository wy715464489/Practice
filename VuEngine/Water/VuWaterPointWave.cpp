//*****************************************************************************
//
//  Copyright (c) 2009-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  Water point wave class
// 
//*****************************************************************************

#include "VuWaterPointWave.h"
#include "VuWater.h"
#include "VuEngine/Gfx/VuGfxUtil.h"


IMPLEMENT_RTTI(VuWaterPointWave, VuWaterWave);


//*****************************************************************************
VuWaterPointWave::VuWaterPointWave(const VuWaterPointWaveDesc &desc):
	mAge(0),
	mpCallbackIF(VUNULL),
	mbExpired(false)
{
	setDesc(desc);

	updateBounds();
}

//*****************************************************************************
bool VuWaterPointWave::tick(float fdt)
{
	mAge += fdt;

	if (mAge > (mDesc.mRangeEnd - mDesc.mRangeStart)*mInvSpeed + mDesc.mFalloffTime)
	{
		if ( !mbExpired && mpCallbackIF )
			mpCallbackIF->onPointWaveExpired();
		mbExpired = true;

		if ( refCount() == 1 )
		{
			// done
			return false;
		}
	}

	// not done
	return true;
}

//*****************************************************************************
void VuWaterPointWave::getSurfaceData(VuWaterSurfaceDataParams &params)
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
void VuWaterPointWave::debugDraw2d()
{
	VuGfxUtil *pGfxUtil = VuGfxUtil::IF();

	// set up matrix
	VuMatrix mat = pGfxUtil->getMatrix();
	mat.translateLocal(mDesc.mPos);
	mat.scaleLocal(VuVector3(mDesc.mRangeEnd, mDesc.mRangeEnd, 1));

	pGfxUtil->pushMatrix(mat);
	{
		pGfxUtil->drawEllipseOutline2d(0, VuColor(192,192,192), VuRect(-1, -1, 2, 2), 32);
	}
	pGfxUtil->popMatrix();
}

//*****************************************************************************
void VuWaterPointWave::setDesc(const VuWaterPointWaveDesc &desc)
{
	mDesc = desc;

	mInvSpeed = 1.0f/mDesc.mSpeed;
	mInvRangeEndMinusRangeStart = 1.0f/(mDesc.mRangeEnd - mDesc.mRangeStart);
	mInvFalloffTime = 1.0f/mDesc.mFalloffTime;
	mInvMagnitude = 1.0f/mDesc.mMagnitude;
}

//*****************************************************************************
void VuWaterPointWave::updateBounds()
{
	// aabb
	VuVector2 vCenter(mDesc.mPos.mX, mDesc.mPos.mY);
	VuVector2 vExtents(mDesc.mRangeEnd, mDesc.mRangeEnd);

	VuVector2 aabbMin = vCenter - vExtents;
	VuVector2 aabbMax = vCenter + vExtents;

	mBoundingAabb.mMin = VuVector3(aabbMin.mX, aabbMin.mY, mDesc.mPos.mZ);
	mBoundingAabb.mMax = VuVector3(aabbMax.mX, aabbMax.mY, mDesc.mPos.mZ);

	// disk
	mBoundingDiskCenter = vCenter;
	mBoundingDiskRadius = mDesc.mRangeEnd;
}

//*****************************************************************************
template<int VERTEX_TYPE, int CLIP_TYPE>
void VuWaterPointWave::getSurfaceData(VuWaterSurfaceDataParams &params)
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
			// transform into local space
			float localX = vert.mpPosition->mX - mDesc.mPos.mX;
			float localY = vert.mpPosition->mY - mDesc.mPos.mY;

			// check if within range
			float dist = VuSqrt(localX*localX + localY*localY);
			if ( dist < mDesc.mRangeEnd )
			{
				// adjust distance
				float adjDist = VuMax(dist - mDesc.mRangeStart, 0.0f);

				float startTime = adjDist*mInvSpeed;
				if ( mAge > startTime )
				{
					float fMag = mDesc.mMagnitude*(mDesc.mRangeEnd - mDesc.mRangeStart - adjDist)*mInvRangeEndMinusRangeStart;
					float endTime = startTime + mDesc.mFalloffTime;
					if ( mAge < endTime )
					{
						if ( fMag > 0 )
						{
							float u = mDesc.mFrequency*(mAge - startTime);
							float v = mInvFalloffTime*(endTime - mAge);

							float sin_u, cos_u;
							VuSinCos(u, sin_u, cos_u);

							float w = fMag*v;
							float height = w*sin_u;

							*vert.mpHeight += height;

							if ( VERTEX_TYPE == VuWaterSurfaceDataParams::VT_PHYSICS )
							{
								//float u = mDesc.mFrequency*(mAge - startTime);
								float du_dt = mDesc.mFrequency;

								//float v = mInvFalloffTime*(endTime - mAge);
								float dv_dt = -mInvFalloffTime;

								//float w = fMag*v; (dfMag_dt = 0)
								float dw_dt = fMag*dv_dt;

								//float height = w*sin_u;
								float dheight_dt = w*cos_u*du_dt + sin_u*dw_dt;

								// velocity
								vert.mpDxyzDt->mZ += dheight_dt;
							}

							if ( VERTEX_TYPE == VuWaterSurfaceDataParams::VT_RENDER )
							{
								//float dist = VuSqrt(localX*localX + localY*localY);
								float ddist_dx = localX/dist;
								float ddist_dy = localY/dist;

								//float adjDist = VuMax(dist - mDesc.mRangeStart, 0);
								float dadjDist_dx = VuSelect(dist - mDesc.mRangeStart, ddist_dx, 0.0f);
								float dadjDist_dy = VuSelect(dist - mDesc.mRangeStart, ddist_dy, 0.0f);

								//float startTime = adjDist*mInvSpeed;
								float dstartTime_dx = dadjDist_dx*mInvSpeed;
								float dstartTime_dy = dadjDist_dy*mInvSpeed;

								//float fMag = mDesc.mMagnitude*(mDesc.mRangeEnd - mDesc.mRangeStart - adjDist)*mInvRangeEndMinusRangeStart;
								float dfMag_dx = -dadjDist_dx*mDesc.mMagnitude*mInvRangeEndMinusRangeStart;
								float dfMag_dy = -dadjDist_dy*mDesc.mMagnitude*mInvRangeEndMinusRangeStart;

								//float endTime = startTime + mDesc.mFalloffTime;
								float dendTime_dx = dstartTime_dx;
								float dendTime_dy = dstartTime_dy;

								//float u = mDesc.mFrequency*(mAge - startTime);
								float du_dx = -mDesc.mFrequency*dstartTime_dx;
								float du_dy = -mDesc.mFrequency*dstartTime_dy;

								//float v = mInvFalloffTime*(endTime - mAge);
								float dv_dx = mInvFalloffTime*dendTime_dx;
								float dv_dy = mInvFalloffTime*dendTime_dy;

								//float w = fMag*v;
								float dw_dx = fMag*dv_dx + v*dfMag_dx;
								float dw_dy = fMag*dv_dy + v*dfMag_dy;

								//float height = w*sin_u;
								float dheight_dx = w*cos_u*du_dx + sin_u*dw_dx;
								float dheight_dy = w*cos_u*du_dy + sin_u*dw_dy;

								// normal
								vert.mpDzDxy->mX += dheight_dx;
								vert.mpDzDxy->mY += dheight_dy;
							}
						}
					}

					if ( VERTEX_TYPE == VuWaterSurfaceDataParams::VT_RENDER )
					{
						// foam
						float foam = 1.0f - (mAge - mDesc.mFalloffTime)*mDesc.mSpeed*mInvRangeEndMinusRangeStart;
						foam = VuMin(foam, 1.0f);
						foam *= fMag*mInvMagnitude*mDesc.mFoaminess;
						*vert.mpFoam += foam;
					}
				}
			}
		}

		// next vert
		vert.mpPosition = (VuPackedVector3 *)((VUBYTE *)vert.mpPosition + params.mStride);
		vert.mpHeight = (float *)((VUBYTE *)vert.mpHeight + params.mStride);
		if ( VERTEX_TYPE == VuWaterSurfaceDataParams::VT_PHYSICS )
		{
			vert.mpDxyzDt = (VuPackedVector3 *)((VUBYTE *)vert.mpDxyzDt + params.mStride);
		}
		if ( VERTEX_TYPE == VuWaterSurfaceDataParams::VT_RENDER )
		{
			vert.mpDzDxy = (VuPackedVector2 *)((VUBYTE *)vert.mpDzDxy + params.mStride);
			vert.mpFoam = (float *)((VUBYTE *)vert.mpFoam + params.mStride);
		}
	}
}
