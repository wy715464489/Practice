//*****************************************************************************
//
//  Copyright (c) 2012-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  Blob Shadow
// 
//*****************************************************************************

#include "VuBlobShadow.h"
#include "VuEngine/Assets/VuAssetFactory.h"
#include "VuEngine/Assets/VuTextureAsset.h"
#include "VuEngine/Assets/VuCollisionMeshAsset.h"
#include "VuEngine/Gfx/VuGfxUtil.h"
#include "VuEngine/Gfx/GfxSort/VuGfxSort.h"
#include "VuEngine/Gfx/Shaders/VuBlobShadowShader.h"
#include "VuEngine/Gfx/VuGfxDrawParams.h"
#include "VuEngine/HAL/Gfx/VuGfx.h"
#include "VuEngine/Dynamics/VuDynamics.h"


//*****************************************************************************
class VuBlobShadowConvexResult : public btCollisionWorld::ConvexResultCallback
{
public:
	typedef VuArray<VuVector3> Verts;

	VuBlobShadowConvexResult(Verts &verts) : mVerts(verts), mHitFraction(1.0f)
	{
		mVerts.resize(0);
	}

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

			if ( VuCross(v1 - v0, v2 - v0).mZ > 0.0f )
			{
				mVerts.push_back(transform.transform(v0));
				mVerts.push_back(transform.transform(v1));
				mVerts.push_back(transform.transform(v2));

				mHitFraction = VuMin(mHitFraction, convexResult.m_hitFraction);
			}
		}

		return 1.0f;
	}

	Verts	&mVerts;
	float	mHitFraction;
};

//*****************************************************************************
VuBlobShadow::VuBlobShadow():
	mpGfxSortMaterial(VUNULL),
	mVerts(3*8),
	mHeightFade(0.0f)
{
}

//*****************************************************************************
VuBlobShadow::~VuBlobShadow()
{
	reset();
}

//*****************************************************************************
void VuBlobShadow::reset()
{
	VuGfxSort::IF()->releaseMaterial(mpGfxSortMaterial);
	mpGfxSortMaterial = VUNULL;
}

//*****************************************************************************
void VuBlobShadow::setTexture(const std::string &assetName)
{
	if ( mpGfxSortMaterial && mpGfxSortMaterial->mpTextureAssets[0]->getAssetName() == assetName )
		return;

	reset();

	if ( VuAssetFactory::IF()->doesAssetExist<VuTextureAsset>(assetName) )
	{
		VuBlobShadowShader *pBlobShadowShader = VuGfxUtil::IF()->blobShadowShader();

		VuGfxSortMaterialDesc desc;
		desc.addTexture("tex0", VuGfxSortMaterialDesc::TEXTURE, assetName.c_str());
		mpGfxSortMaterial = VuGfxSort::IF()->createMaterial(pBlobShadowShader->getPipelineState(), desc);
	}
}

//*****************************************************************************
void VuBlobShadow::calculate(const VuMatrix &transform, const VuAabb &aabb)
{
	mTransform = transform;
	mAabb = aabb;
	mAabb.mMin *= mParams.mScale;
	mAabb.mMax *= mParams.mScale;
	mTransform.translateLocal(aabb.getCenter());

	float radius = aabb.getExtents().mag();

	btSphereShape sphereShape(radius);

	btTransform from = VuDynamicsUtil::toBtTransform(mTransform);
	btTransform to = from;
	from.getOrigin().setZ(mTransform.getTrans().mZ + radius);
	to.getOrigin().setZ(mTransform.getTrans().mZ - mParams.mHeightFadeFar);

	VuBlobShadowConvexResult result(mVerts);
	result.m_collisionFilterMask = (short)mParams.mCollisionMask;

	VuDynamics::IF()->getDynamicsWorld()->convexSweepTest(&sphereShape, from, to, result);

	mHeightFade = 1.0f - VuLinStep(mParams.mHeightFadeNear + radius, mParams.mHeightFadeFar + radius, result.mHitFraction*(mParams.mHeightFadeFar + radius));
}

