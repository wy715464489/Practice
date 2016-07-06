//*****************************************************************************
//
//  Copyright (c) 2008-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  List container
// 
//*****************************************************************************

#pragma once


template<class T>
class VuList
{
public:
	inline VuList() : mpHead(VUNULL), mpTail(VUNULL), mSize(0) {}

	inline void		clear()	{ mpHead = VUNULL; mpTail = VUNULL; mSize = 0; }

	inline void		push_front(T *pElement);
	inline void		push_back(T *pElement);

	inline T		*pop_front();
	inline T		*pop_back();

	inline void		insertBefore(T *pElement, T *pNext);
	inline void		insertAfter(T *pElement, T *pPrev);
	inline void		remove(T *pElement);

	inline int		size() const	{ return mSize; }
	inline T		*get(int index);
	inline const T	*get(int index) const;

	inline T		*front()		{ return mpHead; }
	inline const T	*front() const	{ return mpHead; }

	inline T		*back()			{ return mpTail; }
	inline const T	*back() const	{ return mpTail; }

private:
	T	*mpHead;
	T	*mpTail;
	int	mSize;
};


template<class T>
class VuListElement
{
public:
	inline T		*next()			{ return mpNext; }
	inline const T	*next() const	{ return mpNext; }

	inline T		*prev()			{ return mpPrev; }
	inline const T	*prev() const	{ return mpPrev; }

protected:
	friend class VuList<T>;

	T	*mpNext;
	T	*mpPrev;
};




#include "VuList.inl"
