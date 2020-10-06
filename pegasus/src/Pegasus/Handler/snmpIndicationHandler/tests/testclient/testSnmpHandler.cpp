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

#include <Pegasus/Common/PegasusAssert.h>
#include <Pegasus/Common/Thread.h>
#include <Pegasus/Common/Constants.h>
#include <Pegasus/Common/FileSystem.h>
#include <Pegasus/General/Stopwatch.h>
#include <Pegasus/Client/CIMClient.h>
#include <Pegasus/Common/HostAddress.h>
#ifdef PEGASUS_USE_NET_SNMP
# include <net-snmp/net-snmp-config.h>
# include <net-snmp/net-snmp-includes.h>
#endif

PEGASUS_USING_PEGASUS;
PEGASUS_USING_STD;

// Interop namespace used with PEGASUS_NAMESPACENAME_INTEROP in Constants.h
const CIMNamespaceName SOURCE_NAMESPACE =
    CIMNamespaceName ("test/TestProvider");

const String INDICATION_CLASS_NAME = "Test_IndicationProviderClass";

const String SNMPV1_HANDLER_NAME = "SNMPHandler01";
const String SNMPV2C_HANDLER_NAME = "SNMPHandler02";
const String SNMPV2C_IPV6_HANDLER_NAME = "SNMPHandler03";
const String SNMPV3_HANDLER_NAME = "SNMPHandler04";
const String FILTER_NAME = "SNMPIPFilter01";

enum SNMPVersion {_SNMPV1_TRAP = 2, _SNMPV2C_TRAP = 3, _SNMPV3_TRAP=5};
enum TargetHostFormat {_HOST_NAME = 2, _IPV4_ADDRESS = 3, _IPV6_ADDRESS = 4};

#define PORT_NUMBER 2006

AtomicInt errorsEncountered(0);

////////////////////////////////////////////////////////////////////////////////
//
// Thread Parameters Class
//
////////////////////////////////////////////////////////////////////////////////

class T_Parms
{
public:
    CIMClient* client;
    Uint32 indicationSendCount;
    Uint32 uniqueID;
};

///////////////////////////////////////////////////////////////////////////

CIMObjectPath _getFilterObjectPath(const String & name)
{
    Array<CIMKeyBinding> keyBindings;
    keyBindings.append (CIMKeyBinding ("SystemCreationClassName",
        System::getSystemCreationClassName (), CIMKeyBinding::STRING));
    keyBindings.append (CIMKeyBinding ("SystemName",
        System::getFullyQualifiedHostName (), CIMKeyBinding::STRING));
    keyBindings.append (CIMKeyBinding ("CreationClassName",
        PEGASUS_CLASSNAME_INDFILTER.getString(), CIMKeyBinding::STRING));
    keyBindings.append (CIMKeyBinding ("Name", name,
        CIMKeyBinding::STRING));
    return(CIMObjectPath("", CIMNamespaceName (),
        PEGASUS_CLASSNAME_INDFILTER, keyBindings));
}

CIMObjectPath _getHandlerObjectPath(const String & name)
{
    Array<CIMKeyBinding> keyBindings;
    keyBindings.append (CIMKeyBinding ("SystemCreationClassName",
        System::getSystemCreationClassName (), CIMKeyBinding::STRING));
    keyBindings.append (CIMKeyBinding ("SystemName",
        System::getFullyQualifiedHostName (), CIMKeyBinding::STRING));
    keyBindings.append (CIMKeyBinding ("CreationClassName",
        PEGASUS_CLASSNAME_INDHANDLER_SNMP.getString(),
        CIMKeyBinding::STRING));
    keyBindings.append (CIMKeyBinding ("Name", name,
        CIMKeyBinding::STRING));
    return(CIMObjectPath("", CIMNamespaceName (),
        PEGASUS_CLASSNAME_INDHANDLER_SNMP, keyBindings));
}

CIMObjectPath _getSubscriptionObjectPath(
    const String & filterName,
    const String & handlerName)
{
    CIMObjectPath filterObjectPath = _getFilterObjectPath(filterName);

    CIMObjectPath handlerObjectPath = _getHandlerObjectPath(handlerName);

    Array<CIMKeyBinding> subscriptionKeyBindings;
    subscriptionKeyBindings.append (CIMKeyBinding ("Filter",
        CIMValue(filterObjectPath)));
    subscriptionKeyBindings.append (CIMKeyBinding ("Handler",
        CIMValue(handlerObjectPath)));
    return(CIMObjectPath("", CIMNamespaceName (),
        PEGASUS_CLASSNAME_INDSUBSCRIPTION, subscriptionKeyBindings));
}

