//*****************************************************************************
//
//  Copyright (c) 2007-2008 Ralf Knoesel
//  Copyright (c) 2008-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  Asset Factory inline implementation
// 
//*****************************************************************************

#include "VuAsset.h"
#include "VuEngine/Util/VuHash.h"


//*****************************************************************************
template<class T>
T *VuAssetFactory::createAsset(const std::string &strAsset, int flags)
{
	VuAsset *pAsset = createAsset(T::msRTTI.mstrType, strAsset, flags);
	if ( pAsset )
	{
		VUASSERT(pAsset->isDerivedFrom(T::msRTTI), "VuAssetFactory::createAsset() RTTI mismatch");
	}

	return static_cast<T *>(pAsset);
}

//*****************************************************************************
template<class T>
bool VuAssetFactory::doesAssetExist(const std::string &strAsset)
{
	return doesAssetExist(T::msRTTI.mstrType, strAsset);
}

//*****************************************************************************
template<class T>
const VuAssetFactory::AssetNames &VuAssetFactory::getAssetNames()
{
	return getAssetNames(T::msRTTI.mstrType);
}

//*****************************************************************************
template<class T>
void VuAssetFactory::forgetAsset(const std::string &strAsset)
{
	forgetAsset(T::msRTTI.mstrType, strAsset);
}

//*****************************************************************************
VUUINT32 VuAssetFactory::calcAssetHashID(const char *strType, const char *strAsset)
{
	VUUINT32 hash = VuHash::fnv32String(strType);
	return VuHash::fnv32String(strAsset, hash);
}
