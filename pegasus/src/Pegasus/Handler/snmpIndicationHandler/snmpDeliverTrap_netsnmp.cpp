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

#include <Pegasus/Repository/Packer.h>
#include <Pegasus/Common/Tracer.h>
#include "snmpDeliverTrap_netsnmp.h"

PEGASUS_NAMESPACE_BEGIN

void snmpDeliverTrap_netsnmp::initialize()
{
    PEG_METHOD_ENTER(TRC_IND_HANDLER, "snmpDeliverTrap_netsnmp::initialize");

    // Defined default MIB modules (in net-snmp-config.h) do not need to be
    // loaded and loading them can cause some stderr;
    // use environment variable MIBS to override the default MIB modules.
    // If there is no MIBS environment variable, add it in.

    setenv("MIBS", "", 0);

    // Initialize the mib reader
    netsnmp_set_mib_directory("");
    init_mib();

    // Initializes the SNMP library
    init_snmp("snmpIndicationHandler");

    // don't load/save persistent file
#ifdef NETSNMP_DS_LIB_DONT_PERSIST_STATE
    netsnmp_ds_set_boolean(NETSNMP_DS_LIBRARY_ID,
        NETSNMP_DS_LIB_DONT_PERSIST_STATE, 1);
#endif

    // windows32 specific initialization (is a NOOP on unix)
    SOCK_STARTUP;

    PEG_METHOD_EXIT();
}

void snmpDeliverTrap_netsnmp::terminate()
{
    PEG_METHOD_ENTER(TRC_IND_HANDLER, "snmpDeliverTrap_netsnmp::terminate");

    // Shuts down the application, and appropriate clean up
    snmp_shutdown("snmpIndicationHandler");

    SOCK_CLEANUP;

    PEG_METHOD_EXIT();
}

void snmpDeliverTrap_netsnmp::deliverTrap(
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
    const Array<String>& vbValues)
{

    PEG_METHOD_ENTER(TRC_IND_HANDLER, "snmpDeliverTrap_netsnmp::deliverTrap");

    void* sessionHandle;
    struct snmp_session* sessionPtr;

    struct snmp_pdu* snmpPdu;

    // Creates a SNMP session
    _createSession(
        targetHost, 
        targetHostFormat,
        portNumber,
        securityName,
        snmpVersion,
        engineID,
        snmpSecLevel,
        snmpSecAuthProto,
        snmpSecAuthKey,
        snmpSecPrivProto,
        snmpSecPrivKey,
        sessionHandle,
        sessionPtr);

    try
    {
        _createPdu(snmpVersion, trapOid, sessionPtr, snmpPdu);
    }
    catch (...)
    {
        _destroySession(sessionHandle);

        PEG_METHOD_EXIT();
        throw;
    }

    // Pack OIDs into the PDU
    try
    {
        _packOidsIntoPdu(vbOids, vbTypes, vbValues, snmpPdu);
    }
    catch (Exception& e)
    {
        Logger::put_l(Logger::STANDARD_LOG, System::CIMSERVER, Logger::WARNING,
            MessageLoaderParms(
                _MSG_PACK_CIM_PROPERTY_TO_PDU_FAILED_KEY,
                _MSG_PACK_CIM_PROPERTY_TO_PDU_FAILED,
                e.getMessage()));
    }
    catch (...)
    {
        PEG_TRACE_CSTRING(TRC_DISCARDED_DATA, Tracer::LEVEL2,
            "Snmp Indication Handler failed to pack a CIM "
                "Property into the SNMP PDU: Unknown exception.");
    }

    // Send the trap to the destination
    if (snmp_sess_send(sessionHandle, snmpPdu) == 0)
    {
        Sint32 libErr, sysErr;
        char* errStr;

        // snmp_sess_send failed
        // get library, system errno
        snmp_sess_error(sessionHandle, &libErr, &sysErr, &errStr);

        String exceptionStr = _MSG_SESSION_SEND_FAILED;
        exceptionStr.append(errStr);

        free(errStr);

        snmp_free_pdu(snmpPdu);

        _destroySession(sessionHandle);

        PEG_METHOD_EXIT();

        throw PEGASUS_CIM_EXCEPTION_L(CIM_ERR_FAILED,
            MessageLoaderParms(_MSG_SESSION_SEND_FAILED_KEY, exceptionStr));
    }

    _destroySession(sessionHandle);

    PEG_METHOD_EXIT();
}

