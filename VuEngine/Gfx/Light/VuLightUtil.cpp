//*****************************************************************************
//
//  Copyright (c) 2013-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  Light Util
// 
//*****************************************************************************

#include "VuLightUtil.h"
#include "VuEngine/Entities/Lights/VuDirectionalLightEntity.h"
#include "VuEngine/Entities/Lights/VuAmbientLightEntity.h"
#include "VuEngine/Entities/Lights/VuDynamicLightEntity.h"
#include "VuEngine/Components/Transform/VuTransformComponent.h"
#include "VuEngine/Components/3dDraw/Vu3dDrawStaticModelComponent.h"
#include "VuEngine/Math/VuMathUtil.h"

#define FOLIAGE_SELF_SHADOWING_OFFSET 0.01f


//*****************************************************************************
void VuLightUtil::gatherLightsRecursive(VuEntity *pEntity, VuLightInfo &lightInfo)
{
	if ( pEntity->isDerivedFrom(VuDirectionalLightEntity::msRTTI) )
	{
		const VuDirectionalLightEntity *pDirectionalLightEntity = static_cast<const VuDirectionalLightEntity *>(pEntity);
		if ( pDirectionalLightEntity->isDefaultLight() )
		{
			lightInfo.mDirLightPos = pDirectionalLightEntity->getTransformComponent()->getWorldPosition();
			lightInfo.mDirLightDir = -pDirectionalLightEntity->getTransformComponent()->getWorldTransform().getAxisZ();
			lightInfo.mDirLightFrontColor = pDirectionalLightEntity->frontColor();
		}
	}
	else if ( pEntity->isDerivedFrom(VuAmbientLightEntity::msRTTI) )
	{
		const VuAmbientLightEntity *pAmbientLightEntity = static_cast<const VuAmbientLightEntity *>(pEntity);
		if ( pAmbientLightEntity->isDefaultLight() )
			lightInfo.mAmbLightColor = pAmbientLightEntity->color();
	}
	else if ( pEntity->isDerivedFrom(VuDynamicLightEntity::msRTTI) )
	{
		const VuDynamicLightEntity *pDynamicLightEntity = static_cast<const VuDynamicLightEntity *>(pEntity);
		const VuDynamicLight &dynamicLight = pDynamicLightEntity->dynamicLight();

		if ( VuDist(lightInfo.mVisibilityPos, dynamicLight.mPosition) < lightInfo.mVisibilityRadius + dynamicLight.mFalloffRangeMax )
			lightInfo.mDynamicLights.push_back(dynamicLight);
	}

	// recurse
	for ( int iChild = 0; iChild < pEntity->getChildEntityCount(); iChild++ )
		gatherLightsRecursive(pEntity->getChildEntity(iChild), lightInfo);
}

//*****************************************************************************
void VuLightUtil::gatherOccludersRecursive(VuEntity *pEntity, VuEntity *pIgnoreEntity, VuLightInfo &lightInfo)
{
	if ( Vu3dDrawStaticModelComponent *pDrawComponent = pEntity->getComponent<Vu3dDrawStaticModelComponent>() )
	{
		if ( pDrawComponent->castBakedShadow() && (pEntity != pIgnoreEntity) )
		{
			// determine model matrix
			VuTransformComponent *pTransformComponent = pEntity->getTransformComponent();
			VuMatrix modelMat = pTransformComponent->getWorldTransform();
			modelMat.scaleLocal(pTransformComponent->getWorldScale());

			// calculate visibility sphere (applies scaling too)
			VuVector3 vMinWorld = modelMat.transform(pDrawComponent->getModelAabb().mMin);
			VuVector3 vMaxWorld = modelMat.transform(pDrawComponent->getModelAabb().mMax);

			VuVector3 otherVisibilityPos = 0.5f*(vMinWorld + vMaxWorld);
			float otherVisibilityRadius = 0.5f*(vMaxWorld - vMinWorld).mag();

			// does this entity occlude?
			VuVector3 lightPos = lightInfo.mDirLightPos - lightInfo.mDirLightDir*VuDot(lightInfo.mDirLightDir, lightInfo.mVisibilityPos - lightInfo.mDirLightPos);
			float dist = VuMathUtil::distPointLineSeg(otherVisibilityPos, lightPos, lightInfo.mVisibilityPos);
			if ( dist < otherVisibilityRadius + lightInfo.mVisibilityRadius )
				lightInfo.mOccluders.push_back(VuLightInfo::VuOccluder(&pDrawComponent->modelInstance(), modelMat));
		}
	}

	// recurse
	for ( int iChild = 0; iChild < pEntity->getChildEntityCount(); iChild++ )
		gatherOccludersRecursive(pEntity->getChildEntity(iChild), pIgnoreEntity, lightInfo);
}

