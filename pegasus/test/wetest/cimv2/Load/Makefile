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
# Makefile builds a test cimv2 namespace
# Note: For best results, ensure that the test/cimv2 namespace does not
#       exist before calling this Makefile.

ROOT = ../../../..
include $(ROOT)/mak/config.mak
include $(ROOT)/mak/configschema.mak

# The following variable determine the version of the Schema to
# be loaded into the test cimv2 namespace.
# Update the following variable to change the version.

TESTCIMV2NS=test/cimv2

CIM_SCHEMA_VER=25
CIM_SCHEMA_DIR=$(PEGASUS_ROOT)/Schemas/CIM25

# ***** CIM_SCHEMA_DIR INFO ****
# If CIM_SCHEMA_DIR changes to use a preliminary schema which
# has experimentals make sure and change the path below to appopriate
# directory path.  Example:  CIMPrelim271 is preliminary and has
# experimental classes.  Since experimental classes exist the -aE
# option of the mof compiler needs to be set.
# *****
ifeq ($(CIM_SCHEMA_DIR), $(PEGASUS_ROOT)/Schemas/CIMPrelim271)
ALLOW_EXPERIMENTAL = "-aE"
else
ALLOW_EXPERIMENTAL =
endif

repository:
	@ $(ECHO) +++++ Loading CIM_Schema$(CIM_SCHEMA_VER) into $(TESTCIMV2NS) namespace ...
	@ cimmofl "-R$(REPOSITORY_DIR)" "-N$(REPOSITORY_NAME)" "-M$(REPOSITORY_MODE)" "-I$(CIM_SCHEMA_DIR)" -n$(TESTCIMV2NS) $(ALLOW_EXPERIMENTAL) $(CIM_SCHEMA_DIR)/CIM_Schema$(CIM_SCHEMA_VER).mof

repositoryServer:
	@ $(ECHO) +++++ Loading CIM_Schema$(CIM_SCHEMA_VER) into $(TESTCIMV2NS) namespace ...
	@ cimmof "-I$(CIM_SCHEMA_DIR)" -n$(TESTCIMV2NS) $(ALLOW_EXPERIMENTAL) $(CIM_SCHEMA_DIR)/CIM_Schema$(CIM_SCHEMA_VER).mof

clean:

depend:

sub:

misc:

tests:

messages:

poststarttests:

general:
