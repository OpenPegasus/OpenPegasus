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
#
# Makefile for loading for all the MOF files for the test enhancement providers
# present in this directory. It creates a new namespace called SampleProvider/
# inside $PEGASUS_HOME/repository.
# The implementation of sample providers and classes registered by this
# Makefile are present in the following directories:
#
# 1. ExceptionsTest/
# 2. CIMOMSample/
# 3. LargeDataProvider/
# 4. MultithreadingTests/
# 5. TimingTests/
# 6. XMLParserGen/
#

ROOT = ../../..

include $(ROOT)/mak/config.mak
include $(ROOT)/mak/configschema.mak

# Loads the sample MOF into a separate namespace.

SAMPLEPROVIDERNS=root/SampleProvider

# Load the sample MOF with the local compiler. Cleans the existing repository
# before reloading.

repository:
	@ $(ECHO) +++++ Loading CIM_Core$(CIM_SCHEMA_VER) into $(SAMPLEPROVIDERNS) namespace ...
	@ cimmofl -R$(REPOSITORY_DIR) -N$(REPOSITORY_NAME) -M$(REPOSITORY_MODE) -I$(CIM_SCHEMA_DIR) -n$(SAMPLEPROVIDERNS) $(ALLOW_EXPERIMENTAL) $(CIM_SCHEMA_DIR)/CIM_Core$(CIM_SCHEMA_VER).mof
	@ $(ECHO) +++++ Loading CIM_Event$(CIM_SCHEMA_VER) into $(SAMPLEPROVIDERNS) namespace ...
	@ cimmofl -R$(REPOSITORY_DIR) -N$(REPOSITORY_NAME) -M$(REPOSITORY_MODE) -I$(CIM_SCHEMA_DIR) -n$(SAMPLEPROVIDERNS) $(ALLOW_EXPERIMENTAL) $(CIM_SCHEMA_DIR)/CIM_Event$(CIM_SCHEMA_VER).mof

	@ $(ECHO) +++++ Loading All Sample Providers class definitions into $(SAMPLEPROVIDERNS) namespace ...
	@ cimmofl -R$(REPOSITORY_DIR) -N$(REPOSITORY_NAME) -M$(REPOSITORY_MODE) -n$(SAMPLEPROVIDERNS)	CIMOMSampleProvider.mof
	@ cimmofl -R$(REPOSITORY_DIR) -N$(REPOSITORY_NAME) -M$(REPOSITORY_MODE) -n$(SAMPLEPROVIDERNS) LargeDataProvider.mof
	@ cimmofl -R$(REPOSITORY_DIR) -N$(REPOSITORY_NAME) -M$(REPOSITORY_MODE) -n$(SAMPLEPROVIDERNS) MultithreadingSampleProvider.mof
	@ cimmofl -R$(REPOSITORY_DIR) -N$(REPOSITORY_NAME) -M$(REPOSITORY_MODE) -n$(SAMPLEPROVIDERNS) ExceptionsTest.mof
	@ cimmofl -R$(REPOSITORY_DIR) -N$(REPOSITORY_NAME) -M$(REPOSITORY_MODE) -n$(SAMPLEPROVIDERNS) TimingProvider.mof

	@ $(ECHO) +++++ Registering All Sample Providers ....
	@ cimmofl -R$(REPOSITORY_DIR) -N$(REPOSITORY_NAME) -M$(REPOSITORY_MODE) -n$(INTEROPNS) CIMOMSampleProviderR.mof
	@ cimmofl -R$(REPOSITORY_DIR) -N$(REPOSITORY_NAME) -M$(REPOSITORY_MODE) -n$(INTEROPNS) LargeDataProviderR.mof
	@ cimmofl -R$(REPOSITORY_DIR) -N$(REPOSITORY_NAME) -M$(REPOSITORY_MODE) -n$(INTEROPNS) MultithreadingSampleProviderR.mof
	@ cimmofl -R$(REPOSITORY_DIR) -N$(REPOSITORY_NAME) -M$(REPOSITORY_MODE) -n$(INTEROPNS) ExceptionsTestR.mof
	@ cimmofl -R$(REPOSITORY_DIR) -N$(REPOSITORY_NAME) -M$(REPOSITORY_MODE) -n$(INTEROPNS) TimingProviderR.mof

# Load the sample mofe through the Client interface compiler but do not
# clean out the existing version.
repositoryServer:
	@ $(ECHO) +++++ Loading CIM_Core$(CIM_SCHEMA_VER) into $(SAMPLEPROVIDERNS) namespace ...
	@ cimmof -I$(CIM_SCHEMA_DIR) -n$(SAMPLEPROVIDERNS) $(ALLOW_EXPERIMENTAL) $(CIM_SCHEMA_DIR)/CIM_Core$(CIM_SCHEMA_VER).mof
	@ $(ECHO) +++++ Loading CIM_Event$(CIM_SCHEMA_VER) into $(SAMPLEPROVIDERNS) namespace ...
	@ cimmof -I$(CIM_SCHEMA_DIR) -n$(SAMPLEPROVIDERNS) $(ALLOW_EXPERIMENTAL) $(CIM_SCHEMA_DIR)/CIM_Event$(CIM_SCHEMA_VER).mof
	@ $(ECHO) +++++ Loading All the Sample Providers class definitions into $(SAMPLEPROVIDERNS) namespace ...
	@ cimmof -n$(SAMPLEPROVIDERNS) CIMOMSampleProvider.mof
	@ cimmof -n$(SAMPLEPROVIDERNS) LargeDataProvider.mof
	@ cimmof -n$(SAMPLEPROVIDERNS) MultithreadingSampleProvider.mof
	@ cimmof -n$(SAMPLEPROVIDERNS) ExceptionsTest.mof
	@ cimmof -n$(SAMPLEPROVIDERNS) TimingProvider.mof

	@ $(ECHO) +++++ Registering All Sample Providers Schema Providers  ...
	@ cimmof -n$(INTEROPNS)	CIMOMSampleProviderR.mof
	@ cimmof -n$(INTEROPNS) LargeDataProviderR.mof
	@ cimmof -n$(INTEROPNS) MultithreadingSampleProviderR.mof
	@ cimmof -n$(INTEROPNS) ExceptionsTestR.mof
	@ cimmof -n$(INTEROPNS) TimingProviderR.mof

unregister:
	$(MAKE) -i unregisterproviders

unregisterproviders:
	cimprovider -r -m ExceptionsTestModule
	cimprovider -r -m LargeDataProviderModule
	cimprovider -r -m CIMOMSampleProviderModule
	cimprovider -r -m MultithreadingSampleProviderModule
	cimprovider -r -m TimingProviderModule

depend:

sub:

misc:

tests:

messages:

poststarttests:

general:

clean:
