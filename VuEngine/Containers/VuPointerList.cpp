//*****************************************************************************
//
//  Copyright (c) 2008-2008 Ralf Knoesel
//  Copyright (c) 2008-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  Poitner list
// 
//*****************************************************************************

#include "VuPointerList.h"


//*****************************************************************************
VuPointerFreeList::VuPointerFreeList(int maxElementCount):
	mMaxElementCount(maxElementCount),
	mFreeElementCount(maxElementCount)
{
	// allocate
	mpNodes = new VuPointerList<void>::Node[maxElementCount];

	// set pointers
	for ( int i = 0; i < maxElementCount; i++ )
		mpNodes[i].mpNext = &mpNodes[i+1];
	mpNodes[maxElementCount - 1].mpNext = VUNULL;

	// set head
	mpHead = mpNodes;
}

//*****************************************************************************
VuPointerFreeList::~VuPointerFreeList()
{
	VUASSERT(mFreeElementCount == mMaxElementCount, "VuPointerFreeList leak");

	delete[] mpNodes;
}