CIMObjectPath _createHandlerInstance(
    CIMClient & client,
    const String & name,
    const String & targetHost,
    const String & securityName,
    const Uint16 targetHostFormat,
    const Uint16 snmpVersion,
    const String & snmpEngineID,
    const Uint8 & snmpSecLevel,
    const Uint8 & snmpSecAuthProto,
    const String & snmpSecAuthKey,
    const Uint8 & snmpSecPrivProto,
    const String & snmpSecPrivKey)
{
    CIMInstance handlerInstance (PEGASUS_CLASSNAME_INDHANDLER_SNMP);
    handlerInstance.addProperty (CIMProperty (CIMName
        ("SystemCreationClassName"), System::getSystemCreationClassName ()));
    handlerInstance.addProperty (CIMProperty (CIMName ("SystemName"),
        System::getFullyQualifiedHostName ()));
    handlerInstance.addProperty (CIMProperty (CIMName ("CreationClassName"),
        PEGASUS_CLASSNAME_INDHANDLER_SNMP.getString ()));
    handlerInstance.addProperty (CIMProperty (CIMName ("Name"), name));
    handlerInstance.addProperty (CIMProperty (CIMName ("TargetHost"),
        targetHost));
    handlerInstance.addProperty (CIMProperty (CIMName ("TargetHostFormat"),
        CIMValue ((Uint16) targetHostFormat)));
    handlerInstance.addProperty (CIMProperty (CIMName ("SNMPSecurityName"),
        securityName));
    handlerInstance.addProperty (CIMProperty (CIMName ("SnmpVersion"),
        CIMValue ((Uint16) snmpVersion)));
    handlerInstance.addProperty (CIMProperty (CIMName ("PortNumber"),
        CIMValue ((Uint32) PORT_NUMBER)));

#ifdef PEGASUS_ENABLE_NET_SNMPV3
    if(snmpVersion == _SNMPV3_TRAP)
    {

        handlerInstance.addProperty (CIMProperty (
            CIMName ("SNMPEngineID"),snmpEngineID));
        handlerInstance.addProperty (CIMProperty (
            CIMName ("SNMPSecurityLevel"),snmpSecLevel)); //AuthPriv
        handlerInstance.addProperty (CIMProperty (
            CIMName ("SNMPSecurityAuthProtocol"),snmpSecAuthProto));

        size_t snmpSecAuthProtoLen=0;
 
        oid *snmpSecAuthProtoOid = NULL;
        if(snmpSecAuthKey.size() > 0)
        {
           if(snmpSecAuthProto == 1)
            {
                snmpSecAuthProtoOid = snmp_duplicate_objid(
                    usmHMACMD5AuthProtocol,
                    USM_AUTH_PROTO_MD5_LEN);
                snmpSecAuthProtoLen = USM_AUTH_PROTO_MD5_LEN;
            }
            else if(snmpSecAuthProto == 2)
            {
                snmpSecAuthProtoOid = snmp_duplicate_objid(
                    usmHMACSHA1AuthProtocol,
                    USM_AUTH_PROTO_SHA_LEN);
                snmpSecAuthProtoLen = USM_AUTH_PROTO_SHA_LEN;
            }
            else 
            {
                cout << "Invalid authentication protocol specified to " << 
                    "create handler." << endl;
                PEGASUS_TEST_ASSERT(false);
            }

            CString snmpSecAuthKeyCstr = snmpSecAuthKey.getCString();
            size_t authKeyLen = snmpSecAuthKey.size();
            u_char * snmpSecAuthKeyPtr = (u_char *)malloc(authKeyLen);
            u_char * encryptedSecurityAuthKey = (u_char *)malloc(authKeyLen);
            size_t securityAuthKeyLen = USM_AUTH_KU_LEN;
            memcpy(snmpSecAuthKeyPtr,(const char *)snmpSecAuthKeyCstr,
                strlen(snmpSecAuthKeyCstr));
            snmpSecAuthKeyPtr[authKeyLen] = '\0';
 
            if(generate_Ku(snmpSecAuthProtoOid,
                snmpSecAuthProtoLen,
                snmpSecAuthKeyPtr,
                strlen(snmpSecAuthKeyCstr),
                encryptedSecurityAuthKey,
                &securityAuthKeyLen) != SNMPERR_SUCCESS)
            {
                cout << "Failed to generate the snmp authentication key" 
                    << endl;
                free(snmpSecAuthKeyPtr);
                free(encryptedSecurityAuthKey);
                PEGASUS_TEST_ASSERT(false);
            }

            Array<Uint8> authKey;
            for(Uint32 i=0; i<securityAuthKeyLen; i++)
            {
                authKey.append(encryptedSecurityAuthKey[i]);
            }
            handlerInstance.addProperty (CIMProperty (
                CIMName ("SNMPSecurityAuthKey"),
                authKey));
            free(snmpSecAuthKeyPtr);
            free(encryptedSecurityAuthKey);
        }

        handlerInstance.addProperty (CIMProperty (
            CIMName ("SNMPSecurityPrivProtocol"),snmpSecPrivProto)); 
 
        if(snmpSecPrivKey.size() > 0 )
        {
            CString snmpSecPrivKeyCstr = snmpSecPrivKey.getCString();
            u_char * snmpSecPrivKeyPtr = (u_char *)malloc(USM_PRIV_KU_LEN);
            u_char * encryptedSecurityPrivKey = 
                (u_char *)malloc(USM_PRIV_KU_LEN);
            size_t securityPrivKeyLen = USM_PRIV_KU_LEN;
            memcpy(snmpSecPrivKeyPtr,(const char *)snmpSecPrivKeyCstr,
                USM_PRIV_KU_LEN);
            if(generate_Ku(snmpSecAuthProtoOid,
                snmpSecAuthProtoLen,
                snmpSecPrivKeyPtr,
                strlen(snmpSecPrivKeyCstr),
                encryptedSecurityPrivKey,
                &securityPrivKeyLen) != SNMPERR_SUCCESS)
            {
                cout << "Failed to generate the snmp privacy key" 
                    << endl;
                free(snmpSecPrivKeyPtr);
                free(encryptedSecurityPrivKey);
                PEGASUS_TEST_ASSERT(false);
            }
     
            Array<Uint8> privKey;
            for(Uint32 i=0; i<securityPrivKeyLen; i++)
            {
                privKey.append(encryptedSecurityPrivKey[i]);
            }

            handlerInstance.addProperty (CIMProperty (
                CIMName ("SNMPSecurityPrivKey"),
                privKey));
            free(snmpSecPrivKeyPtr);
            free(encryptedSecurityPrivKey);
        }  
    }
#endif // ifdef PEGASUS_ENABLE_NET_SNMPV3

    return client.createInstance(
        PEGASUS_NAMESPACENAME_INTEROP, handlerInstance);
}

