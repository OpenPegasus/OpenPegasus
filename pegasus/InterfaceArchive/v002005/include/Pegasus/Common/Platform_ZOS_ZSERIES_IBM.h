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
// Author: Markus Mueller (mmueller@de.ibm.com)
//
// Modified By: Carol Ann Krug Graves, Hewlett-Packard Company
//                (carolann_graves@hp.com)
//
//%/////////////////////////////////////////////////////////////////////////////

#ifndef Pegasus_Platform_ZOS_ZSERIES_IBM_h
#define Pegasus_Platform_ZOS_ZSERIES_IBM_h

// added for Native ASCII Support
#pragma runopts("FILETAG(AUTOCVT,AUTOTAG)")

//#define _XOPEN_SOURCE_EXTENDED 1
//#define _OPEN_SYS 1
#define _MSE_PROTOS
#define _OPEN_SOURCE 3
// timezone
#define _ALL_SOURCE
// longlong
//#define _LONG_LONG 1

#include <limits.h>

#include <cstddef>

#include <stdarg.h>
#include <stdio.h>

#define PEGASUS_OS_TYPE_UNIX

#define PEGASUS_OS_ZOS

#define PEGASUS_ARCHITECTURE_ZSERIES

#define PEGASUS_COMPILER_IBM

#define PEGASUS_UINT64 unsigned long long int

#define PEGASUS_SINT64 long long int

#define PEGASUS_HAVE_NAMESPACES

#define PEGASUS_HAVE_EXPLICIT

#define PEGASUS_HAVE_MUTABLE

#define PEGASUS_HAVE_FOR_SCOPE

#define PEGASUS_HAVE_TEMPLATE_SPECIALIZATION

#define PEGASUS_HAVE_INTERNAL_INLINES

// removed as Pegasus will work with Native ASCII support on zOS
// #define PEGASUS_HAVE_EBCDIC

#define PEGASUS_STATIC_CDECL __cdecl

#define PEGASUS_HAS_SIGNALS

#define PEGASUS_MAXHOSTNAMELEN  256

#define PEGASUS_NO_PASSWORDFILE


//#define snprintf(sptr,len,form,data) sprintf(sptr,form,data)

#define ZOS_SECURITY_NAME "CIMServer Security"
inline int snprintf(char *sptr, size_t len, const char* form, ...)
{
    int returnvalue;
    va_list arguments;
    va_start(arguments, form);
    returnvalue = vsprintf(sptr, form, arguments);
    va_end(arguments);
    return returnvalue;
}

#define ZOS_DEFAULT_PEGASUS_REPOSITORY "/var/wbem"

#define PEGASUS_SOCKLEN_T size_t

#endif /* Pegasus_Platform_ZOS_ZSERIES_IBM_h */
