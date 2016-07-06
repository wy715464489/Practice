//*****************************************************************************
//
//  Copyright (c) 2011-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  Directional Corona entity
// 
//*****************************************************************************

#include "VuEngine/Entities/VuEntity.h"
#include "VuEngine/Components/Transform/VuTransformComponent.h"
#include "VuEngine/Components/3dDraw/Vu3dDrawComponent.h"
#include "VuEngine/Components/3dLayout/Vu3dLayoutComponent.h"
#include "VuEngine/Components/Script/VuScriptComponent.h"
#include "VuEngine/Components/Motion/VuMotionComponent.h"
#include "VuEngine/Properties/VuBasicProperty.h"
#include "VuEngine/Properties/VuAngleProperty.h"
#include "VuEngine/Properties/VuAssetProperty.h"
#include "VuEngine/Assets/VuTextureAsset.h"
#include "VuEngine/Assets/VuCollisionMeshAsset.h"
#include "VuEngine/Gfx/VuGfxUtil.h"
#include "VuEngine/Gfx/Camera/VuCamera.h"
#include "VuEngine/Gfx/GfxSort/VuGfxSort.h"
#include "VuEngine/Gfx/Shaders/VuBasicShaders.h"
#include "VuEngine/HAL/Gfx/VuGfx.h"
#include "VuEngine/Managers/VuTickManager.h"
#include "VuEngine/Managers/VuViewportManager.h"
#include "VuEngine/Dynamics/VuRigidBody.h"
#include "VuEngine/Dynamics/Util/VuDynamicsRayTest.h"


// constants
#define QUERY_RADIUS 1.0f


class VuDirectionalCoronaEntity : public VuEntity, public VuMotionComponentIF
{
	DECLARE_RTTI

public:
	VuDirectionalCoronaEntity();

	virtual void		onGameInitialize();
	virtual void		onGameRelease();

private:
	// scripting
	VuRetVal			Show(const VuParams &params)	{ show(); return VuRetVal(); }
	VuRetVal			Hide(const VuParams &params)	{ hide(); return VuRetVal(); }

	void				tickCorona(float fdt);

	void				show();
	void				hide();

	void				imageModified();
	void				drawLayout(const Vu3dLayoutDrawParams &params);
	void				draw(const VuGfxDrawParams &params);

	// VuMotionComponentIF interface
	virtual void		onMotionUpdate();

	// components
	Vu3dLayoutComponent		*mp3dLayoutComponent;
	Vu3dDrawComponent		*mp3dDrawComponent;
	VuScriptComponent		*mpScriptComponent;
	VuMotionComponent		*mpMotionComponent;

	// properties
	bool				mbInitiallyVisible;
	std::string			mTextureAssetName;
	VuColor				mTextureColor;
	float				mTextureSize;
	float				mRotationOffset;
	float				mRotationAmount;

	bool				mbVisible;
	VuGfxSortMaterial	*mpGfxSortMaterial;

	class VuQuery
	{
	public:
		VuQuery() : mbTestVisibility(false), mVisibility(0.0f) {}
		bool	mbTestVisibility;
		float	mVisibility;
	};
	VuQuery				mQueries[VuViewportManager::MAX_VIEWPORTS];

	struct DrawCallbackData
	{
		VuVector3	mPosition;
		VuColor		mColor;
		float		mSize;
		float		mRotationOffset;
		float		mRotationAmount;
	};
	static void			drawCallback(void *data);
};


IMPLEMENT_RTTI(VuDirectionalCoronaEntity, VuEntity);
IMPLEMENT_ENTITY_REGISTRATION(VuDirectionalCoronaEntity);

class VuDirectionalCoronaRayTestResult : public VuDynamicsRayTest::VuClosestResult
{
public:
	virtual bool	needsCollision(VuRigidBody *pRigidBody)
	{
		if ( pRigidBody->getExtendedFlags() & EXT_COL_ENGINE_NOT_CORONA )
			return false;

		return true;
	}

	virtual bool	addResult(const VuRigidBody *pRigidBody, float hitFraction, int triangleIndex, const VuVector3 &normal)
	{
		if ( const VuCollisionMeshAsset *pAsset = static_cast<const VuCollisionMeshAsset *>(pRigidBody->getCollisionShape()->getUserPointer()) )
			if ( !(pAsset->getTriangleMaterial(triangleIndex).mFlags & VuCollisionMeshAsset::VuMaterial::IS_CORONA_COLLISION) )
				return false;

		return VuClosestResult::addResult(pRigidBody, hitFraction, triangleIndex, normal);
	}
};


//*****************************************************************************
VuDirectionalCoronaEntity::VuDirectionalCoronaEntity():
	mbInitiallyVisible(true),
	mTextureColor(255,255,255,255),
	mTextureSize(10.0f),
	mRotationOffset(0),
	mRotationAmount(VU_PI),
	mbVisible(false),
	mpGfxSortMaterial(VUNULL)
{
	// properties
	addProperty(new VuBoolProperty("Initially Visible", mbInitiallyVisible));
	addProperty(new VuAssetNameProperty(VuTextureAsset::msRTTI.mstrType, "Texture Name", mTextureAssetName));
	addProperty(new VuFloatProperty("Texture Size", mTextureSize));
	addProperty(new VuColorProperty("Texture Color", mTextureColor));
	addProperty(new VuAngleProperty("Rotation Offset", mRotationOffset));
	addProperty(new VuAngleProperty("Rotation Amount", mRotationAmount));

	// components
	addComponent(mp3dLayoutComponent = new Vu3dLayoutComponent(this));
	addComponent(mp3dDrawComponent = new Vu3dDrawComponent(this));
	addComponent(mpScriptComponent = new VuScriptComponent(this, 100, false));
	addComponent(mpMotionComponent = new VuMotionComponent(this, this));

	mpTransformComponent->setMask(VuTransformComponent::TRANS|VuTransformComponent::ROT);
	mp3dDrawComponent->setDrawMethod(this, &VuDirectionalCoronaEntity::draw);
	mp3dDrawComponent->updateVisibility(VuAabb(VuVector3(-1e9, -1e9, -1e9), VuVector3(1e9, 1e9, 1e9)));
	mp3dLayoutComponent->setDrawMethod(this, &VuDirectionalCoronaEntity::drawLayout);
	mp3dLayoutComponent->setLocalBounds(VuAabb(VuVector3(-0.5f), VuVector3(0.5f)));

	// scripting
	ADD_SCRIPT_INPUT_NOARGS(mpScriptComponent, VuDirectionalCoronaEntity, Show);
	ADD_SCRIPT_INPUT_NOARGS(mpScriptComponent, VuDirectionalCoronaEntity, Hide);
}

