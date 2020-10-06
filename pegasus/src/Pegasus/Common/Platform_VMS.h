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
//
//%/////////////////////////////////////////////////////////////////////////////

/*****************************************************************************
  The platform configuration files in pegasus/src/Pegasus/Common/Platform_*.h
  are intended to normalize platform/compiler differences so that Pegasus code
  can be written to work on all platforms without requiring platform ifdefs.
  Since these files are part of the Pegasus SDK, their contents should be
  the minimum that achieves the stated purpose.

  Examples of differences that are normalized here are support for C++
  namespaces, 64-bit integer type, support for C++ template specialization.

  Examples of things that are not appropriate in these files are the enablement
  of Pegasus features for a specific platform and other build configuration.
******************************************************************************/

#ifndef Pegasus_Platform_VMS_h
#define Pegasus_Platform_VMS_h

//#pragma message disable codcauunr
//#pragma message disable unscomzer
//#pragma message disable unrintunr
#if defined(PEGASUS_PLATFORM_VMS_ALPHA_DECCXX)
#pragma message disable labelnotreach
#endif
//#pragma message disable longextern
//#pragma message disable missingreturn

#define BadCreateHandler BADCREATEHANDLER
#define PegasusCreateProvider PEGASUSCREATEPROVIDER
#define PegasusCreateHandler PEGASUSCREATEHANDLER
#define PegasusCreateProviderAdapter PEGASUSCREATEPROVIDERADAPTER
#define PegasusCreateProviderManager PEGASUSCREATEPROVIDERMANAGER
#define getProviderManagerInterfaceNames GETPROVIDERMANAGERINTERFACENAME
#define getProviderManagerInterfaceVersions GETPROVIDERMANAGERINTERFACEVERS
#define callme CALLME

// BSD 4.3 is the default. _SOCKADDR_LEN changes to BSD 4.4 for IPV6 support.
#ifndef _SOCKADDR_LEN
#define _SOCKADDR_LEN 1
#endif

/* use POSIX read-write locks on this platform */
#define PEGASUS_USE_POSIX_RWLOCK

#define PEGASUS_INTEGERS_BOUNDARY_ALIGNED

#endif  /* Pegasus_Platform_VMS_h */
