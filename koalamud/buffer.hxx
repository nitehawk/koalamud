/***************************************************************\
*                        KoalaMud Gen 2                         *
*    Copyright (c) 2002 First Step Internet Services, Inc.      *
*                     All Rights Reserved                       *
*        Distributed under the terms of the FSI License         *
* See file LICENSE in the root of this package for information  *
\***************************************************************/
/***************************************************************\
*	Module: CORE/Buffer
* Description:
* 	Ring buffer class primarily for usage in descriptor classes.
* 	Requested buffer size is rounded up to the nearest multiple
* 		of the shared memory page size.
* 	IMPORTANT:
* 		This buffer class requires shared memory to be available
* 			for its operations
* Classes:
* 	Buffer
\***************************************************************/

#ifndef KOALA_BUFFER_HXX
#define KOALA_BUFFER_HXX "%A%"

#include <zthread/FastRecursiveMutex.h>

#include <sys/ipc.h>
#include <sys/shm.h>
#include <unistd.h>

#include "memory.hxx"

namespace koalamud
{

/** Ring buffer class
 * This uses shared memory and mmap to provide an easy to manage ring buffer
 */
class Buffer
{
	protected:
		/** Shared memory handle */
		int _shmhandle;
		/** Size of buffer in bytes */
		int _size;
		/** Pointer to beginning of lower memory segment */
		char *_lower_segment;
		/** Pointer to beginning of upper memory segment */
		char *_upper_segment;
		/** Pointer to head of ring buffer */
		char *_head;
		/** Pointer to tail of ring buffer */
		char *_tail;
		/** Mark whether or not the buffer is valid */
		bool valid;
		/** Buffer lock */
		ZThread::FastRecursiveMutex _lock;

	public:
		/* Build and destroy it */
		Buffer(unsigned int size=4096);
		~Buffer(void);

		/** Is this buffer valid (fully constructed ready to use) */
		bool isValid(void) const { return valid; }

		/* Status */
		/** Get amount of free space in buffer */
		int getFree(void) { return (_head + _size - _tail); }
		/** Get the amount of buffer space used */
		int getUsed(void) { return (_tail - _head); }
		/** Is the buffer full */
		bool isFull(void) { return (getFree() == 0); }
		/** Is the buffer empty */
		bool isEmpty(void) { return (_head == _tail); }

		/* Put data in */
		/** Lock the buffer and return the tail of the buffer */
		char *getTail(void) { lock(); return _tail;}
		/** Update the buffer tail and unlock buffer */
		void externDatain(int amt) { _tail += amt; unlock(); }
		int getData(char *buf, int max);

		char *getLine(void);

		/** Lock buffer mutex */
		void lock(void) { _lock.acquire(); }
		/** Unlock buffer mutex */
		void unlock(void) { _lock.release(); }

		/** Operator new overload */
		void * operator new(size_t obj_size)
			{ return koalamud::PoolAllocator::alloc(obj_size); }
		/** Operator delete overload */
		void operator delete(void *ptr)
			{ koalamud::PoolAllocator::free(ptr); }
};

}; /* end koalamud namespace */

#endif  // KOALA_BUFFER_HXX
