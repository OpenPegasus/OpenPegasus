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

#ifndef _CmpiData_h_
#define _CmpiData_h_

#ifndef PEGASUS_CMPI_DATA_NEED_IMPLICIT_CONVERTERS
#define PEGASUS_CMPI_DATA_NEED_IMPLICIT_CONVERTERS
#endif

#ifdef PEGASUS_CMPI_DATA_NEED_IMPLICIT_CONVERTERS
#define EXPLICIT
#else
#define EXPLICIT explicit
#endif

#include "cmpidt.h"
#include "cmpift.h"

class CmpiString;
class CmpiObjectPath;
class CmpiInstance;
class CmpiDateTime;
class CmpiArray;
class CmpiBooleanData;
class CmpiCharData;

#include "CmpiString.h"
#include "CmpiObjectPath.h"
#include "CmpiInstance.h"
#include "CmpiDateTime.h"
#include "CmpiArray.h"
#include "Linkage.h"

/**
    This class wraps a CMPIData value item. Data extraction uses type
    conversion operators. Extraction operations can be appended to a property
    type retrieval statement
      like this:

    CmpiObjectPath cop  = ...;
    CmpiString     name = cop.getKey ("DeviceID").getString ();

    Type mismatches will be signalled by exceptions.
*/
class PEGASUS_CMPI_PROVIDER_LINKAGE CmpiData
{
    friend class CmpiInstance;
    friend class CmpiObjectPath;
    friend class CmpiResult;
    friend class CmpiArgs;
    friend class CmpiArrayIdx;
    friend class CmpiContext;
    friend class CmpiPropertyMI;
    friend class CmpiEnumeration;
protected:

    /**
        CmpiData actually is a CMPIData struct.
    */
    CMPIData _data;

    /**
        Constructor - CMPIData as input.
    */
    EXPLICIT CmpiData(const CMPIData& data);

public:

    // ATTN: there is no way to construct a char16-valued CmpiData object.
    // REASON: error: `CmpiData::CmpiData(CMPIChar16)' and
    //'CmpiData::CmpiData(CMPIUint16)' cannot be overloaded

    // ATTN: there is no way to construct a Boolean-valued CmpiData object.
    // REASON: error: `CmpiData::CmpiData(CMPIBoolean)' and
    //`CmpiData::CmpiData(CMPIUint8)' cannot be overloaded

    // ATTN: CmpiData(0) is ambiguous.

    // ATTN: initializing constructors will cause overloading ambiguities since
    // some of the data types cannot be distinguished by type overloading
    // (boolean, char16, uint32).

    /**
        Constructor - Empty constructor.
    */
    CmpiData();

    /**
        Constructor - signed 8 bit as input.
    */
    EXPLICIT CmpiData(CMPISint8 d);

    /**
        Constructor - signed 16 bit as input.
    */
    EXPLICIT CmpiData(CMPISint16 d);

    /**
        Constructor - signed 32 bit as input.
    */
    EXPLICIT CmpiData(CMPISint32 d);

    /**
        Constructor - signed 64 bit as input.
    */
    EXPLICIT CmpiData(CMPISint64 d);

    /**
        Constructor - unsigned 8 bit as input.
    */
    EXPLICIT CmpiData(CMPIUint8 d);

    /**
        Constructor - unsigned 16 bit as input.
    */
    EXPLICIT CmpiData(CMPIUint16 d);

    /**
        Constructor - unsigned 32 bit as input.
    */
    EXPLICIT CmpiData(CMPIUint32 d);

    /**
        Constructor - unsigned 64 bit as input.
    */
    EXPLICIT CmpiData(CMPIUint64 d);

    /**
        Constructor - 32 bit float as input.
    */
    EXPLICIT CmpiData(CMPIReal32 d);

    /**
        Constructor - 64 bit float as input.
    */
    EXPLICIT CmpiData(CMPIReal64 d);

    /**
        Constructor - String as input.
    */
    EXPLICIT CmpiData(const CmpiString& d);

    /**
        Constructor - char* as input.
    */
    EXPLICIT CmpiData(const char* d);

    /**
        Constructor - ObjectPath as input.
    */
    EXPLICIT CmpiData(const CmpiObjectPath& d);

    /**
        Constructor - Instance as input.
    */
    EXPLICIT CmpiData(const CmpiInstance& d);

    /**
        Constructor - DateTime as input.
    */
    EXPLICIT CmpiData(const CmpiDateTime& d);

