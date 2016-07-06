//*****************************************************************************
//
//  Copyright (c) 2008-2008 Ralf Knoesel
//  Copyright (c) 2008-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  PSSM implementation class
// 
//*****************************************************************************

#include "VuPSSM.h"
#include "VuGfxComposerCommands.h"
#include "VuEngine/Components/3dDraw/Vu3dDrawManager.h"
#include "VuEngine/Gfx/Camera/VuCamera.h"
#include "VuEngine/Gfx/Shadow/VuShadowVolume.h"
#include "VuEngine/Gfx/Shaders/VuShadowShader.h"
#include "VuEngine/Gfx/GfxSort/VuGfxSort.h"
#include "VuEngine/Gfx/Light/VuLightManager.h"
#include "VuEngine/HAL/Gfx/VuGfx.h"
#include "VuEngine/HAL/Gfx/VuTexture.h"
#include "VuEngine/HAL/Gfx/VuShadowRenderTarget.h"
#include "VuEngine/HAL/Gfx/VuShaderProgram.h"
#include "VuEngine/HAL/Gfx/VuPipelineState.h"
#include "VuEngine/Math/VuMatrix.h"
#include "VuEngine/Math/VuRect.h"
#include "VuEngine/Math/VuAabb.h"


struct ShadowRenderConstants
{
	VuMatrix				mTextureMatrices[4];
	VuVector4				mSplitPositions;
	VuShadowRenderTarget	*mpShadowRenderTarget;
	int						mMapCount;
	int						mSize;
};
static ShadowRenderConstants sShadowRenderConstants;


//*****************************************************************************
VuPSSM::VuPSSM():
	mCount(0),
	mSize(0),
	mRejectionScale(0),
	mpShadowRenderTarget(VUNULL)
{
	mTextureMatrices.resize(4);
	mSplitPositions.resize(5);
	mSplitRatios.resize(5);
}

//*****************************************************************************
VuPSSM::~VuPSSM()
{
	destroyResources();
}

//*****************************************************************************
void VuPSSM::setSplitPositions(float split1, float split2, float split3)
{
	mSplitPositions[1] = split1;
	mSplitPositions[2] = split2;
	mSplitPositions[3] = split3;
}

//*****************************************************************************
void VuPSSM::submitCommands(const VuCamera &camera, VUUINT32 zoneMask, bool bReflection, const VuVector4 &reflectionPlane)
{
	updateResources();

	VuGfxSort::IF()->setViewportLayer(VuGfxSort::VPL_SHADOW1);

	// submit per-map commands
	for ( int i = 0; i < mCount; i++ )
	{
		// submit per-map commands
		VuGfxSort::IF()->setViewportLayer(VuGfxSort::VPL_SHADOW1 + i);
		VuGfxComposerSceneCommands::submitShadow(mpShadowRenderTarget, i);
	}

	// calculate the distances of split planes
	updateSplitRatios(camera);

	// calculate light view matrix
	VuVector3 lightPos = VuLightManager::IF()->directionalLight().mPosition;
	VuVector3 lightDir = VuLightManager::IF()->directionalLight().mDirection;

	VuMatrix lightMatrix;
	calcLightMatrix(lightPos, lightDir, lightMatrix);

	// create shadow volumes
	VuShadowClip combinedShadowClip;
	combinedShadowClip.create(lightPos, lightDir, camera.getFrustum());

	VuShadowVolume shadowVolumes[4];
	for ( int i = 0; i < mCount; i++ )
	{
		// calculate light aabb
		VuAabb lightAabb;
		calcLightAabb(i, camera, lightMatrix, lightAabb);

		// calculate light crop matrix
		VuMatrix lightCropMatrix;
		calcLightCropMatrix(i, lightAabb, lightMatrix, lightCropMatrix);

		// calculate texture scale bias matrix
		VuMatrix texScaleBiasMatrix;
		calcTextScaleBiasMatrix(i, texScaleBiasMatrix);

		// calculate final texture matrix
		mTextureMatrices[i] = lightCropMatrix*texScaleBiasMatrix;

		// create frustum for split
		VuFrustum frustum = camera.getFrustum();
		frustum.mDMin = mSplitPositions[i];
		frustum.mDMax = mSplitPositions[i+1];
		frustum.mUBound *= mSplitPositions[i]/camera.getNearPlane();
		frustum.mRBound *= mSplitPositions[i]/camera.getNearPlane();
		frustum.update();

		// create shadow volume
		shadowVolumes[i].mCropMatrix = lightCropMatrix;

		// for last (largest) shadow volume, use frustum... else use light aabb
		if ( i == mCount - 1 )
			shadowVolumes[i].mShadowClip.create(lightPos, lightDir, frustum);
		else
			shadowVolumes[i].mShadowClip.create(lightMatrix, lightAabb);
	}

	// submit draw commands
	VuGfxDrawShadowParams params(camera, combinedShadowClip);
	params.mShadowVolumeCount = mCount;
	params.mpShadowVolumes = shadowVolumes;
	params.mRejectionScale = mRejectionScale;
	params.mbDrawReflection = bReflection;
	params.mReflectionPlane = reflectionPlane;
	params.mZoneMask = zoneMask;

	Vu3dDrawManager::IF()->drawShadow(params);

	// submit render constants
	VuGfxSort::IF()->setViewportLayer(VuGfxSort::VPL_WORLD);
	submitRenderConstants();
}

