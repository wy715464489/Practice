//*****************************************************************************
//
//  Copyright (c) 2008-2008 Ralf Knoesel
//  Copyright (c) 2008-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  Water wake wave implementation
// 
//*****************************************************************************

#include <float.h>
#include "VuWaterWakeWave.h"
#include "VuWater.h"
#include "VuEngine/Gfx/Camera/VuCamera.h"
#include "VuEngine/HAL/Gfx/VuGfx.h"
#include "VuEngine/Gfx/Shaders/VuBasicShaders.h"
#include "VuEngine/Gfx/VuGfxUtil.h"
#include "VuEngine/Dev/VuDevProfile.h"


IMPLEMENT_RTTI(VuWaterWakeWave, VuWaterWave);
IMPLEMENT_RTTI(VuWaterFlatWakeWave, VuWaterWave);


//*****************************************************************************
VuWaterWakeWave::VuWaterWakeWave(const VuWaterWakeWaveDesc &desc, const VuWaterWakeWaveParams &params):
	mDesc(desc),
	mNode0(params),
	mNode1(params)
{
	updateBounds();
}

//*****************************************************************************
void VuWaterWakeWave::update(const VuWaterWakeWaveParams &params)
{
	mNode0 = params;

	// rebin
	updateBounds();
	VuWater::IF()->rebinWave(this);
}

//*****************************************************************************
bool VuWaterWakeWave::interpolate(const VuVector2 &vPos, VuWaterWakeWaveParams &node) const
{
	VuVector2 vSrc0(mNode0.mPosition.mX, mNode0.mPosition.mY);
	VuVector2 vSrc1(mNode1.mPosition.mX, mNode1.mPosition.mY);

	// calculate distances from edges
	float dist0 = VuDot(vPos - vSrc0, mNode0.mDirection);
	float dist1 = VuDot(vPos - vSrc1, mNode1.mDirection);

	// between edges?
	if ( dist0*dist1 < 0 )
	{
		// calculate interpolation ratios
		float total = dist1 - dist0;
		float ratio0 = dist1/total;
		float ratio1 = 1.0f - ratio0;

		// interpolate node data
		node.mPosition = ratio0*mNode0.mPosition + ratio1*mNode1.mPosition;
		node.mRange = ratio0*mNode0.mRange + ratio1*mNode1.mRange;

		// check if within influence
		VuVector2 vSrc(node.mPosition.mX, node.mPosition.mY);
		VuVector2 vDiff = vPos - vSrc;
		float dist_squared = vDiff.magSquared();
		if ( dist_squared < node.mRange*node.mRange )
		{
			// interpolate more node data
			node.mAge = ratio0*mNode0.mAge + ratio1*mNode1.mAge;
			node.mSpeed = ratio0*mNode0.mSpeed + ratio1*mNode1.mSpeed;

			// adjust distance and age
			float dist = VuSqrt(dist_squared);
			float raw_adjusted_dist = dist - node.mRange*mDesc.mRangeStartRatio;
			float adjusted_dist = VuMax(0.0f, raw_adjusted_dist);
			float age = node.mAge - adjusted_dist/node.mSpeed;

			if ( age > 0 )
			{
				// interpolate more node data
				node.mFalloffTime = ratio0*mNode0.mFalloffTime + ratio1*mNode1.mFalloffTime;
				node.mMagnitude = ratio0*mNode0.mMagnitude + ratio1*mNode1.mMagnitude;
				node.mDirection = ratio0*mNode0.mDirection + ratio1*mNode1.mDirection;
				node.mFrequency = ratio0*mNode0.mFrequency + ratio1*mNode1.mFrequency;
				node.mDecayTime = ratio0*mNode0.mDecayTime + ratio1*mNode1.mDecayTime;

				return true;
			}
		}
	}

	return false;
}

//*****************************************************************************
bool VuWaterWakeWave::tick(float fdt)
{
	mNode0.mAge += fdt;
	mNode1.mAge += fdt;

	if ( refCount() == 1 &&
		mNode0.mAge > mNode0.mFalloffTime &&
		mNode1.mAge > mNode1.mFalloffTime )
	{
		// done
		return false;
	}

	// not done
	return true;
}

