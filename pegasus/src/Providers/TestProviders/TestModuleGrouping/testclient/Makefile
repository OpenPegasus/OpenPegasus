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

PEGASUS_ZOS_PROGRAM_OBJECT = yes

DIR = Providers/TestProviders/TestModuleGrouping/testclient

include $(ROOT)/mak/config.mak

LOCAL_DEFINES = -DPEGASUS_INTERNALONLY

LIBRARIES = \
    pegclient \
    pegcommon

EXTRA_INCLUDES = $(SYS_INCLUDES)

PROGRAM = TestGroupMethodProvider

SOURCES = TestGroupMethodProvider.cpp

include $(ROOT)/mak/program.mak

tests:

poststarttests:
	@$(ECHO) "Registering the providers ...."
	@$(PROGRAM) registerProviders
	@$(ECHO) "Testing the module grouping...."
	@$(PROGRAM) testGrouping
	@$(ECHO) "Deregistering the providers ...."
	@$(PROGRAM) deregisterProviders
	@$(ECHO) "Registering the providers ...."
	@$(PROGRAM) registerProviders
	@$(ECHO) "Testing the module grouping...."
	@$(PROGRAM) testGrouping
	@$(ECHO) "Disabling the provider  module TestGroupingProviderModule2 ...."
	@cimprovider -d -m TestGroupingProviderModule2
	@$(ECHO) "Testing the module grouping with TestGroupingProviderModule2 disabled...."
	@$(PROGRAM) testGroupingWithProviderDisabled
	@$(ECHO) "Enabling the provider  module TestGroupingProviderModule2 ...."
	@cimprovider -e -m TestGroupingProviderModule2
	@$(ECHO) "Testing the module grouping...."
	@$(PROGRAM) testGrouping
	@$(ECHO) "Setting the TestGroupingProviderModule2 group to none..."
	@cimprovider -g "" -m TestGroupingProviderModule2
	@$(ECHO) "Testing the module grouping with TestGroupingProviderModule2 removed from grouping...."
	@$(PROGRAM) testGroupingWithProviderDisabled
	@$(ECHO) "Setting the TestGroupingProviderModule2 group to TestGroup1..."
	@cimprovider -g TestGroup1 -m TestGroupingProviderModule2
	@$(ECHO) "Testing the module grouping...."
	@$(PROGRAM) testGrouping
	@$(ECHO) "Deregistering the providers ...."
	@$(PROGRAM) deregisterProviders
	@$(ECHO) "Testing the indication providers with grouping enabled...."
	@$(ECHO) "Setting maxFailedProviderModuleRestarts to 0...."
	@cimconfig -s maxFailedProviderModuleRestarts=0
	@$(PROGRAM) testIndications
	@$(ECHO) "Testing with maxFailedProviderModuleRestarts=1...."
	@cimconfig -s maxFailedProviderModuleRestarts=1
	@$(PROGRAM) testIndications restart
	@$(ECHO) "Setting maxFailedProviderModuleRestarts to 3...."
	@cimconfig -s maxFailedProviderModuleRestarts=3
