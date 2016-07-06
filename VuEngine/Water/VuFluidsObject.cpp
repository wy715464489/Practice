//*****************************************************************************
//
//  Copyright (c) 2007-2008 Ralf Knoesel
//  Copyright (c) 2008-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  FluidsObject class
// 
//*****************************************************************************

#include "VuFluidsObject.h"
#include "VuWater.h"
#include "VuEngine/Assets/VuFluidsMeshAsset.h"
#include "VuEngine/Assets/VuAssetFactory.h"
#include "VuEngine/Dynamics/VuDynamics.h"
#include "VuEngine/Math/VuMatrix.h"
#include "VuEngine/Math/VuAabb.h"
#include "VuEngine/Dev/VuDev.h"


struct TriElement
{
	const VuVector3	*mpPos0;
	const VuVector3	*mpPos1;
	const VuVector3	*mpPos2;
	const VuVector3	*mpWaterVel0;
	const VuVector3	*mpWaterVel1;
	const VuVector3	*mpWaterVel2;
	float			mWaterHeight0;
	float			mWaterHeight1;
	float			mWaterHeight2;
	float			mArea;
};


//*****************************************************************************
VuFluidsObject::VuFluidsObject():
	mpFluidsMeshAsset(VUNULL),
	mHydrodynamicCenter(0,0,0),
	mWaterDragDensity(1000.0f),
	mWaterBuoyancyDensity(1000.0f),
	mLinVelFactor(1.0f),
	mSkinFrictionCoeff(0,0,0),
	mIgnoreWaveCount(0),
	mSubmerged(false),
	mSubmergedVolume(0),
	mSubmergedArea(0),
	mMinWaterHeight(FLT_MAX),
	mTotalForce(0,0,0),
	mTotalTorque(0,0,0),
	mBuoyancyForce(0,0,0),
	mBuoyancyTorque(0,0,0),
	mDragForce(0,0,0),
	mDragTorque(0,0,0),
	mDampingForce(0,0,0),
	mDampingTorque(0,0,0),
	mAvgWaterNormal(0,0,1),
	mAvgWaterVel(0,0,0),
	mMinPressureForce(-FLT_MAX)
{
	mppIgnoreWaves = new const VuWaterWave *[VuWaterSurfaceDataParams::MAX_IGNORE_WAVE_COUNT];
}

//*****************************************************************************
VuFluidsObject::~VuFluidsObject()
{
	if ( mpFluidsMeshAsset )
		VuAssetFactory::IF()->releaseAsset(mpFluidsMeshAsset);

	delete[] mppIgnoreWaves;
}

//*****************************************************************************
void VuFluidsObject::setAsset(const std::string &assetName)
{
	// release old asset
	if ( mpFluidsMeshAsset )
	{
		VuAssetFactory::IF()->releaseAsset(mpFluidsMeshAsset);
		mpFluidsMeshAsset = VUNULL;
	}

	// load new asset
	if ( VuAssetFactory::IF()->doesAssetExist<VuFluidsMeshAsset>(assetName) )
		mpFluidsMeshAsset = VuAssetFactory::IF()->createAsset<VuFluidsMeshAsset>(assetName);

	// re-allocate arrays
	mTransformedVerts.clear();
	mTransformedEdges.clear();
	if ( mpFluidsMeshAsset )
	{
		mTransformedVerts.resize(mpFluidsMeshAsset->getVertCount());
		mTransformedEdges.resize(mpFluidsMeshAsset->getEdgeCount());

		memset(&mTransformedVerts[0], 0, sizeof(mTransformedVerts[0])*mTransformedVerts.size());
		memset(&mTransformedEdges[0], 0, sizeof(mTransformedEdges[0])*mTransformedEdges.size());
	}
}

//*****************************************************************************
void VuFluidsObject::setIgnoreWaveCount(int count)
{
	VUASSERT(count <= VuWaterSurfaceDataParams::MAX_IGNORE_WAVE_COUNT, "VuFluidsObject::setIgnoreWaveCount() max exceeded");

	mIgnoreWaveCount = count;
}

