/*
 *  types.c
 *  Pura
 *
 *  Created by Daniel Klein on 30.01.07.
 *  Copyright 2007 __MyCompanyName__. All rights reserved.
 *
 */

#include "types.h"

void types_testTypes()
{
	/* TODO: Add complete set of type tests here! */
	
	/*logVerbose( "Testing type size assumptions...\n" );*/
	
	/* int64/uint64 size test */
	int uint64Size= sizeof( uint64 );
	/*logVerbose( "uint64 should be 8 and is %i.\n", uint64Size );*/
	if( uint64Size != 8 )
		error( "Type size mismatch! Please check type settings in type.h and recompile!" );
	
	int int64Size= sizeof( int64 );
	/*logVerbose( "int64 should be 8 and is %i.\n", int64Size );*/
	if( int64Size != 8 )
		error( "Type size mismatch! Please check type settings in type.h and recompile!" );
	
	/* check system pointer size -> must not be larger than slot size */
}