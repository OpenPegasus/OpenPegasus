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

ROOT = ../../../..

include $(ROOT)/mak/config.mak

DIRS = \
    TestUtilLib \
    TestMethod \
    TestMethod/tests \
    TestError \
    TestError/tests \
    TestProperty/tests \
    TestFail_1 \
    TestFail_1/tests \
    TestFail_2 \
    TestFail_2/tests\
    TestFail_3 \
    TestFail_3/tests \
    TestFail_4 \
    TestFail_4/tests \
    TestFail_5 \
    TestFail_5/tests \
    TestProviderThreaded \
    TestProviderThreaded/tests \
    TestAssociation \
    TestAssociation/tests \
    TestIndicationStressTestProvider \
    TestIndicationStressTestProvider/tests \
    TestCMPIEnumeration \
    TestCMPIEnumeration/tests \
    TestCMPIInstance \
    TestCMPIInstance/tests \
    TestKeyReturned \
    TestKeyReturned/tests

ifeq ($(PEGASUS_ENABLE_CQL),true)
DIRS += \
    TestIndications \
    TestIndications/tests

ifdef PEGASUS_ENABLE_EXECQUERY
DIRS += \
    TestCMPIBroker \
    TestCMPIBroker/tests \
    TestCMPIBrokerEnc \
    TestCMPIBrokerEnc/tests \
    TestExecQuery \
    TestExecQuery/tests
endif
endif

include $(ROOT)/mak/recurse.mak