//*****************************************************************************
void VuFluidsObject::setTransform(const VuMatrix &matModel)
{
	if ( !mpFluidsMeshAsset )
		return;

	// reset counters
	mMinWaterHeight = FLT_MAX;

	int vertCount = mpFluidsMeshAsset->getVertCount();
	int edgeCount = mpFluidsMeshAsset->getEdgeCount();

	// transform verts
	{
		for ( int i = 0; i < vertCount; i++ )
		{
			mTransformedVerts[i].mPos = matModel.transform(mpFluidsMeshAsset->getVerts()[i]);
			mTransformedVerts[i].mWaterVel = VuVector3(0,0,0);
		}
	}

	// aabb center
	VuVector3 vCenter = matModel.transform(mpFluidsMeshAsset->getAabb().getCenter());

	// get surface data
	VuWaterSurfaceDataParams params(VuWaterSurfaceDataParams::VT_PHYSICS);
	params.mVertCount = vertCount;
	params.mBoundingAabb = VuAabb(mpFluidsMeshAsset->getAabb(), matModel);
	params.mBoundingCenter = vCenter;
	params.mBoundingRadius = mpFluidsMeshAsset->getAabb().getExtents().mag2d();

	params.mIgnoreWaveCount = mIgnoreWaveCount;
	for ( int i = 0; i < mIgnoreWaveCount; i++ )
		params.mapIgnoreWaves[i] = mppIgnoreWaves[i];

	params.mpPhysicsVertex = (VuWaterPhysicsVertex *)&mTransformedVerts[0];
	params.mStride = sizeof(mTransformedVerts[0]);

	VuWater::IF()->getSurfaceData(params);

	// calculate vert properties
	VUUINT32 submerged = 0;
	for ( int i = 0; i < vertCount; i++ )
	{
		float fRelHeight = mTransformedVerts[i].mPos.mZ - mTransformedVerts[i].mWaterHeight;
		mTransformedVerts[i].mSubmerged = fRelHeight < 0;
		mMinWaterHeight = VuMin(mMinWaterHeight, fRelHeight);
		submerged |= mTransformedVerts[i].mSubmerged;
	}
	mSubmerged = submerged ? true : false;

	// calculate edge intersections with water surface
	for ( int i = 0; i < edgeCount; i++ )
	{
		const VuFluidsMeshAsset::Edge &edge = mpFluidsMeshAsset->getEdges()[i];
		TransformedEdge &xedge = mTransformedEdges[i];
		const TransformedVert &v0 = mTransformedVerts[edge.vi0];
		const TransformedVert &v1 = mTransformedVerts[edge.vi1];
		xedge.mPartiallySubmerged = v0.mSubmerged ^ v1.mSubmerged;
		if ( xedge.mPartiallySubmerged )
		{
			float fRelHeight0 = v0.mPos.mZ - v0.mWaterHeight;
			float fRelHeight1 = v1.mPos.mZ - v1.mWaterHeight;
			float fRatio = (0 - fRelHeight0)/(fRelHeight1 - fRelHeight0);
			xedge.mIntersectionPos = v0.mPos + fRatio*(v1.mPos - v0.mPos);
			xedge.mIntersectionWaterVel = v0.mWaterVel + fRatio*(v1.mWaterVel - v0.mWaterVel);
			xedge.mIntersectionWaterHeight = v0.mWaterHeight + fRatio*(v1.mWaterHeight - v0.mWaterHeight);
		}
	}
}

