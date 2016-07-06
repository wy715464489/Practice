//*****************************************************************************
//
//  Copyright (c) 2007-2008 Ralf Knoesel
//  Copyright (c) 2008-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  VuMathUtil class
// 
//*****************************************************************************

#include <float.h>
#include "VuMathUtil.h"
#include "VuVector2.h"
#include "VuVector3.h"
#include "VuVector4.h"
#include "VuMatrix.h"
#include "VuRect.h"
#include "VuAabb.h"


//*****************************************************************************
VuVector2 VuMathUtil::closestPointOnSegment(const VuVector2 &A, const VuVector2 &B, const VuVector2 &P)
{
    VuVector2 D = B - A;
    float numer = VuDot(P - A, D);
    if ( numer <= 0.0f )
        return A;
    float denom = VuDot(D, D);
    if ( numer >= denom )
        return B;
    return A + (numer/denom)*D;
}

//*****************************************************************************
bool VuMathUtil::testIntersectionLineSegmentRect(const VuVector2 &A, const VuVector2 &B, const VuRect &R)
{
	VuVector2 vSegOrigin = 0.5f*(A + B);
	VuVector2 vSegDir = (B - A);
	float fSegExtent = 0.5f*(B - A).mag();
	if ( fSegExtent <= FLT_MIN )
		return false;
	vSegDir /= 2.0f*fSegExtent;

	VuVector2 vDiff = vSegOrigin - R.getCenter();

	float fLhs, fRhs;

	float fAWdU0 = VuAbs(vSegDir.mX);
	float fADdU0 = VuAbs(vDiff.mX);
	fRhs = 0.5f*R.mWidth + fSegExtent*fAWdU0;
	if ( fADdU0 > fRhs )
		return false;

	float fAWdU1 = VuAbs(vSegDir.mY);
	float fADdU1 = VuAbs(vDiff.mY);
	fRhs = 0.5f*R.mHeight + fSegExtent*fAWdU1;
	if ( fADdU1 > fRhs )
		return false;

	VuVector2 vPerp(vDiff.mY, -vDiff.mX);
	fLhs = VuAbs(VuDot(vSegDir, vPerp));
	fRhs = 0.5f*R.mWidth*fAWdU1 + 0.5f*R.mHeight*fAWdU0;
	return fLhs <= fRhs;
}

//*****************************************************************************
bool VuMathUtil::testIntersectionLineSegBox(const VuVector3 &A, const VuVector3 &B, const VuAabb &aabb, const VuMatrix &transform)
{
	VuVector3 segCenter = 0.5f*(A + B);
	VuVector3 segDir = (B - A);
	float segExtent = 0.5f*(B - A).mag();
	if ( segExtent <= FLT_MIN )
		return false;
	segDir /= 2.0f*segExtent;

	VuVector3 boxCenter = transform.transform(aabb.getCenter());
	VuVector3 boxExtents = aabb.getExtents();
	VuVector3 diff = segCenter - boxCenter;

    float AWdU[3], ADdU[3], AWxDdU[3], RHS;

    AWdU[0] = VuAbs(VuDot(segDir, transform.getAxisX()));
    ADdU[0] = VuAbs(VuDot(diff, transform.getAxisX()));
    RHS = boxExtents.mX + segExtent*AWdU[0];
    if (ADdU[0] > RHS)
        return false;

    AWdU[1] = VuAbs(VuDot(segDir, transform.getAxisY()));
    ADdU[1] = VuAbs(VuDot(diff, transform.getAxisY()));
    RHS = boxExtents.mY + segExtent*AWdU[1];
    if (ADdU[1] > RHS)
        return false;

    AWdU[2] = VuAbs(VuDot(segDir, transform.getAxisZ()));
    ADdU[2] = VuAbs(VuDot(diff, transform.getAxisZ()));
    RHS = boxExtents.mZ + segExtent*AWdU[2];
    if (ADdU[2] > RHS)
        return false;

	VuVector3 WxD = VuCross(segDir, diff);

    AWxDdU[0] = VuAbs(VuDot(WxD, transform.getAxisX()));
    RHS = boxExtents.mY*AWdU[2] + boxExtents.mZ*AWdU[1];
    if (AWxDdU[0] > RHS)
        return false;

    AWxDdU[1] = VuAbs(VuDot(WxD, transform.getAxisY()));
    RHS = boxExtents.mX*AWdU[2] + boxExtents.mZ*AWdU[0];
    if (AWxDdU[1] > RHS)
        return false;

    AWxDdU[2] = VuAbs(VuDot(WxD, transform.getAxisZ()));
    RHS = boxExtents.mX*AWdU[1] + boxExtents.mY*AWdU[0];
    if (AWxDdU[2] > RHS)
        return false;

    return true;
}

