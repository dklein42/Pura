/*
 *  native.h
 *  Pura
 *
 *  Created by Daniel Klein on 21.02.07.
 *  Copyright 2007 __MyCompanyName__. All rights reserved.
 *
 */

#ifndef _native_h_
#define _native_h_

void native_handleNativeMethodCall( Class* newClass, method_info* methodInfo, Stack* stack );

#endif /* _native_h_ */