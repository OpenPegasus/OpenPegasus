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
// Based on DNSService.h file

#ifndef _NTP_H
#define _NTP_H

//------------------------------------------------------------------------------
// INCLUDES
//------------------------------------------------------------------------------
//Pegasus includes
#include <Pegasus/Common/Config.h>
#include <Pegasus/Common/String.h>
#include <Pegasus/Common/CIMDateTime.h>
#include <Pegasus/Common/OperationContext.h>
#include <Pegasus/Common/CIMName.h>


//------------------------------------------------------------------------------
PEGASUS_USING_PEGASUS;
PEGASUS_USING_STD;

// Role definitions
static const String NTP_ROLE_CLIENT("server");

// Defines
#define CLASS_NAME CIMName ("PG_NTPService")
#define SYSTEM_CREATION_CLASS_NAME CIMName ("CIM_UnitaryComputerSystem")
#define CREATION_CLASS_NAME CIMName ("PG_NTPService")
static const String NTP_CAPTION("NTP daemon");
static const String NTP_DESCRIPTION(
    "Describes the Network Time Protocol (NTP) daemon");

// Insert MOF property definitions
static const int MAX_KEYS = 4;
static const String PROPERTY_SYSTEM_CREATION_CLASS_NAME(
    "SystemCreationClassName");
static const String PROPERTY_SYSTEM_NAME("SystemName");
static const String PROPERTY_CREATION_CLASS_NAME("CreationClassName");
static const String PROPERTY_NAME("Name");
static const String PROPERTY_CAPTION("Caption");
static const String PROPERTY_DESCRIPTION("Description");
static const String PROPERTY_SERVER_ADDRESS("ServerAddress");

//------------------------------------------------------------------------------
// Class [NTPService] Definition
//------------------------------------------------------------------------------
class NTPService
{
public:
    //Constructor/Destructor
    NTPService();
    ~NTPService();

public:
    //
    // Public Functions - Interface
    //

    // This function retrieve TRUE, if user have permissions, otherwise FALSE
    Boolean AccessOk(const OperationContext & context);

    // This function retrieves the system name
    Boolean getSystemName(String & hostName);

    // This function retrieves the local host name
    Boolean getLocalHostName(String & hostName);

    // This function returns TRUE if the Service Name is valid
    // returns the Name property on the 'strValue' argument
    Boolean getNTPName(String & strValue);

    // This function returns TRUE if the Caption is valid
    // returns the Caption property on the 'strValue' argument
    Boolean getCaption(String & strValue);

    // This function returns TRUE if the Description is valid
    // returns the Description property on the 'strValue' argument
    Boolean getDescription(String & strValue);

    // This function returns TRUE if the ServerAddress is valid
    // returns the ServerAddress property on the 'strValue' argument
    Boolean getServerAddress(Array<String> & strValue);

private:

    //
    // Private Functions
    //

    // This function retrieves the NTP information from "/etc/ntp.conf" file
    Boolean getNTPInfo();

    // This function resolves host name servers
    Boolean getHostName(String serverAddress, String & nameServer);

    // This function verify if host is address
    Boolean isHostAddress(String host);

    // This function resolves address servers
    Boolean getHostAddress(String nameServer, String & serverAddress);

    // This function retrieves the key value from line buffer
    Boolean getKeyValue(String strLine, String & strValue);

    //
    // Class Variables
    //
    String ntpName;
    Array<String> ntpServerAddress;
};
#endif
