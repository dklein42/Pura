/*
 *  native.c
 *  Handling of native method calls. Please note, that this is a proprietary API which does not conform
 *  to the Java Native Interface. (This API is much simpler and does not involve the loading of external
 *  native libraries.)
 *
 *  Copyright (c) 2006, 2007 Daniel Klein. All rights reserved.
 *  
 *  Licensed under GPL version 2.
 *  See here for the full license: http://www.gnu.org/licenses/gpl.html
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <sys/time.h>
#include "puraGlobals.h"
#include "stack.h"
#include "heap.h"
#include "class.h"
#include "methodArea.h"
#include "native.h"

/* All native method implementation-functions use the following semantics:
   my.package.name.MyClass.myMethod() becomes the function my_method_name_MyClass_myMethod() */

/* non static methods -> instance reference in first parameter
	return the number of slots that you push onto the stack as return parameters -> use sf_pushXY( stack->currentFrame, value ) */
/* Print String. Get the internal char[] and print the chars one after another. */
int java_io_PrintStream_print_String( Class* cls, int parameterSlotCount, slot* parameters, Stack* stack )
{
	Class* stringClass= ma_getClass( "java/lang/String" );
	variable* varInfo= cls_resolveField( &stringClass, "value", "[C" );
	reference charArray= heap_getSlotFromInstance( parameters[1], stringClass, varInfo->slot_index );
	
	int i;
	int len= heap_getArraySize(charArray);
	for( i= 0; i < len; i++ )
	{
		uint16 ch= heap_getShortFromArray( charArray, i );
		putchar( ch );
	}
	
	return 0;
}

int java_io_PrintStream_print_int( Class* cls, int parameterSlotCount, slot* parameters, Stack* stack )
{
	printf( "%i", parameters[1] );
	return 0;
}

int java_io_PrintStream_print_float( Class* cls, int parameterSlotCount, slot* parameters, Stack* stack )
{
	printf( "%f", (float)parameters[1] );
	return 0;
}

int java_io_PrintStream_print_long( Class* cls, int parameterSlotCount, slot* parameters, Stack* stack )
{
	int64 value= (int64)(((uint64)parameters[1] << 32) | parameters[2]);
	printf( "%lli", value );
	return 0;
}

int java_io_PrintStream_print_double( Class* cls, int parameterSlotCount, slot* parameters, Stack* stack )
{
	double value= (double)(((uint64)parameters[1] << 32) | parameters[2]);
	printf( "%f", value );
	return 0;
}

/* Resolve the (internal) address of the objectref (first parameter on the stack) and return it. */
int java_lang_Object_hashCode( Class* cls, int parameterSlotCount, slot* parameters, Stack* stack )
{
	slot value= heap_getAddressOfInstance( parameters[0] );
	stack_pushSlot( stack, value );
	return 1;
}

/* Return the Class name of the instance of the given reference as a String.
   Note: This always creates a new String instance due to missing internalization of strings.*/
int java_lang_Object_getClassName( Class* cls, int parameterSlotCount, slot* parameters, Stack* stack )
{
	Class* classOfRef= heap_getClassOfInstance( parameters[0] );
	reference string= heap_newStringInstance( classOfRef->className );
	stack_pushSlot( stack, string );
	return 1;
}

/* Returns the current stack trace depth of the Java stack. This excludes all stack trace- and exeption-handling methods. */ 
int java_lang_Throwable_getStackTraceDepth( Class* cls, int parameterSlotCount, slot* parameters, Stack* stack )
{
	/* We use this to determine where to start with the real stack trace. Everything above belongs to the initialization of the exception instances. */
	Class* originalException= heap_getClassOfInstance( parameters[0] );
	
	/* Now step down the stack until we reach the initializer (constructor) of the original exception that we're currently recursively initializing. */
	StackFrame* sf= stack->currentFrame;
	int exceptionInitCount= 0;
	while( sf->currentClass != originalException )
	{
		sf= sf->prevStackFrame;
		exceptionInitCount++;
	}
	
	/* This is the initializer of the original constructor, so finally skip this one, too,  and we've reached our starting point. */
	sf= sf->prevStackFrame;
	exceptionInitCount++;
	
	/* Return depth of the current stack. */
	stack_pushSlot( stack, stack->frameCount - exceptionInitCount - 1 );
	return 1;
}

