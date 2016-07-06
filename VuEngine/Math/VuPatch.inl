//*****************************************************************************
//
//  Copyright (c) 2008-2008 Ralf Knoesel
//  Copyright (c) 2008-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  Patch inline implementation
// 
//*****************************************************************************


//*****************************************************************************
template<class T>
void VuPatch<T>::set(const T &i0, const T &i1, const T &i2, const T &i3,
	const T &o0, const T &o1, const T &o2, const T &o3,
	const T &o4, const T &o5, const T &o6, const T &o7)
{
	T du, dv;

	// vert 0
	du = (o7 - i1)*(1.0f/6.0f);
	dv = (o0 - i3)*(1.0f/6.0f);
	mVal[0] = i0;
	mVec[7] = i0 - dv;
	mVec[0] = i0 - du;
	mInt[0] = i0 - du - dv;

	// vert 1
	du = (i0 - o2)*(1.0f/6.0f);
	dv = (o1 - i2)*(1.0f/6.0f);
	mVal[1] = i1;
	mVec[1] = i1 + du;
	mVec[2] = i1 - dv;
	mInt[1] = i1 - dv + du;

	// vert 2
	du = (i3 - o3)*(1.0f/6.0f);
	dv = (i1 - o4)*(1.0f/6.0f);
	mVal[2] = i2;
	mVec[3] = i2 + dv;
	mVec[4] = i2 + du;
	mInt[2] = i2 + du + dv;

	// vert 3
	du = (o6 - i2)*(1.0f/6.0f);
	dv = (i0 - o5)*(1.0f/6.0f);
	mVal[3] = i3;
	mVec[5] = i3 - du;
	mVec[6] = i3 + dv;
	mInt[3] = i3 + dv - du;
}

//*****************************************************************************
template<class T>
T VuPatch<T>::interpolate(float u, float v) const
{
	float U = 1.0f - u;
	float u0 = U*U*U;
	float u1 = 3.0f*u*U*U;
	float u2 = 3.0f*u*u*U;
	float u3 = u*u*u;

	float V = 1.0f - v;
	float v0 = V*V*V;
	float v1 = 3.0f*v*V*V;
	float v2 = 3.0f*v*v*V;
	float v3 = v*v*v;

	T val =
		mVal[0]*v0*u0 + mVec[7]*v1*u0 + mVec[6]*v2*u0 + mVal[3]*v3*u0 + 
		mVec[0]*v0*u1 + mInt[0]*v1*u1 + mInt[3]*v2*u1 + mVec[5]*v3*u1 + 
		mVec[1]*v0*u2 + mInt[1]*v1*u2 + mInt[2]*v2*u2 + mVec[4]*v3*u2 + 
		mVal[1]*v0*u3 + mVec[2]*v1*u3 + mVec[3]*v2*u3 + mVal[2]*v3*u3;

	return val;
}

//*****************************************************************************
template<class T>
T VuPatch<T>::interpolate(float u, float v, T &gradU, T &gradV) const
{
	float U = 1.0f - u;
	float u0 = U*U*U;
	float u1 = 3.0f*u*U*U;
	float u2 = 3.0f*u*u*U;
	float u3 = u*u*u;

	float V = 1.0f - v;
	float v0 = V*V*V;
	float v1 = 3.0f*v*V*V;
	float v2 = 3.0f*v*v*V;
	float v3 = v*v*v;

	T val =
		mVal[0]*v0*u0 + mVec[7]*v1*u0 + mVec[6]*v2*u0 + mVal[3]*v3*u0 + 
		mVec[0]*v0*u1 + mInt[0]*v1*u1 + mInt[3]*v2*u1 + mVec[5]*v3*u1 + 
		mVec[1]*v0*u2 + mInt[1]*v1*u2 + mInt[2]*v2*u2 + mVec[4]*v3*u2 + 
		mVal[1]*v0*u3 + mVec[2]*v1*u3 + mVec[3]*v2*u3 + mVal[2]*v3*u3;

	// calculate gradient in u direction
	float du0_du = -3*U*U;
	float du1_du = 3*U*(1.0f - 3.0f*u);
	float du2_du = 3*u*(2.0f - 3.0f*u);
	float du3_du = 3*u*u;

	gradU =
		mVal[0]*v0*du0_du + mVec[7]*v1*du0_du + mVec[6]*v2*du0_du + mVal[3]*v3*du0_du + 
		mVec[0]*v0*du1_du + mInt[0]*v1*du1_du + mInt[3]*v2*du1_du + mVec[5]*v3*du1_du + 
		mVec[1]*v0*du2_du + mInt[1]*v1*du2_du + mInt[2]*v2*du2_du + mVec[4]*v3*du2_du + 
		mVal[1]*v0*du3_du + mVec[2]*v1*du3_du + mVec[3]*v2*du3_du + mVal[2]*v3*du3_du;

	// calculate gradient in y direction
	float dv0_dv = -3*V*V;
	float dv1_dv = 3*V*(1.0f - 3.0f*v);
	float dv2_dv = 3*v*(2.0f - 3.0f*v);
	float dv3_dv = 3*v*v;

	gradV =
		mVal[0]*dv0_dv*u0 + mVec[7]*dv1_dv*u0 + mVec[6]*dv2_dv*u0 + mVal[3]*dv3_dv*u0 + 
		mVec[0]*dv0_dv*u1 + mInt[0]*dv1_dv*u1 + mInt[3]*dv2_dv*u1 + mVec[5]*dv3_dv*u1 + 
		mVec[1]*dv0_dv*u2 + mInt[1]*dv1_dv*u2 + mInt[2]*dv2_dv*u2 + mVec[4]*dv3_dv*u2 + 
		mVal[1]*dv0_dv*u3 + mVec[2]*dv1_dv*u3 + mVec[3]*dv2_dv*u3 + mVal[2]*dv3_dv*u3;

	return val;
}
