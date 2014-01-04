/*
 *  interpreter.h
 *  Implements the interpreter loop and the according handling and setup.
 *
 *  Copyright (c) 2006, 2007 Daniel Klein. All rights reserved.
 *  
 *  Licensed under GPL version 2.
 *  See here for the full license: http://www.gnu.org/licenses/gpl.html
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