    /**
        Constructor - Array as input.
    */
    EXPLICIT CmpiData(const CmpiArray& d);

    /**
        Constructor - CmpiData reference as input.
     */
    CmpiData(const CmpiData& d);

    /**
        Destructor.
    */
    ~CmpiData();

    /**
        Assignment operator.
     */
    CmpiData& operator= (const CmpiData& d);

#ifdef PEGASUS_CMPI_DATA_NEED_IMPLICIT_CONVERTERS

    /**
        Extracting String.
    */
    operator CmpiString() const;

    /**
        Extracting const char *.
    */
    operator const char* () const;

    /**
        Extracting CmpiDataTime.
    */
    operator CmpiDateTime() const;

    /**
        Extracting signed 8 bit.
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

    /**
        Extracting Array.
    */
    operator CmpiArray() const;

    /**
        Extracting Instance.
    */
    operator CmpiInstance() const;

    /**
        Extracting ObjectPath.
    */
    operator CmpiObjectPath() const;

#endif /* PEGASUS_CMPI_DATA_NEED_IMPLICIT_CONVERTERS */

    /**
        Get boolean value.
    */
    CMPIBoolean getBoolean() const;

    /**
        Get signed 8 bit value.
    */
    CMPISint8 getSint8() const;

    /**
        Get unsigned 8 bit value.
    */
    CMPIUint8 getUint8() const;

    /**
        Get signed 16 bit value.
    */
    CMPISint16 getSint16() const;

    /**
        Get unsigned 16 bit value.
    */
    CMPIUint16 getUint16() const;

    /**
        Get signed 32 bit value.
    */
    CMPISint32 getSint32() const;

    /**
        Get unsigned 32 bit value.
    */
    CMPIUint32 getUint32() const;

    /**
        Get signed 64 bit value.
    */
    CMPISint64 getSint64() const;

    /**
        Get unsigned 64 bit value.
    */
    CMPIUint64 getUint64() const;

    /**
        Get float 32 bit value.
    */
    CMPIReal32 getReal32() const;

    /**
        Get float 64 bit value.
    */
    CMPIReal64 getReal64() const;

    /**
        Get character value.
    */
    CMPIChar16 getChar16() const;

    /**
        Get string value.
    */
    CmpiString getString() const;

    /**
        Get char * value.
    */
    const char* getCString() const;

    /**
        Get DateTime value.
    */
    CmpiDateTime getDateTime() const;

    /**
        Get array value.
    */
    CmpiArray getArray() const;

    /**
        Get Instance value.
    */
    CmpiInstance getInstance() const;

    /**
        Get ObjectPath value.
    */
    CmpiObjectPath getObjectPath() const;

    /**
        Set boolean value.
    */
    void setBoolean (const CmpiBoolean d);

    /**
        Set signed 8 bit value.
    */
    void setSint8 (const CMPISint8 d);

    /**
        Set unsigned 8 bit value.
    */
    void setUint8 (const CMPIUint8 d);

    /**
        Set signed 16 bit value.
    */
    void setSint16 (const CMPISint16 d);

    /**
        Set unsigned 16 bit value.
    */
    void setUint16 (const CMPIUint16 d);

    /**
        Set signed 32 bit value.
    */
    void setSint32 (const CMPISint32 d);

    /**
        Set unsigned 32 bit value.
    */
    void setUint32 (const CMPIUint32 d);

    /**
        Set signed 64 bit value.
    */
    void setSint64 (const CMPISint64 d);

    /**
        Set unsigned 64 bit value.
    */
    void setUint64 (const CMPIUint64 d);

    /**
        Set float 32 bit value.
    */
    void setReal32 (const CMPIReal32 d);

    /**
        Set float 64 bit value.
    */
    void setReal64 (const CMPIReal64 d);

    /**
        Set character value.
    */
    void setChar16 (const CMPIChar16 d);

    /**
        Set string value.
    */
    void setString (const CmpiString d);

    /**
        Set char * value.
    */
    void setCString (const char* d);

    /**
        Set DateTime value.
    */
    void setDateTime (const CmpiDateTime d);

    /**
        Set array value.
    */
    void setArray (const CmpiArray d);

    /**
        Set Instance value.
    */
    void setInstance (const CmpiInstance d);

    /**
        Set ObjectPath value.
    */
    void setObjectPath (const CmpiObjectPath d);

    /**
        test for null value
    */
    int isNullValue() const;

    /**
        test for not found value
    */
    int isNotFound() const;

    /**
        return the type
    */
    CMPIType getType() const;
};

#endif
