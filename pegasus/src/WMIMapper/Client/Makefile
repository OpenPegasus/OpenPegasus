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

DIR = WMIMapper/Client

include $(ROOT)/mak/config.mak

ifeq ($(PEGASUS_WMIMAPPER_NEED_MSSDK),true)
  EXTRA_INCLUDES = -I"$(MSSdk)/Include" $(SYS_INCLUDES)
endif

LOCAL_DEFINES = -DPEGASUS_CLIENT_INTERNAL -DPEGASUS_INTERNALONLY

LIBRARY = pegwmiclient

LIBRARIES = \
    pegcommon \
	pegclient \
    wmiprovider

PRE_DEPEND_INCLUDES = -I./depends

SOURCES = \
	WMIClientRep.cpp \
	CIMClientRep.cpp \
    WMIClient.cpp \
    CIMOperationRequestEncoder.cpp \
    CIMOperationResponseDecoder.cpp \
    ClientAuthenticator.cpp \
    CIMClientException.cpp

SYS_LIBS = ws2_32.lib advapi32.lib

include $(ROOT)/mak/library.mak

copy2:
	$(COPY) "$(ROOT)/src/Pegasus/Client/ClientPerfDataStore.cpp" "./ClientPerfDataStore.cpp"
	$(COPY) "$(ROOT)/src/Pegasus/Client/ClientPerfDataStore.h" "./ClientPerfDataStore.h"
	$(COPY) "$(ROOT)/src/Pegasus/Client/ClientOpPerformanceDataHandler.h" "./ClientOpPerformanceDataHandler.h"
	$(COPY) "$(ROOT)/src/Pegasus/Client/CIMClientRep.cpp" "./CIMClientRep.cpp"
	$(COPY) "$(ROOT)/src/Pegasus/Client/CIMClientRep.h" "./CIMClientRep.h"
	$(COPY) "$(ROOT)/src/Pegasus/Client/CIMOperationRequestEncoder.cpp" "./CIMOperationRequestEncoder.cpp"
	$(COPY) "$(ROOT)/src/Pegasus/Client/CIMOperationRequestEncoder.h" "./CIMOperationRequestEncoder.h"
	$(COPY) "$(ROOT)/src/Pegasus/Client/CIMOperationResponseDecoder.cpp" "./CIMOperationResponseDecoder.cpp"
	$(COPY) "$(ROOT)/src/Pegasus/Client/CIMOperationResponseDecoder.h" "./CIMOperationResponseDecoder.h"
	$(COPY) "$(ROOT)/src/Pegasus/Client/ClientAuthenticator.cpp" "./ClientAuthenticator.cpp"
	$(COPY) "$(ROOT)/src/Pegasus/Client/ClientAuthenticator.h" "./ClientAuthenticator.h"
	$(COPY) "$(ROOT)/src/Pegasus/Client/CIMClientException.cpp" "./CIMClientException.cpp"
	$(COPY) "$(ROOT)/src/Pegasus/Client/CIMClientException.h" "./CIMClientException.h"
	$(COPY) "$(ROOT)/src/Pegasus/Client/Linkage.h" "./Linkage.h"

clean2:
	$(RM) ./ClientPerfDataStore.cpp
	$(RM) ./ClientPerfDataStore.h
	$(RM) ./ClientOpPerformanceDataHandler.h
	$(RM) ./CIMClientRep.cpp
	$(RM) ./CIMClientRep.h
	$(RM) ./CIMOperationRequestEncoder.cpp
	$(RM) ./CIMOperationRequestEncoder.h
	$(RM) ./CIMOperationResponseDecoder.cpp
	$(RM) ./CIMOperationResponseDecoder.h
	$(RM) ./ClientAuthenticator.cpp
	$(RM) ./ClientAuthenticator.h
	$(RM) ./CIMClientException.cpp
	$(RM) ./CIMClientException.h
	$(RM) ./Linkage.h
