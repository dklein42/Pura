/*
 *  stack.c
 *  Pura
 *
 *  Created by Daniel Klein on 28.12.06.
 *  Copyright 2006 __MyCompanyName__. All rights reserved.
 *
 */

#include "puraGlobals.h"
#include "class.h"
#include "memoryManager.h"
#include "stack.h"

uint32 initialStackSize= DEFAULT_STACK_SIZE;

Stack* stack_create( uint32 stackSize )
{
	logVerbose( "Creating stack with a size of %i bytes.\n", stackSize );
	
	Stack* stack= mm_staticMalloc( sizeof(Stack) );
	
	stack->basePointer= mm_staticMalloc( stackSize );
	stack->frameCount= 0;
	stack->maxSize= stackSize;
	
	return stack;
}

void stack_free( Stack* stack )
{
	logVerbose( "Freeing up stack.\n" );

	mm_staticFree( stack->basePointer );
	mm_staticFree( stack );
}

uint32 stack_getSize( Stack* stack )
{
	return ((byte*)stack->stackPointer) - stack->basePointer;
}

/* Creates an initial stack frame, so that a parameter can be passed to the main method. The parameter has to be manually pushed after this method finnishes. */
StackFrame* stack_createInitialStackFrame( Stack* stack )
{
	logVerbose( "Pushing initial stack frame.\n" );
	
	/* allocate initial stack frame */
	StackFrame* sf= (StackFrame*)stack->basePointer;
	
	/* initialize stack frame */
	sf->currentClass= NULL;
	sf->prevStackFrame= NULL;
	sf->methodInfo= NULL;
	sf->pc= (byte*)0xFFFFFFFF; /* This is only used for storing the pc if another method is called. The initial value is for easier debugging. */
	
	/* adjust stack info */
	stack->stackPointer= (slot*)(sf+1);
	stack->currentFrame= sf;
	stack->frameCount++;
	
	return sf;
}

/* Pushes a new stack frame onto the stack and handles any possible parameters. */ 
StackFrame* stack_pushFrame( Stack* stack, Class* cls, method_info* methodInfo )
{
	/* make sure that we do have enough space left on the stack */
	int expectedSize= sizeof(StackFrame) + methodInfo->code->max_locals*sizeof(slot) + methodInfo->code->max_stack*sizeof(slot);

	if( stack_getSize(stack) + expectedSize > stack->maxSize )
		error( "Stack overflow error!\n" );
		/* TODO: Print stack trace here! */

	/* Copy the parameters to their new destination first, so that we can overwrite them on the old position. But we have to copy them in reverse oder, so that we
		don't overwrite any parameter, no matter how many we have. */
	if( methodInfo->parameterSlotCount > 0 )
	{
		slot* newLocals= (slot*)( ((byte*)(stack->stackPointer - 1)) + sizeof(StackFrame) ); /* - parameterSlotCount + parameterSlotCount */
		slot* prevOperands= stack->stackPointer - 1;

		int i;
		for( i= 0; i < methodInfo->parameterSlotCount; i++ )
		{
			*newLocals= *prevOperands;
			newLocals--;
			prevOperands--;
		}
	}
	
	/* allocate stack frame */
	StackFrame* sf= (StackFrame*)( ((slot*)stack->stackPointer) - methodInfo->parameterSlotCount );
	
	/* initialize stack frame */
	sf->prevStackFrame= stack->currentFrame;
	sf->currentClass= cls;
	sf->methodInfo= methodInfo;
	sf->pc= (byte*)0xFFFFFFFF; /* This value is only for debugging purposes. Otherwise pc is unused until the next method call. */
	
	/* adjust stack info */
	stack->stackPointer= ((slot*)(sf+1))+methodInfo->code->max_locals;
	stack->currentFrame= sf;
	stack->frameCount++;
	
	logVerbose( "\tPushing new stack frame.\n\tFrame number %i, size %i bytes, %i parameter slots, stack is now %i bytes high.\n", stack->frameCount, expectedSize, methodInfo->parameterSlotCount, stack_getSize(stack) );
	return sf;
}

