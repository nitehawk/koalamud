/***************************************************************\
*                        KoalaMud Gen 2                         *
*    Copyright (c) 2002 First Step Internet Services, Inc.      *
*                     All Rights Reserved                       *
*        Distributed under the terms of the FSI License         *
* See file LICENSE in the root of this package for information  *
\***************************************************************/
/***************************************************************\
*	Module: CORE
* Description:
* Classes:
\***************************************************************/

#ifndef KOALA_MAIN_HXX
#define KOALA_MAIN_HXX "%A%"

#include <zthread/PoolExecutor.h>
#include "koalastatus.h"
#include "memory.hxx"

#define TPMIN	1
#define TPMAX 2

#ifdef KOALA_MAIN_CXX
bool guiactive;
KoalaStatus *stat;
ZThread::Executor *executor;
koalamud::PoolAllocator poolalloc;
#else
extern bool guiactive;
extern KoalaStatus *stat;
extern ZThread::Executor *executor;
extern koalamud::PoolAllocator poolalloc;
#endif // KOALA_MAIN_CXX


#endif  // KOALA_MAIN_HXX
