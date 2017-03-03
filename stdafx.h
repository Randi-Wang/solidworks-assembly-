// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

//#pragma once
//
//#include "targetver.h"
//
//#include <stdio.h>
//#include <tchar.h>
//
//
//
//// TODO: reference additional headers your program requires here
//#import "C://Program Files//SOLIDWORKS Corp//SOLIDWORKS//sldworks.tlb" raw_interfaces_only, raw_native_types, no_namespace, named_guids  // SOLIDWORKS type library
//
//#import "C://Program Files//SOLIDWORKS Corp//SOLIDWORKS//swconst.tlb" raw_interfaces_only, raw_native_types, no_namespace, named_guids   // SOLIDWORKS constants type library


#pragma once

#define WIN32_LEAN_AND_MEAN // Exclude rarely-used stuff from Windows headers
#include <stdio.h>
#include <tchar.h>
#define _ATL_CSTRING_EXPLICIT_CONSTRUCTORS // some CString constructors will be explicit

#include <windows.h>
#include <atlbase.h> //Microsoft's ATL helper classes
#include <iostream>

using namespace std; //Use the standard C++ libraries for text output.

#import "C://Program Files//SOLIDWORKS Corp//SOLIDWORKS//sldworks.tlb" raw_interfaces_only, raw_native_types, no_namespace, named_guids //the SOLIDWORKS type library
#import "C://Program Files//SOLIDWORKS Corp//SOLIDWORKS//swconst.tlb" raw_interfaces_only, raw_native_types, no_namespace, named_guids //the SOLIDWORKS constant type library
					 // TODO: reference additional headers your program requires here
