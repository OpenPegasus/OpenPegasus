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

DIR = Providers/TestProviders/FaultyInstanceProvider/testclient

include $(ROOT)/mak/config.mak

LIBRARIES = \
    pegclient \
    peggeneral \
    pegcommon

EXTRA_INCLUDES = $(SYS_INCLUDES)

LOCAL_DEFINES = -DPEGASUS_INTERNALONLY

PROGRAM = TestFaultyInstanceProviderClient

SOURCES = TestFaultyInstanceProviderClient.cpp

include $(ROOT)/mak/program.mak

include $(ROOT)/mak/test.mak

tests:

poststarttests:

runAvailabilityTest:
# When tests are run through valgrind the client timeout
# is disabled hence no point running these tests, since
# the client will never timeout.
ifndef PEGASUS_TEST_VALGRIND_LOG_DIR
	@cimprovider -dm TestFaultyProviderModule
	@cimprovider -em TestFaultyProviderModule
	@cimprovider -dm TestGoodInstanceProviderModule
	@cimprovider -em TestGoodInstanceProviderModule
	$(PROGRAM)	
endif
