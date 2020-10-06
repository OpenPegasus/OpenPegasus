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

#ifndef _CmpiString_h_
#define _CmpiString_h_

#include "cmpidt.h"
#include "cmpift.h"
#include <string.h>

#if !defined(PEGASUS_PLATFORM_WIN64_IA64_MSVC) && \
    !defined(PEGASUS_PLATFORM_WIN64_X86_64_MSVC) && \
    !defined(PEGASUS_PLATFORM_WIN32_IX86_MSVC)
# include <strings.h>
#endif

#ifndef CmpiBoolean
# define CmpiBoolean CMPIBoolean
# define CmpiRc      CMPIrc
#endif

#include "CmpiStatus.h"
#include "Linkage.h"

#if defined(PEGASUS_PLATFORM_WIN64_IA64_MSVC) || \
    defined(PEGASUS_PLATFORM_WIN64_X86_64_MSVC) || \
    defined(PEGASUS_PLATFORM_WIN32_IX86_MSVC)
# define strcasecmp _stricmp
#endif

/**
    This class wraps a CIMOM specific string representation.
*/

class PEGASUS_CMPI_PROVIDER_LINKAGE CmpiString
{
    friend class CmpiBroker;
    friend class CmpiData;
    friend class CmpiObjectPath;
    friend class CmpiInstance;
    friend class CmpiObject;
    friend class CmpiArgs;
    friend class CmpiArrayIdx;
    void *enc;
    /**
        Constructor - Internal use only
    */
    CmpiString(CMPIString* c);

    /**
        Gets the encapsulated CMPIString.
    */
    CMPIString *getEnc() const;

public:

    /**
        Constructor - Empty string.
    */
    CmpiString();

    /**
        Constructor - char* .
    */
    CmpiString(const char *s);

    /**
        Constructor - Copy constructor.
    */
    CmpiString(const CmpiString& s);

    /**
        charPtr - get pointer to char* representation.
    */
    const char* charPtr() const;

    /**
        charPtr - Case sensitive compare.
    */
    CmpiBoolean equals(const char *str) const;
    CmpiBoolean equals(const CmpiString& str) const;

    /**
        charPtr - Case insensitive compare.
    */
    CmpiBoolean equalsIgnoreCase(const char *str) const;
    CmpiBoolean equalsIgnoreCase(const CmpiString& str) const;
};

#endif
