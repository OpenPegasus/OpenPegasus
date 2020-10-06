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
# Makefile builds the Pegasus InterOp Namespace
# Note: For best results, ensure that the InterOp namespace does not
#       exist before calling this Makefile.

ROOT = ../../..
include $(ROOT)/mak/config.mak
include $(ROOT)/mak/configschema.mak

PG_SCHEMA_VER=20
PG_SCHEMA_DIR=VER$(PG_SCHEMA_VER)
PG_COMPUTERSYSTEM_MOF_PATH=$(ROOT)/Schemas/Pegasus/ManagedSystem/$(PG_SCHEMA_DIR)

all:

# Delete and rebuild the repository using the MOF compiler.

repository:
	@$(ECHO) +++++ Loading CIM_Core$(CIM_SCHEMA_VER) into $(INTEROPNS) namespace ...
	@$(CIMMOFCLI) "-R$(REPOSITORY_DIR)" "-N$(REPOSITORY_NAME)" "-M$(REPOSITORY_MODE)" "-I$(CIM_SCHEMA_DIR)" "-n$(INTEROPNS)" $(ALLOW_EXPERIMENTAL) "$(CIM_SCHEMA_DIR)/CIM_Core$(CIM_SCHEMA_VER).mof"
	@$(ECHO) +++++ Loading CIM_Event$(CIM_SCHEMA_VER) into $(INTEROPNS) namespace ...
	@$(CIMMOFCLI) "-R$(REPOSITORY_DIR)" "-N$(REPOSITORY_NAME)" "-M$(REPOSITORY_MODE)" "-I$(CIM_SCHEMA_DIR)" "-n$(INTEROPNS)" $(ALLOW_EXPERIMENTAL) "$(CIM_SCHEMA_DIR)/CIM_Event$(CIM_SCHEMA_VER).mof"
#
ifeq ($(PEGASUS_ENABLE_INTEROP_PROVIDER),true)
	@$(ECHO) +++++ Loading CIM_Interop$(CIM_SCHEMA_VER) into $(INTEROPNS) namespace ...
	@$(CIMMOFCLI) "-R$(REPOSITORY_DIR)" "-N$(REPOSITORY_NAME)" "-M$(REPOSITORY_MODE)" "-I$(CIM_SCHEMA_DIR)" "-n$(INTEROPNS)" $(ALLOW_EXPERIMENTAL) "$(CIM_SCHEMA_DIR)/CIM_Interop$(CIM_SCHEMA_VER).mof" 
endif
#
	@$(ECHO) +++++ Loading PG_InterOpSchema$(PG_INTEROP_SCHEMA_VER).mof into $(INTEROPNS) namespace...
	@$(CIMMOFCLI) "-R$(REPOSITORY_DIR)" "-N$(REPOSITORY_NAME)" "-M$(REPOSITORY_MODE)" "-I$(PG_INTEROP_MOF_PATH)" "-n$(INTEROPNS)" "$(PG_INTEROP_MOF_PATH)/PG_InterOpSchema$(PG_INTEROP_SCHEMA_VER).mof"
#
# Bug 1222
# The following code is temporary.  These classes should be incorporated into PG_InteropSchema
# however, this cannot be done now because we cannot conditionally compile MOF.	
ifeq ($(PEGASUS_ENABLE_INTEROP_PROVIDER),true)
	@$(ECHO) +++++ Loading PG_CIMXMLCommunicationMechanism$(PG_INTEROP_SCHEMA_VER) into $(INTEROPNS) namespace ...
	@$(CIMMOFCLI) "-R$(REPOSITORY_DIR)" "-N$(REPOSITORY_NAME)" "-M$(REPOSITORY_MODE)" "-I$(PG_INTEROP_MOF_PATH)" "-n$(INTEROPNS)" $(ALLOW_EXPERIMENTAL) "$(PG_INTEROP_MOF_PATH)/PG_CIMXMLCommunicationMechanism$(PG_INTEROP_SCHEMA_VER).mof" 
	@$(ECHO) +++++ Loading PG_Namespace$(PG_INTEROP_SCHEMA_VER) into $(INTEROPNS) namespace ...
	@$(CIMMOFCLI) "-R$(REPOSITORY_DIR)" "-N$(REPOSITORY_NAME)" "-M$(REPOSITORY_MODE)" "-I$(PG_INTEROP_MOF_PATH)" "-n$(INTEROPNS)" $(ALLOW_EXPERIMENTAL) "$(PG_INTEROP_MOF_PATH)/PG_Namespace$(PG_INTEROP_SCHEMA_VER).mof"
	@$(ECHO) +++++ Loading Computer System schema into $(INTEROPNS) namespace ...
	@$(CIMMOFCLI) "-R$(REPOSITORY_DIR)" "-N$(REPOSITORY_NAME)" "-M$(REPOSITORY_MODE)" "-I$(PG_COMPUTERSYSTEM_MOF_PATH)" "-n$(INTEROPNS)" $(ALLOW_EXPERIMENTAL) "$(PG_COMPUTERSYSTEM_MOF_PATH)/PG_ComputerSystem$(PG_SCHEMA_VER).mof"
	@$(ECHO) +++++ Loading PG_ServerProfile schema into $(INTEROPNS) namespace ...
	@$(CIMMOFCLI) "-R$(REPOSITORY_DIR)" "-N$(REPOSITORY_NAME)" "-M$(REPOSITORY_MODE)" "-I$(PG_INTEROP_MOF_PATH)" "-n$(INTEROPNS)" $(ALLOW_EXPERIMENTAL) "$(PG_INTEROP_MOF_PATH)/PG_ServerProfile$(PG_INTEROP_SCHEMA_VER).mof"

