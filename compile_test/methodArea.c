/*
 *  methodArea.c
 *  Pura
 *
 *  Created by Daniel Klein on 06.11.06.
 *  Copyright 2006 __MyCompanyName__. All rights reserved.
 *
 */

#include "puraGlobals.h"
#include "fileClassLoader.h"
#include "class.h"
#include "memoryManager.h"
#include "methodArea.h"

#define MAX_CLASSES 64

Class** loadedClasses;
int numberOfLoadedClasses= 0;

void ma_init()
{
	logVerbose( "Initializing Method Area...\n" );
	loadedClasses= (Class**)mm_staticMalloc( sizeof(Class*) * MAX_CLASSES );
}

Class* ma_loadClass( const char* className )
{
	logVerbose( "Loading class %s\n", className );
	
	ClassLoaderState clState;
	Class* cl= mm_staticMalloc( sizeof(Class) );

	/* Handle arrays separately. */
	if( *className == '[' )
	{
		cls_initArrayClass( cl, className );
	}
	else
	{
		/* init classloader */
		cl_init( &clState, className );
	
		/* load class */
		cls_load( cl, &clState );
	
		/* free classloader */
		cl_free( &clState );
	}
	
	/* add loaded class to the method area's class repository */
	loadedClasses[numberOfLoadedClasses]= cl;
	numberOfLoadedClasses++;
	
	return cl; 
}

/* Searches the method area's loaded classes repository for the given class. If the class is loaded it is returned, otherwise NULL. */
Class* ma_containsClass( const char* className )
{
	int i;
	for( i= 0; i < numberOfLoadedClasses; i++ )
	{
		Class* cls= loadedClasses[i];
		
		if( strcmp(cls->className, className) == 0 )
			return cls;
	}

	return NULL;
}

/* Returns an already loaded class, or tries to load the requested class if it is not present in the method area yet, and returns it afterwards.
   If the class has not been found and can not be loaded, the VM stops execution. */
Class* ma_getClass( const char* className )
{
	Class* cls= ma_containsClass( className );
	
	/* found and done */
	if( cls != NULL )
		return cls;
	
	/* Not found? Try to load it. */
	cls= ma_loadClass( className );
	
	/* Class present now? */
	if( cls != NULL )
		return cls;
	
	/* Still not available? -> Error! */
	logError( "Could not find class %s!\n", className );
	error( "VM haltet." );
	return NULL; /* never reached */
}