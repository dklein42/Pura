/*
 *  memoryManager.c
 *  Memory management for static (allocated by native JVM code) and dynamic (Java Heap, allocated for objects) memory.
 *  Please note, that the current implementation simply wraps the usual C library functions (malloc(), etc.).
 *
 *  Copyright (c) 2006, 2007 Daniel Klein. All rights reserved.
 *  
 *  Licensed under GPL version 2.
 *  See here for the full license: http://www.gnu.org/licenses/gpl.html
 */

#include <stdlib.h>
#include <stdio.h>
#include "memoryManager.h"

/* Note: This is a quick hack to get Pura running using the version of the GCC compiler that by default
   is installed with Cygwin. (GCC version 3.3.) Unfortunately this version doesn't support the malloc_size()
   function. This hack disables the usage, but also renders the memory book keeping pretty useless.
   TODO: Find a better solution for this.*/
#ifdef __GNUC__
#if (__GNUC__ <= 3)
#define malloc_size(a) 0
#warning "malloc_size() function not supported! Memory management bookkeeping will not work."
#else
#define CAN_USE_MALLOC_SIZE 1
#endif
#endif

uint32 currentStaticMemoryUsage= 0;
uint32 maxStaticMemoryUsed= 0;
uint32 currentDynamicMemoryUsage= 0;
uint32 maxDynamicMemoryUsed= 0;
long numberOfStaticAllocations= 0;
long numberOfDynamicAllocations= 0;
long numberOfStaticFrees= 0;
long numberOfDynamicFrees= 0;
long staticPaddingLoss= 0;
long dynamicPaddingLoss= 0;

void* mm_staticMalloc( uint32 size )
{
	void* ptr= malloc( size );
	uint32 allocatedSize= malloc_size(ptr);

	numberOfStaticAllocations++;
	currentStaticMemoryUsage+= allocatedSize;
	staticPaddingLoss+= allocatedSize - size;
	
	if( maxStaticMemoryUsed < currentStaticMemoryUsage )
		maxStaticMemoryUsed= currentStaticMemoryUsage;
	
	logMemory( "Allocating static memory block with a size of %i (%i) bytes.\n", size, allocatedSize );
	return ptr;
}

void* mm_dynamicMalloc( uint32 size )
{
	void* ptr= malloc( size );
	uint32 allocatedSize= malloc_size(ptr);

	numberOfDynamicAllocations++;
	currentDynamicMemoryUsage+= allocatedSize;
	dynamicPaddingLoss+= allocatedSize - size;
	
	if( maxDynamicMemoryUsed < currentDynamicMemoryUsage )
		maxDynamicMemoryUsed= currentDynamicMemoryUsage;
	
	logMemory( "Allocating heap memory block with a size of %i (%i) bytes.\n", size, allocatedSize );
	return ptr;
}

void mm_staticFree( void* ptr )
{
	uint32 size= malloc_size(ptr);
	free( ptr );

	currentStaticMemoryUsage-= size;
	numberOfStaticFrees++;
	
	logMemory( "Freeing static memory block with size of %i bytes.\n", size );
}

void mm_dynamicFree( void* ptr )
{
	uint32 size= malloc_size(ptr);
	free( ptr );

	currentDynamicMemoryUsage-= size;
	numberOfDynamicFrees++;
	
	logMemory( "Freeing heap memory block with size of %i bytes.\n", size );
}

void* mm_staticReAlloc( void* ptr, uint32 size )
{
	uint32 oldSize= malloc_size(ptr);
	void* newPtr= realloc( ptr, size );
	
	if( newPtr == NULL )
		return NULL;
	
	uint32 newSize= malloc_size(newPtr);
	
	currentStaticMemoryUsage-= oldSize;
	currentStaticMemoryUsage+= newSize;
	staticPaddingLoss+= newSize - size;

	if( maxStaticMemoryUsed < currentStaticMemoryUsage )
		maxStaticMemoryUsed= currentStaticMemoryUsage;
	
	logMemory( "Reallocating static memory block. Size was %i and is now %i.\n", oldSize, newSize );
	
	if( newSize < oldSize )
		logWarning( "Reallocation resulted in a smaller memory block than before!" );
	
	return newPtr;
}

uint32 mm_getGetCurrentMemoryUsage()
{
	return currentStaticMemoryUsage + currentDynamicMemoryUsage;
}

uint32 mm_getGetCurrentStaticMemoryUsage()
{
	return currentStaticMemoryUsage;
}

uint32 mm_getGetCurrentDynamicMemoryUsage()
{
	return currentDynamicMemoryUsage;
}

void mm_printStatistics()
{
/* quick hack, see above */
#ifdef CAN_USE_MALLOC_SIZE
	logMemory( "\nMemory Statistics:\n" );
	logMemory( "- Internal allocations: %i (%i bytes)\n", numberOfStaticAllocations, maxStaticMemoryUsed );
	logMemory( "- Internal frees: %i\n", numberOfStaticFrees );
	logMemory( "- Heap allocations: %i (%i bytes)\n", numberOfDynamicAllocations, maxDynamicMemoryUsed );
	logMemory( "- Heap frees: %i\n", numberOfDynamicFrees );
	logMemory( "- Overall memory lost due to alignemnt: %i bytes (%i%%).\n", staticPaddingLoss+dynamicPaddingLoss, ((staticPaddingLoss+dynamicPaddingLoss)*100)/(maxStaticMemoryUsed+maxDynamicMemoryUsed) );
	logMemory( "- Overall max memory usage: %i bytes.\n\n", maxStaticMemoryUsed+maxDynamicMemoryUsed );
#endif
}
