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

DIR = Providers/TestProviders/IndicationStressTestProvider/testclient

include $(ROOT)/mak/config.mak

PEGASUS_ZOS_PROGRAM_OBJECT = yes
LIBRARIES = \
     peglistener \
     pegclient \
     pegexportserver \
     peggeneral \
     pegcommon

EXTRA_INCLUDES = $(SYS_INCLUDES)

ifeq ($(OS),VMS)
 LOCAL_DEFINES = -DINDICATION_DIR="""$(DISPLAYCONSUMER_DIR)"""
else
 LOCAL_DEFINES = -DINDICATION_DIR=\"$(DISPLAYCONSUMER_DIR)\"
endif

LOCAL_DEFINES += -DPEGASUS_INTERNALONLY

PROGRAM = TestIndicationStressTest
PROGRAM2 = $(PROGRAM) IndicationStressTestClass test/TestProvider

SOURCES = IndicationStressTest.cpp

include $(ROOT)/mak/program.mak

include $(ROOT)/mak/test.mak

tests:

## poststarttests: wql_10_1  cql_10_1 wql_10_10 cql_10_10 wql_10_100 cql_10_100 wql_100_10 cql_100_10

## Note: cmpitests and rcmpitests targets are used by the CMPI version of
## Indication stress-test testcase Makefile from the directory
## TestProviders/CMPI/TestIndicationStressTestProvider/tests . 
cmpitests:
	$(MAKE) poststarttests2 PROGRAM2=$(PROGRAM) \
			PROGRAM2+=TestCMPI_IndicationStressTestClass \
			PROGRAM2+=test/TestProvider

rcmpitests:
	$(MAKE) poststarttests2 PROGRAM2=$(PROGRAM) \
			PROGRAM2+=TestCMPI_IndicationStressTestClass \
			PROGRAM2+=test/Remote/Localhost

poststarttests2:
	$(MAKE) wql_10_1
	$(MAKE) wql_10_1 p=HTTP
ifeq ($(PEGASUS_ENABLE_CQL),true)
	$(MAKE) cql_10_1
	$(MAKE) cql_10_1 p=HTTP
endif
	@$(ECHO) "+++++ TestIndicationStressTest passed all tests"

poststarttests:
	$(MAKE) poststarttests2
ifeq ($(PEGASUS_TEST_IPV6), true)
	$(MAKE) poststarttests2 PROGRAM2=$(PROGRAM) \
			PROGRAM2+=IPv6TestClass \
			PROGRAM2+=test/TestProvider
endif

wql_10_1:
	$(PROGRAM2) setup$L WQL $p 
	$(PROGRAM2) run 1 1 $I 
	$(PROGRAM2) run 10 1
	$(PROGRAM2) cleanup

## use the cql_i_t and wql_i_t rule as follows:
## 
## make cql_i_t i=10 t=2 
##        will run 10 iterations in 2 threads
##
## make wql_i_t i=20 t=3 
##        will run 20 iterations in 3 threads
##

## You can use the "p" option to select the protocol used, 
## by the CIM-XML Indication Handler, to send Indications
## to the Server-resident Listener.
##
##  make cql_i_t i=10 t=2 p=HTTPS
##        will send 20 Indications to the embedded Indication
##        Consumer over HTTPS using port 5989.

## By default, setup configures both a Client-resident
## and Server-resider Listener. You can use the "L"
## option to test a single Listener. Valid values for
## L are either CL to test the Client-resident Listener,
## setupCL, or SL the Server-resident Listener, setupSL. 

cql_i_t:
	$(PROGRAM2) setup$L DMTF:CQL $p
	$(PROGRAM2) run $i $t $I
	$(PROGRAM2) cleanup

wql_i_t: 
	$(PROGRAM2) setup$L WQL $p
	$(PROGRAM2) run $i $t $I
	$(PROGRAM2) cleanup


wql_10_10:
	$(PROGRAM2) setup$L WQL $p
	$(PROGRAM2) run 10 10 $I
	$(PROGRAM2) cleanup

wql_10_100:
	$(PROGRAM2) setup$L WQL $p
	$(PROGRAM2) run 10 100 $I
	$(PROGRAM2) cleanup

wql_100_10:
	$(PROGRAM2) setup$L WQL $p
	$(PROGRAM2) run 100 10 $I
	$(PROGRAM2) cleanup

wql_100_100:
	$(PROGRAM2) setup$L WQL $p
	$(PROGRAM2) run 100 100 $I
	$(PROGRAM2) cleanup

wql_1000_100:
	$(PROGRAM2) setup$L WQL $p
	$(PROGRAM2) run 1000 100 $I
	$(PROGRAM2) cleanup

cql_10_1:
	$(PROGRAM2) setup$L DMTF:CQL $p 
	$(PROGRAM2) run 1 1 $I
	$(PROGRAM2) run 10 1
	$(PROGRAM2) cleanup

cql_10_10:
	$(PROGRAM2) setup$L DMTF:CQL $p
	$(PROGRAM2) run 10 10 $I
	$(PROGRAM2) cleanup

cql_10_100:
	$(PROGRAM2) setup$L DMTF:CQL $p
	$(PROGRAM2) run 10 100 $I
	$(PROGRAM2) cleanup

cql_100_10:
	$(PROGRAM2) setup$L DMTF:CQL $p
	$(PROGRAM2) run 100 10 $I
	$(PROGRAM2) cleanup

cql_100_100:
	$(PROGRAM2) setup$L DMTF:CQL $p
	$(PROGRAM2) run 100 100 $I
	$(PROGRAM2) cleanup

cql_1000_100:
	$(PROGRAM2) setup$L DMTF:CQL $p
	$(PROGRAM2) run 1000 100 $I
	$(PROGRAM2) cleanup

testsclean:
	$(PROGRAM2) cleanup

run_TS1_AuthenticationEnabled: testsclean run_TS1_INTERNAL run_TS1_HTTPS_VERIFY_USER
	@ $(ECHO) run_TS1_AuthenticationEnabled  +++++ passed all test

run_TS1_AuthenticationDisabled: testsclean run_TS1_INTERNAL run_TS1_HTTP run_TS1_HTTPS
	@ $(ECHO) run_TS1_AuthenticationDisabled  +++++ passed all test

run_TS1_INTERNAL:
	$(PROGRAM2) setup WQL
	$(PROGRAM2) run 10 1
	$(PROGRAM2) cleanup

run_TS1_HTTP:
	$(PROGRAM2) setup WQL HTTP
	$(PROGRAM2) run 10 1
	$(PROGRAM2) cleanup

run_TS1_HTTPS:
ifdef PEGASUS_HAS_SSL
ifdef PEGASUS_TEST_USER_DEFINED
	$(PROGRAM2) setup WQL HTTPS
	$(PROGRAM2) run 10 1
	$(PROGRAM2) cleanup
endif
endif

run_TS1_HTTPS_VERIFY_USER:
ifdef PEGASUS_HAS_SSL
ifdef PEGASUS_TEST_USER_DEFINED
	$(PROGRAM2) setup WQL HTTPS
	$(PROGRAM2) run 10 1 $(PEGASUS_TEST_USER_ID)
	$(PROGRAM2) cleanup
endif
endif
createSubscriptions:
	$(PROGRAM2) setup$L WQL $p

reregister_providerTests:
	cimprovider -d -m IndicationStressTestProviderModule
	cimprovider -d -m IndicationStressTestConsumerModule
	cimprovider -e -m IndicationStressTestProviderModule
	cimprovider -e -m IndicationStressTestConsumerModule
	$(PROGRAM2) run 1 1 $I
	$(PROGRAM2) cleanup
