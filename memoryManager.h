/*
 *  memoryManager.h
 *  Memory management for static (allocated by native JVM code) and dynamic (Java Heap, allocated for objects) memory.
 *  Please note, that the current implementation simply wraps the usual C library functions (malloc(), etc.).
 *
 *  Copyright (c) 2006, 2007 Daniel Klein. All rights reserved.
 *  
 *  Licensed under GPL version 2.
 *  See here for the full license: http://www.gnu.org/licenses/gpl.html
 */

#ifndef _memoryManager_h_
#define _memoryManager_h_

#include "types.h"

void* mm_staticMalloc( uint32 size );
void* mm_dynamicMalloc( uint32 size );
void mm_staticFree( void* ptr );
void mm_dynamicFree( void* ptr );
void* mm_staticReAlloc( void* ptr, uint32 size );
uint32 mm_getGetCurrentMemoryUsage();
uint32 mm_getGetCurrentStaticMemoryUsage();
uint32 mm_getGetCurrentDynamicMemoryUsage();
void mm_printStatistics();

#endif /* _memoryManager_h_ */