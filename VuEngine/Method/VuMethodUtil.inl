//*****************************************************************************
//
//  Copyright (c) 2008-2008 Ralf Knoesel
//  Copyright (c) 2008-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  Method utility inline functionality
// 
//*****************************************************************************


//*****************************************************************************
bool isMethodOfObjectWithDelete::operator()(VuMethodInterface *pMethod)
{
	if ( pMethod->getObj() != mpObj )
		return false;

	delete pMethod;

	return true;
}