/* Return the n-th (stored in parameter 1) StackTraceElement, not counting stack trace/exception creation method calls. */
/* TODO: Implement line number (table) support. */
int java_lang_Throwable_getStackTraceElement( Class* cls, int parameterSlotCount, slot* parameters, Stack* stack )
{
	/* We use this to determine where to start with the real stack trace. Everything above belongs to the initialization of the exception instances. */
	Class* originalException= heap_getClassOfInstance( parameters[0] );
	
	/* Return StackTraceElement for stack frame n counting from top (i.e. the current one), but omitting the Exception initializers. */
	int32 frameDepth= parameters[1];
	
	/* frameDepth must not be higher than the stack's real height. */
	if( frameDepth > stack->frameCount )
		error( "Stack access error while tracing the stack." );
	
	/* Now step down the stack until we reach the initializer (constructor) of the original exception that we're currently recursively initializing. */
	StackFrame* sf= stack->currentFrame;
	
	while( sf->currentClass != originalException )
		sf= sf->prevStackFrame;
	
	/* This is the initializer of the original constructor, so finally skip this one, too, and we've reached our starting point. */
	sf= sf->prevStackFrame;
	
	/* Now, step down the stack, stack frame by stack frame, to the according stack frame number, staring from here. */
	int i;
	for( i= 0; i < frameDepth; i++ )
		sf= sf->prevStackFrame;
	
	/* Create the StackTraceElement now and initialize its values. */
	Class* steClass= ma_getClass("java/lang/StackTraceElement");
	reference ste= heap_newInstance( steClass );
	
	/* declaringClass */
	reference strClassName= heap_newStringInstance( sf->currentClass->className );
	heap_setSlotOfInstance( ste, steClass, 0, strClassName ); 
	
	/* methodName */
	reference strMethodName= heap_newStringInstance( sf->methodInfo->name );
	heap_setSlotOfInstance( ste, steClass, 1, strMethodName ); 
	
	/* fileName */
	if( sf->currentClass->sourceFileName != NULL )
	{
		reference strFileName= heap_newStringInstance( sf->currentClass->sourceFileName );
		heap_setSlotOfInstance( ste, steClass, 2, strFileName );
	}
	else
	{
		heap_setSlotOfInstance( ste, steClass, 2, NULL_REFERENCE );
	}
	
	/* lineNumber -> TODO: Not supported yet, we have to parse the LineNumberTable attribute before we can use this. */
	heap_setSlotOfInstance( ste, steClass, 3, -1 ); 
	
	stack_pushSlot( stack, ste );
	return 1;
}

/* static methods -> no instance reference supplied! */
/* Return the current time in milliseconds.  */
int java_lang_System_currentTimeMillis( Class* cls, int parameterSlotCount, slot* parameters, Stack* stack )
{
	struct timeval tv;
	gettimeofday( &tv, NULL );

	int64 currTime= (int64)tv.tv_sec * 1000;
	currTime+= (int64)tv.tv_usec / 1000;
	
	/* If gettimeofday() is not supported, time() may be used. But it only has an accuracy of one second(!).
	int64 currTime= (int64)time( NULL ); */
	
	stack_pushLong( stack, currTime );	
	return 2;
}

/* This is where we decide which method to call. The return value is the number of slots on the stack that are return values. Return values should be normally pushed
	onto the stack. */
