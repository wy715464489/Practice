//*****************************************************************************
//
//  Copyright (c) 2008-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  Stack container
// 
//*****************************************************************************

#pragma once


template<class T, int MAX_SIZE>
class VuStaticStack
{
public:
	inline VuStaticStack();
	inline ~VuStaticStack();

	inline void		clear();

	inline int		size() const		{ return mSize; }
	inline int		capacity() const	{ return MAX_SIZE; }

	inline void		push(const T& val);
	inline void		pop();

	inline T		&top();
	inline const T	&top() const;

private:
	T				mMemory[MAX_SIZE];
	int				mSize;
};


#include "VuStack.inl"
