//*****************************************************************************
//
//  Copyright (c) 2008-2008 Ralf Knoesel
//  Copyright (c) 2008-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  FFT functionality
// 
//*****************************************************************************

#pragma once


//*****************************************************************************
// Replaces 'data' by its 'ndim'-dimensional discrete Fourier transform, if 'isign' is input as 1.
// If 'isign' is input as -1, data is replaced by its inverse transform times the product of these
// lengths of all dimensions.
// [Numerical Recipes in C, the Art of Scientific Computing (2nd Edition)]
//*****************************************************************************
void VuFFT(float data[], unsigned long nn[], int ndim, int isign);

//*****************************************************************************
// Given a three-dimensional real array, this routine returns (for isign-1) the complex
// fast Fourier transform as two complex arrays.  For isign=-1, the inverse transform
// (times nn1*nn2*nn3/2 as a constant multiplicative factor) is performed.
// [Numerical Recipes in C, the Art of Scientific Computing (2nd Edition)]
//*****************************************************************************
void VuFFTReal3(float ***data, float **speq, unsigned long nn1, unsigned long nn2, unsigned long nn3, int isign);

//*****************************************************************************
// allocate/free a float tensor with range t[nrl..nrh][ncl..nch]
//*****************************************************************************
float **VuFFTAllocateFloatTensor2(long nrl, long nrh, long ncl, long nch);
void VuFFTFreeFloatTensor2(float **t, long nrl, long nrh, long ncl, long nch);

//*****************************************************************************
// allocate/free a float tensor with range t[nrl..nrh][ncl..nch][ndl..ndh]
//*****************************************************************************
float ***VuFFTAllocateFloatTensor3(long nrl, long nrh, long ncl, long nch, long ndl, long ndh);
void VuFFTFreeFloatTensor3(float ***t, long nrl, long nrh, long ncl, long nch, long ndl, long ndh);
