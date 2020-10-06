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

#ifndef _CmpiObject_h_
#define _CmpiObject_h_

#include "cmpidt.h"
#include "cmpift.h"

#ifndef CmpiBoolean
#define CmpiBoolean CMPIBoolean
#define CmpiRc      CMPIrc
#endif

//#include "CmpiBaseMI.h"
#include "CmpiString.h"
#include "Linkage.h"

class CmpiBroker;

/**
    Abstract base class for all Cmpi classes.
*/

class PEGASUS_CMPI_PROVIDER_LINKAGE CmpiObject
{
    friend class CmpiBroker;
    friend class CmpiObjectPath;
    friend class CmpiInstance;
protected:
    /**
        Protected pointer to encapsulated CMPI instance.
    */
    void *enc;
    /**
        Constructor - Do nothing.
    */
    CmpiObject();

    /**
        Constructor - Normal base class constructor.
    */
    CmpiObject(const void* enc);

    CmpiString doToString(CMPIBroker *mb);

    CmpiBoolean doIsA(CMPIBroker *mb, const char *typeName) const;
private:
public:
    /**
        isNull - Test for valid encapsualtion pointer.
    */
    CmpiBoolean isNull() const;

    /**
        toString - Produces CIMOM specific string representation of object.
    */
    CmpiString toString();

    /**
        isA - checks this objects type.
    */
    CmpiBoolean isA(const char *typeName) const;
};

#endif

