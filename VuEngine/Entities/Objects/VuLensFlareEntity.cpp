//*****************************************************************************
//
//  Copyright (c) 2014-2014 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  Lens Flare entity
// 
//*****************************************************************************

#include "VuEngine/Entities/VuEntity.h"
#include "VuEngine/Components/Transform/VuTransformComponent.h"
#include "VuEngine/Components/3dDraw/Vu3dDrawComponent.h"
#include "VuEngine/Components/3dLayout/Vu3dLayoutComponent.h"
#include "VuEngine/Components/Script/VuScriptComponent.h"
#include "VuEngine/Components/Motion/VuMotionComponent.h"
#include "VuEngine/Properties/VuBasicProperty.h"
#include "VuEngine/Properties/VuDBEntryProperty.h"
#include "VuEngine/Assets/VuAssetFactory.h"
#include "VuEngine/Assets/VuCollisionMeshAsset.h"
#include "VuEngine/Assets/VuTextureAsset.h"
#include "VuEngine/Managers/VuTickManager.h"
#include "VuEngine/Managers/VuViewportManager.h"
#include "VuEngine/Dynamics/VuRigidBody.h"
#include "VuEngine/Dynamics/Util/VuDynamicsRayTest.h"
#include "VuEngine/Gfx/VuGfxUtil.h"
#include "VuEngine/Gfx/Shaders/VuBasicShaders.h"
#include "VuEngine/HAL/Gfx/VuGfx.h"


// constants
#define QUERY_RADIUS 1.0f


class VuLensFlareEntity : public VuEntity, public VuMotionComponentIF
{
	DECLARE_RTTI

public:
	VuLensFlareEntity();

	virtual void	onGameInitialize();
	virtual void	onGameRelease();

private:
	// scripting
	VuRetVal			Show(const VuParams &params)	{ show(); return VuRetVal(); }
	VuRetVal			Hide(const VuParams &params)	{ hide(); return VuRetVal(); }

	void				tickCorona(float fdt);

	void				show();
	void				hide();

	void				drawLayout(const Vu3dLayoutDrawParams &params);
	void				draw(const VuGfxDrawParams &params);

	// VuMotionComponentIF interface
	virtual void		onMotionUpdate();

	// components
	Vu3dLayoutComponent	*mp3dLayoutComponent;
	Vu3dDrawComponent	*mp3dDrawComponent;
	VuScriptComponent	*mpScriptComponent;
	VuMotionComponent	*mpMotionComponent;

	// properties
	bool				mbInitiallyVisible;
	std::string			mLensFlareName;

	bool				mbVisible;
	VuDBEntryProperty	*mpDBEntry;
	VuGfxSortMaterial	*mpModulatedMaterial;
	VuGfxSortMaterial	*mpAdditiveMaterial;

	// parameters
	struct Element
	{
		Element() : mDistance(0), mSize(0.1f), mRotationAmount(180), mRotationOffset(0), mColor(255,255,255), mOffset(0,0), mTexCoords(0,0,1,1) {}

		float		mDistance;
		float		mSize;
		float		mRotationAmount;
		float		mRotationOffset;
		VuColor		mColor;
		VuVector2	mOffset;
		VuRect		mTexCoords;
	};
	typedef std::vector<Element> Elements;

	struct Params
	{
		Params() : mFadeStartAngle(80.0f), mFadeEndAngle(90.0f), mOcclusionFadeTime(0.25f), mAdditive(true) {}

		float		mFadeStartAngle;
		float		mFadeEndAngle;
		float		mOcclusionFadeTime;
		bool		mAdditive;

		float		mCosFadeStartAngle;
		float		mCosFadeEndAngle;

		Elements	mElements;
	};
	Params		mParams;

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
		VuLensFlareEntity	*mpEntity;
		VuVector3			mDirection;
		float				mVisibility;
	};
	static void			staticDrawCallback(void *data) { static_cast<DrawCallbackData *>(data)->mpEntity->drawCallback(data); }
	void				drawCallback(void *data);
};


IMPLEMENT_RTTI(VuLensFlareEntity, VuEntity);
IMPLEMENT_ENTITY_REGISTRATION(VuLensFlareEntity);


