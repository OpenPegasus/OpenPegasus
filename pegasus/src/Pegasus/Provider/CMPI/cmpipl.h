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
//%////////////////////////////////////////////////////////////////////////////

#ifndef _CMPIPL_H_
#define _CMPIPL_H_


#ifdef PEGASUS_PLATFORM_LINUX_GENERIC_GNU
# define CMPI_PLATFORM_LINUX_GENERIC_GNU 1
#endif

#ifdef PEGASUS_PLATFORM_HPUX_ACC
# define CMPI_PLATFORM_HPUX_ACC 1
#endif

#if defined(PEGASUS_PLATFORM_WIN64_IA64_MSVC) || \
    defined(PEGASUS_PLATFORM_WIN64_X86_64_MSVC) || \
    defined(PEGASUS_PLATFORM_WIN32_IX86_MSVC)
# define CMPI_PLATFORM_WIN32_IX86_MSVC 1
#endif

#ifdef PEGASUS_PLATFORM_SOLARIS_SPARC_GNU
# define CMPI_PLATFORM_SOLARIS_SPARC_GNU 1
#endif

#ifdef PEGASUS_PLATFORM_SOLARIS_SPARC_CC
# define CMPI_PLATFORM_SOLARIS_SPARC_CC 1
#endif

#ifdef PEGASUS_PLATFORM_AIX_RS_IBMCXX
# define CMPI_PLATFORM_AIX_RS_IBMCXX 1
#endif

#   ifdef PEGASUS_PLATFORM_PASE_ISERIES_IBMCXX
#      define CMPI_PLATFORM_PASE_ISERIES_IBMCXX 1
#   endif

#if defined(PEGASUS_PLATFORM_ZOS_ZSERIES_IBM) || \
    defined(PEGASUS_PLATFORM_ZOS_ZSERIES64_IBM)
# define CMPI_PLATFORM_ZOS_ZSERIES_IBM 1
#endif

#ifdef PEGASUS_PLATFORM_TRU64_ALPHA_DECCXX
# define CMPI_PLATFORM_TRU64_ALPHA_DECCXX 1
#endif

#ifdef PEGASUS_PLATFORM_DARWIN_PPC_GNU
# define CMPI_PLATFORM_DARWIN_PPC_GNU 1
#endif

#ifdef PEGASUS_PLATFORM_DARWIN_IX86_GNU
# define CMPI_PLATFORM_DARWIN_IX86_GNU 1
#endif

#ifdef PEGASUS_PLATFORM_VMS_ALPHA_DECCXX
# define CMPI_PLATFORM_VMS_ALPHA_DECCXX 1
#endif

#ifdef PEGASUS_PLATFORM_VMS_IA64_DECCXX
# define CMPI_PLATFORM_VMS_IA64_DECCXX 1
#endif

#endif
