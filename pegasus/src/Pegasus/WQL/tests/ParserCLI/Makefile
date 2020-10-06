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
DIR = Pegasus/WQL/tests/ParserCLI

LIBRARIES = pegwql pegcommon pegquerycommon

include $(ROOT)/mak/config.mak

LOCAL_DEFINES = -DPEGASUS_INTERNALONLY

PROGRAM = TestParserCLI
SOURCES = ParserCLI.cpp

include $(ROOT)/mak/program.mak

# Warning: These tests are based on a specific set of paramaters and their
# values in ParserCLI. the values of w,x,y,z are preset and the
# end result must be true.   Thus, for example w is always TRUE

#
# Normal test behaviour - The test are first parsed and then evaluated.
#     If either the parse or the evaluation fails then the test fails. 
#
#
# Keywords to modify te normal test behaviour:
#
#      parseError    - indicates a parse error is expected
#                      The test will fail if the parse passes.
#
#      evaluateError - indicates an evaluate error is expected
#                      The test will fail if the evaluation passes.
#
#      noEvaluate    - indicates the test should be parsed only and 
#                      not evaluated. 
#                       - This should be used for SNIA extensions only.
#                       - test6noEval is simply to test the test. 
#

TESTLIST = test1 test2 test2a test3 test4 test5 test6 test6not test6noEval test7

ifdef PEGASUS_SNIA_EXTENSIONS
    TESTLIST += test8 test9 test10
else
    TESTLIST += test8not test9not test10not
endif

test1:
	@ $(PROGRAM) "SELECT w,x,y,z FROM ClassName"

test2:
	@ $(PROGRAM) "SELECT w,x,y,z FROM ClassName WHERE w = TRUE OR w = FALSE"

test2a:
	@ $(PROGRAM) "SELECT w,x,y,z FROM ClassName WHERE w = TRUE AND w != FALSE"

test3:
ifeq ($(OS),VMS)
	@ $(PROGRAM) "SELECT w,x,y,z FROM ClassName WHERE w = TRUE AND x >= 10 AND y <= 13.10 AND z = ""Ten""" 
else
	@ $(PROGRAM) "SELECT w,x,y,z FROM ClassName WHERE w = TRUE AND x >= 10 AND y <= 13.10 AND z = \"Ten\"" 
endif

test4:
ifeq ($(OS),VMS)
	@ $(PROGRAM) "SELECT * FROM ClassName WHERE (w = TRUE AND x >= 10 AND y <= 13.10 AND z = ""Ten"") AND NOT w = TRUE IS NOT TRUE"
else
	@ $(PROGRAM) "SELECT * FROM ClassName WHERE (w = TRUE AND x >= 10 AND y <= 13.10 AND z = \"Ten\") AND NOT w = TRUE IS NOT TRUE"
endif

test5:
	@ $(PROGRAM) "SELECT * FROM ClassName WHERE NOT NOT NOT x < 5"

test6:
	@ $(PROGRAM) "SELECT w,x,y,z FROM ClassName WHERE w <> FALSE"

test6not:
	@ $(PROGRAM) "evaluateError" "SELECT w,x,y,z FROM ClassName WHERE w = FALSE"

test6noEval:
	@ $(PROGRAM) "noEvaluate" "SELECT w,x,y,z FROM ClassName WHERE w = FALSE"

test7:
	@ $(PROGRAM) "SELECT * FROM ClassName WHERE v IS NULL"

test8:
	@ $(PROGRAM) "SELECT * FROM CIM_InstCreation WHERE SourceInstance ISA CIM_ComputerSystem"

test8not:
	@ $(PROGRAM) "parseError" "SELECT * FROM CIM_InstCreation WHERE SourceInstance ISA CIM_ComputerSystem"

test9:
	@ $(PROGRAM) "SELECT * FROM CIM_InstCreation WHERE SourceInstance.OperationalStatus <> PreviousInstance.OperationalStatus"

test9not:
	@ $(PROGRAM) "parseError" "SELECT * FROM CIM_InstCreation WHERE SourceInstance.OperationalStatus <> PreviousInstance.OperationalStatus"

test10:
	@ $(PROGRAM) "noEvaluate" "SELECT * FROM CIM_InstModification WHERE SourceInstance ISA CIM_EthernetPort AND SourceInstance.OperationalStatus <> PreviousInstance.OperationalStatus"

test10not:
	@ $(PROGRAM) "parseError" "SELECT * FROM CIM_InstModification WHERE SourceInstance ISA CIM_EthernetPort AND SourceInstance.OperationalStatus <> PreviousInstance.OperationalStatus"

tests: $(TESTLIST)

poststarttests:



