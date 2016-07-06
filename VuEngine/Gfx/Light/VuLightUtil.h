//*****************************************************************************
//
//  Copyright (c) 2013-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  Light Util
// 
//*****************************************************************************

#pragma once

#include "VuDynamicLight.h"
#include "VuEngine/Math/VuMatrix.h"

class VuAabb;
class VuDynamicLight;
class VuStaticModelInstance;
class VuEntity;


namespace VuLightUtil
{
	class VuLightInfo
	{
	public:
		VuLightInfo(const VuAabb &aabb, const VuMatrix &modelMat);

		VuVector3	mVisibilityPos;
		float		mVisibilityRadius;

		VuVector3	mDirLightPos;
		VuVector3	mDirLightDir;

		VuColor		mDirLightFrontColor;
		VuColor		mAmbLightColor;

		std::vector<VuDynamicLight>	mDynamicLights;

		class VuOccluder
		{
		public:
			VuOccluder(VuStaticModelInstance *pModelInstance, const VuMatrix &modelMat) : mpModelInstance(pModelInstance), mModelMat(modelMat) {}
			VuStaticModelInstance	*mpModelInstance;
			VuMatrix				mModelMat;
		};
		std::vector<VuOccluder> mOccluders;
	};

	void		gatherLightsRecursive(VuEntity *pEntity, VuLightInfo &lightInfo);
	void		gatherOccludersRecursive(VuEntity *pEntity, VuEntity *pIgnoreEntity, VuLightInfo &lightInfo);
	VuVector4	calculateVertexColor(const VuVector3 &position, const VuVector3 &normal, const VuLightInfo &lightInfo, bool receiveShadows);
	VuVector4	calculateFoliageColor(const VuVector3 &position, const VuLightInfo &lightInfo, bool receiveShadows);
}