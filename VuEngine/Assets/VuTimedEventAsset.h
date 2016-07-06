//*****************************************************************************
//
//  Copyright (c) 2007-2008 Ralf Knoesel
//  Copyright (c) 2008-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  Timed Event Asset class
// 
//*****************************************************************************

#pragma once

#include "VuEngine/Assets/VuAsset.h"


class VuTimedEventAsset : public VuAsset
{
	DECLARE_RTTI

protected:
	~VuTimedEventAsset() {}

public:
	VuTimedEventAsset() : mEvents(0) {}

	class VuEvent
	{
	public:
		VuEvent() : mTime(0) {}
		float			mTime;
		std::string		mType;
		VuJsonContainer	mParams;
	};

	int					getEventCount() const		{ return (int)mEvents.size(); }
	const VuEvent		&getEvent(int index) const	{ return mEvents[index]; }

	static void			schema(const VuJsonContainer &creationInfo, VuJsonContainer &schema);
	static bool			bake(const VuJsonContainer &creationInfo, VuAssetBakeParams &bakeParams);

private:
	virtual bool		load(VuBinaryDataReader &reader);
	virtual void		unload();

	typedef std::vector<VuEvent> Events;

	Events				mEvents;
};
