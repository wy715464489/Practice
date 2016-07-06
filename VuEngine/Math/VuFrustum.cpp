//*****************************************************************************
//
//  Copyright (c) 2008-2008 Ralf Knoesel
//  Copyright (c) 2008-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  Frustum class
// 
//*****************************************************************************

#include "VuFrustum.h"
#include "VuAabb.h"
#include "VuMatrix.h"
#include "VuMathUtil.h"


//*****************************************************************************
VuFrustum::VuFrustum() :
	mO(0,0,0),
	mD(0,1,0),
	mU(0,0,1),
	mR(1,0,0),
	mDMin(1),
	mDMax(2),
	mUBound(1),
	mRBound(1)
{
	update();
}

//*****************************************************************************
VuFrustum::VuFrustum(const VuVector3 &vO, const VuVector3 &vD, const VuVector3 &vU, const VuVector3 &vR,
					 float fDMin, float fDMax, float fUBound, float fRBound):
	mO(vO),
	mD(vD),
	mU(vU),
	mR(vR),
	mDMin(fDMin),
	mDMax(fDMax),
	mUBound(fUBound),
	mRBound(fRBound)
{
	update();
}

//*****************************************************************************
void VuFrustum::update()
{
	mDRatio = mDMax/mDMin;
	mMTwoUF = -2*mUBound*mDMax;
	mMTwoRF = -2*mRBound*mDMax;
}

//*****************************************************************************
void VuFrustum::getVerts(VuVector3 *pVerts) const
{
	VuVector3 vDScaled = mDMin*mD;
	VuVector3 vUScaled = mUBound*mU;
	VuVector3 vRScaled = mRBound*mR;

	pVerts[0] = vDScaled - vRScaled - vUScaled;
	pVerts[1] = vDScaled + vRScaled - vUScaled;
	pVerts[2] = vDScaled + vRScaled + vUScaled;
	pVerts[3] = vDScaled - vRScaled + vUScaled;

	for ( int i = 0, ip = 4; i < 4; i++, ip++ )
	{
		pVerts[ip] = mO + mDRatio*pVerts[i];
		pVerts[i] += mO;
	}
}

//*****************************************************************************
void VuFrustum::getPlanes(VuVector4 *pPlanes) const
{
	VuVector3 vDScaled = mDMax*mD;
	VuVector3 vUScaled = (mDRatio*mUBound)*mU;
	VuVector3 vRScaled = (mDRatio*mRBound)*mR;

	VuVector3 ll = mO + vDScaled - vRScaled - vUScaled;
	VuVector3 lr = mO + vDScaled + vRScaled - vUScaled;
	VuVector3 ur = mO + vDScaled + vRScaled + vUScaled;
	VuVector3 ul = mO + vDScaled - vRScaled + vUScaled;

	// near and far planes
	pPlanes[0] = VuMathUtil::planeFromNormalPoint( mD, mO + mDMin*mD);
	pPlanes[1] = VuMathUtil::planeFromNormalPoint(-mD, mO + mDMax*mD);

	// left/right/top/bottom planes
	pPlanes[2] = VuMathUtil::planeFromVerts(mO, ll, ul); // left
	pPlanes[3] = VuMathUtil::planeFromVerts(mO, ur, lr); // right
	pPlanes[4] = VuMathUtil::planeFromVerts(mO, ul, ur); // top
	pPlanes[5] = VuMathUtil::planeFromVerts(mO, lr, ll); // bottom
}

