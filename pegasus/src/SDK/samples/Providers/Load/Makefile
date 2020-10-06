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

ROOT = ../..
include $(ROOT)/mak/config.mak

ifeq ($(PEGASUS_INTEROP_NAMESPACE),root/interop)
INTEROPNS=root/interop
else
ifeq ($(PEGASUS_INTEROP_NAMESPACE),interop)
INTEROPNS=interop
else
INTEROPNS=root/PG_InterOp
endif
endif

SAMPLECXXPROVIDERNS=SDKExamples/DefaultCXX
SAMPLECMPIPROVIDERNS=SDKExamples/CMPI

createCXXrepository:
	@$(ECHO) "+++++ Creating $(SAMPLECXXPROVIDERNS) namespace ..."
	@$(ECHO) "+++++ Loading SDK_CIM_Schema.mof into" \
            "$(SAMPLECXXPROVIDERNS) namespace ..."
	@$(PEGASUS_BIN_DIR)/cimmof -I$(PEGASUS_CIM_SCHEMA_DIR) \
            -n$(SAMPLECXXPROVIDERNS) SDK_CIM_Schema.mof
	@ $(ECHO) "+++++ Loading class definitions into " \
            "$(SAMPLECXXPROVIDERNS) namespace ..."
	@ $(PEGASUS_BIN_DIR)/cimmof -n$(SAMPLECXXPROVIDERNS) \
             SampleProviderSchema.mof

createCMPIrepository:
	@$(ECHO) "+++++ Creating $(SAMPLECMPIPROVIDERNS) namespace ..."
	@$(ECHO) "+++++ Loading SDK_CIM_Schema.mof into" \
            "$(SAMPLECMPIPROVIDERNS) namespace ..."
	@$(PEGASUS_BIN_DIR)/cimmof -I$(PEGASUS_CIM_SCHEMA_DIR) \
            -n$(SAMPLECMPIPROVIDERNS) SDK_CIM_Schema.mof
	@ $(PEGASUS_BIN_DIR)/cimmof -n$(SAMPLECMPIPROVIDERNS) CWS_FilesAndDir.mof

createrepository: createCXXrepository createCMPIrepository
	@$(ECHO) "+++++ Repository created."

registerproviders:
	@ $(ECHO) "+++++ Registering providers for" \
            "$(SAMPLECXXPROVIDERNS) namespace  ..."
	@ $(PEGASUS_BIN_DIR)/cimmof -n$(INTEROPNS) InstanceProviderR.mof
	@ $(PEGASUS_BIN_DIR)/cimmof -n$(INTEROPNS) MethodProviderR.mof
	@ $(PEGASUS_BIN_DIR)/cimmof -n$(INTEROPNS) IndicationProviderR.mof
	@ $(PEGASUS_BIN_DIR)/cimmof -n$(INTEROPNS) SimpleDisplayConsumerR.mof
	@ $(PEGASUS_BIN_DIR)/cimmof -n$(INTEROPNS) CWS_FilesAndDirR.mof
	@ $(PEGASUS_BIN_DIR)/cimmof -n$(INTEROPNS) AssociationProviderR.mof

deregisterproviders:
	@ $(ECHO) "+++++ Deregistering SDKInstanceProviderModule ..."
	@ $(PEGASUS_BIN_DIR)/cimprovider -r -m SDKInstanceProviderModule
	@ $(ECHO) "+++++ Deregistering SDKMethodProviderModule ..."
	@ $(PEGASUS_BIN_DIR)/cimprovider -r -m SDKMethodProviderModule
	@ $(ECHO) "+++++ Deregistering SDKIndicationProviderModule ..."
	@ $(PEGASUS_BIN_DIR)/cimprovider -r -m SDKIndicationProviderModule
	@ $(ECHO) "+++++ Deregistering SDKDisplayConsumerModule ..."
	@ $(PEGASUS_BIN_DIR)/cimprovider -r -m SDKDisplayConsumerModule
	@ $(ECHO) "+++++ Deregistering SDKFilesandDirectoriesModule ..."
	@ $(PEGASUS_BIN_DIR)/cimprovider -r -m SDKFilesAndDirectoriesModule
	@ $(ECHO) "+++++ Deregistering SDKAssociationProviderModule ..."
	@ $(PEGASUS_BIN_DIR)/cimprovider -r -m SDKAssociationProviderModule
