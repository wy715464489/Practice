//*****************************************************************************
//
//  Copyright (c) 2009-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  Audio property classes
// 
//*****************************************************************************

#pragma once

#include "VuProperty.h"
#include "VuEnumProperty.h"


class VuAudioNameProperty : public VuStringEnumProperty
{
public:
	VuAudioNameProperty(const char *strName, std::string &pValue) : VuStringEnumProperty(strName, pValue) {}

	virtual eType		getType() const	{ return AUDIO; }
};


class VuAudioEventNameProperty : public VuAudioNameProperty
{
public:
	VuAudioEventNameProperty(const char *strName, std::string &pValue) : VuAudioNameProperty(strName, pValue) {}

	virtual int			getChoiceCount() const;
	virtual const char	*getChoice(int index) const;
};

class VuAudioReverbNameProperty : public VuAudioNameProperty
{
public:
	VuAudioReverbNameProperty(const char *strName, std::string &pValue) : VuAudioNameProperty(strName, pValue) {}

	virtual int			getChoiceCount() const;
	virtual const char	*getChoice(int index) const;
};
