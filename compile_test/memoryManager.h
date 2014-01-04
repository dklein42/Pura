/*
 *  memoryManager.h
 *  Pura
 *
 *  Created by Daniel Klein on 27.02.07.
 *  Copyright 2007 __MyCompanyName__. All rights reserved.
 *
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