class VuLensFlareRayTestResult : public VuDynamicsRayTest::VuClosestResult
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
VuLensFlareEntity::VuLensFlareEntity():
	mbInitiallyVisible(true),
	mbVisible(false),
	mpAdditiveMaterial(VUNULL),
	mpModulatedMaterial(VUNULL)
{
	// properties
	addProperty(new VuBoolProperty("Initially Visible", mbInitiallyVisible));
	addProperty(mpDBEntry = new VuDBEntryProperty("Type", mLensFlareName, "LensFlareDB"));

	// components
	addComponent(mp3dLayoutComponent = new Vu3dLayoutComponent(this));
	addComponent(mp3dDrawComponent = new Vu3dDrawComponent(this));
	addComponent(mpScriptComponent = new VuScriptComponent(this, 100, false));
	addComponent(mpMotionComponent = new VuMotionComponent(this, this));

	mpTransformComponent->setMask(VuTransformComponent::TRANS|VuTransformComponent::ROT);
	mp3dDrawComponent->setDrawMethod(this, &VuLensFlareEntity::draw);
	mp3dDrawComponent->updateVisibility(VuAabb(VuVector3(-1e9, -1e9, -1e9), VuVector3(1e9, 1e9, 1e9)));
	mp3dLayoutComponent->setDrawMethod(this, &VuLensFlareEntity::drawLayout);
	mp3dLayoutComponent->setLocalBounds(VuAabb(VuVector3(-0.5f), VuVector3(0.5f)));

	// scripting
	ADD_SCRIPT_INPUT_NOARGS(mpScriptComponent, VuLensFlareEntity, Show);
	ADD_SCRIPT_INPUT_NOARGS(mpScriptComponent, VuLensFlareEntity, Hide);
}

//*****************************************************************************
void VuLensFlareEntity::onGameInitialize()
{
	if ( mbInitiallyVisible )
		show();

	// reset
	mParams = Params();

	const VuJsonContainer &data = mpDBEntry->getEntryData();
	const std::string &textureAsset = data["Texture"].asString();
	if ( VuAssetFactory::IF()->doesAssetExist<VuTextureAsset>(textureAsset) )
	{
		VuGfxSortMaterialDesc desc;
		desc.addTexture("tex0", VuGfxSortMaterialDesc::TEXTURE, textureAsset.c_str());

		// create modulated material
		{
			VuPipelineState *pPS = VuGfxUtil::IF()->basicShaders()->get3dXyzUvMaterial(VuBasicShaders::FLV_MODULATED)->mpPipelineState;
			mpModulatedMaterial = VuGfxSort::IF()->createMaterial(pPS, desc);
		}

		// create additive material
		{
			VuPipelineState *pPS = VuGfxUtil::IF()->basicShaders()->get3dXyzUvMaterial(VuBasicShaders::FLV_ADDITIVE)->mpPipelineState;
			mpAdditiveMaterial = VuGfxSort::IF()->createMaterial(pPS, desc);
		}

		// params
		VuDataUtil::getValue(data["Fade Start Angle"], mParams.mFadeStartAngle);
		VuDataUtil::getValue(data["Fade End Angle"], mParams.mFadeEndAngle);
		VuDataUtil::getValue(data["Occlusion Fade Time"], mParams.mOcclusionFadeTime);
		VuDataUtil::getValue(data["Additive"], mParams.mAdditive);

		mParams.mCosFadeStartAngle = VuCos(VuDegreesToRadians(VuMin(mParams.mFadeStartAngle, 90.0f)));
		mParams.mCosFadeEndAngle = VuCos(VuDegreesToRadians(VuMin(mParams.mFadeEndAngle, 90.0f)));

		const VuJsonContainer &elementsData = data["Elements"];
		for ( int i = 0; i < elementsData.size(); i++ )
		{
			const VuJsonContainer &elementData = elementsData[i];

			Element element;
			VuDataUtil::getValue(elementData["Distance"], element.mDistance);
			VuDataUtil::getValue(elementData["Size"], element.mSize);
			VuDataUtil::getValue(elementData["Rotation Amount"], element.mRotationAmount);
			VuDataUtil::getValue(elementData["Rotation Offset"], element.mRotationOffset);
			VuDataUtil::getValue(elementData["Color"], element.mColor);
			VuDataUtil::getValue(elementData["Offset"], element.mOffset);
			VuDataUtil::getValue(elementData["Tex Coords"], element.mTexCoords);

			element.mRotationAmount = VuDegreesToRadians(element.mRotationAmount);
			element.mRotationOffset = VuDegreesToRadians(element.mRotationOffset);

			mParams.mElements.push_back(element);
		}
	}

	VuTickManager::IF()->registerHandler(this, &VuLensFlareEntity::tickCorona, "Corona");
}

