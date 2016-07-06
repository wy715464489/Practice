//*****************************************************************************
//
//  Copyright (c) 2014-2014 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  Extended VuFastContainer functionality.
// 
//*****************************************************************************

#include <ctype.h>
#include "VuFastDataUtil.h"
#include "VuEngine/Json/VuFastContainer.h"
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
bool VuFastDataUtil::getValue(const VuFastContainer &container, int &iVal)
{
	return container.getValue(iVal);
}

//*****************************************************************************
bool VuFastDataUtil::getValue(const VuFastContainer &container, float &fVal)
{
	return container.getValue(fVal);
}

//*****************************************************************************
bool VuFastDataUtil::getValue(const VuFastContainer &container, bool &bVal)
{
	return container.getValue(bVal);
}

//*****************************************************************************
bool VuFastDataUtil::getValue(const VuFastContainer &container, std::string &strVal)
{
	return container.getValue(strVal);
}

//*****************************************************************************
bool VuFastDataUtil::getValue(const VuFastContainer &container, VUINT64 &i64Val)
{
	return container.getValue(i64Val);
}

//*****************************************************************************
bool VuFastDataUtil::getValue(const VuFastContainer &container, VuVector2 &vVal)
{
	bool all = true;

	all &= getValue(container["X"], vVal.mX);
	all &= getValue(container["Y"], vVal.mY);

	return all;
}

//*****************************************************************************
bool VuFastDataUtil::getValue(const VuFastContainer &container, VuVector3 &vVal)
{
	bool all = true;

	all &= getValue(container["X"], vVal.mX);
	all &= getValue(container["Y"], vVal.mY);
	all &= getValue(container["Z"], vVal.mZ);

	return all;
}

//*****************************************************************************
bool VuFastDataUtil::getValue(const VuFastContainer &container, VuVector4 &vVal)
{
	bool all = true;

	all &= getValue(container["X"], vVal.mX);
	all &= getValue(container["Y"], vVal.mY);
	all &= getValue(container["Z"], vVal.mZ);
	all &= getValue(container["W"], vVal.mW);

	return all;
}

//*****************************************************************************
bool VuFastDataUtil::getValue(const VuFastContainer &container, VuPackedVector2 &vVal)
{
	bool all = true;

	all &= getValue(container["X"], vVal.mX);
	all &= getValue(container["Y"], vVal.mY);

	return all;
}

//*****************************************************************************
bool VuFastDataUtil::getValue(const VuFastContainer &container, VuPackedVector3 &vVal)
{
	bool all = true;

	all &= getValue(container["X"], vVal.mX);
	all &= getValue(container["Y"], vVal.mY);
	all &= getValue(container["Z"], vVal.mZ);

	return all;
}

//*****************************************************************************
bool VuFastDataUtil::getValue(const VuFastContainer &container, VuPackedVector4 &vVal)
{
	bool all = true;

	all &= getValue(container["X"], vVal.mX);
	all &= getValue(container["Y"], vVal.mY);
	all &= getValue(container["Z"], vVal.mZ);
	all &= getValue(container["W"], vVal.mW);

	return all;
}

//*****************************************************************************
bool VuFastDataUtil::getValue(const VuFastContainer &container, VuMatrix &mat)
{
	bool all = true;

	all &= getValue(container["X"], mat.mX);
	all &= getValue(container["Y"], mat.mY);
	all &= getValue(container["Z"], mat.mZ);
	all &= getValue(container["T"], mat.mT);

	return all;
}

//*****************************************************************************
bool VuFastDataUtil::getValue(const VuFastContainer &container, VuQuaternion &quat)
{
	return getValue(container, quat.mVec);
}

//*****************************************************************************
bool VuFastDataUtil::getValue(const VuFastContainer &container, VuAnimationTransform &animTransform)
{
	bool all = true;

	all &= getValue(container["T"], animTransform.mTranslation);
	all &= getValue(container["R"], animTransform.mRotation);
	all &= getValue(container["S"], animTransform.mScale);

	return all;
}

//*****************************************************************************
bool VuFastDataUtil::getValue(const VuFastContainer &container, VuColor &cVal)
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
bool VuFastDataUtil::getValue(const VuFastContainer &container, VuRect &rect)
{
	bool all = true;

	all &= getValue(container["X"], rect.mX);
	all &= getValue(container["Y"], rect.mY);
	all &= getValue(container["W"], rect.mWidth);
	all &= getValue(container["H"], rect.mHeight);

	return all;
}

//*****************************************************************************
bool VuFastDataUtil::getValue(const VuFastContainer &container, VuAabb &aabb)
{
	bool all = true;

	all &= getValue(container["Min"], aabb.mMin);
	all &= getValue(container["Max"], aabb.mMax);

	return all;
}

