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


#ifndef _Pegasus_Common_CIMPropertyListRep_h
#define _Pegasus_Common_CIMPropertyListRep_h

#include <Pegasus/Common/Config.h>
#include <Pegasus/Common/Array.h>
#include <Pegasus/Common/CIMName.h>

PEGASUS_NAMESPACE_BEGIN

class CIMPropertyListRep
{
public:
    CIMPropertyListRep()
        : _refCounter(1),
          isNull(true),
          isCimNameTagsUpdated(false)
    {
    }

    CIMPropertyListRep(const CIMPropertyListRep& x)
        : _refCounter(1),
        propertyNames(x.propertyNames),
        cimNameTags(x.cimNameTags),
        isNull(x.isNull),
        isCimNameTagsUpdated(x.isCimNameTagsUpdated)
    {
    }

    CIMPropertyListRep& operator=(const CIMPropertyListRep& x)
    {
        if (&x != this)
        {
            propertyNames = x.propertyNames;
            cimNameTags = x.cimNameTags;
            isNull = x.isNull;
            isCimNameTagsUpdated = x.isCimNameTagsUpdated;
        }
        return *this;
    }

    ~CIMPropertyListRep()
    {
        cimNameTags.clear();
    }

    // reference counter as member to avoid
    // virtual function resolution overhead
    AtomicInt _refCounter;

    Array<CIMName> propertyNames;
    Array<Uint32> cimNameTags;
    Boolean isNull;
    Boolean isCimNameTagsUpdated;
};

PEGASUS_NAMESPACE_END

#endif /* _Pegasus_Common_CIMPropertyListRep_h */
