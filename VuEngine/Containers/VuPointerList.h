//*****************************************************************************
//
//  Copyright (c) 2008-2008 Ralf Knoesel
//  Copyright (c) 2008-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  Pointer list implementation
// 
//*****************************************************************************

#pragma once


class VuPointerFreeList;

template<class T>
class VuPointerList
{
public:
	class Node
	{
	public:
		T		*mpValue;
		Node	*mpNext;
	};

	VuPointerList() : mpHead(VUNULL) {}

	void	add(T *pValue, VuPointerFreeList &freeList);
	bool	remove(T *pValue, VuPointerFreeList &freeList);
	void	clear(VuPointerFreeList &freeList);
	int		size();

	Node	*mpHead;
};

class VuPointerFreeList
{
public:
	VuPointerFreeList(int maxElementCount);
	~VuPointerFreeList();

	template<class T> typename VuPointerList<T>::Node	*pop();
	template<class T> void								push(typename VuPointerList<T>::Node *pNode);

	int							getUsedElementCount()	{ return mMaxElementCount - mFreeElementCount; }
	int							getFreeElementCount()	{ return mFreeElementCount; }

private:
	int							mMaxElementCount;
	int							mFreeElementCount;
	VuPointerList<void>::Node	*mpNodes;
	VuPointerList<void>::Node	*mpHead;
};


#include "VuPointerList.inl"