CIMObjectPath _createFilterInstance(
    CIMClient & client,
    const String & name,
    const String & query,
    const String & qlang)
{
    CIMInstance filterInstance (PEGASUS_CLASSNAME_INDFILTER);
    filterInstance.addProperty (CIMProperty (CIMName
        ("SystemCreationClassName"), System::getSystemCreationClassName ()));
    filterInstance.addProperty (CIMProperty (CIMName ("SystemName"),
        System::getFullyQualifiedHostName ()));
    filterInstance.addProperty (CIMProperty (CIMName ("CreationClassName"),
        PEGASUS_CLASSNAME_INDFILTER.getString ()));
    filterInstance.addProperty (CIMProperty (CIMName ("Name"), name));
    filterInstance.addProperty (CIMProperty (CIMName ("Query"), query));
    filterInstance.addProperty (CIMProperty (CIMName ("QueryLanguage"),
        String (qlang)));
    filterInstance.addProperty (CIMProperty (CIMName ("SourceNamespace"),
        SOURCE_NAMESPACE.getString ()));

    return client.createInstance(PEGASUS_NAMESPACENAME_INTEROP, filterInstance);
}

CIMObjectPath _createSubscriptionInstance(
    CIMClient & client,
    const CIMObjectPath & filterPath,
    const CIMObjectPath & handlerPath)
{
    CIMInstance subscriptionInstance (PEGASUS_CLASSNAME_INDSUBSCRIPTION);
    subscriptionInstance.addProperty (CIMProperty (CIMName ("Filter"),
        filterPath, 0, PEGASUS_CLASSNAME_INDFILTER));
    subscriptionInstance.addProperty (CIMProperty (CIMName ("Handler"),
        handlerPath, 0, PEGASUS_CLASSNAME_INDHANDLER_SNMP));
    subscriptionInstance.addProperty (CIMProperty
        (CIMName ("SubscriptionState"), CIMValue ((Uint16) 2)));

    return client.createInstance(
        PEGASUS_NAMESPACENAME_INTEROP, subscriptionInstance);
}

void _sendTestIndication(
    CIMClient* client,
    const CIMName & methodName,
    Uint32 indicationSendCount)
{
    //
    //  Invoke method to send test indication
    //
    Array <CIMParamValue> inParams;
    Array <CIMParamValue> outParams;
    Array <CIMKeyBinding> keyBindings;
    Sint32 result;

    CIMObjectPath className (String::EMPTY, CIMNamespaceName (),
        CIMName ("Test_IndicationProviderClass"), keyBindings);

    inParams.append(CIMParamValue(String("indicationSendCount"),
        CIMValue(indicationSendCount)));

    CIMValue retValue = client->invokeMethod
        (SOURCE_NAMESPACE,
         className,
         methodName,
         inParams,
         outParams);

    retValue.get (result);
    PEGASUS_TEST_ASSERT (result == 0);
}

void _deleteSubscriptionInstance(
    CIMClient & client,
    const String & filterName,
    const String & handlerName)
{
    CIMObjectPath subscriptionObjectPath =
       _getSubscriptionObjectPath(filterName, handlerName);
    client.deleteInstance(
        PEGASUS_NAMESPACENAME_INTEROP, subscriptionObjectPath);
}

void _deleteHandlerInstance(
    CIMClient & client,
    const String & name)
{
    CIMObjectPath handlerObjectPath = _getHandlerObjectPath(name);
    client.deleteInstance (PEGASUS_NAMESPACENAME_INTEROP, handlerObjectPath);
}

void _deleteFilterInstance(
    CIMClient & client,
    const String & name)
{
    CIMObjectPath filterObjectPath = _getFilterObjectPath(name);
    client.deleteInstance (PEGASUS_NAMESPACENAME_INTEROP, filterObjectPath);
}

void _usage()
{
   cerr << endl
        << "Usage:" << endl
        << "    TestSnmpHandler setup [ WQL | DMTF:CQL ]\n"
        << "    TestSnmpHandler run <indicationSendCount> "
        << "[<threads>]\n"
        << "    where: " << endl
        << "       <indicationSendCount> is the number of indications to\n"
        << "            generate and has to be greater than zero." << endl
        << "       <threads> is an optional number of client threads to\n"
        << "            create, default is one." << endl
        << "    TestSnmpHandler cleanup\n"
        << "    TestSnmpHandler removelog\n\n"
        << "Note :\n"
        << "For running snmp v3 tests create an user by name \"sahana\" in\n"
        << "smpd.conf and snmptrapd.conf with the following credentials :- \n"
        << "engineId = 0x80001f88808a67e858ee38ec4c \n"
        << "Authentication protocol = MD5 \n"
        << "Privacy Protocol = DES \n"
        << "Authentication key = setup_passphrase \n"
        << endl << endl;
}

