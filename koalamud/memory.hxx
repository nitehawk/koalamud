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
* classes.  
* Classes:
\***************************************************************/

#ifndef KOALA_MEMORY_HXX
#define KOALA_MEMORY_HXX "%A%"

#include <iostream>
#include <zthread/FastRecursiveMutex.h>

#include <qtextstream.h>

namespace koalamud {

/**  Memory Pool Allocator
 *
 * @author Matthew Schlegel <nitehawk@koalamud.org>
 *
 * Pool based memory allocation class to be used in overloaded new/delete
 * operators.  Provides low overhead allocation/deallocation by allocating
 * large chunks of memory for fixed size class groups.
 *
 * This implementation manages multiple memory pools with varying sizes.  Pool
 * selection is based on requested block size so that objects of the same size
 * will share a single pool.  There is a small amount of overhead required for
 * each pool managed in order to provide locking around pointer updates and
 * the memory overhead to track free blocks in each pool.  We use the memory
 * we allocate for the pool for most of the tracking, but we do allocate a
 * chunk of memory separately to track the first free node of each pool.
 *
 * Both alloc's and free's need to lock the affected pool before updating
 * pointers.  Failure to lock pools before updating could have a devistating
 * effect in KoalaMud's multithreaded environment.
 *
 * New pools are allocated on demand based upon the following criteria:
 * a) There is no existing pool with large enough blocks
 * b) There are pools with big enough blocks, but the overhead in using those
 * blocks is greater then 25%
 *
 * Block sizes for automatically allocated objects may be adjusted slightly to
 * make a better fit into the memory that will be allocated for it.  There
 * will most likely be blocksizes that can't be adjusted without significant
 * overhead.  Those block sizes will most certainly leave extra room in the
 * pool that we will attempt to fill with blocks from a smaller pool.
 *
 * There is a bit of overhead involved in allocating a new pool or starting a
 * block size on an exiting pool, but that overhead is only required on the
 * first allocation of a block size or when the block needs to be expanded.
 * These large block allocations include malloc'ing the block of memory for
 * the calculated block size, and walking through the block to stamp next
 * pointers through it.  For new pools, it is also necessary to create the
 * mutex controlling access to a block.  When adding a new allocation size to
 * an existing pool (120 byte allocations would use the same pool as 128 byte
 * allocations typically), the pool structure will have a pointer to the real
 * pool structure and a counter.
 *
 * The allocator also keeps an allocation counter for each block size so that
 * we can see how memory is being used.  This information is also used to
 * manually force specific pool sizes for heavily used object sizes.  There
 * is also a counter that tracks the largest number of blocks available for a
 * given memory size.
 *
 * @note  We automatically allocate a pool of 20 byte blocks during
 * construction.  We use the beginning of this block to put all of our
 * tracking overhead in.
 *
 */
class PoolAllocator
{
	/* Constants */
	public:
		/** This is used to limit the byte amount of overhead in larger blocks.
		 * It may need to be adjusted if we end up with sparse usage of bigger
		 * pools.
		 * @note This value has a dramatic effect on the time required to allocate
		 * the first block for a given block size.  pool overlap searches are
		 * bound to this value as a maximum, so high values here will cause longer
		 * times in search loops for block linking. */
		static const unsigned int maxblockoverhead = 128;
		/** We need an upper limit to the block sizes we manage so that we don't
		 * kill ourselves in overhead. */
		static const unsigned int maxblocksize = 4096; 
		/** With a 4 byte minimum block size, we are at 100% overhead with the
		 * pool pointer.  We likely won't see any pools this size, but we do need
		 * a lower limit. */
		static const unsigned int minblocksize = 4;
		/** Size of pools we allocate.  For larger block sizes, we may allocate
		 * a multiple of this size to reduce the allocation overhead impact */
		static const unsigned int poolallocsize = 65536;
	
	/* All of our tracking variables are private in case we later need to
	 * subclass our allocator */
	protected:
		/* predefine the pool info struct */
		struct TAG_PoolInfo;

		/** Allocation block
		 * This structure is written throughout the allocated block to make
		 * allocating the smaller chunks and tracking them easy.
		 */
		typedef struct TAG_allocblock {
			/** Pointer to the pool this block is a part of - used to free */
			struct TAG_PoolInfo *pool;
			/** Pointer to next free block */
			struct TAG_allocblock *next;
		} T_allocblock;
		/** Pool information structure
		 * This struct tracks all of the information for a single pool
		 * 		free list pointer
		 * 		locking mutex pointer
		 * 		total allocated blocks
		 * 		Total free blocks
		 * 		next pool (so we can traverse existing pools easily)
		 * Everything except the mutex itself lives inside of our blocks.  The
		 * pool information structures consume the first 24 bytes of the first
		 * block allocated for a pool.
		 */
		typedef struct TAG_PoolInfo {
			/** Size of blocks in this pool */
			unsigned int poolsize;
			/** Pointer to first free block */
			T_allocblock* freelisthead;
			/** Total number of blocks allocated */
			unsigned int blockcount;
			/** Total number of free blocks in this pool */
			unsigned int freeblocks;
			/** Pointer to next pool */
			struct TAG_PoolInfo *pnext;
			/** Pointer to lock for pool */
			ZThread::FastRecursiveMutex *lock;
		} T_PoolInfo;

		/** This structure is used for tracking the statistics of the allocation
		 * engine as well as the location of pools */
		typedef struct TAG_StatsInfo {
			/** Pool this block size uses.  pool->poolsize may not equal block size
			 */
			T_PoolInfo *pool;
			/** Total allocations for this block size */
			unsigned int allocationcount;
		} T_StatsInfo;

		/** Pool List head */
		T_PoolInfo *poollist;
		/** Pointer to array of stats info - bounds are 0 - maxblocksize-1 */
		T_StatsInfo *stats;

		/** This lock is needed for creating new pools to prevent poollist and
		 * stats corruption */
		ZThread::FastRecursiveMutex syslock;
			
	protected:
		PoolAllocator(void);

	public:

		void createpool(unsigned int blocksize);
		unsigned int expandpool(unsigned int poolsize, unsigned int newcount = 1);

		void *ialloc(size_t size);
		void ifree(void *ptr);

	protected:
		unsigned int stamppool(T_PoolInfo *pool, T_allocblock* start,
														unsigned int count, T_allocblock* list=NULL);
		/** Get a pointer to that stats structure for a pool size */
		T_StatsInfo *poolstat(unsigned int poolsize) { return &stats[poolsize-1];}
		T_PoolInfo *getpoolfor(unsigned int poolsize);

	friend ostream& operator<<(ostream& os, const PoolAllocator& pa);
	friend QTextStream& operator<<(QTextStream& os, const PoolAllocator& pa);

	public:  /* Singleton support stuff */
		/** Singleton Based allocate */
		static void *alloc(size_t size)
			{
				return instance()->ialloc(size);
			}
		/** Singleton based free */
		static void free(void *ptr)
			{
				instance()->ifree(ptr);
			}
		/** Get a pointer to the singleton instance */
		static PoolAllocator *instance(void)
			{
				static PoolAllocator *_instance = NULL;
				if (_instance == NULL)
					_instance = new PoolAllocator;
				return _instance;
			}
};

}; /* Koalamud namespace */
#endif //  KOALA_MEMORY_HXX
