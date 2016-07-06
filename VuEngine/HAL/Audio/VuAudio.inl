//*****************************************************************************
//
//  Copyright (c) 2008-2008 Ralf Knoesel
//  Copyright (c) 2008-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  Audio interface inline functionality.
// 
//*****************************************************************************

//*****************************************************************************
FMOD_VECTOR VuAudio::toFmodVector(const VuVector3 &vec)
{
	FMOD_VECTOR out;

	out.x = vec.mX;
	out.y = vec.mY;
	out.z = vec.mZ;

	return out;
}

//*****************************************************************************
VuVector3 VuAudio::toVuVector3(const FMOD_VECTOR &vec)
{
	VuVector3 out;

	out.mX = vec.x;
	out.mY = vec.y;
	out.mZ = vec.z;

	return out;
}