//*****************************************************************************
VuVector4 VuLightUtil::calculateVertexColor(const VuVector3 &position, const VuVector3 &normal, const VuLightInfo &lightInfo, bool receiveShadows)
{
	VuVector4 vertexColor(0,0,0,0);

	// dynamic lights
	for ( int iLight = 0; iLight < (int)lightInfo.mDynamicLights.size(); iLight++ )
	{
		const VuDynamicLight &dynamicLight = lightInfo.mDynamicLights[iLight];
		const VuRenderLight &renderLight = dynamicLight.getRenderLight();

		VuVector3 delta = position - dynamicLight.mPosition;
		float dist = delta.mag();
		VuVector3 dir = delta/dist;
		float cosAngle = VuDot(dir, dynamicLight.mDirection);
		float amount = 1.0f - VuSmoothStep(renderLight.mRange.mX, renderLight.mRange.mY, dist);
		amount *= VuSmoothStep(renderLight.mRange.mW, renderLight.mRange.mZ, cosAngle);
		vertexColor += VuMax(VuDot(normal, -dir), 0.0f)*amount*renderLight.mDiffuseColor;
	}

	// clamp
	vertexColor.mX = VuMin(vertexColor.mX, 1.0f);
	vertexColor.mY = VuMin(vertexColor.mY, 1.0f);
	vertexColor.mZ = VuMin(vertexColor.mZ, 1.0f);

	// directional shadow
	vertexColor.mW = 1.0f;
	if ( receiveShadows )
	{
		if ( VuDot(normal, lightInfo.mDirLightDir) <= 0.0f )
		{
			float dist = VuDot(lightInfo.mDirLightDir, position - lightInfo.mDirLightPos);
			if ( dist > 0.0f )
			{
				VuVector3 lightPos = position - dist*lightInfo.mDirLightDir;
				for ( int iOccluder = 0; iOccluder < (int)lightInfo.mOccluders.size(); iOccluder++ )
				{
					const VuLightInfo::VuOccluder &occluder = lightInfo.mOccluders[iOccluder];
					VuVector3 collidePos = position;
					if (occluder.mpModelInstance->collideRay(occluder.mModelMat, lightPos, collidePos, true))
						vertexColor.mW = 0.0f;
				}
			}
		}
	}

	return vertexColor;
}

//*****************************************************************************
VuVector4 VuLightUtil::calculateFoliageColor(const VuVector3 &position, const VuLightInfo &lightInfo, bool receiveShadows)
{
	VuVector4 vertexColor(0,0,0,0);

	// dynamic lights
	for ( int iLight = 0; iLight < (int)lightInfo.mDynamicLights.size(); iLight++ )
	{
		const VuDynamicLight &dynamicLight = lightInfo.mDynamicLights[iLight];
		const VuRenderLight &renderLight = dynamicLight.getRenderLight();

		VuVector3 delta = position - dynamicLight.mPosition;
		float dist = delta.mag();
		VuVector3 dir = delta/dist;
		float cosAngle = VuDot(dir, dynamicLight.mDirection);
		float amount = 1.0f - VuSmoothStep(renderLight.mRange.mX, renderLight.mRange.mY, dist);
		amount *= VuSmoothStep(renderLight.mRange.mW, renderLight.mRange.mZ, cosAngle);
		vertexColor += amount*renderLight.mDiffuseColor;
	}

	// clamp
	vertexColor.mX = VuMin(vertexColor.mX, 1.0f);
	vertexColor.mY = VuMin(vertexColor.mY, 1.0f);
	vertexColor.mZ = VuMin(vertexColor.mZ, 1.0f);

	// determine if in shadow
	vertexColor.mW = 1.0f;
	if ( receiveShadows )
	{
		float dist = VuDot(lightInfo.mDirLightDir, position - lightInfo.mDirLightPos);
		if ( dist > 0.0f )
		{
			VuVector3 lightPos = position - dist*lightInfo.mDirLightDir;
			for ( int iOccluder = 0; iOccluder < (int)lightInfo.mOccluders.size(); iOccluder++ )
			{
				const VuLightUtil::VuLightInfo::VuOccluder &occluder = lightInfo.mOccluders[iOccluder];
				VuVector3 collidePos = position - lightInfo.mDirLightDir*FOLIAGE_SELF_SHADOWING_OFFSET;
				if ( occluder.mpModelInstance->collideRay(occluder.mModelMat, lightPos, collidePos, true) )
				{
					vertexColor.mW = 0.0f;
					break;
				}
			}
		}
	}

	return vertexColor;
}

//*****************************************************************************
VuLightUtil::VuLightInfo::VuLightInfo(const VuAabb &aabb, const VuMatrix &modelMat):
	mDirLightPos(0,0,0),
	mDirLightDir(0,0,0),
	mDirLightFrontColor(0,0,0),
	mAmbLightColor(0,0,0)
{
	// calculate visibility sphere (applies scaling too)
	VuVector3 vMinWorld = modelMat.transform(aabb.mMin);
	VuVector3 vMaxWorld = modelMat.transform(aabb.mMax);

	mVisibilityPos = 0.5f*(vMinWorld + vMaxWorld);
	mVisibilityRadius = 0.5f*(vMaxWorld - vMinWorld).mag();
}
