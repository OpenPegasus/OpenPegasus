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

OLD_REPOSITORY_PATH = $(TMP_DIR)/
NEW_REPOSITORY_PATH = $(TMP_DIR)/newRepository
MASTER_REPOSITORY   = masterRepository
TESTNAMESPACE       = test/repupgrade
PGINTEROP           = root/PG_InterOp
RESULTFILE          = $(TMP_DIR)/result
ifeq ($(PEGASUS_INTEROP_NAMESPACE),interop)
MASTERRESULTFILE    = $(ROOT)/src/$(DIR)/result.master.interop
else
MASTERRESULTFILE    = $(ROOT)/src/$(DIR)/result.master
endif

DIR = Clients/repupgrade/tests

include $(ROOT)/mak/config.mak

LIBRARIES = \
    pegrepository \
    pegconfig \
    pegcommon

ifeq ($(PEGASUS_ENABLE_COMPRESSED_REPOSITORY),true)
    LIBRARIES += z
endif

LOCAL_DEFINES = -DPEGASUS_INTERNALONLY

PROGRAM = TestUpgrade

SOURCES = UpgradeTest.cpp

include $(ROOT)/mak/program.mak
include $(ROOT)/mak/test.mak

tests:
ifndef PEGASUS_USE_RELEASE_DIRS
	@$(RMREPOSITORY) $(OLD_REPOSITORY_PATH)repository
	@$(RMREPOSITORY) $(NEW_REPOSITORY_PATH)
	@ cimmofl "-R$(OLD_REPOSITORY_PATH)" "-M$(REPOSITORY_MODE)" "-I$(CIM_SCHEMA_DIR)" "-n$(TESTNAMESPACE)" $(ALLOW_EXPERIMENTAL) $(CIM_SCHEMA_DIR)/Core$(CIM_SCHEMA_VER)_Qualifiers.mof
	@ cimmofl "-R$(OLD_REPOSITORY_PATH)" "-M$(REPOSITORY_MODE)" "-n$(TESTNAMESPACE)" $(ROOT)/src/Providers/TestProviders/Load/TST_Family.mof
	@ cimmofl "-R$(OLD_REPOSITORY_PATH)" "-M$(REPOSITORY_MODE)" "-I$(CIM_SCHEMA_DIR)" "-n$(PGINTEROP)" $(ALLOW_EXPERIMENTAL) $(CIM_SCHEMA_DIR)/Core$(CIM_SCHEMA_VER)_Qualifiers.mof
ifeq ($(PEGASUS_INTEROP_NAMESPACE),interop)
	@ cimmofl "-R$(OLD_REPOSITORY_PATH)" "-M$(REPOSITORY_MODE)" "-n$(PGINTEROP)" $(ROOT)/src/Clients/repupgrade/tests/PGInteropTST_Family.mof
	@ cimmofl "-R$(OLD_REPOSITORY_PATH)" "-N$(MASTER_REPOSITORY)" "-M$(REPOSITORY_MODE)" "-I$(CIM_SCHEMA_DIR)" "-n$(NAMESPACE_INTEROP)" $(ALLOW_EXPERIMENTAL) $(CIM_SCHEMA_DIR)/Core$(CIM_SCHEMA_VER)_Qualifiers.mof
	@ cimmofl "-R$(OLD_REPOSITORY_PATH)" "-N$(MASTER_REPOSITORY)" "-M$(REPOSITORY_MODE)" "-n$(NAMESPACE_INTEROP)" $(ROOT)/src/Clients/repupgrade/tests/InteropTST_Family.mof
endif
	@ $(MKDIRHIER) $(NEW_REPOSITORY_PATH)
ifeq ($(PEGASUS_INTEROP_NAMESPACE),interop)
	@repupgrade -o $(OLD_REPOSITORY_PATH)repository -n $(NEW_REPOSITORY_PATH) -i
	$(PROGRAM) $(OLD_REPOSITORY_PATH)repository $(NEW_REPOSITORY_PATH) $(OLD_REPOSITORY_PATH)masterRepository
else
	@repupgrade -o $(OLD_REPOSITORY_PATH)repository -n $(NEW_REPOSITORY_PATH)
	$(PROGRAM) $(OLD_REPOSITORY_PATH)repository $(NEW_REPOSITORY_PATH)
