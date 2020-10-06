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
// Author: Paulo F. Borges (pfborges@wowmail.com)
//
// Modified By:
//
//%/////////////////////////////////////////////////////////////////////////////

#ifndef _NISTESTCLIENT_H
#define _NISTESTCLIENT_H

//------------------------------------------------------------------------------
// INCLUDES
//------------------------------------------------------------------------------
//Includes Pegasus
#include <Pegasus/Common/Config.h>
#include <Pegasus/Common/String.h>
#include <Pegasus/Client/CIMClient.h>

// Used by gethostbyaddr and gethostname functions
#include <ctype.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <unistd.h>
#include <sys/param.h>

//------------------------------------------------------------------------------
PEGASUS_USING_PEGASUS;
PEGASUS_USING_STD;

// Insert here your defines
#define SYSTEM_CREATION_CLASS_NAME CIMName ("CIM_UnitaryComputerSystem")
static const CIMNamespaceName NAMESPACE("root/cimv2");
static const CIMName CLASS_NAME("PG_NISServerService");
static const String CAPTION("NIS Service");
static const String DESCRIPTION("This is the PG_NISServerService object");
static const String NAME_FORMAT("IP");
static const String FILE_NAMESVRS("/etc/rc.config.d/namesvrs");

// Keys
static const String KEY_DOMAIN("NIS_DOMAIN");
static const String KEY_MASTER_SERVER("NIS_MASTER_SERVER");
static const String KEY_SLAVE_SERVER("NIS_SLAVE_SERVER");
static const String KEY_PLUS_SERVER("NISPLUS_SERVER");
static const String KEY_WAIT_SERVER("WAIT_FOR_NIS_SERVER");


class NISTestClient
{
  public:
     NISTestClient(CIMClient &client);
     ~NISTestClient(void);

     void testEnumerateInstanceNames(CIMClient &client, Boolean verbose);
     void testEnumerateInstances(CIMClient &client, Boolean verbose);
     void testGetInstance(CIMClient &client, Boolean verbose);

     // utility methods for common test functions
     void errorExit(const String &message);
     void testLog(const String &message);

     // Insert here the public validate functions

     // Return TRUE if CreationClassName property is ok
     Boolean goodCreationClassName(String & param, Boolean verbose);

     // Return TRUE if SystemCreationClassName property is ok
     Boolean goodSystemCreationClassName(String & param, Boolean verbose);

     // Return TRUE if SystemName property is ok
     Boolean goodSystemName(String & param, Boolean verbose);

     // Return TRUE if Name property is ok
     Boolean goodName(String & param, Boolean verbose);

     // Return TRUE if Caption property is ok
     Boolean goodCaption(String & param, Boolean verbose);

     // Return TRUE if Description property is ok
     Boolean goodDescription(String & param, Boolean verbose);

     // Return TRUE if ServerType property is ok
     Boolean goodServerType(Uint16 & param, Boolean verbose);

     // Return TRUE if ServerWaitFlag property is ok
     Boolean goodServerWaitFlag(Uint16 & param, Boolean verbose);

  private:
     // Function: getHostName
     // This function resolv name server
     Boolean getHostName(String serverAddress, String & hostName);

     // Function: getLocalHostName
     // This function retrieve local host name
     Boolean getLocalHostName(String & hostName);

     // Verify string into array
     Boolean FindInArray(Array<String> src, String text);

     // Insert here the private functions

     // validate keys of the class
     void _validateKeys(CIMObjectPath &cimRef, Boolean verboseTest);

     // validate properties of the class
     void _validateProperties(CIMInstance &cimInst, Boolean verboseTest);
};

#endif
