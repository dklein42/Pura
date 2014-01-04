/*
 *  heap.c
 *  Implementation of the Java Heap. This includes object and array creation and access functions.
 *
 *  Copyright (c) 2006, 2007 Daniel Klein. All rights reserved.
 *  
 *  Licensed under GPL version 2.
 *  See here for the full license: http://www.gnu.org/licenses/gpl.html
 */

#include <string.h>
#include "puraGlobals.h"
#include "methodArea.h"
#include "class.h"
#include "memoryManager.h"
#include "interpreter.h"
#include "heap.h"

/* Stores the pointers to instances (object or array). References are realized as indices into this list.
   The size of the list is doubled if a new instance is created and no free entry is left.
   Note, that due to the (yet) missing garbage collector entries will never be freed up. */
Object** objectPointerList;
uint32 maxObjectPointerListEntries= INITIAL_NUMBER_OF_POSSIBLE_REFERENCES;
uint32 objectPointerListEntryCount= 1; /* never use 0, it's used as NULL_REFERENCE */

/* Double the size of the pointer list. */
void increasePointerList()
{
	logVerbose( "Starting reallocation...\n" );
	
	uint32 before= maxObjectPointerListEntries;
	maxObjectPointerListEntries*= 2;
	
	void* newPtr= mm_staticReAlloc( objectPointerList, sizeof(Object*) * maxObjectPointerListEntries );
	
	if( newPtr == NULL )
		error( "Memory Reallocation Error!" );
	
	objectPointerList= (Object**)newPtr;
	logVerbose( "Increasing object pointer list size. Size was %i and is %i now.\n", before, maxObjectPointerListEntries );
}

/* Returns an unused entry of the object pointer list.
	Note, that this method currently is NOT thread safe!
	TODO: Implement a better version, that is also able to find empty spots in the list, if the end is reached.
   But this won't be necessary until we've got a garbage collector. */
reference getFreeObjectPointerListEntry()
{
	if( objectPointerListEntryCount == maxObjectPointerListEntries )
		increasePointerList();
	
	return objectPointerListEntryCount++;
}



/* Initializes the heap. */
void heap_init()
{
	logVerbose( "Initializing heap, using an initial object pointer list size of %i.\n", INITIAL_NUMBER_OF_POSSIBLE_REFERENCES );
	
	/* allocate and initialize reference list with NULL pointers */
	objectPointerList= (Object**)mm_staticMalloc( sizeof(Object*) * INITIAL_NUMBER_OF_POSSIBLE_REFERENCES );
	
	int i;
	for( i= 0; i < maxObjectPointerListEntries; i++ )
		objectPointerList[i]= NULL;
}

/* Creates a new instance of the given class and returns the reference to it. */ 
reference heap_newInstance( Class* cls )
{
	/* if this class has superclasses, instanciate them first (via recursion) */
	reference superInstance= NULL_REFERENCE;
	if( cls->superClass != NULL )
		superInstance= heap_newInstance( cls->superClass );
	
	/* first, find a free entry in the object pointer list */
	reference newRef= getFreeObjectPointerListEntry();
	
	/* allocate object instance */
	Object* object= mm_dynamicMalloc( sizeof(Object) + cls->instance_variable_slot_count*sizeof(slot) );
	
	/* initialize instance */
	object->cls= cls;
	/* object->gcMarker= false; */
	object->superInstance= superInstance;
	
	/* initialize slots with zeros */
	slot* instanceData= (slot*)(object+1);
	int i;
	for( i= 0; i < cls->instance_variable_slot_count; i++)
		instanceData[i]= 0;
	
	/* save the pointer to this instance into the object pointer list */
	objectPointerList[newRef]= object;
	
	return newRef;
}

/* Returns the contents of the given slot (i.e. instance variable). */
uint32 heap_getSlotFromInstance( reference ref, Class* fieldClass, uint32 slotIndex )
{
	/* resolve first reference */
	Object* obj= objectPointerList[ref];
	
	/* see if this instance matches with the class that we're looking for, otherwise recursively go to superclass */
	if( obj->cls != fieldClass )
	{
		if( obj->cls->superClass == NULL )
			error( "An instance of the given class of the field could not be found in this class hierarchy." );
		
		return heap_getSlotFromInstance( obj->superInstance, fieldClass, slotIndex );
	}
			
	return ((slot*)(obj+1))[slotIndex];	
}