//*****************************************************************************
// returns the angle between two lines
//*****************************************************************************
float VuMathUtil::angleLineLine(
	const VuVector3 &LA0,
	const VuVector3 &LA1,
	const VuVector3 &LB0,
	const VuVector3 &LB1)
{
	VuVector3 LA = (LA1 - LA0);
	VuVector3 LB = (LB1 - LB0);
	float lenA = LA.mag();
	float lenB = LB.mag();

	VUASSERT(lenA > FLT_MIN, "VuMathUtil::angleLineLine() invalid line A");
	VUASSERT(lenB > FLT_MIN, "VuMathUtil::angleLineLine() invalid line B");

	return VuACos(VuClamp(VuDot(LA/lenA, LB/lenB), -1.0f, 1.0f));
}

//*****************************************************************************
// returns the angle between two lines
// the sign is determined by comparing the cross
// product (BxA) with the normal N
//*****************************************************************************
float VuMathUtil::signedAngleLineLine(
	const VuVector3 &LA0,
	const VuVector3 &LA1,
	const VuVector3 &LB0,
	const VuVector3 &LB1,
	const VuVector3 &N)
{
	float angle = angleLineLine(LA0, LA1, LB0, LB1);

	return VuDot(VuCross(LB1 - LB0, LA1 - LA0), N) < 0 ? -angle : angle;
}

//*****************************************************************************
// returns the distance between a point P and a segment L0-L1
// (and optionally the projection along the segment)
//*****************************************************************************
float VuMathUtil::distPointLineSeg(
	const VuVector2 &P,
	const VuVector2 &L0,
	const VuVector2 &L1,
	float *pProj)
{
	float u = 0;
	float den = (L1 - L0).magSquared();
	VUASSERT(den > FLT_MIN, "VuMathUtil::distPointLineSeg() invalid line");
	u = VuDot(L1 - L0, P - L0)/den;

	u = VuClamp(u, 0.0f, 1.0f);

	float dist = ((L0 + u*(L1 - L0)) - P).mag();

	if ( pProj )
		*pProj = u;

	return dist;
}

//*****************************************************************************
// returns the distance between a point P and a segment L0-L1
// (and optionally the projection along the segment)
//*****************************************************************************
float VuMathUtil::distPointLineSeg(
	const VuVector3 &P,
	const VuVector3 &L0,
	const VuVector3 &L1,
	float *pProj)
{
	float u = 0;
	float den = (L1 - L0).magSquared();
	VUASSERT(den > FLT_MIN, "VuMathUtil::distPointLineSeg() invalid line");
	u = VuDot(L1 - L0, P - L0)/den;

	u = VuClamp(u, 0.0f, 1.0f);

	float dist = ((L0 + u*(L1 - L0)) - P).mag();

	if ( pProj )
		*pProj = u;

	return dist;
}

//*****************************************************************************
// returns the point on line segment S0->S1 which is closest to point P
//*****************************************************************************
void VuMathUtil::closestPointLineSeg(
	const VuVector3 &P,
	const VuVector3 &S0,
	const VuVector3 &S1,
	VuVector3 &closest)
{
	float u;
	
	distPointLineSeg(P, S0, S1, &u);
	
	closest = S0 + u*(S1 - S0);
}

