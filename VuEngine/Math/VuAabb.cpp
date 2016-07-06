//*****************************************************************************
//
//  Copyright (c) 2006-2008 Ralf Knoesel
//  Copyright (c) 2008-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  Aabb class
// 
//*****************************************************************************

#include "VuAabb.h"
#include "VuMatrix.h"
#include "VuPackedVector.h"


VuAabb VuAabb::smAabbZero(VuVector3(0,0,0), VuVector3(0,0,0));
VuAabb VuAabb::smAabbOne(VuVector3(-1,-1,-1), VuVector3(1,1,1));


//*****************************************************************************
void VuAabb::addAabb(const VuAabb &aabb)
{
	mMin = VuMin(mMin, aabb.mMin);
	mMax = VuMax(mMax, aabb.mMax);
}

//*****************************************************************************
void VuAabb::addAabb(const VuAabb &aabb, const VuMatrix &mat)
{
	VuVector3 verts[8];
	aabb.getVerts(verts);

	for ( int i = 0; i < 8; i++ )
		addPoint(mat.transform(verts[i]));
}

//*****************************************************************************
void VuAabb::addSphere(const VuVector3 &vPos, float radius)
{
	mMin.mX = VuMin(mMin.mX, vPos.mX - radius);
	mMin.mY = VuMin(mMin.mY, vPos.mY - radius);
	mMin.mZ = VuMin(mMin.mZ, vPos.mZ - radius);

	mMax.mX = VuMax(mMax.mX, vPos.mX + radius);
	mMax.mY = VuMax(mMax.mY, vPos.mY + radius);
	mMax.mZ = VuMax(mMax.mZ, vPos.mZ + radius);
}

//*****************************************************************************
void VuAabb::getVerts(VuVector3 *pVerts) const
{
	pVerts[0] = VuVector3(mMin.mX, mMin.mY, mMin.mZ);
	pVerts[1] = VuVector3(mMax.mX, mMin.mY, mMin.mZ);
	pVerts[2] = VuVector3(mMin.mX, mMax.mY, mMin.mZ);
	pVerts[3] = VuVector3(mMax.mX, mMax.mY, mMin.mZ);
	pVerts[4] = VuVector3(mMin.mX, mMin.mY, mMax.mZ);
	pVerts[5] = VuVector3(mMax.mX, mMin.mY, mMax.mZ);
	pVerts[6] = VuVector3(mMin.mX, mMax.mY, mMax.mZ);
	pVerts[7] = VuVector3(mMax.mX, mMax.mY, mMax.mZ);
}

//*****************************************************************************
void VuAabb::getVerts(VuPackedVector3 *pVerts) const
{
	pVerts[0] = VuPackedVector3(mMin.mX, mMin.mY, mMin.mZ);
	pVerts[1] = VuPackedVector3(mMax.mX, mMin.mY, mMin.mZ);
	pVerts[2] = VuPackedVector3(mMin.mX, mMax.mY, mMin.mZ);
	pVerts[3] = VuPackedVector3(mMax.mX, mMax.mY, mMin.mZ);
	pVerts[4] = VuPackedVector3(mMin.mX, mMin.mY, mMax.mZ);
	pVerts[5] = VuPackedVector3(mMax.mX, mMin.mY, mMax.mZ);
	pVerts[6] = VuPackedVector3(mMin.mX, mMax.mY, mMax.mZ);
	pVerts[7] = VuPackedVector3(mMax.mX, mMax.mY, mMax.mZ);
}

//*****************************************************************************
const VUUINT16 *VuAabb::getEdgeIndices() const
{
	static VUUINT16 sEdgeIndices[] = {
		0, 1, 1, 3, 3, 2, 2, 0,
		0, 4, 1, 5, 3, 7, 2, 6,
		4, 5, 5, 7, 7, 6, 6, 4
	};

	return sEdgeIndices;
}

//*****************************************************************************
const VUUINT16 *VuAabb::getTriIndices() const
{
	static VUUINT16 sTriIndices[] = {
		0, 2, 1, 1, 2, 3, // -z
		6, 2, 4, 4, 2, 0, // -x
		4, 0, 5, 5, 0, 1, // -y
		5, 1, 7, 7, 1, 3, // +x
		7, 3, 6, 6, 3, 2, // +y
		6, 4, 7, 7, 4, 5, // +z
	};

	return sTriIndices;
}

//*****************************************************************************
void VuAabb::getPosNorVerts(VuPackedVector3 *pVerts) const
{
	VuPackedVector3 vPosVerts[8];
	getVerts(vPosVerts);

	#define WRITE_SIDE(offset, idx0, idx1, idx2, idx3, norx, nory, norz)								\
	{																									\
		pVerts[offset + 0] = vPosVerts[idx0]; pVerts[offset + 1] = VuPackedVector3(norx, nory, norz);	\
		pVerts[offset + 2] = vPosVerts[idx1]; pVerts[offset + 3] = VuPackedVector3(norx, nory, norz);	\
		pVerts[offset + 4] = vPosVerts[idx2]; pVerts[offset + 5] = VuPackedVector3(norx, nory, norz);	\
		pVerts[offset + 6] = vPosVerts[idx3]; pVerts[offset + 7] = VuPackedVector3(norx, nory, norz);	\
	}

	WRITE_SIDE( 0, 2, 0, 4, 6, -1.0f,  0.0f,  0.0f);
	WRITE_SIDE( 8, 1, 3, 7, 5,  1.0f,  0.0f,  0.0f);
	WRITE_SIDE(16, 0, 1, 5, 4,  0.0f, -1.0f,  0.0f);
	WRITE_SIDE(24, 3, 2, 6, 7,  0.0f,  1.0f,  0.0f);
	WRITE_SIDE(32, 2, 3, 1, 0,  0.0f,  0.0f, -1.0f);
	WRITE_SIDE(40, 4, 5, 7, 6,  0.0f,  0.0f,  1.0f);
}

//*****************************************************************************
const VUUINT16 *VuAabb::getPosNorTriIndices() const
{
	static VUUINT16 sPosNorTriIndices[] = {
		 0,  1,  2,  2,  3,  0,
		 4,  5,  6,  6,	 7,  4,
		 8,  9, 10, 10,	11,  8,
		12, 13, 14, 14,	15, 12,
		16, 17, 18, 18,	19, 16,
		20, 21, 22, 22,	23, 20,
	};

	return sPosNorTriIndices;
}