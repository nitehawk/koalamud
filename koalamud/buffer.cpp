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

#define KOALA_BUFFER_CXX "%A%"

#include <string.h>
#include <sys/mman.h>

#include "buffer.hxx"

namespace koalamud {

/** Prepare a ring buffer.
 * This sets up the memory mappings and initializes a ring buffer for usage.
 */
Buffer::Buffer(unsigned int size = 4096)
{
	/* Round requested size up to the nearest page */
	int req = getpagesize();
	req += (size - 1) - ((size - 1) % req);

	/* Set buffer size */
	_size = req;

	/* Find a place in virtual memory for the mapping */
	if ((_lower_segment = (char *)mmap(NULL, 2*req,
									PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_SHARED,
									-1, 0)) == (char *)MAP_FAILED)
	{
		valid = false;
		cerr << "Failed to locate memory segment for mapping new buffer" << endl;
		return;
	}
	_upper_segment = _lower_segment + req;

	/* Now that we have a location to map into, acquire a shared memory segment
	 */
	if ((_shmhandle = shmget(IPC_PRIVATE, req, IPC_CREAT | 0700)) == -1) {
		valid = false;
		cerr << "Failed to get shared memory segment for new buffer" << endl;
		return;
	}

	/* We have a shared memory segment, unmap our pointers and attached our
	 * shared memory in place */
	munmap(_lower_segment, req);
	char *shmaddr = (char *)shmat(_shmhandle, _lower_segment, 0);
	if (shmaddr != _lower_segment) {
		valid = false;
		cerr<<"SHM attached in wrong location while creating new buffer" << endl;
		return;
	}

	munmap(_upper_segment, req);
	shmaddr = (char *)shmat(_shmhandle, _upper_segment, 0);
	if (shmaddr != _upper_segment) {
		valid = false;
		cerr<<"SHM attached in wrong location while creating new buffer" << endl;
		return;
	}

	shmctl(_shmhandle, IPC_RMID, NULL);

	_head = _tail = _lower_segment;
	
	valid = true;
}

/** Destroy a ring buffer */
Buffer::~Buffer(void)
{
	valid=false;
	shmdt(_lower_segment);
	shmdt(_upper_segment);
}

/** Return a pointer to a line of input or NULL if a full line of input is not
 * available.  Line will have \r and \n stripped off of it.
 *
 * NOTE:  To prevent a buffer lock condition where the buffer is full, but no
 * newline exists, the entire buffer will be returned when the buffer is full
 */
char *Buffer::getLine(void)
{
	lock();
	/* Since there isn't a version of strchr or index that will use a length
	 * flag, we'll roll our own. */
	char *pos = _head;
	while (pos != _tail) {
		if (*pos == '\n')
			break;
		++pos;
	}
	if (*pos != '\n')
	{
		/* We don't have a full line available */
		if (isFull())
		{
			/* Buffer is full, return the whole thing and reset */
			char *line = strndup(_head, _size);
			_head = _tail = _lower_segment;
			unlock();
			return line;
		}
		unlock();
		return NULL;
	}

	/* We found the end of the line, null terminate at pos and check for a \r */
	*pos = '\0';
	if (*(pos-1) == '\r') *(pos-1) = '\0';
	if (*(pos+1) == '\r')
	{
		++pos;
		*pos = '\0';
	}

	/* duplicate the line of text */
	char *line = strdup(_head);

	/* Move head pointer to pos+1 */
	_head = pos + 1;

	/* Adjust head and tail pointers if empty or in upper segment */
	if (_head == _tail)
	{
		_head = _tail = _lower_segment;
	}
	else if (_head > _upper_segment)
	{
		_head -= _size;
		_tail -= _size;
	}

	unlock();
	return line;
}

/** Get some data out of the buffer.
 * This fills @a buf with up to @a max bytes of data.  This copies data
 * through any possible null terminator as well.
 */
int Buffer::getData(char *buf, int max)
{
	lock();
	/* If max is greater then the amount of data in the buffer, get everything.
	 */
	int getthis = getUsed();
	if (max < getthis)
		getthis = max;

	strncpy(buf, _head, getthis);

	/* Move head pointer to pos+1 */
	_head += getthis;

	/* Adjust head and tail pointers if empty or in upper segment */
	if (_head == _tail)
	{
		_head = _tail = _lower_segment;
	}
	else if (_head > _upper_segment)
	{
		_head -= _size;
		_tail -= _size;
	}

	unlock();
	return getthis;
}

/** Return true if getData would return anything other than NULL */
bool Buffer::canReadLine(void)
{
	lock();
	/* Since there isn't a version of strchr or index that will use a length
	 * flag, we'll roll our own. */
	char *pos = _head;
	while (pos != _tail) {
		if (*pos == '\n')
			break;
		++pos;
	}
	if (*pos != '\n')
	{
		/* We don't have a full line available */
		if (isFull())
		{
			unlock();
			return true;
		}
		unlock();
		return false;
	}

	unlock();
	return true;
}

}; /* end koalamud namespace */
