//*****************************************************************************
//
//  Copyright (c) 2008-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  VuGfxSortMaterial class
// 
//*****************************************************************************

#include "VuGfxSortMaterial.h"
#include "VuEngine/HAL/Gfx/VuGfx.h"
#include "VuEngine/HAL/Gfx/VuShaderProgram.h"
#include "VuEngine/HAL/Gfx/VuVertexDeclaration.h"
#include "VuEngine/HAL/Gfx/VuPipelineState.h"
#include "VuEngine/Math/VuPackedVector.h"
#include "VuEngine/Util/VuColor.h"
#include "VuEngine/Util/VuHash.h"
#include "VuEngine/Util/VuBinaryDataUtil.h"
#include "VuEngine/Assets/VuTextureAsset.h"
#include "VuEngine/Assets/VuCubeTextureAsset.h"
#include "VuEngine/Assets/VuAssetFactory.h"


//*****************************************************************************
static int CompareConstants(const void *p1, const void *p2)
{
	const VuGfxSortMaterialDesc::VuConstantArrayEntry *pEntry1 = static_cast<const VuGfxSortMaterialDesc::VuConstantArrayEntry *>(p1);
	const VuGfxSortMaterialDesc::VuConstantArrayEntry *pEntry2 = static_cast<const VuGfxSortMaterialDesc::VuConstantArrayEntry *>(p2);

	return strcmp(pEntry1->mName, pEntry2->mName);
}

//*****************************************************************************
static int CompareTextures(const void *p1, const void *p2)
{
	const VuGfxSortMaterialDesc::VuTextureArrayEntry *pEntry1 = static_cast<const VuGfxSortMaterialDesc::VuTextureArrayEntry *>(p1);
	const VuGfxSortMaterialDesc::VuTextureArrayEntry *pEntry2 = static_cast<const VuGfxSortMaterialDesc::VuTextureArrayEntry *>(p2);

	return strcmp(pEntry1->mName, pEntry2->mName);
}

//*****************************************************************************
void VuGfxSortMaterialDesc::VuConstantArrayEntry::set(const char *strName, eConstType type, const VuConstValue &value)
{
	VUASSERT(strlen(strName) + 1 < sizeof(mName), "VuGfxSortMaterialDesc::addConstant() constant name too long");

	VU_STRCPY(mName, sizeof(mName), strName);
	mType = type;
	mValue = value;
}

//*****************************************************************************
void VuGfxSortMaterialDesc::VuConstantArray::add(const char *strName, eConstType type, const VuConstValue &value)
{
	for ( int i = 0; i < mCount; i++ )
	{
		if ( strcmp(maConstants[i].mName, strName) == 0 )
		{
			maConstants[i].set(strName, type, value);
			return;
		}
	}

	if ( mCount == MAX_CONSTANT_COUNT )
	{
		VUASSERT(0, "Material Constant exceeds max count.");
		return;
	}

	maConstants[mCount++].set(strName, type, value);

	// sort
	qsort(maConstants, mCount, sizeof(maConstants[0]), CompareConstants);
}

//*****************************************************************************
void VuGfxSortMaterialDesc::VuConstantArray::load(VuBinaryDataReader &reader)
{
	reader.readValue(mCount);
	for ( int i = 0; i < mCount; i++ )
	{
		VU_STRCPY(maConstants[i].mName, sizeof(maConstants[i].mName), reader.readString());
		reader.readValue(maConstants[i].mType);
		reader.readValue(maConstants[i].mValue.mFloat4[0]);
		reader.readValue(maConstants[i].mValue.mFloat4[1]);
		reader.readValue(maConstants[i].mValue.mFloat4[2]);
		reader.readValue(maConstants[i].mValue.mFloat4[3]);
	}
}

//*****************************************************************************
void VuGfxSortMaterialDesc::VuConstantArray::save(VuBinaryDataWriter &writer) const
{
	writer.writeValue(mCount);
	for ( int i = 0; i < mCount; i++ )
	{
		writer.writeString(maConstants[i].mName);
		writer.writeValue(maConstants[i].mType);
		writer.writeValue(maConstants[i].mValue.mFloat4[0]);
		writer.writeValue(maConstants[i].mValue.mFloat4[1]);
		writer.writeValue(maConstants[i].mValue.mFloat4[2]);
		writer.writeValue(maConstants[i].mValue.mFloat4[3]);
	}
}