//*****************************************************************************
VuTexture *VuPSSM::getTexture(int index)
{
	return mpShadowRenderTarget->getColorTexture(index);
}

//*****************************************************************************
void VuPSSM::resolveConstants(VuShaderProgram *pSP, VuMatExt *pMatExt)
{
	// PSSM
	pMatExt->mhConstShadowTextureMatrices	= pSP->getConstantByName("gShadowTextureMatrices");
	pMatExt->mhConstShadowMapSize			= pSP->getConstantByName("gShadowMapSize");
	pMatExt->mhConstShadowMapTexelSize		= pSP->getConstantByName("gShadowMapTexelSize");
	pMatExt->mhConstShadowMapSplits			= pSP->getConstantByName("gShadowMapSplits");
	pMatExt->miSampShadowMaps[0]			= pSP->getSamplerIndexByName("gShadowMap0");
	pMatExt->miSampShadowMaps[1]			= pSP->getSamplerIndexByName("gShadowMap1");
	pMatExt->miSampShadowMaps[2]			= pSP->getSamplerIndexByName("gShadowMap2");
	pMatExt->miSampShadowMaps[3]			= pSP->getSamplerIndexByName("gShadowMap3");

	pMatExt->mShadowMapCount = 0;
	for ( int i = 0; i < 4; i++ )
	{
		if ( pMatExt->miSampShadowMaps[i] >= 0 )
			pMatExt->mShadowMapCount++;
		else
			break;
	}
}

//*****************************************************************************
void VuPSSM::setConstants(VuShaderProgram *pSP, const VuMatExt *pMatExt)
{
	const ShadowRenderConstants &rc = sShadowRenderConstants;

	if ( pMatExt->mhConstShadowTextureMatrices  )
	{
		float fSize = (float)rc.mSize;
		float fTexelSize = 1.0f/fSize;

		pSP->setConstantMatrixArray(pMatExt->mhConstShadowTextureMatrices, rc.mTextureMatrices, rc.mMapCount, false);

		if ( pMatExt->mhConstShadowMapSize )
			pSP->setConstantFloat(pMatExt->mhConstShadowMapSize, fSize);

		if ( pMatExt->mhConstShadowMapTexelSize )
			pSP->setConstantFloat(pMatExt->mhConstShadowMapTexelSize, fTexelSize);

		if ( pMatExt->mhConstShadowMapSplits )
			pSP->setConstantVector4(pMatExt->mhConstShadowMapSplits, rc.mSplitPositions);

		for ( int i = 0; i < pMatExt->mShadowMapCount; i++ )
			VuGfx::IF()->setDepthTexture(pMatExt->miSampShadowMaps[i], rc.mpShadowRenderTarget, i);
	}
}