void _setup (CIMClient & client, const String& qlang)
{
    CIMObjectPath filterObjectPath;
    CIMObjectPath snmpv1HandlerObjectPath;
    CIMObjectPath snmpv2HandlerObjectPath;
    CIMObjectPath snmpv2IPV6HandlerObjectPath;
    CIMObjectPath snmpv3HandlerObjectPath;

    try
    {
        filterObjectPath = _createFilterInstance (client, FILTER_NAME,
            String ("SELECT * FROM Test_IndicationProviderClass"),
            qlang);
    }
    catch (CIMException& e)
    {
        if (e.getCode() == CIM_ERR_ALREADY_EXISTS)
        {
            filterObjectPath = _getFilterObjectPath(FILTER_NAME);
            cerr << "----- Warning: Filter Instance Not Created: "
                << e.getMessage () << endl;
        }
        else
        {
            cerr << "----- Error: Filter Instance Not Created: " << endl;
            throw;
        }
    }

    try
    {
        // Create SNMPv1 trap handler
        snmpv1HandlerObjectPath = _createHandlerInstance (client,
            SNMPV1_HANDLER_NAME,
            System::getFullyQualifiedHostName(),
            "",
            _HOST_NAME,
            _SNMPV1_TRAP,
            String(),0,1,String(),1,String());
    }
    catch (CIMException& e)
    {
        if (e.getCode() == CIM_ERR_ALREADY_EXISTS)
        {
            snmpv1HandlerObjectPath = _getHandlerObjectPath(
                SNMPV1_HANDLER_NAME);
            cerr << "----- Warning: SNMPv1 Trap Handler Instance Not Created: "
                << e.getMessage () << endl;
        }
        else
        {
            cerr << "----- Error: SNMPv1 Trap Handler Instance Not Created: "
                << endl;
            throw;
        }
    }

    try
    {
        _createSubscriptionInstance (client, filterObjectPath,
             snmpv1HandlerObjectPath);
    }
    catch (CIMException& e)
    {
        if (e.getCode() == CIM_ERR_ALREADY_EXISTS)
        {
            cerr << "----- Warning: Client Subscription Instance: "
                << e.getMessage () << endl;
        }
        else
        {
            cerr << "----- Error: Client Subscription Instance: " << endl;
            throw;
        }
    }

    try
    {
        String ipAddress;
        int af;
        System::getHostIP(System::getFullyQualifiedHostName (), &af, ipAddress);
        // Create SNMPv2 trap handler
        snmpv2HandlerObjectPath = _createHandlerInstance (client,
            SNMPV2C_HANDLER_NAME,
            ipAddress,
            "public",
            af == AF_INET ? _IPV4_ADDRESS : _IPV6_ADDRESS,
            _SNMPV2C_TRAP,
            String(),0,1,String(),1,String());
    }
    catch (CIMException& e)
    {
        if (e.getCode() == CIM_ERR_ALREADY_EXISTS)
        {
            snmpv2HandlerObjectPath = _getHandlerObjectPath(
                SNMPV2C_HANDLER_NAME);
            cerr << "----- Warning: SNMPv2c Trap Handler Instance Not Created: "
                << e.getMessage () << endl;
        }
        else
        {
            cerr << "----- Error: SNMPv2c Trap Handler Instance Not Created: "
                << endl;
            throw;
        }
    }

    try
    {
        _createSubscriptionInstance (client, filterObjectPath,
             snmpv2HandlerObjectPath);
    }
    catch (CIMException& e)
    {
        if (e.getCode() == CIM_ERR_ALREADY_EXISTS)
        {
            cerr << "----- Warning: Client Subscription Instance: "
                << e.getMessage () << endl;
        }
        else
        {
            cerr << "----- Error: Client Subscription Instance: " << endl;
            throw;
        }
    }

#if defined(PEGASUS_ENABLE_IPV6)
    // create a subscription with trap destination of IPV6 address format
    try
    {
        // Create SNMPv2 IPV6 trap handler
        snmpv2IPV6HandlerObjectPath = _createHandlerInstance (client,
            SNMPV2C_IPV6_HANDLER_NAME,
            String("::1"),
            "public",
            _IPV6_ADDRESS,
            _SNMPV2C_TRAP,
            String(),0,1,String(),1,String());
    }
    catch (CIMException& e)
    {
        if (e.getCode() == CIM_ERR_ALREADY_EXISTS)
        {
            snmpv2IPV6HandlerObjectPath = _getHandlerObjectPath(
                SNMPV2C_IPV6_HANDLER_NAME);
            cerr << "----- Warning: SNMPv2c IPV6 Trap Handler Instance "
                "Not Created: " << e.getMessage () << endl;
        }
        else
        {
            cerr << "----- Error: SNMPv2c IPV6 Trap Handler Instance Not "
                "Created: " << endl;
            throw;
        }
    }

    try
    {
        _createSubscriptionInstance (client, filterObjectPath,
             snmpv2IPV6HandlerObjectPath);
    }
    catch (CIMException& e)
    {
        if (e.getCode() == CIM_ERR_ALREADY_EXISTS)
        {
            cerr << "----- Warning: Client Subscription Instance: "
                << e.getMessage () << endl;
        }
        else
        {
            cerr << "----- Error: Client Subscription Instance: " << endl;
            throw;
        }
    }
#endif

#ifdef PEGASUS_ENABLE_NET_SNMPV3
    // create a snmp V3 trap handler.
    try
    {
        String ipAddress;
        int af;
        System::getHostIP(System::getFullyQualifiedHostName (), &af, ipAddress);
        // Create SNMPv3 trap handler
        snmpv3HandlerObjectPath = _createHandlerInstance (client,
            SNMPV3_HANDLER_NAME,
            System::getFullyQualifiedHostName(),
            "sahana",
            _HOST_NAME,
            _SNMPV3_TRAP, 
            "0x80001f88808a67e858ee38ec4c",
            3,
            1,
            "setup_passphrase",
            1,
            "setup_passphrase");

    }
    catch (CIMException& e)
    {
        if (e.getCode() == CIM_ERR_ALREADY_EXISTS)
        {
            snmpv3HandlerObjectPath = _getHandlerObjectPath(
                SNMPV2C_IPV6_HANDLER_NAME);
            cerr << "----- Warning: SNMPv3 Trap Handler Instance "
                "Not Created: " << e.getMessage () << endl;
        }
        else
        {
            cerr << "----- Error: SNMPv3 Trap Handler Instance Not "
                "Created: " << endl;
            throw;
        }
    }

    try
    {
        _createSubscriptionInstance (client, filterObjectPath,
             snmpv3HandlerObjectPath);
    }
    catch (CIMException& e)
    {
        if (e.getCode() == CIM_ERR_ALREADY_EXISTS)
        {
            cerr << "----- Warning: Client Subscription Instance: "
                << e.getMessage () << endl;
        }
        else
        {
            cerr << "----- Error: Client Subscription Instance: " << endl;
            throw;
        }
    }
#endif // ifdef PEGASUS_ENABLE_NET_SNMPV3
}

void _cleanup (CIMClient & client)
{
    try
    {
        _deleteSubscriptionInstance (client, FILTER_NAME,
            SNMPV1_HANDLER_NAME);
    }
    catch (CIMException& e)
    {
        if (e.getCode() != CIM_ERR_NOT_FOUND)
        {
            cerr << "----- Error: deleteSubscriptionInstance failure: "
                 << endl;
            throw;
        }
    }
    try
    {
        _deleteSubscriptionInstance (client, FILTER_NAME,
            SNMPV2C_HANDLER_NAME);
    }
    catch (CIMException& e)
    {
        if (e.getCode() != CIM_ERR_NOT_FOUND)
        {
            cerr << "----- Error: deleteSubscriptionInstance failure: "
                 << endl;
            throw;
        }
    }

#if defined(PEGASUS_ENABLE_IPV6)
    try
    {
        _deleteSubscriptionInstance (client, FILTER_NAME,
            SNMPV2C_IPV6_HANDLER_NAME);
    }
    catch (CIMException& e)
    {
        if (e.getCode() != CIM_ERR_NOT_FOUND)
        {
            cerr << "----- Error: deleteSubscriptionInstance failure: "
                 << endl;
            throw;
        }
    }
#endif

#ifdef PEGASUS_ENABLE_NET_SNMPV3
    try
    {
        _deleteSubscriptionInstance (client, FILTER_NAME,
            SNMPV3_HANDLER_NAME);
    }
    catch (CIMException& e)
    {
        if (e.getCode() != CIM_ERR_NOT_FOUND)
        {
            cerr << "----- Error: deleteSubscriptionInstance failure: "
                 << endl;
            throw;
        }
    }
#endif

    try
    {
        _deleteFilterInstance (client, FILTER_NAME);
    }
    catch (CIMException& e)
    {
        if (e.getCode() != CIM_ERR_NOT_FOUND)
        {
            cerr << "----- Error: deleteFilterInstance failure: " << endl;
            throw;
        }
    }

    try
    {
        _deleteHandlerInstance (client, SNMPV1_HANDLER_NAME);
    }
    catch (CIMException& e)
    {
        if (e.getCode() != CIM_ERR_NOT_FOUND)
        {
            cerr << "----- Error: deleteHandlerInstance failure: " << endl;
            throw;
        }
    }
    try
    {
        _deleteHandlerInstance (client, SNMPV2C_HANDLER_NAME);
    }
    catch (CIMException& e)
    {
        if (e.getCode() != CIM_ERR_NOT_FOUND)
        {
            cerr << "----- Error: deleteHandlerInstance failure: " << endl;
            throw;
        }
    }
#if defined(PEGASUS_ENABLE_IPV6)
    try
    {
        _deleteHandlerInstance (client, SNMPV2C_IPV6_HANDLER_NAME);
    }
    catch (CIMException& e)
    {
        if (e.getCode() != CIM_ERR_NOT_FOUND)
        {
            cerr << "----- Error: deleteHandlerInstance failure: " << endl;
            throw;
        }
    }
#endif

#ifdef PEGASUS_ENABLE_NET_SNMPV3
    try
    {
        _deleteHandlerInstance (client, SNMPV3_HANDLER_NAME);
    }
    catch (CIMException& e)
    {
        if (e.getCode() != CIM_ERR_NOT_FOUND)
        {
            cerr << "----- Error: deleteHandlerInstance failure: " << endl;
            throw;
        }
    }
#endif
}

static void _testEnd(const String& uniqueID, const double elapsedTime)
{
    cout << "+++++ thread" << uniqueID << ": passed in " << elapsedTime
        << " seconds" << endl;
}