//*****************************************************************************
void VuLensFlareEntity::onGameRelease()
{
	hide();
	VuGfxSort::IF()->releaseMaterial(mpModulatedMaterial);
	VuGfxSort::IF()->releaseMaterial(mpAdditiveMaterial);

	VuTickManager::IF()->unregisterHandlers(this);
}

//*****************************************************************************
// As a cpu optimization, these tests can be performed asynchronously
// by the dynamics thread, if another frame of lag is acceptable.
//*****************************************************************************
void VuLensFlareEntity::tickCorona(float fdt)
{
	float realDeltaTime = VuTickManager::IF()->getRealDeltaTime();

	for ( int iViewport = 0; iViewport < VuViewportManager::IF()->getViewportCount(); iViewport++ )
	{
		VuQuery &query = mQueries[iViewport];
		if ( query.mbTestVisibility )
		{
			const VuCamera &camera = VuViewportManager::IF()->getViewport(iViewport).mCamera;
			VuVector3 eye = camera.getEyePosition();
			VuVector3 dir = mpTransformComponent->getWorldTransform().getAxisZ();
			VuVector3 target = eye + dir*(camera.getFarPlane() - QUERY_RADIUS);

			VuLensFlareRayTestResult result;
			VuDynamicsRayTest::test(eye, target, result);

			if ( result.mbHasHit )
				query.mVisibility = VuMax(query.mVisibility - realDeltaTime/mParams.mOcclusionFadeTime, 0.0f);
			else
				query.mVisibility = VuMin(query.mVisibility + realDeltaTime/mParams.mOcclusionFadeTime, 1.0f);
		}
		query.mbTestVisibility = false;
	}
}

//*****************************************************************************
void VuLensFlareEntity::show()
{
	if ( !mbVisible )
	{
		mbVisible = true;
		mp3dDrawComponent->show();
	}
}

//*****************************************************************************
void VuLensFlareEntity::hide()
{
	if ( mbVisible )
	{
		mbVisible = false;
		mp3dDrawComponent->hide();
	}
}

//*****************************************************************************
void VuLensFlareEntity::drawLayout(const Vu3dLayoutDrawParams &params)
{
	// draw center arrow
	{
		VuMatrix mat = mpTransformComponent->getWorldTransform();
		mat.rotateXLocal(VuDegreesToRadians(-90.0f));
		VuGfxUtil::IF()->drawArrowLines(VuColor(255,255,255), 4.0f, 1.0f, 1.0f, mat*params.mCamera.getViewProjMatrix());
		mat.rotateYLocal(VuDegreesToRadians(90.0f));
		VuGfxUtil::IF()->drawArrowLines(VuColor(255,255,255), 4.0f, 1.0f, 1.0f, mat*params.mCamera.getViewProjMatrix());
	}
}

//*****************************************************************************
void VuLensFlareEntity::draw(const VuGfxDrawParams &params)
{
	if ( mpModulatedMaterial == VUNULL )
		return;

	// query visibility
	VuQuery &query = mQueries[VuGfxSort::IF()->getViewport()];
	query.mbTestVisibility = true;
	if ( query.mVisibility > 0 )
	{
//		const VuCamera &camera = params.mCamera;
		VuVector3 dir = mpTransformComponent->getWorldTransform().getAxisZ();
		if ( VuDot(params.mCamera.getTransform().getAxisY(), dir) > mParams.mCosFadeEndAngle )
		{
			// submit draw command
			DrawCallbackData *pData = static_cast<DrawCallbackData *>(VuGfxSort::IF()->allocateCommandMemory(sizeof(DrawCallbackData)));

			pData->mpEntity = this;
			pData->mDirection = dir;
			pData->mVisibility = query.mVisibility;

			if ( mParams.mAdditive )
				VuGfxSort::IF()->submitDrawCommand<false>(VuGfxSort::TRANS_UI_ADDITIVE, mpAdditiveMaterial, VUNULL, staticDrawCallback);
			else
				VuGfxSort::IF()->submitDrawCommand<true>(VuGfxSort::TRANS_UI_MODULATE, mpModulatedMaterial, VUNULL, staticDrawCallback, 0.0f);
		}
	}
}

