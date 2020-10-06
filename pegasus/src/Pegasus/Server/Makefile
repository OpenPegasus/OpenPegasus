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
ROOT = ../../..

DIR = Pegasus/Server

include $(ROOT)/mak/config.mak

EXTRA_INCLUDES = $(SYS_INCLUDES)

LOCAL_DEFINES = -DPEGASUS_SERVER_INTERNAL -DPEGASUS_INTERNALONLY

STATIC=1
LIBRARY = pegserver

LIBRARIES = \
	pegrepository \
	pegprovider \
	pegexportserver \
	pegpmrouter \
	pegpmservice \
	pegprovidermanager \
	pegconfig \
	pegauthentication \
	peghandlerservice \
	pegindicationservice \
	pegquerycommon \
	pegwql \
	pegclient \
	pegprm \
	DefaultProviderManager \
	ConfigSettingProvider \
	ProviderRegistrationProvider\
	NamespaceProvider \
	peggeneral \
	pegcommon \
	cimrsserver

ifndef PEGASUS_PAM_AUTHENTICATION
    LIBRARIES += peguser \
		 UserAuthProvider
endif

ifeq ($(PEGASUS_ENABLE_INTEROP_PROVIDER),true)
LIBRARIES += \
	InteropProvider
ifdef PEGASUS_SLP_REG_TIMEOUT
LIBRARIES += \
	pegslp_client
endif
endif

ifndef PEGASUS_DISABLE_PERFINST
LIBRARIES += \
	CIMOMStatDataProvider \
	InteropProvider
endif

ifeq ($(PEGASUS_ENABLE_FQL),true)
LIBRARIES += pegfql
endif

ifeq ($(PEGASUS_ENABLE_CQL),true)
LIBRARIES += \
        pegcql \
	CIMQueryCapabilitiesProvider
endif

ifdef PEGASUS_HAS_SSL
LIBRARIES += \
	CertificateProvider
endif

ifeq ($(PEGASUS_ENABLE_PROTOCOL_WSMAN),true)
    LIBRARIES += pegwsmserver
endif

ifeq ($(PEGASUS_ENABLE_PROTOCOL_WEB),true)
    LIBRARIES += pegwebserver
endif

PRE_DEPEND_INCLUDES = -I./depends

SOURCES = \
	EnumerationContext.cpp \
	EnumerationContextTable.cpp \
	CIMOperationRequestDecoder.cpp \
	CIMOperationResponseEncoder.cpp \
	CIMOperationRequestDispatcher.cpp \
	CIMOperationRequestAuthorizer.cpp \
	HTTPAuthenticatorDelegator.cpp \
	ShutdownProvider.cpp \
	ShutdownService.cpp \
	CIMServer.cpp \
	CIMServerState.cpp \
	reg_table.cpp \
	QuerySupportRouter.cpp \
	WQLOperationRequestDispatcher.cpp

ifeq ($(OS),zos)
SOURCES += \
	ConsoleManager_zOS.cpp
endif

ifeq ($(PEGASUS_ENABLE_CQL),true)
SOURCES += \
        CQLOperationRequestDispatcher.cpp
endif


ifeq ($(OS_TYPE),windows)
SYS_LIBS = ws2_32.lib advapi32.lib
endif

include $(ROOT)/mak/library.mak
