//*****************************************************************************
//
//  Copyright (c) 2011-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  Objective-C support for Ios File interface.
//
//*****************************************************************************


#include "VuIosFileObjC.h"


//*****************************************************************************
bool VuIosFileObjC::GetDocumentPath(char *path, int size)
{
	bool success = false;
	NSArray *paths = NSSearchPathForDirectoriesInDomains(NSDocumentDirectory, NSUserDomainMask, YES);
	
	if ( [paths count] > 0 )
	{
		NSString *basePath = [paths objectAtIndex:0];
		const char *strPath = [basePath UTF8String];
		
		if ( strlen(strPath) < size )
		{
			strcpy(path, strPath);
			success = true;
		}
	}
	
	return success;
}