ThreadReturnType PEGASUS_THREAD_CDECL _executeTests(void *parm)
{
    Thread *my_thread = (Thread *)parm;
    AutoPtr<T_Parms> parms((T_Parms *)my_thread->get_parm());
    CIMClient* client = parms->client;
    Uint32 indicationSendCount = parms->indicationSendCount;
    Uint32 id = parms->uniqueID;
    char id_[4];
    memset(id_,0x00,sizeof(id_));
    sprintf(id_,"%u",id);
    String uniqueID = "_";
    uniqueID.append(id_);

    try
    {
        Stopwatch elapsedTime;

        elapsedTime.start();
        try
        {
            _sendTestIndication (client, CIMName ("SendTestIndicationTrap"),
            indicationSendCount);
        }
        catch (Exception & e)
        {
            cerr << "----- sendTestIndication failed: " << e.getMessage () <<
                endl;
            exit (-1);
        }
        elapsedTime.stop();
        _testEnd(uniqueID, elapsedTime.getElapsed());
    }
    catch(Exception  & e)
    {
        cout << e.getMessage() << endl;
    }

    return ThreadReturnType(0);
}

Thread * _runTestThreads(
    CIMClient* client,
    Uint32 indicationSendCount,
    Uint32 uniqueID)
{
    // package parameters, create thread and run...
    AutoPtr<T_Parms> parms(new T_Parms());
    parms->client = client;
    parms->indicationSendCount = indicationSendCount;
    parms->uniqueID = uniqueID;
    AutoPtr<Thread> t(new Thread(_executeTests, (void*)parms.release(), false));
    t->run();
    return t.release();
}

Uint32 _getReceivedTrapCount(Uint16 snmpVersion, const String& logFile)
{
    String trap1 = "Trap Info: TRAP, SNMP v1, community public";
    String trap2 = "Trap Info: TRAP2, SNMP v2c, community public";
    String trap3 = "Trap Info: TRAP2, SNMP v3, user sahana, context ";

    Uint32 receivedTrap1Count = 0;
    Uint32 receivedTrap2Count = 0;
#ifdef PEGASUS_ENABLE_NET_SNMPV3
    Uint32 receivedTrap3Count = 0;
#endif

    ifstream ifs(logFile.getCString());
    if (!ifs)
    {
        return (0);
    }

    String line;
    while (GetLine(ifs, line))
    {
        if (String::compare(line, trap1) == 0)
        {
            receivedTrap1Count++;
        }
        if (String::compare(line, trap2) == 0)
        {
            receivedTrap2Count++;
        }
#ifdef PEGASUS_ENABLE_NET_SNMPV3
        if (String::compare(line, trap3) == 0)
        {
            receivedTrap3Count++;
        }
#endif
    }

    ifs.close();

    switch (snmpVersion)
    {
        case _SNMPV1_TRAP:
        {
            return (receivedTrap1Count);
        }
        case _SNMPV2C_TRAP:
        {
            return (receivedTrap2Count);
        }
#ifdef PEGASUS_ENABLE_NET_SNMPV3
        case _SNMPV3_TRAP:
        {
            return (receivedTrap3Count);
        }
#endif
        default:
        {
            return (0);
        }
    }

}

#ifdef PEGASUS_USE_NET_SNMP
// Stop snmptrapd process if it is running and remove
// procIdFile file if it exists
//
void _stopSnmptrapd(const String& processIdFile)
{
    Uint32 receiverPid;
    FILE *fd;
    if ((fd = fopen(processIdFile.getCString(), "r")) != NULL)
    {
        if (fscanf(fd, "%d\n", &receiverPid) != 1)
        {
            throw Exception("Failed to read trapd pid from procIdFile.");
        }

        kill(receiverPid, SIGTERM);

        fclose(fd);
    }

    if (FileSystem::exists(processIdFile))
    {
        FileSystem::removeFile(processIdFile);
    }
}

static Boolean _startSnmptrapd(
    FILE **trapInfo,
    const String& processIdFile,
    const String& logFile)
{
    String snmptrapdCmd;

    Uint32 portNumber = PORT_NUMBER;
    char portNumberStr[32];
    sprintf(portNumberStr, "%lu", (unsigned long) portNumber);

    //
    // build snmptrapd cmd options
    //

    // Specify logging incoming traps to trapLogFile
    // Save the process ID of the snmptrapd in procIdFile
    snmptrapdCmd.append(
        "/usr/sbin/snmptrapd -f -Lf ");
    snmptrapdCmd.append(logFile);
    snmptrapdCmd.append(" -p ");
    snmptrapdCmd.append(processIdFile);

    // Specify incoming trap format
    snmptrapdCmd.append( " -F \"\nTrap Info: %P\nVariable: %v\n\"");

    // Specify listening address
#if defined(PEGASUS_ENABLE_IPV6)
    snmptrapdCmd.append(" UDP6:");
    snmptrapdCmd.append(portNumberStr);
    snmptrapdCmd.append(",UDP:");
#else
    snmptrapdCmd.append(" UDP:");
#endif
    snmptrapdCmd.append(System::getFullyQualifiedHostName ());

    snmptrapdCmd.append(":");
    snmptrapdCmd.append(portNumberStr);

    if ((*trapInfo = popen(snmptrapdCmd.getCString(), "r")) == NULL)
    {
        throw Exception ("snmptrapd can not be started");
    }

#define MAX_ITERATIONS 300
#define SLEEP_SEC 1

    Uint32 iterations = 0;

    // Wait until snmptrapd started
    while (iterations < MAX_ITERATIONS)
    {
        iterations++;
        if (FileSystem::exists(processIdFile))
        {
            return (true);
        }
        else
        {
            System::sleep(SLEEP_SEC);

        }
    }

    throw Exception ("snmptrapd can not be started");
}
#endif

