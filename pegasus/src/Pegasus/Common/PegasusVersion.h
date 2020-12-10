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
/*
//
//%/////////////////////////////////////////////////////////////////////////////
*/

#ifndef Pegasus_Version_h
#define Pegasus_Version_h

#include <Pegasus/Common/Config.h>

#ifdef PEGASUS_OVERRIDE_PRODUCT_ID
# include <Pegasus/Common/ProductVersion.h>
#else
//
// The Pegasus version string should be changed immediatly after a release
// to include the word development and should then be changed immediatly
// prior to release to remove it. Leaving the Version set to the prior release
// makes it difficult to determine a development version from a released
//  version that may be in use.
//
// The sequence would look like:
//  # define PEGASUS_PRODUCT_VERSION "2.5"
//  # define PEGASUS_PRODUCT_STATUS  ""       // Blank implies Released
//
//  # define PEGASUS_PRODUCT_VERSION "2.5.1"
//  # define PEGASUS_PRODUCT_STATUS  "Development"
//
//  # define PEGASUS_PRODUCT_VERSION "2.5.1"
//  # define PEGASUS_PRODUCT_STATUS  "Pre-Release"
//
//  # define PEGASUS_PRODUCT_VERSION "2.5.1"
//  # define PEGASUS_PRODUCT_STATUS  ""       // Blank implies Released
//
//  # define PEGASUS_PRODUCT_VERSION "2.5.2"
//  # define PEGASUS_PRODUCT_STATUS  "Development"
//
//  # define PEGASUS_PRODUCT_VERSION "2.5.2"
//  # define PEGASUS_PRODUCT_STATUS  "Pre-Release"
//
//  # define PEGASUS_PRODUCT_VERSION "2.5.2"
//  # define PEGASUS_PRODUCT_STATUS  ""       // Blank implies Released
//


# define PEGASUS_PRODUCT_NAME    "CIM Server"

# define PEGASUS_PRODUCT_STATUS  ""
# define PEGASUS_PRODUCT_VERSION "2.14.1"

# define PEGASUS_CIMOM_GENERIC_NAME "Pegasus"
// If the following is non-zero length it becomes SLP description.
# define PEGASUS_CIMOM_DESCRIPTION ""
#endif

#ifdef PEGASUS_OS_HPUX
// Define the "what" string for binary files

# ifndef PLATFORM_BUILD_DATE
#  define PLATFORM_BUILD_DATE __DATE__
# endif

# ifndef PLATFORM_COMPONENT_NAME
#  define PLATFORM_COMPONENT_NAME  "Pegasus Component"
# endif

# ifndef PLATFORM_COMPONENT_VERSION
#  define PLATFORM_COMPONENT_VERSION PEGASUS_PRODUCT_VERSION
# endif

# ifndef PLATFORM_COMPONENT_STATUS
#  define PLATFORM_COMPONENT_STATUS PEGASUS_PRODUCT_STATUS
# endif

# ifndef COMPONENT_BUILD_DATE
#  define COMPONENT_BUILD_DATE __DATE__
# endif

#if defined (PEGASUS_PLATFORM_HPUX_IA64_ACC) || \
    defined (PEGASUS_PLATFORM_HPUX_IA64_GNU)
#pragma VERSIONID \
    "@(#)" PEGASUS_PRODUCT_NAME " " PEGASUS_PRODUCT_VERSION " "\
        PEGASUS_PRODUCT_STATUS "(" PLATFORM_BUILD_DATE ")" "\n@(#)"\
        PEGASUS_PRODUCT_NAME "-" PLATFORM_COMPONENT_NAME " "\
        PLATFORM_COMPONENT_VERSION " " PLATFORM_COMPONENT_STATUS "("\
        COMPONENT_BUILD_DATE ")"
#else
static const char *PLATFORM_EMBEDDED_IDENTIFICATION_STRING =
    "@(#)" PEGASUS_PRODUCT_NAME " " PEGASUS_PRODUCT_VERSION " "
        PEGASUS_PRODUCT_STATUS "(" PLATFORM_BUILD_DATE ")" "\n@(#)"
        PEGASUS_PRODUCT_NAME "-" PLATFORM_COMPONENT_NAME " "
        PLATFORM_COMPONENT_VERSION " " PLATFORM_COMPONENT_STATUS "("
        COMPONENT_BUILD_DATE ")";
#endif

#endif /* PEGASUS_OS_HPUX */

// This macro can be examined by preprocessor directives (i.e., #if) to
// make compile time branch decisions based on the Pegasus version number.
// When read from left to right, the bytes have the following meaning:
//
//     Byte 0: major version number
//     Byte 1: minor version number
//     Byte 2: revision
//     Byte 3: reserved
//
// This macro appeared for the first time in version 2.5.1 (0x02050100).
// Hence its absence indicates a version earlier than that. The following
// tests for a Pegasus version of at least 2.5.1:
//
//   #if defined(PEGASUS_VERSION_NUMBER)
//     ..
//   #endif
//
// And this tests whether the version is greater than 2.5.2:
//
//    #if defined(PEGASUS_VERSION_NUMBER) && PEGASUS_VERSION_NUMBER > 0x02050200
//      ..
//    #endif
//
// CAUTION: always check whether PEGASUS_VERSION_NUMBER is defined when
// integrating with versions prior to 2.5.1.
//
#define PEGASUS_VERSION_NUMBER 0x02140100

#endif /* Pegasus_Version_h */
