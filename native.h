/*
 *  native.h
 *  Handling of native method calls. Please note, that this is a proprietary API which does not conform
 *  to the Java Native Interface. (This API is much simpler and does not involve the loading of external
 *  native libraries.)
 *
 *  Copyright (c) 2006, 2007 Daniel Klein. All rights reserved.
 *  
 *  Licensed under GPL version 2.
 *  See here for the full license: http://www.gnu.org/licenses/gpl.html
 */

#ifndef _native_h_
#define _native_h_

void native_handleNativeMethodCall( Class* newClass, method_info* methodInfo, Stack* stack );

#endif /* _native_h_ */