/* Set the content of the given slot (i.e. instance variable).  */
void heap_setSlotOfInstance( reference ref, Class* fieldClass, uint32 slotIndex, uint32 value )
{
	/* resolve first reference */
	Object* obj= objectPointerList[ref];
	
	/* see if this instance matches with the class that we're looking for, otherwise recursively go to superclass */
	if( obj->cls != fieldClass )
	{
		if( obj->cls->superClass == NULL )
			error( "An instance of the given class of the field could not be found in this class hierarchy." );
		
		heap_setSlotOfInstance( obj->superInstance, fieldClass, slotIndex, value );
		return;
	}
	
	((slot*)(obj+1))[slotIndex]= value;	
}

/* Get two slots in a row -> long or double instance variables */
uint64 heap_getTwoSlotsFromInstance( reference ref, Class* fieldClass, uint32 slotIndex )
{
	/* resolve first reference */
	Object* obj= objectPointerList[ref];
	
	/* see if this instance matches with the class that we're looking for, otherwise recursively go to superclass */
	if( obj->cls != fieldClass )
	{
		if( obj->cls->superClass == NULL )
			error( "An instance of the given class of the field could not be found in this class hierarchy." );
		
		return heap_getTwoSlotsFromInstance( obj->superInstance, fieldClass, slotIndex );
	}
	
	slot* instanceData= (slot*)(obj+1);
	uint32 value1= instanceData[slotIndex];
	uint32 value2= instanceData[slotIndex+1];
	return ((uint64)value1 << 32) | value2;
}

/* Set two slots in a row -> long or double instance variables */
void heap_setTwoSlotsOfInstance( reference ref, Class* fieldClass, uint32 slotIndex, uint64 value )
{
	/* resolve first reference */
	Object* obj= objectPointerList[ref];
	
	/* see if this instance matches with the class that we're looking for, otherwise recursively go to superclass */
	if( obj->cls != fieldClass )
	{
		if( obj->cls->superClass == NULL )
			error( "An instance of the given class of the field could not be found in this class hierarchy." );
		
		heap_setTwoSlotsOfInstance( obj->superInstance, fieldClass, slotIndex, value );
	}
	
	uint32 value1= (uint32)((value >> 32) & 0xFFFFFFFF);
	uint32 value2= (uint32)(value & 0xFFFFFFFF);

	slot* instanceData= (slot*)(obj+1);
	instanceData[slotIndex]= value1;	
	instanceData[slotIndex+1]= value2;	
}

/* Creates a new instance of an array with a maximum data type size of one slot. */ 
reference heap_newOneSlotArrayInstance( int32 count, Class* cls )
{
	/* all arrays have Object as their superclasses, so create an Object instance as superclass manually here */
	Class* objectClass= ma_getClass( "java/lang/Object" );
	reference superInstance= heap_newInstance( objectClass );
	
	/* first, find a free entry in the object pointer list */
	reference newRef= getFreeObjectPointerListEntry();
	
	/* allocate object instance */
	Object* object= mm_dynamicMalloc( sizeof(Object) + sizeof(slot) + count*sizeof(slot) );
	
	/* initialize instance */
	object->cls= cls;
	/* object->gcMarker= false; */
	object->superInstance= superInstance;
	
	/* initialize */
	slot* arrayCount= (slot*)(object+1);
	*arrayCount= count;
	
	slot* instanceData= arrayCount+1;
	int i;
	for( i= 0; i < count; i++)
		instanceData[i]= 0;
	
	/* save the pointer to this instance into the according object pointer list */
	objectPointerList[newRef]= object;
	
	return newRef;
}

