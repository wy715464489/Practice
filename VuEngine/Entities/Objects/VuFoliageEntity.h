//*****************************************************************************
//
//  Copyright (c) 2011-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  Foliage entity
// 
//*****************************************************************************

#include "VuEngine/Entities/VuEntity.h"
#include "VuEngine/Properties/VuAssetProperty.h"
#include "VuEngine/Managers/VuFoliageManager.h"

class Vu3dLayoutComponent;
class Vu3dDrawComponent;
class Vu3dLayoutDrawParams;
class VuGfxDrawParams;


class VuFoliageEntity : public VuEntity
{
	DECLARE_RTTI

public:
	VuFoliageEntity();

	virtual void			onLoad(const VuJsonContainer &data) { loadInternal(data); }
	virtual void			onSave(VuJsonContainer &data) const { saveInternal(data); }
	virtual void			onBake();
	virtual void			onClearBaked();
	virtual void			onGameInitialize();
	virtual void			onGameRelease();

private:
	void					loadInternal(const VuJsonContainer &data);
	void					saveInternal(VuJsonContainer &data) const;

	void					drawLayout(const Vu3dLayoutDrawParams &params);
	void					draw(const VuGfxDrawParams &params);
	void					transformModified();
	void					textureModified();

	void					createBucket();
	void					releaseBucket();

	void					calculateDrawColor(VuColor &drawColor);
	static void				collideRayRecursive(const VuEntity *pEntity, const VuVector3 &v0, VuVector3 &v1);

	// components
	Vu3dLayoutComponent		*mp3dLayoutComponent;
	Vu3dDrawComponent		*mp3dDrawComponent;

	// properties
	std::string				mTextureAssetName;
	bool					mFogEnabled;
	bool					mManualColor;
	bool					mReceiveShadows;
	bool					mDirectionalLighting;
	VuColor					mColor;
	VuVector2				mUV0;
	VuVector2				mUV1;
	float					mDrawDist;

	VuVector4				mBakedColor;

	VuAssetProperty<VuTextureAsset>	*mpTextureAssetProperty;
	VuFoliageManager::VuBucket		*mpBucket;
};
