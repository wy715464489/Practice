//*****************************************************************************
//
//  Copyright (c) 2008-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  RadixSort
// 
//*****************************************************************************

#include "VuRadixSort.h"
#include "VuEngine/Math/VuMath.h"


void VuRadixSort::sort(VUUINT64 *input, int count, int *outputOrder, int swapMemorySize, void *pSwapMemory)
{
	unsigned int counter[256];
	unsigned int offset[256];
	VUBYTE *pByte;

	// determine max # values from scratch pad size
	int maxCount = swapMemorySize/(8 + 4);
	if ( count > maxCount )
		VUPRINTF("WARNING: VuRadixSort::sort() swap space too small.\n");
	count = VuMin(count, maxCount);

	// use scratch pad for swap space
	VUUINT64 *valueSwap = static_cast<VUUINT64 *>(pSwapMemory);
	int *orderSwap = reinterpret_cast<int *>(valueSwap + count);

	// for each byte position
#if VU_LITTLE_ENDIAN 
	for ( int pos = 0; pos < 8; pos++ )
#elif VU_BIG_ENDIAN
	for ( int pos = 7; pos >=0 ; pos-- )
#else
	#error Endianness not defined!
#endif
	{
		// count how many values for each byte
		memset(counter, 0, sizeof(counter));
		for ( int i = 0; i < count; i++ )
		{
			pByte = ((VUBYTE *)&input[i]) + pos;
			counter[*pByte]++;
		}

		// calculate offset based on counts
		offset[0] = 0;
		for ( int i = 1; i < 256; i++ )
			offset[i] = offset[i - 1] + counter[i - 1];

		// do the sort for this byte position
		for ( int i = 0; i < count; i++ )
		{
			pByte = ((VUBYTE *)&input[i]) + pos;
			valueSwap[offset[*pByte]] = input[i];
			orderSwap[offset[*pByte]] = outputOrder[i];
			offset[*pByte]++;
		}

		// swap
		VuSwap<VUUINT64 *>(input, valueSwap);
		VuSwap<int *>(outputOrder, orderSwap);
	}
}
