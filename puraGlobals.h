/*
 *  puraGlobals.h
 *  Global variables, definitions and functions.
 *
 *  Copyright (c) 2006, 2007 Daniel Klein. All rights reserved.
 *  
 *  Licensed under GPL version 2.
 *  See here for the full license: http://www.gnu.org/licenses/gpl.html
 */

#ifndef _puraGlobals_h_
#define _puraGlobals_h_

/* logging functions */

/* Enable to disable verbose logging, which speeds up execution quite a bit. */
/* #define VERBOSE_LOGGING_DISABLED */

#include "logging.h"

#ifdef VERBOSE_LOGGING_DISABLED
#define logVerbose(a,...) ""
#endif

/*#ifndef __inline__
#define __inline ""
#endif*/

/* Datatypes used by Pura and Java data types defines */
#include "types.h"

/* global variables */
extern const char* STR_VERSION;
extern const char* classpath;
extern const char** mainClassArguments;

/* global methods */
void error( const char* error );
void errorNo( const char* error );

#define PURA_CLASSPATH_ENVIRONMENT_VARIABLE_NAME "PURA_CLASSPATH"

#endif /*_puraGlobals_h*/