//*****************************************************************************
// returns the distance between a point P and a triagle A,B,C
//*****************************************************************************
float VuMathUtil::distPointTriangle(
	const VuVector3 &P,
	const VuVector3 &A,
	const VuVector3 &B,
	const VuVector3 &C)
{
	VuVector3 diff = A - P;
	VuVector3 edge0 = B - A;
	VuVector3 edge1 = C - A;
	float a00 = edge0.magSquared();
	float a01 = VuDot(edge0, edge1);
	float a11 = edge1.magSquared();
	float b0 = VuDot(diff, edge0);
	float b1 = VuDot(diff, edge1);
	float c = diff.magSquared();
	float det = VuAbs(a00*a11 - a01*a01);
	float s = a01*b1 - a11*b0;
	float t = a01*b0 - a00*b1;
	float sqrDistance;

	if (s + t <= det)
	{
		if (s < 0.0f)
		{
			if (t < 0.0f)  // region 4
			{
				if (b0 < 0.0f)
				{
					t = 0.0f;
					if (-b0 >= a00)
					{
						s = 1.0f;
						sqrDistance = a00 + 2.0f*b0 + c;
					}
					else
					{
						s = -b0/a00;
						sqrDistance = b0*s + c;
					}
				}
				else
				{
					s = 0.0f;
					if (b1 >= 0.0f)
					{
						t = 0.0f;
						sqrDistance = c;
					}
					else if (-b1 >= a11)
					{
						t = 1.0f;
						sqrDistance = a11 + 2.0f*b1 + c;
					}
					else
					{
						t = -b1/a11;
						sqrDistance = b1*t + c;
					}
				}
			}
			else  // region 3
			{
				s = 0.0f;
				if (b1 >= 0.0f)
				{
					t = 0.0f;
					sqrDistance = c;
				}
				else if (-b1 >= a11)
				{
					t = 1.0f;
					sqrDistance = a11 + 2.0f*b1 + c;
				}
				else
				{
					t = -b1/a11;
					sqrDistance = b1*t + c;
				}
			}
		}
		else if (t < 0.0f)  // region 5
		{
			t = 0.0f;
			if (b0 >= 0.0f)
			{
				s = 0.0f;
				sqrDistance = c;
			}
			else if (-b0 >= a00)
			{
				s = 1.0f;
				sqrDistance = a00 + 2.0f*b0 + c;
			}
			else
			{
				s = -b0/a00;
				sqrDistance = b0*s + c;
			}
		}
		else  // region 0
		{
			// minimum at interior point
			float invDet = 1.0f/det;
			s *= invDet;
			t *= invDet;
			sqrDistance = s*(a00*s + a01*t + 2.0f*b0) + t*(a01*s + a11*t + 2.0f*b1) + c;
		}
	}
	else
	{
		float tmp0, tmp1, numer, denom;

		if (s < 0.0f)  // region 2
		{
			tmp0 = a01 + b0;
			tmp1 = a11 + b1;
			if (tmp1 > tmp0)
			{
				numer = tmp1 - tmp0;
				denom = a00 - 2.0f*a01 + a11;
				if (numer >= denom)
				{
					s = 1.0f;
					t = 0.0f;
					sqrDistance = a00 + 2.0f*b0 + c;
				}
				else
				{
					s = numer/denom;
					t = 1.0f - s;
					sqrDistance = s*(a00*s + a01*t + 2.0f*b0) + t*(a01*s + a11*t + 2.0f*b1) + c;
				}
			}
			else
			{
				s = 0.0f;
				if (tmp1 <= 0.0f)
				{
					t = 1.0f;
					sqrDistance = a11 + 2.0f*b1 + c;
				}
				else if (b1 >= 0.0f)
				{
					t = 0.0f;
					sqrDistance = c;
				}
				else
				{
					t = -b1/a11;
					sqrDistance = b1*t + c;
				}
			}
		}
		else if (t < 0.0f)  // region 6
		{
			tmp0 = a01 + b1;
			tmp1 = a00 + b0;
			if (tmp1 > tmp0)
			{
				numer = tmp1 - tmp0;
				denom = a00 - 2.0f*a01 + a11;
				if (numer >= denom)
				{
					t = 1.0f;
					s = 0.0f;
					sqrDistance = a11 + 2.0f*b1 + c;
				}
				else
				{
					t = numer/denom;
					s = 1.0f - t;
					sqrDistance = s*(a00*s + a01*t + 2.0f*b0) + t*(a01*s + a11*t + 2.0f*b1) + c;
				}
			}
			else
			{
				t = 0.0f;
				if (tmp1 <= 0.0f)
				{
					s = 1.0f;
					sqrDistance = a00 + 2.0f*b0 + c;
				}
				else if (b0 >= 0.0f)
				{
					s = 0.0f;
					sqrDistance = c;
				}
				else
				{
					s = -b0/a00;
					sqrDistance = b0*s + c;
				}
			}
		}
		else  // region 1
		{
			numer = a11 + b1 - a01 - b0;
			if (numer <= 0.0f)
			{
				s = 0.0f;
				t = 1.0f;
				sqrDistance = a11 + 2.0f*b1 + c;
			}
			else
			{
				denom = a00 - 2.0f*a01 + a11;
				if (numer >= denom)
				{
					s = 1.0f;
					t = 0.0f;
					sqrDistance = a00 + 2.0f*b0 + c;
				}
				else
				{
					s = numer/denom;
					t = 1.0f - s;
					sqrDistance = s*(a00*s + a01*t + 2.0f*b0) + t*(a01*s + a11*t + 2.0f*b1) + c;
				}
			}
		}
	}

	// Account for numerical round-off error.
	if (sqrDistance < 0.0f)
		sqrDistance = 0.0f;

	return VuSqrt(sqrDistance);
}

//*****************************************************************************
// returns the distance between a point P and a box defined by an aabb and xform
//*****************************************************************************
float VuMathUtil::distPointBox(const VuVector3 &P, const VuAabb &aabb, const VuMatrix &xform, VuVector3 &closestPoint)
{
	VuVector3 boxCenter = xform.transform(aabb.getCenter());
	VuVector3 boxExtent = aabb.getExtents();

	// Work in the box's coordinate system.
	VuVector3 diff = P - boxCenter;

	// Compute squared distance and closest point on box.
	float squareDist = 0.0f;
	VuVector3 closest;

	// x-axis
	closest.mX = VuDot(diff, xform.getAxisX());
	if ( closest.mX < -boxExtent.mX )
	{
		float delta = closest.mX + boxExtent.mX;
		squareDist += delta*delta;
		closest.mX = -boxExtent.mX;
	}
	else if ( closest.mX > boxExtent.mX )
	{
		float delta = closest.mX - boxExtent.mX;
		squareDist += delta*delta;
		closest.mX = boxExtent.mX;
	}

	// y-axis
	closest.mY = VuDot(diff, xform.getAxisY());
	if ( closest.mY < -boxExtent.mY )
	{
		float delta = closest.mY + boxExtent.mY;
		squareDist += delta*delta;
		closest.mY = -boxExtent.mY;
	}
	else if ( closest.mY > boxExtent.mY )
	{
		float delta = closest.mY - boxExtent.mY;
		squareDist += delta*delta;
		closest.mY = boxExtent.mY;
	}

	// z-axis
	closest.mZ = VuDot(diff, xform.getAxisZ());
	if ( closest.mZ < -boxExtent.mZ )
	{
		float delta = closest.mZ + boxExtent.mZ;
		squareDist += delta*delta;
		closest.mZ = -boxExtent.mZ;
	}
	else if ( closest.mZ > boxExtent.mZ )
	{
		float delta = closest.mZ - boxExtent.mZ;
		squareDist += delta*delta;
		closest.mZ = boxExtent.mZ;
	}

	closestPoint = boxCenter;
	closestPoint += closest.mX*xform.getAxisX();
	closestPoint += closest.mY*xform.getAxisY();
	closestPoint += closest.mZ*xform.getAxisZ();

	float dist = VuSqrt(squareDist);

	return dist;
}

//*****************************************************************************
// returns the distance between two lines (and optionally projection along each)
//*****************************************************************************
float VuMathUtil::distLineLine(
		const VuVector3 &LA0,
		const VuVector3 &LA1,
		const VuVector3 &LB0,
		const VuVector3 &LB1,
		float *pProjA,
		float *pProjB)
{
	VuVector3	u = LA1 - LA0;
	VuVector3	v = LB1 - LB0;
	VuVector3	w = LA0 - LB0;
	float		a = VuDot(u,u);
	float		b = VuDot(u,v);
	float		c = VuDot(v,v);
	float		d = VuDot(u,w);
	float		e = VuDot(v,w);
	float		D = a*c - b*b;

	float sc, tc;
	if ( D < FLT_MIN )
	{
		// the lines are parallel
		sc = 0.0f;
		tc = (b > c ? d/b : e/c);
	}
	else
	{
		sc = (b*e - c*d)/D;
		tc = (a*e - b*d)/D;
	}
	
	// difference between the two closest points
	VuVector3 dP = w + (sc*u) - (tc*v);

	if ( pProjA ) *pProjA = sc;
	if ( pProjB ) *pProjB = tc;

	return dP.mag();
}