//*****************************************************************************
void VuPSSM::updateResources()
{
	bool update = false;
	if ( mpShadowRenderTarget == VUNULL )
		update = true;
	else if ( mpShadowRenderTarget->getCount() != mCount )
		update = true;
	else if ( mpShadowRenderTarget->getWidth() != mSize )
		update = true;

	if ( update )
	{
		VuGfxSort::IF()->flush();

		destroyResources();

		mpShadowRenderTarget = VuGfx::IF()->createShadowRenderTarget(mSize, mSize, mCount);

		sShadowRenderConstants.mpShadowRenderTarget = mpShadowRenderTarget;
	}
}

//*****************************************************************************
void VuPSSM::destroyResources()
{
	if ( mpShadowRenderTarget )
		mpShadowRenderTarget->removeRef();
	mpShadowRenderTarget = VUNULL;
}

//*****************************************************************************
void VuPSSM::updateSplitRatios(const VuCamera &camera)
{
	// make sure border values are accurate
	mSplitPositions[0] = camera.getNearPlane();
	mSplitPositions[mCount] = camera.getFarPlane();

	// calculate split ratios
	for ( int i = 0; i <= mCount; i++ )
		mSplitRatios[i] = (mSplitPositions[i] - camera.getNearPlane())/(camera.getFarPlane() - camera.getNearPlane());
}

//*****************************************************************************
void VuPSSM::calcLightMatrix(const VuVector3 &lightPos, const VuVector3 &lightDir, VuMatrix &lightMatrix)
{
	// pick some random numbers for up-vector right and left parts
	// (reduce chance of severe shadow map jitter due to texel-axis aligned geometry)
	#define OFFSET0  0.237f
	#define OFFSET1 -0.173f

	// view matrix
	VuVector3 lightUp = VuAbs(lightDir.mZ) < 0.707f ? VuVector3(OFFSET0, OFFSET1, 1) : VuVector3(OFFSET0, 1, OFFSET1);

	VuVector3 vAxisForward = lightDir;
	VuVector3 vAxisRight = VuCross(vAxisForward, lightUp).normal();
	VuVector3 vAxisUp = VuCross(vAxisRight, vAxisForward);

	lightMatrix.loadIdentity();
	lightMatrix.setAxisX(vAxisRight);
	lightMatrix.setAxisY(vAxisUp);
	lightMatrix.setAxisZ(-vAxisForward);
	lightMatrix.setTrans(lightPos);
	lightMatrix.invert();

	// flip the z axis (so that z=0 is near plane)
	lightMatrix.scale(VuVector3(1,1,-1));
}

//*****************************************************************************
void VuPSSM::calcLightAabb(int iSplit, const VuCamera &camera, const VuMatrix &lightMatrix, VuAabb &lightAabb)
{
	VuVector3 vPos;
	float fRadius;
	camera.getMinEnclosingSphere(vPos, fRadius, mSplitRatios[iSplit], mSplitRatios[iSplit+1]);

	fRadius += 1.0f/mSize;

	lightAabb.addPoint(lightMatrix.transform(vPos));
	lightAabb.mMax += VuVector3(fRadius, fRadius, fRadius);
	lightAabb.mMin -= VuVector3(fRadius, fRadius, fRadius);

	// use default near plane
	lightAabb.mMin.mZ = 0;
}

