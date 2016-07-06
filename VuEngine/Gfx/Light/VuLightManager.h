//*****************************************************************************
//
//  Copyright (c) 2006-2008 Ralf Knoesel
//  Copyright (c) 2008-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  LightManager class.
// 
//*****************************************************************************

#pragma once

#include "VuEngine/VuSystemComponent.h"
#include "VuAmbientLight.h"
#include "VuDirectionalLight.h"
#include "VuDynamicLight.h"
#include "VuEngine/Managers/VuViewportManager.h"
#include "VuEngine/Containers/VuArray.h"

class VuGfxDrawParams;
class VuDbvt;


class VuLightManager : public VuSystemComponent
{
	DECLARE_SYSTEM_COMPONENT(VuLightManager)

public:
	VuLightManager();
	~VuLightManager();

protected:
	// called by engine
	friend class VuEngine;
	virtual bool		init();

public:
	// global ambient/directional lights
	VuAmbientLight		&ambientLight()	 	{ return mAmbientLight; } 
	VuDirectionalLight	&directionalLight()	{ return mDirectionalLight; } 

	typedef VuArray<VuRenderLight> RenderLights;
	void				gatherLights(const VuGfxDrawParams &params);
	void				getShaderLights(const VuVector3 &point, VUUINT32 groupMask, VuShaderLights &shaderLights);

	void				reset();
	void				synchronize();

protected:
	// called by lights
	friend class VuDynamicLight;
	void				addDynamicLight(VuDynamicLight *pLight);
	void				removeDynamicLight(VuDynamicLight *pLight);
	void				updateDynamicLight(VuDynamicLight *pLight);

private:
	void				draw();

	RenderLights		mRenderLights[2][VuViewportManager::MAX_VIEWPORTS][2];
	int					mSimIndex;
	int					mGfxIndex;

	VuAmbientLight		mAmbientLight;
	VuDirectionalLight	mDirectionalLight;

	VuDbvt				*mpDbvt;

	bool				mbDebugDrawDynamicLights;
};