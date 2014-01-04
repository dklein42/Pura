/*
 *  interpreter.h
 *  Pura
 *
 *  Created by Daniel Klein on 06.11.06.
 *  Copyright 2006 __MyCompanyName__. All rights reserved.
 *
 */

#ifndef _interpreter_h_
#define _interpreter_h_

#include "types.h"
#include "stack.h"

#define ARRAY_TYPE_BOOLEAN 4
#define ARRAY_TYPE_CHAR		5
#define ARRAY_TYPE_FLOAT	6
#define ARRAY_TYPE_DOUBLE	7
#define ARRAY_TYPE_BYTE		8
#define ARRAY_TYPE_SHORT	9
#define ARRAY_TYPE_INT		10
#define ARRAY_TYPE_LONG		11

extern boolean opcodeStatsEnabled;

void interpreter_start( const char* mainClass );

#endif /*_interpreter_h_*/