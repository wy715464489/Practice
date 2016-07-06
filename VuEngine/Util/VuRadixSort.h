//*****************************************************************************
//
//  Copyright (c) 2008-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  RadixSort
// 
//*****************************************************************************

#pragma once


namespace VuRadixSort
{
	void sort(VUUINT64 *input, int count, int *outputOrder, int swapMemorySize, void *pSwapMemory);
}
