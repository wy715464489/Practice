//*****************************************************************************
//
//  Copyright (c) 2007-2008 Ralf Knoesel
//  Copyright (c) 2008-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  Event class
// 
//*****************************************************************************

#include <stdarg.h>
#include "VuParams.h"
#include "VuEngine/Util/VuHash.h"
#include "VuEngine/Assets/VuAsset.h"
#include "VuEngine/Assets/VuAssetFactory.h"
#include "VuEngine/Entities/VuEntity.h"
#include "VuEngine/Entities/VuEntityRepository.h"

static const char *sRetValTypes[] =
{
	"Void",		// Void
	"Int",		// Int
	"Float",	// Float
	"Bool",		// Bool
	"String",	// String
};
static int const sNumRetValTypes = sizeof(sRetValTypes)/sizeof(sRetValTypes[0]);
VU_COMPILE_TIME_ASSERT(sNumRetValTypes == VuRetVal::TYPE_COUNT);

//*****************************************************************************
const char *VuRetVal::typeToString(VuRetVal::eType type)
{
	VUASSERT(type >= 0 && type < sNumRetValTypes, "VuRetVal::typeToString() type out of range");

	return sRetValTypes[type];
}

//*****************************************************************************
bool VuRetVal::stringToType(const char *strType, VuRetVal::eType &type)
{
	for ( int i = 0; i < sNumRetValTypes; i++ )
	{
		if ( strcmp(sRetValTypes[i], strType) == 0 )
		{
			type = static_cast<VuRetVal::eType>(i);
			return true;
		}
	}

	return false;
}

static const char *sParamTypes[] =
{
	"Int",			// Int
	"UnsignedInt",	// UnsignedInt
	"Float",		// Float
	"Bool",			// Bool
	"String",		// String
	"Vector2",		// Vector2
	"Vector3",		// Vector3
	"Vector4",		// Vector4
	"Color",		// Color
	"Guid",			// Guid
	"Pointer",		// Pointer
	"Asset",		// Asset
	"Entity",		// Entity
};
static int const sNumParamTypes = sizeof(sParamTypes)/sizeof(sParamTypes[0]);
VU_COMPILE_TIME_ASSERT(sNumParamTypes == VuParams::TYPE_COUNT);

//*****************************************************************************
const char *VuParams::typeToString(VuParams::eType type)
{
	VUASSERT(type >= 0 && type < sNumParamTypes, "VuParam::typeToString() type out of range");

	return sParamTypes[type];
}

//*****************************************************************************
bool VuParams::stringToType(const char *strType, VuParams::eType &type)
{
	for ( int i = 0; i < sNumParamTypes; i++ )
	{
		if ( strcmp(sParamTypes[i], strType) == 0 )
		{
			type = static_cast<VuParams::eType>(i);
			return true;
		}
	}

	return false;
}

//*****************************************************************************
VuParams::VuParams(const void *pData, int size)
{
	setData(pData, size);
}

//*****************************************************************************
void VuParams::addAsset(const VuAsset *pAsset)
{
	VUUINT32 hash = pAsset ? pAsset->getHashID() : 0;
	addValue(Asset, &hash, sizeof(hash));
}

//*****************************************************************************
void VuParams::addEntity(const VuEntity *pEntity)
{
	VUUINT32 hash = pEntity ? pEntity->getHashedLongName() : 0;
	addValue(Entity, &hash, sizeof(hash));
}

//*****************************************************************************
// adds a generic value to a parameter list
//*****************************************************************************
void VuParams::addValue(eType type, const void *pValue, int size)
{
	if ( verifyDataSize(size) )
	{
		VU_MEMCPY(&mData[mSize], MAX_DATA_SIZE - mSize, &type, sizeof(type));
		mSize += sizeof(type);

		VU_MEMCPY(&mData[mSize], MAX_DATA_SIZE - mSize, pValue, size);
		mSize += size;
	}
}

//*****************************************************************************
// Make sure that enough space exists to add data of a given size.
//*****************************************************************************
bool VuParams::verifyDataSize(int valueSize)
{
	if ( mSize + sizeof(eType) + valueSize <= MAX_DATA_SIZE )
		return true;

	VUASSERT(0, "VuParams::verifyDataSize() MAX_DATA_SIZE exceeded");

	return false;
}

