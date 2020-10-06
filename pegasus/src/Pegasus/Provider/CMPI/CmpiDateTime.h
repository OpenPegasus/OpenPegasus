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

#ifndef _CmpiDateTime_h_
#define _CmpiDateTime_h_

#include "cmpidt.h"
#include "cmpift.h"

#include "CmpiObject.h"
#include "CmpiStatus.h"
#include "CmpiBaseMI.h"
#include "Linkage.h"


/** This class represents the instance of a CIM DateTime artifact.
*/

class PEGASUS_CMPI_PROVIDER_LINKAGE CmpiDateTime : public CmpiObject
{
protected:
    void *makeDateTime(CMPIBroker *mb);
    void *makeDateTime(CMPIBroker *mb, const char* utcTime);
    void *makeDateTime(
        CMPIBroker *mb,
        const CMPIUint64 binTime,
        const CmpiBoolean interval);
private:
public:

    /**
        Constructor from CMPI type
    */
    CmpiDateTime(const CMPIDateTime* enc);

    /**
        Constructor - Return current time and date.
    */
    CmpiDateTime();

    CmpiDateTime(const CmpiDateTime& original);

    /**
        Constructor - Initialize time and date according to &lt;utcTime&gt;.
        @param utcTime Date/Time definition in UTC format
    */
    CmpiDateTime(const char* utcTime);

    /**
        Constructor - Initialize time and date according to &lt;binTime&gt;.
        @param binTime Date/Time definition in binary format in microsecods
         starting since 00:00:00 GMT, Jan 1,1970.
        @param interval defines Date/Time definition to be an interval value
    */
    CmpiDateTime(const CMPIUint64 binTime, const CmpiBoolean interval);

    /**
        isInterval - Tests whether time is an interval value.
        @return CmpiBoolean true when time is an interval value.
    */
    CmpiBoolean isInterval() const;


    /**
        getDateTime - Returns DateTime in binary format.
        @return Uint64 DateTime in binary.
    */
    CMPIUint64 getDateTime() const;

    /**
        Gets the encapsulated CMPIDateTime.
    */
    CMPIDateTime *getEnc() const;
};

#endif
