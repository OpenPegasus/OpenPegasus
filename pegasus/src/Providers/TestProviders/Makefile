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

include $(ROOT)/mak/config.mak

TEST_DIRS = \
	AlertIndicationProvider \
	CLITestProvider \
	CLITestProvider/tests \
	DefaultInstanceProvider \
	DynamicIndicationProvider \
	FamilyProvider \
	FamilyProvider/tests \
	IndicationStressTestProvider \
	IndicationStressTestProvider/testclient \
	IndicationStressTestProvider/testconsumer \
	IndicationTestProvider \
	InstanceProvider \
	InstanceProvider/testclient \
	ChunkingStressProvider \
	ChunkingStressProvider/testclient \
	MethodTestProvider \
	MethodTestProvider/testclient \
	MCCA_TestAssocProvider \
	MCCA_TestAssocProvider/testclient \
	OOPModuleFailureProvider \
	OOPModuleFailureProvider/testclient \
	PG_TestPropertyTypes \
	ProcessIndicationProvider \
	UserContextTestProvider \
	UserContextTestProvider/testclient \
	CIMOMHandleTestProvider \
	CIMOMHandleTestProvider/testclient \
	TestAggregationOutputProvider \
	TestAggregationOutputProvider/testclient \
	LocalizedProvider \
	LocalizedProvider/msg \
	PerformanceTests \
        EmbeddedInstanceProvider \
        EmbeddedInstanceProvider/testClient \
        TestModuleGrouping \
	FaultyInstanceProvider \
	FaultyInstanceProvider/testclient \
	TestGoodInstanceProvider \
	DeliveryRetryTestProvider \
	ResponseStressTestProvider \
        ResponseStressTestProvider/testclient \
	DeliveryRetryTestProvider \
        TestProviderLifecycleIndications \
	TestCIM_ErrorProvider \
	TestCIM_ErrorProvider/tests \
        TestProviderLifecycleIndications/testclient \
        TestProviderLifecycleIndications/testconsumer


ifeq ($(PEGASUS_ENABLE_INTEROP_PROVIDER),true)
TEST_DIRS += \
	TestServerProfileProviders
endif

ifeq ($(PEGASUS_ENABLE_CMPI_PROVIDER_MANAGER), true)
TEST_DIRS += \
	CMPI
endif

ifeq ($(PEGASUS_ENABLE_PROTOCOL_WSMAN),true)
TEST_DIRS += \
	IndicationTestProvider/wsmanTestClient
endif

include $(ROOT)/mak/recurse.mak
