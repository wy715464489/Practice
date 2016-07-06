//*****************************************************************************
//
//  Copyright (c) 2007-2008 Ralf Knoesel
//  Copyright (c) 2008-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  Water surface entity
// 
//*****************************************************************************

#pragma once

#include "VuEngine/Entities/VuEntity.h"
#include "VuEngine/Water/VuWater.h"
#include "VuEngine/Components/Motion/VuMotionComponentIF.h"
#include "VuEngine/Properties/VuAssetProperty.h"

class Vu3dLayoutDrawParams;
class VuGfxDrawParams;
class VuTransformComponent;
class Vu3dDrawComponent;
class Vu3dLayoutComponent;
class VuScriptComponent;
class VuMotionComponent;


class VuWaterSurfaceEntity : public VuEntity, public VuMotionComponentIF
{
	DECLARE_RTTI

public:
	VuWaterSurfaceEntity();
	~VuWaterSurfaceEntity();

	virtual void		onPostLoad();
	virtual void		onGameInitialize();
	virtual void		onGameRelease();

	VuWaterShader		*getShader() const { return mpShader; }
	VuWaterMapAsset		*getWaterMapAsset() const;
	VuLightMapAsset		*getLightMapAsset() const;

	int					getSizeX() const { return mSizeX; }
	int					getSizeY() const { return mSizeY; }
	virtual float		getLocalWaterHeight(float localX, float localY) const { return 0.0f; }

protected:
	// event handlers
	void				OnWaterSettingsChanged(const VuParams &params);

	virtual void		surfaceModified();
	void				shaderModified();
	void				createSurfaceDesc(VuWaterSurfaceDesc &desc);
	void				createShaderDesc(VuWaterShaderDesc &desc);
	virtual void		drawLayout(const Vu3dLayoutDrawParams &params);
	virtual bool		collideLayout(const VuVector3 &v0, VuVector3 &v1);
	void				draw(const VuGfxDrawParams &params);

	void				configReflection(bool value) { shaderModified(); }
	void				configNormalMap(bool value) { shaderModified(); }
	void				configShaderLOD(int value) { shaderModified(); }

	// VuMotionComponentIF interface
	virtual void		onMotionUpdate();

	Vu3dDrawComponent		*mp3dDrawComponent;
	Vu3dLayoutComponent		*mp3dLayoutComponent;
	VuScriptComponent		*mpScriptComponent;
	VuMotionComponent		*mpMotionComponent;

	typedef VuAssetProperty<VuWaterMapAsset> WaterMapAsset;
	typedef VuAssetProperty<VuLightMapAsset> LightMapAsset;

	int					mSizeX;
	int					mSizeY;
	float				mMaxWaveDepth;
	float				mMaxWaveHeight;
	float				mReflectionHeight;
	float				mReflectionOffset;
	int					mMinRecursionDepth;
	float				mDrawDist;
	std::string			mWaterMapAssetName;
	std::string			mLightMapAssetName;

	VuWaterShaderDesc	mShaderDesc;
	VuWaterSurface		*mpSurface;
	VuWaterShader		*mpShader;

	WaterMapAsset		*mpWaterMapAssetProperty;
	LightMapAsset		*mpLightMapAssetProperty;
};
