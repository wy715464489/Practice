//*****************************************************************************
//
//  Copyright (c) 2011-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  Foliage Manager
// 
//*****************************************************************************

#pragma once

#include "VuEngine/VuSystemComponent.h"
#include "VuEngine/Math/VuVector2.h"
#include "VuEngine/Math/VuVector3.h"
#include "VuEngine/Util/VuColor.h"

class VuCamera;
class VuTextureAsset;
class VuColor;
class VuVertexDeclaration;
class VuCompiledShaderAsset;
class VuGfxSortMaterial;


class VuFoliageManager : public VuSystemComponent
{
	DECLARE_SYSTEM_COMPONENT(VuFoliageManager)

protected:
	// called by game
	friend class VuEngine;
	virtual bool init();
	virtual void release();

public:
	VuFoliageManager();

	class VuBucket;

	VuBucket			*createBucket(VuTextureAsset *pTextureAsset, bool fogEnabled);
	void				releaseBucket(VuBucket *pBucket);

	struct DrawParams
	{
		VuVector3	mPos;
		float		mScaleX;
		float		mScaleZ;
		VuColor		mColor;
		VuVector2	mUV0;
		VuVector2	mUV1;
	};

	void				drawLayout(VuTextureAsset *pTextureAsset, bool fogEnabled, const DrawParams &params, const VuCamera &camera);
	void				draw(VuBucket *pBucket, const DrawParams &params, const VuCamera &camera);
	void				draw();

private:
	static void			staticDrawCallback(void *data);
	void				drawCallback(void *data);

	typedef std::list<VuBucket *> Buckets;

	enum eFlavor { FLAVOR_SIMPLE, FLAVOR_FOG, FLAVOR_COUNT };
	class VuFlavor
	{
	public:
		VuFlavor() : mpCompiledShaderAsset(VUNULL), mpMaterial(VUNULL) {}
		VuCompiledShaderAsset	*mpCompiledShaderAsset;
		VuGfxSortMaterial		*mpMaterial;
		Buckets					mBuckets;
	};

	VuFlavor			mFlavors[FLAVOR_COUNT];
};