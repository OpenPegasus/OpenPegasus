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

#ifndef Pegasus_Platform_WIN32_IX86_MSVC_h
#define Pegasus_Platform_WIN32_IX86_MSVC_h

#define PEGASUS_OS_TYPE_WINDOWS

#define PEGASUS_EXPORT __declspec(dllexport)

#define PEGASUS_IMPORT __declspec(dllimport)

#define PEGASUS_OS_WIN32

#define PEGASUS_ARCHITECTURE_IX86

#define PEGASUS_COMPILER_MSVC

#define PEGASUS_UINT64 unsigned __int64

#define PEGASUS_SINT64 __int64

#define PEGASUS_SINT64_LITERAL(X) Sint64(X)
#define PEGASUS_UINT64_LITERAL(X) Uint64(X)

#define PEGASUS_64BIT_CONVERSION_WIDTH "I64"

#define PEGASUS_HAVE_NAMESPACES

#define PEGASUS_HAVE_EXPLICIT

#define PEGASUS_HAVE_MUTABLE

// #define PEGASUS_HAVE_FOR_SCOPE

#define PEGASUS_HAVE_TEMPLATE_SPECIALIZATION

#define PEGASUS_HAVE_IOS_BINARY

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


#endif /* Pegasus_Platform_WIN32_IX86_MSVC_h */
