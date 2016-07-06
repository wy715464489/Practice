//*****************************************************************************
//
//  Copyright (c) 2012-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  VuVertexColorComponent class
// 
//*****************************************************************************

#include "VuVertexColorComponent.h"
#include "Vu3dDrawStaticModelComponent.h"
#include "VuEngine/VuEngine.h"
#include "VuEngine/Entities/VuEntity.h"
#include "VuEngine/Entities/Lights/VuAmbientLightEntity.h"
#include "VuEngine/Entities/Lights/VuDirectionalLightEntity.h"
#include "VuEngine/Entities/Lights/VuDynamicLightEntity.h"
#include "VuEngine/Components/Transform/VuTransformComponent.h"
#include "VuEngine/Assets/VuMaterialAsset.h"
#include "VuEngine/Gfx/Model/VuStaticModelInstance.h"
#include "VuEngine/Gfx/GfxScene/VuGfxStaticScene.h"
#include "VuEngine/Gfx/GfxScene/VuGfxSceneChunk.h"
#include "VuEngine/Gfx/GfxScene/VuGfxSceneNode.h"
#include "VuEngine/Gfx/GfxScene/VuGfxSceneMesh.h"
#include "VuEngine/Gfx/GfxScene/VuGfxSceneMeshPart.h"
#include "VuEngine/Gfx/GfxScene/VuGfxSceneMeshInstance.h"
#include "VuEngine/Gfx/GfxScene/VuGfxSceneMaterial.h"
#include "VuEngine/Gfx/Light/VuLightUtil.h"
#include "VuEngine/HAL/Gfx/VuGfx.h"
#include "VuEngine/HAL/Gfx/VuVertexBuffer.h"
#include "VuEngine/Gfx/VuGfxUtil.h"
#include "VuEngine/Math/VuMathUtil.h"


IMPLEMENT_RTTI(VuVertexColorComponent, VuComponent);



//*****************************************************************************
VuVertexColorComponent::VuVertexColorComponent(VuEntity *pOwner):
	VuComponent(pOwner)
{
}

//*****************************************************************************
void VuVertexColorComponent::onLoad(const VuJsonContainer &data)
{
	if ( mModelVertexColors.mChunkCount )
		VUPRINTF("Warning:  vertex colors loaded more than once for %s\n", getOwnerEntity()->getLongName().c_str());

	clear();

	if ( data.isArray() )
	{
		if ( Vu3dDrawStaticModelComponent *pDrawComponent = getOwnerEntity()->getComponent<Vu3dDrawStaticModelComponent>() )
		{
			mModelVertexColors.load(data[0]);
			mLod1VertexColors.load(data[1]);
			mLod2VertexColors.load(data[2]);
			mReflectionVertexColors.load(data[3]);

			if ( VuGfxUtil::IF()->getUltraModelLOD() || VuEngine::IF()->editorMode() )
				mUltraVertexColors.load(data[4]);
		}
	}
}

//*****************************************************************************
void VuVertexColorComponent::onPostLoad()
{
	apply();
}

//*****************************************************************************
void VuVertexColorComponent::onSave(VuJsonContainer &data) const
{
	if ( Vu3dDrawStaticModelComponent *pDrawComponent = getOwnerEntity()->getComponent<Vu3dDrawStaticModelComponent>() )
	{
		int hasVertexColors = 0;
		hasVertexColors |= mModelVertexColors.mChunkCount;
		hasVertexColors |= mLod1VertexColors.mChunkCount;
		hasVertexColors |= mLod2VertexColors.mChunkCount;
		hasVertexColors |= mReflectionVertexColors.mChunkCount;
		hasVertexColors |= mUltraVertexColors.mChunkCount;

		if ( hasVertexColors )
		{
			mModelVertexColors.save(data[0]);
			mLod1VertexColors.save(data[1]);
			mLod2VertexColors.save(data[2]);
			mReflectionVertexColors.save(data[3]);
			mUltraVertexColors.save(data[4]);
		}
	}
}

//*****************************************************************************
void VuVertexColorComponent::onBake()
{
	clear();

	if ( Vu3dDrawStaticModelComponent *pDrawComponent = getOwnerEntity()->getComponent<Vu3dDrawStaticModelComponent>() )
	{
		recalculate(pDrawComponent->modelInstance(), mModelVertexColors);
		recalculate(pDrawComponent->lod1ModelInstance(), mLod1VertexColors);
		recalculate(pDrawComponent->lod2ModelInstance(), mLod2VertexColors);
		recalculate(pDrawComponent->reflectionModelInstance(), mReflectionVertexColors);
		recalculate(pDrawComponent->ultraModelInstance(), mUltraVertexColors);
	}

	apply();
}

