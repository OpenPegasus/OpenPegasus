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

#include "sr_conf.h"
#include <stdio.h>
#include <stdarg.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <stdlib.h>

#include <sys/types.h>
#include <sys/sem.h>
#include <sys/stat.h>
#include <sys/param.h>

#include <signal.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <errno.h>

#include "sr_snmp.h"
#include "comunity.h"
#include "v2clssc.h"
#include "sr_trans.h"
#include "context.h"
#include "method.h"
#include "diag.h"
#include "subagent.h"
#include "agentsoc.h"
#include "evq.h"
#include "sig.h"

#include <iostream>
#include <Pegasus/Common/Config.h>

#include "snmpDeliverTrap.h"

PEGASUS_NAMESPACE_BEGIN

// l10n
// lots of hacking to make up these messages

static const char _MSG_INITSUBAGENT_FAILED[] =
    "InitSubagent Failed to initialize";
static const char _MSG_INITSUBAGENT_FAILED_KEY[] =
    "Handler.snmpIndicationHandler.snmpDeliverTrap_emanate."
        "_MSG_INITSUBAGENT_FAILED";

static const char _MSG_INVALID_TRAPOID[] = "Invalid trapOid.";
static const char _MSG_INVALID_KEY[] =
    "Handler.snmpIndicationHandler.snmpDeliverTrap_emanate._MSG_INVALID";

static const char _MSG_DESTINATION_NOT_FOUND[] =
    "Can not find trap destination.";
static const char _MSG_DESTINATION_NOT_FOUND_KEY[] =
    "Handler.snmpIndicationHandler.snmpDeliverTrap_emanate."
        "_MSG_DESTINATION_NOT_FOUND";

static const char _MSG_TARGETHOSTFORMAT_NOT_SUPPORTED[] =
    "Host Name and IPV4 Address are the only supported TargetHostFormat "
        "values.";
static const char _MSG_TARGETHOSTFORMAT_NOT_SUPPORTED_KEY[] =
    "Handler.snmpIndicationHandler.snmpDeliverTrap_emanate."
        "_MSG_TARGETHOSTFORMAT_NOT_SUPPORTED";

static const char _MSG_INVALID_SECURITY_NAME[] = "Invalid SNMP SecurityName.";
static const char _MSG_INVALID_SECURITY_NAME_KEY[] =
    "Handler.snmpIndicationHandler.snmpDeliverTrap_emanate._MSG_INVALID";

static const char _MSG_INVALID_OCTET_VALUE[] =
    "Invalid octet value in trap destination.";
static const char _MSG_INVALID_OCTET_VALUE_KEY[] =
    "Handler.snmpIndicationHandler.snmpDeliverTrap_emanate."
        "_MSG_INVALID_OCTET_VALUE";

static const char _MSG_CREATE_OCTET_FAILED[] =
    "Creation of empty 4 length OctetString failed.";
static const char _MSG_CREATE_OCTET_FAILED_KEY[] =
    "Handler.snmpIndicationHandler.snmpDeliverTrap_emanate."
        "_MSG_CREATE_OCTET_FAILED";

static const char _MSG_INVALID_ENTERPRISEOID[] = "Invalid enterpriseOid.";
static const char _MSG_INVALID_ENTERPRISEOID_KEY[] =
    "Handler.snmpIndicationHandler.snmpDeliverTrap_emanate._MSG_INVALID";

static const char _MSG_INVALID_PROPERTYOID[] = "Invalid OID of CIM Property.";
static const char _MSG_INVALID_PROPERTYOID_KEY[] =
    "Handler.snmpIndicationHandler.snmpDeliverTrap_emanate."
        "_MSG_INVALID_PROPERTYOID";

static const char _MSG_INVALID_PROPERTYVALUE[] =
    "Invalid value of CIM Property.";
static const char _MSG_INVALID_PROPERTYVALUE_KEY[] =
    "Handler.snmpIndicationHandler.snmpDeliverTrap_emanate."
        "_MSG_INVALID_PROPERTY";

