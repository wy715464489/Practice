//*****************************************************************************
//
//  Copyright (c) 2009-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  Object array inline implementation
// 
//*****************************************************************************


//*****************************************************************************
template<typename T>
VuObjectArray<T>::VuObjectArray(const VuObjectArray &otherArray)
{
	init();

	int otherSize = otherArray.size();
	resize(otherSize);
	otherArray.copy(0, otherSize, mpData);
}

//*****************************************************************************
template<typename T>
void VuObjectArray<T>::initializeFromBuffer(void *buffer, int size, int capacity)
{
	clear();
	mbOwnsMemory = false;
	mpData = (T *)buffer;
	mSize = size;
	mCapacity = capacity;
}

//*****************************************************************************
template<typename T>
void VuObjectArray<T>::clear()
{
	destroy(0,size());
	deallocate();
	init();
}

//*****************************************************************************
template<typename T>
void VuObjectArray<T>::pop_back()
{
	mSize--;
	mpData[mSize].~T();
}

//*****************************************************************************
template<typename T>
void VuObjectArray<T>::resize(int newsize, const T &fillData)
{
	int curSize = size();

	if ( newsize < size() )
	{
		for( int i = curSize; i < newsize; i++ )
			mpData[i].~T();
	}
	else
	{
		if ( newsize > size() )
			reserve(newsize);

		for ( int i = curSize; i < newsize; i++ )
			new (&mpData[i]) T(fillData);
	}

	mSize = newsize;
}

//*****************************************************************************
template<typename T>
T &VuObjectArray<T>::expand(const T& fillValue)
{	
	if ( mSize == capacity() )
		reserve(allocSize(mSize));

	new (&mpData[mSize]) T(fillValue); //use the in-place new (not really allocating heap memory)

	return mpData[mSize++];		
}

//*****************************************************************************
template<typename T>
void VuObjectArray<T>::push_back(const T& val)
{	
	if ( mSize == capacity() )
		reserve(allocSize(mSize));
	
	new (&mpData[mSize]) T(val);

	mSize++;
}

//*****************************************************************************
template<typename T>
void VuObjectArray<T>::reserve(int count)
{
	if ( capacity() < count )
	{
		T *s = (T *)allocate(count);
		copy(0, size(), s);
		destroy(0,size());
		deallocate();
		mbOwnsMemory = true;
		mpData = s;
		mCapacity = count;
	}
}

//*****************************************************************************
template<typename T>
void VuObjectArray<T>::remove(const T &key)
{
	int findIndex = findLinearSearch(key);
	if ( findIndex < size() )
	{
		swap(findIndex, size() - 1);
		pop_back();
	}
}

//*****************************************************************************
template <typename T>
template <typename L>
void VuObjectArray<T>::quickSortInternal(L CompareFunc, int lo, int hi)
{
//  lo is the lower index, hi is the upper index
//  of the region of array a that is to be sorted
	int i=lo, j=hi;
	T x=mpData[(lo+hi)/2];

	//  partition
	do
	{    
		while (CompareFunc(mpData[i],x)) 
			i++; 
		while (CompareFunc(x,mpData[j])) 
			j--;
		if (i<=j)
		{
			swap(i,j);
			i++; j--;
		}
	} while (i<=j);

	//  recursion
	if (lo<j) 
		quickSortInternal(CompareFunc, lo, j);
	if (i<hi) 
		quickSortInternal(CompareFunc, i, hi);
}

//*****************************************************************************
template <typename T>
template <typename L>
void VuObjectArray<T>::quickSort(L CompareFunc)
{
	if ( size() > 1)
		quickSortInternal(CompareFunc, 0, size() - 1);
}

//*****************************************************************************
///heap sort from http://www.csse.monash.edu.au/~lloyd/tildeAlgDS/Sort/Heap/
//*****************************************************************************
template <typename T>
template <typename L>
void VuObjectArray<T>::downHeap(T *pArr, int k, int n,L CompareFunc)
{
	/*  PRE: a[k+1..N] is a heap */
	/* POST:  a[k..N]  is a heap */
	
	T temp = pArr[k - 1];
	/* k has child(s) */
	while (k <= n/2) 
	{
		int child = 2*k;
		
		if ((child < n) && CompareFunc(pArr[child - 1] , pArr[child]))
		{
			child++;
		}
		/* pick larger child */
		if (CompareFunc(temp , pArr[child - 1]))
		{
			/* move child up */
			pArr[k - 1] = pArr[child - 1];
			k = child;
		}
		else
		{
			break;
		}
	}
	pArr[k - 1] = temp;
} /*downHeap*/

//*****************************************************************************
template <typename T>
void VuObjectArray<T>::swap(int index0, int index1)
{
	if (index0 == index1)
		return;

	char temp[sizeof(T)];
	memmove(temp,&mpData[index0],sizeof(T));
	memmove(&mpData[index0],&mpData[index1],sizeof(T));
	memmove(&mpData[index1],temp,sizeof(T));
}

//*****************************************************************************
template <typename T>
template <typename L>
void VuObjectArray<T>::heapSort(L CompareFunc)
{
	/* sort a[0..N-1],  N.B. 0 to N-1 */
	int k;
	int n = mSize;
	for (k = n/2; k > 0; k--) 
	{
		downHeap(mpData, k, n, CompareFunc);
	}

	/* a[1..N] is now a heap */
	while ( n>=1 ) 
	{
		swap(0,n-1); /* largest of a[0..n-1] */

		n = n - 1;
		/* restore a[1..i-1] heap */
		downHeap(mpData, 1, n, CompareFunc);
	} 
}

//*****************************************************************************
///non-recursive binary search, assumes sorted array
//*****************************************************************************
template <typename T>
int	VuObjectArray<T>::findBinarySearch(const T& key) const
{
	int first = 0;
	int last = size();

	//assume sorted array
	while (first <= last) {
		int mid = (first + last) / 2;  // compute mid point.
		if (key > mpData[mid]) 
			first = mid + 1;  // repeat search in top half.
		else if (key < mpData[mid]) 
			last = mid - 1; // repeat search in bottom half.
		else
			return mid;     // found it. return position /////
	}
	return size();    // failed to find key
}

//*****************************************************************************
template <typename T>
int	VuObjectArray<T>::findLinearSearch(const T& key) const
{
	int index=size();
	int i;

	for (i=0;i<size();i++)
	{
		if (mpData[i] == key)
		{
			index = i;
			break;
		}
	}
	return index;
}

//*****************************************************************************
template<typename T>
void VuObjectArray<T>::copy(int start, int end, T *dest) const
{
	int i;
	for ( i = start; i < end; i++)
		new (&dest[i]) T(mpData[i]);
}

//*****************************************************************************
template<typename T>
void VuObjectArray<T>::init()
{
	mSize = 0;
	mCapacity = 0;
	mpData = VUNULL;
	mbOwnsMemory = true;
}

//*****************************************************************************
template<typename T>
void VuObjectArray<T>::destroy(int first, int last)
{
	int i;
	for ( i = first; i < last; i++)
		mpData[i].~T();
}

//*****************************************************************************
template<typename T>
void *VuObjectArray<T>::allocate(int size)
{
	if ( size )
		return VU_ALIGNED_ALLOC(size*sizeof(T), 16);
	return 0;
}

//*****************************************************************************
template<typename T>
void VuObjectArray<T>::deallocate()
{
	if ( mbOwnsMemory )
		VU_ALIGNED_FREE(mpData);
	mpData = 0;
}