//*****************************************************************************
// returns the intersection of a plane defined by point P and normal N
// and a line segment
//*****************************************************************************
bool VuMathUtil::planeLineSegIntersection(
	const VuVector3 &P,
	const VuVector3 &N,
	const VuVector3 &S0,
	const VuVector3 &S1,
	VuVector3 &inter)
{
	float num = VuDot(N, P - S0);
	float den = VuDot(N, S1 - S0);
	if ( VuAbs(den) <= FLT_MIN )
		return false;

	float u = num/den;
	inter = S0 + u*(S1 - S0);

	return u >= 0 && u <= 1;
}

//*****************************************************************************
// Finds the intersection of a line segment with the front of a sphere defined
// by center C and radius R.
//*****************************************************************************
bool VuMathUtil::sphereLineSegIntersection(
	const VuVector3 &C,
	float R,
	const VuVector3 &S0,
	const VuVector3 &S1,
	VuVector3 &inter)
{
	// quadratic equation terms
	float a = (S1 - S0).magSquared();
	float b = 2*VuDot(S1 - S0, S0 - C);
	float c = C.magSquared() + S0.magSquared() - 2*(VuDot(C, S0)) - R*R;

	// solve quadratic equation
	float rootTerm = b*b - 4*a*c;
	if ( rootTerm < 0 )
		return false;

	// there are two solutions, choose smaller one because we want the
	// intersection of line segment S0 -> S1 with the front of the sphere
	float u = (-b - VuSqrt(rootTerm))/(2*a);

	// check if outside line segment
	if ( u < 0 || u > 1 )
		return false;

	inter = S0 + u*(S1 - S0);
	return true;
}

//*****************************************************************************
// Finds the intersection of a line segment with the triangle defined
// by points A, B, C
//*****************************************************************************
bool VuMathUtil::triangleLineSegIntersection(
	const VuVector3 &A,
	const VuVector3 &B,
	const VuVector3 &C,
	const VuVector3 &S0,
	const VuVector3 &S1,
	VuVector3 &inter)
{
	// compute segment
	VuVector3 vSegOrigin = 0.5f*(S0 + S1);
	VuVector3 vSegDir = (S1 - S0);
	float fSegExtent = 0.5f*vSegDir.normalize();

	// compute the offset origin, edges, and normal
	VuVector3 vDiff = vSegOrigin - A;
	VuVector3 vEdge1 = B - A;
	VuVector3 vEdge2 = C - A;
	VuVector3 vNormal = VuCross(vEdge1, vEdge2);

	// Solve Q + t*D = b1*E1 + b2*E2 (Q = kDiff, D = segment direction,
	// E1 = kEdge1, E2 = kEdge2, N = Cross(E1,E2)) by
	//   |Dot(D,N)|*b1 = sign(Dot(D,N))*Dot(D,Cross(Q,E2))
	//   |Dot(D,N)|*b2 = sign(Dot(D,N))*Dot(D,Cross(E1,Q))
	//   |Dot(D,N)|*t = -sign(Dot(D,N))*Dot(Q,N)
	float fDdN = VuDot(vSegDir, vNormal);
	float fSign;
	if (fDdN > FLT_EPSILON)
	{
		fSign = 1.0f;
	}
	else if (fDdN < -FLT_EPSILON)
	{
		fSign = -1.0f;
		fDdN = -fDdN;
	}
	else
	{
		// Segment and triangle are parallel, call it a "no intersection"
		// even if the segment does intersect.
		return false;
	}

	float fDdQxE2 = fSign*VuDot(vSegDir, VuCross(vDiff, vEdge2));
	if (fDdQxE2 >= 0.0f)
	{
		float fDdE1xQ = fSign*VuDot(vSegDir, VuCross(vEdge1, vDiff));
		if (fDdE1xQ >= 0.0f)
		{
			if (fDdQxE2 + fDdE1xQ <= fDdN)
			{
				// line intersects triangle, check if segment does
				float fQdN = -fSign*VuDot(vDiff, vNormal);
				float fExtDdN = fSegExtent*fDdN;
				if (-fExtDdN <= fQdN && fQdN <= fExtDdN)
				{
					// segment intersects triangle
					float fInv = 1.0f/fDdN;
					float fSegmentT = fQdN*fInv;
					inter = vSegOrigin + fSegmentT*vSegDir;

					return true;
				}
				// else: |t| > extent, no intersection
			}
			// else: b1+b2 > 1, no intersection
		}
		// else: b2 < 0, no intersection
	}
	// else: b1 < 0, no intersection

	return false;
}

