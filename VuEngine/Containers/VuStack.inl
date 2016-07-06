//*****************************************************************************
//
//  Copyright (c) 2008-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  Stack container inline implementation
// 
//*****************************************************************************

#include "VuEngine/Math/VuMath.h"


//*****************************************************************************
template<class T, int MAX_SIZE>
VuStaticStack<T, MAX_SIZE>::VuStaticStack():
	mSize(0)
{
}

//*****************************************************************************
template<class T, int MAX_SIZE>
VuStaticStack<T, MAX_SIZE>::~VuStaticStack()
{
	clear();
}

//*****************************************************************************
template<class T, int MAX_SIZE>
void VuStaticStack<T, MAX_SIZE>::clear()
{
	// call destructors
	while ( size() )
		pop();
}

//*****************************************************************************
template<class T, int MAX_SIZE>
void VuStaticStack<T, MAX_SIZE>::push(const T& val)
{
	VUASSERT(size() < MAX_SIZE, "VuStaticStack::push() overflow");

	mSize = size() + 1;

	// copy-construct new object at top
	new (&top()) T(val);
}

//*****************************************************************************
template<class T, int MAX_SIZE>
void VuStaticStack<T, MAX_SIZE>::pop()
{
	VUASSERT(mSize > 0, "VuStaticStack::pop() called on empty stack");

	// in-place destruct top
	top().~T();

	mSize--;
}

//*****************************************************************************
template<class T, int MAX_SIZE>
T &VuStaticStack<T, MAX_SIZE>::top()
{
	VUASSERT(mSize > 0, "VuStaticStack::top() called on empty stack");

	return mMemory[size() - 1];
}

//*****************************************************************************
template<class T, int MAX_SIZE>
const T &VuStaticStack<T, MAX_SIZE>::top() const
{
	VUASSERT(mSize > 0, "VuStaticStack::top() called on empty stack");

	return mMemory[size() - 1];
}
