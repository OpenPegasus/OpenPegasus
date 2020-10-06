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
DIR = Pegasus/IndicationService/tests/DisableEnable
include $(ROOT)/mak/config.mak
include ../libraries.mak

EXTRA_INCLUDES = $(SYS_INCLUDES)

LOCAL_DEFINES = -DPEGASUS_INTERNALONLY 

PROGRAM = TestDisableEnable

SOURCES = DisableEnable.cpp

include $(ROOT)/mak/program.mak

tests:

poststarttests:
	@$(ECHO) "TestDisableEnable setup WQL"
	@$(PROGRAM) setup "WQL"
	@$(ECHO) "TestDisableEnable create WQL"
	@$(PROGRAM) create "WQL"
	cimprovider -d -m "ProcessIndicationProviderModule"
	@$(ECHO) "TestDisableEnable delete WQL"
	@$(PROGRAM) delete "WQL"
	cimprovider -e -m "ProcessIndicationProviderModule"
	@$(ECHO) "TestDisableEnable create WQL"
	@$(PROGRAM) create "WQL"
	cimprovider -d -m "ProcessIndicationProviderModule"
	cimprovider -e -m "ProcessIndicationProviderModule"
	@$(ECHO) "TestDisableEnable delete WQL"
	@$(PROGRAM) delete "WQL"
	@$(ECHO) "TestDisableEnable create WQL"
	@$(PROGRAM) create "WQL"
	@$(ECHO) "TestDisableEnable create2 WQL"
	@$(PROGRAM) create2 "WQL"
	@$(ECHO) "TestDisableEnable addProvider WQL"
	@$(PROGRAM) addProvider "WQL"
	@$(ECHO) "TestDisableEnable modifyProvider1 WQL"
	@$(PROGRAM) modifyProvider1 "WQL"
	@$(ECHO) "TestDisableEnable modifyProvider2 WQL"
	@$(PROGRAM) modifyProvider2 "WQL"
	@$(ECHO) "TestDisableEnable deleteProvider WQL"
	@$(PROGRAM) deleteProvider "WQL"
	@$(ECHO) "TestDisableEnable delete2 WQL"
	@$(PROGRAM) delete2 "WQL"
	@$(ECHO) "TestDisableEnable delete WQL"
	@$(PROGRAM) delete "WQL"
	@$(ECHO) "TestDisableEnable cleanup WQL"
	@$(PROGRAM) cleanup "WQL"
ifeq ($(PEGASUS_ENABLE_CQL),true)
	@$(ECHO) "TestDisableEnable setup DMTF:CQL"
	@$(PROGRAM) setup "DMTF:CQL"
	@$(ECHO) "TestDisableEnable create DMTF:CQL"
	@$(PROGRAM) create "DMTF:CQL"
	cimprovider -d -m "ProcessIndicationProviderModule"
	@$(ECHO) "TestDisableEnable delete DMTF:CQL"
	@$(PROGRAM) delete "DMTF:CQL"
	cimprovider -e -m "ProcessIndicationProviderModule"
	@$(ECHO) "TestDisableEnable create DMTF:CQL"
	@$(PROGRAM) create "DMTF:CQL"
	cimprovider -d -m "ProcessIndicationProviderModule"
	cimprovider -e -m "ProcessIndicationProviderModule"
	@$(ECHO) "TestDisableEnable delete DMTF:CQL"
	@$(PROGRAM) delete "DMTF:CQL"
	@$(ECHO) "TestDisableEnable create DMTF:CQL"
	@$(PROGRAM) create "DMTF:CQL"
	@$(ECHO) "TestDisableEnable create2 DMTF:CQL"
	@$(PROGRAM) create2 "DMTF:CQL"
	@$(ECHO) "TestDisableEnable addProvider DMTF:CQL"
	@$(PROGRAM) addProvider "DMTF:CQL"
	@$(ECHO) "TestDisableEnable modifyProvider1 DMTF:CQL"
	@$(PROGRAM) modifyProvider1 "DMTF:CQL"
	@$(ECHO) "TestDisableEnable modifyProvider2 DMTF:CQL"
	@$(PROGRAM) modifyProvider2 "DMTF:CQL"
	@$(ECHO) "TestDisableEnable deleteProvider DMTF:CQL"
	@$(PROGRAM) deleteProvider "DMTF:CQL"
	@$(ECHO) "TestDisableEnable delete2 DMTF:CQL"
	@$(PROGRAM) delete2 "DMTF:CQL"
	@$(ECHO) "TestDisableEnable delete DMTF:CQL"
	@$(PROGRAM) delete "DMTF:CQL"
	@$(ECHO) "TestDisableEnable cleanup DMTF:CQL"
	@$(PROGRAM) cleanup "DMTF:CQL"
endif
	@$(ECHO) "+++++ TestDisableEnable passed all tests"