//*****************************************************************************
bool VuMathUtil::lineSegLineSegIntersection2d(
	const VuVector2 &a0, 
	const VuVector2 &a1, 
	const VuVector2 &b0, 
	const VuVector2 &b1, 
	VuVector2 &hitPos)
{
	float den = ((b1.mY - b0.mY) * (a1.mX - a0.mX) - (b1.mX - b0.mX) * (a1.mY - a0.mY)); 
	
	if(VuAbs(den) < FLT_MIN) return false;

	float ta = ((b1.mX - b0.mX) * (a0.mY - b0.mY) - (b1.mY - b0.mY) * (a0.mX - b0.mX)) / den;
	float tb = ((a1.mX - a0.mX) * (a0.mY - b0.mY) - (a1.mY - a0.mY) * (a0.mX - b0.mX)) / den;

	if((ta >= 0.0f && ta <= 1.0f) &&
	   (tb >= 0.0f && tb <= 1.0f))
	{
		hitPos = a0 + (ta * (a1 - a0));

		return true;
	}

	return false;
}

//*****************************************************************************
bool VuMathUtil::crossedWidthBoundedPlane(const VuVector3 &vCurPos, 
										 const VuVector3 &vPrevPos, 
										 const VuVector3 &vLineCenter, 
										 const VuVector3 &vNormal, 
										 float width,
										 bool &forward)
{
	float fPrevDist = VuDot(vNormal, vPrevPos - vLineCenter);
	float fCurDist = VuDot(vNormal, vCurPos - vLineCenter);

	if ( fPrevDist*fCurDist <= 0 )
	{
		if(fPrevDist < 0.0f)
		{
			forward = true;
		}
		else
		{
			forward = false;
		}

		fPrevDist = VuAbs(fPrevDist);
		fCurDist = VuAbs(fCurDist);

		// check left/right
		VuVector3 vProjPos = vPrevPos + (vCurPos - vPrevPos)*fPrevDist/(fPrevDist + fCurDist);
		float fDistXY = (vProjPos - vLineCenter).mag2d();
		if ( fDistXY < 0.5f * width )
		{
			return true;
		}
	}

	return false;
}

//*****************************************************************************
void VuMathUtil::buildOrientationMatrix(const VuVector3 &vFwd, const VuVector3 &vUp, VuMatrix &mat)
{
	VuVector3 vAxisForward = vFwd;
	VuVector3 vAxisRight = VuCross(vFwd, vUp);
	VuVector3 vAxisUp = VuCross(vAxisRight, vFwd);

	if ( vAxisForward.magSquared() < FLT_MIN || vAxisRight.magSquared() < FLT_MIN || vAxisUp.magSquared() < FLT_MIN )
	{
		vAxisRight = VuVector3(1,0,0);
		vAxisForward = VuVector3(0,1,0);
		vAxisUp = VuVector3(0,0,1);
	}

	vAxisRight = vAxisRight.normal();
	vAxisForward = vAxisForward.normal();
	vAxisUp = vAxisUp.normal();

	mat.loadIdentity();
	mat.setAxisX(vAxisRight);
	mat.setAxisY(vAxisForward);
	mat.setAxisZ(vAxisUp);
}

//*****************************************************************************
void VuMathUtil::buildOrientationMatrixUp(const VuVector3 &vFwd, const VuVector3 &vUp, VuMatrix &mat)
{
	VuVector3 vAxisUp = vUp;
	VuVector3 vAxisRight = VuCross(vFwd, vUp);
	VuVector3 vAxisForward = VuCross(vUp, vAxisRight);

	if ( vAxisForward.magSquared() < FLT_MIN || vAxisRight.magSquared() < FLT_MIN || vAxisUp.magSquared() < FLT_MIN )
	{
		vAxisRight = VuVector3(1,0,0);
		vAxisForward = VuVector3(0,1,0);
		vAxisUp = VuVector3(0,0,1);
	}

	vAxisRight = vAxisRight.normal();
	vAxisForward = vAxisForward.normal();
	vAxisUp = vAxisUp.normal();

	mat.loadIdentity();
	mat.setAxisX(vAxisRight);
	mat.setAxisY(vAxisForward);
	mat.setAxisZ(vAxisUp);
}

