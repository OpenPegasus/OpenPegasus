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
## DIR = pegasus/src/utils/mu/tests/SortCmd

include $(ROOT)/mak/config.mak


LOCAL_DEFINES = -DPEGASUS_INTERNALONLY
LOCAL_CLEAN_TARGET=local-clean

## PROGRAM = 
## SOURCES =

include $(ROOT)/mak/program.mak

.PHONY: FORCE local-clean tests poststarttests

FORCE: 

local-clean:
	$(RM) *.tmp

tests: testdiffsort testsort1
	@ $(ECHO) $(SORT) +++++ all test passed	

##
## test that charecters are not added by sorting
##
testsort1:	
	@ $(SORT) file1_unsorted > file11.tmp
	@ $(SORT) file11.tmp > file12.tmp
	@ $(SORT) file12.tmp > file13.tmp
	@ $(SORT) file13.tmp > file14.tmp
	@ $(DIFF) file11.tmp  file14.tmp
	@ $(RM) *.tmp
	@ $(ECHO) $(SORT) +++++  test1 passed	
##
## test that sort order is correct by comparing to saved output file
##
## this test is dependent upon the sort commands default sorting algorythum
## and therefore is platform dependent so it is not run as a standard 
## test but is left for future development.
##
testsort2:
	@ $(SORT) file1_unsorted > file11.tmp
	@ $(DIFF) file11.tmp file1_sorted
	@ $(RM) *.tmp
	@ $(ECHO) $(SORT) +++++  test2 passed

##
## test the diffsort macro
## 
testdiffsort:
	@ $(call DIFFSORT,file1_unsorted,file1_reverse_sorted)
	@ $(ECHO) diffsort +++++ passed	
     

poststarttests:

