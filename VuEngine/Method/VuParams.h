//*****************************************************************************
//
//  Copyright (c) 2007-2008 Ralf Knoesel
//  Copyright (c) 2008-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  Classes supporting parameters
// 
//*****************************************************************************

#pragma once

#include "VuEngine/Math/VuVector2.h"
#include "VuEngine/Math/VuVector3.h"
#include "VuEngine/Math/VuVector4.h"
#include "VuEngine/Util/VuColor.h"
#include "VuEngine/Util/VuGuid.h"

class VuAsset;
class VuEntity;


//*****************************************************************************
// Return Value
// Return values are passed immediately on the stack.  They are never deferred
// or used over a network layer.
//*****************************************************************************
class VuRetVal
{
public:
	enum eType { Void, Int, Float, Bool, String, TYPE_COUNT };

	VuRetVal()								{ mType = Void; }
	explicit VuRetVal(int value)			{ mType = Int;		mIntValue = value; }
	explicit VuRetVal(float value)			{ mType = Float;	mFloatValue = value; }
	explicit VuRetVal(bool value)			{ mType = Bool;		mBoolValue = value; }
	explicit VuRetVal(const char *value)	{ mType = String;	mStringValue = value; }

	eType				getType() const		{ return mType; }

	int					asInt() const		{ VUASSERT(mType == Int, "VuRetVal::asInt() illegal conversion");		return mIntValue; }
	float				asFloat() const		{ VUASSERT(mType == Float, "VuRetVal::asFloat() illegal conversion");	return mFloatValue; }
	bool				asBool() const		{ VUASSERT(mType == Bool, "VuRetVal::asBool() illegal conversion");		return mBoolValue; }
	const char			*asString() const	{ VUASSERT(mType == String, "VuRetVal::asString() illegal conversion");	return mStringValue; }

	static const char	*typeToString(eType type);
	static bool			stringToType(const char *strType, eType &type);

private:
	eType	mType;
	union
	{
		int				mIntValue;
		float			mFloatValue;
		bool			mBoolValue;
		const char		*mStringValue;
	};
};


//*****************************************************************************
// Parameters
// Holds the values and types of the parameters passed along with an event.
//*****************************************************************************
class VuParams
{
public:
	VuParams() : mSize(0) {}
	VuParams(const void *pData, int size);

	enum eType { Invalid = -1, Int, UnsignedInt, Float, Bool, String, Vector2, Vector3, Vector4, Color, Guid, Pointer, Asset, Entity, TYPE_COUNT };

	// parameter building
	void			addInt(const VUINT &value)			{ addValue(Int, &value, sizeof(value)); }
	void			addUnsignedInt(const VUUINT &value)	{ addValue(UnsignedInt, &value, sizeof(value)); }
	void			addFloat(const float &value)		{ addValue(Float, &value, sizeof(value)); }
	void			addBool(const bool &value)			{ addValue(Bool, &value, sizeof(value)); }
	void			addString(const char *str)			{ addValue(String, str, (int)strlen(str) + 1); }
	void			addVector2(const VuVector2 &value)	{ addValue(Vector2, &value, sizeof(value)); }
	void			addVector3(const VuVector3 &value)	{ addValue(Vector3, &value, sizeof(value)); }
	void			addVector4(const VuVector4 &value)	{ addValue(Vector4, &value, sizeof(value)); }
	void			addColor(const VuColor &value)		{ addValue(Color, &value, sizeof(value)); }
	void			addGuid(const VuGuid &value)		{ addValue(Guid, &value, sizeof(value)); }
	void			addPointer(const void *value)		{ addValue(Pointer, &value, sizeof(value)); }
	void			addAsset(const VuAsset *pAsset);
	void			addEntity(const VuEntity *pEntity);

	// parameter access
	class VuAccessor
	{
	public:
		VuAccessor(const VuParams &params);

		// accessors (will assert if types don't match)
		VUINT		getInt()			{ return getBasicValue<VUINT>(Int, 0); }
		VUUINT		getUnsignedInt()	{ return getBasicValue<VUUINT>(UnsignedInt, 0); }
		float		getFloat()			{ return getBasicValue<float >(Float, 0.0f); }
		bool		getBool()			{ return getBasicValue<bool>(Bool, false); }
		const char	*getString();
		VuVector2	getVector2()		{ return getBasicValue<VuVector2>(Vector2, VuVector2(0,0)); }
		VuVector3	getVector3()		{ return getBasicValue<VuVector3>(Vector3, VuVector3(0,0,0)); }
		VuVector4	getVector4()		{ return getBasicValue<VuVector4>(Vector4, VuVector4(0,0,0,0)); }
		VuColor		getColor()			{ return getBasicValue<VuColor>(Color, VuColor(0,0,0)); }
		VuGuid		getGuid()			{ return getBasicValue<VuGuid>(Guid, VuGuid()); }
		void		*getPointer()		{ return getBasicValue<void *>(Pointer, VUNULL); }
		VuAsset		*getAsset();
		VuEntity	*getEntity();
		VUUINT32	getEntityHash();

		eType		getNextType() const;

	protected:
		friend class VuParams;
		template<class T>
		const T				getBasicValue(eType type, const T &defaultValue);

		bool				verifyNextType(eType type);

		const VUBYTE		*mpDataRemaining;
		int					mSizeRemaining;
	};

	static const char	*typeToString(eType type);
	static bool			stringToType(const char *strType, eType &type);

	const VUBYTE	*getData() const { return mData; }
	int				getSize() const { return mSize; }

	void			setData(const void *pData, int size);

	inline bool		operator == (const VuParams &other) const;
	inline bool		operator != (const VuParams &other) const { return !(*this == other); }

	void			addParams(const VuAccessor &params);

private:
	enum { MAX_DATA_SIZE = 128 };

	void			addValue(eType type, const void *pValue, int size);
	bool			verifyDataSize(int valueSize);

	VUBYTE			mData[MAX_DATA_SIZE];
	int				mSize;
};


//*****************************************************************************
// Parameter Declaration
// Declares a list of parameters accepted by a handler (event, method, etc).
//*****************************************************************************
class VuParamDecl
{
public:
	enum { MAX_NUM_PARAMS = 8 };

	VuParamDecl() : mNumParams(0) {}
	VuParamDecl(int numParams, ...);

	VUUINT32		calcHash() const;

	int				mNumParams;
	VuParams::eType	maParamTypes[MAX_NUM_PARAMS];
};


#include "VuParams.inl"
