//*****************************************************************************
//
//  Copyright (c) 2007-2008 Ralf Knoesel
//  Copyright (c) 2008-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  Extended VuJsonContainer functionality.
// 
//*****************************************************************************

#include <ctype.h>
#include "VuDataUtil.h"
#include "VuEngine/Json/VuJsonContainer.h"
#include "VuEngine/Entities/VuEntityUtil.h"
#include "VuEngine/Math/VuVector2.h"
#include "VuEngine/Math/VuVector3.h"
#include "VuEngine/Math/VuVector4.h"
#include "VuEngine/Math/VuPackedVector.h"
#include "VuEngine/Math/VuMatrix.h"
#include "VuEngine/Math/VuQuaternion.h"
#include "VuEngine/Math/VuRect.h"
#include "VuEngine/Math/VuAabb.h"
#include "VuEngine/Util/VuColor.h"
#include "VuEngine/Util/VuBase64.h"
#include "VuEngine/Util/VuStringFormat.h"
#include "VuEngine/Util/VuHash.h"
#include "VuEngine/Util/VuEndianUtil.h"
#include "VuEngine/Gfx/Font/VuFontDrawParams.h"
#include "VuEngine/Animation/VuAnimationTransform.h"


// static variables

static const char *sTextureAddressModeLookup[] =
{
	"WRAP",		// VUGFX_ADDRESS_WRAP,
	"CLAMP",	// VUGFX_ADDRESS_CLAMP,
};
VU_COMPILE_TIME_ASSERT(sizeof(sTextureAddressModeLookup)/sizeof(sTextureAddressModeLookup[0]) == VUGFX_TEXTURE_ADDRESS_COUNT);

static const char *sTextureFilterTypeLookup[] =
{
	"NONE",			// VUGFX_TEXF_NONE,
	"POINT",		// VUGFX_TEXF_POINT,
	"LINEAR",		// VUGFX_TEXF_LINEAR,
	"ANISOTROPIC",	// VUGFX_TEXF_ANISOTROPIC,
};
VU_COMPILE_TIME_ASSERT(sizeof(sTextureFilterTypeLookup)/sizeof(sTextureFilterTypeLookup[0]) == VUGFX_TEXTURE_FILTER_TYPE_COUNT);

static const char *sTextureTypeLookup[] =
{
	"DEFAULT",		// VUGFX_TEXTURE_TYPE_DEFAULT,
	"BUMP",			// VUGFX_TEXTURE_TYPE_BUMP,
	"SDF",			// VUGFX_TEXTURE_TYPE_SDF,
};
VU_COMPILE_TIME_ASSERT(sizeof(sTextureTypeLookup)/sizeof(sTextureTypeLookup[0]) == VUGFX_TEXTURE_TYPE_COUNT);

static const char *sFormatDXLookup[] =
{
	"32BIT",	// VUGFX_FORMAT_DX_32BIT,
	"S3TC",		// VUGFX_FORMAT_DX_S3TC,
};
VU_COMPILE_TIME_ASSERT(sizeof(sFormatDXLookup)/sizeof(sFormatDXLookup[0]) == VUGFX_FORMAT_DX_COUNT);

static const char *sFormatIOSLookup[] =
{
	"32BIT",		// VUGFX_FORMAT_IOS_32BIT,
	"S3TC",			// VUGFX_FORMAT_IOS_S3TC,
	"PVRTC",		// VUGFX_FORMAT_IOS_PVRTC,
};
VU_COMPILE_TIME_ASSERT(sizeof(sFormatIOSLookup)/sizeof(sFormatIOSLookup[0]) == VUGFX_FORMAT_IOS_COUNT);

static const char *sFormatOGLESLookup[] =
{
	"32BIT",		// VUGFX_FORMAT_OGLES_32BIT,
	"ETC1/DXT5",	// VUGFX_FORMAT_OGLES_ETC1_DXT5,
};
VU_COMPILE_TIME_ASSERT(sizeof(sFormatOGLESLookup)/sizeof(sFormatOGLESLookup[0]) == VUGFX_FORMAT_OGLES_COUNT);

static const char *sEtcQualityLookup[] =
{
	"LOW",		// VuEtc::QUALITY_LOW,
	"MEDIUM",	// VuEtc::QUALITY_MEDIUM,
	"HIGH",		// VuEtc::QUALITY_HIGH,
};
VU_COMPILE_TIME_ASSERT(sizeof(sEtcQualityLookup)/sizeof(sEtcQualityLookup[0]) == VuEtc::QUALITY_COUNT);



//*****************************************************************************
// get value
//*****************************************************************************

//*****************************************************************************
bool VuDataUtil::getValue(const VuJsonContainer &container, int &iVal)
{
	return container.getValue(iVal);
}

//*****************************************************************************
bool VuDataUtil::getValue(const VuJsonContainer &container, float &fVal)
{
	return container.getValue(fVal);
}

//*****************************************************************************
bool VuDataUtil::getValue(const VuJsonContainer &container, bool &bVal)
{
	return container.getValue(bVal);
}

//*****************************************************************************
bool VuDataUtil::getValue(const VuJsonContainer &container, std::string &strVal)
{
	return container.getValue(strVal);
}

//*****************************************************************************
bool VuDataUtil::getValue(const VuJsonContainer &container, VUINT64 &i64Val)
{
	return container.getValue(i64Val);
}

//*****************************************************************************
bool VuDataUtil::getValue(const VuJsonContainer &container, VuVector2 &vVal)
{
	bool all = true;

	all &= getValue(container["X"], vVal.mX);
	all &= getValue(container["Y"], vVal.mY);

	return all;
}

//*****************************************************************************
bool VuDataUtil::getValue(const VuJsonContainer &container, VuVector3 &vVal)
{
	bool all = true;

	all &= getValue(container["X"], vVal.mX);
	all &= getValue(container["Y"], vVal.mY);
	all &= getValue(container["Z"], vVal.mZ);

	return all;
}

//*****************************************************************************
bool VuDataUtil::getValue(const VuJsonContainer &container, VuVector4 &vVal)
{
	bool all = true;

	all &= getValue(container["X"], vVal.mX);
	all &= getValue(container["Y"], vVal.mY);
	all &= getValue(container["Z"], vVal.mZ);
	all &= getValue(container["W"], vVal.mW);

	return all;
}

//*****************************************************************************
bool VuDataUtil::getValue(const VuJsonContainer &container, VuPackedVector2 &vVal)
{
	bool all = true;

	all &= getValue(container["X"], vVal.mX);
	all &= getValue(container["Y"], vVal.mY);

	return all;
}

//*****************************************************************************
bool VuDataUtil::getValue(const VuJsonContainer &container, VuPackedVector3 &vVal)
{
	bool all = true;

	all &= getValue(container["X"], vVal.mX);
	all &= getValue(container["Y"], vVal.mY);
	all &= getValue(container["Z"], vVal.mZ);

	return all;
}

//*****************************************************************************
bool VuDataUtil::getValue(const VuJsonContainer &container, VuPackedVector4 &vVal)
{
	bool all = true;

	all &= getValue(container["X"], vVal.mX);
	all &= getValue(container["Y"], vVal.mY);
	all &= getValue(container["Z"], vVal.mZ);
	all &= getValue(container["W"], vVal.mW);

	return all;
}

//*****************************************************************************
bool VuDataUtil::getValue(const VuJsonContainer &container, VuMatrix &mat)
{
	bool all = true;

	all &= getValue(container["X"], mat.mX);
	all &= getValue(container["Y"], mat.mY);
	all &= getValue(container["Z"], mat.mZ);
	all &= getValue(container["T"], mat.mT);

	return all;
}

//*****************************************************************************
bool VuDataUtil::getValue(const VuJsonContainer &container, VuQuaternion &quat)
{
	return getValue(container, quat.mVec);
}

//*****************************************************************************
bool VuDataUtil::getValue(const VuJsonContainer &container, VuAnimationTransform &animTransform)
{
	bool all = true;

	all &= getValue(container["T"], animTransform.mTranslation);
	all &= getValue(container["R"], animTransform.mRotation);
	all &= getValue(container["S"], animTransform.mScale);

	return all;
}

//*****************************************************************************
bool VuDataUtil::getValue(const VuJsonContainer &container, VuColor &cVal)
{
	bool rgb = true;

	int r = cVal.mR, g = cVal.mG, b = cVal.mB;
	rgb &= getValue(container["R"], r);
	rgb &= getValue(container["G"], g);
	rgb &= getValue(container["B"], b);

	int a = 255;
	getValue(container["A"], a);

	cVal = VuColor((VUUINT8)r, (VUUINT8)g, (VUUINT8)b, (VUUINT8)a);

	return rgb;
}

//*****************************************************************************
bool VuDataUtil::getValue(const VuJsonContainer &container, VuRect &rect)
{
	bool all = true;

	all &= getValue(container["X"], rect.mX);
	all &= getValue(container["Y"], rect.mY);
	all &= getValue(container["W"], rect.mWidth);
	all &= getValue(container["H"], rect.mHeight);

	return all;
}

//*****************************************************************************
bool VuDataUtil::getValue(const VuJsonContainer &container, VuAabb &aabb)
{
	bool all = true;

	all &= getValue(container["Min"], aabb.mMin);
	all &= getValue(container["Max"], aabb.mMax);

	return all;
}

//*****************************************************************************
bool VuDataUtil::getValue(const VuJsonContainer &container, VuArray<VUBYTE> &bytes)
{
	if ( container["Base64"].getType() == VuJsonContainer::stringValue )
	{
		return VuBase64::decode(container["Base64"].asCString(), bytes);
	}
	return false;
}

//*****************************************************************************
bool VuDataUtil::getValue(const VuJsonContainer &container, VuRetVal::eType &retValType)
{
	return VuRetVal::stringToType(container.asCString(), retValType);
}

//*****************************************************************************
bool VuDataUtil::getValue(const VuJsonContainer &container, VuParams::eType &paramType)
{
	return VuParams::stringToType(container.asCString(), paramType);
}

//*****************************************************************************
bool VuDataUtil::getValue(const VuJsonContainer &container, VuParamDecl &paramDecl)
{
	paramDecl.mNumParams = VuMin(container.size(), VuParamDecl::MAX_NUM_PARAMS);
	for ( int i = 0; i < paramDecl.mNumParams; i++ )
		if ( !getValue(container[i], paramDecl.maParamTypes[i]) )
			return false;
	return true;
}

//*****************************************************************************
bool VuDataUtil::getValue(const VuJsonContainer &container, VuStringFormat &stringFormat)
{
	bool all = true;

	all &= getValue(container["AlignH"], stringFormat.mAlignH);
	all &= getValue(container["AlignV"], stringFormat.mAlignV);
	all &= getValue(container["Clip"], stringFormat.mClip);
	all &= getValue(container["Wordbreak"], stringFormat.mWordbreak);

	return all;
}

//*****************************************************************************
bool VuDataUtil::getValue(const VuJsonContainer &container, VuFontDrawParams &params)
{
	bool all = true;

	all &= getValue(container["Size"], params.mSize);
	all &= getValue(container["Weight"], params.mWeight);
	all &= getValue(container["Softness"], params.mSoftness);
	all &= getValue(container["Color"], params.mColor);
	all &= getValue(container["OutlineWeight"], params.mOutlineWeight);
	all &= getValue(container["OutlineSoftness"], params.mOutlineSoftness);
	all &= getValue(container["OutlineColor"], params.mOutlineColor);
	all &= getValue(container["Slant"], params.mSlant);
	all &= getValue(container["TabSize"], params.mTabSize);
	all &= getValue(container["Stretch"], params.mStretch);

	return all;
}

//*****************************************************************************
bool VuDataUtil::getValue(const VuJsonContainer &container, VuGfxTextureAddress &mode)
{
	std::string strMode;
	if ( getValue(container, strMode) )
	{
		for ( int i = 0; i < VUGFX_TEXTURE_ADDRESS_COUNT; i++ )
		{
			if ( strMode == sTextureAddressModeLookup[i] )
			{
				mode = static_cast<VuGfxTextureAddress>(i);
				return true;
			}
		}
	}

	return false;
}

//*****************************************************************************
bool VuDataUtil::getValue(const VuJsonContainer &container, VuGfxTextureFilterType &type)
{
	std::string strType;
	if ( getValue(container, strType) )
	{
		for ( int i = 0; i < VUGFX_TEXTURE_FILTER_TYPE_COUNT; i++ )
		{
			if ( strType == sTextureFilterTypeLookup[i] )
			{
				type = static_cast<VuGfxTextureFilterType>(i);
				return true;
			}
		}
	}

	return false;
}

//*****************************************************************************
bool VuDataUtil::getValue(const VuJsonContainer &container, VuGfxTextureType &type)
{
	std::string strType;
	if ( getValue(container, strType) )
	{
		for ( int i = 0; i < VUGFX_TEXTURE_TYPE_COUNT; i++ )
		{
			if ( strType == sTextureTypeLookup[i] )
			{
				type = static_cast<VuGfxTextureType>(i);
				return true;
			}
		}
	}

	return false;
}

//*****************************************************************************
bool VuDataUtil::getValue(const VuJsonContainer &container, VuGfxFormatDX &format)
{
	std::string strType;
	if ( getValue(container, strType) )
	{
		for ( int i = 0; i < VUGFX_FORMAT_DX_COUNT; i++ )
		{
			if ( strType == sFormatDXLookup[i] )
			{
				format = static_cast<VuGfxFormatDX>(i);
				return true;
			}
		}
	}

	return false;
}

//*****************************************************************************
bool VuDataUtil::getValue(const VuJsonContainer &container, VuGfxFormatIOS &format)
{
	std::string strType;
	if ( getValue(container, strType) )
	{
		for ( int i = 0; i < VUGFX_FORMAT_IOS_COUNT; i++ )
		{
			if ( strType == sFormatIOSLookup[i] )
			{
				format = static_cast<VuGfxFormatIOS>(i);
				return true;
			}
		}
	}

	return false;
}

//*****************************************************************************
bool VuDataUtil::getValue(const VuJsonContainer &container, VuGfxFormatOGLES &format)
{
	std::string strType;
	if ( getValue(container, strType) )
	{
		for ( int i = 0; i < VUGFX_FORMAT_OGLES_COUNT; i++ )
		{
			if ( strType == sFormatOGLESLookup[i] )
			{
				format = static_cast<VuGfxFormatOGLES>(i);
				return true;
			}
		}
	}

	return false;
}

//*****************************************************************************
bool VuDataUtil::getValue(const VuJsonContainer &container, VuEtc::eQuality &etcQuality)
{
	std::string strType;
	if ( getValue(container, strType) )
	{
		for ( int i = 0; i < VuEtc::QUALITY_COUNT; i++ )
		{
			if ( strType == sEtcQualityLookup[i] )
			{
				etcQuality = static_cast<VuEtc::eQuality>(i);
				return true;
			}
		}
	}

	return false;
}

//*****************************************************************************
bool VuDataUtil::getValue(const VuJsonContainer &container, VuTimeUtil::VuTimeStruct &ts)
{
	bool all = true;

	all &= getValue(container["Year"], ts.mYear);
	all &= getValue(container["Month"], ts.mMonth);
	all &= getValue(container["Day"], ts.mDay);
	all &= getValue(container["Hour"], ts.mHour);
	all &= getValue(container["Minute"], ts.mMinute);
	all &= getValue(container["Second"], ts.mSecond);

	return all;
}


//*****************************************************************************
// put value
//*****************************************************************************

//*****************************************************************************
void VuDataUtil::putValue(VuJsonContainer &container, int iVal)
{
	container.putValue(iVal);
}

//*****************************************************************************
void VuDataUtil::putValue(VuJsonContainer &container, float fVal)
{
	container.putValue(fVal);
}

//*****************************************************************************
void VuDataUtil::putValue(VuJsonContainer &container, bool bVal)
{
	container.putValue(bVal);
}

//*****************************************************************************
void VuDataUtil::putValue(VuJsonContainer &container, const char *strVal)
{
	container.putValue(strVal);
}

//*****************************************************************************
void VuDataUtil::putValue(VuJsonContainer &container, const std::string &strVal)
{
	container.putValue(strVal);
}

//*****************************************************************************
void VuDataUtil::putValue(VuJsonContainer &container, const VUINT64 &i64Val)
{
	container.putValue(i64Val);
}

