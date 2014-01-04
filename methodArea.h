/*
 *  methodArea.h
 *  Class runtime storage and handling.
 *
 *  Copyright (c) 2006, 2007 Daniel Klein. All rights reserved.
 *  
 *  Licensed under GPL version 2.
 *  See here for the full license: http://www.gnu.org/licenses/gpl.html
 */

#ifndef _methodArea_h_
#define _methodArea_h_

#include "class.h"

void ma_init();
Class* ma_loadClass( const char* className );
Class* ma_containsClass( const char* className );
Class* ma_getClass( const char* className );

#endif /*_methodArea_h_*/