//*****************************************************************************
bool VuFrustum::isAabbVisible(const VuAabb &aabb, const VuMatrix &matWorld) const
{
	// box center, extents and axes (in world space)
	VuVector3 vC = matWorld.transform(aabb.getCenter());
	float afE[3] = { 0.5f*(aabb.mMax.mX - aabb.mMin.mX), 0.5f*(aabb.mMax.mY - aabb.mMin.mY), 0.5f*(aabb.mMax.mZ - aabb.mMin.mZ) };
	VuVector3 vA[3] = { matWorld.getAxisX(), matWorld.getAxisY(), matWorld.getAxisZ() };

	// handle scaling
	float fScaleX = vA[0].mag();
	float fScaleY = vA[1].mag();
	float fScaleZ = vA[2].mag();
	afE[0] *= fScaleX;
	afE[1] *= fScaleY;
	afE[2] *= fScaleZ;
	vA[0] /= fScaleX;
	vA[1] /= fScaleY;
	vA[2] /= fScaleZ;


	VuVector3 vDiff = vC - mO; // C-E

	float afA[3];      // Dot(R,A[i])
	float afB[3];      // Dot(U,A[i])
	float afC[3];      // Dot(D,A[i])
	float afD[3];      // (Dot(R,C-E),Dot(U,C-E),Dot(D,C-E))
	float afNA[3];     // dmin*Dot(R,A[i])
	float afNB[3];     // dmin*Dot(U,A[i])
	float afNC[3];     // dmin*Dot(D,A[i])
	float afND[3];     // dmin*(Dot(R,C-E),Dot(U,C-E),?)
	float afRC[3];     // rmax*Dot(D,A[i])
	float afRD[3];     // rmax*(?,?,Dot(D,C-E))
	float afUC[3];     // umax*Dot(D,A[i])
	float afUD[3];     // umax*(?,?,Dot(D,C-E))
	float afNApRC[3];  // dmin*Dot(R,A[i]) + rmax*Dot(D,A[i])
	float afNAmRC[3];  // dmin*Dot(R,A[i]) - rmax*Dot(D,A[i])
	float afNBpUC[3];  // dmin*Dot(U,A[i]) + umax*Dot(D,A[i])
	float afNBmUC[3];  // dmin*Dot(U,A[i]) - umax*Dot(D,A[i])
	float afRBpUA[3];  // rmax*Dot(U,A[i]) + umax*Dot(R,A[i])
	float afRBmUA[3];  // rmax*Dot(U,A[i]) - umax*Dot(R,A[i])
	float fDdD, fRad, fP, fMin, fMax, fTmp;
	int i, j;

	// M = D
	afD[2] = VuDot(vDiff, mD);
	for (i = 0; i < 3; i++)
	{
		afC[i] = VuDot(vA[i], mD);
	}
	fRad =
		afE[0]*VuAbs(afC[0]) +
		afE[1]*VuAbs(afC[1]) +
		afE[2]*VuAbs(afC[2]);
	if (afD[2] + fRad < mDMin
	||  afD[2] - fRad > mDMax)
	{
		return false;
	}

	// M = n*R - r*D
	for (i = 0; i < 3; i++)
	{
		afA[i] = VuDot(vA[i], mR);
		afRC[i] = mRBound*afC[i];
		afNA[i] = mDMin*afA[i];
		afNAmRC[i] = afNA[i] - afRC[i];
	}
	afD[0] = VuDot(vDiff, mR);
	fRad =
		afE[0]*VuAbs(afNAmRC[0]) +
		afE[1]*VuAbs(afNAmRC[1]) +
		afE[2]*VuAbs(afNAmRC[2]);
	afND[0] = mDMin*afD[0];
	afRD[2] = mRBound*afD[2];
	fDdD = afND[0] - afRD[2];
	if (fDdD + fRad < mMTwoRF || fDdD > fRad)
	{
		return false;
	}

	// M = -n*R - r*D
	for (i = 0; i < 3; i++)
	{
		afNApRC[i] = afNA[i] + afRC[i];
	}
	fRad =
		afE[0]*VuAbs(afNApRC[0]) +
		afE[1]*VuAbs(afNApRC[1]) +
		afE[2]*VuAbs(afNApRC[2]);
	fDdD = -(afND[0] + afRD[2]);
	if (fDdD + fRad < mMTwoRF || fDdD > fRad)
	{
		return false;
	}

	// M = n*U - u*D
	for (i = 0; i < 3; i++)
	{
		afB[i] = VuDot(vA[i], mU);
		afUC[i] = mUBound*afC[i];
		afNB[i] = mDMin*afB[i];
		afNBmUC[i] = afNB[i] - afUC[i];
	}
	afD[1] = VuDot(vDiff, mU);
	fRad =
		afE[0]*VuAbs(afNBmUC[0]) +
		afE[1]*VuAbs(afNBmUC[1]) +
		afE[2]*VuAbs(afNBmUC[2]);
	afND[1] = mDMin*afD[1];
	afUD[2] = mUBound*afD[2];
	fDdD = afND[1] - afUD[2];
	if (fDdD + fRad < mMTwoUF || fDdD > fRad)
	{
		return false;
	}

	// M = -n*U - u*D
	for (i = 0; i < 3; i++)
	{
		afNBpUC[i] = afNB[i] + afUC[i];
	}
	fRad =
		afE[0]*VuAbs(afNBpUC[0]) +
		afE[1]*VuAbs(afNBpUC[1]) +
		afE[2]*VuAbs(afNBpUC[2]);
	fDdD = -(afND[1] + afUD[2]);
	if (fDdD + fRad < mMTwoUF || fDdD > fRad)
	{
		return false;
	}

	// M = A[i]
	for (i = 0; i < 3; i++)
	{
		fP = mRBound*VuAbs(afA[i]) +
		mUBound*VuAbs(afB[i]);
		afNC[i] = mDMin*afC[i];
		fMin = afNC[i] - fP;
		if (fMin < 0.0f)
		{
			fMin *= mDRatio;
		}
		fMax = afNC[i] + fP;
		if (fMax > 0.0f)
		{
			fMax *= mDRatio;
		}
		fDdD = afA[i]*afD[0] + afB[i]*afD[1] + afC[i]*afD[2];
		if (fDdD + afE[i] < fMin || fDdD - afE[i] > fMax)
		{
			return false;
		}
	}

	// M = Cross(R,A[i])
	for (i = 0; i < 3; i++)
	{
		fP = mUBound*VuAbs(afC[i]);
		fMin = -afNB[i] - fP;
		if (fMin < 0.0f)
		{
			fMin *= mDRatio;
		}
		fMax = -afNB[i] + fP;
		if (fMax > 0.0f)
		{
			fMax *= mDRatio;
		}
		fDdD = afC[i]*afD[1] - afB[i]*afD[2];
		fRad =
			afE[0]*VuAbs(afB[i]*afC[0]-afB[0]*afC[i]) +
			afE[1]*VuAbs(afB[i]*afC[1]-afB[1]*afC[i]) +
			afE[2]*VuAbs(afB[i]*afC[2]-afB[2]*afC[i]);
		if (fDdD + fRad < fMin || fDdD - fRad > fMax)
		{
			return false;
		}
	}

	// M = Cross(U,A[i])
	for (i = 0; i < 3; i++)
	{
		fP = mRBound*VuAbs(afC[i]);
		fMin = afNA[i] - fP;
		if (fMin < 0.0f)
		{
			fMin *= mDRatio;
		}
		fMax = afNA[i] + fP;
		if (fMax > 0.0f)
		{
			fMax *= mDRatio;
		}
		fDdD = -afC[i]*afD[0] + afA[i]*afD[2];
		fRad =
			afE[0]*VuAbs(afA[i]*afC[0]-afA[0]*afC[i]) +
			afE[1]*VuAbs(afA[i]*afC[1]-afA[1]*afC[i]) +
			afE[2]*VuAbs(afA[i]*afC[2]-afA[2]*afC[i]);
		if (fDdD + fRad < fMin || fDdD - fRad > fMax)
		{
			return false;
		}
	}

	// M = Cross(n*D+r*R+u*U,A[i])
	for (i = 0; i < 3; i++)
	{
		float fRB = mRBound*afB[i];
		float fUA = mUBound*afA[i];
		afRBpUA[i] = fRB + fUA;
		afRBmUA[i] = fRB - fUA;
	}
	for (i = 0; i < 3; i++)
	{
		fP = mRBound*VuAbs(afNBmUC[i]) +
		mUBound*VuAbs(afNAmRC[i]);
		fTmp = -mDMin*afRBmUA[i];
		fMin = fTmp - fP;
		if (fMin < 0.0f)
		{
			fMin *= mDRatio;
		}
		fMax = fTmp + fP;
		if (fMax > 0.0f)
		{
			fMax *= mDRatio;
		}
		fDdD = afD[0]*afNBmUC[i] - afD[1]*afNAmRC[i] - afD[2]*afRBmUA[i];
		fRad = 0.0f;
		for (j = 0; j < 3; j++)
		{
			fRad += afE[j]*VuAbs(afA[j]*afNBmUC[i] -
			afB[j]*afNAmRC[i] - afC[j]*afRBmUA[i]);
		}
		if (fDdD + fRad < fMin || fDdD - fRad > fMax)
		{
			return false;
		}
	}

	// M = Cross(n*D+r*R-u*U,A[i])
	for (i = 0; i < 3; i++)
	{
		fP = mRBound*VuAbs(afNBpUC[i]) +
		mUBound*VuAbs(afNAmRC[i]);
		fTmp = -mDMin*afRBpUA[i];
		fMin = fTmp - fP;
		if (fMin < 0.0f)
		{
			fMin *= mDRatio;
		}
		fMax = fTmp + fP;
		if (fMax > 0.0f)
		{
			fMax *= mDRatio;
		}
		fDdD = afD[0]*afNBpUC[i] - afD[1]*afNAmRC[i] - afD[2]*afRBpUA[i];
		fRad = 0.0f;
		for (j = 0; j < 3; j++)
		{
			fRad += afE[j]*VuAbs(afA[j]*afNBpUC[i] -
			afB[j]*afNAmRC[i] - afC[j]*afRBpUA[i]);
		}
		if (fDdD + fRad < fMin || fDdD - fRad > fMax)
		{
			return false;
		}
	}

	// M = Cross(n*D-r*R+u*U,A[i])
	for (i = 0; i < 3; i++)
	{
		fP = mRBound*VuAbs(afNBmUC[i]) +
		mUBound*VuAbs(afNApRC[i]);
		fTmp = mDMin*afRBpUA[i];
		fMin = fTmp - fP;
		if (fMin < 0.0f)
		{
			fMin *= mDRatio;
		}
		fMax = fTmp + fP;
		if (fMax > 0.0f)
		{
			fMax *= mDRatio;
		}
		fDdD = afD[0]*afNBmUC[i] - afD[1]*afNApRC[i] + afD[2]*afRBpUA[i];
		fRad = 0.0f;
		for (j = 0; j < 3; j++)
		{
			fRad += afE[j]*VuAbs(afA[j]*afNBmUC[i] -
			afB[j]*afNApRC[i] + afC[j]*afRBpUA[i]);
		}
		if (fDdD + fRad < fMin || fDdD - fRad > fMax)
		{
			return false;
		}
	}

	// M = Cross(n*D-r*R-u*U,A[i])
	for (i = 0; i < 3; i++)
	{
		fP = mRBound*VuAbs(afNBpUC[i]) +
		mUBound*VuAbs(afNApRC[i]);
		fTmp = mDMin*afRBmUA[i];
		fMin = fTmp - fP;
		if (fMin < 0.0f)
		{
			fMin *= mDRatio;
		}
		fMax = fTmp + fP;
		if (fMax > 0.0f)
		{
			fMax *= mDRatio;
		}
		fDdD = afD[0]*afNBpUC[i] - afD[1]*afNApRC[i] + afD[2]*afRBmUA[i];
		fRad = 0.0f;
		for (j = 0; j < 3; j++)
		{
			fRad += afE[j]*VuAbs(afA[j]*afNBpUC[i] -
			afB[j]*afNApRC[i] + afC[j]*afRBmUA[i]);
		}
		if (fDdD + fRad < fMin || fDdD - fRad > fMax)
		{
			return false;
		}
	}

	return true;
}
