/***************************************************************\
*                        KoalaMud Gen 2                         *
*    Copyright (c) 2002 First Step Internet Services, Inc.      *
*                     All Rights Reserved                       *
*        Distributed under the terms of the FSI License         *
* See file LICENSE in the root of this package for information  *
\***************************************************************/
/***************************************************************\
*	Module:  Memory Handlers
*	Author:  Matthew Schlegel
* Description:  This file defines memory allocators and other memory handling
* classes.  There is a bit of code duplication that would be nice to
* eliminate, but I don't currently see a good way to do it.
* Classes:
\***************************************************************/

#include <malloc.h>
#include <string.h>
#include <math.h>
#include <zthread/Guard.h>
#include "memory.hxx"

namespace koalamud {

/** Pool Allocator Constructor
 *
 * This constructor sets up all of the internal state for the pool allocation
 * engine and allocates a block of memory for the tracking/stats overhead and
 * first usage block.
 *
 * @warning This function should *only* be called in a single threaded
 * environment.  Usage while multiple threads are excuting and have access to
 * it can lead to an unknown state and cause catastrophic failure.
 */
PoolAllocator::PoolAllocator(void)
{
	/* Lock the allocator while we set it up. */
	ZThread::Guard<ZThread::FastRecursiveMutex> guard(syslock);

	/* Because the allocation engine is not yet setup, we have to be very
	 * careful in what member functions we use.  Most of them rely on having all
	 * of the tracking overhead already setup. */

	unsigned int counter = 0;
	unsigned int statsoverhead = sizeof(T_StatsInfo) * maxblocksize;
	unsigned int poolalloc = poolallocsize;

	/* Figure out how big of a pool we need to accomidate the stats overhead */
	while (poolalloc < (statsoverhead + (5 * sizeof(T_PoolInfo))))
	{
		poolalloc += poolallocsize;
	}

	/* Create the pool and do some initialization */
	poollist = (T_PoolInfo *)malloc(poolalloc);
	if (poollist == NULL)
	{
		cout << "FATAL: PoolAllocator unable to allocate first memory pool!"
			<< endl;
		exit(-1);
	}
	/* Initialize the entire pool to 0's to save some time */
	bzero(poollist, poolalloc);
	/* Do basic pool initialization */
	poollist->poolsize = sizeof(T_PoolInfo);
	poollist->lock = new ZThread::FastRecursiveMutex;

	/* Set our stats pointers */
	stats = (T_StatsInfo *)(((unsigned)poollist)+sizeof(T_PoolInfo));

	/* Set the appropriate pointer in stats array */
	poolstat(poollist->poolsize)->pool = poollist;

	/* Calculate the number of blocks we can fit in the remaining memory */
	counter = (unsigned int)floor((poolalloc - statsoverhead)/
									(poollist->poolsize + sizeof(T_PoolInfo *)));

	/* Setup pool freelist head and walk through the allocated memory to stamp
	 * our tracking records. */
	poollist->freelisthead = (T_allocblock*)((unsigned)stats + statsoverhead);
	poollist->blockcount +=stamppool(poollist, poollist->freelisthead, counter);
	poollist->freeblocks = poollist->blockcount;
}

/** Prepare a new pool block for usage
 *
 * This function sets up T_allocblock structures throughout the block of
 * memory to prepare it for usage by the allocation engine.
 *
 * @param pool Memory pool the block belongs to
 * @param start Location to start stamping from
 * @param count Number of blocks to mark
 * @param list The last block we stamp will have its next pointer set to list
 *
 * @return Number of blocks stamped
 *
 * @warning It is the calling functions responsibility to calculate the number
 * of blocks that can be stamped.  Stamppool has no way to know if its
 * activities will write outside of allocated memory, thus it relies on an
 * accurate count to prevent going to far.
 */
unsigned int PoolAllocator::stamppool(T_PoolInfo *pool, T_allocblock* start, 
																			unsigned int count,
																			T_allocblock *list=NULL)
{
	unsigned int step = pool->poolsize + sizeof(T_PoolInfo *);
	cout << "Block step: " << step << endl;
	T_allocblock *cur = start;
	for (unsigned int i=1; i < count; i++)
	{
		cur->pool = pool;
		cur->next = (T_allocblock *)((unsigned)cur + step);
		cur = cur->next;
	}
	cur->pool = pool;
	cur->next = list;
	return count;
}

/** Create a new allocation pool
 *
 * This function will create a new memory allocation pool with the specified
 * block size.  It does verify that the pool size has not been defined after
 * locking the system lock to prevent race conditions.
 *
 * @param blocksize  Size of blocks in this pool
 */
void PoolAllocator::createpool(unsigned int blocksize)
{
	if (blocksize > maxblocksize)
	{
		cout << "Maximum block size is: " << maxblocksize << endl 
			   << "Attempt to create pool with block size: " << blocksize << endl;
		return;
	}

	syslock.acquire();
	if (poolstat(blocksize)->pool != NULL)
	{
		cout << "Pool with block size " << blocksize << " currently exists." 
				 << endl;
		syslock.release();
		return;
	}
	T_PoolInfo *newpool;

	unsigned int blockcount = 0;
	unsigned int poolalloc = poolallocsize;
	
	/* FIXME:  We may want to allocate a bigger chunk of memory for bigger
	 * blocks */

	/* Create the pool and do some initialization */
	newpool = (T_PoolInfo *)malloc(poolalloc);
	if (poollist == NULL)
	{
		cerr << "SEVERE: Pool allocation engine unable ot allocate new pool " <<
			blocksize << endl;
		syslock.release();
		return;
	}
	/* Initialize the entire pool to 0's to save some time */
	bzero(newpool, poolalloc);
	/* Do basic pool initialization */
	newpool->poolsize = blocksize;
	newpool->lock = new ZThread::FastRecursiveMutex;
	newpool->pnext = poollist;
	poollist = newpool;

	newpool->lock->acquire();

	/* Set the appropriate pointer in stats array */
	poolstat(blocksize)->pool = newpool;
	syslock.release();

	/* Calculate the number of blocks we can fit in the remaining memory */
	blockcount = (unsigned int)floor(poolalloc/
									(newpool->poolsize + sizeof(T_PoolInfo *)));

	/* Setup pool freelist head and walk through the allocated memory to stamp
	 * our tracking records. */
	newpool->freelisthead=(T_allocblock*)((unsigned)newpool + sizeof(T_PoolInfo));
	newpool->blockcount +=stamppool(newpool, newpool->freelisthead, blockcount);
	newpool->freeblocks = newpool->blockcount;

	/* release lock and we're done */
	newpool->lock->release();
}

/** Expand an existing memory pool
 * This function will expand the specified memory pool to provide room for a
 * minimum of the specified number of objects.  We don't allocate less then
 * our pool allocation size, but we may need to allocate multiple blocks to
 * fit the objects into.
 *
 * @param poolsize Pool that we need to expand.  This must be an existing
 * 									pool.
 * @param newcount number of new objects to make room for - default 1
 *
 * @return Number of new blocks allocated, may be different from newcount.  0
 * means no action was taken.
 *
 * @note The available block count for the pool will be updated by this
 * function.  Do *NOT* update this separately.
 *
 * @note This function will not create new pools, only expand existing ones.
 * It also may not allocate blocks of exactly poolsize bytes.  If 'poolsize'
 * is using a pool for a larger blocksize, the larger block size will be used
 * in all calculations.
 */
unsigned int PoolAllocator::expandpool(unsigned int poolsize,
																			 unsigned int newcount = 1)
{
	/* First things first, does a pool exist for the given size */
	T_PoolInfo *pool;

	if ((pool = poolstat(poolsize)->pool) == NULL)
	{
		cout << "Attempt to expand nonexisting pool size " << poolsize << endl;
		return 0;
	}

	/* Lock the pool from other modification */
	pool->lock->acquire();

	/* There is an existing pool and we have a pointer to it.  Now we need to
	 * figure out how much memory we need to allocate. */
	unsigned int poolalloc = poolallocsize;
	while (poolalloc < (pool->poolsize * newcount))
	{
		poolalloc += poolallocsize;
	}

	/* Allocate memory */
	T_allocblock *newchunk;
	if ((newchunk = (T_allocblock*)malloc(poolalloc)) == NULL)
	{
		cout << "SEVERE: Unable to allocate " << poolalloc << " bytes for pool "
				 << pool->poolsize << endl << endl;
		pool->lock->release();
		return 0;
	}

	/* Calculate actual block count */
	unsigned int blockcount = (unsigned int)floor(poolalloc/
								(pool->poolsize+sizeof(T_PoolInfo *)));
	/* Stamp out the block */
	unsigned int newblocks;
	newblocks = stamppool(pool,newchunk,blockcount,pool->freelisthead);
	pool->blockcount += newblocks;
	pool->freeblocks += newblocks;

	/* Link the new block in */
	pool->freelisthead = newchunk;

	/* Release our lock */
	pool->lock->release();

	return blockcount;
}

/** Allocate a block of memory for the given size.
 * Pickup the first free block for the given size and return a pointer to the
 * beginning of the block.  Link to an existing pool or create a new pool as
 * needed to fulfill the request.  Extend a pool that is out of free blocks if
 * needed.
 *
 * @param size Size of memory block to allocate
 * @return Pointer to allocated block
 */
void *PoolAllocator::alloc(size_t size)
{
	/* Make sure we aren't allocating too big of a block */
	if (size > maxblocksize)
	{
		cerr << "SEVERE:  Attempt to allocate block larger then maxblocksize"
				 << endl;
		return NULL;
	}

	/* This will be the block we allocate */
	T_allocblock *block;
	/* Pool we will allocate from */
	T_PoolInfo *pool;
	/* Pointer to the stats info */
	T_StatsInfo *ppoolstat;

	/* Attempt to get a pointer to the pool stats */
	ppoolstat = poolstat(size);

	/* Check to see if the pool exists for this block size.  Create/link to a
	 * pool if it doesn't */
	if (ppoolstat->pool == NULL)
	{
		pool = getpoolfor(size);
		if (pool == NULL)
		{
			cerr << "SEVERE:  Unable to get pool for block size " << size << endl;
			return NULL;
		}
	} else {
		pool = ppoolstat->pool;
	}

	/* If we get to here, then we have valid pool and ppoolstat pointers.  Next
	 * we need to check for free blocks in the pool. */
	pool->lock->acquire();
	if (pool->freelisthead == NULL)
	{
		/* Expand the pool */
		if (expandpool(size) == 0)
		{
			pool->lock->release();
			cerr << "SEVERE:  Pool " << size << " is empty and unable to extend!"
					 << endl;
			return NULL;
		}
	}
	
	/* We pointers are available.  Attach block and update counters. */
	block = pool->freelisthead;
	pool->freelisthead = block->next;
	pool->freeblocks--;
	ppoolstat->allocationcount++;

	/* Unlock pool */
	pool->lock->release();

	/* Return a pointer to the allocated memory */
	return (void *)((unsigned int)block + sizeof(T_PoolInfo *));
}

/** Free an allocated block
 * Release the memory held by ptr back to the free pool for the pool attached
 * to the block.  Every block allocated by our system has a pointer to the
 * source pool positioned just before the block allocation.
 *
 * @param ptr Pointer to allocated block.
 */
void PoolAllocator::free(void *ptr)
{
	/* Get a pointer to the real beginning of the allocblock */
	T_allocblock *block=(T_allocblock*)((unsigned int)ptr -sizeof(T_PoolInfo*));
	/* Lock the pool */
	block->pool->lock->acquire();
	/* Link the freed block into the list */
	block->next = block->pool->freelisthead;
	block->pool->freelisthead = block;
	block->pool->freeblocks += 1;
	/* Release the lock - all done */
	block->pool->lock->release();
}

/** Get a pool for a specified block size
 * Return a pointer to a pool for a specific block size.  Create the pool or
 * link to an existing pool if the specified size doesn't already exist.
 *
 * @param poolsize Block Size requested
 * @return Pointer to a pool for the specified block size
 *
 * @note We have to grab the system lock in the process of doing this.
 * Failure to do so can corrupt the entire allocation system or result in
 * significant memory leaks.
 */
PoolAllocator::T_PoolInfo* PoolAllocator::getpoolfor(unsigned int poolsize)
{
	T_PoolInfo *pool = NULL;

	if (poolsize > maxblocksize)
	{
		return NULL;
	}

	syslock.acquire();
	if (poolstat(poolsize)->pool != NULL)
	{
		syslock.release();
		return (poolstat(poolsize)->pool);
	}

	/* Define a search range and check for an existing pool that fits */
	unsigned int searchhigh = poolsize + (poolsize/4) + 1;
	if (searchhigh > poolsize + maxblockoverhead)
		searchhigh = poolsize + maxblockoverhead;

	if (searchhigh > maxblocksize)
		searchhigh = maxblocksize;
	
	/* Search through the pool stats list for the first existing pool.  If we
	 * find one, link to it. */
	for (unsigned int i=poolsize+1; i <= searchhigh; i++)
	{
		if (poolstat(i)->pool != NULL)
		{
			pool = poolstat(i)->pool;
			poolstat(poolsize)->pool = pool;
		}
	}

	/* If we didn't find an existing pool in our search, make our requested
	 * poolsize even and create a pool.
	 * FIXME:  We may eventually want to do some additional magic here to make
	 * pools overlap a bit more. */
	if (pool == NULL)
	{
		unsigned int poolblock = poolsize;
		/* Adjust to even */
		if ((poolblock % 2) != 0)
		{
			poolblock += 1;
		}
		createpool(poolblock);
		pool = poolstat(poolblock)->pool;
		poolstat(poolsize)->pool = pool;
	}

	syslock.release();
	return pool;
}

/** Write all of the pool information to the specified ostream */
ostream& operator<<(ostream& os, const PoolAllocator& pa)
{
	QString str;
	QTextOStream qs(&str);
	
	qs << pa;
	os << str;
	
	return os;
}

/** Write all of the pool information to the specified ostream */
QTextStream& operator<<(QTextStream& os, const PoolAllocator& pa)
{
	unsigned int poolcount = 0;
	unsigned int totblocks = 0;
	unsigned int freeblocks = 0;
	unsigned long allocamt = 0;
	unsigned long freeamt = 0;
	os << "Pool Allocator Status Report: " << endl;
	os << "BlockSize\t| Free Blocks\t| Total Blocks\t| Total Allocations"
		 << endl;
	for (unsigned int i = 0; i < PoolAllocator::maxblocksize; i++)
	{
		if (pa.stats[i].pool != NULL)
		{
			os << i + 1 << "\t\t| ";
			if (pa.stats[i].pool->poolsize != (i+1))
			{
				os << "shared with poolsize " << pa.stats[i].pool->poolsize << "\t| ";
			} else {
				os << pa.stats[i].pool->freeblocks << "\t\t| "
				   << pa.stats[i].pool->blockcount << "\t\t| ";
				poolcount++;
				allocamt += pa.stats[i].pool->blockcount * (pa.stats[i].pool->poolsize
											+ sizeof(PoolAllocator::T_PoolInfo *));
				freeamt += pa.stats[i].pool->freeblocks * (pa.stats[i].pool->poolsize
											+ sizeof(PoolAllocator::T_PoolInfo *));
				totblocks += pa.stats[i].pool->blockcount;
				freeblocks += pa.stats[i].pool->freeblocks;
			}
			os << pa.stats[i].allocationcount << endl;
		}
	}
	unsigned int overhead = sizeof(PoolAllocator::T_StatsInfo) *
													PoolAllocator::maxblocksize;
	overhead += sizeof(PoolAllocator::T_PoolInfo) * poolcount;
	allocamt += overhead;
	os << endl << "Summary: " << endl;
	os << "Total Memory Allocated: " << allocamt << " - Overhead: " 
		 << overhead << " - Free Memory: "
		 << freeamt << endl;
	os << totblocks << " Blocks allocated in " << poolcount << " pools with "
	   << freeblocks << " blocks free." << endl;

	return os;
}

}; /* Koalamud namespace */
