/*
 *  fileClassLoader.c
 *  Pura
 *
 *  Created by Daniel Klein on 06.11.06.
 *  Copyright 2006 __MyCompanyName__. All rights reserved.
 *
 */

#include <stdio.h>
#include <memory.h>
#include "memoryManager.h"
#include "puraGlobals.h"
#include "fileClassLoader.h"

#define STR_CLASSPATH_DELIMITERS ":;"

boolean isSlashAtTheEnd( const char* str )
{
	while( *str != '\0' )
		str++;
		
	str--;
		
	return *str == '/';
}

void replaceDots( char* str )
{
	while( *str != '\0' )
	{
		if( *str == '.' )
			*str= '/';
			
		str++;
	}
}

FILE* findClassInClasspath( const char* className )
{
	char cp[256];
	strcpy( cp, classpath );
	char tmpName[256];
	
	/* get first token/path */
	char* token= strtok( &cp[0], STR_CLASSPATH_DELIMITERS );
	
	/* concatenate class name and path */
	if( strlen(token) + strlen(className) > 255 )
		error( "Path for class is too long!\n" );
	
	strcpy( tmpName, token );
	if( !isSlashAtTheEnd(token) )
		strcat( tmpName, "/" );
	strcat( tmpName, className );
	
	logVerbose( "Looking for file %s... ", tmpName );
	
	FILE* f= fopen( tmpName, "rb" );

	if( f != NULL )
	{
		logVerbose( "Found!\n" );
		return f;
	}
	
	logVerbose( "Not Found.\n" );
	
	while( (token= strtok(NULL, STR_CLASSPATH_DELIMITERS)) != NULL )
	{
		/* concatenate class name and path */
		if( strlen(token) + strlen(className) > 255 )
			error( "Path for class is too long!\n" );
	
		strcpy( tmpName, token );
		if( !isSlashAtTheEnd(token) )
			strcat( tmpName, "/" );
		strcat( tmpName, className );
	
		logVerbose( "Looking for file %s... ", tmpName );
	
		f= fopen( tmpName, "rb" );

		if( f != NULL )
		{
			logVerbose( "Found!\n" );
			return f;
		}
	
		logVerbose( "Not Found.\n" );
	}
	
	error( "Could not find class!" );
	return NULL; /* shut up compiler */
}

void cl_init( ClassLoaderState* state, const char* className )
{
	logVerbose( "file class loader is loading class %s\n", className );

	/* append ".class" file extension */
	char classNameWithExtension[256];
	
	if( strlen(className) > 255-6 )
		error( "Name of the main class is too long!\n" );
		
	strcpy( classNameWithExtension, className );
	replaceDots( classNameWithExtension );
	strcat( classNameWithExtension, ".class" );	

	/* find the class file within the classpath and open it*/
	FILE* f= findClassInClasspath( classNameWithExtension );
		
	/* get file size */
	fseek( f, 0, SEEK_END );
	state->size= ftell( f );
	rewind( f );
	
	logVerbose( "class size is %i bytes\n", state->size );
	
	/* allocate heap memory and load file contents */
	state->data= (byte*)mm_staticMalloc( state->size );
	state->currentPosition= state->data;
	
	/* read class data */
	int ret= fread( state->data, 1, state->size, f );
	
	if( ret != state->size )
		error( "File size did not match!" );
	
	fclose( f );
}

void cl_free( ClassLoaderState* state )
{
	logVerbose( "Cleaning up file class loader structure\n" );
	mm_staticFree( state->data );
	state->currentPosition= NULL;
	state->size= 0;
}

void cl_readBytes( ClassLoaderState* state, int numBytes, byte* data )
{
	memcpy( data, state->currentPosition, numBytes );
	state->currentPosition+= numBytes;
}

void cl_skipBytes( ClassLoaderState* state, int numBytes )
{
	state->currentPosition+= numBytes;
}

u1 cl_peekNextU1( ClassLoaderState* state )
{
	return *state->currentPosition;
}

u2 cl_peekNextU2( ClassLoaderState* state )
{
	u2 word= (*state->currentPosition) << 8 ;
	word|= *(state->currentPosition+1);
	return word;
}

u1 cl_readU1( ClassLoaderState* state )
{
	u1 data= *state->currentPosition;
	state->currentPosition++;
	return data;
}

u2 cl_readU2( ClassLoaderState* state )
{
	u2 data= cl_readU1( state ) << 8;
	data|= cl_readU1( state );
	return data;
}

u4 cl_readU4( ClassLoaderState* state )
{
	u4 data= cl_readU2( state ) << 16;
	data|= cl_readU2( state );
	return data;
}
