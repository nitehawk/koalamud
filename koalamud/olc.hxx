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

#ifndef KOALA_OLC_HXX
#define KOALA_OLC_HXX "%A%"

#include <qptrlist.h>

#include "parser.hxx"

namespace koalamud
{

/** This is the base class for all of the OLC classes.
 * It provides the basic functionality required to provide online creation of
 * various pieces.  This class provides all of the layout functions that
 * subclasses will use to implement their screen layout and everything else
 * related.
 *
 * @note We inherit from parser, but we expect the calling class to pass in
 * the old parser which we will return to when we are done.  The editor
 * classes will work in a similar fashion.
 */
class olc : public Parser
{
	public: /* Types needed in olc system */
		/** Field types */
		typedef enum {
			FIELD_STRING, /**< Single line string, use maxlength */
			FIELD_MULTILINE, /**< Multiline description, no maxlength */
			FIELD_INTEGER, /**< maxlength is the maximum value accepted.. */
			FIELD_ENUM, /**< Value is one of the possible choices only */
			FIELD_SET, /**< Value is for multiple flags */
		} fieldtype_t;
		/** List node for enum/set field types */
		typedef struct {
			unsigned int flagnum; /**< Value of flag */
			QString name; /**< String to display */
			/** Operator new overload */
			void * operator new(size_t obj_size)
				{ return koalamud::PoolAllocator::alloc(obj_size); }
			/** Operator delete overload */
			void operator delete(void *ptr)
				{ koalamud::PoolAllocator::free(ptr); }
		} listnode_t;
		/** Field information */
		typedef struct {
			QString name; /**< Field name */
			fieldtype_t type;	/**< Field Type */
			/** Field value */
			union {
				QString *str; /**< For string or multiline values */
				long *numeric; /**< For remaining types */
			};
			bool editable; /**< Is this field editable */
			QPtrList<listnode_t> flaglist; /**< List of values for enum/set types */
			long max; /**< Max length for string value or max integer value */
			long min; /**< Min length for string or min integer value */
			/** Operator new overload */
			void * operator new(size_t obj_size)
				{ return koalamud::PoolAllocator::alloc(obj_size); }
			/** Operator delete overload */
			void operator delete(void *ptr)
				{ koalamud::PoolAllocator::free(ptr); }
		} field_t;
		/** Parser state */
		typedef enum {
			STATE_MAINMENU, /**< Main menu currently displayed */
			STATE_EDITFIELD, /**< Waiting for input to update curfield */
			STATE_EDITENUM, /**< Editing an enum value (separate the complexity) */
			STATE_EDITSET, /**< Editing a set value (separate the complexity) */
			STATE_EDITML, /**< Returning after editing a multiline value */
		} parsestate_t;

	public:
		olc(Char *ch, ParseDescriptor *pd, Parser *oldParser);
		/** Empty virtual destructor to make sure memory is freed correctly */
		virtual ~olc(void) {}

	public:
		virtual void parseLine(QString line);
		virtual void sendMenu(void);
		
	public:
		/** Pure virtual Save function.
		 * Subclasses will implement this function to save to the database */
		virtual void save(void) = 0;
		/** Pure virtual load function.
		 * Subclasses will implement this function to load from the database */
		virtual bool load(void) = 0;

	public:
		QString escapeString(QString str);
	
	public:
		field_t *addField(QString name, fieldtype_t type, QString *strval,
											bool editable, long max = 0, long min=0);
		field_t *addField(QString name, fieldtype_t type, long *numval,
											bool editable, long max = 0, long min=0);

	protected:
		/** Pointer to old parser */
		Parser *_old;
		/** List of fields in this OLC */
		QPtrList<field_t> fieldlist;
		/** Field currently being edited */
		field_t *curfield;
		/** Current Parse State */
		parsestate_t curstate;
};

}; /* end koalamud namespace */

#endif //  KOALA_OLC_HXX
