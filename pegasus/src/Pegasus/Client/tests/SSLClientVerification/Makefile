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
ROOT = ../../../../..
DIR = Pegasus/Client/tests/SSLClientVerification
include $(ROOT)/mak/config.mak

include ../libraries.mak

EXTRA_INCLUDES = $(SYS_INCLUDES)

PROGRAM = TestPegClientSSLClientVerification

SOURCES = SSLClientVerification.cpp


# for testing purposes, the username can be anything, as long as its a valid username
ifeq ($(OS_TYPE),windows)
CURRENT_USER=Administrator
else	
CURRENT_USER=`whoami`
endif

include $(ROOT)/mak/program.mak
include $(ROOT)/mak/commands.mak

tests:

clean:

poststarttests:
ifdef PEGASUS_HAS_SSL
	@$(ECHO) ++++SSLClientVerification Test
	$(TOUCH) $(PEGASUS_HOME)/cimserver_current.conf
	$(COPY) $(PEGASUS_HOME)/cimserver_current.conf $(PEGASUS_HOME)/cimserver_current.conf.bkup
	@$(ECHO) Stopping cimserver
	$(MAKE) --directory=$(PEGASUS_ROOT) -f TestMakefile cimstop
	@$(ECHO) Stopped.
	@$(ECHO) Creating truststore directory
	$(MKDIRHIER) $(PEGASUS_HOME)/truststore
	$(COPY) client_cert.pem $(PEGASUS_HOME)/truststore/a692aa3b.0
	$(MKDIRHIER) $(PEGASUS_HOME)/clientkeystore
	$(COPY) client_key.pem $(PEGASUS_HOME)/clientkeystore/client_key.pem
	$(COPY) client_cert.pem $(PEGASUS_HOME)/clientkeystore/client_cert.pem
ifeq ($(OS_TYPE),windows)
	$(COPY) cimserver_planned.conf.win32 $(PEGASUS_HOME)/cimserver_planned.conf
else
	$(COPY) cimserver_planned.conf $(PEGASUS_HOME)/cimserver_planned.conf
	@$(ECHO) user is $(CURRENT_USER)
	sed 's/%CURRENT_USER%/'$(CURRENT_USER)'/' $(PEGASUS_HOME)/cimserver_planned.conf > $(PEGASUS_HOME)/cimserver_planned.conf.2
	$(MOVE) $(PEGASUS_HOME)/cimserver_planned.conf.2 $(PEGASUS_HOME)/cimserver_planned.conf
endif
	@$(ECHO) Starting cimserver
	$(MAKE) --directory=$(PEGASUS_ROOT) -f TestMakefile cimstart
	@$(ECHO) Running
	$(PEGASUS_HOME)/bin/cimconfig -g sslTrustStore
	$(PEGASUS_HOME)/bin/cimconfig -g sslTrustStoreUserName
	$(PEGASUS_HOME)/bin/cimconfig -g sslClientVerificationMode
	$(PEGASUS_HOME)/bin/$(PROGRAM)
	@$(ECHO) Stopping cimserver
	$(MAKE) --directory=$(PEGASUS_ROOT) -f TestMakefile cimstop
	@$(ECHO) Stopped.
	$(COPY) $(PEGASUS_HOME)/cimserver_current.conf.bkup $(PEGASUS_HOME)/cimserver_planned.conf
	$(RMDIRHIER) $(PEGASUS_HOME)/truststore
	$(RMDIRHIER) $(PEGASUS_HOME)/clientkeystore
	@$(ECHO) Starting cimserver with previous configuration
	$(MAKE) --directory=$(PEGASUS_ROOT) -f TestMakefile cimstart
	@$(ECHO) Running.
	@$(ECHO) ++++SSLClientVerification test completed.
endif