/* Creates a new instance of an array with a maximum data type size of two slots. */ 
reference heap_newTwoSlotsArrayInstance( int32 count, Class* cls )
{
	/* all arrays have Object as their superclasses, so create an Object instance as superclass manually here */
	Class* objectClass= ma_getClass( "java/lang/Object" );
	reference superInstance= heap_newInstance( objectClass );
	
	/* first, find a free entry in the object pointer list */
	reference newRef= getFreeObjectPointerListEntry();
	
	/* allocate object instance */
	Object* object= mm_dynamicMalloc( sizeof(Object) + sizeof(slot) + count*sizeof(slot)*2 );
	
	/* initialize instance */
	object->cls= cls;
	/* object->gcMarker= false; */
	object->superInstance= superInstance;
	
	/* initialize */
	slot* arrayCount= (slot*)(object+1);
	*arrayCount= count;
	
	slot* instanceData= arrayCount+1;
	int i;
	for( i= 0; i < count; i++)
		instanceData[i]= 0;
	
	/* save the pointer to this instance into the according object pointer list */
	objectPointerList[newRef]= object;
	
	return newRef;
}

reference heap_newIntArrayInstance( int32 count )
{
	return heap_newOneSlotArrayInstance( count, ma_getClass("[I") );
}

reference heap_newFloatArrayInstance( int32 count )
{
	return heap_newOneSlotArrayInstance( count, ma_getClass("[F") );
}

reference heap_newLongArrayInstance( int32 count )
{
	return heap_newTwoSlotsArrayInstance( count, ma_getClass("[J") );
}

reference heap_newDoubleArrayInstance( int32 count )
{
	return heap_newTwoSlotsArrayInstance( count, ma_getClass("[D") );
}

/* Creates a new instance of a byte array. */ 
reference heap_newByteArrayInstance( int32 count )
{
	Class* cls= ma_getClass( "[B" );
	
	/* all arrays have Object as their superclasses, so create an Object instance as superclass manually here */
	Class* objectClass= ma_getClass( "java/lang/Object" );
	reference superInstance= heap_newInstance( objectClass );
	
	/* first, find a free entry in the object pointer list */
	reference newRef= getFreeObjectPointerListEntry();
	
	/* allocate object instance */
	Object* object= mm_dynamicMalloc( sizeof(Object) + sizeof(slot) + count*sizeof(int8) );
	
	/* initialize instance */
	object->cls= cls;
	/* object->gcMarker= false; */
	object->superInstance= superInstance;
	
	/* initialize */
	slot* arrayCount= (slot*)(object+1);
	*arrayCount= count;
	
	int8* instanceData= (int8*)(arrayCount+1);
	int i;
	for( i= 0; i < count; i++)
		instanceData[i]= 0;
	
	/* save the pointer to this instance into the according object pointer list */
	objectPointerList[newRef]= object;
	
	return newRef;
}

/* Creates a new instance short array. */ 
reference heap_newShortArrayInstance( int32 count )
{
	Class* cls= ma_getClass( "[S" );
	
	/* all arrays have Object as their superclasses, so create an Object instance as superclass manually here */
	Class* objectClass= ma_getClass( "java/lang/Object" );
	reference superInstance= heap_newInstance( objectClass );
	
	/* first, find a free entry in the object pointer list */
	reference newRef= getFreeObjectPointerListEntry();
	
	/* allocate object instance */
	Object* object= mm_dynamicMalloc( sizeof(Object) + sizeof(slot) + count*sizeof(uint16) );
	
	/* initialize instance */
	object->cls= cls;
	/* object->gcMarker= false; */
	object->superInstance= superInstance;
	
	/* initialize */
	slot* arrayCount= (slot*)(object+1);
	*arrayCount= count;
	
	uint16* instanceData= (uint16*)(arrayCount+1);
	int i;
	for( i= 0; i < count; i++)
		instanceData[i]= 0;
	
	/* save the pointer to this instance into the according object pointer list */
	objectPointerList[newRef]= object;
	
	return newRef;
}