// Creates a SNMP session
void snmpDeliverTrap_netsnmp::_createSession(
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
    snmp_session*& sessionPtr)
{
    PEG_METHOD_ENTER(TRC_IND_HANDLER,
        "snmpDeliverTrap_netsnmp::_createSession");

    Sint32 libErr, sysErr;
    char* errStr;
    String exceptionStr;

    struct snmp_session snmpSession;

    {
        AutoMutex autoMut(_sessionInitMutex);
        snmp_sess_init(&snmpSession);

        CString targetHostCStr = targetHost.getCString();

        // peername has format: targetHost:portNumber
        snmpSession.peername =
            (char*)calloc(1,strlen(targetHostCStr) + 1 + 32);

        if (targetHostFormat == _IPV6_ADDRESS)
        {
            sprintf(snmpSession.peername, "udp6:[%s]:%u",
                (const char*)targetHostCStr,
                portNumber);
        }
        else
        {
            sprintf(snmpSession.peername, "%s:%u",
                (const char*)targetHostCStr,
                portNumber);
        }

        sessionHandle = snmp_sess_open(&snmpSession);
    }

    if (sessionHandle == NULL)
    {
        exceptionStr = _MSG_SESSION_OPEN_FAILED;

        // Get library, system errno
        snmp_error(&snmpSession, &libErr, &sysErr, &errStr);

        exceptionStr.append(errStr);

        free(errStr);

        free(snmpSession.peername);

        PEG_METHOD_EXIT();

        throw PEGASUS_CIM_EXCEPTION_L(CIM_ERR_FAILED,
            MessageLoaderParms(_MSG_SESSION_OPEN_FAILED_KEY, exceptionStr));
    }

    try
    {
        // get the snmp_session pointer
        sessionPtr = snmp_sess_session(sessionHandle);
        if (sessionPtr == NULL)
        {
            exceptionStr = _MSG_GET_SESSION_POINT_FAILED;

            // Get library, system errno
            snmp_sess_error(&snmpSession, &libErr, &sysErr, &errStr);

            exceptionStr.append(errStr);

            free(errStr);

            free(snmpSession.peername);

            throw PEGASUS_CIM_EXCEPTION_L(CIM_ERR_FAILED, MessageLoaderParms(
                _MSG_GET_SESSION_POINTER_FAILED_KEY,
                exceptionStr));
        }

        free(snmpSession.peername);

        switch (snmpVersion)
        {
            case _SNMPv1_TRAP:
            {
                sessionPtr->version = SNMP_VERSION_1;
                _addCommunity(sessionPtr,securityName);
               break;
 
            }
            case _SNMPv2C_TRAP:
            {
                sessionPtr->version = SNMP_VERSION_2c;
                _addCommunity(sessionPtr,securityName);
                break;
            }
#ifdef PEGASUS_ENABLE_NET_SNMPV3 
            case _SNMPv3_TRAP:
            {
                sessionPtr->version = SNMP_VERSION_3;
                CString securityNameCStr = securityName.getCString();
                size_t securityNameLen = strlen(securityNameCStr);
                SNMP_FREE(sessionPtr->securityName);
                sessionPtr->securityName = (char *)calloc(1,securityNameLen+1);
                sessionPtr->securityNameLen = securityNameLen;
                memcpy(sessionPtr->securityName, (const char*)securityNameCStr,
                    securityNameLen);

                CString engineIdCStr = engineID.getCString();
                size_t engineIdHexLen = strlen(engineIdCStr);
                size_t engineIdBinLen = 0;
                u_char *engineIdBin = (u_char *)calloc(1,engineIdHexLen);
                free(sessionPtr->securityEngineID);
                if(!snmp_hex_to_binary(&engineIdBin, &engineIdHexLen, 
                    &engineIdBinLen, 1,engineIdCStr))
                {
                    PEG_TRACE_CSTRING(TRC_DISCARDED_DATA, Tracer::LEVEL2,
                        "Snmp Indication Handler failed to generate binary"
                            " engine ID for sending the SNMPv3 trap.");
                    throw PEGASUS_CIM_EXCEPTION_L(CIM_ERR_FAILED, 
                        MessageLoaderParms(
                            "Handler.snmpIndicationHandler."
                                "snmpIndicationHandler."
                            "FAILED_TO_DELIVER_TRAP",
                        "Failed to deliver trap."));
                }
                sessionPtr->securityEngineIDLen = engineIdBinLen;
                sessionPtr->securityEngineID = engineIdBin; 

                switch(snmpSecLevel)
                {
                    case 1:
                        sessionPtr->securityLevel = SNMP_SEC_LEVEL_NOAUTH;
                        break;
                    case 2:
                        sessionPtr->securityLevel = SNMP_SEC_LEVEL_AUTHNOPRIV;
                        break;
                    case 3:
                        sessionPtr->securityLevel = SNMP_SEC_LEVEL_AUTHPRIV;
                        break;
                    default:
                        //use the dedault in the net-snmp conf file.
                        break;
                }
 
                SNMP_FREE(sessionPtr->securityAuthProto);
                if(snmpSecAuthProto == 1) // MD5
                {
                    sessionPtr->securityAuthProto = snmp_duplicate_objid(
                        usmHMACMD5AuthProtocol,
                        USM_AUTH_PROTO_MD5_LEN);
                    sessionPtr->securityAuthProtoLen = USM_AUTH_PROTO_MD5_LEN;
                }
                else if(snmpSecAuthProto == 2)// SHA
                {
                    sessionPtr->securityAuthProto = snmp_duplicate_objid(
                        usmHMACSHA1AuthProtocol,
                        USM_AUTH_PROTO_SHA_LEN);
                    sessionPtr->securityAuthProtoLen = USM_AUTH_PROTO_SHA_LEN;
                }
                // use the default in net-snmp conf files.

                if(snmpSecAuthKey.size() > 0)
                {
                    for(Uint32 i=0;i<snmpSecAuthKey.size();i++)
                    {
                        sessionPtr->securityAuthKey[i] = snmpSecAuthKey[i];
                    }
                    sessionPtr->securityAuthKeyLen = snmpSecAuthKey.size();
                }

                SNMP_FREE(sessionPtr->securityPrivProto);
                //Privacy
                if(snmpSecPrivProto == 1) //DES
                {
                    sessionPtr->securityPrivProto = snmp_duplicate_objid(
                        usmDESPrivProtocol,
                        USM_PRIV_PROTO_DES_LEN);
                    sessionPtr->securityPrivProtoLen = USM_PRIV_PROTO_DES_LEN;
                }
                else if(snmpSecPrivProto == 2) // AES
                {
                    sessionPtr->securityPrivProto = snmp_duplicate_objid(
                        usmAESPrivProtocol,
                        USM_PRIV_PROTO_AES_LEN);
                    sessionPtr->securityPrivProtoLen = USM_PRIV_PROTO_AES_LEN;
                }
                // use the defaults in net-snmp conf files
 
                // Privacy Key
                if(snmpSecPrivKey.size() > 0)
                {
                    for(Uint32 j=0;j<snmpSecPrivKey.size();j++)
                    {
                        sessionPtr->securityPrivKey[j] = snmpSecPrivKey[j];
                    }
                    sessionPtr->securityPrivKeyLen = snmpSecPrivKey.size();
                }
                break;
            }
#endif // ifdef PEGASUS_ENABLE_NET_SNMPV3 
            default:
            {
                PEG_METHOD_EXIT();
                throw PEGASUS_CIM_EXCEPTION_L(CIM_ERR_NOT_SUPPORTED,
                    MessageLoaderParms(
                        _MSG_VERSION_NOT_SUPPORTED_KEY,
                        _MSG_VERSION_NOT_SUPPORTED));
            }
        }

    }
    catch (...)
    {
        _destroySession(sessionHandle);

        PEG_METHOD_EXIT();
        throw;
    }

    PEG_METHOD_EXIT();
}