//*****************************************************************************
void VuLensFlareEntity::onMotionUpdate()
{
	mpTransformComponent->setWorldTransform(mpMotionComponent->getWorldTransform());
}

//*****************************************************************************
void VuLensFlareEntity::drawCallback(void *data)
{
	DrawCallbackData *pData = static_cast<DrawCallbackData *>(data);

	// calculate light pos and screen center pos
	const VuCamera &camera = VuGfxSort::IF()->getRenderCamera();
	float dist = camera.getFarPlane() - QUERY_RADIUS;
	VuVector3 pos0 = camera.getEyePosition() + pData->mDirection*dist;
	VuVector3 pos1 = camera.getEyePosition() + camera.getTransform().getAxisY()*dist;

	// determine screen-space x coordinate
	VuVector3 screenPos0 = camera.worldToScreen(pos0);
	VuVector3 screenPos1 = camera.worldToScreen(pos1);

	// determine fading
	float alpha = VuLinStep(mParams.mCosFadeEndAngle, mParams.mCosFadeStartAngle, VuDot(camera.getTransform().getAxisY(), pData->mDirection));
	alpha *= pData->mVisibility;

	// draw elements
	for ( Elements::const_iterator iter = mParams.mElements.begin(); iter != mParams.mElements.end(); iter++ )
	{
		// determine position
		VuVector3 screenPos = VuLerp(screenPos0, screenPos1, iter->mDistance);
		screenPos.mX += iter->mOffset.mX;
		screenPos.mY += iter->mOffset.mY;
		VuVector3 pos = camera.screenToWorld(screenPos);
		pos = camera.getEyePosition() + dist*(pos - camera.getEyePosition()).normal();

		// determine size
		float size = iter->mSize*2.0f*dist/camera.getProjMatrix().getAxisY().mY;
		size *= camera.getScreenShotScale();

		// determine rotation factor
		float rotation = 0.5f - screenPos.mX;

		// determine matrix
		VuMatrix mat = camera.getTransform();
		mat.setTrans(pos);
		mat.scaleLocal(VuVector3(size, 1.0f, size));
		mat.rotateYLocal(iter->mRotationOffset + rotation*iter->mRotationAmount);

		// set constants
		VuColor color = iter->mColor;
		color.mA = (VUUINT8)VuRound(color.mA*alpha);
		VuGfxUtil::IF()->basicShaders()->set3dXyzUvConstants(mat*camera.getViewProjMatrix(), color);

		// build verts
		VuVertex3dXyzUv verts[4];
		verts[0].mXyz[0] = -0.5f; verts[0].mXyz[1] = 0.0f; verts[0].mXyz[2] = -0.5f; verts[0].mUv[0] = iter->mTexCoords.getLeft(); verts[0].mUv[1] = iter->mTexCoords.getBottom();
		verts[1].mXyz[0] =  0.5f; verts[1].mXyz[1] = 0.0f; verts[1].mXyz[2] = -0.5f; verts[1].mUv[0] = iter->mTexCoords.getRight(); verts[1].mUv[1] = iter->mTexCoords.getBottom();
		verts[2].mXyz[0] = -0.5f; verts[2].mXyz[1] = 0.0f; verts[2].mXyz[2] =  0.5f; verts[2].mUv[0] = iter->mTexCoords.getLeft(); verts[2].mUv[1] = iter->mTexCoords.getTop();
		verts[3].mXyz[0] =  0.5f; verts[3].mXyz[1] = 0.0f; verts[3].mXyz[2] =  0.5f; verts[3].mUv[0] = iter->mTexCoords.getRight(); verts[3].mUv[1] = iter->mTexCoords.getTop();

		// draw
		VuGfx::IF()->drawPrimitiveUP(VUGFX_PT_TRIANGLESTRIP, 2, verts);
	}
}