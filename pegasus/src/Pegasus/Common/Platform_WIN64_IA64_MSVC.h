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

#ifndef Pegasus_Platform_WIN64_IA64_MSVC_h
#define Pegasus_Platform_WIN64_IA64_MSVC_h

#define PEGASUS_OS_TYPE_WINDOWS

#define PEGASUS_EXPORT __declspec(dllexport)

#define PEGASUS_IMPORT __declspec(dllimport)

#define PEGASUS_ARCHITECTURE_IA64

#define PEGASUS_COMPILER_MSVC

#define PEGASUS_UINT64 unsigned __int64

#define PEGASUS_SINT64 __int64

#define PEGASUS_SINT64_LITERAL(X) Sint64(X)
#define PEGASUS_UINT64_LITERAL(X) Uint64(X)

#define PEGASUS_64BIT_CONVERSION_WIDTH "I64"

#define PEGASUS_HAVE_NAMESPACES

// #define PEGASUS_HAVE_FOR_SCOPE

#define PEGASUS_HAVE_TEMPLATE_SPECIALIZATION

#define PEGASUS_HAVE_IOS_BINARY

#define PEGASUS_MAXHOSTNAMELEN  256

#define PEGASUS_DISABLE_INTERNAL_INLINES

// Exclude rarely-used stuff from Windows headers
// Note: this is also an easy way to avoid the often introduced
// winsock.h/winsock2.h mixing compiler error problems.
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

// Suppress this warning: "identifier was truncated to '255' characters in the
// debug information":
#pragma warning( disable : 4786 )

// Suppress this warning: "class <XXX> needs to have dll-interface to be used
// by clients of class <YYY>"
#pragma warning ( disable : 4251 )

// Supress this warning : non dll-interface class <XXX> used as base
// for dll-interface class <YYY>
#pragma warning( disable : 4275 )

// Supresss this warning : C++ exception specification ignored except
// to indicate a function is not __declspec(nothrow)
#pragma warning( disable : 4290 )

// Suppress this warning : "'this' : used in base member initializer list"
#pragma warning ( disable : 4355 )

// Suppress this warning: 'type' : forcing value to bool 'true' or 'false'
// (performance warning)
#ifdef PEGASUS_INTERNALONLY
#pragma warning( disable : 4800 )
#endif

// VC 8 specific pragmas
#if _MSC_VER >= 1400
// Suppress this warning: '<function>' was declared deprecated
// This warning is supported beginning with VC 8.
#pragma warning ( disable : 4996 )

// Suppress this warning: behavior change: an object of POD type constructed
// with an initializer of the form () will be default-initialized
#pragma warning ( disable : 4345 )
#endif

//
// Prior to Microsoft Visual Studio 7.0, there were no stream inserters for
// __int64 and unsigned __int64. We declare them if the _MSC_VER is less than
// 1300 (subtract 600 to get the version of Visual Studio). Look in
// SystemWindows.cpp for the definitions.
//
#if (_MSC_VER < 1300) && defined(PEGASUS_INTERNALONLY) && defined(__cplusplus)
# include <iostream>
namespace std
{
    inline ostream& operator<<(ostream& os, const unsigned __int64& x)
    {
        char buffer[64];
        sprintf(buffer, "%" PEGASUS_64BIT_CONVERSION_WIDTH "u", x);
        return os << buffer;
    }

    inline ostream& operator<<(ostream& os, const __int64& x)
    {
        char buffer[64];
        sprintf(buffer, "%" PEGASUS_64BIT_CONVERSION_WIDTH "d", x);
        return os << buffer;
    }
}
#endif /* _MSC_VER < 1300 */

#if _MSC_VER > 1400
#define PEGASUS_HAVE_WINDOWS_IPV6ONLY
#endif

#define PEGASUS_HAVE_WINDOWS_THREADS

/* use OpenPegasus semaphore based read/write locking mechanism */
#define PEGASUS_USE_SEMAPHORE_RWLOCK

#define PEGASUS_INTEGERS_BOUNDARY_ALIGNED

#endif /* Pegasus_Platform_WIN64_IA64_MSVC_h */
