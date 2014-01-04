/*
 *  puraGlobals.c
 *  Pura
 *
 *  Created by Daniel Klein on 06.11.06.
 *  Copyright 2006 __MyCompanyName__. All rights reserved.
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include "puraGlobals.h"

/* global variables */
const char* STR_VERSION= "0.14";
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
