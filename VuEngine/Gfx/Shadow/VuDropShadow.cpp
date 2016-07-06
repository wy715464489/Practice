//*****************************************************************************
//
//  Copyright (c) 2012-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  Drop Shadow
// 
//*****************************************************************************

#include "VuDropShadow.h"
#include "VuEngine/Assets/VuAssetFactory.h"
#include "VuEngine/Assets/VuTextureAsset.h"
#include "VuEngine/Assets/VuCollisionMeshAsset.h"
#include "VuEngine/Gfx/VuGfxUtil.h"
#include "VuEngine/Gfx/GfxSort/VuGfxSort.h"
#include "VuEngine/Gfx/Shaders/VuBlobShadowShader.h"
#include "VuEngine/Gfx/VuGfxDrawParams.h"
#include "VuEngine/HAL/Gfx/VuGfx.h"
#include "VuEngine/HAL/Gfx/VuTexture.h"
#include "VuEngine/HAL/Gfx/VuRenderTarget.h"
#include "VuEngine/Dynamics/VuDynamics.h"
#include "VuEngine/Gfx/Light/VuLightManager.h"
#include "VuEngine/Gfx/Shadow/VuShadowVolume.h"
#include "VuEngine/Dev/VuDevMenu.h"
#include "VuEngine/Dev/VuDevUtil.h"


static bool sShowDropShadow = false;


//*****************************************************************************
class VuDropShadowConvexResult : public btCollisionWorld::ConvexResultCallback
{
public:
	typedef VuArray<VuVector3> Verts;

	VuDropShadowConvexResult(Verts &verts, const VuVector3 &lightDir) : mVerts(verts), mLightDir(lightDir), mHitFraction(1.0f)
	{
		mVerts.resize(0);
	}

	virtual bool needsCollision(btBroadphaseProxy *proxy0) const
	{
		btCollisionObject *pColObj = static_cast<btCollisionObject *>(proxy0->m_clientObject);
		if ( pColObj->getInternalType() != btCollisionObject::CO_RIGID_BODY )
			return false;

		if ( !pColObj->getCollisionShape()->getUserPointer() )
			return false;

		VuRigidBody *pRigidBody = static_cast<VuRigidBody *>(pColObj);
		if ( pRigidBody->getExtendedFlags() & EXT_COL_ENGINE_BREAKABLE )
			return false;

		return true;
	}

	virtual	btScalar addSingleResult(btCollisionWorld::LocalConvexResult &convexResult, bool normalInWorldSpace)
	{
		const VuRigidBody *pRigidBody = static_cast<const VuRigidBody *>(convexResult.m_hitCollisionObject);
		const VuCollisionMeshAsset *pAsset = static_cast<const VuCollisionMeshAsset *>(pRigidBody->getCollisionShape()->getUserPointer());

		int triIndex = convexResult.m_localShapeInfo->m_triangleIndex;

		if ( pAsset->getTriangleMaterial(triIndex).mFlags & VuCollisionMeshAsset::VuMaterial::RECEIVE_SHADOWS )
		{
			VuMatrix transform = pRigidBody->getVuCenterOfMassTransform();

			VuVector3 v0 = pAsset->getVert(pAsset->getIndex(triIndex*3 + 0));
			VuVector3 v1 = pAsset->getVert(pAsset->getIndex(triIndex*3 + 1));
			VuVector3 v2 = pAsset->getVert(pAsset->getIndex(triIndex*3 + 2));

			if ( VuDot(mLightDir, VuCross(v1 - v0, v2 - v0)) < 0.0f )
			{
				mVerts.push_back(transform.transform(v0));
				mVerts.push_back(transform.transform(v1));
				mVerts.push_back(transform.transform(v2));

				mHitFraction = VuMin(mHitFraction, convexResult.m_hitFraction);
			}
		}

		return 1.0f;
	}

	Verts		&mVerts;
	VuVector3	mLightDir;
	float		mHitFraction;
};

//*****************************************************************************
class VuDropShadowRayResult : public btCollisionWorld::ClosestRayResultCallback
{
public:
	VuDropShadowRayResult(const btVector3 &rayFromWorld, const btVector3 &rayToWorld) : ClosestRayResultCallback(rayFromWorld, rayToWorld) {}

	virtual bool needsCollision(btBroadphaseProxy *proxy0) const
	{
		const btCollisionObject *pColObj = static_cast<const btCollisionObject *>(proxy0->m_clientObject);
		if ( pColObj->getInternalType() != btCollisionObject::CO_RIGID_BODY )
			return false;

		if ( !pColObj->getCollisionShape()->getUserPointer() )
			return false;

		const VuRigidBody *pRigidBody = static_cast<const VuRigidBody *>(pColObj);
		if ( pRigidBody->getExtendedFlags() & EXT_COL_ENGINE_BREAKABLE )
			return false;

		return true;
	}
};

//*****************************************************************************
VuDropShadow::VuDropShadow(int textureSize):
	mTextureSize(textureSize),
	mpGfxSortMaterial(VUNULL),
	mVerts(3*8)
{
	static VuDevBoolOnce sbOnce;
	if ( sbOnce && VuDevMenu::IF() )
		VuDevMenu::IF()->addBool("Shadow/ShowDropShadow", sShowDropShadow);

	VuBlobShadowShader *pBlobShadowShader = VuGfxUtil::IF()->blobShadowShader();

	VuGfxSortMaterialDesc desc;
	mpGfxSortMaterial = VuGfxSort::IF()->createMaterial(pBlobShadowShader->getPipelineState(), desc);

	// create texture
	mpRenderTarget = VuGfx::IF()->createRenderTarget(textureSize, textureSize);
}

//*****************************************************************************
VuDropShadow::~VuDropShadow()
{
	VuGfxSort::IF()->releaseMaterial(mpGfxSortMaterial);

	mpRenderTarget->removeRef();
}

//*****************************************************************************
void VuDropShadow::calculate(const VuMatrix &transform, const VuAabb &aabb)
{
	mObjectPos = transform.transform(aabb.getCenter());
	mObjectRadius = aabb.getExtents().mag();

	VuVector3 lightDir = VuLightManager::IF()->directionalLight().mDirection;

	// ray test
	{
		btVector3 from = VuDynamicsUtil::toBtVector3(mObjectPos);
		btVector3 to = VuDynamicsUtil::toBtVector3(mObjectPos + mParams.mHeightFadeFar*lightDir);

		VuDropShadowRayResult result(from, to);
		result.m_collisionFilterMask = (short)mParams.mCollisionMask;

		VuDynamics::IF()->getDynamicsWorld()->rayTest(from, to, result);

		mHeightFade = 1.0f - VuLinStep(mParams.mHeightFadeNear, mParams.mHeightFadeFar, result.m_closestHitFraction*mParams.mHeightFadeFar);
	}

	// sphere sweep
	mVerts.resize(0);
	if ( mHeightFade > FLT_EPSILON )
	{
		btSphereShape sphereShape(mObjectRadius);

		VuVector3 from = mObjectPos - mObjectRadius*lightDir;
		VuVector3 to = mObjectPos + mParams.mHeightFadeFar*lightDir;

		VuDropShadowConvexResult result(mVerts, lightDir);
		result.m_collisionFilterMask = (short)mParams.mCollisionMask;

		btTransform fromTransform;
		fromTransform.setIdentity();
		fromTransform.setOrigin(VuDynamicsUtil::toBtVector3(from));

		btTransform toTransform;
		toTransform.setIdentity();
		toTransform.setOrigin(VuDynamicsUtil::toBtVector3(to));

		VuDynamics::IF()->getDynamicsWorld()->convexSweepTest(&sphereShape, fromTransform, toTransform, result);
	}
}

//*****************************************************************************
void VuDropShadow::draw(const VuGfxDrawParams &params, Callback *pCB)
{
	if ( mpGfxSortMaterial == VUNULL )
		return;

	int vertCount = mVerts.size();
	if ( vertCount == 0 )
		return;

	float distFade = 1.0f - VuLinStep(mParams.mDistanceFadeNear, mParams.mDistanceFadeFar, VuDist(mObjectPos, params.mEyePos));

	VuColor color(0,0,0);
	color.mA = (VUUINT8)VuRound(255*mParams.mAlpha*mHeightFade*distFade);
	if ( color.mA == 0 )
		return;

	VUUINT prevVPL = VuGfxSort::IF()->getViewportLayer();

	VuGfxSort::IF()->setViewportLayer(VuGfxSort::VPL_SHADOW1);

	submitClearCommand();

	VuVector3 lightPos = VuLightManager::IF()->directionalLight().mPosition;
	VuVector3 lightDir = VuLightManager::IF()->directionalLight().mDirection;

	// calculate light view matrix
	VuMatrix lightMatrix;
	calcLightMatrix(lightPos, lightDir, lightMatrix);

	// calculate light aabb
	VuAabb lightAabb;
	calcLightAabb(lightMatrix, lightAabb);

	// calculate light crop matrix
	VuMatrix lightCropMatrix;
	calcLightCropMatrix(lightAabb, lightMatrix, lightCropMatrix);

	// calculate texture scale bias matrix
	VuMatrix texScaleBiasMatrix;
	calcTextScaleBiasMatrix(texScaleBiasMatrix);

	// calculate final texture matrix
	VuMatrix textureMatrix = lightCropMatrix*texScaleBiasMatrix;

	// create shadow volume
	VuShadowVolume shadowVolume;
	shadowVolume.mCropMatrix = lightCropMatrix;

	VuShadowClip shadowClip;
	VuGfxDrawShadowParams shadowParams(params.mCamera, shadowClip);
	shadowParams.mShadowVolumeCount = 1;
	shadowParams.mpShadowVolumes = &shadowVolume;
	shadowParams.mRejectionScale = params.mRejectionScale;
	shadowParams.mbDrawReflection = params.mbDrawReflection;
	shadowParams.mReflectionPlane = params.mReflectionPlane;

	pCB->onDropShadowDraw(shadowParams);

	if ( sShowDropShadow )
	{
		VuGfxSort::IF()->setViewportLayer(VuGfxSort::VPL_UI);
		
		int displayWidth, displayHeight;
		VuGfx::IF()->getDisplaySize(VUNULL, displayWidth, displayHeight);
		float displayAspectRatio = (float)displayWidth/displayHeight;
		
		VuRect rect(0.8f, 1.0f - 0.2f*displayAspectRatio, 0.2f, 0.2f*displayAspectRatio);
		VuGfxUtil::IF()->drawTexture2d(0, mpRenderTarget->getColorTexture(), rect);
	}

	VuGfxSort::IF()->setViewportLayer(prevVPL);

	submitShadow(textureMatrix, color);
}

