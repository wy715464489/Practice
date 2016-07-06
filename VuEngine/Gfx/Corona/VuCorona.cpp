//*****************************************************************************
//
//  Copyright (c) 2011-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  Corona
// 
//*****************************************************************************

#include "VuCorona.h"
#include "VuEngine/Assets/VuAssetFactory.h"
#include "VuEngine/Assets/VuTextureAsset.h"
#include "VuEngine/Assets/VuCollisionMeshAsset.h"
#include "VuEngine/Gfx/VuGfxUtil.h"
#include "VuEngine/Gfx/VuGfxDrawParams.h"
#include "VuEngine/Gfx/GfxSort/VuGfxSort.h"
#include "VuEngine/Gfx/Shaders/VuBasicShaders.h"
#include "VuEngine/HAL/Gfx/VuGfx.h"
#include "VuEngine/Managers/VuViewportManager.h"
#include "VuEngine/Dynamics/VuRigidBody.h"
#include "VuEngine/Dynamics/Util/VuDynamicsRayTest.h"


class VuCoronaRayTestResult : public VuDynamicsRayTest::VuClosestResult
{
public:
	VuCoronaRayTestResult(VUUINT32 collisionMask) : mCollisionMask(collisionMask) {}

	virtual bool	addResult(const VuRigidBody *pRigidBody, float hitFraction, int triangleIndex, const VuVector3 &normal)
	{
		if ( const VuCollisionMeshAsset *pAsset = static_cast<const VuCollisionMeshAsset *>(pRigidBody->getCollisionShape()->getUserPointer()) )
			if ( !(pAsset->getTriangleMaterial(triangleIndex).mFlags & VuCollisionMeshAsset::VuMaterial::IS_CORONA_COLLISION) )
				return false;

		return VuClosestResult::addResult(pRigidBody, hitFraction, triangleIndex, normal);
	}

	virtual bool needsCollision(VuRigidBody *pRigidBody)
	{
		if ( pRigidBody->getExtendedFlags() & EXT_COL_ENGINE_NOT_CORONA )
			return false;

		if ( pRigidBody->getCollisionGroup() & mCollisionMask )
			return true;

		return false;
	}

	VUUINT32	mCollisionMask;
};


//*****************************************************************************
VuCorona::VuCorona():
	mDrawDist(FLT_MAX),
	mFadeDist(0.0f),
	mQueryRadius(1.0f),
	mConeAngle(VU_2PI),
	mPenumbraAngle(0.0f),
	mbEnableBackLight(false),
	mTextureColor(255,255,255,255),
	mbTextureSizeScreenSpace(false),
	mTextureSize(10.0f),
	mRotationOffset(0),
	mRotationAmount(VU_PI),
	mCollisionMask((VUUINT32)~0),
	mpGfxSortMaterial(VUNULL)
{
}

//*****************************************************************************
VuCorona::~VuCorona()
{
	VuGfxSort::IF()->releaseMaterial(mpGfxSortMaterial);
}

//*****************************************************************************
void VuCorona::setTextureAsset(const std::string &assetName)
{
	VuGfxSort::IF()->releaseMaterial(mpGfxSortMaterial);

	// create gfx sort material
	VuPipelineState *pPS = VuGfxUtil::IF()->basicShaders()->get3dXyzUvMaterial(VuBasicShaders::FLV_ADDITIVE)->mpPipelineState;
	VuGfxSortMaterialDesc desc;
	if ( VuAssetFactory::IF()->doesAssetExist<VuTextureAsset>(assetName) )
		desc.addTexture("tex0", VuGfxSortMaterialDesc::TEXTURE, assetName.c_str());
	mpGfxSortMaterial = VuGfxSort::IF()->createMaterial(pPS, desc);
}

//*****************************************************************************
void VuCorona::updateVisibility(const VuVector3 &position)
{
	for ( int iViewport = 0; iViewport < VuViewportManager::IF()->getViewportCount(); iViewport++ )
	{
		VuQuery &query = mQueries[iViewport];
		query.mVisibility = 0.0f;
		if ( query.mbTestVisibility )
		{
			VuVector3 eye = VuViewportManager::IF()->getViewport(iViewport).mCamera.getEyePosition();
			VuVector3 target = position;
			VuVector3 dir = (target - eye).normal();
			target -= dir*mQueryRadius;

			VuCoronaRayTestResult result(mCollisionMask);
			VuDynamicsRayTest::test(eye, target, result);
			if ( !result.mbHasHit )
			{
				query.mVisibility = 1.0f;
			}
		}
		query.mbTestVisibility = false;
	}
}

//*****************************************************************************
void VuCorona::draw(const VuMatrix &transform, const VuGfxDrawParams &params)
{
	const VuVector3 &position = transform.getTrans();
	const VuVector3 &direction = transform.getAxisY();

	float distSquared = (position - params.mEyePos).magSquared();
	if ( distSquared > mDrawDist*mDrawDist )
		return;

	// determine angular visibility
	float minAngle = VuMin(0.5f*mConeAngle, 0.5f*mConeAngle + mPenumbraAngle);
	float maxAngle = VuMax(0.5f*mConeAngle, 0.5f*mConeAngle + mPenumbraAngle);
	minAngle = VuClamp(minAngle, 0.0f, VU_PI - FLT_EPSILON);
	maxAngle = VuClamp(maxAngle, 0.0f, VU_PI);

	VuVector3 viewDir = (params.mEyePos - position);
	float dist = viewDir.normalize();
	float cosAngle = VuDot(viewDir, direction);
	if ( mbEnableBackLight )
		cosAngle = VuAbs(cosAngle);
	float angle = VuACos(VuClamp(cosAngle, -1.0f, 1.0f));
	if( angle < maxAngle )
	{
		float angularVisibility = 1.0f;
		if ( angle > minAngle )
			angularVisibility = (maxAngle - angle)/(maxAngle - minAngle);

		// determine color
		VuColor color = mTextureColor;
		color.mA = (VUUINT8)VuRound(angularVisibility*color.mA);
		if ( color.mA )
		{
			// query visibility
			VuQuery &query = mQueries[VuGfxSort::IF()->getViewport()];
			query.mbTestVisibility = true;
			if ( query.mVisibility > 0 )
			{
				// determine size
				float size = mTextureSize;
				if ( mbTextureSizeScreenSpace )
				{
					size *= 0.01f; // %
					size *= 2.0f*dist/params.mCamera.getProjMatrix().getAxisY().mY;
					size *= params.mCamera.getScreenShotScale();
				}

				// fade in
				float dist = VuSqrt(distSquared);
				if ( dist > mDrawDist - mFadeDist )
					color.mA = (VUUINT8)VuRound(color.mA*(mDrawDist - dist)/mFadeDist);

				// submit draw command
				DrawCallbackData *pData = static_cast<DrawCallbackData *>(VuGfxSort::IF()->allocateCommandMemory(sizeof(DrawCallbackData)));

				pData->mPosition = position;
				pData->mColor = color;
				pData->mSize = size;
				pData->mRotationOffset = mRotationOffset;
				pData->mRotationAmount = mRotationAmount;

				VuGfxSort::IF()->submitDrawCommand<false>(VuGfxSort::TRANS_UI_ADDITIVE, mpGfxSortMaterial, VUNULL, drawCallback);
			}
		}
	}
}

//*****************************************************************************
void VuCorona::drawCallback(void *data)
{
	DrawCallbackData *pData = static_cast<DrawCallbackData *>(data);

	// determine color
	VuColor color = pData->mColor;

	const VuCamera &camera = VuGfxSort::IF()->getRenderCamera();

	// determine screen-space x coordinate
	VuVector3 screenPos = camera.worldToScreen(pData->mPosition);
	float screenX = 0.5f - screenPos.mX;

	// determine matrix
	VuMatrix mat = camera.getTransform();
	mat.setTrans(pData->mPosition);
	mat.scaleLocal(VuVector3(pData->mSize, 1.0f, pData->mSize));
	mat.rotateYLocal(pData->mRotationOffset + screenX*pData->mRotationAmount);

	// set constants
	VuGfxUtil::IF()->basicShaders()->set3dXyzUvConstants(mat*camera.getViewProjMatrix(), color);

	// build verts
	VuVertex3dXyzUv verts[4];
	verts[0].mXyz[0] = -0.5f; verts[0].mXyz[1] = 0.0f; verts[0].mXyz[2] = -0.5f; verts[0].mUv[0] = 0.0f; verts[0].mUv[1] = 1.0f;
	verts[1].mXyz[0] =  0.5f; verts[1].mXyz[1] = 0.0f; verts[1].mXyz[2] = -0.5f; verts[1].mUv[0] = 1.0f; verts[1].mUv[1] = 1.0f;
	verts[2].mXyz[0] = -0.5f; verts[2].mXyz[1] = 0.0f; verts[2].mXyz[2] =  0.5f; verts[2].mUv[0] = 0.0f; verts[2].mUv[1] = 0.0f;
	verts[3].mXyz[0] =  0.5f; verts[3].mXyz[1] = 0.0f; verts[3].mXyz[2] =  0.5f; verts[3].mUv[0] = 1.0f; verts[3].mUv[1] = 0.0f;

	// draw
	VuGfx::IF()->drawPrimitiveUP(VUGFX_PT_TRIANGLESTRIP, 2, verts);
}