//*****************************************************************************
void VuVertexColorComponent::onClearBaked()
{
	clear();
	apply();
}

//*****************************************************************************
void VuVertexColorComponent::clear()
{
	mModelVertexColors.clear();
	mLod1VertexColors.clear();
	mLod2VertexColors.clear();
	mReflectionVertexColors.clear();
	mUltraVertexColors.clear();
}

//*****************************************************************************
void VuVertexColorComponent::apply()
{
	if ( Vu3dDrawStaticModelComponent *pDrawComponent = getOwnerEntity()->getComponent<Vu3dDrawStaticModelComponent>() )
	{
		bool success = true;

		success &= pDrawComponent->modelInstance().setVertexColors(mModelVertexColors.mppVertexBuffers, mModelVertexColors.mChunkCount);
		success &= pDrawComponent->lod1ModelInstance().setVertexColors(mLod1VertexColors.mppVertexBuffers, mLod1VertexColors.mChunkCount);
		success &= pDrawComponent->lod2ModelInstance().setVertexColors(mLod2VertexColors.mppVertexBuffers, mLod2VertexColors.mChunkCount);
		success &= pDrawComponent->reflectionModelInstance().setVertexColors(mReflectionVertexColors.mppVertexBuffers, mReflectionVertexColors.mChunkCount);
		success &= pDrawComponent->ultraModelInstance().setVertexColors(mUltraVertexColors.mppVertexBuffers, mUltraVertexColors.mChunkCount);

		if ( !success )
			VUPRINTF("Warning:  vertex color mismatch for %s\n", getOwnerEntity()->getLongName().c_str());
	}	
}

//*****************************************************************************
void VuVertexColorComponent::recalculate(VuStaticModelInstance &modelInstance, VertexColors &vertexColors)
{
	if ( needsVertexColors(modelInstance) )
	{
		VuGfxStaticScene *pScene = modelInstance.getGfxStaticScene();
		int chunkCount = (int)pScene->mChunks.size();

		// allocate vertex colors
		RawVertexColors rawVertexColors;
		rawVertexColors.resize(chunkCount);
		for ( int iChunk = 0; iChunk < chunkCount; iChunk++ )
		{
			VuGfxSceneChunk *pChunk = pScene->mChunks[iChunk];
			int vertexCount = pChunk->mpVertexBuffer->mSize/pChunk->mVertexStride;
			rawVertexColors[iChunk].resize(vertexCount);

			memset(&rawVertexColors[iChunk][0], 0xff, vertexCount*4);
		}

		// determine model matrix
		VuTransformComponent *pTransformComponent = getOwnerEntity()->getTransformComponent();
		VuMatrix modelMat = pTransformComponent->getWorldTransform();
		modelMat.scaleLocal(pTransformComponent->getWorldScale());

		// gather light info
		VuLightUtil::VuLightInfo lightInfo(modelInstance.getAabb(), modelMat);
		lightInfo.mOccluders.push_back(VuLightUtil::VuLightInfo::VuOccluder(&modelInstance, modelMat));
		VuLightUtil::gatherLightsRecursive(getOwnerEntity()->getRootEntity(), lightInfo);
		VuLightUtil::gatherOccludersRecursive(getOwnerEntity()->getRootEntity(), getOwnerEntity(), lightInfo);

		// recursively calculate vertex colors
		for ( VuGfxStaticScene::Nodes::iterator iter = pScene->mNodes.begin(); iter != pScene->mNodes.end(); iter++ )
			recalculateRecursive(*iter, modelMat, lightInfo, rawVertexColors);

		// copy to vertex colors
		vertexColors.setRaw(rawVertexColors);
	}
}

