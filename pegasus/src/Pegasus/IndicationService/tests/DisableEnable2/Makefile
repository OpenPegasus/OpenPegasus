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
DIR = Pegasus/IndicationService/tests/DisableEnable2
include $(ROOT)/mak/config.mak
include ../libraries.mak

EXTRA_INCLUDES = $(SYS_INCLUDES)

LOCAL_DEFINES = -DPEGASUS_INTERNALONLY 

PROGRAM = TestDisableEnable2

SOURCES = DisableEnable2.cpp

include $(ROOT)/mak/program.mak

tests:

poststarttests:
	@$(ECHO) "TestDisableEnable2 setup WQL"
	@$(PROGRAM) setup "WQL"
	@$(ECHO) "TestDisableEnable2 sendFail WQL"
	@$(PROGRAM) sendFail "WQL"
	@$(ECHO) "TestDisableEnable2 create WQL"
	@$(PROGRAM) create "WQL"
	@$(ECHO) "TestDisableEnable2 sendSucceed WQL"
	@$(PROGRAM) sendSucceed "WQL"
	cimprovider -d -m IndicationTestProviderModule
	@$(ECHO) "TestDisableEnable2 sendBlock WQL"
	@$(PROGRAM) sendBlock "WQL"
	@$(ECHO) "TestDisableEnable2 delete WQL"
	@$(PROGRAM) delete "WQL"
	@$(ECHO) "TestDisableEnable2 sendBlock WQL"
	@$(PROGRAM) sendBlock "WQL"
	cimprovider -e -m IndicationTestProviderModule
	@$(ECHO) "TestDisableEnable2 sendFail WQL"
	@$(PROGRAM) sendFail "WQL"
	@$(ECHO) "TestDisableEnable2 create WQL"
	@$(PROGRAM) create "WQL"
	@$(ECHO) "TestDisableEnable2 sendSucceed WQL"
	@$(PROGRAM) sendSucceed "WQL"
	cimprovider -d -m IndicationTestProviderModule
	@$(ECHO) "TestDisableEnable2 sendBlock WQL"
	@$(PROGRAM) sendBlock "WQL"
	cimprovider -e -m IndicationTestProviderModule
	@$(ECHO) "TestDisableEnable2 sendSucceed WQL"
	@$(PROGRAM) sendSucceed "WQL"
	@$(ECHO) "TestDisableEnable2 delete WQL"
	@$(PROGRAM) delete "WQL"
	@$(ECHO) "TestDisableEnable2 sendFail WQL"
	@$(PROGRAM) sendFail "WQL"
	@$(ECHO) "TestDisableEnable2 cleanup WQL"
	@$(PROGRAM) cleanup "WQL"
ifeq ($(PEGASUS_ENABLE_CQL),true)
	@$(ECHO) "TestDisableEnable2 setup DMTF:CQL"
	@$(PROGRAM) setup "DMTF:CQL"
	@$(ECHO) "TestDisableEnable2 sendFail DMTF:CQL"
	@$(PROGRAM) sendFail "DMTF:CQL"
	@$(ECHO) "TestDisableEnable2 create DMTF:CQL"
	@$(PROGRAM) create "DMTF:CQL"
	@$(ECHO) "TestDisableEnable2 sendSucceed DMTF:CQL"
	@$(PROGRAM) sendSucceed "DMTF:CQL"
	cimprovider -d -m IndicationTestProviderModule
	@$(ECHO) "TestDisableEnable2 sendBlock DMTF:CQL"
	@$(PROGRAM) sendBlock "DMTF:CQL"
	@$(ECHO) "TestDisableEnable2 delete DMTF:CQL"
	@$(PROGRAM) delete "DMTF:CQL"
	@$(ECHO) "TestDisableEnable2 sendBlock DMTF:CQL"
	@$(PROGRAM) sendBlock "DMTF:CQL"
	cimprovider -e -m IndicationTestProviderModule
	@$(ECHO) "TestDisableEnable2 sendFail DMTF:CQL"
	@$(PROGRAM) sendFail "DMTF:CQL"
	@$(ECHO) "TestDisableEnable2 create DMTF:CQL"
	@$(PROGRAM) create "DMTF:CQL"
	@$(ECHO) "TestDisableEnable2 sendSucceed DMTF:CQL"
	@$(PROGRAM) sendSucceed "DMTF:CQL"
	cimprovider -d -m IndicationTestProviderModule
	@$(ECHO) "TestDisableEnable2 sendBlock DMTF:CQL"
	@$(PROGRAM) sendBlock "DMTF:CQL"
	cimprovider -e -m IndicationTestProviderModule
	@$(ECHO) "TestDisableEnable2 sendSucceed DMTF:CQL"
	@$(PROGRAM) sendSucceed "DMTF:CQL"
	@$(ECHO) "TestDisableEnable2 delete DMTF:CQL"
	@$(PROGRAM) delete "DMTF:CQL"
	@$(ECHO) "TestDisableEnable2 sendFail DMTF:CQL"
	@$(PROGRAM) sendFail "DMTF:CQL"
	@$(ECHO) "TestDisableEnable2 cleanup DMTF:CQL"
	@$(PROGRAM) cleanup "DMTF:CQL"
