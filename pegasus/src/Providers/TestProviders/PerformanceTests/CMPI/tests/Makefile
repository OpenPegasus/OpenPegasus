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
# Test the new CMPI performance test providers for functionality

ROOT = ../../../../../..
include $(ROOT)/mak/config.mak
SHH = @

DIR = Providers/TestProviders/PerformanceTests/CMPI/tests

PROVIDERNS=test/TestProvider

clean:

general:

tests:

poststarttests:
	@$(ECHO) 1. +++++ Test all enumerate operations on the classes

	$(SHH)cimcli ei "cmpiPerf_TestClassA" -n "$(PROVIDERNS)" --sum -count 1
	$(SHH)cimcli ni "cmpiPerf_TestClassA" -n "$(PROVIDERNS)" --sum -count 1
	$(SHH)cimcli ei "cmpiPerf_TestClassB" -n "$(PROVIDERNS)" --sum -count 1
	$(SHH)cimcli ni "cmpiPerf_TestClassB" -n "$(PROVIDERNS)" --sum -count 1
	$(SHH)cimcli ei "cmpiPerf_TestAssocClass" -n "$(PROVIDERNS)" --sum -count 1
	$(SHH)cimcli ni "cmpiPerf_TestAssocClass" -n "$(PROVIDERNS)" --sum -count 1

	@$(ECHO) 2. +++++ Testing getInstance CMPI Perftest providers in $(PROVIDERNS)

	$(SHH)cimcli gi "cmpiPerf_TestClassA.theKey=2" -n "$(PROVIDERNS)" --sum -count 1
	$(SHH)cimcli gi "cmpiPerf_TestClassB.theKey=0" -n "$(PROVIDERNS)" --sum -count 1

	@$(ECHO) 3. +++++ Testing associate operations CMPI Perftest in $(PROVIDERNS)

	$(SHH)cimcli an "cmpiPerf_TestClassA.theKey=2" -n "$(PROVIDERNS)" --sum -count 1
	$(SHH)cimcli a  "cmpiPerf_TestClassA.theKey=2" -n "$(PROVIDERNS)" --sum -count 1

depend:

strip-license:

prepend-license:

# END_OF_FILE
