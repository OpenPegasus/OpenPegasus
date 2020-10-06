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

# ATTN: This Makefile uses the testcase in the testclient directory under
#TestProviders/IndicationStressTestProvider.

ROOT = ../../../../../../
DIR = Providers/TestProviders/CMPI/TestIndicationStressTestProvider/tests
MAKEFILE_DIR = ../../../IndicationStressTestProvider/testclient
include $(ROOT)/mak/config.mak
include $(ROOT)/mak/test.mak

tests:

poststarttests:
	$(MAKE) --directory=$(MAKEFILE_DIR) -f Makefile  cmpitests

# NOTE: Test will enabled on Remote CMPI once Remote CMPI Bugs
# are fixed.

#ifdef PEGASUS_ENABLE_REMOTE_CMPI
#	@$(ECHO)  Testing with remote namespace
#	$(MAKE) --directory=$(MAKEFILE_DIR) -f Makefile  rcmpitests
#else
#	@$(ECHO) "+++++ PEGASUS_ENABLE_REMOTE_CMPI is NOT defined: Skipping remote CMPI tests"
#endif

#l10n

clean:

depend:

general:

strip-license:

prepend-license:


