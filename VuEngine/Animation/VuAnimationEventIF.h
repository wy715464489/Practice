//*****************************************************************************
//
//  Copyright (c) 2009-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  Animation Event Interface class
// 
//*****************************************************************************

#pragma once

class VuJsonContainer;


//*****************************************************************************
class VuAnimationEventIF
{
	public:
		virtual void onAnimationEvent(const std::string &type, const VuJsonContainer &params) = 0;
};