//*****************************************************************************
void VuVertexColorComponent::recalculateRecursive(VuGfxSceneNode *pNode, const VuMatrix &transform, const VuLightUtil::VuLightInfo &lightInfo, RawVertexColors &vertexColors)
{
	VuMatrix nodeTransform = pNode->mTransform*transform;

	if ( pNode->mpMeshInstance )
	{
		VuGfxSceneMesh *pMesh = pNode->mpMeshInstance->mpMesh;

		for ( VuGfxSceneMesh::Parts::iterator itPart = pMesh->mParts.begin(); itPart != pMesh->mParts.end(); itPart++ )
		{
			VuGfxSceneMaterial *pMaterial = (*itPart)->mpMaterial;
			VuMaterialAsset *pMaterialAsset = pMaterial->mpMaterialAsset;

			if ( pMaterialAsset->mbSceneLighting )
			{
				VuGfxSceneMeshPart *pPart = *itPart;
				VuGfxSceneChunk *pChunk = pPart->mpChunk;
				std::vector<VUUINT32> &vcBuffer = vertexColors[pChunk->mIndex];
				int vertexStride = pChunk->mVertexStride;

				const VUBYTE *pVertexData = static_cast<const VUBYTE *>(pChunk->mpVertexBuffer->getShadowBuffer());

				#pragma omp parallel for
				for ( int index = pPart->mMinIndex; index <= pPart->mMaxIndex; index++ )
				{
					// assume position is defined as first 3 floats of vertex
					float *pPosition = (float *)(pVertexData + vertexStride*index);
					VuVector3 position(pPosition[0], pPosition[1], pPosition[2]);
					position = nodeTransform.transform(position);

					// assume normal is defined as next 3 shorts
					VUINT16 *pNormal = (VUINT16 *)(pPosition + 3);
					VuVector3 normal(pNormal[0]/32767.0f, pNormal[1]/32767.0f, pNormal[2]/32767.0f);
					normal = nodeTransform.transformNormal(normal);
					normal.normalize();

					vcBuffer[index] = VuColor(VuLightUtil::calculateVertexColor(position, normal, lightInfo, pMaterialAsset->mbDoesReceiveShadows));
				}
			}
		}
	}

	// recurse
	for ( VuGfxSceneNode::Children::iterator iter = pNode->mChildren.begin(); iter != pNode->mChildren.end(); iter++ )
		recalculateRecursive(*iter, nodeTransform, lightInfo, vertexColors);
}

//*****************************************************************************
bool VuVertexColorComponent::needsVertexColors(VuStaticModelInstance &modelInstance)
{
	if ( VuGfxStaticScene *pScene = modelInstance.getGfxStaticScene() )
	{
		for ( VuGfxStaticScene::Materials::iterator iter = pScene->mMaterials.begin(); iter != pScene->mMaterials.end(); iter++ )
		{
			if ( (*iter)->mpMaterialAsset->mbSceneLighting )
				return true;
		}
	}

	return false;
}

//*****************************************************************************
void VuVertexColorComponent::VertexColors::clear()
{
	for ( int i = 0; i < mChunkCount; i++ )
		mppVertexBuffers[i]->removeRef();
	delete[] mppVertexBuffers;

	mppVertexBuffers = VUNULL;
	mChunkCount = 0;
}

//*****************************************************************************
void VuVertexColorComponent::VertexColors::load(const VuJsonContainer &data)
{
	mChunkCount = data.size();
	if ( mChunkCount )
	{
		mppVertexBuffers = new VuVertexBuffer *[mChunkCount];

		for ( int i = 0; i < mChunkCount; i++ )
		{
			VuVertexBuffer *pVertexBuffer = VUNULL;

			const void *pData;
			int size;
			if ( data[i].getValue(pData, size) )
			{
				pVertexBuffer = VuGfx::IF()->createVertexBuffer(size, 0);
				pVertexBuffer->setData(pData, size);
			}
			else
			{
				pVertexBuffer = VuGfxUtil::IF()->blackVertexColors();
				pVertexBuffer->addRef();
			}

			mppVertexBuffers[i] = pVertexBuffer;
		}
	}
}

//*****************************************************************************
void VuVertexColorComponent::VertexColors::save(VuJsonContainer &data) const
{
	for ( int i = 0; i < mChunkCount; i++ )
	{
		VuVertexBuffer *pVertexBuffer = mppVertexBuffers[i];
		data[i].putValue(pVertexBuffer->getShadowBuffer(), pVertexBuffer->mSize);
	}
}

//*****************************************************************************
void VuVertexColorComponent::VertexColors::setRaw(RawVertexColors &rawVertexColors)
{
	mChunkCount = (int)rawVertexColors.size();
	if ( mChunkCount )
	{
		mppVertexBuffers = new VuVertexBuffer *[mChunkCount];
		for ( int i = 0; i < mChunkCount; i++ )
		{
			auto &chunkColors = rawVertexColors[i];
			int vertexCount = (int)chunkColors.size();

			mppVertexBuffers[i] = VuGfx::IF()->createVertexBuffer(vertexCount*4, 0);
			mppVertexBuffers[i]->setData(&chunkColors[0], vertexCount*4);
		}
	}
}
