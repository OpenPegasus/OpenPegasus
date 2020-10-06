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

DIR = WMIMapper/WMIServer

include $(ROOT)/mak/config.mak

LIBRARIES = \
    pegcommon \
    pegconfig \
    pegwmiserver \
    pegclient \
    pegservice

EXTRA_INCLUDES = $(SYS_INCLUDES)

LOCAL_DEFINES += -DPEGASUS_WMIPROVIDER_INTERNAL -DPEGASUS_INTERNALONLY

PROGRAM = WMIServer

SOURCES = cimserver.cpp

include $(ROOT)/mak/program.mak

ifeq ($(OS_TYPE),windows)
  SYS_LIBS = ws2_32.lib advapi32.lib	
endif

tests:

poststarttests:

install:
	$(RM) $(PEGASUS_HOME)/cimserver_planned.conf
	$(RM) $(PEGASUS_HOME)/cimserver_current.conf
	$(RM) $(PEGASUS_HOME)/server.pem
	$(RM) $(PEGASUS_HOME)/client.pem
	$(RM) $(PEGASUS_HOME)/ssl.rnd
	$(RM) $(PEGASUS_HOME)/ssl.cnf
	$(COPY) cimserver_planned.conf $(PEGASUS_HOME)
	$(COPY) cimserver_current.conf $(PEGASUS_HOME)
	$(COPY) $(PEGASUS_ROOT)/server.pem $(PEGASUS_HOME)
	$(COPY) $(PEGASUS_ROOT)/client.pem $(PEGASUS_HOME)
	$(COPY) $(PEGASUS_ROOT)/ssl.rnd    $(PEGASUS_HOME)
	$(COPY) $(PEGASUS_ROOT)/ssl.cnf    $(PEGASUS_HOME)

run: install
ifeq ($(OS_TYPE),windows)
	cimserver -l
else
	cimserver
endif

copy2:
	$(COPY) $(ROOT)/src/Service/Service.cpp ./Service.cpp
	$(COPY) $(ROOT)/src/Service/Service.h ./Service.h

clean2:
	$(RM) ./Service.cpp
	$(RM) ./Service.h

