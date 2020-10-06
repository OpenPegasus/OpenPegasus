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

#include <iostream>

#include <net-snmp/net-snmp-config.h>
#include <net-snmp/net-snmp-includes.h>
#include <net-snmp/config_api.h>
#include <Pegasus/Common/Mutex.h>
#include "snmpDeliverTrap.h"

PEGASUS_NAMESPACE_BEGIN

static const char _MSG_SESSION_OPEN_FAILED[] =
    "Snmp Indication Handler failed to open the SNMP session: ";
static const char _MSG_SESSION_OPEN_FAILED_KEY[] =
    "Handler.snmpIndicationHandler.snmpDeliverTrap_netsnmp."
        "_MSG_SESSION_OPEN_FAILED";

static const char _MSG_GET_SESSION_POINT_FAILED[] =
    "Snmp Indication Handler failed to get the SNMP session pointer: ";
static const char _MSG_GET_SESSION_POINTER_FAILED_KEY[] =
    "Handler.snmpIndicationHandler.snmpDeliverTrap_netsnmp."
        "_MSG_GET_SESSION_POINT_FAILED";

static const char _MSG_PDU_CREATE_FAILED[] =
    "Snmp Indication Handler failed to create the SNMP PDU.";
static const char _MSG_PDU_CREATE_FAILED_KEY[] =
    "Handler.snmpIndicationHandler.snmpDeliverTrap_netsnmp."
        "_MSG_PDU_CREATE_FAILED";

static const char _MSG_VERSION_NOT_SUPPORTED[] =
    "SNMPv1 Trap, SNMPv2C Trap and SNMPV3 are the only supported SNMPVersion "
         "values.";
static const char _MSG_VERSION_NOT_SUPPORTED_KEY[] =
    "Handler.snmpIndicationHandler.snmpDeliverTrap_netsnmp."
        "_MSG_VERSION_NOT_SUPPORTED";

static const char _MSG_SESSION_SEND_FAILED[] =
    "Snmp Indication Handler failed to send the trap: ";
static const char _MSG_SESSION_SEND_FAILED_KEY[] =
    "Handler.snmpIndicationHandler.snmpDeliverTrap_netsnmp."
        "_MSG_SESSION_SEND_FAILED";

static const char _MSG_PACK_TRAP_INFO_INTO_PDU_FAILED[] =
    "Snmp Indication Handler failed to pack trap information into the "
        "SNMP PDU: \"$0\".";
static const char _MSG_PACK_TRAP_INFO_INTO_PDU_FAILED_KEY[] =
    "Handler.snmpIndicationHandler.snmpDeliverTrap_netsnmp."
        "_MSG_PACK_TRAP_INFO_INTO_PDU_FAILED";

static const char _MSG_ADD_SYSUPTIME_TO_PDU_FAILED[] =
    "Snmp Indication Handler failed to add sysUpTime to the SNMP PDU: \"$0\".";
static const char _MSG_ADD_SYSUPTIME_TO_PDU_FAILED_KEY[] =
    "Handler.snmpIndicationHandler.snmpDeliverTrap_netsnmp."
        "_MSG_ADD_SYSUPTIME_TO_PDU_FAILED";

static const char _MSG_ADD_SNMP_TRAP_TO_PDU_FAILED[] =
    "Snmp Indication Handler failed to add SNMP Trap to the SNMP PDU: \"$0\".";
static const char _MSG_ADD_SNMP_TRAP_TO_PDU_FAILED_KEY[] =
    "Handler.snmpIndicationHandler.snmpDeliverTrap_netsnmp."
        "_MSG_ADD_SNMP_TRAP_TO_PDU_FAILED";

static const char _MSG_PACK_CIM_PROPERTY_TO_PDU_FAILED[] =
    "Snmp Indication Handler failed to pack a CIM Property into the "
        "SNMP PDU: \"$0\".";
static const char _MSG_PACK_CIM_PROPERTY_TO_PDU_FAILED_KEY[] =
    "Handler.snmpIndicationHandler.snmpDeliverTrap_netsnmp."
        "_MSG_PACK_CIM_PROPERTY_TO_PDU_FAILED";

static const char _MSG_READ_OBJID_FAILED[] =
    "Snmp Indication Handler failed to convert trapOid \"$0\" from a "
        "numeric form to a list of subidentifiers.";
static const char _MSG_READ_OBJID_FAILED_KEY[] =
    "Handler.snmpIndicationHandler.snmpDeliverTrap_netsnmp."
        "_MSG_READ_OBJID_FAILED";

static const char _MSG_PARSE_CIM_PROPERTY_OID_FAILED[] =
    "Snmp Indication Handler failed to convert a CIM property OID \"$0\" from "
        "a numeric form to a list of subidentifiers.";
static const char _MSG_PARSE_CIM_PROPERTY_OID_FAILED_KEY[] =
    "Handler.snmpIndicationHandler.snmpDeliverTrap_netsnmp."
        "__MSG_PARSE_CIM_PROPERTY_OID_FAILED";

