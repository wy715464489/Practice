//*****************************************************************************
//
//  Copyright (c) 2007-2008 Ralf Knoesel
//  Copyright (c) 2008-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  Water surface implementation
// 
//*****************************************************************************

#include <float.h>
#include "VuWaterSurface.h"
#include "VuWater.h"
#include "VuEngine/Assets/VuWaterMapAsset.h"
#include "VuEngine/Assets/VuLightMapAsset.h"
#include "VuEngine/Math/VuVector2.h"
#include "VuEngine/Math/VuMatrix.h"
#include "VuEngine/Containers/VuDbrt.h"
#include "VuEngine/Util/VuImageUtil.h"


//*****************************************************************************
VuWaterSurface::VuWaterSurface(const VuWaterSurfaceDesc &desc, VuEntity *pOwnerEntity):
	mpOwnerEntity(pOwnerEntity),
	mpDbrtNode(VUNULL),
	mpWaveDbrt(VUNULL),
	mpWaterMap(VUNULL),
	mpLightMap(VUNULL)
{
	modify(desc);

	// create wave dynamic bounding rectangle tree
	mpWaveDbrt = new VuDbrt(128, 64);
}

//*****************************************************************************
VuWaterSurface::~VuWaterSurface()
{
	VuWater::IF()->removeSurface(this);

	delete mpWaveDbrt;
}

//*****************************************************************************
void VuWaterSurface::modify(const VuWaterSurfaceDesc &desc)
{
	// save desc
	mDesc = desc;

	// recalculate params
	mSizeX = 1 << desc.mPowSizeX;
	mSizeY = 1 << desc.mPowSizeY;

	// recalcualte transform and inverse transform
	mTransform.loadIdentity();
	mTransform.translate(mDesc.mPos);
	mTransform.rotateZLocal(mDesc.mRotZ);
	mInverseTransform = mTransform;
	mInverseTransform.invert();

	// recalculate aabb/extent
	mExtent = VuVector2(0.5f*mSizeX, 0.5f*mSizeY);
	VuAabb localAabb;
	localAabb.mMin = VuVector3(-mExtent.mX, -mExtent.mY, -mDesc.mMaxWaveDepth);
	localAabb.mMax = VuVector3( mExtent.mX,  mExtent.mY,  mDesc.mMaxWaveHeight);
	mWorldAabb = VuAabb(localAabb, mTransform);

	// maps
	mpWaterMap = mDesc.mpWaterMapAsset;
	mpLightMap = mDesc.mpLightMapAsset;

	VuWater::IF()->rebinSurface(this);
}

//*****************************************************************************
VUUINT8 VuWaterSurface::getShadow(const VuVector3 &vPos) const
{
	if ( mpWaterMap )
	{
		VuVector3 vLocalPos = mInverseTransform.transform(vPos);
	
		float fX = (vLocalPos.mX + mExtent.mX)/mSizeX;
		float fY = (vLocalPos.mY + mExtent.mY)/mSizeY;

		int mapWidth = mpWaterMap->getWidth();
		int mapHeight = mpWaterMap->getHeight();

		fX = fX*(mapWidth - 1);
		fY = fY*(mapHeight - 1);

		int x = VuTruncate(fX);
		int y = VuTruncate(fY);
		float remainderX = fX - x;
		float remainderY = fY - y;

		const VUUINT16 *pTexels = &mpWaterMap->getSFD16().begin();

		VUUINT16 texel0 = pTexels[(y+0)*mapWidth + (x+0)];
		float shadow0 = (float)VU_EXTRACT_R_FROM_565(texel0);

		VUUINT16 texel1 = pTexels[(y+0)*mapWidth + (x+1)];
		float shadow1 = (float)VU_EXTRACT_R_FROM_565(texel1);

		VUUINT16 texel2 = pTexels[(y+1)*mapWidth + (x+0)];
		float shadow2 = (float)VU_EXTRACT_R_FROM_565(texel2);

		VUUINT16 texel3 = pTexels[(y+1)*mapWidth + (x+1)];
		float shadow3 = (float)VU_EXTRACT_R_FROM_565(texel3);

		return (VUUINT8)VuRound(VuLerp( VuLerp(shadow0, shadow1, remainderX), VuLerp(shadow2, shadow3, remainderX), remainderY));
	}

	return 255;
}