//*****************************************************************************
void VuWaterWakeWave::getSurfaceData(VuWaterSurfaceDataParams &params)
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
void VuWaterWakeWave::debugDraw3d(const VuCamera &camera)
{
	VuGfxUtil *pGfxUtil = VuGfxUtil::IF();

	VuColor color = refCount() > 1 ? VuColor(255,255,0) : VuColor(192,192,192);

	// draw segment
	pGfxUtil->drawLine3d(color, mNode0.mPosition, mNode1.mPosition, camera.getViewProjMatrix());

	// calculate extents
	VuVector3 vLeft0, vRight0, vLeft1, vRight1;
	calculateExtents(mDesc, mNode0, mNode1, vLeft0, vRight0, vLeft1, vRight1);

	// draw outline
	pGfxUtil->drawLine3d(color, vLeft1, vRight1, camera.getViewProjMatrix());
	pGfxUtil->drawLine3d(color, vLeft0, vLeft1, camera.getViewProjMatrix());
	pGfxUtil->drawLine3d(color, vRight0, vRight1, camera.getViewProjMatrix());
}

//*****************************************************************************
void VuWaterWakeWave::debugDraw2d()
{
	VuGfxUtil *pGfxUtil = VuGfxUtil::IF();

	VuColor color = refCount() > 1 ? VuColor(255,255,0) : VuColor(192,192,192);

	// draw segment
	pGfxUtil->drawLine2d(0, color, VuVector2(mNode0.mPosition.mX, mNode0.mPosition.mY), VuVector2(mNode1.mPosition.mX, mNode1.mPosition.mY));

	// calculate extents
	VuVector3 vLeft0, vRight0, vLeft1, vRight1;
	calculateExtents(mDesc, mNode0, mNode1, vLeft0, vRight0, vLeft1, vRight1);

	// draw outline
	pGfxUtil->drawLine2d(0, color, VuVector2(vLeft1.mX, vLeft1.mY), VuVector2(vRight1.mX, vRight1.mY));
	pGfxUtil->drawLine2d(0, color, VuVector2(vLeft0.mX, vLeft0.mY), VuVector2(vLeft1.mX, vLeft1.mY));
	pGfxUtil->drawLine2d(0, color, VuVector2(vRight0.mX, vRight0.mY), VuVector2(vRight1.mX, vRight1.mY));
}

//*****************************************************************************
void VuWaterWakeWave::updateBounds()
{
	// aabb
	VuVector2 aabbMin = VuVector2(FLT_MAX, FLT_MAX);
	VuVector2 aabbMax = VuVector2(-FLT_MAX, -FLT_MAX);

	VuVector2 vPos0(mNode0.mPosition.mX, mNode0.mPosition.mY);
	VuVector2 vTangent0(mNode0.mDirection.mY, -mNode0.mDirection.mX);
	VuMinMax(vPos0 - vTangent0*mNode0.mRange, aabbMin, aabbMax);
	VuMinMax(vPos0 + vTangent0*mNode0.mRange, aabbMin, aabbMax);

	VuVector2 vPos1(mNode1.mPosition.mX, mNode1.mPosition.mY);
	VuVector2 vTangent1(mNode1.mDirection.mY, -mNode1.mDirection.mX);
	VuMinMax(vPos1 - vTangent1*mNode1.mRange, aabbMin, aabbMax);
	VuMinMax(vPos1 + vTangent1*mNode1.mRange, aabbMin, aabbMax);

	mBoundingAabb.mMin = VuVector3(aabbMin.mX, aabbMin.mY, VuMin(mNode0.mPosition.mZ, mNode1.mPosition.mZ));
	mBoundingAabb.mMax = VuVector3(aabbMax.mX, aabbMax.mY, VuMax(mNode0.mPosition.mZ, mNode1.mPosition.mZ));

	// disk
	mBoundingDiskCenter = 0.5f*(aabbMin + aabbMax);
	mBoundingDiskRadius = 0.5f*(aabbMin - aabbMax).mag();
}