void _removeTrapLogFile(const String& logFile)
{
    // if trapLogFile exists, remove it
    if (FileSystem::exists(logFile))
    {
        FileSystem::removeFile(logFile);
    }
}

void _receiveExpectedTraps(
    CIMClient& workClient,
    Uint32 indicationSendCount,
    Uint32 runClientThreadCount,
    const String& logFile)
{
    Uint32 indicationTrapV1SendCount = 0;
    Uint32 indicationTrapV2SendCount = 0;
    Uint32 indicationTrapV3SendCount = 0;

    CIMClient * clientConnections = new CIMClient[runClientThreadCount];

    // determine total number of indication send count
    indicationTrapV1SendCount =
        indicationSendCount * runClientThreadCount;

    // if IPV6 is enabled, an additional SNMPv2c trap is sent to IPV6 address
#if defined(PEGASUS_ENABLE_IPV6)
    indicationTrapV2SendCount = 2 * indicationTrapV1SendCount;
#else
    indicationTrapV2SendCount = indicationTrapV1SendCount;
#endif

#ifdef PEGASUS_ENABLE_NET_SNMPV3
    indicationTrapV3SendCount =
        indicationSendCount * runClientThreadCount;
#endif

    // calculate the timeout based on the total send count allowing
    // using the MSG_PER_SEC rate
    // allow 20 seconds of test overhead for very small tests

#define MSG_PER_SEC 4

    Uint32 testTimeout = PEGASUS_DEFAULT_CLIENT_TIMEOUT_MILLISECONDS
                        + (indicationTrapV2SendCount/MSG_PER_SEC)*1000;

    // connect the clients
    for(Uint32 i = 0; i < runClientThreadCount; i++)
    {
        clientConnections[i].setTimeout(testTimeout);
        clientConnections[i].connectLocal();
    }

    // run tests
    Thread ** clientThreads = new Thread *[runClientThreadCount];

    Stopwatch trapReceiverElapsedTime;

    trapReceiverElapsedTime.start();

    for(Uint32 i = 0; i < runClientThreadCount; i++)
    {
        clientThreads[i] = _runTestThreads(&clientConnections[i],
            indicationSendCount, i);
    }

    for(Uint32 i=0; i< runClientThreadCount; i++)
    {
        clientThreads[i]->join();
        delete clientThreads[i];
    }

    delete[] clientConnections;
    delete[] clientThreads;

    //
    //  Allow time for the trap to be received
    //  Wait in SLEEP_SEC second intervals.
    //  Put msg out every MSG_SEC intervals
    //

#define SLEEP_SEC 1
#define COUT_TIME_INTERVAL 30
#define MAX_NO_CHANGE_ITERATIONS COUT_TIME_INTERVAL*3

    Uint32 noChangeIterations = 0;
    Uint32 priorReceivedTrap1Count = 0;
    Uint32 priorReceivedTrap2Count = 0;
    Uint32 priorReceivedTrap3Count = 0;
    Uint32 currentReceivedTrap1Count = 0;
    Uint32 currentReceivedTrap2Count = 0;
    Uint32 currentReceivedTrap3Count = 0;
    Uint32 totalIterations = 0;

    //
    // Wait for the trap receiver to receive the expected
    // number of Indication traps, indicationTrapV1SendCount
    // and indicationTrapV2SendCount.
    //
    // We will continue to wait until either (indicationTrapV1SendCount
    // and indicationTrapV2SendCount) Indications have been received
    // by the trap receiver or no new
    // Indications have been received in the previous
    // MAX_NO_CHANGE_ITERATIONS.
    // iterations.
    //

    Boolean receivedTrapCountComplete = false;
    Boolean receiverTrap1NoChange = true;
    Boolean receiverTrap2NoChange = true;
    Boolean receiverTrap3NoChange = true;

    while (noChangeIterations <= MAX_NO_CHANGE_ITERATIONS)
    {
        totalIterations++;

        currentReceivedTrap1Count =
            _getReceivedTrapCount(_SNMPV1_TRAP, logFile);
        currentReceivedTrap2Count =
            _getReceivedTrapCount(_SNMPV2C_TRAP, logFile);
        currentReceivedTrap3Count =
            _getReceivedTrapCount(_SNMPV3_TRAP,logFile);

        if (totalIterations % COUT_TIME_INTERVAL == 1 &&
            !(receivedTrapCountComplete))
        {
            cout << "++++ The trap receiver has received "
            << currentReceivedTrap1Count << " of "
            << indicationTrapV1SendCount << " SNMPv1 trap."
            << endl;
            cout << "++++ The trap receiver has received "
            << currentReceivedTrap2Count << " of "
            << indicationTrapV2SendCount << " SNMPv2c trap."
            << endl;
            cout << "++++ The trap receiver has received "
            << currentReceivedTrap3Count<< " of "
            << indicationTrapV3SendCount << " SNMPv3 trap."
            << endl;
        }

        if ((indicationTrapV1SendCount == currentReceivedTrap1Count) &&
            (indicationTrapV2SendCount == currentReceivedTrap2Count) &&
            (indicationTrapV3SendCount == currentReceivedTrap3Count))
        {
             receivedTrapCountComplete = true;
             trapReceiverElapsedTime.stop();
        }
        if (!(receiverTrap1NoChange =
                (priorReceivedTrap1Count == currentReceivedTrap1Count)))
        {
             priorReceivedTrap1Count = currentReceivedTrap1Count;
        }

        if (!(receiverTrap2NoChange =
                (priorReceivedTrap2Count == currentReceivedTrap2Count)))
        {
             priorReceivedTrap2Count = currentReceivedTrap2Count;
        }
        if (!(receiverTrap3NoChange =
                (priorReceivedTrap3Count == currentReceivedTrap3Count)))
        {
             priorReceivedTrap3Count = currentReceivedTrap3Count;
        }

        if (receivedTrapCountComplete)
        {
            cout << "++++ The trap receiver has received "
            << currentReceivedTrap1Count << " of "
            << indicationTrapV1SendCount << " SNMPv1 trap."
            << endl;
            cout << "++++ The trap receiver has received "
            << currentReceivedTrap2Count << " of "
            << indicationTrapV2SendCount << " SNMPv2c trap."
            << endl;
            cout << "++++ The trap receiver has received "
            << currentReceivedTrap3Count << " of "
            << indicationTrapV3SendCount<< " SNMPv3 trap."
            << endl;

            break;
        }
        if (receiverTrap1NoChange || 
            receiverTrap2NoChange || 
            receiverTrap3NoChange)
        {
           noChangeIterations++;
        }
        else
        {
           noChangeIterations = 0;
        }

        System::sleep (SLEEP_SEC);
    }

    if (!receivedTrapCountComplete)
    {
        trapReceiverElapsedTime.stop();
    }

    // assert that all indications sent have been received.
    PEGASUS_TEST_ASSERT(indicationTrapV1SendCount ==
       currentReceivedTrap1Count);
    PEGASUS_TEST_ASSERT(indicationTrapV2SendCount ==
       currentReceivedTrap2Count);
    PEGASUS_TEST_ASSERT(indicationTrapV3SendCount ==
       currentReceivedTrap3Count);
}

