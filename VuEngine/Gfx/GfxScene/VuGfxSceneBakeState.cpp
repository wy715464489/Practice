//*****************************************************************************
//
//  Copyright (c) 2012-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  Bake State class
// 
//*****************************************************************************

#include "VuGfxSceneBakeState.h"


//*****************************************************************************
int VuGfxSceneBakeState::chunkIndex(const std::string &shaderFileName) const
{
	for ( int i = 0; i < (int)mChunks.size(); i++ )
		if ( mChunks[i].mShaderFileName == shaderFileName )
			return i;
	return -1;
}
