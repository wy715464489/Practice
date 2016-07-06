//*****************************************************************************
//
//  Copyright (c) 2007-2008 Ralf Knoesel
//  Copyright (c) 2008-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  Asset property classes
// 
//*****************************************************************************

#pragma once

#include "VuProperty.h"
#include "VuEnumProperty.h"


class VuAssetNameProperty : public VuStringEnumProperty
{
public:

	VuAssetNameProperty(const char *strAssetType, const char *strName, std::string &pValue);

	virtual eType		getType() const					{ return ASSET; }

	virtual int			getChoiceCount() const;
	virtual const char	*getChoice(int index) const;

	const char			*getAssetType() const { return mstrAssetType; }

protected:
	const char			*mstrAssetType;
};


template<class AssetType>
class VuAssetProperty : public VuAssetNameProperty
{
public:

	VuAssetProperty(const char *strName, std::string &pValue);
	~VuAssetProperty();

	AssetType			*getAsset()	{ return mpAsset; }

protected:
	virtual void		onValueChanged();

	AssetType			*mpAsset;
};


class VuBaseAssetProperty : public VuAssetNameProperty
{
public:

	VuBaseAssetProperty(const char *strAssetType, const char *strName, std::string &pValue);
	~VuBaseAssetProperty();

	VuAsset				*getAsset()	{ return mpAsset; }

protected:
	virtual void		onValueChanged();

	VuAsset				*mpAsset;
};


#include "VuAssetProperty.inl"
