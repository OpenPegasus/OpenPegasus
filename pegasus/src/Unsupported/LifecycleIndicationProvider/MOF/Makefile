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
# Makefile builds a Sample Provider Namespace
# ATTN: Today this makefile is implementation dependent.
#       It assumes that the repository is a set of directories
#       that can be removed by removing the directories.
#       This must be updated to an implementation independent repository
#       remove function.

ROOT = ../../../..
include $(ROOT)/mak/config.mak
include $(ROOT)/mak/configschema.mak

# Loads the sample MOF into a separate namespace.
# The following variable determine the version of the Schema to
# be loaded into the Sample Provider namespace.
# Update the following variable to change the version.

SAMPLEPROVIDERNS=root/SampleProvider
SAMPLEPROVIDERNSDIRECTORY=$(REPOSITORY_ROOT)/root\#SampleProvider

# Load the sample MOF with the local compiler.
repository:
	@ $(ECHO) +++++ Loading TestLifecycleIndication class definitions into $(SAMPLEPROVIDERNS) namespace...
	@ cimmofl "-R$(REPOSITORY_DIR)" "-N$(REPOSITORY_NAME)" "-n$(SAMPLEPROVIDERNS)" LifecycleIndicationProvider.mof
	@ $(ECHO) +++++ Registering Lifecycle Indication Provider...
	@ cimmofl "-R$(REPOSITORY_DIR)" "-N$(REPOSITORY_NAME)" "-n$(INTEROPNS)" LifecycleIndicationProviderR.mof

# Load the sample mof through the Client interface compiler but do not
# clean out the existing version.
repositoryServer:
	@ $(ECHO) +++++ Loading TestLifecycleIndication class definitions into $(SAMPLEPROVIDERNS) namespace...
	@ cimmof "-n$(SAMPLEPROVIDERNS)" LifecycleIndicationProvider.mof
	@ $(ECHO) +++++ Registering Lifecycle Indication Provider...
	@ cimmof "-n$(INTEROPNS)" LifecycleIndicationProviderR.mof

unregister:
	$(MAKE) -i unregisterproviders

unregisterproviders:
	cimprovider -r -m SampleLifecycleIndicationProviderModule

depend:

sub:

misc:

tests:

poststarttests:

general:

clean:
