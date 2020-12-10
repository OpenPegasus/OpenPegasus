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
# Pegasus top level make file
# see the usage rule for options

ROOT = .

include $(ROOT)/env_var.status
include $(ROOT)/mak/config.mak

## Include mu in the clean structure
## This required because mu is not part of the hiearchial build
##structure.  It is built with the buildmu target.  Adding this
## variable allows it to be cleaned with the normal clean target.
RECURSE_EXTRA_CLEAN_DIRS += src/utils/mu

# This is a recurse make file
# Defines subdirectories to go to recursively

DIRS = src

TEST_DIRS = test

# Define the inclusion of the recurse.mak file to execute the next
# level of makefiles defined by the DIRS variable

defaultrule: cimprovagt32 all setupdevserver

include $(ROOT)/mak/recurse.mak

.PHONY: FORCE

FORCE:

usage: FORCE
	$(USAGE)
	$(USAGE)"Makefile targets:"
	$(USAGE)
	$(USAGE)"Recursive rules - These are the primatives that traverse the tree"
	$(USAGE)"invoking the specified command in each subdirectory directory."
	$(USAGE)"NOTE: all is special, it specifies no target and therefore invokes"
	$(USAGE)"the default rule for that directory."
	$(USAGE)"all                 - recursive DEFAULT rule"
	$(USAGE)"clean               - recursive clean"
	$(USAGE)"depend              - buildmu recursive depend"
	$(USAGE)"messages            - rootbundle recursive messages"
	$(USAGE)"tests               - recursive tests"
	$(USAGE)"poststarttests      - recursive poststarttests"
	$(USAGE)
	$(USAGE)"Combinational rules - Combine other rules to achieve results"
	$(USAGE)"DEFAULT RULE        - all, setupdevserver"
	$(USAGE)"new                 - clean repositoryclean"
	$(USAGE)"build               - depend all, setupdevserver"
	$(USAGE)"world               - build unittests servertests"
	$(USAGE)
	$(USAGE)"Functional rules - Other rules to achieve specified results"
	$(USAGE)"clobber             -removes objects built during compile"
	$(USAGE)"                     specifically the following directories are removed:"
	$(USAGE)"                      $(PEGASUS_HOME)/bin"
	$(USAGE)"                      $(PEGASUS_HOME)/lib"
	$(USAGE)"                      $(PEGASUS_HOME)/obj"
	$(USAGE)"buildmu             - builds the mu utility"
	$(USAGE)"buildclientlibs     - Build only the Client and Common libraries"
	$(USAGE)"setupdevserver      - setup the development server env"
	$(USAGE)"cleandevserver      - cleans the development server env"
	$(USAGE)"repository          - builds the base repository. Does not remove other"
	$(USAGE)"                      namespaces than the base namespaces."
	$(USAGE)"testrepository      - builds items for the test suites into the repository"
	$(USAGE)"repositoryclean     - removes the complete repository"
	$(USAGE)"listplatforms       - List all valid platforms"
	$(USAGE)
	$(USAGE)"Test rules (accessable here but implemented in TestMakefile)"
	$(USAGE)"alltests            - unittests and servertests"
	$(USAGE)"unittests           - runs the unit functional test"
	$(USAGE)"servertests         - runs basic server tests"
	$(USAGE)"perftests           - runs basic server performance tests"
	$(USAGE)"standardtests       - runs server extended tests"
	$(USAGE)"testusage           - TestMakefile usage"
	$(USAGE)"testusage2          - TestMakefile usage2"
	$(USAGE)"stresstests         - Runs the default stresstests"
	$(USAGE)
	$(USAGE)"--------------------"
	$(USAGE)"Quick start:"
	$(USAGE)"  After checkout of new tree:"
	$(USAGE)"  use \"make listplatforms\" to view a list of platforms"
	$(USAGE)"  set PEGASUS_PLATFORM=<your platofrm>"
	$(USAGE)"  set PEGASUS_ROOT=<location of your pegasus source>"
	$(USAGE)"  set PEGASUS_HOME=<build output location"
	$(USAGE)"  make world"
	$(USAGE)
	$(USAGE)"  This will build everthing with a default configuration"
	$(USAGE)"  and run the automated tests."
	$(USAGE)
	$(USAGE)"--------------------"
	$(USAGE)"Examples:"
	$(USAGE)"  After \"cvs checkout\" of new tree:    make world"
	$(USAGE)
	$(USAGE)"  After changes to include files:      make"
	$(USAGE)
	$(USAGE)"  After changes to the files included: make build"
	$(USAGE)
	$(USAGE)"  After \"cvs update\" or to start over: make new world"
	$(USAGE)
	