//*****************************************************************************
void VuBlobShadow::draw(const VuGfxDrawParams &params)
{
	struct DrawData
	{
		static void callback(void *data)
		{
			DrawData *pData = static_cast<DrawData *>(data);

			VuGfx::IF()->drawPrimitiveUP(VUGFX_PT_TRIANGLELIST, pData->mVertexCount/3, pData + 1);
		}

		int			mVertexCount;
	};

	if ( mpGfxSortMaterial == VUNULL )
		return;

	int vertCount = mVerts.size();
	if ( vertCount == 0 )
		return;

	float distFade = 1.0f - VuLinStep(mParams.mDistanceFadeNear, mParams.mDistanceFadeFar, VuDist(mTransform.getTrans(), params.mEyePos));

	VuColor color(0,0,0);
	color.mA = (VUUINT8)VuRound(255*mParams.mAlpha*mHeightFade*distFade);

	if ( color.mA == 0 )
		return;

	int axisX, axisY;
	if ( mTransform.getAxisY().mag2dSquared() > mTransform.getAxisZ().mag2dSquared() )
	{
		axisY = 1;
		if ( mTransform.getAxisX().mag2dSquared() > mTransform.getAxisZ().mag2dSquared() )
			axisX = 0;
		else
			axisX = 2;
	}
	else
	{
		axisY = 2;
		if ( mTransform.getAxisX().mag2dSquared() > mTransform.getAxisY().mag2dSquared() )
			axisX = 0;
		else
			axisX = 1;
	}

	VuVector2 texAxisX(mTransform.getAxis(axisX).mX, mTransform.getAxis(axisX).mY);
	VuVector2 texAxisY(mTransform.getAxis(axisY).mX, mTransform.getAxis(axisY).mY);
	VuVector2 texSize(mAabb.getExtents().getValue(axisX), mAabb.getExtents().getValue(axisY));
	VuVector2 texCenter(mTransform.getTrans().mX, mTransform.getTrans().mY);

	// submit draw command
	int dataSize = sizeof(DrawData) + vertCount*sizeof(VuBlobShadowVertex);
	DrawData *pData = static_cast<DrawData *>(VuGfxSort::IF()->allocateCommandMemory(dataSize));
	pData->mVertexCount = vertCount;

	VuVector3 *pRawVert = &mVerts.begin();
	VuBlobShadowVertex *pVert = reinterpret_cast<VuBlobShadowVertex *>(pData + 1);
	for ( int i = 0; i < vertCount; i++ )
	{
		VuVector2 texDelta = VuVector2(pRawVert->mX, pRawVert->mY) - texCenter;
		float texCoordX = VuDot(texAxisX, texDelta)/texSize.mX;
		float texCoordY = VuDot(texAxisY, texDelta)/texSize.mY;
		texCoordX = 0.5f + texCoordX*0.5f;
		texCoordY = 0.5f + texCoordY*0.5f;

		pVert->mXyz[0] = pRawVert->mX; pVert->mXyz[1] = pRawVert->mY; pVert->mXyz[2] = pRawVert->mZ + mParams.mOffsetZ;
		pVert->mUv[0] = texCoordX; pVert->mUv[1] = texCoordY;
		pVert->mColor = color;

		pRawVert++;
		pVert++;
	}

	VuGfxSort::IF()->submitDrawCommand<false>(VuGfxSort::TRANS_BLOB_SHADOW, mpGfxSortMaterial, VUNULL, DrawData::callback);
}

//*****************************************************************************
VuBlobShadow::VuParams::VuParams():
	mCollisionMask(COL_ENGINE_STATIC_PROP),
	mAlpha(0.75f),
	mScale(1.5f),
	mDistanceFadeNear(25.0f),
	mDistanceFadeFar(50.0f),
	mHeightFadeNear(0.0f),
	mHeightFadeFar(3.0f),
	mOffsetZ(0.01f)
{
}