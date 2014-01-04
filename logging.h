/*
 *  logging.h
 *  Provides printf()-like logging functionality.
 *
 *  Copyright (c) 2006, 2007 Daniel Klein. All rights reserved.
 *  
 *  Licensed under GPL version 2.
 *  See here for the full license: http://www.gnu.org/licenses/gpl.html
 */
 
#ifndef _logging_h_
#define _logging_h_

#define LOG_VERBOSE 0
#define LOG_INFO 1
#define LOG_WARNING 2
#define LOG_ERROR 3

#include "types.h"

extern int currentLogLevel;
extern boolean logMemoryEnabled;

void logVerbose( const char* format, ... );
void logInfo( const char* format, ... );
void logWarning( const char* format, ... );
void logError( const char* format, ... );
void logMemory( const char* format, ... );

#endif /*_logging_h_*/