/***************************************************************\
*                        KoalaMud Gen 2                         *
*    Copyright (c) 2002 First Step Internet Services, Inc.      *
*                     All Rights Reserved                       *
*        Distributed under the terms of the FSI License         *
* See file LICENSE in the root of this package for information  *
\***************************************************************/
/***************************************************************\
*	Module: SmartPtr
*	Author: Matthew Schlegel
* Description:
* 	Reference counting smart pointer.
*
* 	Note:  The reference count is thread safe, and by extention, the smart
* 	pointers should also be thread safe.  However, a single smart pointer
* 	should not be used in multiple threads at the same time.  Create
* 	additional pointers for other thread usage.
*
* Classes:
* 	KRefObj, KRefPtr
\***************************************************************/

#ifndef KOALA_AUTOPTR_HXX
#define KOALA_AUTOPTR_HXX "%A%"

#include "zthread/ZThread.h"
#include "zthread/Guard.h"
#include "zthread/FastRecursiveMutex.h"

using ZThread::Guard;

namespace KoalaMud {
  
/**
 * @class KRefObj
 *
 * @author Matthew Schlegel
 * @version 1.0
 * @date 2002-09-07
 *
 * Contains the data and functions used by KRefPtr to provide smart reference
 * counted pointers.
 *
 * Every object that will be used with KRefPtr needs to include
 * 'virtual public KRefObj' in the class declaration.  The side effect is that
 * KRefPtrs cannot be used for any object, but the advantage is increased
 * flexibility in dealing with released pointers to provide an opportunity for
 * further cleanup and memory freeing.
 *
 */
class KRefObj {
  
  private:
	/** Type of our lock */
	typedef class ZThread::FastRecursiveMutex MUTEX;
	/** Actual lock object */
  MUTEX  _Mutex;
	/** Reference count */
  unsigned int _count;
  
  public:
  
  /**
   * Initialize our counter 
   */
  KRefObj(void) {
    Guard<MUTEX> guard(_Mutex);
    _count = 0;
  };

	/**
	 * Destructor doesn't need to do any cleanup
	 */
	virtual ~KRefObj(void) {}

	/**
	 * Increment count
	 *
	 * NOTE: If overriding this function, be sure to call this version using:
	 *   KRefObj::increfcount
	 * and return the result when finished.
	 */
	virtual unsigned int increfcount(void)
	{
    Guard<MUTEX> guard(_Mutex);
		++_count;
		return _count;
	}

	/**
	 * Decrement count
	 *
	 * @NOTE: If overriding this function, be sure to call this version using:
	 *   KRefObj::decrefcount and return the result when finished.  This is
	 *   provided as virtual to allow a place to force additional cleanup.  It
	 *   is recommended that the cleanup be done as a separate task and not done
	 *   in an overrided version of this function.
	 */
	virtual unsigned int decrefcount(void)
	{
    Guard<MUTEX> guard(_Mutex);
		--_count;
		return _count;
	}
  
}; /* KRefObj */

/**
 * @class KRefPtr
 *
 * @author Matthew Schlegel
 * @version 1.0
 * @date 2002-09-07
 *
 * Provide smart pointer semantics to any KRefObj child class.
 *
 */
template <class COUNTED>
class KRefPtr
{
	private:
		/** Pointer to the underlying object */
		COUNTED *_obj;

	public:
		/**
		 * Build an empty smart pointer.
		 *
		 * We don't have a way of getting the subclass, so we initialize to null
		 */
		KRefPtr(void)
		{
			_obj = NULL;
		}

		/**
		 * Create a smart pointer to an existing object.
		 */
		KRefPtr(COUNTED *ptr)
		{
			_obj = ptr;
			_obj->increfcount();
		}

		/**
		 * Create a new smart pointer from an existing one
		 */
		KRefPtr(const KRefPtr& ptr)
		{
			_obj = ptr._obj;
			_obj->increfcount();
		}

		/**
		 * Free a reference
		 */
		~KRefPtr(void)
		{
			if (_obj != NULL)
			{
				if (_obj->decrefcount() == 0)
				{
					delete _obj;
				}
			}
		} /* ~KRefPtr */

		/**
		 * Assignment from exiting refptr
		 */
		KRefPtr& operator=(const KRefPtr& ptr)
		{
			if (ptr._obj != _obj)
			{
				if (_obj && _obj->decrefcount() == 0)
				{
					delete _obj;
				}

				_obj = ptr._obj;
				_obj->increfcount();
			}
			return *this;
		}

		/**
		 * Assignment to KRefObj
		 */
		KRefPtr& operator=(const COUNTED *ptr)
		{
			if (ptr != _obj)
			{
				if (_obj && _obj->decrefcount() == 0)
				{
					delete _obj;
				}

				_obj = ptr;
				_obj->increfcount();
			}
			return *this;
		}

		/**
		 * Get reference to underlying object
		 */
		COUNTED* operator->() { return _obj;}

		/**
		 * Get const reference to underlying object
		 */
		const COUNTED* operator->() const { return _obj;}

		/**
		 * Get a reference to the underlying object.
		 */
		COUNTED* operator*() { return _obj;}
};

};  /* KoalaMud Namespace */

using KoalaMud::KRefPtr;
using KoalaMud::KRefObj;

#endif //  KOALA_AUTOPTR_HXX