//*****************************************************************************
void VuDirectionalCoronaEntity::onGameInitialize()
{
	// create gfx sort material
	VuPipelineState *pPS = VuGfxUtil::IF()->basicShaders()->get3dXyzUvMaterial(VuBasicShaders::FLV_ADDITIVE)->mpPipelineState;
	VuGfxSortMaterialDesc desc;
	desc.addTexture("tex0", VuGfxSortMaterialDesc::TEXTURE, mTextureAssetName.c_str());
	mpGfxSortMaterial = VuGfxSort::IF()->createMaterial(pPS, desc);

	if ( mbInitiallyVisible )
		show();

	VuTickManager::IF()->registerHandler(this, &VuDirectionalCoronaEntity::tickCorona, "Corona");
}

//*****************************************************************************
void VuDirectionalCoronaEntity::onGameRelease()
{
	hide();
	VuGfxSort::IF()->releaseMaterial(mpGfxSortMaterial);

	VuTickManager::IF()->unregisterHandlers(this);
}

//*****************************************************************************
// As a cpu optimization, these tests can be performed asynchronously
// by the dynamics thread, if another frame of lag is acceptable.
//*****************************************************************************
void VuDirectionalCoronaEntity::tickCorona(float fdt)
{
	for ( int iViewport = 0; iViewport < VuViewportManager::IF()->getViewportCount(); iViewport++ )
	{
		VuQuery &query = mQueries[iViewport];
		query.mVisibility = 0.0f;
		if ( query.mbTestVisibility )
		{
			const VuCamera &camera = VuViewportManager::IF()->getViewport(iViewport).mCamera;
			VuVector3 eye = camera.getEyePosition();
			VuVector3 dir = -mpTransformComponent->getWorldTransform().getAxisY();
			VuVector3 target = eye + dir*(camera.getFarPlane() - QUERY_RADIUS);

			VuDirectionalCoronaRayTestResult result;
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
void VuDirectionalCoronaEntity::show()
{
	if ( !mbVisible )
	{
		mbVisible = true;
		mp3dDrawComponent->show();
	}
}

//*****************************************************************************
void VuDirectionalCoronaEntity::hide()
{
	if ( mbVisible )
	{
		mbVisible = false;
		mp3dDrawComponent->hide();
	}
}

//*****************************************************************************
void VuDirectionalCoronaEntity::drawLayout(const Vu3dLayoutDrawParams &params)
{
	// draw center arrow
	{
		VuMatrix mat = mpTransformComponent->getWorldTransform();
		VuGfxUtil::IF()->drawArrowLines(mTextureColor, 4.0f, 1.0f, 1.0f, mat*params.mCamera.getViewProjMatrix());
		mat.rotateYLocal(VuDegreesToRadians(90.0f));
		VuGfxUtil::IF()->drawArrowLines(mTextureColor, 4.0f, 1.0f, 1.0f, mat*params.mCamera.getViewProjMatrix());
	}
}

//*****************************************************************************
void VuDirectionalCoronaEntity::draw(const VuGfxDrawParams &params)
{
	// query visibility
	VuQuery &query = mQueries[VuGfxSort::IF()->getViewport()];
	query.mbTestVisibility = true;
	if ( query.mVisibility > 0 )
	{
		const VuCamera &camera = params.mCamera;
		VuVector3 dir = -mpTransformComponent->getWorldTransform().getAxisY();
		float dist = camera.getFarPlane() - QUERY_RADIUS;
		VuVector3 pos = camera.getEyePosition() + dir*dist;

		// determine size
		float size = mTextureSize*0.01f*2.0f*dist/params.mCamera.getProjMatrix().getAxisY().mY;
		size *= params.mCamera.getScreenShotScale();

		// submit draw command
		DrawCallbackData *pData = static_cast<DrawCallbackData *>(VuGfxSort::IF()->allocateCommandMemory(sizeof(DrawCallbackData)));

		pData->mPosition = pos;
		pData->mColor = mTextureColor;
		pData->mSize = size;
		pData->mRotationOffset = mRotationOffset;
		pData->mRotationAmount = mRotationAmount;

		VuGfxSort::IF()->submitDrawCommand<false>(VuGfxSort::TRANS_UI_ADDITIVE, mpGfxSortMaterial, VUNULL, drawCallback);
	}
}

//*****************************************************************************
void VuDirectionalCoronaEntity::onMotionUpdate()
{
	mpTransformComponent->setWorldTransform(mpMotionComponent->getWorldTransform());
}

//*****************************************************************************
void VuDirectionalCoronaEntity::drawCallback(void *data)
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