int _beginTest(CIMClient& workClient,
    Uint32 indicationSendCount,
    Uint32 runClientThreadCount,
    const String& processIdFile,
    const String& logFile)
{

#ifdef PEGASUS_USE_NET_SNMP

    // Stop snmptrapd process if it is running
    _stopSnmptrapd(processIdFile);

    // if trapLogFile exists, remove it
    _removeTrapLogFile(logFile);

    FILE * trapInfo;

    try
    {
        _startSnmptrapd(&trapInfo, processIdFile, logFile);
    }
    catch (Exception & e)
    {
        cerr << e.getMessage() << endl;
        return (-1);
    }

    // Extended for all snmp implementation
    _receiveExpectedTraps(workClient, indicationSendCount,
        runClientThreadCount, logFile);

    // Stop snmptrapd process if it is running and remove procIdFile
    _stopSnmptrapd(processIdFile);

    pclose(trapInfo);

    // if error encountered then fail the test.
    if (errorsEncountered.get())
    {
        cout << "+++++ test failed" << endl;
        return (-1);
    }
    else
    {
        cout << "+++++ passed all tests" << endl;
    }

    return (0);

#else
    cerr << "Cannot create a trap receiver." << endl;
    return (-1);
#endif
}

int main (int argc, char** argv)
{
    // This client connection is used solely to create and delete subscriptions.
    CIMClient workClient;
    try
    {
        workClient.connectLocal();

        String processIdFile = TRAP_DIR;
        processIdFile.append("/procIdFile");

        String logFile = TRAP_DIR;
        logFile.append("/trapLogFile");

        if (argc <= 1 || argc > 4)
        {
            cerr << "Invalid argument count: " << argc << endl;
            _usage();
            return 1;
        }
        else if (strcmp(argv[1], "setup") == 0)
        {
            if (argc < 3)
            {
                cerr << "Missing query language" << endl;
                _usage();
                return -1;
            }

            if ((strcmp(argv[2], "WQL") != 0) &&
                (strcmp(argv[2], "DMTF:CQL") != 0))
            {
                cerr << "Invalid query language: '" << argv[2] << "'" << endl;
                _usage();
                return -1;
            }

            _setup(workClient, argv[2]);

            cout << "+++++ setup completed successfully" << endl;
            return 0;
        }
        else if (String::equalNoCase(argv[1], "run"))
        {
            if (argc < 3)
            {
                cerr << "Invalid indicationSendCount." << endl;
                _usage ();
                return -1;
            }

            Uint32 indicationSendCount = atoi(argv[2]);

            Uint32 runClientThreadCount = 1;

            if (argc == 4)
            {
                runClientThreadCount = atoi(argv[3]);
            }

            int rc = _beginTest(workClient, indicationSendCount,
                runClientThreadCount, processIdFile, logFile);
            return rc;
        }
        else if (String::equalNoCase(argv[1], "cleanup"))
        {
            if (argc > 2)
            {
                cerr << "Invalid argument count." << endl;
                _usage ();
                return -1;
            }

            _cleanup (workClient);

            cout << "+++++ cleanup completed successfully" << endl;
            return 0;
        }
        else if (String::equalNoCase(argv[1], "removelog"))
        {
            if (argc > 2)
            {
                cerr << "Invalid argument count." << endl;
                _usage ();
                return -1;
            }

            _removeTrapLogFile(logFile);
            cout << "+++++ removelog completed successfully" << endl;
            return 0;
        }
        else
        {
            cerr << "Invalid option: " << argv[1] << endl;
            _usage ();
            return -1;
        }
    }
    catch (Exception & e)
    {
        cerr << "Error: " << e.getMessage() << endl;
    }

    return -1;
}