//*****************************************************************************
void VuWaterWakeWave::calculateExtents(const VuWaterWakeWaveDesc &desc, const VuWaterWakeWaveParams &node0, const VuWaterWakeWaveParams &node1,
	VuVector3 &vLeft0, VuVector3 &vRight0, VuVector3 &vLeft1, VuVector3 &vRight1)
{
	// calculate tangents
	VuVector3 vTangent0(node0.mDirection.mY, -node0.mDirection.mX, 0);
	float fRangeStart0 = desc.mRangeStartRatio*node0.mRange;
	vTangent0 *= fRangeStart0 + VuMin(node0.mAge*node0.mSpeed, node0.mRange - fRangeStart0);
	vLeft0 =  node0.mPosition - vTangent0;
	vRight0 =  node0.mPosition + vTangent0;

	VuVector3 vTangent1(node1.mDirection.mY, -node1.mDirection.mX, 0);
	float fRangeStart1 = desc.mRangeStartRatio*node1.mRange;
	vTangent1 *= fRangeStart1 + VuMin(node1.mAge*node1.mSpeed, node0.mRange - fRangeStart1);
	vLeft1 = node1.mPosition - vTangent1;
	vRight1 = node1.mPosition + vTangent1;
}

//*****************************************************************************
template<int VERTEX_TYPE, int CLIP_TYPE>
void VuWaterWakeWave::getSurfaceData(VuWaterSurfaceDataParams &params)
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
			VuVector2 vPos2d(vert.mpPosition->mX, vert.mpPosition->mY);
			VuVector2 vSrc0(mNode0.mPosition.mX, mNode0.mPosition.mY);
			VuVector2 vSrc1(mNode1.mPosition.mX, mNode1.mPosition.mY);

			// calculate distances from edges
			float dist0 = VuDot(vPos2d - vSrc0, mNode0.mDirection);
			float dist1 = VuDot(vPos2d - vSrc1, mNode1.mDirection);

			// between edges?
			if ( dist0*dist1 < 0 )
			{
				// calculate interpolation ratios
				float total = dist1 - dist0;
				float ratio0 = dist1/total;
				float ratio1 = 1.0f - ratio0;

				// interpolate node data
				VuWaterWakeWaveParams node;
				node.mPosition = ratio0*mNode0.mPosition + ratio1*mNode1.mPosition;
				node.mRange = ratio0*mNode0.mRange + ratio1*mNode1.mRange;

				// check if within influence
				VuVector2 vSrc(node.mPosition.mX, node.mPosition.mY);
				VuVector2 vDiff = vPos2d - vSrc;
				float dist_squared = vDiff.magSquared();
				if ( dist_squared < node.mRange*node.mRange )
				{
					// interpolate more node data
					node.mAge = ratio0*mNode0.mAge + ratio1*mNode1.mAge;
					node.mSpeed = ratio0*mNode0.mSpeed + ratio1*mNode1.mSpeed;

					// adjust distance and age
					float dist = VuSqrt(dist_squared);
					float raw_adjusted_dist = dist - node.mRange*mDesc.mRangeStartRatio;
					float adjusted_dist = VuMax(0.0f, raw_adjusted_dist);
					float age = node.mAge - adjusted_dist/node.mSpeed;

					if ( age > 0 )
					{
						// interpolate more node data
						node.mFalloffTime = ratio0*mNode0.mFalloffTime + ratio1*mNode1.mFalloffTime;

						// check against falloff time
						if ( node.mAge < node.mFalloffTime )
						{
							// interpolate more node data
							node.mMagnitude = ratio0*mNode0.mMagnitude + ratio1*mNode1.mMagnitude;
							node.mFrequency = ratio0*mNode0.mFrequency + ratio1*mNode1.mFrequency;
							node.mDecayTime = ratio0*mNode0.mDecayTime + ratio1*mNode1.mDecayTime;

							float magnitude = node.mMagnitude;
							float foam = 0.25f;

							// lateral damping
							float inside_damping = 1;
							float outside_damping = 1;
							float dist_ratio = dist/node.mRange;
							if ( dist_ratio < mDesc.mRangeStartRatio )
							{
								inside_damping = dist_ratio/mDesc.mRangeStartRatio;
								magnitude = magnitude*inside_damping;
								foam = VuLerp(1.0f, foam, inside_damping);
							}
							if ( dist_ratio > mDesc.mRangeDecayRatio )
							{
								outside_damping = (1 - dist_ratio)/(1 - mDesc.mRangeDecayRatio);
								magnitude = magnitude*outside_damping;
								foam *= outside_damping;
							}

							// adjust magnitude based on decay time
							float raw_decay_mag = (node.mFalloffTime - node.mAge)/node.mDecayTime;
							float decay_mag = VuMin(raw_decay_mag, 1.0f);

							// calculate wave height
							magnitude = magnitude*decay_mag;

							float u = node.mFrequency*age;

							float sin_u, cos_u;
							VuSinCosEst(VuModAngle(u), sin_u, cos_u);

							float height = magnitude*sin_u;
							if ( VERTEX_TYPE == VuWaterSurfaceDataParams::VT_PHYSICS )
								height *= mDesc.mPhysicsScale;
							*vert.mpHeight += height;

							if ( VERTEX_TYPE == VuWaterSurfaceDataParams::VT_RENDER )
							{
								//float dist0 = VuDot(vPos2d - vSrc0, mNode0.mDirection);
								float ddist0_dx = mNode0.mDirection.mX;
								float ddist0_dy = mNode0.mDirection.mY;

								//float dist1 = VuDot(vPos2d - vSrc1, mNode1.mDirection);
								float ddist1_dx = mNode1.mDirection.mX;
								float ddist1_dy = mNode1.mDirection.mY;

								//float total = dist1 - dist0;
								float dtotal_dx = ddist1_dx - ddist0_dx;
								float dtotal_dy = ddist1_dy - ddist0_dy;

								//float ratio0 = dist1/total;
								float dratio0_dx = (total*ddist1_dx - dist1*dtotal_dx)/(total*total);
								float dratio0_dy = (total*ddist1_dy - dist1*dtotal_dy)/(total*total);

								//float ratio1 = 1.0f - ratio0;
								float dratio1_dx = -dratio0_dx;
								float dratio1_dy = -dratio0_dy;

								//VuWaterWakeWaveParams node;
								VuWaterWakeWaveParams dnode_dx;
								VuWaterWakeWaveParams dnode_dy;

								//node.mPosition = ratio0*mNode0.mPosition + ratio1*mNode1.mPosition;
								dnode_dx.mPosition = dratio0_dx*mNode0.mPosition + dratio1_dx*mNode1.mPosition;
								dnode_dy.mPosition = dratio0_dy*mNode0.mPosition + dratio1_dy*mNode1.mPosition;

								//node.mRange = ratio0*mNode0.mRange + ratio1*mNode1.mRange;
								dnode_dx.mRange = dratio0_dx*mNode0.mRange + dratio1_dx*mNode1.mRange;
								dnode_dy.mRange = dratio0_dy*mNode0.mRange + dratio1_dy*mNode1.mRange;

								//VuVector2 vSrc(node.mPosition.mX, node.mPosition.mY);
								VuVector2 dvSrc_dx(dnode_dx.mPosition.mX, dnode_dx.mPosition.mY);
								VuVector2 dvSrc_dy(dnode_dy.mPosition.mX, dnode_dy.mPosition.mY);

								//VuVector2 vDiff = vPos2d - vSrc;
								VuVector2 dvDiff_dx = VuVector2(1, 0) - dvSrc_dx;
								VuVector2 dvDiff_dy = VuVector2(0, 1) - dvSrc_dy;

								//float dist_squared = vDiff.magSquared();
								float ddist_squared_dx = 2*vDiff.mX*dvDiff_dx.mX + 2*vDiff.mY*dvDiff_dx.mY;
								float ddist_squared_dy = 2*vDiff.mX*dvDiff_dy.mX + 2*vDiff.mY*dvDiff_dy.mY;

								//float node.mAge = ratio0*mNode0.mAge + ratio1*mNode1.mAge;
								dnode_dx.mAge = dratio0_dx*mNode0.mAge + dratio1_dx*mNode1.mAge;
								dnode_dy.mAge = dratio0_dy*mNode0.mAge + dratio1_dy*mNode1.mAge;

								//node.mSpeed = ratio0*mNode0.mSpeed + ratio1*mNode1.mSpeed;
								dnode_dx.mSpeed = dratio0_dx*mNode0.mSpeed + dratio1_dx*mNode1.mSpeed;
								dnode_dy.mSpeed = dratio0_dy*mNode0.mSpeed + dratio1_dy*mNode1.mSpeed;

								//float dist = VuSqrt(dist_squared);
								dist = VuMax(dist, FLT_EPSILON);
								float ddist_dx = ddist_squared_dx/(2*dist);
								float ddist_dy = ddist_squared_dy/(2*dist);

								//float raw_adjusted_dist = dist - node.mRange*mDesc.mRangeStartRatio;
								float draw_adjusted_dist_dx = ddist_dx - dnode_dx.mRange*mDesc.mRangeStartRatio;
								float draw_adjusted_dist_dy = ddist_dy - dnode_dy.mRange*mDesc.mRangeStartRatio;

								//float adjusted_dist = VuMax(0.0f, raw_adjusted_dist);
								float dadjusted_dist_dx = draw_adjusted_dist_dx;
								float dadjusted_dist_dy = draw_adjusted_dist_dy;
								if ( raw_adjusted_dist < 0 )
								{
									dadjusted_dist_dx = 0;
									dadjusted_dist_dy = 0;
								}

								//float age = node.mAge - adjusted_dist/node.mSpeed;
								float dage_dx = dnode_dx.mAge - (node.mSpeed*dadjusted_dist_dx - adjusted_dist*dnode_dx.mSpeed)/(node.mSpeed*node.mSpeed);
								float dage_dy = dnode_dy.mAge - (node.mSpeed*dadjusted_dist_dy - adjusted_dist*dnode_dy.mSpeed)/(node.mSpeed*node.mSpeed);

								//node.mFalloffTime = ratio0*mNode0.mFalloffTime + ratio1*mNode1.mFalloffTime;
								dnode_dx.mFalloffTime = dratio0_dx*mNode0.mFalloffTime + dratio1_dx*mNode1.mFalloffTime;
								dnode_dy.mFalloffTime = dratio0_dy*mNode0.mFalloffTime + dratio1_dy*mNode1.mFalloffTime;

								//node.mMagnitude = ratio0*mNode0.mMagnitude + ratio1*mNode1.mMagnitude;
								dnode_dx.mMagnitude = dratio0_dx*mNode0.mMagnitude + dratio1_dx*mNode1.mMagnitude;
								dnode_dy.mMagnitude = dratio0_dy*mNode0.mMagnitude + dratio1_dy*mNode1.mMagnitude;

								//node.mFrequency = ratio0*mNode0.mFrequency + ratio1*mNode1.mFrequency;
								dnode_dx.mFrequency = dratio0_dx*mNode0.mFrequency + dratio1_dx*mNode1.mFrequency;
								dnode_dy.mFrequency = dratio0_dy*mNode0.mFrequency + dratio1_dy*mNode1.mFrequency;

								//node.mDecayTime = ratio0*mNode0.mDecayTime + ratio1*mNode1.mDecayTime;
								dnode_dx.mDecayTime = dratio0_dx*mNode0.mDecayTime + dratio1_dx*mNode1.mDecayTime;
								dnode_dy.mDecayTime = dratio0_dy*mNode0.mDecayTime + dratio1_dy*mNode1.mDecayTime;

								//float magnitude = node.mMagnitude;
								float dmagnitude_dx = 0;
								float dmagnitude_dy = 0;

								//float dist_ratio = dist/node.mRange;
								float ddist_ratio_dx = (node.mRange*ddist_dx - dist*dnode_dx.mRange)/(node.mRange*node.mRange);
								float ddist_ratio_dy = (node.mRange*ddist_dy - dist*dnode_dy.mRange)/(node.mRange*node.mRange);

								if ( dist_ratio < mDesc.mRangeStartRatio )
								{
									//inside_damping = dist_ratio/mDesc.mRangeStartRatio;
									float dinside_damping_dx = ddist_ratio_dx/mDesc.mRangeStartRatio;
									float dinside_damping_dy = ddist_ratio_dy/mDesc.mRangeStartRatio;

									//magnitude = magnitude*inside_damping;
									dmagnitude_dx = magnitude*dinside_damping_dx + inside_damping*dmagnitude_dx;
									dmagnitude_dy = magnitude*dinside_damping_dy + inside_damping*dmagnitude_dy;
								}
								if ( dist_ratio > mDesc.mRangeDecayRatio )
								{
									//float outside_damping = (1 - dist_ratio)/(1 - mDesc.mRangeDecayRatio);
									float doutside_damping_dx = ddist_ratio_dx/(1 - mDesc.mRangeDecayRatio);
									float doutside_damping_dy = ddist_ratio_dy/(1 - mDesc.mRangeDecayRatio);

									//magnitude = magnitude*damping;
									dmagnitude_dx = magnitude*doutside_damping_dx + outside_damping*dmagnitude_dx;
									dmagnitude_dy = magnitude*doutside_damping_dy + outside_damping*dmagnitude_dy;
								}

								//float raw_decay_mag = (node.mFalloffTime - node.mAge)/node.mDecayTime;
								float draw_decay_mag_dx;
								float draw_decay_mag_dy;
								{
									float u = node.mFalloffTime - node.mAge;
									float du_dx = dnode_dx.mFalloffTime - dnode_dx.mAge;
									float du_dy = dnode_dy.mFalloffTime - dnode_dy.mAge;
									//float raw_decay_mag = u/node.mDecayTime;
									draw_decay_mag_dx = (node.mDecayTime*du_dx - u*dnode_dx.mDecayTime)/(node.mDecayTime*node.mDecayTime);
									draw_decay_mag_dy = (node.mDecayTime*du_dy - u*dnode_dy.mDecayTime)/(node.mDecayTime*node.mDecayTime);
								}

								//float decay_mag = VuMin(raw_decay_mag, 1.0f);
								float ddecay_mag_dx = draw_decay_mag_dx;
								float ddecay_mag_dy = draw_decay_mag_dy;
								if ( raw_decay_mag > 1 )
								{
									ddecay_mag_dx = 0;
									ddecay_mag_dy = 0;
								}

								//magnitude = magnitude*decay_mag;
								dmagnitude_dx = magnitude*ddecay_mag_dx + decay_mag*dmagnitude_dx;
								dmagnitude_dy = magnitude*ddecay_mag_dy + decay_mag*dmagnitude_dy;

								//float u = node.mFrequency*age;
								float du_dx = node.mFrequency*dage_dx + age*dnode_dx.mFrequency;
								float du_dy = node.mFrequency*dage_dy + age*dnode_dy.mFrequency;

								//float height = magnitude*sin_u;
								float dheight_dx = magnitude*cos_u*du_dx + sin_u*dmagnitude_dx;
								float dheight_dy = magnitude*cos_u*du_dy + sin_u*dmagnitude_dy;

								vert.mpDzDxy->mX += dheight_dx;
								vert.mpDzDxy->mY += dheight_dy;
							}

							if ( VERTEX_TYPE == VuWaterSurfaceDataParams::VT_RENDER )
							{
								foam *= decay_mag;
								*vert.mpFoam += foam;
							}
						}
					}
				}
			}
		}

		// next vert
		vert.mpPosition = (VuPackedVector3 *)((VUBYTE *)vert.mpPosition + params.mStride);
		vert.mpHeight = (float *)((VUBYTE *)vert.mpHeight + params.mStride);
		if ( VERTEX_TYPE == VuWaterSurfaceDataParams::VT_RENDER )
		{
			vert.mpDzDxy = (VuPackedVector2 *)((VUBYTE *)vert.mpDzDxy + params.mStride);
			vert.mpFoam = (float *)((VUBYTE *)vert.mpFoam + params.mStride);
		}
	}
}

