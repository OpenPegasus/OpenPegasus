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

#ifndef _CmpiArray_h_
#define _CmpiArray_h_

#ifndef PEGASUS_CMPI_DATA_NEED_IMPLICIT_CONVERTERS
#define PEGASUS_CMPI_DATA_NEED_IMPLICIT_CONVERTERS
#endif

#include "cmpidt.h"

class CmpiString;
class CmpiObjectPath;
class CmpiInstance;
class CmpiDateTime;
class CmpiArray;
class CmpiData;

#include "CmpiString.h"
#include "CmpiObjectPath.h"
#include "CmpiInstance.h"
#include "CmpiDateTime.h"
#include "CmpiArray.h"
#include "CmpiData.h"
#include "CmpiBaseMI.h"
#include "Linkage.h"

// ATTN: consider implementing homogeneous arrays rather than these
// heterogeneous ones.

class PEGASUS_CMPI_PROVIDER_LINKAGE CmpiArrayIdx
{
    friend class CmpiArray;
    const CmpiArray &ar;
    CMPICount idx;
    CmpiArrayIdx(const CmpiArray &a, CMPICount i);
public:

    CmpiData getData() const;

#ifdef PEGASUS_CMPI_DATA_NEED_IMPLICIT_CONVERTERS

    CmpiArrayIdx& operator=(const CmpiData&);

    /**
        Extracting String.
    */
    operator CmpiString() const;

    /**
        Extracting const char * .
    */
    operator const char* () const;

    /**
        Extracting CmpiDataTime.
    */
    operator CmpiDateTime() const;

    /**
        Extracting CmpiInstance.
    */
    operator CmpiInstance() const;

    /**
        Extracting CmpiObjectPath.
    */
    operator CmpiObjectPath() const;

    /**
        Extracting CMPISint8.
    */
    operator CMPISint8() const;

    /**
        Extracting signed 16 bit.
    */
    operator CMPISint16() const;

    /**
        Extracting signed 32 bit.
    */
    operator CMPISint32() const;

    /**
        Extracting signed 64 bit.
    */
    operator CMPISint64() const;

    /**
        Extracting unsigned 8 bit or boolean.
    */
    operator unsigned char() const;

    /**
        Extracting unsigned 16 bit or char16.
    */
    operator unsigned short() const;

    /**
        Extracting unsigned 32 bit.
    */
    operator CMPIUint32() const;

    /**
        Extracting unsigned 64 bit.
    */
    operator CMPIUint64() const;

    /**
        Extracting float 32 bit.
    */
    operator CMPIReal32() const;

    /**
        Extracting float 64 bit.
    */
    operator CMPIReal64() const;

#endif /* PEGASUS_CMPI_DATA_NEED_IMPLICIT_CONVERTERS */

    /**
        Get value.
    */
    CMPIBoolean getBoolean() const;

    /**
        Get value.
    */
    CMPISint8 getSint8() const;

    /**
        Get value.
    */
    CMPIUint8 getUint8() const;

    /**
        Get value.
    */
    CMPISint16 getSint16() const;

    /**
        Get value.
    */
    CMPIUint16 getUint16() const;

    /**
        Get value.
    */
    CMPISint32 getSint32() const;

    /**
        Get value.
    */
    CMPIUint32 getUint32() const;

    /**
        Get value.
    */
    CMPISint64 getSint64() const;

    /**
        Get value.
    */
    CMPIUint64 getUint64() const;

    /**
        Get value.
    */
    CMPIReal32 getReal32() const;

    /**
        Get value.
    */
    CMPIReal64 getReal64() const;

    /**
        Get value.
    */
    CMPIChar16 getChar16() const;

    /**
        Get value.
    */
    CmpiString getString() const;

    /**
        Get value.
    */
    const char* getCString() const;

    /**
        Get value.
    */
    CmpiDateTime getDateTime() const;

    /**
        Get value.
    */
    CmpiInstance getInstance() const;

    /**
        Get value.
    */
    CmpiObjectPath getObjectPath() const;
};


/**
    This class wraps a CMPIData value array.
    Index operations use the [] operator.
    Data extraction uses type conversion operators.
    Extraction operations can be appended to an array indexing operation
      like this:

        CmpiString state;

    CmpiArray states;

        states=ci.getProperty("States");

    state=states[3];

    Assignment statements use array indexing operations as well:

    states[5]="offline";

    Type mismatches will be signalled by exceptions.
*/

class PEGASUS_CMPI_PROVIDER_LINKAGE CmpiArray : public CmpiObject
{
    friend class CmpiArrayIdx;
    friend class CmpiData;
    friend class CmpiEnumeration;

    /**
        Gets the encapsulated CMPIArray.
    */
    CMPIArray *getEnc() const;
    void *makeArray(CMPIBroker *mb,CMPICount max, CMPIType type);
    CmpiArray(CMPIArray *arr);
public:
    CmpiArray(CMPICount max, CMPIType type);
    CmpiArray();
    CMPICount size() const;
    CmpiArrayIdx operator[](CMPICount idx) const;
};

#endif