//*****************************************************************************
VuColor VuWaterSurface::getLight(const VuVector3 &vPos) const
{
	VuColor color(0,0,0);

	if ( mpLightMap )
	{
		VuVector3 vLocalPos = mInverseTransform.transform(vPos);
	
		float fX = (vLocalPos.mX + mExtent.mX)/mSizeX;
		float fY = (vLocalPos.mY + mExtent.mY)/mSizeY;

		int mapWidth = mpLightMap->getWidth();
		int mapHeight = mpLightMap->getHeight();

		fX = fX*(mapWidth - 1);
		fY = fY*(mapHeight - 1);

		int x = VuTruncate(fX);
		int y = VuTruncate(fY);
		float remainderX = fX - x;
		float remainderY = fY - y;

		const VUUINT16 *pTexels = &mpLightMap->getRGB16().begin();

		VUUINT16 texel0 = pTexels[(y+0)*mapWidth + (x+0)];
		float r0 = (float)VU_EXTRACT_R_FROM_565(texel0);
		float g0 = (float)VU_EXTRACT_G_FROM_565(texel0);
		float b0 = (float)VU_EXTRACT_B_FROM_565(texel0);

		VUUINT16 texel1 = pTexels[(y+0)*mapWidth + (x+1)];
		float r1 = (float)VU_EXTRACT_R_FROM_565(texel1);
		float g1 = (float)VU_EXTRACT_G_FROM_565(texel1);
		float b1 = (float)VU_EXTRACT_B_FROM_565(texel1);

		VUUINT16 texel2 = pTexels[(y+1)*mapWidth + (x+0)];
		float r2 = (float)VU_EXTRACT_R_FROM_565(texel2);
		float g2 = (float)VU_EXTRACT_G_FROM_565(texel2);
		float b2 = (float)VU_EXTRACT_B_FROM_565(texel2);

		VUUINT16 texel3 = pTexels[(y+1)*mapWidth + (x+1)];
		float r3 = (float)VU_EXTRACT_R_FROM_565(texel3);
		float g3 = (float)VU_EXTRACT_G_FROM_565(texel3);
		float b3 = (float)VU_EXTRACT_B_FROM_565(texel3);

		color.mR = (VUUINT8)VuRound(VuLerp( VuLerp(r0, r1, remainderX), VuLerp(r2, r3, remainderX), remainderY));
		color.mG = (VUUINT8)VuRound(VuLerp( VuLerp(g0, g1, remainderX), VuLerp(g2, g3, remainderX), remainderY));
		color.mB = (VUUINT8)VuRound(VuLerp( VuLerp(b0, b1, remainderX), VuLerp(b2, b3, remainderX), remainderY));
	}

	return color;
}

//*****************************************************************************
float VuWaterSurface::calcDistance2dSquared(const VuVector3 &vPos) const
{
	VuVector3 vLocalPos = mInverseTransform.transform(vPos);

	VuVector2 delta;
	delta.mX = VuAbs(vLocalPos.mX) - mExtent.mX;
	delta.mY = VuAbs(vLocalPos.mY) - mExtent.mY;

	delta.mX = VuMax(delta.mX, 0.0f);
	delta.mY = VuMax(delta.mY, 0.0f);

	return delta.magSquared();
}

//*****************************************************************************
float VuWaterSurface::calcDistance3dSquared(const VuVector3 &vPos) const
{
	VuVector3 vLocalPos = mInverseTransform.transform(vPos);

	float fExtentZ = 0.5f*(mDesc.mMaxWaveHeight + mDesc.mMaxWaveDepth);
	float fCenterZ = 0.5f*(mDesc.mMaxWaveHeight - mDesc.mMaxWaveDepth);
	vLocalPos.mZ -= fCenterZ;

	VuVector3 delta;
	delta.mX = VuAbs(vLocalPos.mX) - mExtent.mX;
	delta.mY = VuAbs(vLocalPos.mY) - mExtent.mY;
	delta.mZ = VuAbs(vLocalPos.mZ) - fExtentZ;

	delta.mX = VuMax(delta.mX, 0.0f);
	delta.mY = VuMax(delta.mY, 0.0f);
	delta.mZ = VuMax(delta.mZ, 0.0f);

	return delta.magSquared();
}

//*****************************************************************************
float VuWaterSurface::calcReflectionDistance3dSquared(const VuVector3 &vPos) const
{
	VuVector3 vLocalPos = mInverseTransform.transform(vPos);

	float fExtentZ = 0.5f*(mDesc.mReflectionHeight + mDesc.mMaxWaveDepth);
	float fCenterZ = 0.5f*(mDesc.mReflectionHeight - mDesc.mMaxWaveDepth);
	vLocalPos.mZ -= fCenterZ;

	VuVector3 delta;
	delta.mX = VuAbs(vLocalPos.mX) - mExtent.mX;
	delta.mY = VuAbs(vLocalPos.mY) - mExtent.mY;
	delta.mZ = VuAbs(vLocalPos.mZ) - fExtentZ;

	delta.mX = VuMax(delta.mX, 0.0f);
	delta.mY = VuMax(delta.mY, 0.0f);
	delta.mZ = VuMax(delta.mZ, 0.0f);

	float distSquared = delta.magSquared();

	distSquared -= mDesc.mReflectionOffset*mDesc.mReflectionOffset;
	distSquared = VuMax(distSquared, 0.0f);

	return distSquared;
}