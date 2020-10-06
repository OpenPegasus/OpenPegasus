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
//%/////////////////////////////////////////////////////////////////////////

#ifndef _COMPUTERSYSTEM_H
#define _COMPUTERSYSTEM_H

#define CAPTION "Computer System"
#define DESCRIPTION "WBEM-enabled computer system"
#define NAME_FORMAT "Other"
#define STATUS "OK"

#include <Pegasus/Provider/CIMInstanceProvider.h>
#include <stdlib.h>

PEGASUS_USING_STD;
PEGASUS_USING_PEGASUS;

class ComputerSystem
{
public:
    ComputerSystem();
    ~ComputerSystem();

    Boolean getCaption(CIMProperty&);
    Boolean getDescription(CIMProperty&);
    Boolean getInstallDate(CIMProperty&);
    Boolean getCreationClassName(CIMProperty&);
    Boolean getName(CIMProperty&);
    Boolean getStatus(CIMProperty&);
    Boolean getOperationalStatus(CIMProperty&);
    Boolean getStatusDescriptions(CIMProperty&);
    Boolean getNameFormat(CIMProperty&);
    Boolean getPrimaryOwnerName(CIMProperty&);
    Boolean setPrimaryOwnerName(const String&);
    Boolean getPrimaryOwnerContact(CIMProperty&);
    Boolean setPrimaryOwnerContact(const String&);
    Boolean getRoles(CIMProperty&);
    Boolean getOtherIdentifyingInfo(CIMProperty&);
    Boolean getIdentifyingDescriptions(CIMProperty&);
    Boolean getDedicated(CIMProperty&);
    Boolean getResetCapability(CIMProperty&);
    Boolean getPowerManagementCapabilities(CIMProperty&);
    Boolean getElementName(CIMProperty&);

// CIM_UnitaryComputerSystem
    Boolean getInitialLoadInfo(CIMProperty&);
    Boolean getLastLoadInfo(CIMProperty&);
    Boolean getPowerManagementSupported(CIMProperty&);
    Boolean getPowerState(CIMProperty&);
    Boolean getWakeUpType(CIMProperty&);
    Boolean getPrimaryOwnerPager(CIMProperty&);
    Boolean setPrimaryOwnerPager(const String&);
    Boolean getSecondaryOwnerName(CIMProperty&);
    Boolean setSecondaryOwnerName(const String&);
    Boolean getSecondaryOwnerContact(CIMProperty&);
    Boolean setSecondaryOwnerContact(const String&);
    Boolean getSecondaryOwnerPager(CIMProperty&);
    Boolean setSecondaryOwnerPager(const String&);
    Boolean getSerialNumber(CIMProperty&);
    Boolean getIdentificationNumber(CIMProperty&);

    void initialize();
    String getHostName();

    /** Builds a filled-in instance. */
    CIMInstance buildInstance(const CIMName& className);
};

#endif
