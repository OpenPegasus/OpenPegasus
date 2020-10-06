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

DIR = Pegasus/ControlProviders/QueryCapabilitiesProvider/tests

include $(ROOT)/mak/config.mak

LIBRARIES = \
	pegclient \
	pegcommon

LOCAL_DEFINES = -DPEGASUS_INTERNALONLY 

PROGRAM = TestCIMQueryCap
SOURCES = TestCIMQueryCap.cpp

include $(ROOT)/mak/program.mak

CIM_SCHEMA_DIR = $(ROOT)/Schemas/CIMPrelim291
ALLOW_EXPERIMENTAL = "-aE"
SAMPLE_PROVIDER_NS = root/SampleProvider

tests:
 
repository:
	@ $(ECHO) +++++ Loading $(CIM_SCHEMA_DIR)/CIM_Interop.mof into root/cimv2 namespace...
	@ cimmofl "-R$(REPOSITORY_DIR)" "-N$(REPOSITORY_NAME)" "-M$(REPOSITORY_MODE)" "-I$(CIM_SCHEMA_DIR)" $(ALLOW_EXPERIMENTAL) $(CIM_SCHEMA_DIR)/CIM_Interop.mof 
	@ $(ECHO) +++++ Loading $(CIM_SCHEMA_DIR)/CIM_Interop.mof into $(SAMPLE_PROVIDER_NS) namespace...
	@ cimmofl "-R$(REPOSITORY_DIR)" "-N$(REPOSITORY_NAME)" "-M$(REPOSITORY_MODE)" "-I$(CIM_SCHEMA_DIR)" $(ALLOW_EXPERIMENTAL) "-n$(SAMPLE_PROVIDER_NS)" $(CIM_SCHEMA_DIR)/CIM_Interop.mof 


repositoryServer:
	@ $(ECHO) +++++ Loading $(CIM_SCHEMA_DIR)/CIM_Interop.mof into root/cimv2 namespace...
	@ cimmof "-I$(CIM_SCHEMA_DIR)" "$(ALLOW_EXPERIMENTAL)" $(CIM_SCHEMA_DIR)/CIM_Interop.mof
	@ $(ECHO) +++++ Loading $(CIM_SCHEMA_DIR)/CIM_Interop.mof into $(SAMPLE_PROVIDER_NS) namespace...
	@ cimmof "-I$(CIM_SCHEMA_DIR)" "$(ALLOW_EXPERIMENTAL)" "-n$(SAMPLE_PROVIDER_NS)" $(CIM_SCHEMA_DIR)/CIM_Interop.mof


poststarttests:
	$(PROGRAM)
