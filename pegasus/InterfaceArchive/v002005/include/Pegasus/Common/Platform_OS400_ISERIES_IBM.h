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
/* NOCHKSRC */

//
//%/////////////////////////////////////////////////////////////////////////////

#ifndef Pegasus_Platform_OS400_ISERIES_IBM_h
#define Pegasus_Platform_OS400_ISERIES_IBM_h
#ifdef PEGASUS_USE_PEGASUS_COPYRIGHT
#include "qycmcopyriCopyright.h"
#endif

//#define _XOPEN_SOURCE_EXTENDED 1
//#define _OPEN_SYS 1
#define _MSE_PROTOS
#define _OPEN_SOURCE 3

#define _MULTI_THREADED 

#include <cstddef>



#define OS400_DEFAULT_PEGASUS_HOME "/QIBM/UserData/OS400/CIM"

#define OS400_DEFAULT_MESSAGE_SOURCE "/QIBM/ProdData/OS400/CIM/msg" // l10n

#define PEGASUS_OS_TYPE_UNIX

#include <ifs.cleinc>
  
#define PEGASUS_OS_OS400

#define PEGASUS_ARCHITECTURE_ISERIES

#define PEGASUS_COMPILER_IBM

#define PEGASUS_UINT64 unsigned long long

#define PEGASUS_SINT64 signed long long  

#define PEGASUS_HAVE_NAMESPACES

#define PEGASUS_HAVE_EXPLICIT

#define PEGASUS_HAVE_MUTABLE

#define PEGASUS_HAVE_FOR_SCOPE

#define PEGASUS_HAVE_TEMPLATE_SPECIALIZATION

#define PEGASUS_HAVE_IOS_BINARY

#define PEGASUS_STATIC_CDECL __cdecl

#define PEGASUS_NO_PASSWORDFILE

#define PEGASUS_KERBEROS_AUTHENTICATION

// This enables the default pluggable provider manager.
// See ProviderManagerService.cpp
#define PEGASUS_ENABLE_DEFAULT_PROVIDER_MANAGER

#define PEGASUS_USE_RELEASE_CONFIG_OPTIONS

#define PEGASUS_USE_RELEASE_DIRS

#define PEGASUS_SUPPORT_UTF8_FILENAME

#define PEGASUS_REPOSITORY_NOT_NORMALIZED

#include <sys/param.h> // For MAXHOSTNAMELEN
#define PEGASUS_MAXHOSTNAMELEN (MAXHOSTNAMELEN + 1)

#ifndef PEGASUS_USE_EXPERIMENTAL_INTERFACES
#define PEGASUS_USE_EXPERIMENTAL_INTERFACES
#endif

#define PEGASUS_SOCKLEN_T int

#endif /* Pegasus_Platform_OS400_ISERIES_IBM_h */