/* Creates a new instance char array. */ 
reference heap_newCharArrayInstance( int32 count )
{
	Class* cls= ma_getClass( "[C" );
	
	/* all arrays have Object as their superclasses, so create an Object instance as superclass manually here */
	Class* objectClass= ma_getClass( "java/lang/Object" );
	reference superInstance= heap_newInstance( objectClass );
	
	/* first, find a free entry in the object pointer list */
	reference newRef= getFreeObjectPointerListEntry();
	
	/* allocate object instance */
	Object* object= mm_dynamicMalloc( sizeof(Object) + sizeof(slot) + count*sizeof(uint16) );
	
	/* initialize instance */
	object->cls= cls;
	/* object->gcMarker= false; */
	object->superInstance= superInstance;
	
	/* initialize */
	slot* arrayCount= (slot*)(object+1);
	*arrayCount= count;
	
	uint16* instanceData= (uint16*)(arrayCount+1);
	int i;
	for( i= 0; i < count; i++)
		instanceData[i]= 0;
	
	/* save the pointer to this instance into the according object pointer list */
	objectPointerList[newRef]= object;
	
	return newRef;
}

slot heap_getSlotFromArray( reference arRef, int32 position )
{
	Object* obj= objectPointerList[arRef];
	
	/* make sure the object behind this reference is an array */
	if( *obj->cls->className != '[') 
		error( "Object is not an array!" );
	
	slot* instanceData= ((slot*)(obj+1))+1;
	return instanceData[position];
}

/* Return two consecutive slots of an array -> long or double */
uint64 heap_getTwoSlotsFromArray( reference arRef, int32 position )
{
	Object* obj= objectPointerList[arRef];
	int32 correctedPosition= position * 2;
	
	/* make sure the object behind this reference is an array */
	if( *obj->cls->className != '[') 
		error( "Object is not an array!" );
	
	slot* instanceData= ((slot*)(obj+1))+1;
	uint32 value1= instanceData[correctedPosition];
	uint32 value2= instanceData[correctedPosition+1];
	return ((uint64)value1 << 32) | value2 ;
}

int32 heap_getIntFromArray( reference arRef, int32 position )
{
	return heap_getSlotFromArray( arRef, position );
}

float heap_getFloatFromArray( reference arRef, int32 position )
{
	return heap_getSlotFromArray( arRef, position );
}

int64 heap_getLongFromArray( reference arRef, int32 position )
{
	return heap_getTwoSlotsFromArray( arRef, position );
}

double heap_getDoubleFromArray( reference arRef, int32 position )
{
	return heap_getTwoSlotsFromArray( arRef, position );
}

int8 heap_getByteFromArray( reference arRef, int32 position )
{
	Object* obj= objectPointerList[arRef];
	
	/* make sure the object behind this reference is an array */
	if( *obj->cls->className != '[') 
		error( "Object is not an array!" );
	
	slot* count= (slot*)(obj+1);
	int8* instanceData= (int8*)(count+1);
	return instanceData[position];
}

uint16 heap_getShortFromArray( reference arRef, int32 position )
{
	Object* obj= objectPointerList[arRef];
	
	/* make sure the object behind this reference is an array */
	if( *obj->cls->className != '[') 
		error( "Object is not an array!" );
	
	slot* count= (slot*)(obj+1);
	uint16* instanceData= (uint16*)(count+1);
	return instanceData[position];
}

uint16 heap_getCharFromArray( reference arRef, int32 position )
{
	return heap_getShortFromArray( arRef, position );
}
	
void heap_setSlotInArray( reference arRef, int32 position, slot value )
{
	Object* obj= objectPointerList[arRef];
	
	/* make sure the object behind this reference is an array */
	if( *obj->cls->className != '[') 
		error( "Object is not an array!" );

	slot* instanceData= ((slot*)(obj+1))+1;
	instanceData[position]= value;
}

void heap_setByteInArray( reference arRef, int32 position, slot value )
{
	Object* obj= objectPointerList[arRef];
	
	/* make sure the object behind this reference is an array */
	if( *obj->cls->className != '[') 
		error( "Object is not an array!" );
	
	slot* count= (slot*)(obj+1);
	int8* instanceData= (int8*)(count+1);
	instanceData[position]= (int8)(value & 0xFF);
}

void heap_setShortInArray( reference arRef, int32 position, slot value )
{
	Object* obj= objectPointerList[arRef];
	
	/* make sure the object behind this reference is an array */
	if( *obj->cls->className != '[') 
		error( "Object is not an array!" );
	
	slot* count= (slot*)(obj+1);
	uint16* instanceData= (uint16*)(count+1);
	instanceData[position]= (uint16)(value & 0xFFFF);
}

