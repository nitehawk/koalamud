/***************************************************************\
*                        KoalaMud Gen 2                         *
*    Copyright (c) 2002 First Step Internet Services, Inc.      *
*                     All Rights Reserved                       *
*        Distributed under the terms of the FSI License         *
* See file LICENSE in the root of this package for information  *
\***************************************************************/
/***************************************************************\
*	Module:
*	Author:
* Description:
* Classes:
\***************************************************************/

#ifndef KOALA_EXCEPTION_HXX
#define KOALA_EXCEPTION_HXX "%A%"

/** Koalamud namespace contains all of the core classes of the server */
namespace koalamud {
	/** All exceptions go in the exceptions namespace */
	namespace exceptions {

/** Base for all koalamud exceptions */
class koalaexception {};
/** exceptions related to daemonizing */
class daemonize : public koalaexception {};
/** Problem with forking into background */
class forkerror : public daemonize {};
/** problem setting process group */
class pgrperror : public daemonize {};
/** fork successful - throw to cause parent PID to exit */
class forkparent : public daemonize {};
	
	}; /* End exceptions namespace */
}; /* End koalamud namespace */

using koalamud::exceptions::koalaexception;

#endif //  KOALA_EXCEPTION_HXX