//*****************************************************************************
void VuPSSM::calcLightCropMatrix(int iSplit, const VuAabb &lightAabb, const VuMatrix &lightMatrix, VuMatrix &lightCropMatrix)
{
	// build crop matrix
	VuMatrix cropMatrix;
	{
		// calculate scale
		float fScaleX = 2.0f/(lightAabb.mMax.mX - lightAabb.mMin.mX);
		float fScaleY = 2.0f/(lightAabb.mMax.mY - lightAabb.mMin.mY);
		float fScaleZ = 1.0f/(lightAabb.mMax.mZ - lightAabb.mMin.mZ);

		// quantize scale
		float fQuantizer = 64.0f;
		fScaleX = fQuantizer/VuCeil(fQuantizer/fScaleX);
		fScaleY = fQuantizer/VuCeil(fQuantizer/fScaleY);
		fScaleZ = fQuantizer/VuCeil(fQuantizer/fScaleZ);

		// calculate offset
		float fOffsetX = -0.5f*(lightAabb.mMax.mX + lightAabb.mMin.mX)*fScaleX;
		float fOffsetY = -0.5f*(lightAabb.mMax.mY + lightAabb.mMin.mY)*fScaleY;
		float fOffsetZ = -lightAabb.mMin.mZ * fScaleZ;

		// quantize offset
		float fHalfTextureSize = 0.5f*mSize;
		fOffsetX = VuCeil(fOffsetX*fHalfTextureSize)/fHalfTextureSize;
		fOffsetY = VuCeil(fOffsetY*fHalfTextureSize)/fHalfTextureSize;
		fOffsetZ = VuCeil(fOffsetZ*fQuantizer)/fQuantizer;

		cropMatrix = VuMatrix(
			VuVector4( fScaleX,        0,        0,       0),
			VuVector4(       0,	 fScaleY,        0,       0),
			VuVector4(       0,        0,  fScaleZ,       0),
			VuVector4(fOffsetX, fOffsetY, fOffsetZ,       1)
		);
	}

	lightCropMatrix = lightMatrix*cropMatrix;
}

//*****************************************************************************
void VuPSSM::calcTextScaleBiasMatrix(int iSplit, VuMatrix &texScaleBiasMatrix)
{
	float fTexOffset = 0.5f + 0.5f/(float)mSize;

#if defined VUANDROID // iOS uses Metal for PSSM
	texScaleBiasMatrix = VuMatrix(
		VuVector4(      0.5f,          0,        0,       0),
		VuVector4(         0,       0.5f,        0,       0),
		VuVector4(         0,          0,     0.5f,       0),
		VuVector4(fTexOffset, fTexOffset,     0.5f,       1)
	);
#elif defined VUPS4
	texScaleBiasMatrix = VuMatrix(
		VuVector4(      0.5f,          0,        0,       0),
		VuVector4(         0,	   -0.5f,        0,       0),
		VuVector4(         0,          0,     0.5f,       0),
		VuVector4(fTexOffset, fTexOffset,     0.5f,       1)
	);
#else
	texScaleBiasMatrix = VuMatrix(
		VuVector4(      0.5f,          0,        0,       0),
		VuVector4(         0,	   -0.5f,        0,       0),
		VuVector4(         0,          0,        1,       0),
		VuVector4(fTexOffset, fTexOffset,        0,       1)
	);
#endif
}

//*****************************************************************************
void VuPSSM::submitRenderConstants()
{
	struct CommandData
	{
		static void callback(void *data)
		{
			CommandData *pData = static_cast<CommandData *>(data);

			sShadowRenderConstants = pData->mShadowRenderConstants;
		}
		ShadowRenderConstants	mShadowRenderConstants;
	};

	CommandData *pData = static_cast<CommandData *>(VuGfxSort::IF()->allocateCommandMemory(sizeof(CommandData)));

	ShadowRenderConstants &rc = pData->mShadowRenderConstants;
	memset(&rc, 0, sizeof(rc));

	rc.mpShadowRenderTarget = mpShadowRenderTarget;
	rc.mMapCount = mCount;
	rc.mSize = mSize;

	for ( int i = 0; i < mCount; i++ )
	{
		rc.mTextureMatrices[i] = mTextureMatrices[i];
		rc.mSplitPositions.setValue(i, mSplitPositions[i + 1]);
	}

	VuGfxSort::IF()->submitCommand(VuGfxSort::TRANS_BEGIN, 0, &CommandData::callback);
}
