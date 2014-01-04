/*
 *  stack.h
 *  Implements the Java Stack and the according creation-, handling-, push- and pop-methods.
 *
 *  Copyright (c) 2006, 2007 Daniel Klein. All rights reserved.
 *  
 *  Licensed under GPL version 2.
 *  See here for the full license: http://www.gnu.org/licenses/gpl.html
 */

#ifndef _stack_h_
#define _stack_h_

#include "class.h"

#define DEFAULT_STACK_SIZE 10240

extern uint32 initialStackSize;

typedef struct sStack_frame
{
	struct sStack_frame* prevStackFrame; /*previous stack frame bp*/
	Class* currentClass;
	method_info* methodInfo;
	byte* pc; /* program counter storage (Only used to store the pc if another method is invoked on top of this one.) */
} StackFrame;

typedef struct sStack
{
	StackFrame* currentFrame;
	slot* stackPointer;
	uint32 frameCount;
	uint32 maxSize;
	byte* basePointer;
} Stack;

/* stack methods */
Stack* stack_create( uint32 stackSize );
void stack_free( Stack* stack );

StackFrame* stack_createInitialStackFrame( Stack* stack );
StackFrame* stack_pushFrame( Stack* stack, Class* cls, method_info* methodInfo );
StackFrame* stack_popFrame( Stack* stack );

void stack_printStackTrace( Stack* stack );

/* operations on the operand stack of the current stack frame */
void stack_pushSlot( Stack* stack, int32 value );
int32 stack_popSlot( Stack* stack );
void stack_pushByte( Stack* stack, int8 value );
int8 stack_popByte( Stack* stack );
void stack_pushShort( Stack* stack, int16 value );
int16 stack_popShort( Stack* stack );
void stack_pushChar( Stack* stack, uint16 value );
uint16 stack_popChar( Stack* stack );

void stack_pushLongParts( Stack* stack, uint32 value1, uint32 value2 );
void stack_pushLong( Stack* stack, uint64 value );
uint64 stack_popLong( Stack* stack );

void stack_pushFloat( Stack* stack, float f );
float stack_popFloat( Stack* stack );

void stack_pushDouble( Stack* stack, double d );
double stack_popDouble( Stack* stack );

slot stack_getLocalVariable( Stack* stack, int index );
void stack_setLocalVariable( Stack* stack, int index, slot value );

uint32 stack_getSize( Stack* stack );

#endif /*_stack_h_*/
