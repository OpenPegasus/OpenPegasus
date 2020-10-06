//%LICENSE////////////////////////////////////////////////////////////////
//
// Licensed to The Open Group (TOG) under one or more contributor license
// agreements.  Refer to the OpenPegasusNOTICE.txt file distributed with
// this work for additional information regarding copyright ownership.
// Each contributor licenses this file to you under the OpenPegasus Open
// Source License; you may not use this file except in compliance with the
// License.
//
// Permission is hereby granted, free of charge, to any person obtaining a
// copy of this software and associated documentation files (the "Software"),
// to deal in the Software without restriction, including without limitation
// the rights to use, copy, modify, merge, publish, distribute, sublicense,
// and/or sell copies of the Software, and to permit persons to whom the
// Software is furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included
// in all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
// OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
// MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
// IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
// CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
// TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
// SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
//
//////////////////////////////////////////////////////////////////////////

/* this ALWAYS GENERATED file contains the definitions for the interfaces */


/* File created by MIDL compiler version 5.01.0164 */
/* at Tue May 03 17:56:55 2005
 */
/* Compiler settings for C:\temp\WbemConsumer\WbemConsumer.idl:
    Oicf (OptLev=i2), W1, Zp8, env=Win32, ms_ext, c_ext
    error checks: allocation ref bounds_check enum stub_data 
*/
//@@MIDL_FILE_HEADING(  )


/* verify that the <rpcndr.h> version is high enough to compile this file*/
#ifndef __REQUIRED_RPCNDR_H_VERSION__
#define __REQUIRED_RPCNDR_H_VERSION__ 440
#endif

#include "rpc.h"
#include "rpcndr.h"

#ifndef __WbemConsumer_h__
#define __WbemConsumer_h__

#ifdef __cplusplus
extern "C"{
#endif 

/* Forward Declarations */ 

#ifndef __EventConsumer_FWD_DEFINED__
#define __EventConsumer_FWD_DEFINED__

#ifdef __cplusplus
typedef class EventConsumer EventConsumer;
#else
typedef struct EventConsumer EventConsumer;
#endif /* __cplusplus */

#endif /* __EventConsumer_FWD_DEFINED__ */


#ifndef __EventDispatcher_FWD_DEFINED__
#define __EventDispatcher_FWD_DEFINED__

#ifdef __cplusplus
typedef class EventDispatcher EventDispatcher;
#else
typedef struct EventDispatcher EventDispatcher;
#endif /* __cplusplus */

#endif /* __EventDispatcher_FWD_DEFINED__ */


/* header files for imported files */
#include "oaidl.h"
#include "ocidl.h"
#include "wbemprov.h"

void __RPC_FAR * __RPC_USER MIDL_user_allocate(size_t);
void __RPC_USER MIDL_user_free( void __RPC_FAR * ); 


#ifndef __WBEMCONSUMERLib_LIBRARY_DEFINED__
#define __WBEMCONSUMERLib_LIBRARY_DEFINED__

/* library WBEMCONSUMERLib */
/* [helpstring][version][uuid] */ 


EXTERN_C const IID LIBID_WBEMCONSUMERLib;

EXTERN_C const CLSID CLSID_EventConsumer;

#ifdef __cplusplus

class DECLSPEC_UUID("44AA92D6-C186-401A-82EC-4C7B0E42ABD2")
EventConsumer;
#endif

EXTERN_C const CLSID CLSID_EventDispatcher;

#ifdef __cplusplus

class DECLSPEC_UUID("A7CD1178-4FB6-43C6-8F02-048ED46DD6F5")
EventDispatcher;
#endif
#endif /* __WBEMCONSUMERLib_LIBRARY_DEFINED__ */

/* Additional Prototypes for ALL interfaces */

/* end of Additional Prototypes */

#ifdef __cplusplus
}
#endif

#endif