StackFrame* stack_popFrame( Stack* stack )
{
	/* remove stack frame and re-establish the previous one */
	StackFrame* sf= stack->currentFrame;
	stack->frameCount--;
	stack->currentFrame= sf->prevStackFrame;
	stack->stackPointer= (slot*)sf;
	
	logVerbose( "\tPopping stack frame off the stack. Back at frame %i, height %i bytes.\n", stack->frameCount, stack_getSize(stack) );
	return stack->currentFrame;
}

slot stack_getLocalVariable( Stack* stack, int index )
{
	slot* base= (slot*)(stack->currentFrame+1);
	return base[index];
}

void stack_setLocalVariable( Stack* stack, int index, slot value )
{
	slot* base= (slot*)(stack->currentFrame+1);
	base[index]= value;
}

void stack_printStackTrace( Stack* stack )
{
	StackFrame* cf= stack->currentFrame;
	
	logError( "Stack Trace:\n" );
	
	while( cf != NULL )
	{
		logError( "%s%s PC:%i\n", cf->methodInfo->name, cf->methodInfo->descriptor, cf->pc );
		cf= cf->prevStackFrame;
	}
}

/* operations on the operand stack of the current/given stack frame */

void stack_pushSlot( Stack* stack, int32 value )
{
	*stack->stackPointer= value;
	stack->stackPointer++;
}

int32 stack_popSlot( Stack* stack )
{
	stack->stackPointer--;
	return *stack->stackPointer;
}

void stack_pushLongParts( Stack* stack, uint32 value1, uint32 value2 )
{
	*stack->stackPointer= value1;
	stack->stackPointer++;

	*stack->stackPointer= value2;
	stack->stackPointer++;
}

void stack_pushLong( Stack* stack, uint64 value )
{
	*stack->stackPointer= (uint32)((value & 0xFFFFFFFF00000000ll) >> 32);
	stack->stackPointer++;

	*stack->stackPointer= (uint32)(value & 0xFFFFFFFFll);
	stack->stackPointer++;
}

uint64 stack_popLong( Stack* stack )
{
	stack->stackPointer--;
	uint32 value2= *stack->stackPointer;
	stack->stackPointer--;
	uint32 value1= *stack->stackPointer;
	uint64 value= ((uint64)value1 << 32) | value2;
	
	return value;
}

void stack_pushFloat( Stack* stack, float f )
{
	uint32* fp= (uint32*)&f;
	uint32 value= *fp;
	stack_pushSlot( stack, value );
}

float stack_popFloat( Stack* stack )
{
	uint32 value= stack_popSlot( stack );
	float* vp= (float*)&value;
	return *vp;
}

void stack_pushDouble( Stack* stack, double d )
{
	uint64* dp= (uint64*)&d;
	uint64 value= *dp;
	stack_pushLong( stack, value );
}

double stack_popDouble( Stack* stack )
{
	uint64 value= stack_popLong( stack );
	double* dp= (double*)&value;
	return *dp;
}

void stack_pushByte( Stack* stack, int8 value )
{
	int32 extValue= (int32)value;
	stack_pushSlot( stack, extValue );
}

int8 stack_popByte( Stack* stack )
{
	uint32 value= stack_popSlot( stack );
	return (int8)value;
}

void stack_pushShort( Stack* stack, int16 value )
{
	int32 extValue= (int32)value;
	stack_pushSlot( stack, extValue );
}

int16 stack_popShort( Stack* stack )
{
	uint32 value= stack_popSlot( stack );
	return (int16)value;
}

void stack_pushChar( Stack* stack, uint16 value )
{
	uint32 extValue= (uint32)value;
	stack_pushSlot( stack, extValue );
}

uint16 stack_popChar( Stack* stack )
{
	uint32 value= stack_popSlot( stack );
	return (uint16)value;
}