//*****************************************************************************
VuWaterFlatWakeWave::VuWaterFlatWakeWave(const VuWaterWakeWaveDesc &desc, const VuWaterWakeWaveParams &params):
	mDesc(desc),
	mNode0(params),
	mNode1(params)
{
	updateBounds();
}

//*****************************************************************************
void VuWaterFlatWakeWave::update(const VuWaterWakeWaveParams &params)
{
	mNode0 = params;

	// rebin
	updateBounds();
	VuWater::IF()->rebinWave(this);
}

//*****************************************************************************
bool VuWaterFlatWakeWave::tick(float fdt)
{
	mNode0.mAge += fdt;
	mNode1.mAge += fdt;

	if ( refCount() == 1 &&
		mNode0.mAge > mNode0.mFalloffTime &&
		mNode1.mAge > mNode1.mFalloffTime )
	{
		// done
		return false;
	}

	// not done
	return true;
}

//*****************************************************************************
void VuWaterFlatWakeWave::getSurfaceData(VuWaterSurfaceDataParams &params)
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
void VuWaterFlatWakeWave::debugDraw3d(const VuCamera &camera)
{
	VuGfxUtil *pGfxUtil = VuGfxUtil::IF();

	VuColor color = refCount() > 1 ? VuColor(255,255,0) : VuColor(192,192,192);

	// draw segment
	pGfxUtil->drawLine3d(color, mNode0.mPosition, mNode1.mPosition, camera.getViewProjMatrix());

	// calculate extents
	VuVector3 vLeft0, vRight0, vLeft1, vRight1;
	calculateExtents(mDesc, mNode0, mNode1, vLeft0, vRight0, vLeft1, vRight1);

	// draw outline
	pGfxUtil->drawLine3d(color, vLeft1, vRight1, camera.getViewProjMatrix());
	pGfxUtil->drawLine3d(color, vLeft0, vLeft1, camera.getViewProjMatrix());
	pGfxUtil->drawLine3d(color, vRight0, vRight1, camera.getViewProjMatrix());
}

