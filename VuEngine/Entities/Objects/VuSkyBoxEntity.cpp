//*****************************************************************************
//
//  Copyright (c) 2008-2008 Ralf Knoesel
//  Copyright (c) 2008-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  SkyBox entity
// 
//*****************************************************************************

#include "VuEngine/Entities/VuEntity.h"
#include "VuEngine/Components/Transform/VuTransformComponent.h"
#include "VuEngine/Components/3dDraw/Vu3dDrawComponent.h"
#include "VuEngine/Components/3dLayout/Vu3dLayoutComponent.h"
#include "VuEngine/Properties/VuAssetProperty.h"
#include "VuEngine/Assets/VuStaticModelAsset.h"
#include "VuEngine/Gfx/GfxScene/VuGfxStaticScene.h"
#include "VuEngine/Gfx/GfxScene/VuGfxSceneNode.h"
#include "VuEngine/Gfx/GfxScene/VuGfxSceneChunk.h"
#include "VuEngine/Gfx/GfxScene/VuGfxSceneMaterial.h"
#include "VuEngine/Gfx/GfxScene/VuGfxSceneMesh.h"
#include "VuEngine/Gfx/GfxScene/VuGfxSceneMeshInstance.h"
#include "VuEngine/Gfx/GfxScene/VuGfxSceneMeshPart.h"
#include "VuEngine/Gfx/GfxSort/VuGfxSort.h"
#include "VuEngine/HAL/Gfx/VuGfx.h"
#include "VuEngine/Managers/VuViewportManager.h"


class VuSkyBoxEntity : public VuEntity
{
	DECLARE_RTTI

public:
	VuSkyBoxEntity();
	~VuSkyBoxEntity();

	virtual void	onGameInitialize();
	virtual void	onGameRelease();

private:
	// event handlers
	void			OnSwapSkybox(const VuParams &params);

	void			drawLayout(const Vu3dLayoutDrawParams &params);
	void			draw(const VuGfxDrawParams &params);
	void			drawInternal(const VuGfxDrawParams &params, VuStaticModelAsset *pModelAsset);
	void			drawRecursive(VuGfxSceneNode *pNode, const VuMatrix &transform, const VuGfxDrawParams &params) const;

	// components
	Vu3dDrawComponent					*mp3dDrawComponent;
	Vu3dLayoutComponent					*mp3dLayoutComponent;

	// properties
	std::string							mModelAssetName;
	float								mHeightOffset;

	// property references
	VuAssetProperty<VuStaticModelAsset>	*mpModelAssetProperty;

	VuStaticModelAsset					*mpModelAssetOverride[VuViewportManager::MAX_VIEWPORTS];
};


IMPLEMENT_RTTI(VuSkyBoxEntity, VuEntity);
IMPLEMENT_ENTITY_REGISTRATION(VuSkyBoxEntity);


// static functions
struct SkyBoxDrawData
{
	VuMatrix			mTransform;
	VuGfxSceneMeshPart	*mpPart;
};
static void SkyBoxDrawCallback(void *data);


//*****************************************************************************
VuSkyBoxEntity::VuSkyBoxEntity():
	mHeightOffset(0.0f)
{
	// properties
	addProperty(mpModelAssetProperty = new VuAssetProperty<VuStaticModelAsset>("Model Asset", mModelAssetName));
	addProperty(new VuFloatProperty("Height Offset", mHeightOffset));

	// components
	addComponent(mp3dDrawComponent = new Vu3dDrawComponent(this, true));
	mp3dDrawComponent->setDrawMethod(this, &VuSkyBoxEntity::draw);
	mp3dDrawComponent->updateVisibility(VuAabb(VuVector3(-1e9, -1e9, -1e9), VuVector3(1e9, 1e9, 1e9)));

	addComponent(mp3dLayoutComponent = new Vu3dLayoutComponent(this));
	mp3dLayoutComponent->setDrawMethod(this, &VuSkyBoxEntity::drawLayout);
	mp3dLayoutComponent->setLocalBounds(VuAabb(VuVector3(-1e9, -1e9, -1e9), VuVector3(1e9, 1e9, 1e9)));

	// register event handlers
	REG_EVENT_HANDLER(VuSkyBoxEntity, OnSwapSkybox);

	for ( int i = 0; i < VuViewportManager::MAX_VIEWPORTS; i++ )
		mpModelAssetOverride[i] = VUNULL;
}

//*****************************************************************************
VuSkyBoxEntity::~VuSkyBoxEntity()
{
	for ( int i = 0; i < VuViewportManager::MAX_VIEWPORTS; i++ )
		if ( mpModelAssetOverride[i] )
			VuAssetFactory::IF()->releaseAsset(mpModelAssetOverride[i]);
}

//*****************************************************************************
void VuSkyBoxEntity::onGameInitialize()
{
	mp3dDrawComponent->show();
}

//*****************************************************************************
void VuSkyBoxEntity::onGameRelease()
{
	mp3dDrawComponent->hide();
}

