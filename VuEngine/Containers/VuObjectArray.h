//*****************************************************************************
//
//  Copyright (c) 2009-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  Object array
//
//  The VuObjectArray template class uses a subset of the stl::vector interface for its methods.
//  It is developed to replace stl::vector to avoid portability issues, including STL alignment issues to add SIMD/SSE data.
//
//  Based on btAlignedObjectArray.h in the Bullet SDK:
// 
//  Bullet Continuous Collision Detection and Physics Library
//  Copyright (c) 2003-2006 Erwin Coumans  http://continuousphysics.com/Bullet/
//
//  This software is provided 'as-is', without any express or implied warranty.
//  In no event will the authors be held liable for any damages arising from the use of this software.
//  Permission is granted to anyone to use this software for any purpose, 
//  including commercial applications, and to alter it and redistribute it freely, 
//  subject to the following restrictions:
//
//  1. The origin of this software must not be misrepresented; you must not claim that you wrote the original software. If you use this software in a product, an acknowledgment in the product documentation would be appreciated but is not required.
//  2. Altered source versions must be plainly marked as such, and must not be misrepresented as being the original software.
//  3. This notice may not be removed or altered from any source distribution.
//*****************************************************************************

#pragma once


template <typename T> 
class VuObjectArray
{
public:
	VuObjectArray()		{ init(); }
	~VuObjectArray()	{ clear(); }

	///Generally it is best to avoid using the copy constructor of an VuObjectArray, and use a (const) reference to the array instead.
	VuObjectArray(const VuObjectArray &otherArray);

	inline void		initializeFromBuffer(void *buffer, int size, int capacity);

	/// return the number of elements in the array
	inline int		size() const { return mSize; }
	
	/// return the pre-allocated (reserved) elements, this is at least as large as the total number of elements,see size() and reserve()
	inline int		capacity() const { return mCapacity; }
	
	inline const T&	operator[](int n) const	{ return mpData[n]; }
	inline T&		operator[](int n)		{ return mpData[n]; }
	
	///clear the array, deallocated memory. Generally it is better to use array.resize(0), to reduce performance overhead of run-time memory (de)allocations.
	inline void		clear();

	inline void		pop_back();

	///resize changes the number of elements in the array. If the new size is larger, the new elements will be constructed using the optional second argument.
	///when the new number of elements is smaller, the destructor will be called, but memory will not be freed, to reduce performance overhead of run-time memory (de)allocations.
	inline void		resize(int newsize, const T &fillData = T());

	inline T		&expand(const T &fillValue = T());

	inline void		push_back(const T &val);

	inline void		reserve(int count);

	inline void		remove(const T &key);

	template <typename L>
	void			quickSortInternal(L CompareFunc, int lo, int hi);

	template <typename L>
	void			quickSort(L CompareFunc);

	///heap sort from http://www.csse.monash.edu.au/~lloyd/tildeAlgDS/Sort/Heap/
	template <typename L>
	void			downHeap(T *pArr, int k, int n, L CompareFunc);

	void			swap(int index0,int index1);

	template <typename L>
	void			heapSort(L CompareFunc);

	int				findBinarySearch(const T& key) const;

	int				findLinearSearch(const T& key) const;

private:
	inline int		allocSize(int size) { return (size ? size*2 : 1); }
	inline void		copy(int start,int end, T* dest) const;
	inline void		init();
	inline void		destroy(int first,int last);
	inline void		*allocate(int size);
	inline void		deallocate();

	int				mSize;
	int				mCapacity;
	T*				mpData;
	bool			mbOwnsMemory;
};

#include "VuObjectArray.inl"