endif
	@$(RM) $(RESULTFILE)
	@$(MAKE) -i -s tests_ignoreerror
	@$(STRIPCRS)
	@$(COMPARERESULTS)
	@$(ECHO) +++ Test passed +++
	@$(RMREPOSITORY) $(OLD_REPOSITORY_PATH)repository
	@$(RMREPOSITORY) $(NEW_REPOSITORY_PATH)
ifeq ($(PEGASUS_INTEROP_NAMESPACE),interop)
	@$(RMREPOSITORY) $(OLD_REPOSITORY_PATH)masterRepository
endif
	@$(RM) $(RESULTFILE)
endif

tests_ignoreerror:
	@$(ECHO) "  " >> $(RESULTFILE)
	@$(ECHO) "Display usage - error" >> $(RESULTFILE)
	@repupgrade >> $(RESULTFILE) $(REDIRECTERROR)
	@$(ECHO) "  " >> $(RESULTFILE)
	@$(ECHO) "Display Usage - error" >> $(RESULTFILE)
	@repupgrade -o  >> $(RESULTFILE) $(REDIRECTERROR)
	@$(ECHO) "  " >> $(RESULTFILE)
	@$(ECHO) "Display Usage - error" >> $(RESULTFILE)
	@repupgrade -n  >> $(RESULTFILE) $(REDIRECTERROR)
	@$(ECHO) "  " >> $(RESULTFILE)
	@$(ECHO) "Display help message" >> $(RESULTFILE)
	@repupgrade -h  >> $(RESULTFILE) $(REDIRECTERROR)
	@$(ECHO) "  " >> $(RESULTFILE)
	@$(ECHO) "Display help message" >> $(RESULTFILE)
	@repupgrade --help  >> $(RESULTFILE) $(REDIRECTERROR)
	@$(ECHO) "  " >> $(RESULTFILE)
	@$(ECHO) "Non-existent old repository - error case" >> $(RESULTFILE)
	@repupgrade -o non-existent-dir  >> $(RESULTFILE) $(REDIRECTERROR)
	@$(ECHO) "  " >> $(RESULTFILE)
	@$(ECHO) "Non-existent new repository - error case" >> $(RESULTFILE)
	@repupgrade -n non-existent-dir  >> $(RESULTFILE) $(REDIRECTERROR)
	@$(ECHO) "  " >> $(RESULTFILE)
	@$(ECHO) "Combining options - error case" >> $(RESULTFILE)
	@repupgrade -h -o >> $(RESULTFILE) $(REDIRECTERROR)
	@$(ECHO) "  " >> $(RESULTFILE)
	@$(ECHO) "Combining options - error case" >> $(RESULTFILE)
	@repupgrade -h -r >> $(RESULTFILE) $(REDIRECTERROR)
	@$(ECHO) "  " >> $(RESULTFILE)
	@$(ECHO) "Combining options - error case" >> $(RESULTFILE)
	@repupgrade -h -n >> $(RESULTFILE) $(REDIRECTERROR)
	@$(ECHO) "  " >> $(RESULTFILE)
	@$(ECHO) "Combining options - error case" >> $(RESULTFILE)
	@repupgrade -o -h >> $(RESULTFILE) $(REDIRECTERROR)
	@$(ECHO) "  " >> $(RESULTFILE)
	@$(ECHO) "Combining options - error case" >> $(RESULTFILE)
	@repupgrade -n -h >> $(RESULTFILE) $(REDIRECTERROR)
	@$(ECHO) "  " >> $(RESULTFILE)
	@$(ECHO) "Combining options - error case" >> $(RESULTFILE)
	@repupgrade -o -o >> $(RESULTFILE) $(REDIRECTERROR)
	@$(ECHO) "  " >> $(RESULTFILE)
	@$(ECHO) "Combining options - error case" >> $(RESULTFILE)
	@repupgrade -n -n >> $(RESULTFILE) $(REDIRECTERROR)
	@$(ECHO) "  " >> $(RESULTFILE)
	@$(ECHO) "Combining options - error case" >> $(RESULTFILE)
	@repupgrade -h -h >> $(RESULTFILE) $(REDIRECTERROR)
	@$(ECHO) "  " >> $(RESULTFILE)
	@$(ECHO) "Combining options - error case" >> $(RESULTFILE)
	@repupgrade -v -v >> $(RESULTFILE) $(REDIRECTERROR)

poststarttests:

