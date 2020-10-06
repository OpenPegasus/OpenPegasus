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

#ifndef _CmpiStatus_h_
#define _CmpiStatus_h_

#include "cmpidt.h"
#include "cmpift.h"

#include "CmpiBaseMI.h"
#include "Linkage.h"

/**
    This class represents the status of a provider function invocation.
*/

class PEGASUS_CMPI_PROVIDER_LINKAGE CmpiStatus
{
    friend class CmpiInstanceMI;
    friend class CmpiMethodMI;
    friend class CmpiBaseMI;
    friend class CmpiAssociationMI;
    friend class CmpiPropertyMI;
    friend class CmpiIndicationMI;
protected:

    /**
        CmpiStatus actually is a CMPIStatus struct.
    */
    CMPIStatus st;

    /**
        status - Returns CMPIStatus struct, to be used by MI drivers only.
    */
    CMPIStatus status() const;

private:

    /**
        Constructor - not to be used.
    */
    CmpiStatus();

public:

    /**
        Aux Constructor - set from CMPIStatus.
        @param stat the CMPIStatus
    */
    CmpiStatus(const CMPIStatus stat);

    /**
        Constructor - set rc only.
        @param rc the return code.
    */
    CmpiStatus(const CMPIrc rc);

    /**
        Constructor - set rc and message.
        @param rc The return code.
        @param msg Descriptive message.
    */
    CmpiStatus(const CMPIrc rcp, const char *msg);

    /**
        rc - get the rc value.
    */
    CMPIrc rc() const;

    /**
        msg - get the msg component.
    */
    const char*  msg() const;
};

#endif

