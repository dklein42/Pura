/*
 *  interpreter.c
 *  Pura
 *
 *  Created by Daniel Klein on 06.11.06.
 *  Copyright 2006 __MyCompanyName__. All rights reserved.
 *
 */

#include <stdio.h>
#include <string.h>
#include <math.h>
#include "memoryManager.h"
#include "puraGlobals.h"
#include "methodArea.h"
#include "stack.h"
#include "class.h"
#include "heap.h"
#include "opcodes.h"
#include "native.h"
#include "interpreter.h"

#define MAIN_METHOD_NAME "main"
#define MAIN_METHOD_DESCRIPTOR "([Ljava/lang/String;)V"

#define STR_STATIC_INITIALIZER_METHOD_NAME "<clinit>"
#define STR_STATIC_INITIALIZER_METHOD_DESCRIPTOR "()V"

void interpreter_interpret( Stack* stack ); 

uint32 numberOfBytecodesExecuted= 0;
boolean opcodeStatsEnabled= false;
uint32* opcodeCount= NULL;

void showOpcodeStats()
{
	if( !opcodeStatsEnabled )
		return;
	
	printf( "\nOpcode Statistics:\n" );
	
	int i;
	for( i= 0; i < 256; i++ )
		if( opcodeCount[i] > 0 )
			printf( "Opcode %13s: %7i (%4.1f%%)\n", opcodeNames[i], opcodeCount[i], (opcodeCount[i]*100)/(float)numberOfBytecodesExecuted );
		
}

void directParameterlessStaticMethodCall( Stack* stack, Class* cls, method_info* method )
{
	/* simulate the case that this frame is the only one on the stack (same as the main method), so that the interpreter function returns from the interpreter loop afterwards */
	StackFrame* prevStackFrame= stack->currentFrame->prevStackFrame;
	stack->currentFrame->prevStackFrame= NULL;
	
	stack_pushFrame( stack, cls, method );
	interpreter_interpret( stack );
	
	/* restore previous stack frame pointer and continue as normal */
	/* TODO: use popFrame here! */
	stack->currentFrame->prevStackFrame= prevStackFrame;
}

/* initialize the given class */
void handleClassInitialization( Stack* stack, Class* cls )
{
	/* Already initialized? -> Nothing to do. */
	if( cls->isInitialized )
		return;
	
	/* Check if the superclass has already been initialized. If not, do so recursively. 
	If the super_class entry is 0, this is our terminating condition, because the current class is java.lang.Object. */
	if( cls->superClass != NULL )
		handleClassInitialization( stack, cls->superClass );
	
	/* Mark as initialized, so that we don't get stuck in a recursion loop while executing the initializer. */ 
	cls->isInitialized= true;
	
	/* Check if there is a "<clinit>" method present in this class. If yes, execute it.*/
	method_info* clInitMethod=	cls_getMethod( cls, STR_STATIC_INITIALIZER_METHOD_NAME, STR_STATIC_INITIALIZER_METHOD_DESCRIPTOR );
	
	if( clInitMethod )
		directParameterlessStaticMethodCall( stack, cls, clInitMethod );	
	
	/* TODO: Do we have to do anything else here? */
}

void initSystemClasses( Stack* stack )
{
	Class* objectClass= ma_getClass( "java/lang/Object" );
	handleClassInitialization( stack, objectClass );
	
	Class* stringClass= ma_getClass( "java/lang/String" );
	handleClassInitialization( stack, stringClass );
	
	/* TODO: Add more here as required. */
}

reference createArgumentArray()
{
	/* Count the number of arguments here. */
	int numArgs= 0;
	const char** arg= mainClassArguments;
	while( *arg != NULL )
	{
		arg++;
		numArgs++;
	}
	
	logVerbose( "Passing %i arguments.\n", numArgs );
	
	/* Create the String[]. */
	reference stringArray= heap_newOneSlotArrayInstance( numArgs, ma_getClass("[Ljava/lang/String;") );
	
	/* And now create a String object for every argument and put its reference into the array. */
	arg= mainClassArguments;
	int i;
	for( i= 0; i < numArgs; i++, arg++ )
	{
		reference str= heap_newStringInstance( *arg );
		heap_setSlotInArray( stringArray, i, str );
	}
	
	return stringArray;
}

void interpreter_start( const char* mainClass )
{
	/* initialize opcode counter */
	opcodeCount= mm_staticMalloc( 256*sizeof(uint32) );
	
	int i;
	for( i= 0; i < 256; i++ )
		opcodeCount[i]= 0;
	
	/* make sure the given class is loaded and get a pointer */
	Class* cls= ma_getClass( mainClass );
		
	/* find main method */
	method_info* mainMethod= cls_getMethod( cls, MAIN_METHOD_NAME, MAIN_METHOD_DESCRIPTOR );
	
	/* create a new stack */
	Stack* stack= stack_create( initialStackSize );
	
	/* setup initial stack frame (for parameter passing to the main method */
	stack_createInitialStackFrame( stack );
	
	/* Pre-Initialize classes, which the JVM may manually create later. We have to do this, because the VM sometimes can not verifiy if the required classes have already
		been initialized if they are manually (i.e. natively within JVM code) created. */
	initSystemClasses( stack );
	
	/* Create a String[] from the arguments and push the according reference onto the current pre-main-method stack frame. It will be copied as the parameter when the
		stack frame for the main method is being pushed afterwards. */
	stack_pushSlot( stack, createArgumentArray() );

	/* setup stack frame for method */
	stack_pushFrame( stack, cls, mainMethod );
	
	/* push parameter onto the stack */
	/* TODO: Implement arrays before we can do this! Until then, code must not refer to the parameter! */
	
	/* execute main method */
	interpreter_interpret( stack );
	
	/* free stack */
	/* Note: This is not necessary, because we're not executing multithreaded (yet), but we may do so later. */
	stack_free( stack );
	
	/* Done executing the main-method. Tell statistics. */
	logVerbose( "\nExecution finished. %i Bytecodes executed.\n", numberOfBytecodesExecuted );
	
	/* Show opcode statistics. */
	showOpcodeStats();
}

u2 getNextU2( byte* pc )
{
	u2 data= *pc << 8;
	pc++;
	data|= *pc;
	pc++;
	return data;
}

void unsupportedError( byte* pc )
{
	logError( "Unsupported opcode %s!\n", opcodeNames[*pc] );
	error( "Execution haltet.\n" );
}

char getTypeOfLastArrayOfMultidimensionalArray( Class* cls, uint16 index )
{
	const char* name= cls_resolveConstantPoolIndexToClassName( cls, index );
	
	while( *name != '\0' )
		name++;
	
	name--;
	return *name;
}

/* TODO: Can we do types correctly for multianewarray??? */
reference createMultiDimensionalArray( int32* values, int32 dimensions, char typeOfLast, Class* classType )
{
	if( dimensions == 1 )
	{
		switch( typeOfLast )
		{
			case BASE_TYPE_REFERENCE:
				return heap_newOneSlotArrayInstance( *values, ma_getClass("[Ljava/lang/Object;") );
				break;
			case BASE_TYPE_BOOLEAN:
			case BASE_TYPE_BYTE:
				return heap_newByteArrayInstance( *values );
				break;
			case BASE_TYPE_CHAR:
				return heap_newCharArrayInstance( *values );
				break;
			case BASE_TYPE_SHORT:
				return heap_newShortArrayInstance( *values );
				break;
			case BASE_TYPE_FLOAT:
				return heap_newFloatArrayInstance( *values );
				break;
			case BASE_TYPE_INT:
				return heap_newIntArrayInstance( *values );
				break;
			case BASE_TYPE_LONG:
				return heap_newLongArrayInstance( *values );
				break;
			case BASE_TYPE_DOUBLE:
				return heap_newDoubleArrayInstance( *values );
				break;
			case BASE_TYPE_ONE_ARRAY_DIMENSION:
			default:
				error( "Unknown type found." );
				break;
		}
	}

	reference thisRef= heap_newOneSlotArrayInstance( *values, classType );
	
	int i;
	for( i= 0; i < *values; i++ )
		heap_setSlotInArray( thisRef, i, createMultiDimensionalArray(values+1, dimensions-1, typeOfLast, classType) );
	
	return thisRef;
}

int checkSurroundedByMatchingCatchClause( uint16 localPC, StackFrame* sf, Class* throwType )
{
	uint16 exceptionTableLength= sf->methodInfo->code->exception_table_length;
	exception_table** exceptionTable= sf->methodInfo->code->exception_table_tab;

	int i;
	for( i= 0; i < exceptionTableLength; i++ )
	{
		if( exceptionTable[i]->start_pc <= localPC && exceptionTable[i]->end_pc >= localPC  )
		{
			/* Got a hit. Make sure we catch the correct exceptions here. */
			Class* catchType= cls_resolveConstantPoolIndexToClass(sf->currentClass, exceptionTable[i]->catch_type );
						
			while( throwType != NULL )
			{
				if( catchType == throwType )
					return i;

				throwType= throwType->superClass;
			}
		}
	}
	
	/* No match found. */
	return -1;
}


