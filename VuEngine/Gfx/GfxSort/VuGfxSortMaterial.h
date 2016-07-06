//*****************************************************************************
//
//  Copyright (c) 2008-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  VuGfxSortMaterial class
// 
//*****************************************************************************

#pragma once

#include "VuEngine/Containers/VuArray.h"
#include "VuEngine/HAL/Gfx/VuGfxTypes.h"

class VuPipelineState;
class VuShaderProgram;
class VuPackedVector3;
class VuPackedVector4;
class VuColor;
class VuBaseTextureAsset;
class VuBinaryDataReader;
class VuBinaryDataWriter;


class VuGfxSortMaterialDesc
{
public:
	VuGfxSortMaterialDesc() {}

	enum { MAX_CONSTANT_COUNT = 16 };
	enum { MAX_TEXTURE_COUNT = 8 };
	enum eTextureType { TEXTURE, CUBE_TEXTURE };
	enum eConstType { CONST_INT, CONST_FLOAT, CONST_FLOAT3, CONST_FLOAT4 };

	union VuConstValue
	{
		int		mInt;
		float	mFloat;
		float	mFloat3[4];
		float	mFloat4[4];
	};

	struct VuConstantArrayEntry
	{
		VuConstantArrayEntry() : mHandle(0) {}
		void			set(const char *strName, eConstType type, const VuConstValue &value);
		char			mName[32];
		VUHANDLE		mHandle;
		eConstType		mType;
		VuConstValue	mValue;
	};

	struct VuConstantArray
	{
		VuConstantArray() : mCount(0) {}
		void					add(const char *strName, eConstType type, const VuConstValue &value);
		void					load(VuBinaryDataReader &reader);
		void					save(VuBinaryDataWriter &writer) const;
		VUUINT32				calcHash() const;
		VuConstantArrayEntry	maConstants[MAX_CONSTANT_COUNT];
		int						mCount;
	};

	struct VuTextureArrayEntry
	{
		VuTextureArrayEntry() : mSampler(-1) {}
		char					mName[32];
		int						mSampler;
		eTextureType			mType;
		char					mAssetName[64];
	};
	struct VuTextureArray
	{
		VuTextureArray() : mCount(0) {}
		void				add(VuTextureArrayEntry &entry);
		void				load(VuBinaryDataReader &reader);
		void				save(VuBinaryDataWriter &writer) const;
		VUUINT32			calcHash() const;
		VuTextureArrayEntry	maTextures[MAX_TEXTURE_COUNT];
		int					mCount;
	};

	void			addConstantBool(const char *strName, bool bValue);
	void			addConstantFloat(const char *strName, float fValue);
	void			addConstantVector3(const char *strName, const VuPackedVector3 &vec);
	void			addConstantVector4(const char *strName, const VuPackedVector4 &vec);
	void			addConstantColor3(const char *strName, const VuColor &color);
	void			addConstantColor4(const char *strName, const VuColor &color);
	void			addConstant(const char *strName, eConstType type, const VuConstValue &value);

	void			addTexture(const char *strName, eTextureType type, const char *strAssetName);

	const VuTextureArrayEntry	*getTextureEntry(const char *strName) const;

	void			loadParams(VuBinaryDataReader &reader);
	void			saveParams(VuBinaryDataWriter &writer);

	VuConstantArray	mConstantArray;
	VuTextureArray	mTextureArray;
};

class VuGfxSortMatExt
{
};

class VuGfxSortMaterial
{
protected:
	friend class VuGfxSort;
	VuGfxSortMaterial(VuPipelineState *pPipelineState, const VuGfxSortMaterialDesc &desc);
	~VuGfxSortMaterial();

	void					addRef()	{ mRefCount++; }
	void					removeRef()	{ mRefCount--; }
	VUINT32					refCount()	{ return mRefCount; }

public:
	// set material shader constants
	void					setConstants() const;
	void					setTextures() const;

	// set everything (only used by tools for now)
	void					use() const;

	VuGfxSortMaterialDesc	mDesc;
	VUUINT32				mSortKey;
	VUUINT32				mConstHash;
	VUUINT32				mTexHash;

	// resources
	VuPipelineState			*mpPipelineState;
	VuShaderProgram			*mpShaderProgram;
	VuBaseTextureAsset		*mpTextureAssets[VuGfxSortMaterialDesc::MAX_TEXTURE_COUNT];

	// optimization

	// camera
	VUHANDLE				mhSpConstViewMatrix;
	VUHANDLE				mhSpConstViewProjMatrix;
	VUHANDLE				mhSpConstEyeWorld;
	VUHANDLE				mhSpConstFarPlane;

	// lights
	VUHANDLE				mhSpConstAmbLightColor;
	VUHANDLE				mhSpConstDirLightWorld;
	VUHANDLE				mhSpConstDirLightFrontColor;
	VUHANDLE				mhSpConstDirLightBackColor;
	VUHANDLE				mhSpConstDirLightSpecularColor;

	// fog
	VUHANDLE				mhSpConstFogStart;
	VUHANDLE				mhSpConstFogInvRange;
	VUHANDLE				mhSpConstFogColor;
	VUHANDLE				mhPsConstDepthFogStart;
	VUHANDLE				mhPsConstDepthFogInvRange;
	VUHANDLE				mhPsConstDepthFogColor;

	// misc
	VUHANDLE				mhSpConstTime;

	// clip
	VUHANDLE				mhSpConstClipPlane;

	// shadow


	// game-specific
	VuGfxSortMatExt			*mpMaterialExtension;

	// debug
#ifndef VURETAIL
	std::string				mDebugName;
#endif

private:
	int						mRefCount;
};