/* Set two consecutive slots in an array. */
void heap_setTwoSlotsInArray( reference arRef, int32 position, uint64 value )
{
	Object* obj= objectPointerList[arRef];
	int32 correctedPosition= position * 2;
	
	/* make sure the object behind this reference is an array */
	if( *obj->cls->className != '[') 
		error( "Object is not an array!" );
	
	uint32 value1= (value >> 32) & 0xFFFFFFFF;
	uint32 value2= value & 0xFFFFFFFF;
	
	slot* instanceData= ((slot*)(obj+1))+1;
	instanceData[correctedPosition]= value1;
	instanceData[correctedPosition+1]= value2;
}

void heap_setCharInArray( reference arRef, int32 position, slot value )
{
	heap_setShortInArray( arRef, position, value );
}

void heap_setIntInArray( reference arRef, int32 position, slot value )
{
	heap_setSlotInArray( arRef, position, value );
}

void heap_setFloatInArray( reference arRef, int32 position, slot value )
{
	heap_setSlotInArray( arRef, position, value );
}

void heap_setLongInArray( reference arRef, int32 position, int64 value )
{
	heap_setTwoSlotsInArray( arRef, position, value );
}

void heap_setDoubleInArray( reference arRef, int32 position, uint64 value )
{
	heap_setTwoSlotsInArray( arRef, position, value );
}

int32 heap_getArraySize( reference ref )
{
	/* check bounds */
	if( ref >= objectPointerListEntryCount )
		error( "ArrayIndexOutOfBoundsException" );
	
	Object* obj= objectPointerList[ref];

	/* make sure the object behind this reference is an array at all*/
	if( *obj->cls->className != '[') 
		error( "Object is not an array!" );
	
	slot* instanceData= (slot*)(obj+1);
	return *instanceData;
}

/* Verifies if the given reference points to an array. */
boolean heap_isArray( reference ref )
{
	/* check bounds */
	if( ref >= objectPointerListEntryCount )
		error( "ArrayIndexOutOfBoundsException" );
	
	Object* obj= objectPointerList[ref];
	
	if( *obj->cls->className != '[') 
		return true;
	
	return false;
}

reference heap_newStringInstance( const char* string )
{
	/* create instance of java.lang.String */
	Class* stringClass= ma_getClass( "java/lang/String" );
	reference newStr= heap_newInstance( stringClass );
	
	/* Get length of the given utf8 string. */
	/* NOTE: This is only ASCII-compatible yet. Implement UTF8 parsing! */
	int len= strlen( string );
	
	/* Create accordingly sized char array. */
	reference charArray= heap_newCharArrayInstance( len );
	
	/* Copy chars over to the char array. */
	int i;
	for( i= 0; i < len; i++ )
		heap_setCharInArray( charArray, i, string[i] );
	
	/* Put the reference to the char array into the according String instance variable. */
	variable* varInfo= cls_resolveField( &stringClass, "value", "[C" );
	heap_setSlotOfInstance( newStr, stringClass, varInfo->slot_index, charArray );
	
	/* done */
	return newStr;
}

Class* heap_getClassOfInstance( reference objectRef )
{
	if( objectRef == 0 || objectRef > objectPointerListEntryCount )
		error( "Invalid reference exception." );
	
	return objectPointerList[objectRef]->cls;
}

slot heap_getAddressOfInstance( reference objectRef )
{
	if( objectRef == 0 || objectRef > objectPointerListEntryCount )
		error( "Invalid reference exception." );
	
	return (slot)objectPointerList[objectRef];
}

/* Recursively checks the given object if it or its super instances are an instance of the given class. */
boolean heap_isObjectInstanceOf( reference objectRef, Class* cls )
{
	if( objectRef == 0 || objectRef > objectPointerListEntryCount )
		error( "Invalid reference exception." );

	Object* obj= objectPointerList[objectRef];
	
	if( obj->cls == cls )
		return true;
	
	if( obj->superInstance == NULL_REFERENCE )
		return false;
	
	return heap_isObjectInstanceOf( obj->superInstance, cls );
}