/* TODO: Is there a way to do this faster? Possibly via method pointers in the method_info structure? */
int doNativeMethodCall( Class* cls, method_info* methodInfo, int parameterSlotCount, slot* parameters, Stack* stack )
{
	const char* className= cls->className;
	char* methodName= methodInfo->name;
	char* methodDescriptor= methodInfo->descriptor;
	
	/* Now look for the appropriate native method. */

	/* All methods for java.lang.System go here */
	if( strcmp(className, "java/lang/System") == 0 )
	{
		if( strcmp(methodName, "currentTimeMillis") == 0 && strcmp(methodDescriptor, "()J") == 0 )
			return java_lang_System_currentTimeMillis( cls, parameterSlotCount, parameters, stack );
	}

	/* All methods for java.lang.Object go here */
	else if( strcmp(className, "java/lang/Object") == 0 )
	{
		if( strcmp(methodName, "hashCode") == 0 && strcmp(methodDescriptor, "()I") == 0 )
			return java_lang_Object_hashCode( cls, parameterSlotCount, parameters, stack );
		if( strcmp(methodName, "getClassName") == 0 && strcmp(methodDescriptor, "()Ljava/lang/String;") == 0 )
			return java_lang_Object_getClassName( cls, parameterSlotCount, parameters, stack );
	}
	
	/* All methods for java.io.PrintStream go here */
	else if( strcmp(className, "java/io/PrintStream") == 0 )
	{
		if( strcmp(methodName, "print") == 0 && strcmp(methodDescriptor, "(Ljava/lang/String;)V") == 0 )
			return java_io_PrintStream_print_String( cls, parameterSlotCount, parameters, stack );
		else if( strcmp(methodName, "print") == 0 && strcmp(methodDescriptor, "(I)V") == 0 )
			return java_io_PrintStream_print_int( cls, parameterSlotCount, parameters, stack );
		else if( strcmp(methodName, "print") == 0 && strcmp(methodDescriptor, "(J)V") == 0 )
			return java_io_PrintStream_print_long( cls, parameterSlotCount, parameters, stack );
		else if( strcmp(methodName, "print") == 0 && strcmp(methodDescriptor, "(F)V") == 0 )
			return java_io_PrintStream_print_long( cls, parameterSlotCount, parameters, stack );
		else if( strcmp(methodName, "print") == 0 && strcmp(methodDescriptor, "(D)V") == 0 )
			return java_io_PrintStream_print_long( cls, parameterSlotCount, parameters, stack );
	}
	
	else if( strcmp(className, "java/lang/Throwable") == 0 )
	{
		if( strcmp(methodName, "getStackTraceDepth") == 0 && strcmp(methodDescriptor, "()I") == 0 )
			return java_lang_Throwable_getStackTraceDepth( cls, parameterSlotCount, parameters, stack );
		else if( strcmp(methodName, "getStackTraceElement") == 0 && strcmp(methodDescriptor, "(I)Ljava/lang/StackTraceElement;") == 0 )
			return java_lang_Throwable_getStackTraceElement( cls, parameterSlotCount, parameters, stack );
	}
	
	error( "Native method call failed!" );
	return 0; /* just for the sacke of the compilers happyness */
}

/* Preparation and cleanup for a native method call. */
void native_handleNativeMethodCall( Class* cls, method_info* methodInfo, Stack* stack )
{
	/* Count parameters and create pointer to first parameter. */
	int parameterSlotCount= methodInfo->parameterSlotCount;
	slot* parameters= stack->stackPointer - parameterSlotCount;
	
	/* method call */
	int numberOfReturnValues= doNativeMethodCall( cls, methodInfo, parameterSlotCount, parameters, stack );
	
	/* Copy the return parameter (one or two slots) to the correct position. */
	int i;
	for( i= 0; i < numberOfReturnValues; i++ )
		parameters[i]= parameters[i+parameterSlotCount];
	
	/* Remove a number of slots, according to the number of the parameters for this call. Due to the fact that the operand stack pointer has been increased by the 
		number of slots for the return values, and that we have copied them to the beginning of the parameters, we correctly set the operand stack pointer by the number
		of parameters now and return the then remaining return values. */
	stack->stackPointer-= parameterSlotCount;
}