endif

repositoryServer:
	@$(ECHO) +++++ Loading CIM_Core$(CIM_SCHEMA_VER) into $(INTEROPNS) namespace ...
	@cimmof "-I$(CIM_SCHEMA_DIR)" "-n$(INTEROPNS)" $(ALLOW_EXPERIMENTAL) "$(CIM_SCHEMA_DIR)/CIM_Core$(CIM_SCHEMA_VER).mof"
	@$(ECHO) +++++ Loading CIM_Event$(CIM_SCHEMA_VER) into $(INTEROPNS) namespace ...
	@cimmof "-I$(CIM_SCHEMA_DIR)" "-n$(INTEROPNS)" $(ALLOW_EXPERIMENTAL) "$(CIM_SCHEMA_DIR)/CIM_Event$(CIM_SCHEMA_VER).mof"

ifeq ($(PEGASUS_ENABLE_INTEROP_PROVIDER),true)
	@$(ECHO) +++++ Loading CIM_Interop$(CIM_SCHEMA_VER) into $(INTEROPNS) namespace ...
	@cimmof "-I$(CIM_SCHEMA_DIR)" "-n$(INTEROPNS)" $(ALLOW_EXPERIMENTAL) "$(CIM_SCHEMA_DIR)/CIM_Interop$(CIM_SCHEMA_VER).mof"
	@$(ECHO) +++++ Loading Computer System schema into $(INTEROPNS) namespace ...
	@$(CIMMOFCLI) "-R$(REPOSITORY_DIR)" "-N$(REPOSITORY_NAME)" "-M$(REPOSITORY_MODE)" "-I$(PG_COMPUTERSYSTEM_MOF_PATH)" "-n$(INTEROPNS)" $(ALLOW_EXPERIMENTAL) "$(PG_COMPUTERSYSTEM_MOF_PATH)/PG_ComputerSystem$(PG_SCHEMA_VER).mof"
endif

	@$(ECHO) +++++ Loading PG_InterOpSchema$(PG_INTEROP_SCHEMA_VER).mof into $(INTEROPNS) namespace...
	@cimmof "-I$(PG_INTEROP_MOF_PATH)" "-n$(INTEROPNS)" "$(PG_INTEROP_MOF_PATH)/PG_InterOpSchema$(PG_INTEROP_SCHEMA_VER).mof"
#
# Bug 1222
# The following code is temporary.  Eventually this class should be incorporated into PG_InteropSchema
# however, this cannot be done now because we cannot conditionally compile MOF.
ifeq ($(PEGASUS_ENABLE_INTEROP_PROVIDER),true)	
	@$(ECHO) +++++ Loading Loading PG_CIMXMLCommunicationMechanism$(PG_INTEROP_SCHEMA_VER) into $(INTEROPNS) namespace ...
	@cimmof "-I$(PG_INTEROP_MOF_PATH)" "-n$(INTEROPNS)" $(ALLOW_EXPERIMENTAL) "$(PG_INTEROP_MOF_PATH)/PG_CIMXMLCommunicationMechanism$(PG_INTEROP_SCHEMA_VER).mof"
	@$(ECHO) +++++ Loading Loading PG_Namespace$(PG_INTEROP_SCHEMA_VER) into $(INTEROPNS) namespace ...
	@cimmof "-I$(PG_INTEROP_MOF_PATH)" "-n$(INTEROPNS)" $(ALLOW_EXPERIMENTAL) "$(PG_INTEROP_MOF_PATH)/PG_Namespace$(PG_INTEROP_SCHEMA_VER).mof"
	@$(ECHO) +++++ Loading PG_ServerProfile schema into $(INTEROPNS) namespace ...
	@cimmof "-I$(PG_INTEROP_MOF_PATH)" "-n$(INTEROPNS)" $(ALLOW_EXPERIMENTAL) "$(PG_INTEROP_MOF_PATH)/PG_ServerProfile$(PG_INTEROP_SCHEMA_VER).mof"
endif


clean:

depend:

sub:

misc:

tests:

poststarttests:

general:
