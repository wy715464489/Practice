//*****************************************************************************
//
//  Copyright (c) 2008-2008 Ralf Knoesel
//  Copyright (c) 2008-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  Pointer list inline implementation
// 
//*****************************************************************************


//*****************************************************************************
template<class T>
void VuPointerList<T>::add(T *pValue, VuPointerFreeList &freeList)
{
	// take element from free list
	Node *pFree = freeList.pop<T>();

	// set value
	pFree->mpValue = pValue;

	// insert
	pFree->mpNext = mpHead;
	mpHead = pFree;
}

//*****************************************************************************
template<class T>
bool VuPointerList<T>::remove(T *pValue, VuPointerFreeList &freeList)
{
	// find in list
	Node *p;
	Node *pPrev = 0;
	for ( p = mpHead; p; pPrev = p, p = p->mpNext )
		if ( p->mpValue == pValue )
			break;

	if ( p == VUNULL )
		return false;

	// remove from list
	if ( pPrev )
		pPrev->mpNext = p->mpNext;
	else
		mpHead = p->mpNext;

	// insert back into free list
	freeList.push<T>(p);

	return true;
}

//*****************************************************************************
template<class T>
void VuPointerList<T>::clear(VuPointerFreeList &freeList)
{
	while ( mpHead )
	{
		// pop head off list
		Node *pFree = mpHead;
		mpHead = mpHead->mpNext;

		// insert into free list
		freeList.push<T>(pFree);
	}
}

//*****************************************************************************
template<class T>
int VuPointerList<T>::size( void )
{
	int size = 0;
	for ( Node *p = mpHead; p; p = p->mpNext )
		size++;

	return size;
}

//*****************************************************************************
template<class T>
typename VuPointerList<T>::Node *VuPointerFreeList::pop()
{
	VUASSERT(mpHead, "VuPointerFreeList::pop() out of free elements");

	typename VuPointerList<T>::Node *pNode = (typename VuPointerList<T>::Node *)mpHead;
	mpHead = mpHead->mpNext;

	mFreeElementCount--;

	return pNode;
}

//*****************************************************************************
template<class T>
void VuPointerFreeList::push(typename VuPointerList<T>::Node *pNode)
{
	pNode->mpNext = (typename VuPointerList<T>::Node *)mpHead;
	mpHead = (typename VuPointerList<void>::Node *)pNode;

	mFreeElementCount++;
}
