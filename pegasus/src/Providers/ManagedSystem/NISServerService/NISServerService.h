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

#ifndef Pegasus_NISServerService_h
#define Pegasus_NISServerService_h

//------------------------------------------------------------------------------
// INCLUDES
//------------------------------------------------------------------------------
//Pegasus includes
#include <Pegasus/Common/Config.h>
#include <Pegasus/Common/String.h>
#include <Pegasus/Common/CIMDateTime.h>
#include <Pegasus/Common/OperationContext.h>
#include <Pegasus/Common/CIMName.h>

//used by gethostname function
#include <unistd.h>

//------------------------------------------------------------------------------
PEGASUS_USING_PEGASUS;
PEGASUS_USING_STD;

// Define MOFs
static const int MAX_KEYS = 4;
static const String CLASS_NAME("PG_NISServerService");

#define SYSTEM_CREATION_CLASS_NAME CIMName ("CIM_UnitaryComputerSystem")
#define CREATION_CLASS_NAME CIMName ("PG_NISServerService")

// File configurations
static const String NIS_FILE_NAMESVRS("/etc/rc.config.d/namesvrs");

// Keys configurations
static const String NIS_KEY_DOMAIN("NIS_DOMAIN");
static const String NIS_KEY_PLUS_SERVER("NISPLUS_SERVER");
static const String NIS_KEY_MASTER_SERVER("NIS_MASTER_SERVER");
static const String NIS_KEY_SLAVE_SERVER("NIS_SLAVE_SERVER");
static const String NIS_KEY_WAIT_SERVER("WAIT_FOR_NIS_SERVER");

// Defines
static const String NIS_CAPTION("NIS Service");
static const String NIS_DESCRIPTION("This is the PG_NISServerService object");

// Insert MOF property definitions
static const String PROPERTY_SYSTEM_CREATION_CLASS_NAME(
    "SystemCreationClassName");
static const String PROPERTY_SYSTEM_NAME("SystemName");
static const String PROPERTY_CREATION_CLASS_NAME("CreationClassName");
static const String PROPERTY_NAME("Name");
static const String PROPERTY_CAPTION("Caption");
static const String PROPERTY_DESCRIPTION("Description");
static const String PROPERTY_SERVER_WAIT_FLAG("ServerWaitFlag");
static const String PROPERTY_SERVER_TYPE("ServerType");

//------------------------------------------------------------------------------
// Class [NISServerService] Definition
//------------------------------------------------------------------------------
class NISServerService
{
public:
    NISServerService();
    ~NISServerService();

public:
    //
    // Public Functions - Interface
    //

    /** Returns TRUE, if user have permissions, otherwise FALSE
    */
    Boolean AccessOk(const OperationContext & context);

    /** Returns the local host name
    */
    Boolean getLocalHostName(String & hostName);

    /** Returns the system name
    */
    Boolean getSystemName(String & hostName);

    /** Returns the class name
    */
    Boolean getCreationClassName(String & strValue);

    /** Returns the NIS domain name
    */
    Boolean getName(String & strValue);

    /** Returns the caption
    */
    Boolean getCaption(String & strValue);

    /** Returns the description
    */
    Boolean getDescription(String & strValue);

    /** Returns one of the following values: 0-Unknown, 1-Other, 2-None,
        3-NIS Master, and 4-NIS Slave.
    */
    Boolean getServerType(Uint16 & paramValue);

    /** Returns one of the following wait flag values: 0-Unknown, 1-Other,
        2-Wait, amd 3-No Wait.
    */
    Boolean getServerWaitFlag(Uint16 & paramValue);

private:
    //
    // Private Functions
    //

    /** Returns the host name based on the server address
    */
    Boolean getHostName(String serverAddress, String & hostName);

    /** Returns NIS information
    */
    Boolean getNISInfo();

    //
    // Class Attributes
    //

    String nisName;
    Uint16 nisServerType;
    Uint16 nisServerWaitFlag;
};
#endif
