/*
 *  puraMain.c
 *  Starting point, parameter handling, environment variable access.
 *
 *  Copyright (c) 2006, 2007 Daniel Klein. All rights reserved.
 *  
 *  Licensed under GPL version 2.
 *  See here for the full license: http://www.gnu.org/licenses/gpl.html
 */

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "puraGlobals.h"
#include "methodArea.h"
#include "memoryManager.h"
#include "interpreter.h"
#include "heap.h"

const char* mainClass;

void handleParameters( int argcnt, const char** args )
{
	/* check if command line parameters exist */
	if( argcnt == 1 )
	{
		logError( "Usage: pura [parameters] <main class> [arguments]\n\n" );
		logError( "Parameters:\n\n" );
		logError( "-cp | -classpath <classpath>\n" );
		logError( "-silent => Disable debug output. (Set log level to WARNING.)\n" );
		logError( "-mem | -memory => Show memory usage information.\n" );
		logError( "-opcodestats => Show Opcode usage statistics.\n" );
		logError( "-all => Show all possible debug output.\n" );
		logError( "-stack <stack size>\n" );
		/*logError( "-kp - Stop until key pressed after output.\n" );*/
		/*logError( "-d <delay> - Delay execution after output for <delay> ms.\n" );*/
		logError( "\n" );
		exit( 0 );
	}
	
	/* check for all possible parameters */
	int i;
	for( i= 1; i < argcnt; i++ )
	{
		/* classpath */
		if( strcasecmp(args[i], "-cp") == 0 || strcasecmp(args[i], "-classpath") == 0 )
		{
			/* is there one parameter left? */
			if( argcnt < i+1 )
				error( "Error while parsing parameters!\n" );
			
			classpath= args[++i];
			
			if( strlen(classpath) > 255 )
				error( "Classpath is too long!" );
			
			/*logVerbose( "Classpath: %s\n", classpath );*/
			continue;
		}
		
		/* stack size */
		else if( strcasecmp(args[i], "-stack") == 0 )
		{
			/* is there a parameter left? */
			if( argcnt < i+1 )
				error( "Error while parsing parameters!\n" );
			
      int32 parsedStackSize= atoi( args[++i] );
			
			if( parsedStackSize < 0 || parsedStackSize > 10000000 )
				error( "The provided stack size is not allowed." );

      initialStackSize= parsedStackSize;

			continue;
		}
		
		/* silent */
		else if( strcasecmp(args[i], "-silent") == 0 )
		{
			currentLogLevel= LOG_WARNING;
			continue;
		}
		
		/* memory */
		else if( strcasecmp(args[i], "-mem") == 0 || strcasecmp(args[i], "-memory") == 0 )
		{
			logMemoryEnabled= true;
			continue;
		}
		
		/* all */
		else if( strcasecmp(args[i], "-all") == 0 )
		{
			currentLogLevel= LOG_VERBOSE;
			logMemoryEnabled= true;
			opcodeStatsEnabled= true;
			continue;
		}
		
		/* opcodestats */
		else if( strcasecmp(args[i], "-opcodestats") == 0 )
		{
			opcodeStatsEnabled= true;
			continue;
		}
		
		/* add more parameters here */
		
		/* No suitable parameter found? Must be the main class then. */
		mainClass= args[i];

		/* Now the arguments follow. Just remember their position for now and return. */
		mainClassArguments= args+(i+1);
		return;
	}
}

void checkEnvironmentVars()
{
	/* If there hasn't been a classpath defined manually via the command line, check if there is an according environment variable and set this. */
	if( classpath == NULL )
	{
		char* envClasspath= getenv( PURA_CLASSPATH_ENVIRONMENT_VARIABLE_NAME );
		
		if( envClasspath != NULL )
		{
			if( strlen(envClasspath) <= 255 )
			{
				classpath= envClasspath;
				logVerbose( "Using environment variable classpath: %s\n", envClasspath );
			}
			else
			{
				error( "Classpath is too long!" );
			}
		}
		else
		{
			/* neither command line parameter, nor environment variable => use default */
			classpath= ".";
			logVerbose( "No classpath defined. Using local directory as default.\n" );
		}
	}
}

int main( int argcnt, const char** args )
{
	/*logVerbose( "Parsing VM parameters...\n" );*/
	handleParameters( argcnt, args );
	checkEnvironmentVars();

	logInfo( "Pura Experimental Java Virtual Machine v%s - (c) 2007 Daniel Klein\n", STR_VERSION );

	ma_init();
	heap_init();
	interpreter_start( mainClass );
	mm_printStatistics();

	return 0;
}