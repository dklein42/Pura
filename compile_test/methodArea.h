/*
 *  methodArea.h
 *  Pura
 *
 *  Created by Daniel Klein on 06.11.06.
 *  Copyright 2006 __MyCompanyName__. All rights reserved.
 *
 */

#ifndef _methodArea_h_
#define _methodArea_h_

#include "class.h"

void ma_init();
Class* ma_loadClass( const char* className );
Class* ma_containsClass( const char* className );
Class* ma_getClass( const char* className );

#endif _methodArea_h_