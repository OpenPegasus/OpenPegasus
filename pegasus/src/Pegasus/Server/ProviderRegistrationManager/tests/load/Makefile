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
# Makefile builds a test repository in a specified directory.

ROOT = ../../../../../..
include $(ROOT)/mak/config.mak

ifndef REPOSITORYDIR
    $(error REPOSITORYDIR must be defined)
endif

ifeq ($(PEGASUS_INTEROP_NAMESPACE),root/interop)
NAMESPACENAME=root/interop
else
ifeq ($(PEGASUS_INTEROP_NAMESPACE),interop)
NAMESPACENAME=interop
else
NAMESPACENAME=root/PG_InterOp
endif
endif

# Update the following two environment variables to change the version
# of the PG_Schema to be loaded in the namespace.

PG_SCHEMA_VER=20
PG_SCHEMA_DIR=VER$(PG_SCHEMA_VER)
PG_MOF_PATH = $(ROOT)/Schemas/Pegasus/InterOp/$(PG_SCHEMA_DIR)

all:

# Delete and rebuild the repository using the MOF compiler.

localrep: clean
	@ $(ECHO) +++++ Loading CIM_Core$(CIM_SCHEMA_VER) into $(NAMESPACENAME) namespace ...
	@ cimmofl "-I$(CIM_SCHEMA_DIR)" "-R$(REPOSITORYDIR)" "-M$(REPOSITORY_MODE)" "-n$(NAMESPACENAME)" $(ALLOW_EXPERIMENTAL) $(CIM_SCHEMA_DIR)/CIM_Core$(CIM_SCHEMA_VER).mof
	@ $(ECHO) +++++ Loading PG_ProviderModule$(PG_SCHEMA_VER) into $(NAMESPACENAME) namespace ...
	@ cimmofl "-I$(PG_MOF_PATH)" "-R$(REPOSITORYDIR)" "-M$(REPOSITORY_MODE)" "-n$(NAMESPACENAME)" $(PG_MOF_PATH)/PG_ProviderModule$(PG_SCHEMA_VER).mof

clean:
	@ $(ECHO) +++++ Removing CIM Repository $(REPOSITORYDIR)/repository
	@ $(RMREPOSITORY) $(REPOSITORYDIR)/repository

depend:

sub:

misc:

tests:

poststarttests:

general:
