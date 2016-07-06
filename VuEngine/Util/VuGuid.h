//*****************************************************************************
//
//  Copyright (c) 2010-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  Guid class
// 
//*****************************************************************************

#pragma once


class VuGuid
{
public:
	VuGuid() : mValue0(0), mValue1(0), mValue2(0), mValue3(0)	{}

	std::string		toString() const;
	VUUINT32		hash() const;

	static VuGuid	create();

private:
	VUUINT32	mValue0;
	VUUINT32	mValue1;
	VUUINT32	mValue2;
	VUUINT32	mValue3;
};