listplatforms: FORCE
	$(USAGE)
	$(USAGE)"The $(words $(VALID_PLATFORMS)) valid platforms are:"
	$(USAGE)" $(foreach w, $(VALID_PLATFORMS), " $w ")"
	$(USAGE)
	$(USAGE)

#########################################################################
# This section defines any prerequisites that are required by the
# recursive rules.
#
# NOTE: You can add prerequisties for the recursive rules but you cannot
#       add any commands to run as part of the rule. You can define them
#       and make will quietly ignore them and they will not be run either
#       before or after the recursive rule.
#
#
messages: rootbundle

depend: buildmu

#########################################################################
# This section defines combinational rules
#
#-----------------------
# build target: builds all source
#
build: cimprovagt32depend cimprovagt32 depend renameinterop all setupdevserver

#------------------------
# rebuild target is being deprecated instead use "make new build"
#
rebuild_msg: FORCE
	@$(ECHO) "==============================================================================="
	@$(ECHO) "Makefile: The rebuild target is being deprecated."
	@$(ECHO) "          Use \"make usage\" for a description of the usage model."
	@$(ECHO) "          Consider using \"make new world\" ."
	@$(ECHO) "          Invoking the old rebuild rule now."
	@$(ECHO) "==============================================================================="

rebuild: rebuild_msg shortsleep new build s_unittests repository

#-----------------------
# new target: cleans everthing
#
# This can be combined on the command line with other rules like:
#
# make new build
# make new world

new: cimprovagt32clean clean repositoryclean

#-----------------------
# world targets: builds everything and dependent on which target may do testing
#
#       Typically used after a fresh checkout from CVS

world: build s_unittests servertests

############################
#
# rules for building 32 bit provider agent
#
cimprovagt32depend: FORCE
ifdef PEGASUS_PLATFORM_FOR_32BIT_PROVIDER_SUPPORT
	$(MAKE) --directory=$(ROOT) -f Makefile.cimprovagt32 depend
endif
cimprovagt32: FORCE
ifdef PEGASUS_PLATFORM_FOR_32BIT_PROVIDER_SUPPORT
	$(MAKE) --directory=$(ROOT) -f Makefile.cimprovagt32
endif

cimprovagt32clean: FORCE
ifdef PEGASUS_PLATFORM_FOR_32BIT_PROVIDER_SUPPORT
	$(MAKE) --directory=$(ROOT) -f Makefile.cimprovagt32 clean
endif

############################
#
# rules defined in TestMakefile that are repeated here for convenience
#
shortsleep: FORCE
	@$(MAKE)  -f TestMakefile shortsleep

servertests: FORCE
	@ $(MAKE) -f TestMakefile servertests

perftests: FORCE
	@ $(MAKE) -f TestMakefile perftests

s_unittests: FORCE
	@ $(MAKE) -f TestMakefile -s unittests

unittests: FORCE
	@ $(MAKE) -f TestMakefile unittests

standardtests: FORCE
	@ $(MAKE) -f TestMakefile standardtests

alltests: FORCE
	@ $(MAKE) -f TestMakefile alltests

testusage: FORCE
	@ $(MAKE) -f TestMakefile usage

testusage2: FORCE
	@ $(MAKE) -f TestMakefile usage2

stresstests:
	@$(ECHO) "Running OpenPegasus StressTests"
	@$(MAKE)  -f TestMakefile stresstests
	@$(ECHO) "+++++ OpenPegasus StressTests complete"

##########################################################################
#
# This section defines functional rules
#
#---------------------
# buildmu target: build mu the make utility that among other things
# includes a depend implementation. This is a separate target because
# it must be build before anything else and before the depend target is used
# on some platforms. Note that mu is not used on all platforms.
#
buildmu: FORCE
	$(MAKE) --directory=$(PEGASUS_ROOT)/src/utils/mu -f Makefile

##########################################################################
#
# renameinterop: This is with regard to PEP304, build time option.
# There are many test cases and result files where namespace name
# root/PG_InterOp is hardcoded. So this searches for root/PG_InterOp
# and changes that to "interop" or "root/interop".
#
# This uses mu replace internally. The original files are all saved as
# filename.save. These file can be restored by running 
# make -f Makefile.interop restore
#
renameinterop: buildmu
ifeq ($(PEGASUS_INTEROP_NAMESPACE),root/interop)
	$(MAKE) --directory=$(PEGASUS_ROOT) -f Makefile.interop replace
endif
ifeq ($(PEGASUS_INTEROP_NAMESPACE),interop)
	$(MAKE) --directory=$(PEGASUS_ROOT) -f Makefile.interop replace
endif

# buildclientlibs: The libpegclient depends on libpegcommon library.
# This build target can be used to build just these two libraries. 
# With this target, SLP support for clients is not enabled.
#

buildclientlibs: FORCE
	$(MAKE) --directory=$(PEGASUS_ROOT)/src/Pegasus/Common -f Makefile
	$(MAKE) --directory=$(PEGASUS_ROOT)/src/Pegasus/Client -f Makefile

#----------------------
# setupdevserver and cleandevserver are used to setup and clear the
# server configuration files needed to run the server in a development
# environment.
#
setupdevserver: FORCE
	$(MAKE) --directory=$(PEGASUS_ROOT)/src/Server -f Makefile install_run
ifeq ($(PEGASUS_ENABLE_PROTOCOL_WEB), true)
	-$(MAKE) -f Makefile.webAdmin setupwebadmin
endif
	@$(ECHO) "PEGASUS Development Server Runtime Environment configured "

cleandevserver: FORCE
	$(MAKE) --directory=$(PEGASUS_ROOT)/src/Server -f Makefile install_run_clean

clobber: FORCE
	- $(RMDIRHIER) $(PEGASUS_HOME)/bin
	- $(RMDIRHIER) $(PEGASUS_HOME)/lib
	- $(RMDIRHIER) $(PEGASUS_HOME)/obj


#---------------------
# The repository target removes the entire repository and rebuilds the Pegasus
# namespaces.  The repositoryServer target does not remove the repository
# before building the Pegasus namespaces.  (The repositoryServer target in
# TestMakefile *does* remove the repository first.)

# Note: Arguments must be quoted to preserve upper case characters in VMS.
repository: repositoryclean
	@ $(MAKE) "-SC" Schemas/Pegasus repository

repositoryclean: FORCE
	@ $(RMREPOSITORY) $(REPOSITORY_ROOT)

repositoryServer: FORCE
	@ $(MAKE) "-SC" Schemas/Pegasus repositoryServer

#---------------------
# The testrepository and testrepositoryServer targets build the Pegasus test
# namespaces.  A pre-existing repository is not removed.

testrepository: FORCE
	@ $(MAKE) "-SC" src/Providers/sample/Load repository
	@ $(MAKE) "-SC" test/wetest repository
	@ $(MAKE) "-SC" src/Clients/benchmarkTest/Load repository
	@ $(MAKE) "-SC" src/Pegasus/CQL/CQLCLI repository
	@ $(MAKE) "-SC" src/Pegasus/Query/QueryExpression/tests repository
	@ $(MAKE) "-SC" src/Providers/TestProviders/Load repository
ifeq ($(PEGASUS_ENABLE_CQL),true)
	@ $(MAKE) "-SC" src/Pegasus/ControlProviders/QueryCapabilitiesProvider/tests repository
endif
ifeq ($(PEGASUS_ENABLE_JMPI_PROVIDER_MANAGER), true)
	@ $(MAKE) "-SC" src/Pegasus/ProviderManager2/JMPI/org/pegasus/jmpi/tests repository
endif
	@ $(MAKE) --directory=$(PEGASUS_ROOT)/src/Clients/cimsub/tests/testscript \
            -f Makefile repository

testrepositoryServer: FORCE
	@ $(MAKE) "-SC" src/Providers/sample/Load repositoryServer
	@ $(MAKE) "-SC" test/wetest repositoryServer
	@ $(MAKE) "-SC" src/Clients/benchmarkTest/Load repositoryServer
	@ $(MAKE) "-SC" src/Pegasus/CQL/CQLCLI repositoryServer
	@ $(MAKE) "-SC" src/Pegasus/Query/QueryExpression/tests repositoryServer
	@ $(MAKE) "-SC" src/Providers/TestProviders/Load repositoryServer
ifeq ($(PEGASUS_ENABLE_CQL),true)
	@ $(MAKE) "-SC" src/Pegasus/ControlProviders/QueryCapabilitiesProvider/tests repositoryServer
endif
ifeq ($(PEGASUS_ENABLE_JMPI_PROVIDER_MANAGER), true)
	@ $(MAKE) "-SC" src/Pegasus/ProviderManager2/JMPI/org/pegasus/jmpi/tests repositoryServer
endif
	@ $(MAKE) --directory=$(PEGASUS_ROOT)/src/Clients/cimsub/tests/testscript \
            -f Makefile repositoryServer

rootbundle:
	$(MAKE) --directory=$(PEGASUS_ROOT)/src/utils/cnv2rootbundle -f Makefile

# DO NOT DELETE
