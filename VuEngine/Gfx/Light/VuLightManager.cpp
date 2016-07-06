//*****************************************************************************
//
//  Copyright (c) 2006-2008 Ralf Knoesel
//  Copyright (c) 2008-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  LightManager class.
// 
//*****************************************************************************

#include "VuLightManager.h"
#include "VuEngine/Managers/VuDrawManager.h"
#include "VuEngine/Managers/VuViewportManager.h"
#include "VuEngine/Gfx/VuGfxDrawParams.h"
#include "VuEngine/Gfx/Camera/VuCamera.h"
#include "VuEngine/Gfx/GfxSort/VuGfxSort.h"
#include "VuEngine/Containers/VuDbvt.h"
#include "VuEngine/Dev/VuDevMenu.h"


// the interface
IMPLEMENT_SYSTEM_COMPONENT(VuLightManager, VuLightManager);

// VuDbvt policies
struct VuGetLightsPolicy
{
	VuGetLightsPolicy(const VuGfxDrawParams &params, VuArray<VuRenderLight> &lights, int viewportIndex);
	void					process(const VuDbvtNode *pNode);
	const VuGfxDrawParams	&mParams;
	VuArray<VuRenderLight>	&mLights;
	VUUINT32				mViewportMask;
};


//*****************************************************************************
VuLightManager::VuLightManager():
	mSimIndex(0),
	mGfxIndex(1),
	mbDebugDrawDynamicLights(false)
{
	// create dynamic bounding volume tree
	mpDbvt = new VuDbvt;

	VuDrawManager::IF()->registerHandler(this, &VuLightManager::draw);
}

//*****************************************************************************
VuLightManager::~VuLightManager()
{
	VuDrawManager::IF()->unregisterHandler(this);

	delete mpDbvt;
}

//*****************************************************************************
bool VuLightManager::init()
{
	if ( VuDevMenu::IF() )
	{
		VuDevMenu::IF()->addBool("Lighting/Draw Dynamic Lights", mbDebugDrawDynamicLights);
	}

	return true;
}

//*****************************************************************************
void VuLightManager::gatherLights(const VuGfxDrawParams &params)
{
	int planeCount = 0;
	VuVector4 planes[7];

	// add frustum planes
	const VuVector4 *cameraPlanes = params.mCamera.getFrustumPlanes();
	for ( int i = 0; i < 6; i++ )
		planes[planeCount++] = cameraPlanes[i];

	// add reflection plane
	if ( params.mbDrawReflection )
		planes[planeCount++] = params.mReflectionPlane;

	// traverse tree
	int viewportIndex = VuGfxSort::IF()->getViewport();
	int reflectionIndex = VuGfxSort::IF()->getReflectionLayer() - VuGfxSort::REFLECTION_ON;
	VUASSERT(viewportIndex >= 0 && viewportIndex < VuViewportManager::MAX_VIEWPORTS, "VuLightManager::gatherLights() bad viewport index");
	VUASSERT(reflectionIndex >= 0 && reflectionIndex < 2, "VuLightManager::gatherLights() bad reflection index");
	RenderLights &lights = mRenderLights[mSimIndex][viewportIndex][reflectionIndex];
	VuGetLightsPolicy policy(params, lights, viewportIndex);
	VuDbvt::collideKDOP<256>(mpDbvt->getRoot(), planes, planeCount, policy);
}