//*****************************************************************************
void VuMathUtil::buildOrthographicProjectionMatrix(const VuRect &rect, float fNear, float fFar, VuMatrix &mat)
{
	float l = rect.getLeft();
	float r = rect.getRight();
	float t = rect.getTop();
	float b = rect.getBottom();
	float n = fNear;
	float f = fFar;

	mat = VuMatrix(
		VuVector4(    2/(r-l),           0,         0,       0),
		VuVector4(          0,     2/(t-b),         0,       0),
		VuVector4(          0,           0, 1/(n - f),       0),
		VuVector4((l+r)/(l-r), (t+b)/(b-t), n/(n - f),       1)
	);

}


/******************************************************************************
  Copyright (c) 2008-2012 Ryan Juckett
  http://www.ryanjuckett.com/
  
  This software is provided 'as-is', without any express or implied
  warranty. In no event will the authors be held liable for any damages
  arising from the use of this software.
  
  Permission is granted to anyone to use this software for any purpose,
  including commercial applications, and to alter it and redistribute it
  freely, subject to the following restrictions:
  
  1. The origin of this software must not be misrepresented; you must not
     claim that you wrote the original software. If you use this software
     in a product, an acknowledgment in the product documentation would be
     appreciated but is not required.
  
  2. Altered source versions must be plainly marked as such, and must not be
     misrepresented as being the original software.
  
  3. This notice may not be removed or altered from any source
     distribution.
******************************************************************************/
void VuMathUtil::calcDampedSimpleHarmonicMotion(
	float *pPos,            // position value to update
	float *pVel,            // velocity value to update
	float equilibriumPos,   // position to approach
	float deltaTime,        // time to update over
	float angularFrequency, // angular frequency of motion
	float dampingRatio)     // damping ratio of motion
{
	const float epsilon = 0.0001f;

	// if there is no angular frequency, the spring will not move
	if (angularFrequency < epsilon)
		return;

	// clamp the damping ratio in legal range
	if (dampingRatio < 0.0f)
		dampingRatio = 0.0f;

	// calculate initial state in equilibrium relative space
	const float initialPos = *pPos - equilibriumPos;
	const float initialVel = *pVel;

	// if over-damped
	if (dampingRatio > 1.0f + epsilon)
	{
		// calculate constants based on motion parameters
		// Note: These could be cached off between multiple calls using the same
		//       parameters for deltaTime, angularFrequency and dampingRatio.
		const float za = -angularFrequency * dampingRatio;
		const float zb = angularFrequency * VuSqrt(dampingRatio*dampingRatio - 1.0f);
		const float z1 = za - zb;
		const float z2 = za + zb;
		const float expTerm1 = VuExp(z1 * deltaTime);
		const float expTerm2 = VuExp(z2 * deltaTime);

		// update motion
		float c1 = (initialVel - initialPos*z2) / (-2.0f*zb); // z1 - z2 = -2*zb
		float c2 = initialPos - c1;
		*pPos = equilibriumPos + c1*expTerm1 + c2*expTerm2;
		*pVel = c1*z1*expTerm1 + c2*z2*expTerm2;
	}
	// else if critically damped
	else if (dampingRatio > 1.0f - epsilon)
	{
		// calculate constants based on motion parameters
		// Note: These could be cached off between multiple calls using the same
		//       parameters for deltaTime, angularFrequency and dampingRatio.
		const float expTerm = VuExp(-angularFrequency * deltaTime);

		// update motion
		float c1 = initialVel + angularFrequency * initialPos;
		float c2 = initialPos;
		float c3 = (c1*deltaTime + c2) * expTerm;
		*pPos = equilibriumPos + c3;
		*pVel = (c1*expTerm) - (c3*angularFrequency);
	}
	// else under-damped
	else
	{
		// calculate constants based on motion parameters
		// Note: These could be cached off between multiple calls using the same
		//       parameters for deltaTime, angularFrequency and dampingRatio.
		const float omegaZeta = angularFrequency * dampingRatio;
		const float alpha = angularFrequency * VuSqrt(1.0f - dampingRatio*dampingRatio);
		const float expTerm = VuExp(-omegaZeta * deltaTime);
		const float cosTerm = VuCos(alpha * deltaTime);
		const float sinTerm = VuSin(alpha * deltaTime);

		// update motion
		float c1 = initialPos;
		float c2 = (initialVel + omegaZeta*initialPos) / alpha;
		*pPos = equilibriumPos + expTerm*(c1*cosTerm + c2*sinTerm);
		*pVel = -expTerm*((c1*omegaZeta - c2*alpha)*cosTerm +
			(c1*alpha + c2*omegaZeta)*sinTerm);
	}
}
