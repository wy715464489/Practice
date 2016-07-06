//*****************************************************************************
//
//  Copyright (c) 2008-2015 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  Class to manage a pool of memory, allowing alloc() and dealloc()
//  calls.
// 
//*****************************************************************************

#pragma once

class VuMemoryManager
{
public:
	VuMemoryManager() : mpPool(VUNULL), mBytesUsed(0) {}

	// init/release
	bool	init(void *pPool, size_t poolSize);
	void	release();

	// allocate/deallocate
	void	*allocate(size_t bytes, size_t alignment = 16);
	void	deallocate(void *p);

	// See notes below
	void	*reallocate(void *p, size_t bytes);

	// stats
	struct Stats
	{
		size_t	mBytesUsed;
		size_t	mBytesFree;
	};
	void	getStats(Stats &stats);

private:

	void	*mpPool;
	size_t	mBytesUsed;
};


//*****************************************************************************
//	void *VuMemoryManager::realloc(void *p, SFINT bytes)
//
//	Returns a pointer to a chunk of size n that contains the same data
//	as does chunk p up to the minimum of (n, p's size) bytes, or null
//	if no space is available.
//
//	The returned pointer may or may not be the same as p. The algorithm
//	prefers extending p in most cases when possible, otherwise it
//	employs the equivalent of a malloc-copy-free sequence.
//
//	If p is null, realloc is equivalent to malloc.
//
//	If space is not available, realloc returns null, errno is set (if on
//	ANSI) and p is NOT freed.
//
//	if n is for fewer bytes than already held by p, the newly unused
//	space is lopped off and freed if possible.  realloc with a size
//	argument of zero (re)allocates a minimum-sized chunk.
//*****************************************************************************