//*****************************************************************************
void VuFluidsObject::updateForces(float fdt, btRigidBody &rb)
{
	if ( !mpFluidsMeshAsset )
		return;

	if ( rb.getInvMass() == 0 )
		return;

	mBuoyancyForce = VuVector3(0,0,0);
	mBuoyancyTorque = VuVector3(0,0,0);
	mDragForce = VuVector3(0,0,0);
	mDragTorque = VuVector3(0,0,0);
	mDampingForce = VuVector3(0,0,0);
	mDampingTorque = VuVector3(0,0,0);
	mAvgWaterNormal = VuVector3(0,0,0);
	mAvgWaterVel = VuVector3(0,0,0);

	int triCount = mpFluidsMeshAsset->getTriCount();

	VuMatrix matRB = VuDynamicsUtil::toVuMatrix(rb.getCenterOfMassTransform());
	matRB.translateLocal(mHydrodynamicCenter);
	VuVector3 vRBLinearVel = mLinVelFactor*VuDynamicsUtil::toVuVector3(rb.getLinearVelocity());
	VuVector3 vRBAngularVel = VuDynamicsUtil::toVuVector3(rb.getAngularVelocity());
	VuVector3 vRBPos = matRB.getTrans();
	float fMass = 1.0f/rb.getInvMass();

	VuVector3 vGravity = VuDynamics::IF()->getGravity();

	// reset counters
	mSubmergedVolume = 0;
	mSubmergedArea = 0;

	// integrate force and torque
	for ( int iTri = 0; iTri < triCount; iTri++ )
	{
		const VuFluidsMeshAsset::Tri &tri = mpFluidsMeshAsset->getTris()[iTri];

		const TransformedVert *pv0 = &mTransformedVerts[tri.vi0];
		const TransformedVert *pv1 = &mTransformedVerts[tri.vi1];
		const TransformedVert *pv2 = &mTransformedVerts[tri.vi2];

		// transform triangle normal
		VuVector3 vTransformedTriNormal = matRB.transformNormal(tri.mNormal);

		// count how many verts are submerged
		VUUINT32 submergedCount = pv0->mSubmerged + pv1->mSubmerged + pv2->mSubmerged;
		if ( submergedCount )
		{
			// tessellate into tri elements
			TriElement triElements[3];
			int triElementCount = 0;

			if ( submergedCount == 3 )
			{
				// all verts below water
				triElementCount = 1;

				triElements[0].mpPos0 = &pv0->mPos;
				triElements[0].mpPos1 = &pv1->mPos;
				triElements[0].mpPos2 = &pv2->mPos;

				triElements[0].mpWaterVel0 = &pv0->mWaterVel;
				triElements[0].mpWaterVel1 = &pv1->mWaterVel;
				triElements[0].mpWaterVel2 = &pv2->mWaterVel;

				triElements[0].mWaterHeight0 = pv0->mWaterHeight;
				triElements[0].mWaterHeight1 = pv1->mWaterHeight;
				triElements[0].mWaterHeight2 = pv2->mWaterHeight;

				triElements[0].mArea = tri.mArea;
			}
			else
			{
				// partially submerged
				const TransformedEdge *pe0 = &mTransformedEdges[tri.ei0];
				const TransformedEdge *pe1 = &mTransformedEdges[tri.ei1];
				const TransformedEdge *pe2 = &mTransformedEdges[tri.ei2];

				// rearrange tri so that e0 and e2 are intersections
				if ( !pe0->mPartiallySubmerged )
				{
					VuSwap<const TransformedVert *>(pv0, pv2); VuSwap<const TransformedEdge *>(pe0, pe2);
					VuSwap<const TransformedVert *>(pv1, pv2); VuSwap<const TransformedEdge *>(pe1, pe2);
				}
				else if ( !pe2->mPartiallySubmerged )
				{
					VuSwap<const TransformedVert *>(pv0, pv1); VuSwap<const TransformedEdge *>(pe0, pe1);
					VuSwap<const TransformedVert *>(pv1, pv2); VuSwap<const TransformedEdge *>(pe1, pe2);
				}

				// build tri elements
				TriElement *pte = triElements;
				if ( pv0->mSubmerged )
				{
					pte->mpPos0 = &pv0->mPos;
					pte->mpPos1 = &pe0->mIntersectionPos;
					pte->mpPos2 = &pe2->mIntersectionPos;

					pte->mpWaterVel0 = &pv0->mWaterVel;
					pte->mpWaterVel1 = &pe0->mIntersectionWaterVel;
					pte->mpWaterVel2 = &pe2->mIntersectionWaterVel;

					pte->mWaterHeight0 = pv0->mWaterHeight;
					pte->mWaterHeight1 = pe0->mIntersectionWaterHeight;
					pte->mWaterHeight2 = pe2->mIntersectionWaterHeight;

					pte->mArea = 0.5f*VuCross(pe0->mIntersectionPos - pv0->mPos, pe2->mIntersectionPos - pv0->mPos).mag();

					triElementCount++;
					pte++;
				}

				if ( pv1->mSubmerged )
				{
					pte->mpPos0 = &pv1->mPos;
					pte->mpPos1 = &pe2->mIntersectionPos;
					pte->mpPos2 = &pe0->mIntersectionPos;

					pte->mpWaterVel0 = &pv1->mWaterVel;
					pte->mpWaterVel1 = &pe2->mIntersectionWaterVel;
					pte->mpWaterVel2 = &pe0->mIntersectionWaterVel;

					pte->mWaterHeight0 = pv1->mWaterHeight;
					pte->mWaterHeight1 = pe2->mIntersectionWaterHeight;
					pte->mWaterHeight2 = pe0->mIntersectionWaterHeight;

					pte->mArea = 0.5f*VuCross(pe2->mIntersectionPos - pv1->mPos, pe0->mIntersectionPos - pv1->mPos).mag();

					triElementCount++;
					pte++;
				}

				if ( pv2->mSubmerged )
				{
					pte->mpPos0 = &pv2->mPos;
					pte->mpPos1 = &pe2->mIntersectionPos;
					pte->mpPos2 = &pv1->mPos;

					pte->mpWaterVel0 = &pv2->mWaterVel;
					pte->mpWaterVel1 = &pe2->mIntersectionWaterVel;
					pte->mpWaterVel2 = &pv1->mWaterVel;

					pte->mWaterHeight0 = pv2->mWaterHeight;
					pte->mWaterHeight1 = pe2->mIntersectionWaterHeight;
					pte->mWaterHeight2 = pv1->mWaterHeight;

					pte->mArea = 0.5f*VuCross(pe2->mIntersectionPos - pv2->mPos, pv1->mPos - pv2->mPos).mag();

					triElementCount++;
					pte++;
				}
			}

			// add force and torque for elements
			for ( int iElement = 0; iElement < triElementCount; iElement++ )
			{
				TriElement &triElement = triElements[iElement];

				// calculate water normal
				VuVector3 vWaterNormal;
				{
					VuVector3 sv0(triElement.mpPos0->mX, triElement.mpPos0->mY, triElement.mWaterHeight0);
					VuVector3 sv1(triElement.mpPos1->mX, triElement.mpPos1->mY, triElement.mWaterHeight1);
					VuVector3 sv2(triElement.mpPos2->mX, triElement.mpPos2->mY, triElement.mWaterHeight2);
					vWaterNormal = VuCross(sv1 - sv0, sv2 - sv0);
					if ( vWaterNormal.magSquared() > FLT_EPSILON )
					{
						float fMag = vWaterNormal.normalize();
						vWaterNormal.mX = VuSelect(vWaterNormal.mZ, vWaterNormal.mX, -vWaterNormal.mX);
						vWaterNormal.mY = VuSelect(vWaterNormal.mZ, vWaterNormal.mY, -vWaterNormal.mY);
						vWaterNormal.mZ = VuSelect(vWaterNormal.mZ, vWaterNormal.mZ, -vWaterNormal.mZ);
						float fArea = 0.5f*fMag;

						// handle case where water heights come from different water surfaces
						fArea = VuMin(triElement.mArea, fArea);

						mAvgWaterNormal += vWaterNormal*fArea;
					}
				}

				// drag
				{
					// calculate centroid velocity relative to water
					VuVector3 vTriCentroidPos = (1.0f/3.0f)*(*triElement.mpPos0 + *triElement.mpPos1 + *triElement.mpPos2);
					VuVector3 vTriCentroidWaterVel = (1.0f/3.0f)*(*triElement.mpWaterVel0 + *triElement.mpWaterVel1 + *triElement.mpWaterVel2);
					VuVector3 vRelPos = vTriCentroidPos - vRBPos;
					VuVector3 vRelVel = vRBLinearVel + VuCross(vRBAngularVel, vRelPos);
					vRelVel -= vTriCentroidWaterVel;
					float fRelVel = vRelVel.mag();

					// average water velocity calculation
					mAvgWaterVel += vTriCentroidWaterVel*triElement.mArea;

					// centroid velocity squared
					VuVector3 vDragForce = -fRelVel*vRelVel;

					// separate pressure and skin friction components
					VuVector3 vPressureForce = vTransformedTriNormal*VuDot(vDragForce, vTransformedTriNormal);
					VuVector3 vSkinFrictionForce = vDragForce - vPressureForce;

					// apply user-defined skin friction
					vSkinFrictionForce =
						mSkinFrictionCoeff.mX*matRB.getAxisX()*VuDot(vSkinFrictionForce, matRB.getAxisX()) +
						mSkinFrictionCoeff.mY*matRB.getAxisY()*VuDot(vSkinFrictionForce, matRB.getAxisY()) +
						mSkinFrictionCoeff.mZ*matRB.getAxisZ()*VuDot(vSkinFrictionForce, matRB.getAxisZ());

					// ignore pressure force if on backside of triangle
					if ( VuDot(vRelVel, vTransformedTriNormal) < 0 )
						vPressureForce = VuVector3(0,0,0);

					// kill downward pressure component
					vPressureForce.mZ = VuMax(mMinPressureForce, vPressureForce.mZ);

					// apply skin friction and pressure forces
					vDragForce = vSkinFrictionForce;
					vDragForce += vPressureForce;

					vDragForce *= 0.5f*triElement.mArea*mWaterDragDensity;

					// apply dragforce to totals
					mDragForce += vDragForce;
					mDragTorque += VuCross(vRelPos, vDragForce);
				}

				// bouyancy force
				{
					// calculate volume and centroid of water mass above triangle
					// (volume consists of 3 tetrahedra defined by verts 0, 1, 2,
					//  and the verts on the water surface directly above them)
					const VuVector3 &A = *triElement.mpPos0;
					const VuVector3 &B = *triElement.mpPos1;
					const VuVector3 &C = *triElement.mpPos2;
					VuVector3 vE0 = B - A;
					VuVector3 vE1 = C - B;
					VuVector3 vE2 = A - C;
					float h0 = triElement.mWaterHeight0 - A.mZ;
					float h1 = triElement.mWaterHeight1 - B.mZ;
					float h2 = triElement.mWaterHeight2 - C.mZ;

					float fVolume1 = h0*VuAbs((vE0.mX)*(vE2.mY) - (vE2.mX)*(vE0.mY))/6.0f;
					float fVolume2 = h1*VuAbs((vE1.mX)*(vE0.mY) - (vE0.mX)*(vE1.mY))/6.0f;
					float fVolume3 = h2*VuAbs((vE2.mX)*(vE1.mY) - (vE1.mX)*(vE2.mY))/6.0f;
					float fVolume = (fVolume1 + fVolume2 + fVolume3);

					if ( fVolume > FLT_MIN )
					{
						VuVector3 vCentroid(0,0,0);
						vCentroid += fVolume1*VuVector3(A.mX, A.mY, A.mZ + h0);
						vCentroid += fVolume2*VuVector3(B.mX, B.mY, B.mZ + h0 + h1);
						vCentroid += fVolume3*VuVector3(C.mX, C.mY, C.mZ + h0 + h1 + h2);
						vCentroid /= fVolume;
						vCentroid += A + B + C;
						vCentroid *= 0.25f;

						// if tri is on top of boat, negate
						fVolume *= VuSelect(vTransformedTriNormal.mZ, -1.0f, 1.0f);

						// calculate buoyancy force
						VuVector3 vBuoyancyForce = -(mWaterBuoyancyDensity*fVolume)*vGravity;

						// apply buoyancy force to totals
						mBuoyancyForce += vBuoyancyForce;
						mBuoyancyTorque += VuCross(vCentroid - vRBPos, vBuoyancyForce);

						// track submerged volume and area
						mSubmergedVolume += fVolume;
						mSubmergedArea += triElement.mArea;
					}
				}
			}
		}
	}

	if ( mAvgWaterNormal.magSquared() > FLT_EPSILON )
		mAvgWaterNormal.normalize();
	else
		mAvgWaterNormal = VuVector3(0,0,1);

	if ( mSubmergedArea > FLT_EPSILON )
		mAvgWaterVel /= mSubmergedArea;

	//VUASSERT(_finite(mAvgWaterNormal.mX) && _finite(mAvgWaterNormal.mY) && _finite(mAvgWaterNormal.mZ), "VuFluidsObject::addForces() bad normal");

	// damping force
	float fDampingForce = -VuDot(vRBLinearVel, mAvgWaterNormal);
	if ( fDampingForce > 0 )
	{
		fDampingForce *= 10.0f*fDampingForce*fMass*(mSubmergedArea/mpFluidsMeshAsset->getTotalArea());
		VuVector3 vDampingForce = fDampingForce*mAvgWaterNormal;

		mDampingForce += vDampingForce;
	}

	mTotalForce = mBuoyancyForce + mDragForce + mDampingForce;
	mTotalTorque = mBuoyancyTorque + mDragTorque + mDampingTorque;

	//int flags = VUGFX_TEXT_DRAW_VCENTER|VUGFX_TEXT_DRAW_HCENTER|VUGFX_TEXT_DRAW_NOCLIP;
	//float fDampingImpulse = mDampingForce.mag()*rb.getInvMass()*fdt;
	//VuDev::IF()->printf(VuDynamicsUtil::toVuVector3(rb.getCenterOfMassPosition()), flags, VuColor(255,255,255), "%f", fDampingImpulse);

	VUASSERT(VuIsFinite(mTotalForce.mX) && VuIsFinite(mTotalForce.mY) && VuIsFinite(mTotalForce.mZ), "VuFluidsObject::addForces() bad force");
	VUASSERT(VuIsFinite(mTotalTorque.mX) && VuIsFinite(mTotalTorque.mY) && VuIsFinite(mTotalTorque.mZ), "VuFluidsObject::addForces() bad torque");

	// clamp force and torque
	//VUPRINTF("\n");
	//VUPRINTF("vRBLinearVel %f %f %f\n", vRBLinearVel.mX, vRBLinearVel.mY, vRBLinearVel.mZ);
	//VUPRINTF("vRBAngularVel %f %f %f\n", vRBAngularVel.mX, vRBAngularVel.mY, vRBAngularVel.mZ);
	//VUPRINTF("vForce %f %f %f\n", mTotalForce.mX, mTotalForce.mY, mTotalForce.mZ);
	//VUPRINTF("vTorque %f %f %f\n", mTotalTorque.mX, mTotalTorque.mY, mTotalTorque.mZ);
}

//*****************************************************************************
void VuFluidsObject::getAabb(VuAabb &aabb) const
{
	if ( mpFluidsMeshAsset )
		aabb = mpFluidsMeshAsset->getAabb();
	else
		aabb = VuAabb(VuVector3(0,0,0), VuVector3(0,0,0));
}