//*****************************************************************************
void VuDropShadow::submitClearCommand()
{
	// submit clear command
	struct Clear
	{
		static void callback(void *data)
		{
			Clear *pData = static_cast<Clear *>(data);

			VuSetRenderTargetParams params(pData->mpRenderTarget);
			params.mColorLoadAction = VuSetRenderTargetParams::LoadActionClear;
			params.mDepthLoadAction = VuSetRenderTargetParams::LoadActionClear;
			VuGfx::IF()->setRenderTarget(params);
		}

		VuRenderTarget	*mpRenderTarget;
	};
	Clear *pData = static_cast<Clear *>(VuGfxSort::IF()->allocateCommandMemory(sizeof(Clear)));
	pData->mpRenderTarget = mpRenderTarget;
	VuGfxSort::IF()->submitCommand(VuGfxSort::TRANS_BEGIN, 0, Clear::callback);
}

//*****************************************************************************
void VuDropShadow::submitShadow(const VuMatrix &textureMatrix, const VuColor &color)
{
	struct Shadow
	{
		static void callback(void *data)
		{
			Shadow *pData = static_cast<Shadow *>(data);

			VuGfx::IF()->setTexture(0, pData->mpTexture);
			VuGfx::IF()->drawPrimitiveUP(VUGFX_PT_TRIANGLELIST, pData->mVertexCount/3, pData + 1);
		}

		int			mVertexCount;
		VuTexture	*mpTexture;
	};

	// submit draw command
	int vertCount = mVerts.size();
	int dataSize = sizeof(Shadow) + vertCount*sizeof(VuBlobShadowVertex);
	Shadow *pData = static_cast<Shadow *>(VuGfxSort::IF()->allocateCommandMemory(dataSize));
	pData->mVertexCount = vertCount;
	pData->mpTexture = mpRenderTarget->getColorTexture();

	VuVector3 *pRawVert = &mVerts.begin();
	VuBlobShadowVertex *pVert = reinterpret_cast<VuBlobShadowVertex *>(pData + 1);
	for ( int i = 0; i < vertCount; i++ )
	{
		VuVector3 shadowTexCoord = textureMatrix.transformCoord(*pRawVert);

		pVert->mXyz[0] = pRawVert->mX; pVert->mXyz[1] = pRawVert->mY; pVert->mXyz[2] = pRawVert->mZ + mParams.mOffsetZ;
		pVert->mUv[0] = shadowTexCoord.mX; pVert->mUv[1] = shadowTexCoord.mY;

		pVert->mColor = color;

		pRawVert++;
		pVert++;
	}

	VuGfxSort::IF()->submitDrawCommand<false>(VuGfxSort::TRANS_BLOB_SHADOW, mpGfxSortMaterial, VUNULL, Shadow::callback);
}

//*****************************************************************************
void VuDropShadow::calcLightMatrix(const VuVector3 &lightPos, const VuVector3 &lightDir, VuMatrix &lightMatrix)
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
void VuDropShadow::calcLightAabb(const VuMatrix &lightMatrix, VuAabb &lightAabb)
{
	float radius = mObjectRadius + mObjectRadius/mTextureSize;

	lightAabb.addPoint(lightMatrix.transform(mObjectPos));
	lightAabb.mMax += VuVector3(radius, radius, radius);
	lightAabb.mMin -= VuVector3(radius, radius, radius);
}

//*****************************************************************************
void VuDropShadow::calcLightCropMatrix(const VuAabb &lightAabb, const VuMatrix &lightMatrix, VuMatrix &lightCropMatrix)
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
		float fHalfTextureSize = 0.5f*mTextureSize;
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
void VuDropShadow::calcTextScaleBiasMatrix(VuMatrix &texScaleBiasMatrix)
{
	float fTexOffset = 0.5f + 0.5f/(float)mTextureSize;

	texScaleBiasMatrix = VuMatrix(
		VuVector4(      0.5f,          0,        0,       0),
		VuVector4(         0,	   -0.5f,        0,       0),
		VuVector4(         0,          0,        1,       0),
		VuVector4(fTexOffset, fTexOffset,        0,       1)
	);
}

//*****************************************************************************
VuDropShadow::VuParams::VuParams():
	mCollisionMask(COL_ENGINE_STATIC_PROP),
	mAlpha(0.5f),
	mDistanceFadeNear(25.0f),
	mDistanceFadeFar(50.0f),
	mHeightFadeNear(0.0f),
	mHeightFadeFar(5.0f),
	mOffsetZ(0.01f)
{
}