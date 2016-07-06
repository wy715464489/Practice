//*****************************************************************************
//
//  Copyright (c) 2008-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  Pfx Node
// 
//*****************************************************************************

#pragma once

#include "VuEngine/Objects/VuRefObj.h"
#include "VuEngine/Objects/VuRTTI.h"
#include "VuEngine/Properties/VuProperties.h"


class VuPfxNode : public VuRefObj
{
	DECLARE_RTTI

protected:
	~VuPfxNode();

public:
	virtual void	load(const VuJsonContainer &data);
	virtual void	save(VuJsonContainer &data) const;
	virtual void	load(const VuFastContainer &data);

	// these virtual functions may be be used to customize derived classes
	// (called after the corresponding base method is called)
	virtual void	onLoad() {}

	typedef std::map<std::string, VuPfxNode *> ChildNodes;

	ChildNodes		mChildNodes;
	VuProperties	mProperties;
	std::string		mName;

private:
	void			loadChildNodes(const VuJsonContainer &data);
	void			saveChildNodes(VuJsonContainer &data) const;
	void			loadChildNodes(const VuFastContainer &data);
};
