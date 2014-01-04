/*
 *  puraGlobals.c
 *  Global variables, definitions and functions.
 *
 *  Copyright (c) 2006, 2007 Daniel Klein. All rights reserved.
 *  
 *  Licensed under GPL version 2.
 *  See here for the full license: http://www.gnu.org/licenses/gpl.html
 */

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include "puraGlobals.h"

/* global variables */
const char* STR_VERSION= "0.18";
const char* classpath= NULL;
const char** mainClassArguments= NULL;

/* error message reporting */
void error( const char* error )
{
	printf( "Error: %s\n", error );
	fflush( stdout );
	exit( -1 );
}

void errorNo( const char* error )
{
	printf( "Error: %s (%i)\n", error, errno );
	fflush( stdout );
	exit( -1 );
}