//*****************************************************************************
VUUINT32 VuGfxSortMaterialDesc::VuConstantArray::calcHash() const
{
	VUUINT32 hash = VU_FNV32_INIT;

	for ( int i = 0; i < mCount; i++ )
	{
		const VuConstantArrayEntry *pConstant = &maConstants[i];

		hash = VuHash::fnv32String(pConstant->mName, hash);
		hash = VuHash::fnv32(&pConstant->mType, sizeof(pConstant->mType), hash);
		hash = VuHash::fnv32(&pConstant->mValue, sizeof(pConstant->mValue), hash);
	}

	return hash;
}

//*****************************************************************************
void VuGfxSortMaterialDesc::VuTextureArray::add(VuTextureArrayEntry &entry)
{
	for ( int i = 0; i < mCount; i++ )
	{
		if ( strcmp(maTextures[i].mName, entry.mName) == 0 )
		{
			maTextures[i] = entry;
			return;
		}
	}

	if ( mCount == MAX_TEXTURE_COUNT )
	{
		VUASSERT(0, "Material Texture exceeds max count.");
		return;
	}

	maTextures[mCount++] = entry;

	// sort
	qsort(maTextures, mCount, sizeof(maTextures[0]), CompareTextures);
}

//*****************************************************************************
void VuGfxSortMaterialDesc::VuTextureArray::load(VuBinaryDataReader &reader)
{
	reader.readValue(mCount);
	for ( int i = 0; i < mCount; i++ )
	{
		VU_STRCPY(maTextures[i].mName, sizeof(maTextures[i].mName), reader.readString());
		reader.readValue(maTextures[i].mType);
		VU_STRCPY(maTextures[i].mAssetName, sizeof(maTextures[i].mAssetName),reader.readString());
	}
}

//*****************************************************************************
void VuGfxSortMaterialDesc::VuTextureArray::save(VuBinaryDataWriter &writer) const
{
	writer.writeValue(mCount);
	for ( int i = 0; i < mCount; i++ )
	{
		writer.writeString(maTextures[i].mName);
		writer.writeValue(maTextures[i].mType);
		writer.writeString(maTextures[i].mAssetName);
	}
}

//*****************************************************************************
VUUINT32 VuGfxSortMaterialDesc::VuTextureArray::calcHash() const
{
	VUUINT32 hash = VU_FNV32_INIT;

	for ( int i = 0; i < mCount; i++ )
	{
		const VuTextureArrayEntry *pConstant = &maTextures[i];

		hash = VuHash::fnv32String(pConstant->mName, hash);
		hash = VuHash::fnv32(&pConstant->mType, sizeof(pConstant->mType), hash);
		hash = VuHash::fnv32String(pConstant->mAssetName, hash);
	}

	return hash;
}

//*****************************************************************************
void VuGfxSortMaterialDesc::addConstantBool(const char *strName, bool bValue)
{
	VuConstValue value;
	memset(&value, 0, sizeof(value));
	value.mInt = bValue;
	addConstant(strName, CONST_INT, value);
}

//*****************************************************************************
void VuGfxSortMaterialDesc::addConstantFloat(const char *strName, float fValue)
{
	VuConstValue value;
	memset(&value, 0, sizeof(value));
	value.mFloat = fValue;
	addConstant(strName, CONST_FLOAT, value);
}

//*****************************************************************************
void VuGfxSortMaterialDesc::addConstantVector3(const char *strName, const VuPackedVector3 &vec)
{
	VuConstValue value;
	memset(&value, 0, sizeof(value));
	value.mFloat3[0] = vec.mX;
	value.mFloat3[1] = vec.mY;
	value.mFloat3[2] = vec.mZ;
	addConstant(strName, CONST_FLOAT3, value);
}

//*****************************************************************************
void VuGfxSortMaterialDesc::addConstantVector4(const char *strName, const VuPackedVector4 &vec)
{
	VuConstValue value;
	memset(&value, 0, sizeof(value));
	value.mFloat4[0] = vec.mX;
	value.mFloat4[1] = vec.mY;
	value.mFloat4[2] = vec.mZ;
	value.mFloat4[3] = vec.mW;
	addConstant(strName, CONST_FLOAT4, value);
}

//*****************************************************************************
void VuGfxSortMaterialDesc::addConstantColor3(const char *strName, const VuColor &color)
{
	VuConstValue value;
	memset(&value, 0, sizeof(value));
	color.toFloat3(value.mFloat3[0], value.mFloat3[1], value.mFloat3[2]);
	addConstant(strName, CONST_FLOAT3, value);
}

//*****************************************************************************
void VuGfxSortMaterialDesc::addConstantColor4(const char *strName, const VuColor &color)
{
	VuConstValue value;
	memset(&value, 0, sizeof(value));
	color.toFloat4(value.mFloat4[0], value.mFloat4[1], value.mFloat4[2], value.mFloat4[3]);
	addConstant(strName, CONST_FLOAT4, value);
}

//*****************************************************************************
void VuGfxSortMaterialDesc::addConstant(const char *strName, eConstType type, const VuConstValue &value)
{
	mConstantArray.add(strName, type, value);
}

//*****************************************************************************
void VuGfxSortMaterialDesc::addTexture(const char *strName, eTextureType type, const char *strAssetName)
{
	VuTextureArrayEntry entry;

	VUASSERT(strlen(strName) + 1 < sizeof(entry.mName), "VuGfxSortMaterialDesc::addTexture() texture name too long");
	VUASSERT(strlen(strAssetName) + 1 < sizeof(entry.mAssetName), "VuGfxSortMaterialDesc::addTexture() asset name too long");

	memset(&entry, 0, sizeof(entry));
	VU_STRNCPY(entry.mName, sizeof(entry.mName), strName, sizeof(entry.mName) - 1);
	entry.mType = type;
	VU_STRNCPY(entry.mAssetName, sizeof(entry.mAssetName), strAssetName, sizeof(entry.mAssetName) - 1);

	mTextureArray.add(entry);
}

//*****************************************************************************
const VuGfxSortMaterialDesc::VuTextureArrayEntry *VuGfxSortMaterialDesc::getTextureEntry(const char *strName) const
{
	const VuTextureArrayEntry *pEntry = mTextureArray.maTextures;
	for ( int i = 0; i < mTextureArray.mCount; i++, pEntry++ )
		if ( strncmp(pEntry->mName, strName, sizeof(pEntry->mName)) == 0 )
			return pEntry;

	return VUNULL;
}

//*****************************************************************************
void VuGfxSortMaterialDesc::loadParams(VuBinaryDataReader &reader)
{
	mConstantArray.load(reader);
	mTextureArray.load(reader);
}

//*****************************************************************************
void VuGfxSortMaterialDesc::saveParams(VuBinaryDataWriter &writer)
{
	mConstantArray.save(writer);
	mTextureArray.save(writer);
}

//*****************************************************************************
VuGfxSortMaterial::VuGfxSortMaterial(VuPipelineState *pPipelineState, const VuGfxSortMaterialDesc &desc):
	mSortKey(0),
	mConstHash(0),
	mTexHash(0),
	mpMaterialExtension(VUNULL),
	mRefCount(1)
{
	mpPipelineState = pPipelineState;
	mpPipelineState->addRef();

	mpShaderProgram = mpPipelineState->mpShaderProgram; // no need to retain, pipeline state already retains a reference

	// handle constants
	for ( int i = 0; i < desc.mConstantArray.mCount; i++ )
	{
		const VuGfxSortMaterialDesc::VuConstantArrayEntry &srcEntry = desc.mConstantArray.maConstants[i];

		VUHANDLE handle = mpShaderProgram->getConstantByName(srcEntry.mName);
		if ( handle )
		{
			VuGfxSortMaterialDesc::VuConstantArrayEntry &dstEntry = mDesc.mConstantArray.maConstants[mDesc.mConstantArray.mCount];

			dstEntry = srcEntry;
			dstEntry.mHandle = handle;

			mDesc.mConstantArray.mCount++;
		}
	}

	// handle textures
	for ( int i = 0; i < desc.mTextureArray.mCount; i++ )
	{
		const VuGfxSortMaterialDesc::VuTextureArrayEntry &srcEntry = desc.mTextureArray.maTextures[i];

		int sampler = mpShaderProgram->getSamplerIndexByName(srcEntry.mName);
		if ( sampler >= 0 )
		{
			VuGfxSortMaterialDesc::VuTextureArrayEntry &dstEntry = mDesc.mTextureArray.maTextures[mDesc.mTextureArray.mCount];

			dstEntry = srcEntry;
			dstEntry.mSampler = sampler;

			if ( dstEntry.mType == VuGfxSortMaterialDesc::TEXTURE )
				mpTextureAssets[mDesc.mTextureArray.mCount] = VuAssetFactory::IF()->createAsset<VuTextureAsset>(dstEntry.mAssetName);
			else if ( dstEntry.mType == VuGfxSortMaterialDesc::CUBE_TEXTURE )
				mpTextureAssets[mDesc.mTextureArray.mCount] = VuAssetFactory::IF()->createAsset<VuCubeTextureAsset>(dstEntry.mAssetName);

			mDesc.mTextureArray.mCount++;
		}
	}

	// get shader constants

	// camera
	mhSpConstViewMatrix				= mpShaderProgram->getConstantByName("gViewMatrix");
	mhSpConstViewProjMatrix			= mpShaderProgram->getConstantByName("gViewProjMatrix");
	mhSpConstEyeWorld				= mpShaderProgram->getConstantByName("gEyeWorld");
	mhSpConstFarPlane				= mpShaderProgram->getConstantByName("gFarPlane");

	// lights
	mhSpConstAmbLightColor			= mpShaderProgram->getConstantByName("gAmbLightColor");
	mhSpConstDirLightWorld			= mpShaderProgram->getConstantByName("gDirLightWorld");
	mhSpConstDirLightFrontColor		= mpShaderProgram->getConstantByName("gDirLightFrontColor");
	mhSpConstDirLightBackColor		= mpShaderProgram->getConstantByName("gDirLightBackColor");
	mhSpConstDirLightSpecularColor	= mpShaderProgram->getConstantByName("gDirLightSpecularColor");

	// fog
	mhSpConstFogStart				= mpShaderProgram->getConstantByName("gFogStart");
	mhSpConstFogInvRange			= mpShaderProgram->getConstantByName("gFogInvRange");
	mhSpConstFogColor				= mpShaderProgram->getConstantByName("gFogColor");
	mhPsConstDepthFogStart			= mpShaderProgram->getConstantByName("gDepthFogStart");
	mhPsConstDepthFogInvRange		= mpShaderProgram->getConstantByName("gDepthFogInvRange");
	mhPsConstDepthFogColor			= mpShaderProgram->getConstantByName("gDepthFogColor");

	// misc
	mhSpConstTime					= mpShaderProgram->getConstantByName("gTime");

	// clip
	mhSpConstClipPlane				= mpShaderProgram->getConstantByName("gClipPlane");
}

//*****************************************************************************
VuGfxSortMaterial::~VuGfxSortMaterial()
{
	// remove member refs
	mpPipelineState->removeRef();
	for ( int i = 0; i < mDesc.mTextureArray.mCount; i++ )
		VuAssetFactory::IF()->releaseAsset(mpTextureAssets[i]);

	delete mpMaterialExtension;
}

//*****************************************************************************
void VuGfxSortMaterial::setConstants() const
{
	const VuGfxSortMaterialDesc::VuConstantArrayEntry *pConstant = mDesc.mConstantArray.maConstants;
	for ( int i = 0; i < mDesc.mConstantArray.mCount; i++ )
	{
		if ( pConstant->mType == VuGfxSortMaterialDesc::CONST_INT )
			mpShaderProgram->setConstantInt(pConstant->mHandle, pConstant->mValue.mInt);
		else if ( pConstant->mType == VuGfxSortMaterialDesc::CONST_FLOAT )
			mpShaderProgram->setConstantFloat(pConstant->mHandle, pConstant->mValue.mFloat);
		else if ( pConstant->mType == VuGfxSortMaterialDesc::CONST_FLOAT3 )
			mpShaderProgram->setConstantFloat3(pConstant->mHandle, pConstant->mValue.mFloat3);
		else if ( pConstant->mType == VuGfxSortMaterialDesc::CONST_FLOAT4 )
			mpShaderProgram->setConstantFloat4(pConstant->mHandle, pConstant->mValue.mFloat4);
		pConstant++;
	}
}

//*****************************************************************************
void VuGfxSortMaterial::setTextures() const
{
	const VuGfxSortMaterialDesc::VuTextureArrayEntry *pEntry = mDesc.mTextureArray.maTextures;
	for ( int i = 0; i < mDesc.mTextureArray.mCount; i++ )
	{
		VuGfx::IF()->setTexture(pEntry->mSampler, mpTextureAssets[i]->getBaseTexture());

		pEntry++;
	}
}

//*****************************************************************************
void VuGfxSortMaterial::use() const
{
	VuGfx::IF()->setPipelineState(mpPipelineState);

	setConstants();
	setTextures();
}