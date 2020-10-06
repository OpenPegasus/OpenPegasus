#//%LICENSE////////////////////////////////////////////////////////////////
#//
#// Licensed to The Open Group (TOG) under one or more contributor license
#// agreements.  Refer to the OpenPegasusNOTICE.txt file distributed with
#// this work for additional information regarding copyright ownership.
#// Each contributor licenses this file to you under the OpenPegasus Open
#// Source License; you may not use this file except in compliance with the
#// License.
#//
#// Permission is hereby granted, free of charge, to any person obtaining a
#// copy of this software and associated documentation files (the "Software"),
#// to deal in the Software without restriction, including without limitation
#// the rights to use, copy, modify, merge, publish, distribute, sublicense,
#// and/or sell copies of the Software, and to permit persons to whom the
#// Software is furnished to do so, subject to the following conditions:
#//
#// The above copyright notice and this permission notice shall be included
#// in all copies or substantial portions of the Software.
#//
#// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
#// OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
#// MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
#// IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
#// CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
#// TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
#// SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
#//
#//////////////////////////////////////////////////////////////////////////
ROOT = ../..

include $(ROOT)/mak/config.mak

DIRS = \
    Common \
    General \
    Query/QueryCommon \
    Client \
    Config \
    Repository \
    getoopt \
    WQL \
	Handler \
    RSServer

ifeq ($(PEGASUS_ENABLE_CQL),true)
    DIRS += CQL
endif

ifeq ($(PEGASUS_ENABLE_FQL),true)
    DIRS += FQL
endif

ifeq ($(PEGASUS_ENABLE_PROTOCOL_WSMAN),true)
    DIRS += \
        WsmServer
    TEST_DIRS += \
        WsmServer/tests
endif

ifeq ($(PEGASUS_ENABLE_PROTOCOL_WEB),true)
    DIRS += \
        WebServer
endif

DIRS += \
    Query/QueryExpression \
    Provider \
    Compiler \
    Compiler/cmdline

TEST_DIRS = \
    Common/tests \
    General/tests \
    Client/tests \
    Config/tests \
    Repository/tests \
    WQL/tests

ifeq ($(PEGASUS_ENABLE_CQL),true)
    TEST_DIRS += \
        CQL/tests \
        CQL/CQLCLI
endif

ifeq ($(PEGASUS_ENABLE_FQL),true)
    TEST_DIRS += FQL/tests
endif

TEST_DIRS += \
    Compiler/tests \
    Query/QueryExpression/tests \
    Query/QueryExpression/tests/Queries


ifneq ($(OS),HPUX)
    TEST_DIRS += \
        Compiler/cmdline/tests
endif

ifndef PEGASUS_PAM_AUTHENTICATION
    DIRS += Security/UserManager \
	    ControlProviders/UserAuthProvider
    TEST_DIRS += Security/UserManager/tests
endif


DIRS += \
    ExportClient \
    Server/ProviderRegistrationManager \
    ExportServer \
    Listener \
    Security/Authentication \
    ControlProviders/ConfigSettingProvider \
    ControlProviders/ProviderRegistrationProvider \
    ControlProviders/NamespaceProvider \
    HandlerService \
    IndicationService \
    ManagedClient \
    DynListener

TEST_DIRS += \
    ExportClient/tests \
    Server/ProviderRegistrationManager/tests \
    ExportServer/tests \
    Listener/tests \
    Security/Authentication/tests \
    ControlProviders/ProviderRegistrationProvider/tests \
    HandlerService/tests \
    IndicationService/tests \
    DynListener/tests 

ifdef PEGASUS_ENABLE_SYSTEM_LOG_HANDLER
    DIRS += \
        Handler/SystemLogListenerDestination
    TEST_DIRS += \
        Handler/SystemLogListenerDestination/tests
endif

ifdef PEGASUS_ENABLE_EMAIL_HANDLER
    DIRS += \
        Handler/EmailListenerDestination
    TEST_DIRS += \
        Handler/EmailListenerDestination/tests
endif

ifeq ($(PEGASUS_ENABLE_PROTOCOL_WSMAN),true)
    DIRS += \
        Handler/wsmanIndicationHandler
    TEST_DIRS += \
        Handler/wsmanIndicationHandler/tests/Destination
endif

DIRS += \
    ProviderManager2 \
    ProviderManager2/Default

TEST_DIRS += \
    ProviderManager2/tests \
    ProviderManager2/Default/tests

ifeq ($(PEGASUS_ENABLE_CMPI_PROVIDER_MANAGER), true)
    DIRS += \
        Provider/CMPI \
        ProviderManager2/CMPI
endif

ifdef PEGASUS_ENABLE_REMOTE_CMPI

    DIRS += \
        ProviderManager2/CMPIR/native \
        ProviderManager2/CMPIR \
        ProviderManager2/CMPIR/tcpcomm \
        ProviderManager2/CMPIR/tcpcommrem \
        ProviderManager2/CMPIR/daemon \
        ProviderManager2/CMPIR/tools
endif

ifeq ($(PEGASUS_ENABLE_JMPI_PROVIDER_MANAGER), true)
    DIRS += \
        ProviderManager2/JMPI
endif

DIRS += \
    ProviderManagerRouter \
    ProviderManagerService \
    ProviderManagerService/ProviderAgent

ifdef PEGASUS_USE_PAM_STANDALONE_PROC
    DIRS += \
        Security/Cimservera
endif

ifndef PEGASUS_DISABLE_PERFINST
    DIRS += \
        ControlProviders/Statistic
    TEST_DIRS += \
        ControlProviders/Statistic/test
endif

ifeq ($(PEGASUS_ENABLE_INTEROP_PROVIDER),true)
    DIRS += \
        ControlProviders/InteropProvider
    TEST_DIRS += \
        ControlProviders/InteropProvider/tests \
        ControlProviders/InteropProvider/ServerProfileTests
endif

ifeq ($(PEGASUS_ENABLE_CQL),true)
    DIRS += \
        ControlProviders/QueryCapabilitiesProvider
    TEST_DIRS += \
        ControlProviders/QueryCapabilitiesProvider/tests
endif

ifdef PEGASUS_HAS_SSL
    DIRS += \
        ControlProviders/CertificateProvider
endif

DIRS += \
    Server \
    Handler/CIMxmlIndicationHandler \
    Handler/FileURIHandler  \
    msg/Server \
    msg/CLI \
    msg/Listener

ifeq ($(PEGASUS_USE_EMANATE), true)
    DIRS += \
        Handler/snmpIndicationHandler
endif

ifeq ($(PEGASUS_USE_NET_SNMP), true)
    DIRS += \
        Handler/snmpIndicationHandler
endif

TEST_DIRS += \
    Server/tests \
    Server/tests/InterfaceRestriction \
    Server/tests/TLSv_1_2_Support \
    Server/tests/PulllopCrash \
    Handler/CIMxmlIndicationHandler/tests/Destination \
    Handler/FileURIHandler/tests

ifeq ($(PEGASUS_USE_EMANATE), true)
    TEST_DIRS += \
        Handler/snmpIndicationHandler/tests/testclient \
        Handler/snmpIndicationHandler/tests/SnmpHandlerException
endif


ifeq ($(PEGASUS_USE_NET_SNMP), true)
    TEST_DIRS += \
        Handler/snmpIndicationHandler/tests/testclient \
        Handler/snmpIndicationHandler/tests/SnmpHandlerException
endif


include $(ROOT)/mak/recurse.mak