void interpreter_interpret( Stack* stack )
{
	/*boolean isWideOpcode= false;*/
	StackFrame* sf= stack->currentFrame;
	
	/* initialize program counter */
	byte* pc= sf->methodInfo->code->code;
	
	/* print debug info */
	logVerbose( "Executing method %s.%s%s...\n", sf->currentClass->className, 
					sf->methodInfo->name, 
					sf->methodInfo->descriptor );
	
	/* main interpreter loop */
	while( true )
	{
		logVerbose( "Executing %s\n", opcodeNames[*pc] );
		opcodeCount[*pc]++;
		
		switch( *pc )
		{
		case NOP: /* no operation opcode */
			/* do nothing */
			pc++;
			break;
				
		/* push constants onto the stack */
		case ACONST_NULL: /* u1; push null reference onto the stack */
			pc++;
			stack_pushSlot( stack, NULL_REFERENCE );
			break;
				
		case ICONST_M1: /* push integer constant -1 onto the stack */
			pc++;
			stack_pushSlot( stack, -1 );
			break;
			
		case ICONST_0: /* push integer value 0 onto the stack */
			pc++;
			stack_pushSlot( stack, 0 );
			break;
				
		case ICONST_1: /* push integer value 1 onto the stack */
			pc++;
			stack_pushSlot( stack, 1 );
			break;
					
		case ICONST_2: /* push integer value 2 onto the stack */
			pc++;
			stack_pushSlot( stack, 2 );
			break;
						
		case ICONST_3: /* push integer value 3 onto the stack */
			pc++;
			stack_pushSlot( stack, 3 );
			break;
							
		case ICONST_4: /* push integer value 4 onto the stack */
			pc++;
			stack_pushSlot( stack, 4 );
			break;
								
		case ICONST_5: /* push integer value 5 onto the stack */
			pc++;
			stack_pushSlot( stack, 5 );
			break;
									
									
		case LCONST_0: /* u1; push the long integer 0 onto the stack */
			pc++;
			stack_pushLong( stack, 0 );
			break;

		case LCONST_1: /* u1; push the long integer 1 onto the stack */
			pc++;
			stack_pushLong( stack, 1 );
			break;
			
		case FCONST_0: /* u1; push the single float 0.0 onto the stack */
			pc++;
			stack_pushFloat( stack, 0.0f );
			break;
				
		case FCONST_1: /* u1; push the single float 1.0 onto the stack */
			pc++;
			stack_pushFloat( stack, 1.0f );
			break;
					
		case FCONST_2: /* u1; push the single float 2.0 onto the stack */
			pc++;
			stack_pushFloat( stack, 2.0f );
			break;
						
						
		case DCONST_0: /* u1, u1; push the double 0.0 onto the stack */
			pc++;
			stack_pushDouble( stack, 0.0 );
			break;
							
		case DCONST_1: /* u1, u1; push the double 1.0 onto the stack */
			pc++;
			stack_pushDouble( stack, 1.0 );
			break;
								
		/* stack manipulation */
		case BIPUSH: /* u1, s1; push one signed byte onto stack (expands to 32bit) */
		{
			pc++;
			int8 value= *pc;
			pc++;
			stack_pushByte( stack, value );
			logVerbose( "\tPushing byte value %i onto the stack.\n", value );
			break;
		}
			
		case SIPUSH: /* u1, s2; push signed short (2 byte) onto stack (expands to 32bit) */
		{
			pc++;
			uint8 value1= *pc;
			pc++;
			uint8 value2= *pc;
			pc++;
			int16 value= (value1 << 8) | value2;
			stack_pushShort( stack, value );
			logVerbose( "\tPushing short value %i onto the stack.\n", value );
			break;
		}
			
		case LDC: /* u1, u1; push single-word constant onto stack */
		{
			pc++;
			u1 index= *pc;
			pc++;
			int32 value= cls_getItemFromConstantPool( sf->currentClass, index );
			stack_pushSlot( stack, value );
			logVerbose( "\tPushing value %i from constant pool index %i.\n", value, index );
			break;
		}
			
		case LDC_W: /* u1, u2; push single-word constant onto stack (wide index) */
		{
			pc++;
			uint8 index1= *pc;
			pc++;
			uint8 index2= *pc;
			pc++;
			uint16 index= (index1 << 8) | index2;
			int32 value= cls_getItemFromConstantPool( sf->currentClass, index );
			stack_pushSlot( stack, value );
			logVerbose( "\tPushing value %i from constant pool index %i.\n", value, index );
			break;
		}
			
		case LDC2_W: /* u1, u2; push two-word constant onto stack (wide index) */
		{
			pc++;
			uint8 index1= *pc;
			pc++;
			uint8 index2= *pc;
			pc++;
			uint16 index= (index1 << 8) | index2;
			uint64 value= cls_getWideItemFromConstantPool( sf->currentClass, index );
			stack_pushLong( stack, value );
			logVerbose( "\tPushing value %i from constant pool index %i.\n", value, index );
			break;
		}
				
		/* working with local variables */
		case ILOAD: /* u1, u1 (u1, u1, u2 using wide opcode); retrieve integer from local variable */
		{
			pc++;
			u1 index= *pc;
			pc++;
			int32 value= stack_getLocalVariable( stack, index );
			stack_pushSlot( stack, value );
			logVerbose( "\tPushing integer %i onto the stack, index is %i.\n", value, index );
			break;
		}
			
		case LLOAD: /* u1, u1 (u1, u1, u2 using wide opcode); retrieve long integer from local variable */
		{
			pc++;
			u1 index= *pc;
			pc++;
			uint32 value1= stack_getLocalVariable( stack, index );
			uint32 value2= stack_getLocalVariable( stack, index+1 );
			stack_pushSlot( stack, value1 );
			stack_pushSlot( stack, value2 );
			logVerbose( "\tPushing long %lli onto the stack, index is %i.\n", ((uint64)value1 << 32) | value2, index );
			break;
		}
			
		case FLOAD: /* u1, u1 (u1, u1, u2 using wide opcode); retrieve float from local variable */
		{
			pc++;
			u1 index= *pc;
			pc++;
			uint32 value= stack_getLocalVariable( stack, index );
			stack_pushSlot( stack, value );
			logVerbose( "\tPushing float %d onto the stack, index is %i.\n", (float)value, index );
			break;
		}
			
		case DLOAD: /* u1, u1 (u1, u1, u2 using wide opcode); retrieve double from local variable */
		{
			pc++;
			u1 index= *pc;
			pc++;
			uint32 value1= stack_getLocalVariable( stack, index );
			uint32 value2= stack_getLocalVariable( stack, index+1 );
			stack_pushSlot( stack, value1 );
			stack_pushSlot( stack, value2 );
			logVerbose( "\tPushing double %d onto the stack, index is %i.\n", (double)(((uint64)value1 << 32) | value2), index );
			break;
		}
			
		case ALOAD: /* u1, u1 (u1, u1, u2 using wide opcode); retrieve object reference from local variable */
		{
			pc++;
			u1 index= *pc;
			pc++;
			uint32 value= stack_getLocalVariable( stack, index );
			stack_pushSlot( stack, value );
			logVerbose( "\tPushing reference %i onto the stack, index is %i.\n", value, index );
			break;
		}
			
		case ILOAD_0: /* u1; retrieve integer from local variable 0 */
		{
			pc++;
			int32 value= stack_getLocalVariable( stack, 0 );
			stack_pushSlot( stack, value );
			logVerbose( "\tLoading integer %i from slot 0.\n", value );
			break;
		}
		
		case ILOAD_1: /* u1; retrieve integer from local variable 1 */
		{
			pc++;
			int32 value= stack_getLocalVariable( stack, 1 );
			stack_pushSlot( stack, value );
			logVerbose( "\tLoading integer %i from slot 1.\n", value );
			break;
		}
			
		case ILOAD_2: /* u1; retrieve integer from local variable 2 */
		{
			pc++;
			int32 value= stack_getLocalVariable( stack, 2 );
			stack_pushSlot( stack, value );
			logVerbose( "\tLoading integer %i from slot 2.\n", value );
			break;
		}
				
		case ILOAD_3: /* u1; retrieve integer from local variable 3 */
		{
			pc++;
			int32 value= stack_getLocalVariable( stack, 3 );
			stack_pushSlot( stack, value );
			logVerbose( "\tLoading integer %i from slot 3.\n", value );
			break;
		}
				
		case LLOAD_0: /* u1; retrieve long integer from local variable 0 */
		{
			pc++;
			uint32 value1= stack_getLocalVariable( stack, 0 );
			uint32 value2= stack_getLocalVariable( stack, 1 );
			stack_pushSlot( stack, value1 );
			stack_pushSlot( stack, value2 );
			logVerbose( "\tPushing long %lli onto the stack.\n", ((uint64)value1 << 32) | value2 );
			break;
		}
			
		case LLOAD_1: /* u1; retrieve long integer from local variable 1 */
		{
			pc++;
			uint32 value1= stack_getLocalVariable( stack, 1 );
			uint32 value2= stack_getLocalVariable( stack, 2 );
			stack_pushSlot( stack, value1 );
			stack_pushSlot( stack, value2 );
			logVerbose( "\tPushing long %lli onto the stack.\n", ((uint64)value1 << 32) | value2 );
			break;
		}
			
		case LLOAD_2: /* u1; retrieve long integer from local variable 2 */
		{
			pc++;
			uint32 value1= stack_getLocalVariable( stack, 2 );
			uint32 value2= stack_getLocalVariable( stack, 3 );
			stack_pushSlot( stack, value1 );
			stack_pushSlot( stack, value2 );
			logVerbose( "\tPushing long %lli onto the stack.\n", ((uint64)value1 << 32) | value2 );
			break;
		}
			
		case LLOAD_3: /* u1; retrieve long integer from local variable 3 */
		{
			pc++;
			uint32 value1= stack_getLocalVariable( stack, 3 );
			uint32 value2= stack_getLocalVariable( stack, 4 );
			stack_pushSlot( stack, value1 );
			stack_pushSlot( stack, value2 );
			logVerbose( "\tPushing long %lli onto the stack.\n", ((uint64)value1 << 32) | value2 );
			break;
		}
			
		case FLOAD_0: /* u1; retrieve float from local variable 0 */
		{
			pc++;
			uint32 value= stack_getLocalVariable( stack, 0 );
			stack_pushSlot( stack, value );
			logVerbose( "\tPushing float %d onto the stack.\n", (float)value );
			break;
		}
			
		case FLOAD_1: /* u1; retrieve float from local variable 1 */
		{
			pc++;
			uint32 value= stack_getLocalVariable( stack, 1 );
			stack_pushSlot( stack, value );
			logVerbose( "\tPushing float %d onto the stack.\n", (float)value );
			break;
		}
			
		case FLOAD_2: /* u1; retrieve float from local variable 2 */
		{
			pc++;
			uint32 value= stack_getLocalVariable( stack, 2 );
			stack_pushSlot( stack, value );
			logVerbose( "\tPushing float %d onto the stack.\n", (float)value );
			break;
		}
			
		case FLOAD_3: /* u1; retrieve float from local variable 3 */
		{
			pc++;
			uint32 value= stack_getLocalVariable( stack, 3 );
			stack_pushSlot( stack, value );
			logVerbose( "\tPushing float %d onto the stack.\n", (float)value );
			break;
		}
			
		case DLOAD_0: /* u1; retrieve double from local variable 0 */
		{
			pc++;
			uint32 value1= stack_getLocalVariable( stack, 0 );
			uint32 value2= stack_getLocalVariable( stack, 1 );
			stack_pushSlot( stack, value1 );
			stack_pushSlot( stack, value2 );
			logVerbose( "\tPushing double %d onto the stack.\n", (double)(((uint64)value1 << 32) | value2) );
			break;
		}
			
		case DLOAD_1: /* u1; retrieve double from local variable 1 */
		{
			pc++;
			uint32 value1= stack_getLocalVariable( stack, 1 );
			uint32 value2= stack_getLocalVariable( stack, 2 );
			stack_pushSlot( stack, value1 );
			stack_pushSlot( stack, value2 );
			logVerbose( "\tPushing double %d onto the stack.\n", (double)(((uint64)value1 << 32) | value2) );
			break;
		}
			
		case DLOAD_2: /* u1; retrieve double from local variable 2 */
		{
			pc++;
			uint32 value1= stack_getLocalVariable( stack, 2 );
			uint32 value2= stack_getLocalVariable( stack, 3 );
			stack_pushSlot( stack, value1 );
			stack_pushSlot( stack, value2 );
			logVerbose( "\tPushing double %d onto the stack.\n", (double)(((uint64)value1 << 32) | value2) );
			break;
		}
			
		case DLOAD_3: /* u1; retrieve double from local variable 3 */
		{
			pc++;
			uint32 value1= stack_getLocalVariable( stack, 3 );
			uint32 value2= stack_getLocalVariable( stack, 4 );
			stack_pushSlot( stack, value1 );
			stack_pushSlot( stack, value2 );
			logVerbose( "\tPushing double %d onto the stack.\n", (double)(((uint64)value1 << 32) | value2) );
			break;
		}
			
		case ALOAD_0: /* u1; retrieve object reference from local variable 0 */
		{
			pc++;
			uint32 value= stack_getLocalVariable( stack, 0 );
			stack_pushSlot( stack, value );
			logVerbose( "\tPushing reference %i onto the stack.\n", value );
			break;
		}
			
		case ALOAD_1: /* u1; retrieve object reference from local variable 1 */
		{
			pc++;
			uint32 value= stack_getLocalVariable( stack, 1 );
			stack_pushSlot( stack, value );
			logVerbose( "\tPushing reference %i onto the stack.\n", value );
			break;
		}
			
		case ALOAD_2: /* u1; retrieve object reference from local variable 2 */
		{
			pc++;
			uint32 value= stack_getLocalVariable( stack, 2 );
			stack_pushSlot( stack, value );
			logVerbose( "\tPushing reference %i onto the stack.\n", value );
			break;
		}
			
		case ALOAD_3: /* u1; retrieve object reference from local variable 3 */
		{
			pc++;
			uint32 value= stack_getLocalVariable( stack, 3 );
			stack_pushSlot( stack, value );
			logVerbose( "\tPushing reference %i onto the stack.\n", value );
			break;
		}
			
			/* working with arrays */
		case BALOAD: /* u1; retrieve byte/boolean from array */
		{
			pc++;
			
			int32 index= stack_popSlot( stack );
			reference arRef= stack_popSlot( stack );
			
			/* do some checks first */
			if( arRef == NULL_REFERENCE )
				error( "NullPointerException" );
			if( index < 0 || index >= heap_getArraySize(arRef)  )
				error( "ArrayIndexOutOfBoundsException" );
			
			slot value= heap_getByteFromArray( arRef, index );
			stack_pushSlot( stack, value );
			
			logVerbose( "\tLoading index %i of the array with reference %i. The value is %i.\n", index, arRef, value );
			break;
		}
			
		case CALOAD: /* u1; retrieve character from array */
		{
			pc++;
			
			int32 index= stack_popSlot( stack );
			reference arRef= stack_popSlot( stack );
			
			/* do some checks first */
			if( arRef == NULL_REFERENCE )
				error( "NullPointerException" );
			if( index < 0 || index >= heap_getArraySize(arRef)  )
				error( "ArrayIndexOutOfBoundsException" );
			
			slot value= heap_getShortFromArray( arRef, index );
			stack_pushSlot( stack, value );
			
			logVerbose( "\tLoading index %i of the array with reference %i. The value is %c.\n", index, arRef, (char)value );
			break;
		}
			
		case SALOAD: /* u1; retrieve short from array */
		{
			pc++;
			
			int32 index= stack_popSlot( stack );
			reference arRef= stack_popSlot( stack );
			
			/* do some checks first */
			if( arRef == NULL_REFERENCE )
				error( "NullPointerException" );
			if( index < 0 || index >= heap_getArraySize(arRef)  )
				error( "ArrayIndexOutOfBoundsException" );
			
			slot value= heap_getShortFromArray( arRef, index );
			stack_pushSlot( stack, value );
			
			logVerbose( "\tLoading index %i of the array with reference %i. The value is %i.\n", index, arRef, value );
			break;
		}
			
		case FALOAD: /* u1; retrieve float from array */
		case AALOAD: /* u1; retrieve object reference from array */
		case IALOAD: /* u1; retrieve integer from array */
		{
			pc++;
			
			int32 index= stack_popSlot( stack );
			reference arRef= stack_popSlot( stack );
			
			/* do some checks first */
			if( arRef == NULL_REFERENCE )
				error( "NullPointerException" );
			if( index < 0 || index >= heap_getArraySize(arRef)  )
				error( "ArrayIndexOutOfBoundsException" );
			
			slot value= heap_getSlotFromArray( arRef, index );
			stack_pushSlot( stack, value );
			
			logVerbose( "\tLoading index %i of the array with reference %i. The value is %i.\n", index, arRef, value );
			break;
		}

		case LALOAD: /* u1; retrieve long integer from array */
		case DALOAD: /* u1; retrieve double-precision float from array */
		{
			pc++;
			
			int32 index= stack_popSlot( stack );
			reference arRef= stack_popSlot( stack );
			
			/* do some checks first */
			if( arRef == NULL_REFERENCE )
				error( "NullPointerException" );
			if( index < 0 || index >= heap_getArraySize(arRef)  )
				error( "ArrayIndexOutOfBoundsException" );
			
			uint64 value= heap_getTwoSlotsFromArray( arRef, index );
			stack_pushLong( stack, value );
			
			logVerbose( "\tLoading index %i of the array with reference %i. The value is %lli.\n", index, arRef, value );
			break;
		}
			
			/* working with local variables */
		case ISTORE: /* u1, u1 (u1, u1, u2 using wide opcode); store integer in local variable */
		{	
			pc++;
			u1 index= *pc;
			pc++;
			int32 value= stack_popSlot( stack );
			stack_setLocalVariable( stack, index, value );
			logVerbose( "\tPopping integer %i from the stack, storing it to slot %i.\n", value, index );
			break;
		}
			
		case LSTORE: /* u1, u1 (u1, u1, u2 using wide opcode); store long integer in local variable */
		{	
			pc++;
			u1 index= *pc;
			pc++;
			uint32 value2= stack_popSlot( stack );
			uint32 value1= stack_popSlot( stack );
			stack_setLocalVariable( stack, index, value1 );
			stack_setLocalVariable( stack, index+1, value2 );
			logVerbose( "\tPopping long %lli from the stack, storing it to slot %i.\n", (int64)(((uint64)value1 << 32) | value2), index );
			break;
		}
			
		case FSTORE: /* u1, u1 (u1, u1, u2 using wide opcode); store float in local variable */
		{	
			pc++;
			u1 index= *pc;
			pc++;
			uint32 value= stack_popSlot( stack );
			stack_setLocalVariable( stack, index, value );
			logVerbose( "\tPopping float %d from the stack, storing it to slot %i.\n", (float)value, index );
			break;
		}
			
		case DSTORE: /* u1, u1 (u1, u1, u2 using wide opcode); store double in local variable */
		{	
			pc++;
			u1 index= *pc;
			pc++;
			uint32 value2= stack_popSlot( stack );
			uint32 value1= stack_popSlot( stack );
			stack_setLocalVariable( stack, index, value1 );
			stack_setLocalVariable( stack, index+1, value2 );
			logVerbose( "\tPopping double %d from the stack, storing it to slot %i.\n", (double)(((uint64)value1 << 32) | value2), index );
			break;
		}
			
		case ASTORE: /* u1, u1 (u1, u1, u2 if using wide opcode); store object reference in local variable */
		{	
			pc++;
			u1 index= *pc;
			pc++;
			uint32 value= stack_popSlot( stack );
			stack_setLocalVariable( stack, index, value );
			logVerbose( "\tPopping reference %i from the stack, storing it to slot %i.\n", value, index );
			break;
		}
			
		case ISTORE_0: /* u1; store integer in local variable 0 */
		{
			pc++;
			int32 value= stack_popSlot( stack );
			stack_setLocalVariable( stack, 0, value );
			logVerbose( "\tStoring integer %i from the stack into slot 0.\n", value );
			break;
		}
					
		case ISTORE_1: /* u1; store integer in local variable 1 */
		{
			pc++;
			int32 value= stack_popSlot( stack );
			stack_setLocalVariable( stack, 1, value );
			logVerbose( "\tStoring integer %i from the stack into slot 1.\n", value );
			break;
		}
						
		case ISTORE_2: /* u1; store integer in local variable 2 */
		{
			pc++;
			int32 value= stack_popSlot( stack );
			stack_setLocalVariable( stack, 2, value );
			logVerbose( "\tStoring integer %i from the stack into slot 2.\n", value );
			break;
		}
							
		case ISTORE_3: /* u1; store integer in local variable 3 */
		{
			pc++;
			int32 value= stack_popSlot( stack );
			stack_setLocalVariable( stack, 3, value );
			logVerbose( "\tStoring integer %i from the stack into slot 3.\n", value );
			break;
		}
								
		case LSTORE_0: /* u1; store long integer in local variable 0 */
		{	
			pc++;
			uint32 value2= stack_popSlot( stack );
			uint32 value1= stack_popSlot( stack );
			stack_setLocalVariable( stack, 0, value1 );
			stack_setLocalVariable( stack, 1, value2 );
			logVerbose( "\tPopping long %lli from the stack, storing it to slot 0.\n", (int64)(((uint64)value1 << 32) | value2) );
			break;
		}
			
		case LSTORE_1: /* u1; store long integer in local variable 1 */
		{	
			pc++;
			uint32 value2= stack_popSlot( stack );
			uint32 value1= stack_popSlot( stack );
			stack_setLocalVariable( stack, 1, value1 );
			stack_setLocalVariable( stack, 2, value2 );
			logVerbose( "\tPopping long %lli from the stack, storing it to slot 1.\n", (int64)(((uint64)value1 << 32) | value2) );
			break;
		}
			
		case LSTORE_2: /* u1; store long integer in local variable 2 */
		{	
			pc++;
			uint32 value2= stack_popSlot( stack );
			uint32 value1= stack_popSlot( stack );
			stack_setLocalVariable( stack, 2, value1 );
			stack_setLocalVariable( stack, 3, value2 );
			logVerbose( "\tPopping long %lli from the stack, storing it to slot 2.\n", (int64)(((uint64)value1 << 32) | value2) );
			break;
		}
			
		case LSTORE_3: /* u1; store long integer in local variable 3 */
		{	
			pc++;
			uint32 value2= stack_popSlot( stack );
			uint32 value1= stack_popSlot( stack );
			stack_setLocalVariable( stack, 3, value1 );
			stack_setLocalVariable( stack, 4, value2 );
			logVerbose( "\tPopping long %lli from the stack, storing it to slot 4.\n", (int64)(((uint64)value1 << 32) | value2) );
			break;
		}
			
		case FSTORE_0: /* u1; store float in local variable 0 */
		{	
			pc++;
			uint32 value= stack_popSlot( stack );
			stack_setLocalVariable( stack, 0, value );
			logVerbose( "\tPopping float %d from the stack, storing it to slot 0.\n", (float)value );
			break;
		}
			
		case FSTORE_1: /* u1; store float in local variable 1 */
		{	
			pc++;
			uint32 value= stack_popSlot( stack );
			stack_setLocalVariable( stack, 1, value );
			logVerbose( "\tPopping float %d from the stack, storing it to slot 1.\n", (float)value );
			break;
		}
			
		case FSTORE_2: /* u1; store float in local variable 2 */
		{	
			pc++;
			uint32 value= stack_popSlot( stack );
			stack_setLocalVariable( stack, 2, value );
			logVerbose( "\tPopping float %d from the stack, storing it to slot 2.\n", (float)value );
			break;
		}
			
		case FSTORE_3: /* u1; store float in local variable 3 */
		{	
			pc++;
			uint32 value= stack_popSlot( stack );
			stack_setLocalVariable( stack, 3, value );
			logVerbose( "\tPopping float %d from the stack, storing it to slot 3.\n", (float)value );
			break;
		}
			
		case DSTORE_0: /* u1; store double in local variable 0 */
		{	
			pc++;
			uint32 value2= stack_popSlot( stack );
			uint32 value1= stack_popSlot( stack );
			stack_setLocalVariable( stack, 0, value1 );
			stack_setLocalVariable( stack, 1, value2 );
			logVerbose( "\tPopping double %d from the stack, storing it to slot 0.\n", (double)(((uint64)value1 << 32) | value2) );
			break;
		}
			
		case DSTORE_1: /* u1; store double in local variable 1 */
		{	
			pc++;
			uint32 value2= stack_popSlot( stack );
			uint32 value1= stack_popSlot( stack );
			stack_setLocalVariable( stack, 1, value1 );
			stack_setLocalVariable( stack, 2, value2 );
			logVerbose( "\tPopping double %d from the stack, storing it to slot 1.\n", (double)(((uint64)value1 << 32) | value2) );
			break;
		}
			
		case DSTORE_2: /* u1; store double in local variable 2 */
		{	
			pc++;
			uint32 value2= stack_popSlot( stack );
			uint32 value1= stack_popSlot( stack );
			stack_setLocalVariable( stack, 2, value1 );
			stack_setLocalVariable( stack, 3, value2 );
			logVerbose( "\tPopping double %d from the stack, storing it to slot 2.\n", (double)(((uint64)value1 << 32) | value2) );
			break;
		}
			
		case DSTORE_3: /* u1; store double in local variable 3 */
		{	
			pc++;
			uint32 value2= stack_popSlot( stack );
			uint32 value1= stack_popSlot( stack );
			stack_setLocalVariable( stack, 3, value1 );
			stack_setLocalVariable( stack, 4, value2 );
			logVerbose( "\tPopping double %d from the stack, storing it to slot 3.\n", (double)(((uint64)value1 << 32) | value2) );
			break;
		}
			
		case ASTORE_0: /* u1; store object reference in local variable 0 */
		{	
			pc++;
			uint32 value= stack_popSlot( stack );
			stack_setLocalVariable( stack, 0, value );
			logVerbose( "\tPopping reference %i from the stack, storing it to slot 0.\n", value );
			break;
		}
			
		case ASTORE_1: /* u1; store object reference in local variable 1 */
		{	
			pc++;
			uint32 value= stack_popSlot( stack );
			stack_setLocalVariable( stack, 1, value );
			logVerbose( "\tPopping reference %i from the stack, storing it to slot 1.\n", value );
			break;
		}
			
		case ASTORE_2: /* u1; store object reference in local variable 2 */
		{	
			pc++;
			uint32 value= stack_popSlot( stack );
			stack_setLocalVariable( stack, 2, value );
			logVerbose( "\tPopping reference %i from the stack, storing it to slot 2.\n", value );
			break;
		}
			
		case ASTORE_3: /* u1; store object reference in local variable 3 */
		{	
			pc++;
			uint32 value= stack_popSlot( stack );
			stack_setLocalVariable( stack, 3, value );
			logVerbose( "\tPopping reference %i from the stack, storing it to slot 3.\n", value );
			break;
		}
			
			/* working with arrays */
		case BASTORE: /* u1; store in byte/boolean */
		{
			pc++;
			
			int32 value= stack_popSlot( stack );
			int32 index= stack_popSlot( stack );
			reference arRef= stack_popSlot( stack );
			
			/* do some checks first */
			if( arRef == NULL_REFERENCE )
				error( "NullPointerException" );
			if( index < 0 || index >= heap_getArraySize(arRef)  )
				error( "ArrayIndexOutOfBoundsException" );
			
			heap_setByteInArray( arRef, index, value );
			
			logVerbose( "\tSetting int %i in index %i of the array with reference %i.\n", value, index, arRef );
			break;
		}
			
		case CASTORE: /* u1; store in character array */
		{
			pc++;
			
			int32 value= stack_popSlot( stack );
			int32 index= stack_popSlot( stack );
			reference arRef= stack_popSlot( stack );
			
			/* do some checks first */
			if( arRef == NULL_REFERENCE )
				error( "NullPointerException" );
			if( index < 0 || index >= heap_getArraySize(arRef)  )
				error( "ArrayIndexOutOfBoundsException" );
			
			heap_setShortInArray( arRef, index, value );
			
			logVerbose( "\tSetting char %c in index %i of the array with reference %i.\n", (char)value, index, arRef );
			break;
		}
			
		case SASTORE: /* u1; store in short array */
		{
			pc++;
			
			int32 value= stack_popSlot( stack );
			int32 index= stack_popSlot( stack );
			reference arRef= stack_popSlot( stack );
			
			/* do some checks first */
			if( arRef == NULL_REFERENCE )
				error( "NullPointerException" );
			if( index < 0 || index >= heap_getArraySize(arRef)  )
				error( "ArrayIndexOutOfBoundsException" );
			
			heap_setShortInArray( arRef, index, value );
			
			logVerbose( "\tSetting int %i in index %i of the array with reference %i.\n", value, index, arRef );
			break;
		}
			
		case FASTORE: /* u1; store in single-precision float array */
		case AASTORE: /* u1; store object reference in array */
		case IASTORE: /* u1; store in integer array */
		{
			pc++;
			
			int32 value= stack_popSlot( stack );
			int32 index= stack_popSlot( stack );
			reference arRef= stack_popSlot( stack );
			
			/* do some checks first */
			if( arRef == NULL_REFERENCE )
				error( "NullPointerException" );
			if( index < 0 || index >= heap_getArraySize(arRef)  )
				error( "ArrayIndexOutOfBoundsException" );
			
			heap_setSlotInArray( arRef, index, value );
			
			logVerbose( "\tSetting int %i in index %i of the array with reference %i.\n", value, index, arRef );
			break;
		}
			
		case LASTORE: /* u1; store in long integer array */
		case DASTORE: /* u1; store in double-precision float array */
		{
			pc++;
			
			int32 value= stack_popSlot( stack );
			int32 index= stack_popSlot( stack );
			reference arRef= stack_popSlot( stack );
			
			/* do some checks first */
			if( arRef == NULL_REFERENCE )
				error( "NullPointerException" );
			if( index < 0 || index >= heap_getArraySize(arRef)  )
				error( "ArrayIndexOutOfBoundsException" );
			
			heap_setTwoSlotsInArray( arRef, index, value );
			
			logVerbose( "\tSetting int %i in index %i of the array with reference %i.\n", value, index, arRef );
			break;
		}
			
		/* stack managment */
		case POP: /* u1; discard top item on stack */
			pc++;
			stack_popSlot( stack );
			break;
			
		case POP2: /* u1; discard top two items on stack */
			pc++;
			stack_popSlot( stack );
			stack_popSlot( stack );
			break;
			
		case DUP: /* u1; duplicate top single item on the stack */
		{
			pc++;
			int32 value= stack_popSlot( stack );
			stack_pushSlot( stack, value );
			stack_pushSlot( stack, value );
			break;
		}
					
		case DUP_X1: /* u1; duplicate top stack item and insert beneath second item */
		{
			pc++;
			int32 value1= stack_popSlot( stack );
			int32 value2= stack_popSlot( stack );
			stack_pushSlot( stack, value1 );
			stack_pushSlot( stack, value2 );
			stack_pushSlot( stack, value1 );
			break;
		}
			
		case DUP_X2: /* u1; duplicate top stack item and insert beneath third item */
		{
			pc++;
			int32 value1= stack_popSlot( stack );
			int32 value2= stack_popSlot( stack );
			int32 value3= stack_popSlot( stack );
			stack_pushSlot( stack, value1 );
			stack_pushSlot( stack, value3 );
			stack_pushSlot( stack, value2 );
			stack_pushSlot( stack, value1 );
			break;
		}
			
		case DUP2: /* u1; duplicate top two stack items */
		{
			pc++;
			int32 value1= stack_popSlot( stack );
			int32 value2= stack_popSlot( stack );
			stack_pushSlot( stack, value2 );
			stack_pushSlot( stack, value1 );
			stack_pushSlot( stack, value2 );
			stack_pushSlot( stack, value1 );
			break;
		}
			
		case DUP2_X1: /* u1; duplicate two items and insert beneath third item */
		{
			pc++;
			int32 value1= stack_popSlot( stack );
			int32 value2= stack_popSlot( stack );
			int32 value3= stack_popSlot( stack );
			stack_pushSlot( stack, value2 );
			stack_pushSlot( stack, value1 );
			stack_pushSlot( stack, value3 );
			stack_pushSlot( stack, value2 );
			stack_pushSlot( stack, value1 );
			break;
		}
			
		case DUP2_X2: /* u1; duplicate two items and insert beneath fourth item */
		{
			pc++;
			int32 value1= stack_popSlot( stack );
			int32 value2= stack_popSlot( stack );
			int32 value3= stack_popSlot( stack );
			int32 value4= stack_popSlot( stack );
			stack_pushSlot( stack, value2 );
			stack_pushSlot( stack, value1 );
			stack_pushSlot( stack, value4 );
			stack_pushSlot( stack, value3 );
			stack_pushSlot( stack, value2 );
			stack_pushSlot( stack, value1 );
			break;
		}
			
		case SWAP: /* u1; swap top two stack items */
		{
			pc++;
			int32 value1= stack_popSlot( stack );
			int32 value2= stack_popSlot( stack );
			stack_pushSlot( stack, value1 );
			stack_pushSlot( stack, value2 );
			break;
		}
			
		/* arithmetic operators */
		case IADD: /* u1; add two integers */
		{
			pc++;
			int32 value2= stack_popSlot( stack );
			int32 value1= stack_popSlot( stack );
			int32 result= value1 + value2;
			stack_pushSlot( stack, result );
			logVerbose( "\tAdding %i and %i, result is %i.\n", value1, value2, result );
			break;
		}
			
		case LADD: /* u1; add two long integers */
		{
			pc++;
			int64 value2= stack_popLong( stack );
			int64 value1= stack_popLong( stack );
			int64 result= value1 + value2;
			stack_pushLong( stack, result );
			logVerbose( "\tAdding %i and %i, result is %i.\n", value1, value2, result );
			break;
		}
			
		case FADD: /* u1; add two floats */
		{
			pc++;
			float value2= stack_popSlot( stack );
			float value1= stack_popSlot( stack );
			float result= value1 + value2;
			stack_pushSlot( stack, result );
			logVerbose( "\tAdding %d and %d, result is %d.\n", value1, value2, result );
			break;
		}
			
		case DADD: /* u1; add two doubles */
		{
			pc++;
			double value2= stack_popLong( stack );
			double value1= stack_popLong( stack );
			double result= value1 + value2;
			stack_pushLong( stack, result );
			logVerbose( "\tAdding %d and %d, result is %d.\n", value1, value2, result );
			break;
		}
			
		case ISUB: /* u1; substract two integers */
		{
			pc++;
			int32 value2= stack_popSlot( stack );
			int32 value1= stack_popSlot( stack );
			int32 result= value1 - value2;
			stack_pushSlot( stack, result );
			logVerbose( "\tSubtracting %i from %i, result is %i.\n", value2, value1, result );
			break;
		}
			
		case LSUB: /* u1; substract two long integers */
		{
			pc++;
			int64 value2= stack_popLong( stack );
			int64 value1= stack_popLong( stack );
			int64 result= value1 - value2;
			stack_pushLong( stack, result );
			logVerbose( "\tSubtracting %i from %i, result is %i.\n", value2, value1, result );
			break;
		}
			
		case FSUB: /* u1; substract two floats */
		{
			pc++;
			float value2= stack_popSlot( stack );
			float value1= stack_popSlot( stack );
			float result= value1 - value2;
			stack_pushSlot( stack, result );
			logVerbose( "\tSubtracting %d from %d, result is %d.\n", value2, value1, result );
			break;
		}
			
		case DSUB: /* u1; substract two doubles */
		{
			pc++;
			double value2= stack_popLong( stack );
			double value1= stack_popLong( stack );
			double result= value1 - value2;
			stack_pushLong( stack, result );
			logVerbose( "\tSubtracting %d from %d, result is %d.\n", value2, value1, result );
			break;
		}
			
		case IMUL: /* u1; multiply two integers */
		{
			pc++;
			int32 value2= stack_popSlot( stack );
			int32 value1= stack_popSlot( stack );
			int32 result= value1 * value2;
			stack_pushSlot( stack, result );
			logVerbose( "\tMultiplying %i and %i, result is %i.\n", value1, value2, result );
			break;
		}
			
		case LMUL: /* u1; multiply two long integers */
		{
			pc++;
			int64 value2= stack_popLong( stack );
			int64 value1= stack_popLong( stack );
			int64 result= value1 * value2;
			stack_pushLong( stack, result );
			logVerbose( "\tMultiplying %i and %i, result is %i.\n", value1, value2, result );
			break;
		}
			
		case FMUL: /* u1; multiply two floats */
		{
			pc++;
			float value2= stack_popSlot( stack );
			float value1= stack_popSlot( stack );
			float result= value1 * value2;
			stack_pushSlot( stack, result );
			logVerbose( "\tMultiplying %d and %d, result is %d.\n", value1, value2, result );
			break;
		}
			
		case DMUL: /* u1; multiply two doubles */
		{
			pc++;
			double value2= stack_popLong( stack );
			double value1= stack_popLong( stack );
			double result= value1 * value2;
			stack_pushLong( stack, result );
			logVerbose( "\tMultiplying %d and %d, result is %d.\n", value1, value2, result );
			break;
		}
			
		case IDIV: /* u1; divides two integers */
		{
			pc++;
			int32 value2= stack_popSlot( stack );
			int32 value1= stack_popSlot( stack );
			
			if( value2 == 0 )
				error( "ArithmeticException: Division by zero." );
			
			int32 result= value1 / value2;
			stack_pushSlot( stack, result );
			logVerbose( "\tDividing %i by %i, result is %i.\n", value1, value2, result );
			break;
		}

		case LDIV: /* u1; divides two long integers */
		{
			pc++;
			int64 value2= stack_popLong( stack );
			int64 value1= stack_popLong( stack );
			
			if( value2 == 0 )
				error( "ArithmeticException: Division by zero." );
			
			int64 result= value1 / value2;
			stack_pushLong( stack, result );
			logVerbose( "\tDividing %i by %i, result is %i.\n", value1, value2, result );
			break;
		}
			
		case FDIV: /* u1; divides two floats */
		{
			pc++;
			float value2= stack_popSlot( stack );
			float value1= stack_popSlot( stack );
			
			if( value2 == 0.0f )
				error( "ArithmeticException: Division by zero." );
			
			float result= value1 / value2;
			stack_pushSlot( stack, result );
			logVerbose( "\tDividing %d by %d, result is %d.\n", value1, value2, result );
			break;
		}
			
		case DDIV: /* u1; divides two doubles */
		{
			pc++;
			double value2= stack_popLong( stack );
			double value1= stack_popLong( stack );
			
			if( value2 == 0.0 )
				error( "ArithmeticException: Division by zero." );
			
			double result= value1 / value2;
			stack_pushLong( stack, result );
			logVerbose( "\tDividing %d by %d, result is %d.\n", value1, value2, result );
			break;
		}
			
		case IREM: /* u1; remainder of two integers */
		{
			pc++;
			int32 value2= stack_popSlot( stack );
			int32 value1= stack_popSlot( stack );
			
			if( value2 == 0 )
				error( "ArithmeticException: Division by zero." );
			
			int32 result= value1 % value2;
			stack_pushSlot( stack, result );
			logVerbose( "\tRemainder of %i divided by %i is %i.\n", value1, value2, result );
			break;
		}
			
		case LREM: /* u1; remainder of two long integers */
		{
			pc++;
			int64 value2= stack_popLong( stack );
			int64 value1= stack_popLong( stack );
			
			if( value2 == 0 )
				error( "ArithmeticException: Division by zero." );
			
			int64 result= value1 % value2;
			stack_pushLong( stack, result );
			logVerbose( "\tRemainder of %i divided by %i is %i.\n", value1, value2, result );
			break;
		}
			
		case FREM: /* u1; remainder of two floats */
		{
			pc++;
			float value2= stack_popSlot( stack );
			float value1= stack_popSlot( stack );
			
			if( value2 == 0.0f )
				error( "ArithmeticException: Division by zero." );
			
			float result= fmod( value1, value2 );
			stack_pushSlot( stack, result );
			logVerbose( "\tRemainder of %d divided by %d is %d.\n", value1, value2, result );
			break;
		}
			
		case DREM: /* u1; remainder of two doubles */
		{
			pc++;
			double value2= stack_popLong( stack );
			double value1= stack_popLong( stack );
			
			if( value2 == 0.0f )
				error( "ArithmeticException: Division by zero." );
			
			double result= fmod( value1, value2 );
			stack_pushLong( stack, result );
			logVerbose( "\tRemainder of %d divided by %d is %d.\n", value1, value2, result );
			break;
		}
			
			/* logical operators */
		case INEG: /* u1; negate a integer */
		{
			pc++;
			int32 value= stack_popSlot( stack );
			stack_pushSlot( stack, -value );
			break;
		}
			
		case LNEG: /* u1; negate a long integer */
		{
			pc++;
			int64 value= stack_popLong( stack );
			stack_pushLong( stack, -value );
			break;
		}
			
		case FNEG: /* u1; negate a float */
		{
			pc++;
			float value= stack_popSlot( stack );
			stack_pushSlot( stack, -value );
			break;
		}
			
		case DNEG: /* u1; negate a double */
		{
			pc++;
			double value= stack_popLong( stack );
			stack_pushLong( stack, -value );
			break;
		}
			
		case ISHL: /* u1; integer shift left */
		{
			pc++;
			int32 value2= stack_popSlot( stack );
			int32 value1= stack_popSlot( stack );
			
			/* use the lest significant 5 bits only */
			value2&= 0x1F;
			
			int32 result= value1 << value2;
			stack_pushSlot( stack, result );
			logVerbose( "\tShifting %i %i bits to the left. Result is %i.\n", value1, value2, result );
			break;
		}
			
		case LSHL: /* u1; long integer shift left */
		{
			pc++;
			int64 value2= stack_popLong( stack );
			int64 value1= stack_popLong( stack );
			
			/* use the least significant 6 bits only */
			value2&= 0x3F;
			
			int32 result= value1 << value2;
			stack_pushLong( stack, result );
			logVerbose( "\tShifting %i %i bits to the left. Result is %i.\n", value1, value2, result );
			break;
		}
			
		case ISHR: /* u1; integer arithmetic shift right */
		{
			pc++;
			int32 value2= stack_popSlot( stack );
			int32 value1= stack_popSlot( stack );
			
			/* use the least significant 5 bits only */
			value2&= 0x1F;
			
			int32 result= value1 >> value2;
			stack_pushSlot( stack, result );
			logVerbose( "\tShifting %i %i bits to the right. Result is %i.\n", value1, value2, result );
			break;
		}
			
		case LSHR: /* u1; long integer arithmetic shift right */
		{
			pc++;
			int64 value2= stack_popLong( stack );
			int64 value1= stack_popLong( stack );
			
			/* use the least significant 6 bits only */
			value2&= 0x3F;
			
			int64 result= value1 >> value2;
			stack_pushLong( stack, result );
			logVerbose( "\tShifting %i %i bits to the right. Result is %i.\n", value1, value2, result );
			break;
		}
			
		case IUSHR: /* u1; integer logical shift right */
		{
			pc++;
			uint32 value2= stack_popSlot( stack );
			uint32 value1= stack_popSlot( stack );
			
			/* use the least significant 5 bits only */
			value2&= 0x1F;
			
			uint32 result= value1 >> value2;
			stack_pushSlot( stack, result );
			logVerbose( "\tShifting %i %i bits to the right without sign extension. Result is %i.\n", value1, value2, result );
			break;
		}

		case LUSHR: /* u1; long integer logical shift right */
		{
			pc++;
			uint64 value2= stack_popLong( stack );
			uint64 value1= stack_popLong( stack );
			
			/* use the least significant 6 bits only */
			value2&= 0x3F;
			
			uint64 result= value1 >> value2;
			stack_pushLong( stack, result );
			logVerbose( "\tShifting %i %i bits to the right without sign extension. Result is %i.\n", value1, value2, result );
			break;
		}
			
		case IAND: /* u1; integer bitwise and */
		{
			pc++;
			uint32 value2= stack_popSlot( stack );
			uint32 value1= stack_popSlot( stack );
			
			uint32 result= value1 & value2;
			stack_pushSlot( stack, result );
			logVerbose( "\tPerforming bitwise AND with %i and %i. Result is %i.\n", value1, value2, result );
			break;
		}

		case LAND: /* u1; long integer bitwise and */
		{
			pc++;
			uint64 value2= stack_popLong( stack );
			uint64 value1= stack_popLong( stack );
			
			uint64 result= value1 & value2;
			stack_pushLong( stack, result );
			logVerbose( "\tPerforming bitwise AND with %i and %i. Result is %i.\n", value1, value2, result );
			break;
		}
			
		case IOR: /* u1; integer bitwise or */
		{
			pc++;
			uint32 value2= stack_popSlot( stack );
			uint32 value1= stack_popSlot( stack );
			
			uint32 result= value1 | value2;
			stack_pushSlot( stack, result );
			logVerbose( "\tPerforming bitwise OR with %i and %i. Result is %i.\n", value1, value2, result );
			break;
		}

		case LOR: /* u1; long integer bitwise or */
		{
			pc++;
			uint64 value2= stack_popLong( stack );
			uint64 value1= stack_popLong( stack );
			
			uint64 result= value1 | value2;
			stack_pushLong( stack, result );
			logVerbose( "\tPerforming bitwise OR with %i and %i. Result is %i.\n", value1, value2, result );
			break;
		}
			
		case IXOR: /* u1; integer bitwise exclusive or */
		{
			pc++;
			uint32 value2= stack_popSlot( stack );
			uint32 value1= stack_popSlot( stack );
			
			uint32 result= value1 ^ value2;
			stack_pushSlot( stack, result );
			logVerbose( "\tPerforming bitwise XOR with %i and %i. Result is %i.\n", value1, value2, result );
			break;
		}
			
		case LXOR: /* u1; long integer bitwise exclusive or */
		{
			pc++;
			uint64 value2= stack_popLong( stack );
			uint64 value1= stack_popLong( stack );
			
			uint64 result= value1 ^ value2;
			stack_pushLong( stack, result );
			logVerbose( "\tPerforming bitwise XOR with %i and %i. Result is %i.\n", value1, value2, result );
			break;
		}
			
			/* arithmetic again */
		case IINC: /* u1, u1, s1 (u1, u1, u2, s2 using wide opcode); increment integer in local variable */
		{
			pc++;
			uint8 index= *pc;
			pc++;
			int8 constValue= *pc;
			pc++;
			
			stack_setLocalVariable( stack, index, stack_getLocalVariable(stack,index)+constValue );
			logVerbose( "\tIncrementing local variable %i by %i. Value is %i now.\n", index, constValue, stack_getLocalVariable(stack,index) );
			break;
		}
			
		/* converting */
		case I2L: /* u1; convert integer to long integer */
		{
			pc++;
			int32 value= stack_popSlot( stack );
			stack_pushLong( stack, (int64)value );
			break;
		}
			
		case I2F: /* u1; convert integer to float */
		{
			pc++;
			uint32 value= stack_popSlot( stack );
			stack_pushFloat( stack, (float)value );
			break;
		}

		case I2D: /* u1; convert integer to double */
		{
			pc++;
			uint32 value= stack_popSlot( stack );
			stack_pushDouble( stack, (double)value );
			break;
		}
			
		case L2I: /* u1; convert long integer to integer */
		{
			pc++;
			int64 value= stack_popLong( stack );
			stack_pushSlot( stack, (int32)value );
			break;
		}
			
		case L2F: /* u1; convert long integer to float */
		{
			pc++;
			int64 value= stack_popLong( stack );
			stack_pushFloat( stack, (float)value );
			break;
		}
			
		case L2D: /* u1; convert long integer to double */
		{
			pc++;
			int64 value= stack_popLong( stack );
			stack_pushDouble( stack, (double)value );
			break;
		}
			
		case F2I: /* u1; convert float to integer */
		{
			pc++;
			float value= stack_popFloat( stack );
			stack_pushSlot( stack, (int32)value );
			break;
		}
			
		case F2L: /* u1; convert float to long integer */
		{
			pc++;
			float value= stack_popFloat( stack );
			stack_pushLong( stack, (int64)value );
			break;
		}
			
		case F2D: /* u1; convert float to double */
		{
			pc++;
			float value= stack_popFloat( stack );
			stack_pushDouble( stack, (double)value );
			break;
		}
			
		case D2I: /* u1; convert double to integer */
		{
			pc++;
			double value= stack_popDouble( stack );
			stack_pushSlot( stack, (int32)value );
			break;
		}
			
		case D2L: /* u1; convert double to long integer */
		{
			pc++;
			double value= stack_popDouble( stack );
			stack_pushLong( stack, (int64)value );
			break;
		}
			
		case D2F: /* u1; convert double to float */
		{
			pc++;
			double value= stack_popDouble( stack );
			stack_pushFloat( stack, (float)value );
			break;
		}
			
		case I2B: /* u1; convert integer to byte */
		{
			pc++;
			int32 value= stack_popSlot( stack );
			stack_pushByte( stack, (int8)value );
			break;
		}
			
		case I2C: /* u1; convert integer to char */
		{
			pc++;
			int32 value= stack_popSlot( stack );
			stack_pushChar( stack, (uint16)value );
			break;
		}
			
		case I2S: /* u1; convert integer to short */
		{
			pc++;
			int32 value= stack_popSlot( stack );
			stack_pushShort( stack, (int16)value );
			break;
		}
			
			/* comparison */
		case LCMP: /* u1; long integer comparison */
		{
			pc++;
			int64 value2= stack_popLong( stack );
			int64 value1= stack_popLong( stack );
			
			/* v1 > v2 = 1; v1 < v2 = -1; v1 == v2 = 0 */ 
			int32 result= value1 > value2 ? 1 : value1 < value2 ? -1 : 0;
			stack_pushSlot( stack, result );
			break;
		}
		case FCMPL: /* u1; single precision float comparison (-1 on NaN) */
		{
			pc++;
			float value2= stack_popFloat( stack );
			float value1= stack_popFloat( stack );
			
			/* v1 > v2 = 1; v1 < v2 = -1; v1 == v2 = 0; Otherwise NaN: -1 */
			/* TODO: Do we correctly recognize NaN this way? */
			int32 result= value1 > value2 ? 1 : value1 < value2 ? -1 : value1 == value2 ? 0 : -1;
			stack_pushSlot( stack, result );
			break;
		}

		case FCMPG: /* u1; single precision float comparison (1 on NaN) */
		{
			pc++;
			float value2= stack_popFloat( stack );
			float value1= stack_popFloat( stack );
			
			/* v1 > v2 = 1; v1 < v2 = -1; v1 == v2 = 0; Otherwise NaN: 1 */
			/* TODO: Do we correctly recognize NaN this way? */
			int32 result= value1 > value2 ? 1 : value1 < value2 ? -1 : value1 == value2 ? 0 : 1;
			stack_pushSlot( stack, result );
			break;
		}
			
		case DCMPL: /* u1; comapre two doubles (-1 on NaN) */
		{
			pc++;
			double value2= stack_popDouble( stack );
			double value1= stack_popDouble( stack );
			
			/* v1 > v2 = 1; v1 < v2 = -1; v1 == v2 = 0; Otherwise NaN: -1 */
			/* TODO: Do we correctly recognize NaN this way? */
			int32 result= value1 > value2 ? 1 : value1 < value2 ? -1 : value1 == value2 ? 0 : -1;
			stack_pushSlot( stack, result );
			break;
		}
			
		case DCMPG: /* u1; compare two doubles (1 on NaN) */
		{
			pc++;
			double value2= stack_popDouble( stack );
			double value1= stack_popDouble( stack );
			
			/* v1 > v2 = 1; v1 < v2 = -1; v1 == v2 = 0; Otherwise NaN: 1 */
			/* TODO: Do we correctly recognize NaN this way? */
			int32 result= value1 > value2 ? 1 : value1 < value2 ? -1 : value1 == value2 ? 0 : 1;
			stack_pushSlot( stack, result );
			break;
		}
			
			/* conditional branching (jumps) */
		case IFEQ: /* u1, s2; jump if zero */
		{
			pc++;
			int32 value= stack_popSlot( stack ); 
			
			/* if value doens't equal 0, increment pc accordingly and continue with next bytecode */
			if( value != 0 )
			{
				pc+= 2;
				logVerbose( "Value is %i, do NOT branch.\n", value );
				break;
			}
			
			/* value equals 0, branch to target address */
			uint8 branchByte1= *pc;
			pc++;
			uint8 branchByte2= *pc;
			pc++;
			
			int16 branchOffset= (branchByte1 << 8) | branchByte2;
			pc-= 3; /* rewind pc to the original opcode address */
			pc+= branchOffset;
			
			logVerbose( "Value is 0, branch to offset %i.\n", branchOffset );
			break;
		}
			
		case IFNE: /* u1, s2; jump if non zero */
		{
			pc++;
			int32 value= stack_popSlot( stack ); 
			
			/* If value is 0, increment pc accordingly and continue with next bytecode. Do not branch. */
			if( value == 0 )
			{
				pc+= 2;
				logVerbose( "\tValue is %i, do NOT branch.\n", value );
				break;
			}
			
			/* value does not equal 0, branch to target address */
			uint8 branchByte1= *pc;
			pc++;
			uint8 branchByte2= *pc;
			pc++;
			
			int16 branchOffset= (branchByte1 << 8) | branchByte2;
			pc-= 3; /* rewind pc to the original opcode address */
			pc+= branchOffset;
			
			logVerbose( "\tValue is %i, branch to offset %i.\n", value, branchOffset );
			break;
		}
			
		case IFLT: /* u1, s2; jump if less than zero */
		{
			pc++;
			int32 value= stack_popSlot( stack ); 
			
			/* If value is 0 or greater than 0, increment pc accordingly and continue with next bytecode. Do not branch. */
			if( value == 0 || value > 0 )
			{
				pc+= 2;
				logVerbose( "Value is %i, do NOT branch.\n", value );
				break;
			}
			
			/* value is less than 0, branch to target address */
			uint8 branchByte1= *pc;
			pc++;
			uint8 branchByte2= *pc;
			pc++;
			
			int16 branchOffset= (branchByte1 << 8) | branchByte2;
			pc-= 3; /* rewind pc to the original opcode address */
			pc+= branchOffset;
			
			logVerbose( "Value is %i, branch to offset %i.\n", value, branchOffset );
			break;
		}
			
		case IFGE: /* u1, s2; jump if greater than or equal to zero */
		{
			pc++;
			int32 value= stack_popSlot( stack ); 
			
			/* If value is less than 0, increment pc accordingly and continue with next bytecode. Do not branch. */
			if( value < 0 )
			{
				pc+= 2;
				logVerbose( "Value is %i, do NOT branch.\n", value );
				break;
			}
			
			/* value is greater than or equal to 0, branch to target address */
			uint8 branchByte1= *pc;
			pc++;
			uint8 branchByte2= *pc;
			pc++;
			
			int16 branchOffset= (branchByte1 << 8) | branchByte2;
			pc-= 3; /* rewind pc to the original opcode address */
			pc+= branchOffset;
			
			logVerbose( "Value is %i, branch to offset %i.\n", value, branchOffset );
			break;
		}
			
		case IFGT: /* u1, s2; jump if greater than zero */
		{
			pc++;
			int32 value= stack_popSlot( stack ); 
			
			/* If value is 0 or less than 0, increment pc accordingly and continue with next bytecode. Do not branch. */
			if( value == 0 || value < 0 )
			{
				pc+= 2;
				logVerbose( "Value is %i, do NOT branch.\n", value );
				break;
			}
			
			/* value is greater than 0, branch to target address */
			uint8 branchByte1= *pc;
			pc++;
			uint8 branchByte2= *pc;
			pc++;
			
			int16 branchOffset= (branchByte1 << 8) | branchByte2;
			pc-= 3; /* rewind pc to the original opcode address */
			pc+= branchOffset;
			
			logVerbose( "Value is %i, branch to offset %i.\n", value, branchOffset );
			break;
		}
			
		case IFLE: /* u1, s2; jump if less than or equal to zero */
		{
			pc++;
			int32 value= stack_popSlot( stack ); 
			
			/* If value is greater than 0, increment pc accordingly and continue with next bytecode. Do not branch. */
			if( value > 0 )
			{
				pc+= 2;
				logVerbose( "Value is %i, do NOT branch.\n", value );
				break;
			}
			
			/* value is less than or equal to 0, branch to target address */
			uint8 branchByte1= *pc;
			pc++;
			uint8 branchByte2= *pc;
			pc++;
			
			int16 branchOffset= (branchByte1 << 8) | branchByte2;
			pc-= 3; /* rewind pc to the original opcode address */
			pc+= branchOffset;
			
			logVerbose( "Value is %i, branch to offset %i.\n", value, branchOffset );
			break;
		}
			
		case IF_ICMPEQ: /* u1, s2; jump if two integers are equal */
		{
			pc++;
			
			int32 value2= stack_popSlot( stack );
			int32 value1= stack_popSlot( stack );
			
			/* if v1 equals v2, branch and continue execution there */
			if( value1 == value2 )
			{
				uint8 branchByte1= *pc;
				pc++;
				uint8 branchByte2= *pc;
				pc++;
				
				int16 branchOffset= (branchByte1 << 8) | branchByte2;
				pc-= 3; /* rewind pc to the original opcode address */
				pc+= branchOffset;
				
				logVerbose( "\tValue1 %i and value2 %i are equal, branch to offset %i.\n", value1, value2, branchOffset );
				break;
			}
			
			/* do not branch and simply continue */
			pc+= 2;
			logVerbose( "\tValue1 %i and value2 %i are not equal, do NOT branch.\n", value1, value2 );
			break;
		}
			
		case IF_ICMPNE: /* u1, s2; jump if two integers are not equal */
		{
			pc++;
			
			int32 value2= stack_popSlot( stack );
			int32 value1= stack_popSlot( stack );
			
			/* if v1 does not equal v2, branch and continue execution there */
			if( value1 != value2 )
			{
				uint8 branchByte1= *pc;
				pc++;
				uint8 branchByte2= *pc;
				pc++;
				
				int16 branchOffset= (branchByte1 << 8) | branchByte2;
				pc-= 3; /* rewind pc to the original opcode address */
				pc+= branchOffset;
				
				logVerbose( "\tValue1 %i and value2 %i are not equal, branch to offset %i.\n", value1, value2, branchOffset );
				break;
			}
			
			/* do not branch and simply continue */
			pc+= 2;
			logVerbose( "\tValue1 %i and value2 %i are equal, do NOT branch.\n", value1, value2 );
			break;
		}
			
		case IF_ICMPLT: /* u1, s2; jump if one integer is less than another */
		{
			pc++;
			
			int32 value2= stack_popSlot( stack );
			int32 value1= stack_popSlot( stack );
			
			/* if v1 is less than v2, branch and continue execution there */
			if( value1 < value2 )
			{
				uint8 branchByte1= *pc;
				pc++;
				uint8 branchByte2= *pc;
				pc++;
				
				int16 branchOffset= (branchByte1 << 8) | branchByte2;
				pc-= 3; /* rewind pc to the original opcode address */
				pc+= branchOffset;
				
				logVerbose( "\tValue1 %i is less than value2 %i, branch to offset %i.\n", value1, value2, branchOffset );
				break;
			}
			
			/* do not branch and simply continue */
			pc+= 2;
			logVerbose( "\tValue1 %i is not less than value2 %i, do NOT branch.\n", value1, value2 );
			break;
		}
			
		case IF_ICMPGE: /* u1, s2; jump if one integer is greater than or equal to another */
		{
			pc++;
			
			int32 value2= stack_popSlot( stack );
			int32 value1= stack_popSlot( stack );
			
			/* if v1 is greater than or equal v2, branch and continue execution there */
			if( value1 >= value2 )
			{
				uint8 branchByte1= *pc;
				pc++;
				uint8 branchByte2= *pc;
				pc++;
				
				int16 branchOffset= (branchByte1 << 8) | branchByte2;
				pc-= 3; /* rewind pc to the original opcode address */
				pc+= branchOffset;
				
				logVerbose( "\tValue1 %i is greater than or equal value2 %i, branch to offset %i.\n", value1, value2, branchOffset );
				break;
			}
			
			/* do not branch and simply continue */
			pc+= 2;
			logVerbose( "\tValue1 %i is not greater than or equal value2 %i, do NOT branch.\n", value1, value2 );
			break;
		}
			
		case IF_ICMPGT: /* u1, s2; jump if one integer is greater than another */
		{
			pc++;
			
			int32 value2= stack_popSlot( stack );
			int32 value1= stack_popSlot( stack );
			
			/* if v1 is greater than v2, branch and continue execution there */
			if( value1 > value2 )
			{
				uint8 branchByte1= *pc;
				pc++;
				uint8 branchByte2= *pc;
				pc++;
				
				int16 branchOffset= (branchByte1 << 8) | branchByte2;
				pc-= 3; /* rewind pc to the original opcode address */
				pc+= branchOffset;
				
				logVerbose( "\tValue1 %i is greater than value2 %i, branch to offset %i.\n", value1, value2, branchOffset );
				break;
			}
			
			/* do not branch and simply continue */
			pc+= 2;
			logVerbose( "\tValue1 %i is not greater than value2 %i, do NOT branch.\n", value1, value2 );
			break;
		}
			
		case IF_ICMPLE: /* u1, s2; jump if one integer is less than or equal to another */
		{
			pc++;
			
			int32 value2= stack_popSlot( stack );
			int32 value1= stack_popSlot( stack );
			
			/* if v1 is less than or equal v2, branch and continue execution there */
			if( value1 <= value2 )
			{
				uint8 branchByte1= *pc;
				pc++;
				uint8 branchByte2= *pc;
				pc++;
				
				int16 branchOffset= (branchByte1 << 8) | branchByte2;
				pc-= 3; /* rewind pc to the original opcode address */
				pc+= branchOffset;
				
				logVerbose( "\tValue1 %i is less than or equal value2 %i, branch to offset %i.\n", value1, value2, branchOffset );
				break;
			}
			
			/* do not branch and simply continue */
			pc+= 2;
			logVerbose( "\tValue1 %i is not less than or equal value2 %i, do NOT branch.\n", value1, value2 );
			break;
		}
			
		case IF_ACMPEQ: /* u1, s2; jump if two object references are equal */
		{
			pc++;
			
			uint32 value2= stack_popSlot( stack );
			uint32 value1= stack_popSlot( stack );
			
			/* if v1 equals v2, branch and continue execution there */
			if( value1 == value2 )
			{
				uint8 branchByte1= *pc;
				pc++;
				uint8 branchByte2= *pc;
				pc++;
				
				int16 branchOffset= (branchByte1 << 8) | branchByte2;
				pc-= 3; /* rewind pc to the original opcode address */
				pc+= branchOffset;
				
				logVerbose( "\tReference 1 (%i) and reference 2 (%i) are equal, branch to offset %i.\n", value1, value2, branchOffset );
				break;
			}
			
			/* do not branch and simply continue */
			pc+= 2;
			logVerbose( "\tReference 1 (%i) and reference 2 (%i) are not equal, do NOT branch.\n", value1, value2 );
			break;
		}
			
		case IF_ACMPNE: /* u1, s2; jump if two object references are not equal */
		{
			pc++;
			
			uint32 value2= stack_popSlot( stack );
			uint32 value1= stack_popSlot( stack );
			
			/* if v1 does not equal v2, branch and continue execution there */
			if( value1 != value2 )
			{
				uint8 branchByte1= *pc;
				pc++;
				uint8 branchByte2= *pc;
				pc++;
				
				int16 branchOffset= (branchByte1 << 8) | branchByte2;
				pc-= 3; /* rewind pc to the original opcode address */
				pc+= branchOffset;
				
				logVerbose( "\tReference 1 (%i) and reference 2 (%i) are not equal, branch to offset %i.\n", value1, value2, branchOffset );
				break;
			}
			
			/* do not branch and simply continue */
			pc+= 2;
			logVerbose( "\tReference 1 (%i) and reference 2 (%i) are equal, do NOT branch.\n", value1, value2 );
			break;
		}
			
		/* non-conditional branching (simple jumps) */
		case GOTO: /* u1, s2; branch to address */
		{
			pc++;
			uint8 branchByte1= *pc;
			pc++;
			uint8 branchByte2= *pc;
			pc++;
			
			int16 branchOffset= (branchByte1 << 8) | branchByte2;
			pc-= 3; /* rewind pc to the original opcode address */
			pc+= branchOffset;
			
			logVerbose( "\tBranching to offset %i.\n", branchOffset );
			break;
		}
			
		case JSR: /* u1, s2; jump subroutine */
		{
			pc++;
			uint8 branchByte1= *pc;
			pc++;
			uint8 branchByte2= *pc;
			pc++;

			/* push the current pc, which points to the following opcode now, onto the stack */
			/* Note: We're pushing a full address with the size of a pointer of the host system onto the stack. So make sure the slot size is greater than or equal the size
				of a native pointer. */
			stack_pushSlot( stack, (uint32)pc );
			
			int16 branchOffset= (branchByte1 << 8) | branchByte2;
			pc-= 3; /* rewind pc to the original opcode address */
			pc+= branchOffset;
			break;
		}
			
		case RET: /* u1, u1 (u1, u1, u2 using wide opcode); return from subroutine */
		{
			pc++;
			uint8 index= *pc;
			pc= (byte*)stack_getLocalVariable(stack,index); /* Restore stored pc. Note that this is a native pointer! */			
			break;
		}
			
		/* switch statements */
		case TABLESWITCH: /* u1, ...; jump according to a table */
		{
			/* remember opcode start and calculate 4 byte boundary padding */
			byte* opcodeStart= pc;
			pc++;
			int padding= (pc - sf->methodInfo->code->code) % 4;
			pc+= padding;
			
			/* get index from stack */
			int32 index= stack_popSlot( stack );
			
			/* read default, low and high */
			uint8 defaultByte1= *pc;
			pc++;
			uint8 defaultByte2= *pc;
			pc++;
			uint8 defaultByte3= *pc;
			pc++;
			uint8 defaultByte4= *pc;
			pc++;
			int32 defaultOffset= (defaultByte1 << 24) | (defaultByte2 << 16) | (defaultByte3 << 8) | defaultByte4;
				
			uint8 lowByte1= *pc;
			pc++;
			uint8 lowByte2= *pc;
			pc++;
			uint8 lowByte3= *pc;
			pc++;
			uint8 lowByte4= *pc;
			pc++;
			int32 low= (lowByte1 << 24) | (lowByte2 << 16) | (lowByte3 << 8) | lowByte4;
				
			uint8 highByte1= *pc;
			pc++;
			uint8 highByte2= *pc;
			pc++;
			uint8 highByte3= *pc;
			pc++;
			uint8 highByte4= *pc;
			pc++;
			int32 high= (highByte1 << 24) | (highByte2 << 16) | (highByte3 << 8) | highByte4;
			
			/* If the given index is not within the bounds of the table of this opcode, jump to the default address. */
			if( index < low || index > high )
			{
				pc= opcodeStart + defaultOffset;
				logVerbose( "\tDefault case, index is %i, low is %i and high is %i. Branching to offset %i.\n", index, low, high, defaultOffset );
				break;
			}
			
			/* The index is within the table, so calculate according table address, fetch the branch offset (table entry) from there and branch accordingly. */
			uint32 tableIndex= (index - low) * 4; /* index entries are 4 bytes in size */ 
			pc+= tableIndex;
			
			uint8 tableEntryByte1= *pc;
			pc++;
			uint8 tableEntryByte2= *pc;
			pc++;
			uint8 tableEntryByte3= *pc;
			pc++;
			uint8 tableEntryByte4= *pc;
			pc++;
			int32 tableEntry= (tableEntryByte1 << 24) | (tableEntryByte2 << 16) | (tableEntryByte3 << 8) | tableEntryByte4;
			
			pc= opcodeStart + tableEntry;
			
			logVerbose( "\tIndex is %i, low is %i and high is %i.\n\tThe according table entry position %i has a branch offset of %i.\n", index, low, high, tableIndex/4, tableEntry );
			break;
		}
			
		case LOOKUPSWITCH: /* u1, s4, s4, ...; match key in table and jump */
		{
			/* remember opcode start and calculate 4 byte boundary padding */
			byte* opcodeStart= pc;
			pc++;
			int padding= (pc - sf->methodInfo->code->code) % 4;
			pc+= padding;
			
			/* get index from stack */
			int32 index= stack_popSlot( stack );
			
			/* read default and pairs */
			uint8 defaultByte1= *pc;
			pc++;
			uint8 defaultByte2= *pc;
			pc++;
			uint8 defaultByte3= *pc;
			pc++;
			uint8 defaultByte4= *pc;
			pc++;
			int32 defaultOffset= (defaultByte1 << 24) | (defaultByte2 << 16) | (defaultByte3 << 8) | defaultByte4;
			
			uint8 pairsByte1= *pc;
			pc++;
			uint8 pairsByte2= *pc;
			pc++;
			uint8 pairsByte3= *pc;
			pc++;
			uint8 pairsByte4= *pc;
			pc++;
			int32 pairs= (pairsByte1 << 24) | (pairsByte2 << 16) | (pairsByte3 << 8) | pairsByte4;
			
			/* Linaearly go through the following table of match/offset entries and check if match matches with the given index. */
			/* TODO: Optimize this by using bin search (divide and conquer). This is possible because the entries are sorted by the match values. */
			boolean isMatchFound= false;
			int i;
			for( i= 0; i < pairs; i++ )
			{
				uint8 matchByte1= *pc;
				pc++;
				uint8 matchByte2= *pc;
				pc++;
				uint8 matchByte3= *pc;
				pc++;
				uint8 matchByte4= *pc;
				pc++;
				int32 match= (matchByte1 << 24) | (matchByte2 << 16) | (matchByte3 << 8) | matchByte4;
				
				/* Check if the match of this table entry matches the given index. If yes, read and set the according offset and branch, 
					otherwise add 4 bytes to the pc and continue. */
				if( index == match )
				{
					uint8 offsetByte1= *pc;
					pc++;
					uint8 offsetByte2= *pc;
					pc++;
					uint8 offsetByte3= *pc;
					pc++;
					uint8 offsetByte4= *pc;
					pc++;
					int32 offset= (offsetByte1 << 24) | (offsetByte2 << 16) | (offsetByte3 << 8) | offsetByte4;
					
					pc= opcodeStart + offset;
					
					logVerbose( "\tIndex is %i, entry %i was a match, offset is %i.\n", index, i, offset );
					isMatchFound= true;
					break;
				}
				else
				{
					pc+= 4;
				}
			}
			
			/* if a match was found we're done */
			if( isMatchFound )
				break;
			
			/* if there was no match, branch to the default offset */
			pc= opcodeStart + defaultOffset;

			logVerbose( "\tDefault case, index is %i. Branching to offset %i.\n", index, defaultOffset );
			break;
		}
			
		/* return from a method */
		case IRETURN: /* u1; return from method with integer result */
		{
			pc++;
			
			/* remember return value */
			int32 retVal= stack_popSlot( stack );

			/* removed finished stack frame */
			stack_popFrame( stack );
			
			/* restore old stack frame and pc */
			sf= stack->currentFrame;
			pc= sf->pc;
			
			/* push return value back onto the operand stack */
			stack_pushSlot( stack, retVal );
			break;
		}
			
		case LRETURN: /* u1; return from method with long integer result */
		{
			pc++;
			
			/* remember return value */
			int64 retVal= stack_popLong( stack );
			
			/* removed finished stack frame */
			stack_popFrame( stack );
			
			/* restore old stack frame and pc */
			sf= stack->currentFrame;
			pc= sf->pc;
			
			/* push return value back onto the operand stack */
			stack_pushLong( stack, retVal );
			break;
		}
			
		case FRETURN: /* u1; return from method with float result */
		{
			pc++;
			
			/* remember return value */
			float retVal= stack_popFloat( stack );
			
			/* removed finished stack frame */
			stack_popFrame( stack );
			
			/* restore old stack frame and pc */
			sf= stack->currentFrame;
			pc= sf->pc;
			
			/* push return value back onto the operand stack */
			stack_pushFloat( stack, retVal );
			break;
		}
			
		case DRETURN: /* u1; return from method with double result */
		{
			pc++;
			
			/* remember return value */
			double retVal= stack_popDouble( stack );
			
			/* removed finished stack frame */
			stack_popFrame( stack );
			
			/* restore old stack frame and pc */
			sf= stack->currentFrame;
			pc= sf->pc;
			
			/* push return value back onto the operand stack */
			stack_pushDouble( stack, retVal );
			break;
		}
			
		case ARETURN: /* u1; return from method with object reference result */
		{
			pc++;
			
			/* remember return value */
			uint32 retVal= stack_popSlot( stack );
			
			/* removed finished stack frame */
			stack_popFrame( stack );
			
			/* restore old stack frame and pc */
			sf= stack->currentFrame;
			pc= sf->pc;
			
			/* push return value back onto the operand stack */
			stack_pushSlot( stack, retVal );
			break;
		}
			
		case RETURN: /* u1; return from a method */
		{
			/* removed finished stack frame */
			stack_popFrame( stack );
			
			/* restore old stack frame and pc */
			sf= stack->currentFrame;
			pc= sf->pc;
			
			/* Do we leave the main-method? -> simply return, and we're done! */
			if( sf->prevStackFrame == NULL )
				return;				

			break;
		}
			
		/* get/set static field */
		case GETSTATIC: /* u1, u2; get value of static field */
		{
			pc++;
			
			uint8 index1= *pc;
			pc++;
			uint8 index2= *pc;
			pc++;
			uint16 index= (index1 << 8) | index2;
			
			/* get field and its class */
			Class* newClass;
			variable* fieldInfo;
			cls_resolveConstantPoolIndexToClassAndVariableInfo( sf->currentClass, index, &newClass, &fieldInfo, true );
			
			/* Check if the given class is initialized. If not, initialize it now. */
			handleClassInitialization( stack, newClass );
			
			/* make sure we have a static field here */
			if( !isFlagSet(fieldInfo->access_flags, ACC_STATIC) )
				error( "IncompatibleClassChangeError: The expected field was not static." );
			
			/* handle possible different field types here and push the according value onto the stack */
			switch( *fieldInfo->descriptor )
			{
				case BASE_TYPE_REFERENCE:
				case BASE_TYPE_ONE_ARRAY_DIMENSION: /* a.k.a. array reference */
				case BASE_TYPE_BOOLEAN:
				case BASE_TYPE_BYTE:
				case BASE_TYPE_CHAR:
				case BASE_TYPE_SHORT:
				case BASE_TYPE_FLOAT:
				case BASE_TYPE_INT:
				{
					/* 32 bit, one slot variable */
					uint32 value= newClass->class_inctance_variable_slots[fieldInfo->slot_index];
					stack_pushSlot( stack, value );
					
					logVerbose( "\tThe value is %i, the slot number %i.\n", value, fieldInfo->slot_index );
					break;
				}
					
				case BASE_TYPE_LONG:
				case BASE_TYPE_DOUBLE:
				{
					/* 64 bit, two slots variable */
					uint32 value1= newClass->class_inctance_variable_slots[fieldInfo->slot_index];
					uint32 value2= newClass->class_inctance_variable_slots[fieldInfo->slot_index+1];
					stack_pushLongParts( stack, value1, value2 ); 
					break;
				}
					
				default:
					error( "Unknown type found." );
					break;
			}
			
			logVerbose( "\tGetting static field %s.%s (type is %s).\n", newClass->className, 
							fieldInfo->name, 
							fieldInfo->descriptor );
			break;
		}
			
		case PUTSTATIC: /* u1, u2; set value of static field */
		{
			pc++;
				
			uint8 index1= *pc;
			pc++;
			uint8 index2= *pc;
			pc++;
			uint16 index= (index1 << 8) | index2;
			
			/* get field and its class */
			Class* newClass;
			variable* fieldInfo;
			cls_resolveConstantPoolIndexToClassAndVariableInfo( sf->currentClass, index, &newClass, &fieldInfo, true );
				
			/* Check if the given class is initialized. If not, initialize it now. */
			handleClassInitialization( stack, newClass );
				
			/* make sure we have a static field here */
			if( !isFlagSet(fieldInfo->access_flags, ACC_STATIC) )
				error( "IncompatibleClassChangeError: The expected field was not static." );
					
				/* handle possible different field types now */
				switch( *fieldInfo->descriptor )
				{
					case BASE_TYPE_REFERENCE:
					case BASE_TYPE_ONE_ARRAY_DIMENSION: /* a.k.a. array reference */
					case BASE_TYPE_BOOLEAN:
					case BASE_TYPE_BYTE:
					case BASE_TYPE_CHAR:
					case BASE_TYPE_SHORT:
					case BASE_TYPE_FLOAT:
					case BASE_TYPE_INT:
					{
						/* 32 bit, one slot variable */
						/* get value */
						uint32 value= stack_popSlot( stack );
						
						/* assign value to field */
						newClass->class_inctance_variable_slots[fieldInfo->slot_index]= value;
						
						logVerbose( "\tThe value is %i, the slot number %i.\n", value, fieldInfo->slot_index );
						break;
					}
							
					case BASE_TYPE_LONG:
					case BASE_TYPE_DOUBLE:
					{
						/* 64 bit, two slots variable */
						uint32 value2= stack_popSlot( stack );
						uint32 value1= stack_popSlot( stack );
						
						newClass->class_inctance_variable_slots[fieldInfo->slot_index]= value1;
						newClass->class_inctance_variable_slots[fieldInfo->slot_index+1]= value2;
						break;
					}
							
					default:
						error( "Unknown type found." );
						break;
				}
						
				logVerbose( "\tPutting into static field %s.%s (type is %s).\n", newClass->className, fieldInfo->name, fieldInfo->descriptor );
				break;
		}
				
		/* get/set object field */
		/* TODO: Implement correct handling for protected fields! */
		case GETFIELD: /* u1, u2; get value of object field */
		{
			pc++;
			uint8 index1= *pc;
			pc++;
			uint8 index2= *pc;
			pc++;
			uint16 index= (index1 << 8) | index2;

			/* reference to the instance where we're going to get the field data from */
			reference ref= stack_popSlot( stack );
			
			/* get info about the class of the instance, and about the variable itself (including its storage position) */
			Class* fieldClass;
			variable* fieldInfo;			
			cls_resolveConstantPoolIndexToClassAndVariableInfo( sf->currentClass, index, &fieldClass, &fieldInfo, false );
			
			/* Check if the given class is initialized. If not, initialize it now. */
			handleClassInitialization( stack, fieldClass );
			
			/* make sure we have a static field here */
			if( isFlagSet(fieldInfo->access_flags, ACC_STATIC) )
				error( "IncompatibleClassChangeError: The requested field was static." );
			
			/* handle possible different field types now */
			switch( *fieldInfo->descriptor )
			{
				case BASE_TYPE_REFERENCE:
				case BASE_TYPE_ONE_ARRAY_DIMENSION: /* a.k.a. array reference */
				case BASE_TYPE_BOOLEAN:
				case BASE_TYPE_BYTE:
				case BASE_TYPE_CHAR:
				case BASE_TYPE_SHORT:
				case BASE_TYPE_FLOAT:
				case BASE_TYPE_INT:
				{
					/* 32 bit, one slot variable */
					/* get value from field */
					uint32 value= heap_getSlotFromInstance( ref, fieldClass, fieldInfo->slot_index );
					
					/* push value onto the stack */
					stack_pushSlot( stack, value );
					
					logVerbose( "\tThe value is %i, the slot number %i.\n", value, fieldInfo->slot_index );
					break;
				}
					
				case BASE_TYPE_LONG:
				case BASE_TYPE_DOUBLE:
				{
					/* 64 bit, two slots variable */
					/* get value from field */
					uint64 value= heap_getTwoSlotsFromInstance( ref, fieldClass, fieldInfo->slot_index );
					
					/* push value onto the stack */
					stack_pushLong( stack, value );
					
					logVerbose( "\tThe value is %i, the slot number %i.\n", value, fieldInfo->slot_index );
					break;
				}
					
				default:
					error( "Unknown type found." );
					break;
			}
			
			logVerbose( "\tGetting field %s (type is %s) of object with reference %i.\n", fieldInfo->name, fieldInfo->descriptor, ref );
			break;
		}
			
		case PUTFIELD: /* u1, u2; set value of object field */
		{
			pc++;
			uint8 index1= *pc;
			pc++;
			uint8 index2= *pc;
			pc++;
			uint16 index= (index1 << 8) | index2;
			
			/* get info about the class of the instance, and about the variable itself (including its storage position) */
			Class* fieldClass;
			variable* fieldInfo;			
			cls_resolveConstantPoolIndexToClassAndVariableInfo( sf->currentClass, index, &fieldClass, &fieldInfo, false );
			
			/* Check if the given class is initialized. If not, initialize it now. */
			handleClassInitialization( stack, fieldClass );
			
			/* make sure we have a static field here */
			if( isFlagSet(fieldInfo->access_flags, ACC_STATIC) )
				error( "IncompatibleClassChangeError: The requested field was static." );
			
			/* handle possible different field types now */
			switch( *fieldInfo->descriptor )
			{
				case BASE_TYPE_REFERENCE:
				case BASE_TYPE_ONE_ARRAY_DIMENSION: /* a.k.a. array reference */
				case BASE_TYPE_BOOLEAN:
				case BASE_TYPE_BYTE:
				case BASE_TYPE_CHAR:
				case BASE_TYPE_SHORT:
				case BASE_TYPE_FLOAT:
				case BASE_TYPE_INT:
				{
					/* 32 bit, one slot variable */
					uint32 value= stack_popSlot( stack );
					
					/* get reference to the instance where we're going to set the field data */
					reference ref= stack_popSlot( stack );
					
					/* set value to field */
					heap_setSlotOfInstance( ref, fieldClass, fieldInfo->slot_index, value );
					
					logVerbose( "\tThe value is %i, the slot number %i the reference is %i.\n", value, fieldInfo->slot_index, ref );
					break;
				}
					
				case BASE_TYPE_LONG:
				case BASE_TYPE_DOUBLE:
				{
					/* 64 bit, two slots variable */
					uint64 value= stack_popLong( stack );
					
					/* get reference to the instance where we're going to set the field data */
					reference ref= stack_popSlot( stack );
					
					/* set value to field */
					heap_setTwoSlotsOfInstance( ref, fieldClass, fieldInfo->slot_index, value );
					
					logVerbose( "\tThe value is %i, the slot number %i the reference is %i.\n", value, fieldInfo->slot_index, ref );
					break;
				}
					
				default:
					error( "Unknown type found." );
					break;
			}
			
			logVerbose( "\tPutting into field %s (type is %s).\n", fieldInfo->name, fieldInfo->descriptor );
			break;
		}
			
		/* method calls */
		case INVOKEVIRTUAL: /* u1, u2; call an instance method */
		{
			pc++;
			uint8 index1= *pc;
			pc++;
			uint8 index2= *pc;
			pc++;
			uint16 index= (index1 << 8) | index2;
			
			/* get the method and its class where it is defined */
			Class* newClass;
			method_info* methodInfo;
			cls_resolveConstantPoolIndexToClassAndMethodInfo( sf->currentClass, index, &newClass, &methodInfo );
			
			/* Fetch objectref from the stack, which is the first parameter for this method call on the stack. */
			uint32 objectRef= *(stack->stackPointer - methodInfo->parameterSlotCount);
			
			/* Now, start searching for the method to call in the hierarchy, starting with objectrefs class. 
				Note: newClass and virtualCallClass may differ here if the reference used for the method call is not of the same type as the object itself. */
			Class* virtualCallClass= heap_getClassOfInstance( objectRef );
			
			if( newClass != virtualCallClass )
			{
				methodInfo= cls_resolveMethod( &virtualCallClass, methodInfo->name, methodInfo->descriptor );
			}
			
			/* Make sure the requested method has been found. */
			if( methodInfo == NULL )
			{
				char* className;
				char* methodName;
				char* methodDesc;
				cls_resolveConstantPoolIndexOfMethodRefToMethodNameAndDescriptor( sf->currentClass, index, &className, &methodName, &methodDesc );
				logVerbose( "Method not found %s.%s%s\n", className, methodName, methodDesc );
				error( "Execution haltet." );
			}
			
			/* Handle native method calls separately. */
			if( isFlagSet(methodInfo->access_flags, ACC_NATIVE) )
			{
				logVerbose( "\t===> Executing native method %s.%s%s...\n", virtualCallClass->className, methodInfo->name, methodInfo->descriptor );
				native_handleNativeMethodCall( virtualCallClass, methodInfo, stack );
				break;
			}
			
			/* prepare pc, push new stack frame and invoke method by continuing execution */
			sf->pc= pc;
			sf= stack_pushFrame( stack, virtualCallClass, methodInfo );  /* overwrites sf! */
			pc= sf->methodInfo->code->code;
			
			logVerbose( "\t===> Executing method %s.%s%s...\n", virtualCallClass->className, sf->methodInfo->name, sf->methodInfo->descriptor );
			break;
		}
			
		/* TODO: Check correct handling for protected methods! */
		/* TODO: Check handling of private methods! They do not need to be resolved recursively! Does it matter if we try anyway? Could be problematic. */
		case INVOKESPECIAL: /* u1, u2; invoke method belonging to a specific class */
		{
			pc++;
			uint8 index1= *pc;
			pc++;
			uint8 index2= *pc;
			pc++;
			uint16 index= (index1 << 8) | index2;
			
			/* get the method and its class */
			Class* newClass;
			method_info* methodInfo;
			cls_resolveConstantPoolIndexToClassAndMethodInfo( sf->currentClass, index, &newClass, &methodInfo );
			
			/* Handle native method calls separately. */
			if( isFlagSet(methodInfo->access_flags, ACC_NATIVE) )
			{
				logVerbose( "===> Executing native method %s.%s%s...\n", newClass->className, methodInfo->name, methodInfo->descriptor );
				native_handleNativeMethodCall( newClass, methodInfo, stack  );
				break;
			}
			
			/* prepare pc, push new stack frame and invoke method by continuing execution */
			sf->pc= pc;
			sf= stack_pushFrame( stack, newClass, methodInfo );  /* overwrites sf! */
			pc= sf->methodInfo->code->code;
			
			logVerbose( "\t===> Executing method %s.%s%s...\n", sf->currentClass->className, sf->methodInfo->name, sf->methodInfo->descriptor );
			break;
		}

		case INVOKESTATIC: /* u1, u2; invoke a static method */
		{
			pc++;
			uint8 index1= *pc;
			pc++;
			uint8 index2= *pc;
			pc++;
			uint16 index= (index1 << 8) | index2;

			/* get method and its class */
			Class* newClass;
			method_info* methodInfo;
			cls_resolveConstantPoolIndexToClassAndMethodInfo( sf->currentClass, index, &newClass, &methodInfo );

			/* Handle native method calls separately. */
			if( isFlagSet(methodInfo->access_flags, ACC_NATIVE) )
			{
				logVerbose( "===> Executing native method %s.%s%s...\n", newClass->className, methodInfo->name, methodInfo->descriptor );
				native_handleNativeMethodCall( newClass, methodInfo, stack  );
				break;
			}
			
			/* Check if the given class is initialized. If not, initialize it now. */
			handleClassInitialization( stack, newClass );

			/* prepare pc, push new stack frame and invoke method by continuing execution */
			sf->pc= pc;
			sf= stack_pushFrame( stack, newClass, methodInfo );  /* overwrites sf! */
			pc= sf->methodInfo->code->code;

			logVerbose( "===> Executing method %s.%s%s...\n", sf->currentClass->className, sf->methodInfo->name, sf->methodInfo->descriptor );
			break;
		}

		case INVOKEINTERFACE: /* u1, u2, u1, u1; invoke an interface method */
		{
			pc++;
			uint8 index1= *pc;
			pc++;
			uint8 index2= *pc;
			pc++;
			uint16 index= (index1 << 8) | index2;
			
			uint8 parameterSlotCount= *pc;
			pc++;
			pc++; /* discard following 0 */
			
			/* Get objectref, which is the first parameter for this method call on the stack, parameterSlotCount deep into the stack. */
			reference objectRef= (reference)*(stack->stackPointer - parameterSlotCount);
			Class* objectClass= heap_getClassOfInstance( objectRef );
			
			/* get the method and its implementation class */
			method_info* methodInfo; /* sf->currentClass -> instead of objectClass correct? */
			cls_resolveConstantPoolIndexToClassAndInterfaceMethodInfo( sf->currentClass, index, &objectClass, &methodInfo );
			
			/* Make sure the requested method has been found. */
			if( methodInfo == NULL )
			{
				char* className;
				char* methodName;
				char* methodDesc;
				cls_resolveConstantPoolIndexOfMethodRefToMethodNameAndDescriptor( sf->currentClass, index, &className, &methodName, &methodDesc );
				logVerbose( "Method not found %s.%s%s\n", className, methodName, methodDesc );
				error( "Execution haltet." );
			}
			
			/* Handle native method calls separately. */
			/*
			if( isFlagSet(methodInfo->access_flags, ACC_NATIVE) )
			{
				logVerbose( "\t===> Executing native method %s.%s%s...\n", cls_resolveConstantPoolIndexToClassName(newClass, newClass->this_class),
								cls_resolveConstantPoolIndexToUtf8(newClass, methodInfo->name_index), 
								cls_resolveConstantPoolIndexToUtf8(newClass, methodInfo->descriptor_index) );
				native_handleNativeMethodCall( newClass, methodInfo, stack  );
				break;
			}
			*/
			 
			/* prepare pc, push new stack frame and invoke method by continuing execution */
			sf->pc= pc;
			sf= stack_pushFrame( stack, objectClass, methodInfo );  /* overwrites sf! */
			pc= sf->methodInfo->code->code;
			
			logVerbose( "\t===> Executing method %s.%s%s...\n", sf->currentClass->className, sf->methodInfo->name, sf->methodInfo->descriptor );
			break;
		}
			
		case XXX_UNUSED_XXX: /* unused */
			unsupportedError( pc );
					break;
					
		/* object/array creation and length */
		case NEW: /* u1, u2; create an object */
		{
			pc++;
			
			uint8 index1= *pc;
			pc++;
			uint8 index2= *pc;
			pc++;
			uint16 index= (index1 << 8) | index2;
			
			/* resolve requested class */
			Class* newCls= cls_resolveConstantPoolIndexToClass( sf->currentClass, index );

			/* Check if the given class is initialized. If not, initialize it now. */
			handleClassInitialization( stack, newCls );
			
			/* allocate new instance */
			reference newRef= heap_newInstance( newCls );
			stack_pushSlot( stack, newRef );
			logVerbose( "\tCreating new instance of class %s. Reference number is %i.\n", newCls->className, newRef );
			break;
		}
				
		case NEWARRAY: /* u1, u1; allocate new array for numbers or booleans */
		{
			pc++;
			uint8 atype= *pc;
			pc++;
			
			int32 count= stack_popSlot( stack );
			
			/* make sure count is not negative */
			/* NOTE: Why da heck is this a SIGNED value at all??? => Well, it uses the Java type int here. But it limits the possible size of an array to 2^31.*/
			if( count < 0 )
				error( "NegativeArraySizeException: A negative number of array entries is not allowed." );
				
			reference arRef= NULL_REFERENCE;
			
			switch( atype )
			{
				case ARRAY_TYPE_BOOLEAN:
				case ARRAY_TYPE_BYTE:
					arRef= heap_newByteArrayInstance( count );
					break;
				case ARRAY_TYPE_CHAR:
					arRef= heap_newCharArrayInstance( count );
					break;
				case ARRAY_TYPE_SHORT:
					arRef= heap_newShortArrayInstance( count );
					break;
				case ARRAY_TYPE_INT:
					arRef= heap_newIntArrayInstance( count );
					break;
				case ARRAY_TYPE_FLOAT:
					arRef= heap_newFloatArrayInstance( count );
					break;
				case ARRAY_TYPE_LONG:
					arRef= heap_newLongArrayInstance( count );
					break;
				case ARRAY_TYPE_DOUBLE:
					arRef= heap_newDoubleArrayInstance( count );
					break;
				default:
					error( "Unknown array type!" );
					break;
			}
			
			/* finally, push the reference of the newly created array onto the stack */
			stack_pushSlot( stack, arRef );
			
			logVerbose( "\tCreating new array with type %i. Reference is %i, size is %i.\n", atype, arRef, count );
			break;
		}
			
		/* NOTE: We do not handle type information here (yet). So these are no more than references/slots to us. */
		case ANEWARRAY: /* u1, u2; allocate new array for objects */
		{
			pc++;
			uint8 index1= *pc;
			pc++;
			uint8 index2= *pc;
			pc++;
			uint16 index= (index1 << 8) | index2;
			
			Class* type= cls_resolveConstantPoolIndexToClass( sf->currentClass, index );
			
			/* Create matching array type. */
			char tmpArrayType[256];
			sprintf( tmpArrayType, "[L%s;", type->className );
			Class* arrayType= ma_getClass( tmpArrayType );
			
			int32 count= stack_popSlot( stack );
				
			/* make sure count is not negative */
			/* NOTE: Why da heck is this a SIGNED value at all??? => Well, it uses the Java type int here. But it limits the possible size of an array to 2^31.*/
			if( count < 0 )
				error( "NegativeArraySizeException: A negative number of array entries is not allowed." );
					
			reference arRef= heap_newOneSlotArrayInstance( count, arrayType );
					
			/* finally, push the reference of the newly created array onto the stack */
			stack_pushSlot( stack, arRef );
				
			logVerbose( "Creating new array of reference type. Reference is %i, size is %i.\n", arRef, count );
			break;
		}
		
		case ARRAYLENGTH: /* u1; get length of array */
		{
			pc++;
			reference arRef= stack_popSlot( stack );
			
			if( arRef == NULL_REFERENCE )
				error( "NullPointerException" );
			
			int32 length= heap_getArraySize( arRef );
			stack_pushSlot( stack, length );
			
			logVerbose( "\tThe length of the array with reference %i is %i.\n", arRef, length );
			break;
		}
			
		/* exception mechanisnm */
		case ATHROW: /* u1; throw an exception error */
		{
			pc++;
			reference objectRef= stack_popSlot( stack );
			Class* classOfObject= heap_getClassOfInstance( objectRef );
			
			/* Calculate local pc. */
			uint16 localPC;
			int isCaughtFrom= -1;
			
			/* Recursively go through the exception tables of all frames on the stack. */
			while( sf->methodInfo != NULL && isCaughtFrom == -1 )
			{
				localPC= pc - sf->methodInfo->code->code;;
				isCaughtFrom= checkSurroundedByMatchingCatchClause( localPC, sf, classOfObject );
				
				if( isCaughtFrom != -1 )
				{				
					localPC= sf->methodInfo->code->exception_table_tab[isCaughtFrom]->handler_pc;
					logVerbose( "Execption caught! Execution continues at method %s.%s%s at bytecode %i.\n", sf->currentClass->className, sf->methodInfo->name, sf->methodInfo->descriptor, localPC );
					stack_pushSlot( stack, objectRef );
					pc= sf->methodInfo->code->code + localPC;
					continue;
				}
				
				/* No handler found yet? Goto previous stack frame and continue. */
				stack_popFrame( stack );
				sf= stack->currentFrame;
				pc= sf->pc;
			}
			
			if( isCaughtFrom != -1 )
				break;
			
			/* No exception handler found, print stack trace and exit. For the stack trace we call the Java method Throwable.printStackTrace(),
				the exit happens automatically, because we're at the lowest stack frame now. */			
			stack_pushSlot( stack, objectRef );
			Class* methodClass= classOfObject;
			stack_pushFrame( stack, methodClass, cls_resolveMethod(&methodClass, "printStackTrace", "()V") );
			interpreter_interpret( stack );
			return;
			break;
		}
			
		case CHECKCAST: /* u1, u2; ensure object or array belongs to type */
		{
			pc++;
			uint8 index1= *pc;
			pc++;
			uint8 index2= *pc;
			pc++;
			uint16 index= (index1 << 8) | index2;
			
			reference objectRef= stack_popSlot( stack );
			stack_pushSlot( stack, objectRef );

			/* A null reference is fine in this case. */ 
			if( objectRef == NULL_REFERENCE )
				break;
			
			Class* poolClass= cls_resolveConstantPoolIndexToClass( sf->currentClass, index );
			boolean result;
			
			/* interface */
			if( isFlagSet(poolClass->access_flags, ACC_INTERFACE) )
			{
				Class* refClass= heap_getClassOfInstance( objectRef );
				result= cls_implementsInterface(refClass, poolClass);
			}
			else /* class */
			{
				result= heap_isObjectInstanceOf(objectRef, poolClass);
			}

			logVerbose( "\tIs instance of: %s\n", result ? "yes" : "no" );
			
			if( result == false )
				error( "ClassCastException" );
			
			break;
		}
			
		case INSTANCEOF: /* u1, u2; negate an integer */
		{
			pc++;
			uint8 index1= *pc;
			pc++;
			uint8 index2= *pc;
			pc++;
			uint16 index= (index1 << 8) | index2;
			
			reference objectRef= stack_popSlot( stack );
			
			if( objectRef == NULL_REFERENCE )
			{
				stack_pushSlot( stack, 0 );
				logVerbose( "\tIs instance of: no\n" );
				break;
			}
				
			
			Class* poolClass= cls_resolveConstantPoolIndexToClass( sf->currentClass, index );
			boolean result;
		
			/* interface */
			if( isFlagSet(poolClass->access_flags, ACC_INTERFACE) )
			{
				Class* refClass= heap_getClassOfInstance( objectRef );
				result= cls_implementsInterface(refClass, poolClass);
				stack_pushSlot( stack, result ? 1 : 0 );
				logVerbose( "\tIs instance of: %s\n", result ? "yes" : "no" );
				break;
			}

			/* class */
			result= heap_isObjectInstanceOf(objectRef, poolClass);
			stack_pushSlot( stack, result ? 1 : 0 );
			logVerbose( "\tIs instance of: %s\n", result ? "yes" : "no" );
			break;
		}
			
		/* monitors */
		case MONITORENTER: /* u1; enter synchronized region of code */
		case MONITOREXIT: /* u1; leave synchronized region of code */
			/* Both, MONITORENTER and MONITOREXIT do not need to be supported, as long as we only have a single thread running.
			   To support multithreading, which the implementation has already been prepared for, these opcodes have to be supported. For now, simply ignore those
				opcodes and don't generate an error either.*/
			logWarning( "Warning: Opcode %s encountered and ignored!", opcodeNames[*pc] );
			pc++;
			stack_popSlot( stack );
			break;
			
		case WIDE: /* u1; next instruction uses 16bit index */
			/* No wide support yet. Many Opcodes will have to be extended for this. */
			unsupportedError( pc );
			break;
				
		/* array creation again */
		/* NOTE: We do not handle type information here (yet). */
		case MULTIANEWARRAY: /* u1, u2, u1; allocate multi-dimensional array */
		{
			pc++;
			uint8 index1= *pc;
			pc++;
			uint8 index2= *pc;
			pc++;
			uint16 index= (index1 << 8) | index2;
			
			uint8 dimensions= *pc;
			pc++;
			
			if( dimensions == 0 )
				error( "MULTIANEWARRAY can not create arrays with a dimension of 0!" );
			
			/* get all the count values from the stack and into a local array */
			int32* countValues= mm_staticMalloc( dimensions * sizeof(int32) );
			
			int dimCount;
			for( dimCount= dimensions-1; dimCount >= 0; dimCount-- )
				countValues[dimCount]= stack_popSlot( stack );
			
			/* create all the arrays recursively now */
			reference ref= createMultiDimensionalArray( countValues, dimensions, getTypeOfLastArrayOfMultidimensionalArray(sf->currentClass, index), cls_resolveConstantPoolIndexToClass(sf->currentClass, index) );
			
			/* done, push reference to the stack and clean up */ 
			mm_staticFree( countValues );
			stack_pushSlot( stack, ref );
			
			logVerbose( "\tCreating multidimensional array with %i dimensions and type %s.\n", dimensions, cls_resolveConstantPoolIndexToClassName(sf->currentClass, index) );
			break;
		}
			
		/* some branches again */
		case IFNULL: /* u1, s2; jump if null */
		{
			pc++;
			uint32 value= stack_popSlot( stack );
			
			/* if value (a reference) equals null (i.e. NULL_REFERENCE), branch to the given opcode */
			if( value == NULL_REFERENCE )
			{
				uint8 branchByte1= *pc;
				pc++;
				uint8 branchByte2= *pc;
				pc++;
			
				int16 branchOffset= (branchByte1 << 8) | branchByte2;
				pc-= 3; /* rewind pc to the original opcode address */
				pc+= branchOffset;
				
				logVerbose( "Reference %i is null, so branch to offset %i.\n", value, branchOffset );
				break;
			}
			
			/* do not branch, just continue with next opcode */
			pc+= 2;

			logVerbose( "Reference %i is not null, so do NOT branch.\n", value );
			break;
		}
			
		case IFNONNULL: /* u1, s2; jump if non null */
		{
			pc++;
			uint32 value= stack_popSlot( stack );
			
			/* if value (a reference) does not equal null (i.e. NULL_REFERENCE), branch to the given opcode */
			if( value != NULL_REFERENCE )
			{
				uint8 branchByte1= *pc;
				pc++;
				uint8 branchByte2= *pc;
				pc++;
				
				int16 branchOffset= (branchByte1 << 8) | branchByte2;
				pc-= 3; /* rewind pc to the original opcode address */
				pc+= branchOffset;
				
				logVerbose( "Reference %i is not null, so branch to offset %i.\n", value, branchOffset );
				break;
			}
			
			/* do not branch, just continue with next opcode */
			pc+= 2;
			
			logVerbose( "Reference %i is null, so do NOT branch.\n", value );
			break;
		}
			
		case GOTO_W: /* u1, s4; branch to address using wide offset */
		{
			pc++;
			uint8 branchByte1= *pc;
			pc++;
			uint8 branchByte2= *pc;
			pc++;
			uint8 branchByte3= *pc;
			pc++;
			uint8 branchByte4= *pc;
			pc++;
			
			int32 branchOffset= (branchByte1 << 24) | (branchByte2 << 16) | (branchByte3 << 8) | branchByte4;
			pc-= 5; /* rewind pc to the original opcode address */
			pc+= branchOffset;
			break;
		}
			
		case JSR_W: /* u1, s4; jump to subroutine using wide offset */
		{
			pc++;
			uint8 branchByte1= *pc;
			pc++;
			uint8 branchByte2= *pc;
			pc++;
			uint8 branchByte3= *pc;
			pc++;
			uint8 branchByte4= *pc;
			pc++;
			
			/* push the current pc, which points to the following opcode now, onto the stack */
			/* Note: We're pushing a full address with the size of a pointer of the host system onto the stack. So make sure the slot size is greater than or equal the size
				of a native pointer. */
			stack_pushSlot( stack, (uint32)pc );
			
			int32 branchOffset= (branchByte1 << 24) | (branchByte2 << 16) | (branchByte3 << 8) | branchByte4;
			pc-= 5; /* rewind pc to the original opcode address */
			pc+= branchOffset;
			break;
		}
			
		case BREAKPOINT: /* RESEVED; must not appear in class file (fails verification) */
			/* It is not planned to have debugging support (yet). */
			unsupportedError( pc );
			break;
					
		/* quick opcodes (may only be internally used by VM) */
		case LDC_QUICK: 
		case UNKNOWN1: /* UNKNOWN */
		case LDC2_W_QUICK: 
		case GETFIELD_QUICK: 
		case PUTFIELD_QUICK: 
		case GETFIELD2_QUICK: 
		case PUTFIELD2_QUICK: 
		case UNKNOWN2: /* UNKNOWN */
		case PUTSTATIC_QUICK: 
		case GETSTATIC2_QUICK:
		case PUTSTATIC2_QUICK:
		case INVOKEVIRTUAL_QUICK:
		case INVOKENONVIRTUAL_QUICK:
		case INVOKESUPER_QUICK:
		case INVOKESTATIC_QUICK:
		case INVOKEINTERFACE_QUICK:
		case INVOKEVIRTUALOBJECT_QUICK:
		case UNKNOWN3: /* UNKNOWN */
		case NEW_QUICK:
		case ANEWARRAY_QUICK:
		case MULTIANEWARRAY_QUICK:
		case CHECKCAST_QUICK:
		case INSTANCEOF_QUICK:
		case INVOKEVIRTUAL_QUICK_W:
		case GETFIELD_QUICK_W:
		case PUTFIELD_QUICK_W:
				
		/* unused opcodes */
		case UNUSED1:
		case UNUSED2:
		case UNUSED3:
		case UNUSED4:
		case UNUSED5:
		case UNUSED6:
		case UNUSED7:
		case UNUSED8:
		case UNUSED9:
		case UNUSED10:
		case UNUSED11:
		case UNUSED12:
		case UNUSED13:
		case UNUSED14:
		case UNUSED15:
		case UNUSED16:
		case UNUSED17:
		case UNUSED18:
		case UNUSED19:
		case UNUSED20:
		case UNUSED21:
		case UNUSED22:
		case UNUSED23:
		case UNUSED24:
		case UNUSED25:
				
		case IMPDEP1: /* RESERVED: implementation depedant 1; must not appear in class file */
		case IMPDEP2: /* RESERVED: implementation depedant 2; must not appear in class file */
				
		default:
			unsupportedError( pc );
			break;
		}
		
		numberOfBytecodesExecuted++;
	}
	
	return;
}