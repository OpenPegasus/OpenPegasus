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

#ifndef _CmpiResult_h_
#define _CmpiResult_h_

#include "cmpidt.h"
#include "cmpift.h"
#include "CmpiObject.h"
#include "CmpiStatus.h"
#include "CmpiData.h"
#include "CmpiInstance.h"
#include "CmpiObjectPath.h"
#include "Linkage.h"


/**
    This class acts as a container to hold values returned by provider
    functions.
*/

class PEGASUS_CMPI_PROVIDER_LINKAGE CmpiResult : public CmpiObject
{
    friend class CmpiInstanceMI;
    friend class CmpiAssociationMI;
    friend class CmpiMethodMI;
    friend class CmpiPropertyMI;
    friend class CmpiIndicationMI;
private:

    /**
        Constructor - Should not be called
    */
    CmpiResult();
protected:

    /**
        Protected constructor used by MIDrivers to encapsulate CMPIResult.
    */
    CmpiResult(CMPIResult* r);

    /**
        getEnc - Gets the encapsulated CMPIResult.
    */
    CMPIResult *getEnc() const;

public:

    /**
        returnData - Return a CmpiData instance.
    */
    void returnData(const CmpiData& d);

    /**
        returnData - Return a CmpiInstance object.
    */
    void returnData(const CmpiInstance& d);

    /**
        returnData - Return a CmpiObjectPath object.
    */
    void returnData(const CmpiObjectPath& d);

    /**
        returnDone - Indicate all data returned.
    */
    void returnDone();
};

#endif