//*****************************************************************************
void VuWaterFlatWakeWave::debugDraw2d()
{
	VuGfxUtil *pGfxUtil = VuGfxUtil::IF();

	VuColor color = refCount() > 1 ? VuColor(255,255,0) : VuColor(192,192,192);

	// draw segment
	pGfxUtil->drawLine2d(0, color, VuVector2(mNode0.mPosition.mX, mNode0.mPosition.mY), VuVector2(mNode1.mPosition.mX, mNode1.mPosition.mY));

	// calculate extents
	VuVector3 vLeft0, vRight0, vLeft1, vRight1;
	calculateExtents(mDesc, mNode0, mNode1, vLeft0, vRight0, vLeft1, vRight1);

	// draw outline
	pGfxUtil->drawLine2d(0, color, VuVector2(vLeft1.mX, vLeft1.mY), VuVector2(vRight1.mX, vRight1.mY));
	pGfxUtil->drawLine2d(0, color, VuVector2(vLeft0.mX, vLeft0.mY), VuVector2(vLeft1.mX, vLeft1.mY));
	pGfxUtil->drawLine2d(0, color, VuVector2(vRight0.mX, vRight0.mY), VuVector2(vRight1.mX, vRight1.mY));
}

//*****************************************************************************
void VuWaterFlatWakeWave::updateBounds()
{
	// aabb
	VuVector2 aabbMin = VuVector2(FLT_MAX, FLT_MAX);
	VuVector2 aabbMax = VuVector2(-FLT_MAX, -FLT_MAX);

	VuVector2 vPos0(mNode0.mPosition.mX, mNode0.mPosition.mY);
	VuVector2 vTangent0(mNode0.mDirection.mY, -mNode0.mDirection.mX);
	VuMinMax(vPos0 - vTangent0*mNode0.mRange, aabbMin, aabbMax);
	VuMinMax(vPos0 + vTangent0*mNode0.mRange, aabbMin, aabbMax);

	VuVector2 vPos1(mNode1.mPosition.mX, mNode1.mPosition.mY);
	VuVector2 vTangent1(mNode1.mDirection.mY, -mNode1.mDirection.mX);
	VuMinMax(vPos1 - vTangent1*mNode1.mRange, aabbMin, aabbMax);
	VuMinMax(vPos1 + vTangent1*mNode1.mRange, aabbMin, aabbMax);

	mBoundingAabb.mMin = VuVector3(aabbMin.mX, aabbMin.mY, VuMin(mNode0.mPosition.mZ, mNode1.mPosition.mZ));
	mBoundingAabb.mMax = VuVector3(aabbMax.mX, aabbMax.mY, VuMax(mNode0.mPosition.mZ, mNode1.mPosition.mZ));

	// disk
	mBoundingDiskCenter = 0.5f*(aabbMin + aabbMax);
	mBoundingDiskRadius = 0.5f*(aabbMin - aabbMax).mag();
}

