//*****************************************************************************
//
//  Copyright (c) 2008-2008 Ralf Knoesel
//  Copyright (c) 2008-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  Array container
// 
//*****************************************************************************

#pragma once


template<class T>
class VuArray
{
public:
	inline VuArray(int initialCapacity = 8);
	inline ~VuArray();

	inline void		clear()	{ mSize = 0; }
	inline void		deallocate();

	inline void		resize(int newSize);
	inline void		reserve(int newCapacity);

	inline int		size() const		{ return mSize; }
	inline int		capacity() const	{ return mCapacity; }

	inline void		push_back(const T &val);
	inline void		erase(int start, int count = 1);
	inline void		eraseSwap(int pos);
	inline void		remove(const T &val);
	inline void		removeSwap(const T &val);

	inline T		&operator[](int offset);
	inline const T	&operator[](int offset) const;

	inline T		&front()		{ return mpData[0]; }
	inline const T	&front() const	{ return mpData[0]; }

	inline T		&back()			{ return (*this)[size() - 1]; }
	inline const T	&back() const	{ return (*this)[size() - 1]; }

	inline T		&begin()		{ return mpData[0]; }
	inline const T	&begin() const	{ return mpData[0]; }

	inline T		&end()			{ return mpData[size()]; }
	inline const T	&end() const	{ return mpData[size()]; }

private:
	T				*mpData;
	int				mSize;
	int				mCapacity;
};


#include "VuArray.inl"
