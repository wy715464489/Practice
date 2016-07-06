//*****************************************************************************
//
//  Copyright (c) 2007-2008 Ralf Knoesel
//  Copyright (c) 2008-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  DB Asset class
// 
//*****************************************************************************

#pragma once

#include "VuGenericDataAsset.h"


class VuDBAsset : public VuGenericDataAsset
{
	DECLARE_RTTI

protected:
	~VuDBAsset() {}

public:
	const VuJsonContainer	&getDB() const { return getDataContainer(); }

	// for DBs with key/value data
	int				getMemberCount() { return (int)mMemberKeys.size(); }
	const char		*getMemberKey(int index) { return mMemberKeys[index]; }

	static void		schema(const VuJsonContainer &creationInfo, VuJsonContainer &schema);

protected:
	virtual bool	load(VuBinaryDataReader &reader);

	typedef std::vector<const char *> MemberKeys;
	MemberKeys		mMemberKeys;
};
