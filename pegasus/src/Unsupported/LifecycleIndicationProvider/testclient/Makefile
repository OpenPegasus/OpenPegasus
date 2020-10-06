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
DIR = Unsupported/LifecycleIndicationProviders/testclient
include $(ROOT)/mak/config.mak

PEGASUS_ZOS_PROGRAM_OBJECT = yes
LIBRARIES = \
     pegclient \
     pegcommon

EXTRA_INCLUDES = $(SYS_INCLUDES)

LOCAL_DEFINES = -DINDICATION_DIR=\"$(DISPLAYCONSUMER_DIR)\"

PROGRAM = TestLifecycleIndication

SOURCES = LifecycleIndication.cpp

SAMPLEPROVIDERNS="root/SampleProvider"
SAMPLECLASS="Sample_LifecycleIndicationProviderClass"
	  
include $(ROOT)/mak/program.mak

tests:

poststarttests:
	@$(ECHO) "TestLifecycleIndication setup WQL"
	@$(PROGRAM) setup WQL
	@$(ECHO) "TestLifecycleIndication createInstance"
	@$(PROGRAM) createInstance
	@$(ECHO) "TestLifecycleIndication createInstance"
	@$(PROGRAM) createInstance
	@$(ECHO) "TestLifecycleIndication createInstance"
	@$(PROGRAM) createInstance
	@$(ECHO) "How many instances exist? Lets enumerate them..."
	-cimcli ei -n $(SAMPLEPROVIDERNS) $(SAMPLECLASS)
	@$(ECHO) "TestLifecycleIndication deleteInstance"
	@$(PROGRAM) deleteInstance
	@$(ECHO) "TestLifecycleIndication deleteInstance"
	@$(PROGRAM) deleteInstance
	@$(ECHO) "We've deleted two instances, should just be one left..."
	-cimcli ei -n $(SAMPLEPROVIDERNS) $(SAMPLECLASS)
	@$(ECHO) "TestLifecycleIndication deleteInstance"
	@$(PROGRAM) deleteInstance
	@$(ECHO) "TestLifecycleIndication cleanup"
	@$(PROGRAM) cleanup
	@$(ECHO) "TestLifecycleIndication setup CIM:CQL"
	@$(PROGRAM) setup CIM:CQL
	@$(ECHO) "TestLifecycleIndication createInstance"
	@$(PROGRAM) createInstance
	@$(ECHO) "TestLifecycleIndication createInstance"
	@$(PROGRAM) createInstance
	@$(ECHO) "TestLifecycleIndication createInstance"
	@$(PROGRAM) createInstance
	@$(ECHO) "How many instances exist? Lets enumerate them..."
	-cimcli ei -n $(SAMPLEPROVIDERNS) $(SAMPLECLASS)
	@$(ECHO) "TestLifecycleIndication deleteInstance"
	@$(PROGRAM) deleteInstance
	@$(ECHO) "TestLifecycleIndication deleteInstance"
	@$(PROGRAM) deleteInstance
	@$(ECHO) "We've deleted two instances, should just be one left..."
	-cimcli ei -n $(SAMPLEPROVIDERNS) $(SAMPLECLASS)
	@$(ECHO) "TestLifecycleIndication deleteInstance"
	@$(PROGRAM) deleteInstance
	@$(ECHO) "TestLifecycleIndication cleanup"
	@$(PROGRAM) cleanup
	@$(ECHO) "+++++ TestLifecycleIndication passed all tests"