//*****************************************************************************
bool VuFastDataUtil::getValue(const VuFastContainer &container, VuArray<VUBYTE> &bytes)
{
	if ( container["Base64"].getType() == VuFastContainer::stringValue )
	{
		return VuBase64::decode(container["Base64"].asCString(), bytes);
	}
	return false;
}

//*****************************************************************************
bool VuFastDataUtil::getValue(const VuFastContainer &container, VuRetVal::eType &retValType)
{
	return VuRetVal::stringToType(container.asCString(), retValType);
}

//*****************************************************************************
bool VuFastDataUtil::getValue(const VuFastContainer &container, VuParams::eType &paramType)
{
	return VuParams::stringToType(container.asCString(), paramType);
}

//*****************************************************************************
bool VuFastDataUtil::getValue(const VuFastContainer &container, VuParamDecl &paramDecl)
{
	paramDecl.mNumParams = VuMin(container.size(), VuParamDecl::MAX_NUM_PARAMS);
	for ( int i = 0; i < paramDecl.mNumParams; i++ )
		if ( !getValue(container[i], paramDecl.maParamTypes[i]) )
			return false;
	return true;
}

//*****************************************************************************
bool VuFastDataUtil::getValue(const VuFastContainer &container, VuStringFormat &stringFormat)
{
	bool all = true;

	all &= getValue(container["AlignH"], stringFormat.mAlignH);
	all &= getValue(container["AlignV"], stringFormat.mAlignV);
	all &= getValue(container["Clip"], stringFormat.mClip);
	all &= getValue(container["Wordbreak"], stringFormat.mWordbreak);

	return all;
}

//*****************************************************************************
bool VuFastDataUtil::getValue(const VuFastContainer &container, VuFontDrawParams &params)
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
bool VuFastDataUtil::getValue(const VuFastContainer &container, VuGfxTextureAddress &mode)
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
bool VuFastDataUtil::getValue(const VuFastContainer &container, VuGfxTextureFilterType &type)
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
bool VuFastDataUtil::getValue(const VuFastContainer &container, VuGfxTextureType &type)
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
bool VuFastDataUtil::getValue(const VuFastContainer &container, VuGfxFormatDX &format)
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
bool VuFastDataUtil::getValue(const VuFastContainer &container, VuGfxFormatIOS &format)
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
bool VuFastDataUtil::getValue(const VuFastContainer &container, VuGfxFormatOGLES &format)
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
bool VuFastDataUtil::getValue(const VuFastContainer &container, VuEtc::eQuality &etcQuality)
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
bool VuFastDataUtil::getValue(const VuFastContainer &container, VuTimeUtil::VuTimeStruct &ts)
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
int VuFastDataUtil::getArrayIndexOfObject(const VuFastContainer &array, const char *key, const char *value)
{
	for ( int i = 0; i < array.size(); i++ )
		if ( strcmp(array[i][key].asCString(), value) == 0 )
			return i;

	return 0;
}

//*****************************************************************************
int VuFastDataUtil::getArrayIndexOfObject(const VuFastContainer &array, const std::string &key, const std::string &value)
{
	return getArrayIndexOfObject(array, key.c_str(), value.c_str());
}

//*****************************************************************************
bool VuFastDataUtil::hasArrayMember(const VuFastContainer &array, const char *key, const char *value)
{
	for ( int i = 0; i < array.size(); i++ )
		if ( strcmp(array[i][key].asCString(), value) == 0 )
			return true;

	return false;
}

//*****************************************************************************
const VuFastContainer &VuFastDataUtil::findArrayMember(const VuFastContainer &array, const char *key, const char *value)
{
	for ( int i = 0; i < array.size(); i++ )
		if ( strcmp(array[i][key].asCString(), value) == 0 )
			return array[i];

	return VuFastContainer::null;
}

//*****************************************************************************
const VuFastContainer &VuFastDataUtil::findArrayMember(const VuFastContainer &array, const char *key, int value)
{
	for ( int i = 0; i < array.size(); i++ )
		if ( array[i][key].asInt() == value )
			return array[i];

	return VuFastContainer::null;
}

//*****************************************************************************
bool VuFastDataUtil::hasArrayMember(const VuFastContainer &array, const std::string &key, const std::string &value)
{
	return hasArrayMember(array, key.c_str(), value.c_str());
}

//*****************************************************************************
const VuFastContainer &VuFastDataUtil::findArrayMember(const VuFastContainer &array, const std::string &key, const std::string &value)
{
	return findArrayMember(array, key.c_str(), value.c_str());
}

//*****************************************************************************
const VuFastContainer &VuFastDataUtil::findArrayMember(const VuFastContainer &array, const std::string &key, int value)
{
	return findArrayMember(array, key.c_str(), value);
}