void VuParams::setData(const void *pData, int size)
{
	if(verifyDataSize(size))
	{
		VU_MEMCPY(mData, MAX_DATA_SIZE, pData, size);
		mSize = size;
	}
}

//*****************************************************************************
void VuParams::addParams(const VuAccessor &params)
{
	mSize = params.mSizeRemaining;
	VU_MEMCPY(mData, MAX_DATA_SIZE, params.mpDataRemaining, params.mSizeRemaining);
}

//*****************************************************************************
VuParams::VuAccessor::VuAccessor(const VuParams &params):
	mpDataRemaining(params.getData()),
	mSizeRemaining(params.getSize())
{
}

//*****************************************************************************
const char *VuParams::VuAccessor::getString()
{
	eType type = String;
	const char *str = "";

	if ( verifyNextType(type) )
	{
		mpDataRemaining += sizeof(type);
		mSizeRemaining -= sizeof(type);

		str = reinterpret_cast<const char *>(mpDataRemaining);
		int valueSize = (int)strlen(str) + 1;

		mpDataRemaining += valueSize;
		mSizeRemaining -= valueSize;
	}

	return str;
}

//*****************************************************************************
VuAsset *VuParams::VuAccessor::getAsset()
{
	eType type = Asset;
	VUUINT32 hash = 0;

	if ( verifyNextType(type) )
	{
		mpDataRemaining += sizeof(type);
		mSizeRemaining -= sizeof(type);

		VU_MEMCPY(&hash, sizeof(hash), mpDataRemaining, sizeof(hash));
		mpDataRemaining += sizeof(hash);
		mSizeRemaining -= sizeof(hash);
	}

	return VuAssetFactory::IF()->findAsset(hash);
}

//*****************************************************************************
VuEntity *VuParams::VuAccessor::getEntity()
{
	VUUINT32 hash = getEntityHash();

	return VuEntityRepository::IF()->findEntity(hash);
}

VUUINT32 VuParams::VuAccessor::getEntityHash()
{
	eType type = Entity;
	VUUINT32 hash = 0;

	if ( verifyNextType(type) )
	{
		mpDataRemaining += sizeof(type);
		mSizeRemaining -= sizeof(type);

		VU_MEMCPY(&hash, sizeof(hash), mpDataRemaining, sizeof(hash));
		mpDataRemaining += sizeof(hash);
		mSizeRemaining -= sizeof(hash);
	}

	return hash;
}

//*****************************************************************************
// Get the next available parameter type (if any).
// Note: need to copy into a local because the data has no alignment restrictions
//*****************************************************************************
VuParams::eType VuParams::VuAccessor::getNextType() const
{
	VuParams::eType type = Invalid;

	if ( mSizeRemaining > 0 )
		VU_MEMCPY(&type, sizeof(type), mpDataRemaining, sizeof(type));

	return type;
}

//*****************************************************************************
// Verify that the next available parameter is of an expected type.
//*****************************************************************************
bool VuParams::VuAccessor::verifyNextType(eType type)
{
	if ( getNextType() == type )
		return true;

	VUASSERT(0, "VuParams::VuAccessor::verifyNextType() type mismatch");

	return false;
}

//*****************************************************************************
VuParamDecl::VuParamDecl(int numParams, ...):
	mNumParams(numParams)
{
	va_list VaList;
	va_start(VaList, numParams);

	VUASSERT(numParams < MAX_NUM_PARAMS, "VuParamDecl::VuParamDecl() max num params exceeded");

	// construct declaration from variable args
	for ( int i = 0; i < numParams; i++ )
	{
		//maParamTypes[i] = va_arg(VaList, VuParams::eType);
		// was causing gcc to choke:
		// VuEngine/Method/VuParams.cpp: In constructor 'VuParamDecl::VuParamDecl(int, ...)':
		//                               warning: 'VuParams::eType' is promoted to 'int' when passed through '...'
		//                               note: (so you should pass 'int' not 'VuParams::eType' to 'va_arg')
		//                               note: if this code is reached, the program will abort
		maParamTypes[i] = (VuParams::eType)va_arg(VaList, int); 
	}
}

//*****************************************************************************
VUUINT32 VuParamDecl::calcHash() const
{
	return VuHash::fnv32(maParamTypes, sizeof(maParamTypes[0])*mNumParams);
}

