//*****************************************************************************
//
//  Copyright (c) 2014-2014 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  Extended VuFastContainer functionality.
// 
//*****************************************************************************

#pragma once

#include "VuHash.h"
#include "VuEngine/Method/VuParams.h"
#include "VuEngine/HAL/Gfx/VuGfxTypes.h"
#include "VuEngine/Containers/VuArray.h"
#include "VuEngine/Gfx/Etc/VuEtc.h"
#include "VuEngine/Util/VuTimeUtil.h"

class VuFastContainer;
class VuVector2;
class VuVector3;
class VuVector4;
class VuPackedVector2;
class VuPackedVector3;
class VuPackedVector4;
class VuMatrix;
class VuQuaternion;
class VuColor;
class VuRect;
class VuAabb;
class VuStringFormat;
class VuFontDrawParams;
class VuAnimationTransform;


namespace VuFastDataUtil
{
	// value access
	bool getValue(const VuFastContainer &container, int &iVal);
	bool getValue(const VuFastContainer &container, float &fVal);
	bool getValue(const VuFastContainer &container, bool &bVal);
	bool getValue(const VuFastContainer &container, std::string &strVal);
	bool getValue(const VuFastContainer &container, VUINT64 &i64Val);
	bool getValue(const VuFastContainer &container, VuVector2 &vVal);
	bool getValue(const VuFastContainer &container, VuVector3 &vVal);
	bool getValue(const VuFastContainer &container, VuVector4 &vVal);
	bool getValue(const VuFastContainer &container, VuPackedVector2 &vVal);
	bool getValue(const VuFastContainer &container, VuPackedVector3 &vVal);
	bool getValue(const VuFastContainer &container, VuPackedVector4 &vVal);
	bool getValue(const VuFastContainer &container, VuMatrix &mat);
	bool getValue(const VuFastContainer &container, VuQuaternion &quat);
	bool getValue(const VuFastContainer &container, VuAnimationTransform &animTransform);
	bool getValue(const VuFastContainer &container, VuColor &cVal);
	bool getValue(const VuFastContainer &container, VuRect &rect);
	bool getValue(const VuFastContainer &container, VuAabb &aabb);
	bool getValue(const VuFastContainer &container, VuArray<VUBYTE> &bytes);
	bool getValue(const VuFastContainer &container, VuRetVal::eType &retValType);
	bool getValue(const VuFastContainer &container, VuParams::eType &paramType);
	bool getValue(const VuFastContainer &container, VuParamDecl &paramDecl);
	bool getValue(const VuFastContainer &container, VuStringFormat &stringFormat);
	bool getValue(const VuFastContainer &container, VuFontDrawParams &params);
	bool getValue(const VuFastContainer &container, VuGfxTextureAddress &mode);
	bool getValue(const VuFastContainer &container, VuGfxTextureFilterType &type);
	bool getValue(const VuFastContainer &container, VuGfxTextureType &type);
	bool getValue(const VuFastContainer &container, VuGfxFormatDX &format);
	bool getValue(const VuFastContainer &container, VuGfxFormatIOS &format);
	bool getValue(const VuFastContainer &container, VuGfxFormatOGLES &format);
	bool getValue(const VuFastContainer &container, VuEtc::eQuality &etcQuality);
	bool getValue(const VuFastContainer &container, VuTimeUtil::VuTimeStruct &ts);

	// array index of object with key/value
	int getArrayIndexOfObject(const VuFastContainer &array, const char *key, const char *value);
	int getArrayIndexOfObject(const VuFastContainer &array, const std::string &key, const std::string &value);

	// find keyed array member
	bool					hasArrayMember(const VuFastContainer &array, const char *key, const char *value);
	const VuFastContainer	&findArrayMember(const VuFastContainer &array, const char *key, const char *value);
	const VuFastContainer	&findArrayMember(const VuFastContainer &array, const char *key, int value);

	bool					hasArrayMember(const VuFastContainer &array, const std::string &key, const std::string &value);
	const VuFastContainer	&findArrayMember(const VuFastContainer &array, const std::string &key, const std::string &value);
	const VuFastContainer	&findArrayMember(const VuFastContainer &array, const std::string &key, int value);
}
