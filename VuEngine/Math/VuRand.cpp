//*****************************************************************************
//
//  Copyright (c) 2008-2008 Ralf Knoesel
//  Copyright (c) 2008-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  Random number functionality
// 
// "Minimal" random number generator of Park and Miller with Bays-Durham shuffle and added
// safeguards.
// 
// [Numerical Recipes in C, the Art of Scientific Computing (2nd Edition)]
//*****************************************************************************

#include "VuRand.h"
#include "VuMath.h"
#include "VuEngine/Math/VuVector2.h"
#include "VuEngine/Math/VuVector3.h"


// static members
VuRand VuRand::mGlobalRand(-1);


#define VURAND_IA 16807
#define VURAND_IM 2147483647
#define VURAND_AM (1.0f/VURAND_IM)
#define VURAND_IQ 127773
#define VURAND_IR 2836
#define VURAND_NDIV (1+(VURAND_IM-1)/VURAND_NTAB)
#define VURAND_EPS 1.2e-7f
#define VURAND_RNMX (1.0f-VURAND_EPS)


//*****************************************************************************
VuRand::VuRand(int seed):
	midum(0),
	miy(0),
	miset(0)
{
	reseed(seed);
}

//*****************************************************************************
void VuRand::reseed(VUINT32 seed)
{
	midum = seed;

	if ( midum == 0 )
	{
		VUUINT64 perf = VuSys::IF()->getPerfCounter();
		midum = (perf & 0xffffffff) + (perf >> 32);
	}

	midum |= 0x80000000;

	// load shuffle table (after 8 warm-ups)
	for ( int j = VURAND_NTAB + 7; j >= 0; j-- )
	{
		VUINT32 k = midum/VURAND_IQ;
		midum = VURAND_IA*(midum - k*VURAND_IQ) - VURAND_IR*k;
		if ( midum < 0 )
			midum += VURAND_IM;
		if ( j < VURAND_NTAB )
			miv[j] = midum;
	}
	miy = miv[0];
}

//*****************************************************************************
float VuRand::rand()
{
	// start here when not initializing
	VUINT32 k = midum/VURAND_IQ;

	// compute idum=(IA*idum)%IM without overflows by Schrage's method
	midum = VURAND_IA*(midum - k*VURAND_IQ) - VURAND_IR*k;
	if ( midum < 0 )
		midum += VURAND_IM;

	// will be in the range 0..NTAB-1
	VUINT32 j = miy/VURAND_NDIV;

	// output previously stored value and refill the shuffle table
	miy = miv[j];
	miv[j] = midum;
	
	// users don't expect endpoint values
	return VuMin(VURAND_AM*miy, VURAND_RNMX);
}

//*****************************************************************************
float VuRand::gaussRand()
{
	float fac, rsq, v1, v2;

	if ( miset == 0 )
	{
		// we don't have an extra deviate handy, so pick two uniform numbers in the
		// square extending from -1 to +1 in each direction, see if they are in the
		// unit circle, and if they are not, try again.
		do
		{
			v1 = 2.0f*rand() - 1.0f;
			v2 = 2.0f*rand() - 1.0f;
			rsq = v1*v1 + v2*v2;
		} while ( rsq >= 1.0f || rsq == 0.0f );

		// now make the Box-Muller transformation to get two normal deviates.  Return
		// one and save the other for next time.
		fac = VuSqrt(-2.0f*VuLog(rsq)/rsq);
		mgset = v1*fac;
		miset = 1;
		return v2*fac;
	}
	else
	{
		// we have an extra deviate handy, so unset the flag, and return it.
		miset = 0;
		return mgset;
	}
}

//*****************************************************************************
// Creates a shuffled array of unique indices from 0 to count-1
void VuRand::createShuffleArray(int count, int *pArray)
{
	for ( int i = 0; i < count; i++ )
		pArray[i] = i;
	for ( int i = 0; i < count - 1; i++ )
	{
		int j = i + (VuTruncate(count*rand())%(count-i));
		VuSwap(pArray[i], pArray[j]);
	}
}

//*****************************************************************************
void VuRand::randomDirection2d(VuVector2 &dir)
{
  float azimuth = 2.0f*VU_PI*rand();
  VuSinCos(azimuth, dir.mY, dir.mX);
}

//*****************************************************************************
void VuRand::randomDirection3d(VuVector3 &dir)
{
	float z = 2.0f*rand() - 1.0f; // z is in range [-1,1]

	VuVector2 planar;
	randomDirection2d(planar);
	planar *= VuSqrt(1.0f - z*z);

	dir.mX = planar.mX;
	dir.mY = planar.mY;
	dir.mZ = z;
}

//*****************************************************************************
// Not sure if this distribution is adequate yet (todo: plot/investigate)
void VuRand::randomOrientation(VuVector3 &euler)
{
	euler.mX = VU_2PI*rand() - VU_PI;
	euler.mY = VU_2PI*rand() - VU_PI;
	euler.mZ = VU_2PI*rand() - VU_PI;
}

//*****************************************************************************
int VuRand::range(int min, int max)
{
	float fval = min + rand()*(max - min);
	fval = VuFloor(fval);
	int ival = VuRound(fval);
	ival = VuClamp(ival, min, max - 1);
	return ival;
}

//*****************************************************************************
float VuRand::range(float min, float max)
{
	return min + rand()*(max - min);
}