void snmpDeliverTrap_netsnmp::_addCommunity(
    struct snmp_session*& sessionPtr,
    const String& securityName)
{
    PEG_METHOD_ENTER(TRC_IND_HANDLER,
        "snmpDeliverTrap_netsnmp::_addCommunity");

    // Community Name, default is public
    String communityName;
    if (securityName.size() == 0)
    {
        communityName.assign("public");
    }
    else
    {
        communityName = securityName;
    }

    free(sessionPtr->community);
              
    CString communityNameCStr = communityName.getCString();
    size_t communityNameLen = strlen(communityNameCStr);

    sessionPtr->community = (u_char*)calloc(1,communityNameLen+1);

    memcpy(sessionPtr->community, (const char*)communityNameCStr,
        communityNameLen);

    sessionPtr->community_len = communityNameLen;
    PEG_METHOD_EXIT();
} 

// Creates a SNMP session
void snmpDeliverTrap_netsnmp::_destroySession(
    void* sessionHandle)
{
    PEG_METHOD_ENTER(TRC_IND_HANDLER,
        "snmpDeliverTrap_netsnmp::_destroySession");

    snmp_sess_close(sessionHandle);

    PEG_METHOD_EXIT();
}

// Creates a SNMP TRAP PDU
void snmpDeliverTrap_netsnmp::_createPdu(
    Uint16 snmpVersion,
    const String& trapOid,
    snmp_session*& sessionPtr,
    snmp_pdu*& snmpPdu)
{
    PEG_METHOD_ENTER(TRC_IND_HANDLER, "snmpDeliverTrap_netsnmp::_createPdu");

    oid _SYSTEM_UP_TIME_OID [] = {1,3,6,1,2,1,1,3,0};
    oid _SNMPTRAP_OID [] = {1,3,6,1,6,3,1,1,4,1,0};

    in_addr_t* pduInAddr;

    switch (snmpVersion)
    {
        case _SNMPv1_TRAP:
        {
            // Create the PDU
            snmpPdu = snmp_pdu_create(SNMP_MSG_TRAP);

            // Failed to create pdu
            if (!snmpPdu)
            {
                PEG_METHOD_EXIT();
                throw PEGASUS_CIM_EXCEPTION_L(CIM_ERR_FAILED,
                    MessageLoaderParms(
                        _MSG_PDU_CREATE_FAILED_KEY,
                        _MSG_PDU_CREATE_FAILED));
            }

            // Make sure that the v1 trap PDU includes the local IP address
            pduInAddr = (in_addr_t*) snmpPdu->agent_addr;
            *pduInAddr = get_myaddr();

            // get system up time
            snmpPdu->time = get_uptime();

            // Pack trap information into the PDU
            try
            {
                _packTrapInfoIntoPdu(trapOid, snmpPdu);
            }
            catch (CIMException& e)
            {
                Logger::put_l(
                    Logger::STANDARD_LOG, System::CIMSERVER, Logger::WARNING,
                    MessageLoaderParms(
                        _MSG_PACK_TRAP_INFO_INTO_PDU_FAILED_KEY,
                        _MSG_PACK_TRAP_INFO_INTO_PDU_FAILED,
                        e.getMessage()));
            }

            break;
        }
        case _SNMPv2C_TRAP:
        case _SNMPv3_TRAP:
        {
            // Create the PDU
            snmpPdu = snmp_pdu_create(SNMP_MSG_TRAP2);

            // Failed to create pdu
            if (!snmpPdu)
            {
                PEG_METHOD_EXIT();

                throw PEGASUS_CIM_EXCEPTION_L(CIM_ERR_FAILED,
                    MessageLoaderParms(
                        _MSG_PDU_CREATE_FAILED_KEY,
                        _MSG_PDU_CREATE_FAILED));
            }

            // Add sysUpTime to the PDU
            char sysUpTime[32];
            sprintf(sysUpTime, "%ld", get_uptime());

            Sint32 retCode;
            retCode = snmp_add_var(
                snmpPdu,
                _SYSTEM_UP_TIME_OID,
                OID_LENGTH(_SYSTEM_UP_TIME_OID),
                't',
                sysUpTime);

            // Failed to add sysUpTime to the pdu
            if (retCode != 0)
            {
                String errMsg = snmp_api_errstring(retCode);

                Logger::put_l(
                    Logger::STANDARD_LOG, System::CIMSERVER, Logger::WARNING,
                    MessageLoaderParms(
                        _MSG_ADD_SYSUPTIME_TO_PDU_FAILED_KEY,
                        _MSG_ADD_SYSUPTIME_TO_PDU_FAILED,
                        errMsg));
            }

            // Add snmp trap to the PDU
            retCode = snmp_add_var(
                snmpPdu,
                _SNMPTRAP_OID,
                OID_LENGTH(_SNMPTRAP_OID),
                'o',
                trapOid.getCString());

            // Failed to add snmp trap to the pdu
            if (retCode != 0)
            {
                String errMsg = snmp_api_errstring(retCode);

                Logger::put_l(
                    Logger::STANDARD_LOG, System::CIMSERVER, Logger::WARNING,
                    MessageLoaderParms(
                        _MSG_ADD_SNMP_TRAP_TO_PDU_FAILED_KEY,
                        _MSG_ADD_SNMP_TRAP_TO_PDU_FAILED,
                        errMsg));
            }

            break;
        }
        default:
        {
            PEG_METHOD_EXIT();

            throw PEGASUS_CIM_EXCEPTION_L(CIM_ERR_NOT_SUPPORTED,
                MessageLoaderParms(
                    _MSG_VERSION_NOT_SUPPORTED_KEY,
                    _MSG_VERSION_NOT_SUPPORTED));
        }
    }

    PEG_METHOD_EXIT();
}


// Pack trap information into the PDU
void snmpDeliverTrap_netsnmp::_packTrapInfoIntoPdu(
    const String& trapOid,
    snmp_pdu* snmpPdu)
{
    PEG_METHOD_ENTER(TRC_IND_HANDLER,
        "snmpDeliverTrap_netsnmp::_packTrapInfoIntoPdu");

    oid enterpriseOid[MAX_OID_LEN];
    size_t enterpriseOidLength;

    Array<String> standard_traps;

    standard_traps.append(String("1.3.6.1.6.3.1.1.5.1"));
    standard_traps.append(String("1.3.6.1.6.3.1.1.5.2"));
    standard_traps.append(String("1.3.6.1.6.3.1.1.5.3"));
    standard_traps.append(String("1.3.6.1.6.3.1.1.5.4"));
    standard_traps.append(String("1.3.6.1.6.3.1.1.5.5"));
    standard_traps.append(String("1.3.6.1.6.3.1.1.5.6"));

    Array<const char*> oidSubIdentifiers;

    CString trapOidCStr = trapOid.getCString();

    char* trapOidCopy = strdup(trapOidCStr);
    char* numericEntOid = (char*) malloc(strlen(trapOidCStr)+1);

    try
    {

#if !defined(PEGASUS_OS_TYPE_WINDOWS) && !defined(PEGASUS_OS_ZOS)
        char* last;
        for (const char* p = strtok_r(trapOidCopy, ".", &last); p;
            p=strtok_r(NULL, ".", &last))
#else
        for (const char* p = strtok(trapOidCopy, "."); p; p=strtok(NULL, "."))
#endif
        {
            oidSubIdentifiers.append(p);
        }

        enterpriseOidLength = MAX_OID_LEN;

        if (Contains(standard_traps, trapOid))
        {
            //
            // if the trapOid is one of the standard traps,
            // then the SNMPV1 enterprise parameter must be set
            // to the value of the trapOid, the generic-trap
            // parameter must be set to one of (0 - 5), and the
            // specific-trap parameter must be set to 0
            //

            // Convert trapOid from numeric form to a list of subidentifiers
            if (read_objid((const char*)trapOidCStr, enterpriseOid,
                &enterpriseOidLength) == 0)
            {
                // Failed to parse trapOid

                PEG_METHOD_EXIT();
                throw PEGASUS_CIM_EXCEPTION_L(CIM_ERR_FAILED,
                    MessageLoaderParms(_MSG_READ_OBJID_FAILED_KEY,
                                       _MSG_READ_OBJID_FAILED,
                                       trapOid));
            }

            // the generic trap is last sub-identifier of the
            // trapOid minus 1
            snmpPdu->trap_type =
                atoi(oidSubIdentifiers[oidSubIdentifiers.size() - 1]) - 1;
            snmpPdu->specific_type = 0;
        }
        else
        {
            //
            // if the trapOid is not one of the standard traps:
            // then 1) the generic-trap parameter must be set to 6,
            // 2) if the next-to-last sub-identifier of the
            // trapOid is zero, then the SNMPV1 enterprise
            // parameter is the trapOid with the last 2
            // sub-identifiers removed, otherwise, the
            // SNMPV1 enterprise parameter is the trapOid
            // with the last sub-identifier removed;
            // 3) the SNMPv1 specific-trap parameter is the last
            // sub-identifier of the trapOid;
            //

            snmpPdu->trap_type = 6;

            snmpPdu->specific_type =
                atoi(oidSubIdentifiers[oidSubIdentifiers.size()-1]);

            strcpy(numericEntOid, oidSubIdentifiers[0]);
            for (Uint32 i = 1; i < oidSubIdentifiers.size()-2; i++)
            {
                strcat(numericEntOid, ".");
                strcat(numericEntOid, oidSubIdentifiers[i]);
            }

            if (strcmp(oidSubIdentifiers[oidSubIdentifiers.size()-2], "0") != 0)
            {
                strcat(numericEntOid, ".");
                strcat(numericEntOid,
                   oidSubIdentifiers[oidSubIdentifiers.size()-2]);
            }

            // Convert ent from numeric form to a list of subidentifiers
            if (read_objid(numericEntOid, enterpriseOid,
                &enterpriseOidLength) == 0)
            {
                // Failed to parse numericEntOid

                PEG_METHOD_EXIT();

                throw PEGASUS_CIM_EXCEPTION_L(CIM_ERR_FAILED,
                    MessageLoaderParms(_MSG_READ_ENTOID_FAILED_KEY,
                                       _MSG_READ_ENTOID_FAILED,
                                       String(numericEntOid)));
            }

        }

        SNMP_FREE(snmpPdu->enterprise);
        snmpPdu->enterprise = (oid*) malloc(enterpriseOidLength * sizeof(oid));
        memcpy(snmpPdu->enterprise, enterpriseOid, 
            enterpriseOidLength * sizeof(oid));

        snmpPdu->enterprise_length = enterpriseOidLength;
    }
    catch (...)
    {
        free(trapOidCopy);
        free(numericEntOid);

        PEG_METHOD_EXIT();
        throw;
    }

    free(trapOidCopy);
    free(numericEntOid);

    PEG_METHOD_EXIT();
}