//*****************************************************************************
void VuSkyBoxEntity::OnSwapSkybox(const VuParams &params)
{
	VuParams::VuAccessor accessor(params);
	int viewportIndex = accessor.getInt();
	VuAsset *pAsset = accessor.getAsset();

	if ( viewportIndex >= 0 && viewportIndex < VuViewportManager::MAX_VIEWPORTS )
	{
		if ( mpModelAssetOverride[viewportIndex] )
		{
			VuAssetFactory::IF()->releaseAsset(mpModelAssetOverride[viewportIndex]);
			mpModelAssetOverride[viewportIndex] = VUNULL;
		}

		if ( pAsset && pAsset->isDerivedFrom(VuStaticModelAsset::msRTTI) )
		{
			mpModelAssetOverride[viewportIndex] = static_cast<VuStaticModelAsset *>(pAsset);
			VuAssetFactory::IF()->addAssetRef(mpModelAssetOverride[viewportIndex]);
		}
	}
}

//*****************************************************************************
void VuSkyBoxEntity::drawLayout(const Vu3dLayoutDrawParams &params)
{
	drawInternal(VuGfxDrawParams(params.mCamera), mpModelAssetProperty->getAsset());
}

//*****************************************************************************
void VuSkyBoxEntity::draw(const VuGfxDrawParams &params)
{
	int viewportIndex = VuGfxSort::IF()->getViewport();
	VUASSERT(viewportIndex < VuViewportManager::MAX_VIEWPORTS, "VuSkyBoxEntity::draw() bad viewport index");

	if ( mpModelAssetOverride[viewportIndex] )
		drawInternal(params, mpModelAssetOverride[viewportIndex]);
	else
		drawInternal(params, mpModelAssetProperty->getAsset());
}

//*****************************************************************************
void VuSkyBoxEntity::drawInternal(const VuGfxDrawParams &params, VuStaticModelAsset *pModelAsset)
{
	if ( pModelAsset )
	{
		// draw to skybox viewport layer
		VUUINT prevVPL = VuGfxSort::IF()->getViewportLayer();
		VuGfxSort::IF()->setViewportLayer(VuGfxSort::VPL_WORLD);

		// calculate matrix (from camera position)
		VuMatrix modelMat;
		modelMat = mpTransformComponent->getWorldTransform();
		modelMat.setTrans(params.mEyePos + VuVector3(0.0f, 0.0f, mHeightOffset));

		// recursively draw parts
		for ( VuGfxStaticScene::Nodes::iterator iter = pModelAsset->getGfxStaticScene()->mNodes.begin(); iter != pModelAsset->getGfxStaticScene()->mNodes.end(); iter++ )
			drawRecursive(*iter, modelMat, params);

		// restore previous viewport layer
		VuGfxSort::IF()->setViewportLayer(prevVPL);
	}
}

//*****************************************************************************
void VuSkyBoxEntity::drawRecursive(VuGfxSceneNode *pNode, const VuMatrix &transform, const VuGfxDrawParams &params) const
{
	VuMatrix nodeTransform = pNode->mTransform*transform;

	if ( pNode->mpMeshInstance )
	{
		VuGfxSceneMesh *pMesh = pNode->mpMeshInstance->mpMesh;

		for ( VuGfxSceneMesh::Parts::iterator iter = pMesh->mParts.begin(); iter != pMesh->mParts.end(); iter++ )
		{
			VuGfxSceneMaterial *pMaterial = (*iter)->mpMaterial;
			VuMaterialAsset *pMaterialAsset = pMaterial->mpMaterialAsset;

			// submit draw command
			SkyBoxDrawData *pData = static_cast<SkyBoxDrawData *>(VuGfxSort::IF()->allocateCommandMemory(sizeof(SkyBoxDrawData)));
			pData->mTransform = nodeTransform;
			pData->mpPart = *iter;

			VuGfxSortMaterial *pGfxSortMaterial = pMaterialAsset->mpGfxSortMaterials[VuMaterialAsset::FLV_OPAQUE];
			VuGfxSort::IF()->submitDrawCommand<false>(VuGfxSort::TRANS_SKYBOX, pGfxSortMaterial, (*iter)->mpChunk->mpGfxSortMesh, SkyBoxDrawCallback);
		}
	}

	// recurse
	for ( VuGfxSceneNode::Children::iterator iter = pNode->mChildren.begin(); iter != pNode->mChildren.end(); iter++ )
		drawRecursive(*iter, nodeTransform, params);
}

//*****************************************************************************
void SkyBoxDrawCallback(void *data)
{
	SkyBoxDrawData *pData = static_cast<SkyBoxDrawData *>(data);
	VuGfxSceneMeshPart *pMeshPart = pData->mpPart;
	VuGfxSceneMaterial *pMaterial = pMeshPart->mpMaterial;
	VuMaterialAsset *pMaterialAsset = pMaterial->mpMaterialAsset;

	pMaterialAsset->setModelMatrix(pData->mTransform);

	VuGfx::IF()->drawIndexedPrimitive(
		VUGFX_PT_TRIANGLELIST,
		pMeshPart->mMinIndex,
		pMeshPart->mVertCount,
		pMeshPart->mStartIndex,
		pMeshPart->mTriCount
	);
}
