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
DIR = Pegasus/IndicationService/tests/ProcessIndication
include $(ROOT)/mak/config.mak
include ../libraries.mak

EXTRA_INCLUDES = $(SYS_INCLUDES)

LOCAL_DEFINES = -DPEGASUS_INTERNALONLY -DINDICATION_DIR=\"$(DISPLAYCONSUMER_DIR)\"

PROGRAM = TestProcessIndication

SOURCES = ProcessIndication.cpp

ifeq ($(OS),VMS)
LOCAL_DEFINES = -DINDICATION_DIR="""$(DISPLAYCONSUMER_DIR)""" -DPEGASUS_INTERNALONLY
endif

include $(ROOT)/mak/program.mak

tests:

poststarttests:
	@$(ECHO) "TestProcessIndication setup WQL"
	@$(PROGRAM) setup WQL
	@$(ECHO) "TestProcessIndication create1 WQL"
	@$(PROGRAM) create1 WQL
	@$(ECHO) "TestProcessIndication sendNormal WQL"
	@$(PROGRAM) sendNormal WQL
	@$(ECHO) "TestProcessIndication checkNormal WQL"
	@$(PROGRAM) checkNormal WQL
	@$(ECHO) "TestProcessIndication sendSubclass WQL"
	@$(PROGRAM) sendSubclass WQL
	@$(ECHO) "TestProcessIndication checkSubclass WQL"
	@$(PROGRAM) checkSubclass WQL
	@$(ECHO) "TestProcessIndication sendMissing WQL"
	@$(PROGRAM) sendMissing WQL
	@$(ECHO) "TestProcessIndication checkMissing WQL"
	@$(PROGRAM) checkMissing WQL
	@$(ECHO) "TestProcessIndication sendExtra WQL"
	@$(PROGRAM) sendExtra WQL
	@$(ECHO) "TestProcessIndication checkExtra WQL"
	@$(PROGRAM) checkExtra WQL
	@$(ECHO) "TestProcessIndication sendMatching WQL"
	@$(PROGRAM) sendMatching WQL
	@$(ECHO) "TestProcessIndication checkMatching WQL"
	@$(PROGRAM) checkMatching WQL
	@$(ECHO) "TestProcessIndication sendUnmatchingNamespace WQL"
	@$(PROGRAM) sendUnmatchingNamespace WQL
	@$(ECHO) "TestProcessIndication checkUnmatchingNamespace WQL"
	@$(PROGRAM) checkUnmatchingNamespace WQL
	@$(ECHO) "TestProcessIndication sendUnmatchingClassName WQL"
	@$(PROGRAM) sendUnmatchingClassName WQL
	@$(ECHO) "TestProcessIndication checkUnmatchingClassName WQL"
	@$(PROGRAM) checkUnmatchingClassName WQL
	@$(ECHO) "TestProcessIndication delete1 WQL"
	@$(PROGRAM) delete1 WQL
	@$(ECHO) "TestProcessIndication create2 WQL"
	@$(PROGRAM) create2 WQL
	@$(ECHO) "TestProcessIndication sendNormal WQL"
	@$(PROGRAM) sendNormal WQL
	@$(ECHO) "TestProcessIndication checkNormalAll WQL"
	@$(PROGRAM) checkNormalAll WQL
	@$(ECHO) "TestProcessIndication sendMissing WQL"
	@$(PROGRAM) sendMissing WQL
	@$(ECHO) "TestProcessIndication checkMissingAll WQL"
	@$(PROGRAM) checkMissingAll WQL
	@$(ECHO) "TestProcessIndication sendExtra WQL"
	@$(PROGRAM) sendExtra WQL
	@$(ECHO) "TestProcessIndication checkExtraAll WQL"
	@$(PROGRAM) checkExtraAll WQL
	@$(ECHO) "TestProcessIndication delete2 WQL"
	@$(PROGRAM) delete2 WQL
	@$(ECHO) "TestProcessIndication cleanup WQL"
	@$(PROGRAM) cleanup WQL
	@$(ECHO) "TestProcessIndication setup DMTF:CQL"
	@$(PROGRAM) setup DMTF:CQL
	@$(ECHO) "TestProcessIndication create1 DMTF:CQL"
	@$(PROGRAM) create1 DMTF:CQL
	@$(ECHO) "TestProcessIndication sendNormal DMTF:CQL"
	@$(PROGRAM) sendNormal DMTF:CQL
	@$(ECHO) "TestProcessIndication checkNormal DMTF:CQL"
	@$(PROGRAM) checkNormal DMTF:CQL
	@$(ECHO) "TestProcessIndication sendSubclass DMTF:CQL"
	@$(PROGRAM) sendSubclass DMTF:CQL
	@$(ECHO) "TestProcessIndication checkSubclass DMTF:CQL"
	@$(PROGRAM) checkSubclass DMTF:CQL
	@$(ECHO) "TestProcessIndication sendMissing DMTF:CQL"
	@$(PROGRAM) sendMissing DMTF:CQL
	@$(ECHO) "TestProcessIndication checkMissing DMTF:CQL"
	@$(PROGRAM) checkMissing DMTF:CQL
	@$(ECHO) "TestProcessIndication sendExtra DMTF:CQL"
	@$(PROGRAM) sendExtra DMTF:CQL
	@$(ECHO) "TestProcessIndication checkExtra DMTF:CQL"
	@$(PROGRAM) checkExtra DMTF:CQL
	@$(ECHO) "TestProcessIndication sendMatching DMTF:CQL"
	@$(PROGRAM) sendMatching DMTF:CQL
	@$(ECHO) "TestProcessIndication checkMatching DMTF:CQL"
	@$(PROGRAM) checkMatching DMTF:CQL
	@$(ECHO) "TestProcessIndication sendUnmatchingNamespace DMTF:CQL"
	@$(PROGRAM) sendUnmatchingNamespace DMTF:CQL
	@$(ECHO) "TestProcessIndication checkUnmatchingNamespace DMTF:CQL"
	@$(PROGRAM) checkUnmatchingNamespace DMTF:CQL
	@$(ECHO) "TestProcessIndication sendUnmatchingClassName DMTF:CQL"
	@$(PROGRAM) sendUnmatchingClassName DMTF:CQL
	@$(ECHO) "TestProcessIndication checkUnmatchingClassName DMTF:CQL"
	@$(PROGRAM) checkUnmatchingClassName DMTF:CQL
	@$(ECHO) "TestProcessIndication delete1 DMTF:CQL"
	@$(PROGRAM) delete1 DMTF:CQL
	@$(ECHO) "TestProcessIndication create2 DMTF:CQL"
	@$(PROGRAM) create2 DMTF:CQL 
	@$(ECHO) "TestProcessIndication sendNormal DMTF:CQL"
	@$(PROGRAM) sendNormal DMTF:CQL
	@$(ECHO) "TestProcessIndication checkNormalAll DMTF:CQL"
	@$(PROGRAM) checkNormalAll DMTF:CQL
	@$(ECHO) "TestProcessIndication sendMissing DMTF:CQL"
	@$(PROGRAM) sendMissing DMTF:CQL
	@$(ECHO) "TestProcessIndication checkMissingAll DMTF:CQL"
	@$(PROGRAM) checkMissingAll DMTF:CQL
	@$(ECHO) "TestProcessIndication sendExtra DMTF:CQL"
	@$(PROGRAM) sendExtra DMTF:CQL
	@$(ECHO) "TestProcessIndication checkExtraAll DMTF:CQL"
	@$(PROGRAM) checkExtraAll DMTF:CQL
	@$(ECHO) "TestProcessIndication delete2 DMTF:CQL"
	@$(PROGRAM) delete2 DMTF:CQL
	@$(ECHO) "TestProcessIndication create3 DMTF:CQL"
	@$(PROGRAM) create3 DMTF:CQL
	@$(ECHO) "TestProcessIndication sendNormal DMTF:CQL"
	@$(PROGRAM) sendNormal DMTF:CQL
	@$(ECHO) "TestProcessIndication checkNormalWhere DMTF:CQL"
	@$(PROGRAM) checkNormalWhere DMTF:CQL
	@$(ECHO) "TestProcessIndication sendMissing DMTF:CQL"
	@$(PROGRAM) sendMissing DMTF:CQL
	@$(ECHO) "TestProcessIndication checkMissingWhere DMTF:CQL"
	@$(PROGRAM) checkMissingWhere DMTF:CQL
	@$(ECHO) "TestProcessIndication delete3 DMTF:CQL"
	@$(PROGRAM) delete3 DMTF:CQL
	@$(ECHO) "TestProcessIndication create4 DMTF:CQL"
	@$(PROGRAM) create4 DMTF:CQL
	@$(ECHO) "TestProcessIndication sendNormal DMTF:CQL"
	@$(PROGRAM) sendNormal DMTF:CQL
	@$(ECHO) "TestProcessIndication checkNormalWhereNotSatisfied DMTF:CQL"
	@$(PROGRAM) checkNormalWhereNotSatisfied DMTF:CQL
	@$(ECHO) "TestProcessIndication sendMissing DMTF:CQL"
	@$(PROGRAM) sendMissing DMTF:CQL
	@$(ECHO) "TestProcessIndication checkMissingWhere DMTF:CQL"
	@$(PROGRAM) checkMissingWhere DMTF:CQL
	@$(ECHO) "TestProcessIndication delete4 DMTF:CQL"
	@$(PROGRAM) delete4 DMTF:CQL
	@$(ECHO) "TestProcessIndication cleanup DMTF:CQL"
	@$(PROGRAM) cleanup DMTF:CQL
	@$(ECHO) "+++++ TestProcessIndication passed all tests"

IndInit_TS1a:
	@$(ECHO) "TestProcessIndication setup WQL"
	@$(PROGRAM) setup WQL
	@$(ECHO) "TestProcessIndication create1 WQL"
	@$(PROGRAM) create1 WQL
	@$(ECHO) "TestProcessIndication create2 WQL"
	@$(PROGRAM) create2 WQL

IndInit_TS1b:
	@$(ECHO) "TestProcessIndication delete1 WQL"
	@$(PROGRAM) delete1 WQL
	@$(ECHO) "TestProcessIndication delete2 WQL"
	@$(PROGRAM) delete2 WQL
	@$(ECHO) "TestProcessIndication cleanup WQL"
	@$(PROGRAM) cleanup WQL
