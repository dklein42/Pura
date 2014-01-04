/*
 *  puraGlobals.h
 *  Pura
 *
 *  Created by Daniel Klein on 06.11.06.
 *  Copyright 2006 __MyCompanyName__. All rights reserved.
 *
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