static const char _MSG_READ_ENTOID_FAILED[] =
    "Snmp Indication Handler failed to convert SNMPV1 enterprise OID \"$0\" "
        "from a numeric form to a list of subidentifiers.";
static const char _MSG_READ_ENTOID_FAILED_KEY[] =
    "Handler.snmpIndicationHandler.snmpDeliverTrap_netsnmp."
        "_MSG_READ_ENTOID_FAILED";

static const char _MSG_UNSUPPORTED_SNMP_DATA_TYPE[] =
    "Type \"$0\" is an unsupported SNMP Data Type for the CIM property.";
static const char _MSG_UNSUPPORTED_SNMP_DATA_TYPE_KEY[] =
    "Handler.snmpIndicationHandler.snmpDeliverTrap_netsnmp."
        "_MSG_UNSUPPORTED_SNMP_DATA_TYPE";

static const char _MSG_ADD_VAR_TO_PDU_FAILED[] =
    "Snmp Indication Handler failed to add a CIM property \"$0\" to the "
        "SNMP PDU: \"$1\".";
static const char _MSG_ADD_VAR_TO_PDU_FAILED_KEY[] =
    "Handler.snmpIndicationHandler.snmpDeliverTrap_netsnmp."
        "_MSG_ADD_VAR_TO_PDU_FAILED";

class snmpDeliverTrap_netsnmp : public snmpDeliverTrap
{
public:

    void initialize();

    void terminate();

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

    // Mutex is needed before a session is created. Sessions created
    // using the Single API do not interact with other SNMP sessions.
    Mutex _sessionInitMutex;

    /**
        Creates a SNMP session.

        @param targetHost       the target system of a hostname or an IPv4
                                address or an IPv6 address to receive a trap
        @param targetHostFormat the format of the targetHost
        @param portNumber       the port number to receive a trap
        @param securityName     the human readable community name
        @param snmpVersion      the snmp version
        @param engineID         the engine ID
        @param snmpSecLevel     the security level
                                (authNoPriv,authPriv,noAuthNoPriv)
        @param snmpSecAuthProto MD5 or SHA
        @param snmpSecAuthKey   encoded authentication key
        @param snmpSecPrivProto AES or DES
        @param snmpSecPrivKey   encoded privacy key
        @param sessionHandle    an opaque pointer of the SNMP session
        @param sessionPtr       the SNMP session pointer to its associated
                                struct snmp_session
    */
    void _createSession(
        const String& targetHost,
        Uint16 targetHostFormat,
        Uint32 portNumber,
        const String& securityName,
        Uint16 snmpVersion,
        const String& engineID,
        const Uint8& snmpSecLevel,
        const Uint8& snmpSecAuthProto,
        const Array<Uint8>& snmpSecAuthKey,
        const Uint8& snmpSecPrivProto,
        const Array<Uint8>& snmpSecPrivKey,
        void*& sessionHandle,
        struct snmp_session*& sessionPtr);

    /**
        Add the community string to the snmp session for V1 and V2C

        @param sessionPtr    the SNMP session pointer to its associated
                             struct snmp_session
        @param securityName     the human readable community name
    */
    void _addCommunity(
        struct snmp_session*& sessionPtr,
        const String& securityName);

    /**
        Creates a SNMP session.

        @param sessionHandle  an opaque pointer of the SNMP session
    */
    void _destroySession(void* sessionHandle);

    /**
        Creates a SNMP PDU.

        @param snmpVersion  the SNMP version
        @param trapOid      the trap OID
        @param sessionPtr   the SNMP session pointer
        @param snmpPdu      the SNMP PDU
    */
    void _createPdu(
        Uint16 snmpVersion,
        const String& trapOid,
        struct snmp_session*& sessionPtr,
        struct snmp_pdu*& snmpPdu);

    /**
        Pack the trap information into the PDU.

        @param trapOid  the trap OID
        @param snmpPdu  the SNMP PDU
    */
    void _packTrapInfoIntoPdu(
        const String& trapOid,
        snmp_pdu* snmpPdu);

    /**
        Pack CIM properties into PDU.

        @param vbOids    the array of CIM property OIDs
        @param vbTypes   the array of CIM property data types
        @param vbValues  the array of CIM property values
        @param snmpPdu   the SNMP PDU
    */
    void _packOidsIntoPdu(
        const Array<String>& vbOids,
        const Array<String>& vbTypes,
        const Array<String>& vbValues,
        snmp_pdu* snmpPdu);

    enum SNMPVersion
    {
        _SNMPv1_TRAP = 2,
        _SNMPv2C_TRAP = 3,
        _SNMPv2C_INFORM = 4,
        _SNMPv3_TRAP = 5,
        _SNMPv3_INFORM = 6
    };

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
};

PEGASUS_NAMESPACE_END
