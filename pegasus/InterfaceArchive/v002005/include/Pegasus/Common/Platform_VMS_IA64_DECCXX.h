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
// Author: Sean Keenan (sean.keenan@hp.com)
//
// Modified By:
//
//%/////////////////////////////////////////////////////////////////////////////

#ifndef Pegasus_Platform_VMS_IA64_DECCXX_h
#define Pegasus_Platform_VMS_IA64_DECCXX_h

#include <stdlib>
#include <stdio.h>
#include <stddef>
#ifdef PEGASUS_OS_VMS
#include "ConfigVms.h"
#endif

#if (__INITIAL_POINTER_SIZE==64)
#define PEGASUS_POINTER_64BIT
#endif

#define PEGASUS_COMPILER_DECCXX

#define PLATFORM_VERSION_SUPPORTED

#define PEGASUS_UINT64 unsigned long long

#define PEGASUS_SINT64 long long

#define PEGASUS_LLONG_MAX 9223372036854775807LL
#define PEGASUS_LLONG_MIN (-PEGASUS_LLONG_MAX - 1LL)

#define PEGASUS_ULLONG_MAX 18446744073709551615ULL

#define PEGASUS_HAVE_NAMESPACES

#define PEGASUS_HAVE_NO_STD

#define PEGASUS_HAVE_EXPLICIT

#define PEGASUS_HAVE_MUTABLE

#define PEGASUS_HAVE_FOR_SCOPE

#define PEGASUS_HAVE_TEMPLATE_SPECIALIZATION

#define PEGASUS_HAVE_BOOLEAN

// #define PEGASUS_EXPLICIT_INSTANTIATION

#define PEGASUS_SUPPRESS_UNREACHABLE_STATEMENTS

#define PEGASUS_SUPPORTS_DYNLIB

#define PEGASUS_MAXHOSTNAMELEN  256

#define PEGASUS_HAVE_BROKEN_GLOBAL_CONSTRUCTION

#define PEGASUS_SOCKLEN_T size_t

// PEGASUS_DISABLE_INTERNAL_INLINES
// Debug builds fail with undefined symbols. Perhaps this is
// because debug (nooptimize) builds have the compiler inline
// feature turned off.
#define PEGASUS_DISABLE_INTERNAL_INLINES

#endif /* Pegasus_Platform_VMS_IA64_DECCXX_h */
