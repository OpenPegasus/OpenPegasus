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

DIR = WMIMapper/wbemexec

include $(ROOT)/mak/config.mak

ifdef PEGASUS_HAS_SSL
  FLAGS += -DPEGASUS_HAS_SSL -DPEGASUS_SSL_RANDOMFILE
  SYS_INCLUDES += -I$(OPENSSL_HOME)/include
endif

SYS_INCLUDES += -I$(PEGASUS_ROOT)/src/Clients/wbemexec

LOCAL_DEFINES = -DPEGASUS_INTERNALONLY

LIBRARIES = \
        pegcommon \
        peggetoopt \
        pegwmiclient \
        pegcliutils \
        pegconfig \
        pegwmiserver

SOURCES = \
        WbemExecClient.cpp \
        WbemExecException.cpp \
        XMLProcess.cpp \
        WbemExecCommand.cpp \
        WMIWbemExecClient.cpp

EXTRA_INCLUDES = $(SYS_INCLUDES)

PROGRAM = wbemexec

SYS_LIBS = ws2_32.lib advapi32.lib

include $(ROOT)/mak/program.mak

copy2:
	$(COPY) "$(ROOT)/src/Clients/wbemexec/WbemExecClient.cpp" "./WbemExecClient.cpp"
	$(COPY) "$(ROOT)/src/Clients/wbemexec/WbemExecClient.h" "./WbemExecClient.h"
	$(COPY) "$(ROOT)/src/Clients/wbemexec/WbemExecException.cpp" "./WbemExecException.cpp"
	$(COPY) "$(ROOT)/src/Clients/wbemexec/WbemExecException.h" "./WbemExecException.h"
	$(COPY) "$(ROOT)/src/Clients/wbemexec/XMLProcess.cpp" "./XMLProcess.cpp"
	$(COPY) "$(ROOT)/src/Clients/wbemexec/XMLProcess.h" "./XMLProcess.h"
	$(COPY) "$(ROOT)/src/Clients/wbemexec/WbemExecCommand.cpp" "./WbemExecCommand.cpp"
	$(COPY) "$(ROOT)/src/Clients/wbemexec/WbemExecCommand.h" "./WbemExecCommand.h"

tests:

poststarttests:

install:
