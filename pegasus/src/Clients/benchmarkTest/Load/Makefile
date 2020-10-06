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
include $(ROOT)/mak/configschema.mak
include $(ROOT)/mak/test.mak
include $(ROOT)/mak/commands.mak

ifdef TESTID
   BenchmarkMOF_NAME = benchmarkProvider_$(TESTID)
else
   BenchmarkMOF_NAME = benchmarkProvider
endif

PROVIDERNS=root/benchmark
PROVIDERNSDIRECTORY=$(REPOSITORY_ROOT)/root\#benchmark

CIMMOFL = cimmofl

repository:
	@ $(ECHO) +++++ Loading CIM_Core$(CIM_SCHEMA_VER) into $(PROVIDERNS) namespace ...
	@ $(CIMMOFL) "-R$(REPOSITORY_DIR)" "-N$(REPOSITORY_NAME)" "-M$(REPOSITORY_MODE)" "-I$(CIM_SCHEMA_DIR)" \
              "-n$(PROVIDERNS)" $(ALLOW_EXPERIMENTAL) $(CIM_SCHEMA_DIR)/CIM_Core$(CIM_SCHEMA_VER).mof
	@ $(ECHO) +++++ Loading $(BenchmarkMOF_NAME) class definitions into $(PROVIDERNS) namespace ...
	@ $(CIMMOFL) "-R$(REPOSITORY_DIR)" "-N$(REPOSITORY_NAME)" "-M$(REPOSITORY_MODE)" "-n$(PROVIDERNS)" $(BenchmarkMOF_NAME).mof
	@ $(ECHO) +++++ Registering benchmark Provider   ...
	@ $(CIMMOFL) "-R$(REPOSITORY_DIR)" "-N$(REPOSITORY_NAME)" "-M$(REPOSITORY_MODE)" "-n$(INTEROPNS)" $(BenchmarkMOF_NAME)R.mof

repositoryServer:
	@ $(ECHO) +++++ Loading CIM_Core$(CIM_SCHEMA_VER) into $(PROVIDERNS) namespace ...
	@ cimmof "-I$(CIM_SCHEMA_DIR)" "-n$(PROVIDERNS)" $(ALLOW_EXPERIMENTAL) $(CIM_SCHEMA_DIR)/CIM_Core$(CIM_SCHEMA_VER).mof
	@ $(ECHO) +++++ Loading $(BenchmarkMOF_NAME) class definitions into $(PROVIDERNS) namespace ...
	@ cimmof "-n$(PROVIDERNS)" $(BenchmarkMOF_NAME).mof
	@ $(ECHO) +++++ Registering benchmark Provider   ...
	@ cimmof "-n$(INTEROPNS)" $(BenchmarkMOF_NAME)R.mof

registerproviders:
	@ $(ECHO) +++++ Registering benchmark Provider   ...
	@ cimmof "-n$(INTEROPNS)" benchmarkProviderR.mof

unregister:
	$(MAKE) -i unregisterproviders

unregisterproviders:
	cimprovider -r -m benchmarkProviderModule

changeconfig:
	$(MAKE) -i cimstart
	$(MAKE) unregister
ifdef TESTID
	$(MAKE) repositoryServer TESTID=$(TESTID)
else
	$(MAKE) repositoryServer
endif


depend:

sub:

misc:

tests:

poststarttests:

general:

clean:

