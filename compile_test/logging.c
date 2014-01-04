/*
 *  logging.c
 *  Pura
 *
 *  Created by Daniel Klein on 06.11.06.
 *  Copyright 2006 __MyCompanyName__. All rights reserved.
 *
 */

#include <stdarg.h>
#include "logging.h"

int currentLogLevel= LOG_VERBOSE;
boolean logMemoryEnabled= false;

void logVerbose( const char* format, ... )
{
	va_list arglist;
	va_start( arglist, format );
	if( currentLogLevel <= LOG_VERBOSE )
		vprintf( format, arglist );
	va_end( arglist );
}

void logMemory( const char* format, ... )
{
	va_list arglist;
	va_start( arglist, format );
	if( logMemoryEnabled )
		vprintf( format, arglist );
	va_end( arglist );
}

void logInfo( const char* format, ... )
{
	va_list arglist;
	va_start( arglist, format );
	if( currentLogLevel <= LOG_INFO )
		vprintf( format, arglist );
	va_end( arglist );
}

void logWarning( const char* format, ... )
{
	va_list arglist;
	va_start( arglist, format );
	if( currentLogLevel <= LOG_WARNING )
		vprintf( format, arglist );
	va_end( arglist );
}

void logError( const char* format, ... )
{
	va_list arglist;
	va_start( arglist, format );
	if( currentLogLevel <= LOG_ERROR )
		vprintf( format, arglist );
	va_end( arglist );
}