//*****************************************************************************
void VuLightManager::getShaderLights(const VuVector3 &point, VUUINT32 groupMask, VuShaderLights &shaderLights)
{
	int viewportIndex = VuGfxSort::IF()->getRenderViewport();
	int reflectionIndex = VuGfxSort::IF()->getRenderReflectionLayer() - VuGfxSort::REFLECTION_ON;

	const RenderLights &lights = mRenderLights[mGfxIndex][viewportIndex][reflectionIndex];

	memset(&shaderLights, 0, sizeof(shaderLights));

	int count = 0;
	const VuRenderLight *pLight = &lights.begin();
	for (int i = 0; i < lights.size(); i++)
	{
		VuVector3 delta;
		delta.mX = point.mX - pLight->mPosition.mX;
		delta.mY = point.mY - pLight->mPosition.mY;
		delta.mZ = point.mZ - pLight->mPosition.mZ;

		float distSquared = delta.magSquared();

		if (distSquared < pLight->mRange.mY*pLight->mRange.mY)
		{
			float dist = VuSqrt(distSquared);
			VuVector3 dir = delta / dist;
			float cosAngle = dir.mX*pLight->mDirection.mX + dir.mY*pLight->mDirection.mY + dir.mZ*pLight->mDirection.mZ;
			if (cosAngle > pLight->mRange.mW)
			{
				float amount = 1.0f;
				if (dist > pLight->mRange.mX)
					amount *= (pLight->mRange.mY - dist) / (pLight->mRange.mY - pLight->mRange.mX);

				if (cosAngle < pLight->mRange.mZ)
					amount *= (pLight->mRange.mW - cosAngle) / (pLight->mRange.mW - pLight->mRange.mZ);

				shaderLights.mDirections[count] = VuVector4(dir.mX, dir.mY, dir.mZ, 0.0f);
				shaderLights.mDiffuseColors[count] = amount*pLight->mDiffuseColor;
				count++;

				if (count == VuShaderLights::MAX_DYNAMIC_LIGHT_COUNT)
					break;
			}
		}

		pLight++;
	}
}

//*****************************************************************************
void VuLightManager::reset()
{
	mAmbientLight.reset();
	mDirectionalLight.reset();
}

//*****************************************************************************
void VuLightManager::synchronize()
{
	mSimIndex ^= 1;
	mGfxIndex ^= 1;
}

//*****************************************************************************
void VuLightManager::addDynamicLight(VuDynamicLight *pLight)
{
	VuDbvtNode *pNode = mpDbvt->insert(pLight, pLight->getAabb());
	pLight->setDbvtNode(pNode);
}

//*****************************************************************************
void VuLightManager::removeDynamicLight(VuDynamicLight *pLight)
{
	mpDbvt->remove(pLight->getDbvtNode());
	pLight->setDbvtNode(VUNULL);
}

//*****************************************************************************
void VuLightManager::updateDynamicLight(VuDynamicLight *pLight)
{
	mpDbvt->update(pLight->getDbvtNode(), pLight->getAabb());
}

//*****************************************************************************
void VuLightManager::draw()
{
	if ( mbDebugDrawDynamicLights )
	{
		VuGfxSort::IF()->setFullScreenLayer(VuGfxSort::FSL_GAME);
		VuGfxSort::IF()->setReflectionLayer(VuGfxSort::REFLECTION_OFF);
		VuGfxSort::IF()->setViewportLayer(VuGfxSort::VPL_WORLD);

		for ( int viewportIndex = 0; viewportIndex < VuViewportManager::IF()->getViewportCount(); viewportIndex++ )
		{
			VuGfxSort::IF()->setViewport(viewportIndex);

			const VuMatrix &viewProjMat = VuViewportManager::IF()->getViewport(viewportIndex).mCamera.getViewProjMatrix();

			const RenderLights &lights = mRenderLights[mSimIndex][viewportIndex][1];
			for ( int iLight = 0; iLight < lights.size(); iLight++ )
			{
				lights[iLight].debugDraw(viewProjMat);
			}
		}

		VuGfxSort::IF()->setFullScreenLayer(0);
		VuGfxSort::IF()->setViewport(0);
		VuGfxSort::IF()->setReflectionLayer(0);
		VuGfxSort::IF()->setViewportLayer(0);
	}
}

//*****************************************************************************
VuGetLightsPolicy::VuGetLightsPolicy(const VuGfxDrawParams &params, VuArray<VuRenderLight> &lights, int viewportIndex):
	mParams(params), mLights(lights), mViewportMask(1<<viewportIndex)
{
	mLights.clear();
}

//*****************************************************************************
void VuGetLightsPolicy::process(const VuDbvtNode *pNode)
{
	VuDynamicLight *pLight = static_cast<VuDynamicLight *>(pNode->mpData);

	if (pLight->mViewportMask & mViewportMask)
	{
		if ((mParams.mbDrawReflection & pLight->mbReflecting) == mParams.mbDrawReflection)
		{
			float distSquared = (pLight->mPosition - mParams.mEyePos).magSquared();
			if (distSquared < pLight->mDrawDistance*pLight->mDrawDistance)
				mLights.push_back(pLight->getRenderLight());
		}
	}
}
