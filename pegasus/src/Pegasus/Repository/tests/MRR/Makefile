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
DIR = Pegasus/MemoryResidentRepository/tests/MemoryResidentRepository
include $(ROOT)/mak/config.mak
include ../libraries.mak

LOCAL_DEFINES = -DPEGASUS_INTERNALONLY

PROGRAM = TestMemoryResidentRepository

SOURCES = MemoryResidentRepository.cpp root_cimv2_namespace.cpp

include $(ROOT)/mak/program.mak

tests:
	$(PROGRAM)

poststarttests:

##==============================================================================
##
## Generates memory-resident repository sources.
##
##==============================================================================

SCHEMAS=$(PEGASUS_ROOT)/Schemas

# set the following parameters for the compiler
#  -d - TBD 
#  -W - suppress the cimmofl usage warning message
#  -m - Compile classes to source files for memory-resident repository

CIMMOFL=cimmofl -d -W -m

# build the root/PG_Internal namespace into the file root_PG_Internal namespace
# The input is all of the mof to be included in this namespace.
repository:
	@ $(CIMMOFL) -n root/cimv2 -I $(SCHEMAS) root_cimv2_namespace.mof
