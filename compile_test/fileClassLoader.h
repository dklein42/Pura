/*
 *  fileClassLoader.h
 *  Pura
 *
 *  Created by Daniel Klein on 06.11.06.
 *  Copyright 2006 __MyCompanyName__. All rights reserved.
 *
 */

#ifndef _fileClassLoader_h_
#define _fileClassLoader_h_

typedef struct sFileClassLoaderState
{
	byte* data;
	byte* currentPosition;
	uint32 size;
} ClassLoaderState;

void cl_init( ClassLoaderState* state, const char* className );
void cl_free( ClassLoaderState* state );

void cl_readBytes( ClassLoaderState* state, int numBytes, byte* data );
void cl_skipBytes( ClassLoaderState* state, int numBytes );
u1 cl_peekNextU1( ClassLoaderState* state );
u2 cl_peekNextU2( ClassLoaderState* state );

u1 cl_readU1( ClassLoaderState* state );
u2 cl_readU2( ClassLoaderState* state );
u4 cl_readU4( ClassLoaderState* state );

#endif /*_fileClassLoader_h_*/