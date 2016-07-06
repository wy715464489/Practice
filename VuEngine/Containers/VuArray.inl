//*****************************************************************************
//
//  Copyright (c) 2008-2008 Ralf Knoesel
//  Copyright (c) 2008-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  Array container inline implementation
// 
//*****************************************************************************


//*****************************************************************************
template<class T>
VuArray<T>::VuArray(int initialCapacity):
	mpData(VUNULL),
	mSize(0),
	mCapacity(0)
{
	reserve(initialCapacity);
}

//*****************************************************************************
template<class T>
VuArray<T>::~VuArray()
{
	VU_ALIGNED_FREE(mpData);
}

//*****************************************************************************
template<class T>
inline void VuArray<T>::deallocate()
{
	VU_ALIGNED_FREE(mpData);
	mpData = VUNULL;
	mSize = 0;
	mCapacity = 0;
}

//*****************************************************************************
template<class T>
void VuArray<T>::resize(int newSize)
{
	if ( newSize > capacity() )
	{
		int newCapacity = capacity() + capacity()/2;	// grow by at least 50%
		if ( newCapacity < 8 )
			newCapacity = 8;
		if ( newSize > newCapacity )
			newCapacity = newSize;
		reserve(newCapacity);
	}

	mSize = newSize;
}

//*****************************************************************************
template<class T>
void VuArray<T>::reserve(int newCapacity)
{
	if ( newCapacity > capacity() )
	{
		T *pData = reinterpret_cast<T *>(VU_ALIGNED_ALLOC(newCapacity*sizeof(T), 16));
		VU_MEMCPY(pData, size()*sizeof(T), mpData, size()*sizeof(T));
		VU_ALIGNED_FREE(mpData);
		mpData = pData;
		mCapacity = newCapacity;
	}
}

//*****************************************************************************
template<class T>
void VuArray<T>::push_back(const T& val)
{
	resize(size() + 1);
	back() = val;
}

//*****************************************************************************
template<class T>
void VuArray<T>::erase(int start, int count)
{
	VUASSERT(start >= 0 && count > 0 && start + count <= size(), "VuArray::erase() invalid offset");

	memmove(&mpData[start], &mpData[start + count], (size() - start - count)*sizeof(T));
	resize(size() - count);
}

//*****************************************************************************
template<class T>
void VuArray<T>::eraseSwap(int pos)
{
	VUASSERT(pos >= 0 && pos < size(), "VuArray::erase() invalid offset");

	if ( pos + 1 < size() )
		VuSwap(mpData[pos], back());
	resize(size() - 1);
}

//*****************************************************************************
template<class T>
void VuArray<T>::remove(const T &val)
{
	for ( int i = 0; i < size(); i++ )
		if ( mpData[i] == val )
			return erase(i);
}

//*****************************************************************************
template<class T>
void VuArray<T>::removeSwap(const T &val)
{
	for ( int i = 0; i < size(); i++ )
		if ( mpData[i] == val )
			return eraseSwap(i);
}

//*****************************************************************************
template<class T>
T &VuArray<T>::operator[](int offset)
{
	VUASSERT(offset >= 0 && offset < size(), "VuArray::[] invalid offset");

	return mpData[offset];
}

//*****************************************************************************
template<class T>
const T &VuArray<T>::operator[](int offset) const
{
	VUASSERT(offset >= 0 && offset < size(), "VuArray::[] invalid offset");

	return mpData[offset];
}
