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

include $(ROOT)/mak/config.mak
include $(ROOT)/mak/configschema.mak

PEGASUS_ZOS_PROGRAM_OBJECT = yes

DIR = Providers/TestProviders/MCCA_TestAssocProvider/testclient

include $(ROOT)/mak/config.mak

LIBRARIES = \
    pegmanagedclient \
    pegclient \
    pegcommon

PROGRAM = TestMCCATestClient
SOURCES = MCCATestClient.cpp

ifeq ($(PEGASUS_ENABLE_SLP),true)
     LIBRARIES += pegslp_client
endif

include $(ROOT)/mak/program.mak

tests:

## Create repository namespaces for this test and
## execute tests. Remove repository when complete.

poststarttests: repository
	
	$(PROGRAM) 1
	$(MAKE) -i unregister

# Load the sample MOF with the local compiler.
repository:

# Loading Core mof into the two new test namespaces	
	@ $(ECHO) +++++ Loading CIM_Core$(CIM_SCHEMA_VER) into root/MCCAtest/A namespace ...
	@ cimmof "-I$(CIM_SCHEMA_DIR)" "-n/root/MCCAtest/A" $(CIM_SCHEMA_DIR)/CIM_Core$(CIM_SCHEMA_VER).mof
	
	@ $(ECHO) +++++ Loading CIM_Core$(CIM_SCHEMA_VER) into root/MCCAtest/B namespace ...
	@ cimmof  "-I$(CIM_SCHEMA_DIR)" "-n/root/MCCAtest/B" $(CIM_SCHEMA_DIR)/CIM_Core$(CIM_SCHEMA_VER).mof

# Loading MCCA_TestClass into the two namespaces
	@ $(ECHO) +++++ Loading MCCA_TestClass into root/MCCAtest/A namespace ...
	@ cimmof  "-nroot/MCCAtest/A" MCCA_TestClass.mof
	@ $(ECHO) +++++ Loading MCCA_TestClass into root/MCCAtest/B namespace ...
	@ cimmof  "-nroot/MCCAtest/B" MCCA_TestClass.mof
# Loading Association Schema definition into both namespaces     
	@ $(ECHO) +++++ Loading MCCA_TestAssocClass into root/MCCAtest/A namespace ...
	@ cimmof  "-nroot/MCCAtest/A" $(ROOT)/src/Providers/TestProviders/MCCA_TestAssocProvider/MCCA_TestAssocPrvSchema.mof
	@ $(ECHO) +++++ Loading MCCA_TestAssocClass into root/MCCBtest/B namespace ...
	@ cimmof  "-nroot/MCCAtest/B" $(ROOT)/src/Providers/TestProviders/MCCA_TestAssocProvider/MCCA_TestAssocPrvSchema.mof
# Registering Provider in interop namespace
	@ $(ECHO) +++++ Loading  MCCA_TestAssocClass provider definitions into $(PEGASUS_INTEROP_NAMESPACE) namespace ...
	@ cimmof  "-n$(INTEROPNS)" $(ROOT)/src/Providers/TestProviders/MCCA_TestAssocProvider/MCCA_TestAssocPrvReg.mof

removeclasses:
	@ $(ECHO) +++++ Removing MCCA_Test Classes from repository ...
	wbemexec DeleteClassA.xml > $(TMP_DIR)removeclasses.log
	wbemexec DeleteClassB.xml >> $(TMP_DIR)removeclasses.log
	wbemexec DeleteAssocA.xml >> $(TMP_DIR)removeclasses.log
	wbemexec DeleteAssocB.xml >> $(TMP_DIR)removeclasses.log

removerepository:
# Removing test repository without cimserver knowing it
# This is really a very dangerous thing to do.  The Server does not know that
# the namespace was removed and still reflects its existence internally 
# until stopped and restarted.	
	@ $(ECHO) +++++ Removing existing repository named root/MCCAtest/A
	@$(RMREPOSITORY) "$(REPOSITORY_ROOT)/root#MCCAtest#A"
	@ $(ECHO) +++++ Removing existing repository named root/MCCAtest/B
	@$(RMREPOSITORY) "$(REPOSITORY_ROOT)/root#MCCAtest#B"
	@$(RM) $(TMP_DIR)removeclasses.log

unregister: removeclasses
	@ $(ECHO) +++++ Unregister MCCA_TestAssocClass provider ...
	$(MAKE) -i unregisterproviders
	@$(RM) $(TMP_DIR)removeclasses.log
## The following statement should be included since this
## Makefile creates its own repository and should probably delete it.
## This creates an inconsistent state for the server until it is stopped
## and restarted.  Therefore, we do not execute the namespace removal. This
## should be modified when we have a clean way to completely remove
## namespaces from a running server.
##	$(MAKE) -i removerepository

unregisterproviders:
	cimprovider -r -m MCCATestAssocProviderModule	
