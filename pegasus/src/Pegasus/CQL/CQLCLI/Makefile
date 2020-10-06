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

DIR = Pegasus/CQL/CQLCLI


ifneq ($(OS), HPUX) 
EXTRA_INCLUDES = -Int_includes
endif

include $(ROOT)/mak/config.mak
include $(ROOT)/mak/configschema.mak

LIBRARIES = \
    pegrepository \
    pegwql \
    pegcql \
    pegquerycommon \
    pegconfig \
    pegcommon

ifeq ($(PEGASUS_ENABLE_COMPRESSED_REPOSITORY),true)
    LIBRARIES += z
endif

EXTRA_INCLUDES = $(SYS_INCLUDES)
#SYS_LIBS += -lfl

LOCAL_DEFINES = -DPEGASUS_INTERNALONLY

PROGRAM = TestCQL

SOURCES = CQLCLI.cpp 

CQLTESTNS=root/SampleProvider
CQLTESTNSDIRECTORY=$(REPOSITORY_ROOT)/root\#SampleProvider     

include $(ROOT)/mak/program.mak

repository:
	@ $(ECHO) +++++ Loading CQL Test Schema into $(CQLTESTNS) namespace ...
	@ cimmofl "-R$(REPOSITORY_DIR)" "-N$(REPOSITORY_NAME)" "-M$(REPOSITORY_MODE)" "-n$(CQLTESTNS)" CQLTestSchema.mof
	@ cimmofl "-R$(REPOSITORY_DIR)" "-N$(REPOSITORY_NAME)" "-M$(REPOSITORY_MODE)" "-n$(CQLTESTNS)" CQLTestSchemaR.mof

repositoryServer:
	@ $(ECHO) +++++ Loading CQL Test Schema into $(CQLTESTNS) namespace ...
	@ cimmof "-n$(CQLTESTNS)" CQLTestSchema.mof
	@ cimmof "-n$(CQLTESTNS)" CQLTestSchemaR.mof

tests:
                                                                                
poststarttests:

