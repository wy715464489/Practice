//*****************************************************************************
//
//  Copyright (c) 2007-2008 Ralf Knoesel
//  Copyright (c) 2008-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  Extended VuJsonContainer functionality.
// 
//*****************************************************************************

#pragma once

#include "VuHash.h"
#include "VuEngine/Method/VuParams.h"
#include "VuEngine/HAL/Gfx/VuGfxTypes.h"
#include "VuEngine/Containers/VuArray.h"
#include "VuEngine/Gfx/Etc/VuEtc.h"
#include "VuEngine/Util/VuTimeUtil.h"

class VuJsonContainer;
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


namespace VuDataUtil
{
	// value access
	bool getValue(const VuJsonContainer &container, int &iVal);
	bool getValue(const VuJsonContainer &container, float &fVal);
	bool getValue(const VuJsonContainer &container, bool &bVal);
	bool getValue(const VuJsonContainer &container, std::string &strVal);
	bool getValue(const VuJsonContainer &container, VUINT64 &i64Val);
	bool getValue(const VuJsonContainer &container, VuVector2 &vVal);
	bool getValue(const VuJsonContainer &container, VuVector3 &vVal);
	bool getValue(const VuJsonContainer &container, VuVector4 &vVal);
	bool getValue(const VuJsonContainer &container, VuPackedVector2 &vVal);
	bool getValue(const VuJsonContainer &container, VuPackedVector3 &vVal);
	bool getValue(const VuJsonContainer &container, VuPackedVector4 &vVal);
	bool getValue(const VuJsonContainer &container, VuMatrix &mat);
	bool getValue(const VuJsonContainer &container, VuQuaternion &quat);
	bool getValue(const VuJsonContainer &container, VuAnimationTransform &animTransform);
	bool getValue(const VuJsonContainer &container, VuColor &cVal);
	bool getValue(const VuJsonContainer &container, VuRect &rect);
	bool getValue(const VuJsonContainer &container, VuAabb &aabb);
	bool getValue(const VuJsonContainer &container, VuArray<VUBYTE> &bytes);
	bool getValue(const VuJsonContainer &container, VuRetVal::eType &retValType);
	bool getValue(const VuJsonContainer &container, VuParams::eType &paramType);
	bool getValue(const VuJsonContainer &container, VuParamDecl &paramDecl);
	bool getValue(const VuJsonContainer &container, VuStringFormat &stringFormat);
	bool getValue(const VuJsonContainer &container, VuFontDrawParams &params);
	bool getValue(const VuJsonContainer &container, VuGfxTextureAddress &mode);
	bool getValue(const VuJsonContainer &container, VuGfxTextureFilterType &type);
	bool getValue(const VuJsonContainer &container, VuGfxTextureType &type);
	bool getValue(const VuJsonContainer &container, VuGfxFormatDX &format);
	bool getValue(const VuJsonContainer &container, VuGfxFormatIOS &format);
	bool getValue(const VuJsonContainer &container, VuGfxFormatOGLES &format);
	bool getValue(const VuJsonContainer &container, VuEtc::eQuality &etcQuality);
	bool getValue(const VuJsonContainer &container, VuTimeUtil::VuTimeStruct &ts);

	// value manipulation
	void putValue(VuJsonContainer &container, int iVal);
	void putValue(VuJsonContainer &container, float fVal);
	void putValue(VuJsonContainer &container, bool bVal);
	void putValue(VuJsonContainer &container, const char *strVal);
	void putValue(VuJsonContainer &container, const std::string &strVal);
	void putValue(VuJsonContainer &container, const VUINT64 &i64Val);
	void putValue(VuJsonContainer &container, const VuVector2 &vVal);
	void putValue(VuJsonContainer &container, const VuVector3 &vVal);
	void putValue(VuJsonContainer &container, const VuVector4 &vVal);
	void putValue(VuJsonContainer &container, const VuPackedVector2 &vVal);
	void putValue(VuJsonContainer &container, const VuPackedVector3 &vVal);
	void putValue(VuJsonContainer &container, const VuPackedVector4 &vVal);
	void putValue(VuJsonContainer &container, const VuMatrix &mat);
	void putValue(VuJsonContainer &container, const VuQuaternion &quat);
	void putValue(VuJsonContainer &container, const VuAnimationTransform &animTransform);
	void putValue(VuJsonContainer &container, const VuColor &cVal);
	void putValue(VuJsonContainer &container, const VuRect &rect);
	void putValue(VuJsonContainer &container, const VuAabb &aabb);
	void putValue(VuJsonContainer &container, const VuArray<VUBYTE> &bytes);
	void putValue(VuJsonContainer &container, const VuRetVal::eType &retValType);
	void putValue(VuJsonContainer &container, const VuParams::eType &paramType);
	void putValue(VuJsonContainer &container, const VuParamDecl &paramDecl);
	void putValue(VuJsonContainer &container, const VuStringFormat &stringFormat);
	void putValue(VuJsonContainer &container, const VuFontDrawParams &params);
	void putValue(VuJsonContainer &container, const VuGfxTextureAddress &mode);
	void putValue(VuJsonContainer &container, const VuGfxTextureFilterType &type);
	void putValue(VuJsonContainer &container, const VuGfxTextureType &type);
	void putValue(VuJsonContainer &container, const VuGfxFormatDX &format);
	void putValue(VuJsonContainer &container, const VuGfxFormatIOS &format);
	void putValue(VuJsonContainer &container, const VuGfxFormatOGLES &format);
	void putValue(VuJsonContainer &container, const VuEtc::eQuality &etcQuality);
	void putValue(VuJsonContainer &container, const VuTimeUtil::VuTimeStruct &ts);

	// hash
	VUUINT32 calcHash32(const VuJsonContainer &container, VUUINT32 hash = VU_FNV32_INIT);
	VUUINT64 calcHash64(const VuJsonContainer &container, VUUINT64 hash = VU_FNV64_INIT);

	// access w/ schema
	const VuJsonContainer *resolvePathRead(const VuJsonContainer &container, const VuJsonContainer &schema, const std::string &path);
	VuJsonContainer       *resolvePathWrite(VuJsonContainer &container, const VuJsonContainer &schema, const std::string &path);

	// array index of object with key/value
	int getArrayIndexOfObject(const VuJsonContainer &array, const char *key, const char *value);
	int getArrayIndexOfObject(const VuJsonContainer &array, const std::string &key, const std::string &value);

	// find keyed array member
	bool					hasArrayMember(const VuJsonContainer &array, const char *key, const char *value);
	const VuJsonContainer	&findArrayMember(const VuJsonContainer &array, const char *key, const char *value);
	const VuJsonContainer	&findArrayMember(const VuJsonContainer &array, const char *key, int value);

	bool					hasArrayMember(const VuJsonContainer &array, const std::string &key, const std::string &value);
	const VuJsonContainer	&findArrayMember(const VuJsonContainer &array, const std::string &key, const std::string &value);
	const VuJsonContainer	&findArrayMember(const VuJsonContainer &array, const std::string &key, int value);
}
