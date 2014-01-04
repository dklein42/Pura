/*
 *  heap.h
 *  Pura
 *
 *  Created by Daniel Klein on 31.01.07.
 *  Copyright 2007 __MyCompanyName__. All rights reserved.
 *
 */

#ifndef _heap_h_
#define _heap_h_

#define NULL_REFERENCE 0
#define INITIAL_NUMBER_OF_POSSIBLE_REFERENCES 1024

#define NO_ARRAY -1
#define NO_TYPE 0
#define CLASS_TYPE 255

/* TODO: Reference indicator bitfield for GC */
typedef struct sObject
{
	Class* cls;
	/* boolean gcMarker; */
	reference superInstance;
} Object;

void heap_init();

reference heap_newInstance( Class* cls );

uint32 heap_getSlotFromInstance( reference ref, Class* fieldClass, uint32 slotIndex );
uint64 heap_getTwoSlotsFromInstance( reference ref, Class* fieldClass, uint32 slotIndex );

void heap_setSlotOfInstance( reference ref, Class* fieldClass, uint32 slotIndex, uint32 value );
void heap_setTwoSlotsOfInstance( reference ref, Class* fieldClass, uint32 slotIndex, uint64 value );

reference heap_newOneSlotArrayInstance( int32 count, Class* cls );
reference heap_newByteArrayInstance( int32 count );
reference heap_newShortArrayInstance( int32 count );
reference heap_newCharArrayInstance( int32 count );
reference heap_newIntArrayInstance( int32 count );
reference heap_newFloatArrayInstance( int32 count );
reference heap_newLongArrayInstance( int32 count );
reference heap_newDoubleArrayInstance( int32 count );

slot heap_getSlotFromArray( reference arRef, int32 position );
int8 heap_getByteFromArray( reference arRef, int32 position );
uint16 heap_getShortFromArray( reference arRef, int32 position );
uint16 heap_getCharFromArray( reference arRef, int32 position );
int32 heap_getIntFromArray( reference arRef, int32 position );
float heap_getFloatFromArray( reference arRef, int32 position );
int64 heap_getLongFromArray( reference arRef, int32 position );
double heap_getDoubleFromArray( reference arRef, int32 position );

void heap_setSlotInArray( reference arRef, int32 position, slot value );
void heap_setByteInArray( reference arRef, int32 position, slot value );
void heap_setShortInArray( reference arRef, int32 position, slot value );
void heap_setCharInArray( reference arRef, int32 position, slot value );
void heap_setIntInArray( reference arRef, int32 position, slot value );
void heap_setFloatInArray( reference arRef, int32 position, slot value );
void heap_setLongInArray( reference arRef, int32 position, int64 value );

int32 heap_getArraySize( reference ref );

reference heap_newStringInstance( const char* string );

Class* heap_getClassOfInstance( reference objectRef );
slot heap_getAddressOfInstance( reference objectRef );

boolean heap_isObjectInstanceOf( reference objectRef, Class* cls );
#endif /*_heap_h_*/