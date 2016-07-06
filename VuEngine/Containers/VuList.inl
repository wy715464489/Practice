//*****************************************************************************
//
//  Copyright (c) 2008-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  List container inline implementation
// 
//*****************************************************************************


//*****************************************************************************
template<class T> void VuList<T>::push_front(T *pElement)
{
	if ( mpHead )
	{
		pElement->mpPrev = VUNULL;
		pElement->mpNext = mpHead;
		mpHead->mpPrev = pElement;
		mpHead = pElement;
	}
	else
	{
		pElement->mpPrev = VUNULL;
		pElement->mpNext = VUNULL;
		mpHead = mpTail = pElement;
	}

	mSize++;
}

//*****************************************************************************
template<class T> void VuList<T>::push_back(T *pElement)
{
	if ( mpTail )
	{
		pElement->mpPrev = mpTail;
		pElement->mpNext = VUNULL;
		mpTail->mpNext = pElement;
		mpTail = pElement;
	}
	else
	{
		pElement->mpPrev = VUNULL;
		pElement->mpNext = VUNULL;
		mpHead = mpTail = pElement;
	}

	mSize++;
}

//*****************************************************************************
template<class T> T *VuList<T>::pop_front()
{
	T *pElement = mpHead;

	if ( pElement )
		remove(pElement);

	return pElement;
}

//*****************************************************************************
template<class T> T *VuList<T>::pop_back()
{
	T *pElement = mpTail;

	if ( pElement )
		remove(pElement);

	return pElement;
}

//*****************************************************************************
template<class T> void VuList<T>::insertBefore(T *pElement, T *pNext)
{
	if ( pNext == mpHead )
	{
		push_front(pElement);
	}
	else
	{
		T *pPrev = pNext->mpPrev;

		pPrev->mpNext = pElement;
		pNext->mpPrev = pElement;

		pElement->mpNext = pNext;
		pElement->mpPrev = pPrev;

		mSize++;
	}
}

//*****************************************************************************
template<class T> void VuList<T>::insertAfter(T *pElement, T *pPrev)
{
	if ( pPrev == mpTail )
	{
		push_back(pElement);
	}
	else
	{
		T *pNext = pPrev->mpNext;

		pPrev->mpNext = pElement;
		pNext->mpPrev = pElement;

		pElement->mpNext = pNext;
		pElement->mpPrev = pPrev;

		mSize++;
	}
}

//*****************************************************************************
template<class T> void VuList<T>::remove(T *pElement)
{
	T *pNext = pElement->mpNext;
	T *pPrev = pElement->mpPrev;

	if ( pElement == mpHead )
		mpHead = pNext;
	if ( pElement == mpTail )
		mpTail = pPrev;

	if ( pNext )
		pNext->mpPrev = pPrev;
	if ( pPrev)
		pPrev->mpNext = pNext;

	pElement->mpNext = VUNULL;
	pElement->mpPrev = VUNULL;

	mSize--;
}

//*****************************************************************************
template<class T> T *VuList<T>::get(int index)
{
	if ( index < 0 || index >= size() )
		return VUNULL;

	T *p = mpHead;
	while ( index )
	{
		p = p->mpNext;
		index--;
	}

	return p;
}

//*****************************************************************************
template<class T> const T *VuList<T>::get(int index) const
{
	if ( index < 0 || index >= size() )
		return VUNULL;

	const T *p = mpHead;
	while ( index )
	{
		p = p->mpNext;
		index--;
	}

	return p;
}