//*****************************************************************************
void VuWaterFlatWakeWave::calculateExtents(const VuWaterWakeWaveDesc &desc, const VuWaterWakeWaveParams &node0, const VuWaterWakeWaveParams &node1,
	VuVector3 &vLeft0, VuVector3 &vRight0, VuVector3 &vLeft1, VuVector3 &vRight1)
{
	// calculate tangents
	VuVector3 vTangent0(node0.mDirection.mY, -node0.mDirection.mX, 0);
	float fRangeStart0 = desc.mRangeStartRatio*node0.mRange;
	vTangent0 *= fRangeStart0 + VuMin(node0.mAge*node0.mSpeed, node0.mRange - fRangeStart0);
	vLeft0 =  node0.mPosition - vTangent0;
	vRight0 =  node0.mPosition + vTangent0;

	VuVector3 vTangent1(node1.mDirection.mY, -node1.mDirection.mX, 0);
	float fRangeStart1 = desc.mRangeStartRatio*node1.mRange;
	vTangent1 *= fRangeStart1 + VuMin(node1.mAge*node1.mSpeed, node0.mRange - fRangeStart1);
	vLeft1 = node1.mPosition - vTangent1;
	vRight1 = node1.mPosition + vTangent1;
}

//*****************************************************************************
template<int VERTEX_TYPE, int CLIP_TYPE>
void VuWaterFlatWakeWave::getSurfaceData(VuWaterSurfaceDataParams &params)
{
	if ( VERTEX_TYPE == VuWaterSurfaceDataParams::VT_PHYSICS )
		return;

	VuWaterVertex vert;
	vert.configure(params.mpRenderVertex);

	for ( int iVert = 0; iVert < params.mVertCount; iVert++ )
	{
		if ( CLIP_TYPE == VuWaterSurfaceDataParams::CT_NOCLIP || params.mppWaterSurface[iVert] == params.mpWaterClipSurface )
		{
			VuVector2 vPos2d(vert.mpPosition->mX, vert.mpPosition->mY);
			VuVector2 vSrc0(mNode0.mPosition.mX, mNode0.mPosition.mY);
			VuVector2 vSrc1(mNode1.mPosition.mX, mNode1.mPosition.mY);

			// calculate distances from edges
			float dist0 = VuDot(vPos2d - vSrc0, mNode0.mDirection);
			float dist1 = VuDot(vPos2d - vSrc1, mNode1.mDirection);

			// between edges?
			if ( dist0*dist1 < 0 )
			{
				// calculate interpolation ratios
				float total = dist1 - dist0;
				float ratio0 = dist1/total;
				float ratio1 = 1.0f - ratio0;

				// interpolate node data
				VuWaterWakeWaveParams node;
				node.mPosition = ratio0*mNode0.mPosition + ratio1*mNode1.mPosition;
				node.mRange = ratio0*mNode0.mRange + ratio1*mNode1.mRange;

				// check if within influence
				VuVector2 vSrc(node.mPosition.mX, node.mPosition.mY);
				VuVector2 vDiff = vPos2d - vSrc;
				float dist_squared = vDiff.magSquared();
				if ( dist_squared < node.mRange*node.mRange )
				{
					// interpolate more node data
					node.mAge = ratio0*mNode0.mAge + ratio1*mNode1.mAge;
					node.mSpeed = ratio0*mNode0.mSpeed + ratio1*mNode1.mSpeed;

					// adjust distance and age
					float dist = VuSqrt(dist_squared);
					float raw_adjusted_dist = dist - node.mRange*mDesc.mRangeStartRatio;
					float adjusted_dist = VuMax(0.0f, raw_adjusted_dist);
					float age = node.mAge - adjusted_dist/node.mSpeed;

					if ( age > 0 )
					{
						// interpolate more node data
						node.mFalloffTime = ratio0*mNode0.mFalloffTime + ratio1*mNode1.mFalloffTime;

						// check against falloff time
						if ( node.mAge < node.mFalloffTime )
						{
							// interpolate more node data
							node.mDecayTime = ratio0*mNode0.mDecayTime + ratio1*mNode1.mDecayTime;

							float foam = 0.25f;

							// lateral damping
							float inside_damping = 1;
							float outside_damping = 1;
							float dist_ratio = dist/node.mRange;
							if ( dist_ratio < mDesc.mRangeStartRatio )
							{
								inside_damping = dist_ratio/mDesc.mRangeStartRatio;
								foam = VuLerp(1.0f, foam, inside_damping);
							}
							if ( dist_ratio > mDesc.mRangeDecayRatio )
							{
								outside_damping = (1 - dist_ratio)/(1 - mDesc.mRangeDecayRatio);
								foam *= outside_damping;
							}

							// adjust magnitude based on decay time
							float raw_decay_mag = (node.mFalloffTime - node.mAge)/node.mDecayTime;
							float decay_mag = VuMin(raw_decay_mag, 1.0f);

							foam *= decay_mag;
							*vert.mpFoam += foam;
						}
					}
				}
			}
		}

		// next vert
		vert.mpPosition = (VuPackedVector3 *)((VUBYTE *)vert.mpPosition + params.mStride);
		vert.mpHeight = (float *)((VUBYTE *)vert.mpHeight + params.mStride);
		vert.mpDzDxy = (VuPackedVector2 *)((VUBYTE *)vert.mpDzDxy + params.mStride);
		vert.mpFoam = (float *)((VUBYTE *)vert.mpFoam + params.mStride);
	}
}
