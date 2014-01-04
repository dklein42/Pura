/*
 *  logging.h
 *  Pura
 *
 *  Created by Daniel Klein on 06.11.06.
 *  Copyright 2006 __MyCompanyName__. All rights reserved.
 *
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