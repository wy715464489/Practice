//*****************************************************************************
//
//  Copyright (c) 2008-2008 Ralf Knoesel
//  Copyright (c) 2008-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  Method utility functionality
// 
//*****************************************************************************

#pragma once

#include "VuMethod.h"


//*****************************************************************************
// Predicate to test if a method belongs to an object.
//*****************************************************************************
class isMethodOfObject
{
public:
	isMethodOfObject(void *pObj) : mpObj(pObj) {}

	inline bool operator()(VuMethodInterface *pMethod) { return pMethod->getObj() == mpObj; }

private:
	void	*mpObj;
};


//*****************************************************************************
// Predicate to test if a method belongs to an object.  Also deletes the method.
//*****************************************************************************
class isMethodOfObjectWithDelete
{
public:
	isMethodOfObjectWithDelete(void *pObj) : mpObj(pObj) {}

	inline bool operator()(VuMethodInterface *pMethod);

private:
	void	*mpObj;
};


#include "VuMethodUtil.inl"