//*****************************************************************************
void VuDataUtil::putValue(VuJsonContainer &container, const VuVector2 &vVal)
{
	putValue(container["X"], vVal.mX);
	putValue(container["Y"], vVal.mY);
}

//*****************************************************************************
void VuDataUtil::putValue(VuJsonContainer &container, const VuVector3 &vVal)
{
	putValue(container["X"], vVal.mX);
	putValue(container["Y"], vVal.mY);
	putValue(container["Z"], vVal.mZ);
}

//*****************************************************************************
void VuDataUtil::putValue(VuJsonContainer &container, const VuVector4 &vVal)
{
	putValue(container["X"], vVal.mX);
	putValue(container["Y"], vVal.mY);
	putValue(container["Z"], vVal.mZ);
	putValue(container["W"], vVal.mW);
}

//*****************************************************************************
void VuDataUtil::putValue(VuJsonContainer &container, const VuPackedVector2 &vVal)
{
	putValue(container["X"], vVal.mX);
	putValue(container["Y"], vVal.mY);
}

//*****************************************************************************
void VuDataUtil::putValue(VuJsonContainer &container, const VuPackedVector3 &vVal)
{
	putValue(container["X"], vVal.mX);
	putValue(container["Y"], vVal.mY);
	putValue(container["Z"], vVal.mZ);
}

//*****************************************************************************
void VuDataUtil::putValue(VuJsonContainer &container, const VuPackedVector4 &vVal)
{
	putValue(container["X"], vVal.mX);
	putValue(container["Y"], vVal.mY);
	putValue(container["Z"], vVal.mZ);
	putValue(container["W"], vVal.mW);
}

//*****************************************************************************
void VuDataUtil::putValue(VuJsonContainer &container, const VuMatrix &mat)
{
	putValue(container["X"], mat.mX);
	putValue(container["Y"], mat.mY);
	putValue(container["Z"], mat.mZ);
	putValue(container["T"], mat.mT);
}

//*****************************************************************************
void VuDataUtil::putValue(VuJsonContainer &container, const VuQuaternion &quat)
{
	putValue(container, quat.mVec);
}

//*****************************************************************************
void VuDataUtil::putValue(VuJsonContainer &container, const VuAnimationTransform &animTransform)
{
	putValue(container["T"], animTransform.mTranslation);
	putValue(container["R"], animTransform.mRotation);
	putValue(container["S"], animTransform.mScale);
}

//*****************************************************************************
void VuDataUtil::putValue(VuJsonContainer &container, const VuColor &cVal)
{
	putValue(container["R"], (int)cVal.mR);
	putValue(container["G"], (int)cVal.mG);
	putValue(container["B"], (int)cVal.mB);
	putValue(container["A"], (int)cVal.mA);
}

//*****************************************************************************
void VuDataUtil::putValue(VuJsonContainer &container, const VuRect &rect)
{
	putValue(container["X"], rect.mX);
	putValue(container["Y"], rect.mY);
	putValue(container["W"], rect.mWidth);
	putValue(container["H"], rect.mHeight);
}

//*****************************************************************************
void VuDataUtil::putValue(VuJsonContainer &container, const VuAabb &aabb)
{
	putValue(container["Min"], aabb.mMin);
	putValue(container["Max"], aabb.mMax);
}

//*****************************************************************************
void VuDataUtil::putValue(VuJsonContainer &container, const VuArray<VUBYTE> &bytes)
{
	std::string str;
	VuBase64::encode(bytes, str);

	putValue(container["Base64"], str);
}

//*****************************************************************************
void VuDataUtil::putValue(VuJsonContainer &container, const VuRetVal::eType &retValType)
{
	container.putValue(VuRetVal::typeToString(retValType));
}

//*****************************************************************************
void VuDataUtil::putValue(VuJsonContainer &container, const VuParams::eType &paramType)
{
	container.putValue(VuParams::typeToString(paramType));
}

//*****************************************************************************
void VuDataUtil::putValue(VuJsonContainer &container, const VuParamDecl &paramDecl)
{
	for ( int i = 0; i < paramDecl.mNumParams; i++ )
		putValue(container[i], paramDecl.maParamTypes[i]);
}

//*****************************************************************************
void VuDataUtil::putValue(VuJsonContainer &container, const VuStringFormat &stringFormat)
{
	putValue(container["AlignH"], stringFormat.mAlignH);
	putValue(container["AlignV"], stringFormat.mAlignV);
	putValue(container["Clip"], stringFormat.mClip);
	putValue(container["Wordbreak"], stringFormat.mWordbreak);
}

//*****************************************************************************
void VuDataUtil::putValue(VuJsonContainer &container, const VuFontDrawParams &params)
{
	putValue(container["Size"], params.mSize);
	putValue(container["Weight"], params.mWeight);
	putValue(container["Softness"], params.mSoftness);
	putValue(container["Color"], params.mColor);
	putValue(container["OutlineWeight"], params.mOutlineWeight);
	putValue(container["OutlineSoftness"], params.mOutlineSoftness);
	putValue(container["OutlineColor"], params.mOutlineColor);
	putValue(container["Slant"], params.mSlant);
	putValue(container["TabSize"], params.mTabSize);
	putValue(container["Stretch"], params.mStretch);
}

//*****************************************************************************
void VuDataUtil::putValue(VuJsonContainer &container, const VuGfxTextureAddress &mode)
{
	putValue(container, sTextureAddressModeLookup[mode]);
}

//*****************************************************************************
void VuDataUtil::putValue(VuJsonContainer &container, const VuGfxTextureFilterType &type)
{
	putValue(container, sTextureFilterTypeLookup[type]);
}

//*****************************************************************************
void VuDataUtil::putValue(VuJsonContainer &container, const VuGfxTextureType &type)
{
	putValue(container, sTextureTypeLookup[type]);
}

//*****************************************************************************
void VuDataUtil::putValue(VuJsonContainer &container, const VuGfxFormatDX &format)
{
	putValue(container, sFormatDXLookup[format]);
}

//*****************************************************************************
void VuDataUtil::putValue(VuJsonContainer &container, const VuGfxFormatIOS &format)
{
	putValue(container, sFormatIOSLookup[format]);
}

//*****************************************************************************
void VuDataUtil::putValue(VuJsonContainer &container, const VuGfxFormatOGLES &format)
{
	putValue(container, sFormatOGLESLookup[format]);
}

//*****************************************************************************
void VuDataUtil::putValue(VuJsonContainer &container, const VuEtc::eQuality &etcQuality)
{
	putValue(container, sEtcQualityLookup[etcQuality]);
}

//*****************************************************************************
void VuDataUtil::putValue(VuJsonContainer &container, const VuTimeUtil::VuTimeStruct &ts)
{
	putValue(container["Year"], (int)ts.mYear);
	putValue(container["Month"], (int)ts.mMonth);
	putValue(container["Day"], (int)ts.mDay);
	putValue(container["Hour"], (int)ts.mHour);
	putValue(container["Minute"], (int)ts.mMinute);
	putValue(container["Second"], (int)ts.mSecond);
}


//*****************************************************************************
VUUINT32 VuDataUtil::calcHash32(const VuJsonContainer &container, VUUINT32 hash)
{
	VuJsonContainer::eType type = container.getType();
	type = VuEndianUtil::swapIfLittle(type);
	hash = VuHash::fnv32(&type, sizeof(type), hash);

	if ( container.isNull() )
	{
		// nothing to do
	}
	else if ( container.isInt() )
	{
		int value = container.asInt();
		value = VuEndianUtil::swapIfLittle(value);
		hash = VuHash::fnv32(&value, sizeof(value), hash);
	}
	else if ( container.isFloat() )
	{
		float value = container.asFloat();
		value = VuEndianUtil::swapIfLittle(value);
		hash = VuHash::fnv32(&value, sizeof(value), hash);
	}
	else if ( container.isBool() )
	{
		bool value = container.asBool();
		value = VuEndianUtil::swapIfLittle(value);
		hash = VuHash::fnv32(&value, sizeof(value), hash);
	}
	else if ( container.isString() )
	{
		hash = VuHash::fnv32String(container.asCString(), hash);
	}
	else if ( container.isArray() )
	{
		int size = container.size();
		int fixedSize = VuEndianUtil::swapIfLittle(size);
		hash = VuHash::fnv32(&fixedSize, sizeof(fixedSize), hash);

		// recurse
		for ( int i = 0; i < size; i++ )
			hash = calcHash32(container[i], hash);
	}
	else if ( container.isObject() )
	{
		int count = container.numMembers();
		int fixedCount = VuEndianUtil::swapIfLittle(count);
		hash = VuHash::fnv32(&fixedCount, sizeof(fixedCount), hash);

		// recurse
		for ( int i = 0; i < count; i++ )
		{
			const std::string &key = container.getMemberKey(i);
			hash = VuHash::fnv32String(key.c_str(), hash);
			hash = calcHash32(container[key], hash);
		}
	}
	else if ( container.isInt64() )
	{
		VUINT64 value = container.asInt64();
		value = VuEndianUtil::swapIfLittle(value);
		hash = VuHash::fnv32(&value, sizeof(value), hash);
	}
	else
	{
		VUASSERT(0, "VuDataUtil::calcHash() unsupported type");
	}

	return hash;
}

//*****************************************************************************
VUUINT64 VuDataUtil::calcHash64(const VuJsonContainer &container, VUUINT64 hash)
{
	VuJsonContainer::eType type = container.getType();
	type = VuEndianUtil::swapIfLittle(type);
	hash = VuHash::fnv64(&type, sizeof(type), hash);

	if ( container.isNull() )
	{
		// nothing to do
	}
	else if ( container.isInt() )
	{
		int value = container.asInt();
		value = VuEndianUtil::swapIfLittle(value);
		hash = VuHash::fnv64(&value, sizeof(value), hash);
	}
	else if ( container.isFloat() )
	{
		float value = container.asFloat();
		value = VuEndianUtil::swapIfLittle(value);
		hash = VuHash::fnv64(&value, sizeof(value), hash);
	}
	else if ( container.isBool() )
	{
		bool value = container.asBool();
		value = VuEndianUtil::swapIfLittle(value);
		hash = VuHash::fnv64(&value, sizeof(value), hash);
	}
	else if ( container.isString() )
	{
		hash = VuHash::fnv64String(container.asCString(), hash);
	}
	else if ( container.isArray() )
	{
		int size = container.size();
		int fixedSize = VuEndianUtil::swapIfLittle(size);
		hash = VuHash::fnv64(&fixedSize, sizeof(fixedSize), hash);

		// recurse
		for ( int i = 0; i < size; i++ )
			hash = calcHash64(container[i], hash);
	}
	else if ( container.isObject() )
	{
		int count = container.numMembers();
		int fixedCount = VuEndianUtil::swapIfLittle(count);
		hash = VuHash::fnv64(&fixedCount, sizeof(fixedCount), hash);

		// recurse
		for ( int i = 0; i < count; i++ )
		{
			const std::string &key = container.getMemberKey(i);
			hash = VuHash::fnv64String(key.c_str(), hash);
			hash = calcHash64(container[key], hash);
		}
	}
	else if ( container.isInt64() )
	{
		VUINT64 value = container.asInt64();
		value = VuEndianUtil::swapIfLittle(value);
		hash = VuHash::fnv64(&value, sizeof(value), hash);
	}
	else
	{
		VUASSERT(0, "VuDataUtil::calcHash() unsupported type");
	}

	return hash;
}

//*****************************************************************************
const VuJsonContainer *VuDataUtil::resolvePathRead(const VuJsonContainer &container, const VuJsonContainer &schema, const std::string &path)
{
	std::string name = VuEntityUtil::getRoot(path);
	std::string remainingPath = VuEntityUtil::getRemainder(path);

	for ( int i = 0; i < schema.size(); i++ )
	{
		const VuJsonContainer &schemaEntry = schema[i];

		if ( schemaEntry["Name"].asString() == name )
		{
			// are we finished?
			if ( remainingPath.length() )
			{
				const std::string &type = schemaEntry["Type"].asString();
				if ( type == "Array" )
				{
					std::string strIndex = VuEntityUtil::getRoot(remainingPath);
					remainingPath = VuEntityUtil::getRemainder(remainingPath);
					int index;
					const char *str = strIndex.c_str();
					str = strchr(str, '[');
					if ( str && VU_SSCANF(str, "[%d", &index) == 1 )
					{
						return resolvePathRead(container[name][index], schemaEntry["Element"]["Schema"], remainingPath); // array recursion
					}
					else
					{
						return VUNULL; // unable to parse index
					}
				}
				else if ( type == "Object" )
				{
					return resolvePathRead(container[name], schemaEntry["Schema"], remainingPath); // object recursion
				}
				else
				{
					return VUNULL; // trying to get a child of a value
				}
			}
			else
			{
				return &container[name]; // found value
			}
		}
	}

	return VUNULL; // path not found
}

//*****************************************************************************
VuJsonContainer *VuDataUtil::resolvePathWrite(VuJsonContainer &container, const VuJsonContainer &schema, const std::string &path)
{
	std::string name = VuEntityUtil::getRoot(path);
	std::string remainingPath = VuEntityUtil::getRemainder(path);

	for ( int i = 0; i < schema.size(); i++ )
	{
		const VuJsonContainer &schemaEntry = schema[i];

		if ( schemaEntry["Name"].asString() == name )
		{
			// are we finished?
			if ( remainingPath.length() )
			{
				const std::string &type = schemaEntry["Type"].asString();
				if ( type == "Array" )
				{
					std::string strIndex = VuEntityUtil::getRoot(remainingPath);
					remainingPath = VuEntityUtil::getRemainder(remainingPath);
					int index;
					const char *str = strIndex.c_str();
					str = strchr(str, '[');
					if ( str && VU_SSCANF(str, "[%d", &index) == 1 )
					{
						return resolvePathWrite(container[name][index], schemaEntry["Element"]["Schema"], remainingPath); // array recursion
					}
					else
					{
						return VUNULL; // unable to parse index
					}
				}
				else if ( type == "Object" )
				{
					return resolvePathWrite(container[name], schemaEntry["Schema"], remainingPath); // object recursion
				}
				else
				{
					return VUNULL; // trying to get a child of a value
				}
			}
			else
			{
				return &container[name]; // found value
			}
		}
	}

	return VUNULL; // path not found
}

//*****************************************************************************
int VuDataUtil::getArrayIndexOfObject(const VuJsonContainer &array, const char *key, const char *value)
{
	for ( int i = 0; i < array.size(); i++ )
		if ( array[i][key].asString().compare(value) == 0 )
			return i;

	return 0;
}

//*****************************************************************************
int VuDataUtil::getArrayIndexOfObject(const VuJsonContainer &array, const std::string &key, const std::string &value)
{
	return getArrayIndexOfObject(array, key.c_str(), value.c_str());
}

//*****************************************************************************
bool VuDataUtil::hasArrayMember(const VuJsonContainer &array, const char *key, const char *value)
{
	for ( int i = 0; i < array.size(); i++ )
		if ( array[i][key].asString().compare(value) == 0 )
			return true;

	return false;
}

//*****************************************************************************
const VuJsonContainer &VuDataUtil::findArrayMember(const VuJsonContainer &array, const char *key, const char *value)
{
	for ( int i = 0; i < array.size(); i++ )
		if ( array[i][key].asString().compare(value) == 0 )
			return array[i];

	return VuJsonContainer::null;
}

//*****************************************************************************
const VuJsonContainer &VuDataUtil::findArrayMember(const VuJsonContainer &array, const char *key, int value)
{
	for ( int i = 0; i < array.size(); i++ )
		if ( array[i][key].asInt() == value )
			return array[i];

	return VuJsonContainer::null;
}

//*****************************************************************************
bool VuDataUtil::hasArrayMember(const VuJsonContainer &array, const std::string &key, const std::string &value)
{
	return hasArrayMember(array, key.c_str(), value.c_str());
}

//*****************************************************************************
const VuJsonContainer &VuDataUtil::findArrayMember(const VuJsonContainer &array, const std::string &key, const std::string &value)
{
	return findArrayMember(array, key.c_str(), value.c_str());
}

//*****************************************************************************
const VuJsonContainer &VuDataUtil::findArrayMember(const VuJsonContainer &array, const std::string &key, int value)
{
	return findArrayMember(array, key.c_str(), value);
}