static const char _MSG_MAKE_VARBIND_FAILED_FOR_OCTET_PRIM_TYPE[] =
    "Failed to MakeVarBindWithValue for type OCTET_PRIM_TYPE.";
static const char _MSG_MAKE_VARBIND_FAILED_FOR_OCTET_PRIM_TYPE_KEY[] =
    "Handler.snmpIndicationHandler.snmpDeliverTrap_emanate."
        "_MSG_MAKE_FAILED_FOR_OCTET_PRIM_TYPE";

static const char _MSG_MAKE_VARBIND_FAILED_FOR_OBJECT_ID_TYPE[] =
    "Failed to MakeVarBindWithValue for type OBJECT_ID_TYPE.";
static const char _MSG_MAKE_VARBIND_FAILED_FOR_OBJECT_ID_TYPE_KEY[] =
    "Handler.snmpIndicationHandler.snmpDeliverTrap_emanate."
        "_MSG_MAKE_FAILED_FOR_OBJECT_ID_TYPE";

static const char _MSG_MAKE_VARBIND_FAILED_FOR_INTEGER_TYPE[] =
    "Failed to MakeVarBindWithValue for type INTEGER_TYPE.";
static const char _MSG_MAKE_VARBIND_FAILED_FOR_INTEGER_TYPE_KEY[] =
   "Handler.snmpIndicationHandler.snmpDeliverTrap_emanate."
        "_MSG_MAKE_FAILED_FOR_INTEGER_TYPE";

static const char _MSG_VERSION_NOT_SUPPORTED[] =
    "SNMPv1 Trap and SNMPv2C Trap are the only supported SNMPVersion values.";
static const char _MSG_VERSION_NOT_SUPPORTED_KEY[] =
    "Handler.snmpIndicationHandler.snmpDeliverTrap_emanate."
        "_MSG_VERSION_NOT_SUPPORTED";

class snmpDeliverTrap_emanate : public snmpDeliverTrap
{
public:

    snmpDeliverTrap_emanate();

    ~snmpDeliverTrap_emanate();

    void initialize();

    void terminate();

    /**
        Send snmp trap to the target.
        @param trapOid                snmp trap OID
        @param securityName           either an SNMPv1 or SNMPv2c community
                                      name or an SNMPv3 user name
        @param targetHost             address of the trap/infom destination
        @param targetHostFormat       targetHost format
        @param otherTargetHostFormat  other target format type
        @param portNumber             UDP port number to send the trap/inform
        @param snmpVersion            snmp version and format to use to send
                                      the indication
        @param engineID               snmp engine ID used to create the SNMPv3
                                      inform
        @param vbOids                 VarBind OIDs
        @param vbTypes                VarBind types
        @param vbValues               VarBind values
    */
    void deliverTrap(
        const String& trapOid,
        const String& securityName,
        const String& targetHost,
        const Uint16& targetHostFormat,
        const String& otherTargetHostFormat,
        const Uint32& portNumber,
        const Uint16& snmpVersion,
        const String& engineID,
        const Uint8& snmpSecLevel,
        const Uint8& snmpSecAuthProto,
        const Array<Uint8>& snmpSecAuthKey,
        const Uint8& snmpSecPrivProto,
        const Array<Uint8>& snmpSecPrivKey,
        const Array<String>& vbOids,
        const Array<String>& vbTypes,
        const Array<String>& vbValues);

private:
    char* _getIPAddress(const CString& hostName);
    Boolean _isValidOctet(const Uint32& octetValue);

    /**
        Values for the TargetHostFormat property of the
        PG_IndicationHandlerSNMPMapper class.
    */
    enum TargetHostFormat
    {
        _OTHER = 1,
        _HOST_NAME = 2,
        _IPV4_ADDRESS = 3,
        _IPV6_ADDRESS = 4
    };

    enum SNMPVersion
    {
        _SNMPv1_TRAP = 2,
        _SNMPv2C_TRAP = 3,
        _SNMPv2C_INFORM = 4,
        _SNMPv3_TRAP = 5,
        _SNMPv3_INFORM = 6
    };
};

PEGASUS_NAMESPACE_END