endif
	@$(ECHO) "TestDisableEnable2 setup2 WQL"
	@$(PROGRAM) setup2 "WQL"
	@$(ECHO) "TestDisableEnable2 sendFail WQL"
	@$(PROGRAM) sendFail "WQL"
	@$(ECHO) "TestDisableEnable2 create2 WQL"
	@$(PROGRAM) create2 "WQL"
	@$(ECHO) "TestDisableEnable2 sendSucceed WQL"
	@$(PROGRAM) sendSucceed "WQL"
	cimprovider -d -m IndicationTestProviderModule
	@$(ECHO) "TestDisableEnable2 sendBlock WQL"
	@$(PROGRAM) sendBlock "WQL"
	@$(ECHO) "TestDisableEnable2 delete2 WQL"
	@$(PROGRAM) delete2 "WQL"
	@$(ECHO) "TestDisableEnable2 sendBlock WQL"
	@$(PROGRAM) sendBlock "WQL"
	cimprovider -e -m IndicationTestProviderModule
	@$(ECHO) "TestDisableEnable2 sendFail WQL"
	@$(PROGRAM) sendFail "WQL"
	@$(ECHO) "TestDisableEnable2 create2 WQL"
	@$(PROGRAM) create2 "WQL"
	@$(ECHO) "TestDisableEnable2 sendSucceed WQL"
	@$(PROGRAM) sendSucceed "WQL"
	cimprovider -d -m IndicationTestProviderModule
	@$(ECHO) "TestDisableEnable2 sendBlock WQL"
	@$(PROGRAM) sendBlock "WQL"
	cimprovider -e -m IndicationTestProviderModule
	@$(ECHO) "TestDisableEnable2 sendSucceed WQL"
	@$(PROGRAM) sendSucceed "WQL"
	@$(ECHO) "TestDisableEnable2 delete2 WQL"
	@$(PROGRAM) delete2 "WQL"
	@$(ECHO) "TestDisableEnable2 sendFail WQL"
	@$(PROGRAM) sendFail "WQL"
	@$(ECHO) "TestDisableEnable2 cleanup2 WQL"
	@$(PROGRAM) cleanup2 "WQL"
ifeq ($(PEGASUS_ENABLE_CQL),true)
	@$(ECHO) "TestDisableEnable2 setup2 DMTF:CQL"
	@$(PROGRAM) setup2 "DMTF:CQL"
	@$(ECHO) "TestDisableEnable2 sendFail DMTF:CQL"
	@$(PROGRAM) sendFail "DMTF:CQL"
	@$(ECHO) "TestDisableEnable2 create2 DMTF:CQL"
	@$(PROGRAM) create2 "DMTF:CQL"
	@$(ECHO) "TestDisableEnable2 sendSucceed DMTF:CQL"
	@$(PROGRAM) sendSucceed "DMTF:CQL"
	cimprovider -d -m IndicationTestProviderModule
	@$(ECHO) "TestDisableEnable2 sendBlock DMTF:CQL"
	@$(PROGRAM) sendBlock "DMTF:CQL"
	@$(ECHO) "TestDisableEnable2 delete2 DMTF:CQL"
	@$(PROGRAM) delete2 "DMTF:CQL"
	@$(ECHO) "TestDisableEnable2 sendBlock DMTF:CQL"
	@$(PROGRAM) sendBlock "DMTF:CQL"
	cimprovider -e -m IndicationTestProviderModule
	@$(ECHO) "TestDisableEnable2 sendFail DMTF:CQL"
	@$(PROGRAM) sendFail "DMTF:CQL"
	@$(ECHO) "TestDisableEnable2 create2 DMTF:CQL"
	@$(PROGRAM) create2 "DMTF:CQL"
	@$(ECHO) "TestDisableEnable2 sendSucceed DMTF:CQL"
	@$(PROGRAM) sendSucceed "DMTF:CQL"
	cimprovider -d -m IndicationTestProviderModule
	@$(ECHO) "TestDisableEnable2 sendBlock DMTF:CQL"
	@$(PROGRAM) sendBlock "DMTF:CQL"
	cimprovider -e -m IndicationTestProviderModule
	@$(ECHO) "TestDisableEnable2 sendSucceed DMTF:CQL"
	@$(PROGRAM) sendSucceed "DMTF:CQL"
	@$(ECHO) "TestDisableEnable2 delete2 DMTF:CQL"
	@$(PROGRAM) delete2 "DMTF:CQL"
	@$(ECHO) "TestDisableEnable2 sendFail DMTF:CQL"
	@$(PROGRAM) sendFail "DMTF:CQL"
	@$(ECHO) "TestDisableEnable2 cleanup2 DMTF:CQL"
	@$(PROGRAM) cleanup2 "DMTF:CQL"
endif
	@$(ECHO) "+++++ TestDisableEnable2 passed all tests"
