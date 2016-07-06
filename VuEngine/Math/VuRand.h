//*****************************************************************************
//
//  Copyright (c) 2008-2008 Ralf Knoesel
//  Copyright (c) 2008-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  Random number functionality
// 
//*****************************************************************************

#pragma once

class VuVector2;
class VuVector3;


#define VURAND_NTAB 32

class VuRand
{
public:
	VuRand(VUINT32 seed = 0);

	void	reseed(VUINT32 seed = 0);

	// Returns a uniform random deviate between 0.0 and 1.0 (exclusive of the endpoint
	// values).
	float	rand();

	// Returns a normally distributed deviate with zero mean and unit variance, using
	// raid() as the source of uniform deviates.
	float	gaussRand();

	// Creates a shuffled array of unique indices from 0 to count-1
	void	createShuffleArray(int count, int *pArray);

	void	randomDirection2d(VuVector2 &dir);
	void	randomDirection3d(VuVector3 &dir);
	void	randomOrientation(VuVector3 &euler);

	int		range(int min, int max); // Returns a random integer number between min (inclusive) and max (exclusive)
	float	range(float min, float max);

	static VuRand	&global() { return mGlobalRand; }

private:
	VUINT32	midum;
	VUINT32	miy;
	VUINT32	miv[VURAND_NTAB];
	VUINT32	miset;
	float	mgset;

	static VuRand	mGlobalRand;
};