// Pack oids into the PDU
void snmpDeliverTrap_netsnmp::_packOidsIntoPdu(
    const Array<String>& vbOids,
    const Array<String>& vbTypes,
    const Array<String>& vbValues,
    snmp_pdu* snmpPdu)
{

    PEG_METHOD_ENTER(TRC_IND_HANDLER,
        "snmpDeliverTrap_netsnmp::_packOidsIntoPdu");

    char dataType;
    oid vbOid[MAX_OID_LEN];
    size_t vbOidLength = MAX_OID_LEN;

    for (Uint32 i=0; i < vbOids.size(); i++)
    {
        if (vbTypes[i] == "OctetString")
        {
            dataType = 's';
        }
        else if (vbTypes[i] == "Integer")
        {
            dataType = 'i';
        }
        else if (vbTypes[i] == "OID")
        {
            dataType = 'o';
        }
        else
        {
            // Integer, OctetString, and OID are supported SNMP Data Types
            // for the CIM Property

            PEG_METHOD_EXIT();

            throw PEGASUS_CIM_EXCEPTION_L(CIM_ERR_FAILED,
                MessageLoaderParms(_MSG_UNSUPPORTED_SNMP_DATA_TYPE_KEY,
                                   _MSG_UNSUPPORTED_SNMP_DATA_TYPE,
                                   vbTypes[i]));

        }

        // Convert oid of a CIM property from numeric form to a list of
        // subidentifiers
        if (read_objid((const char*)vbOids[i].getCString(), vbOid,
            &vbOidLength) == 0)
        {
            // Failed to parse vbOids

            PEG_METHOD_EXIT();

            throw PEGASUS_CIM_EXCEPTION_L(CIM_ERR_FAILED,
                MessageLoaderParms(_MSG_PARSE_CIM_PROPERTY_OID_FAILED_KEY,
                                   _MSG_PARSE_CIM_PROPERTY_OID_FAILED,
                                   vbOids[i]));
        }

        Sint32 retCode;
        retCode = snmp_add_var(snmpPdu, vbOid, vbOidLength, dataType,
                               vbValues[i].getCString());

        // Failed to add vbOid to the pdu
        if (retCode != 0)
        {
            PEG_METHOD_EXIT();

            throw PEGASUS_CIM_EXCEPTION_L(CIM_ERR_FAILED,
                MessageLoaderParms(_MSG_ADD_VAR_TO_PDU_FAILED_KEY,
                                   _MSG_ADD_VAR_TO_PDU_FAILED,
                                   vbOids[i],
                                   String(snmp_api_errstring(retCode))));

        }
    }

    PEG_METHOD_EXIT();
}

PEGASUS_NAMESPACE_END
