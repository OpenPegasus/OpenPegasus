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

#ifndef _COMPUTERSYSTEMPROVIDER_H
#define _COMPUTERSYSTEMPROVIDER_H

#define PROVIDER_NAME "ComputerSystemProvider"

#include "ComputerSystem.h"

#include <Pegasus/Provider/CIMInstanceProvider.h>

// Class Names

#define CLASS_CIM_COMPUTER_SYSTEM            "CIM_ComputerSystem"
#define CLASS_CIM_UNITARY_COMPUTER_SYSTEM    "CIM_UnitaryComputerSystem"
#define CLASS_EXTENDED_COMPUTER_SYSTEM       "PG_ComputerSystem"

// Property Names

// CIM_ManagedElement
#define PROPERTY_CAPTION                     "Caption"
#define PROPERTY_DESCRIPTION                 "Description"

// CIM_ManagedSystemElement
#define PROPERTY_INSTALL_DATE                "InstallDate"
// Name is overridden in CIM_ComputerSystem
// #define PROPERTY_NAME                     "Name"
#define PROPERTY_STATUS                      "Status"
#define PROPERTY_OPERATIONAL_STATUS          "OperationalStatus"
#define PROPERTY_STATUS_DESCRIPTIONS         "StatusDescriptions"

// CIM_LogicalElement
// -- none --

// CIM_System
#define PROPERTY_CREATION_CLASS_NAME         "CreationClassName"
#define PROPERTY_NAME                        "Name"
// NameFormat is overridden in CIM_ComputerSystem
// #define PROPERTY_NAME_FORMAT              "NameFormat"
#define PROPERTY_PRIMARY_OWNER_NAME          "PrimaryOwnerName"
#define PROPERTY_PRIMARY_OWNER_CONTACT       "PrimaryOwnerContact"
#define PROPERTY_ROLES                       "Roles"

// CIM_ComputerSystem
#define PROPERTY_NAME_FORMAT                 "NameFormat"
#define PROPERTY_OTHER_IDENTIFYING_INFO      "OtherIdentifyingInfo"
#define PROPERTY_IDENTIFYING_DESCRIPTIONS    "IdentifyingDescriptions"
#define PROPERTY_DEDICATED                   "Dedicated"
#define PROPERTY_RESET_CAPABILITY            "ResetCapability"
#define PROPERTY_POWER_MANAGEMENT_CAPABILITIES "PowerManagementCapabilities"
#define METHOD_SET_POWER_STATE               "SetPowerState"
#define METHOD_SET_POWER_STATE_WITH_OPTIONS  "SetPowerStateWithOptions"
#define PROPERTY_ELEMENTNAME                 "ElementName"


// CIM_UnitaryComputerSystem
#define PROPERTY_INITIAL_LOAD_INFO           "InitialLoadInfo"
#define PROPERTY_LAST_LOAD_INFO              "LastLoadInfo"
#define PROPERTY_POWER_MANAGEMENT_SUPPORTED  "PowerManagementSupported"
#define PROPERTY_POWER_STATE                 "PowerState"
#define PROPERTY_WAKE_UP_TYPE                "WakeUpType"

// PG_ComputerSystem
#define PROPERTY_IDENTIFICATION_NUMBER       "IdentificationNumber"
#define PROPERTY_PRIMARY_OWNER_PAGER         "PrimaryOwnerPager"
#define PROPERTY_SECONDARY_OWNER_CONTACT     "SecondaryOwnerContact"
#define PROPERTY_SECONDARY_OWNER_NAME        "SecondaryOwnerName"
#define PROPERTY_SECONDARY_OWNER_PAGER       "SecondaryOwnerPager"
#define PROPERTY_SERIAL_NUMBER               "SerialNumber"


PEGASUS_USING_STD;
PEGASUS_USING_PEGASUS;

class ComputerSystemProvider: public CIMInstanceProvider
{
public:
    ComputerSystemProvider();
    ~ComputerSystemProvider();

    //-- CIMInstanceProvider methods

    void getInstance(
        const OperationContext& context,
        const CIMObjectPath& ref,
        const Boolean includeQualifiers,
        const Boolean includeClassOrigin,
        const CIMPropertyList& propertyList,
        InstanceResponseHandler& handler);

    void enumerateInstances(
        const OperationContext& context,
        const CIMObjectPath& ref,
        const Boolean includeQualifiers,
        const Boolean includeClassOrigin,
        const CIMPropertyList& propertyList,
        InstanceResponseHandler& handler);

    void enumerateInstanceNames(
        const OperationContext& context,
        const CIMObjectPath& ref,
        ObjectPathResponseHandler& handler);

    void modifyInstance(
        const OperationContext& context,
        const CIMObjectPath& ref,
        const CIMInstance& instanceObject,
        const Boolean includeQualifiers,
        const CIMPropertyList& propertyList,
        ResponseHandler& handler );

    void createInstance(
        const OperationContext& context,
        const CIMObjectPath& ref,
        const CIMInstance& instanceObject,
        ObjectPathResponseHandler& handler);

    void deleteInstance(
        const OperationContext& context,
        const CIMObjectPath& ref,
        ResponseHandler& handler);

    void initialize(CIMOMHandle& handle);
    void terminate();

protected:

    CIMOMHandle _ch;
    ComputerSystem _cs;

    void _checkClass(CIMName& className);
};

#endif
