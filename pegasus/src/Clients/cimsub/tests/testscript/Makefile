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
include $(ROOT)/mak/test.mak
include $(ROOT)/mak/commands.mak

CIMSUBTESTNS=test/CimsubTestNS0
CIMSUBTESTNS1=test/CimsubTestNS1
CIMSUBTESTNS2=test/CimsubTestNS2
CIMSUBTESTNS3=test/CimsubTestNS3

DIR = Clients/cimsub/tests/testscript

RESULTFILE =  $(TMP_DIR)/result
RESULTFILE_EMAIL =  $(TMP_DIR)/result.email
RESULTFILE_SYSLOG =  $(TMP_DIR)/result.syslog
ifeq ($(PEGASUS_ENABLE_CQL),true)
    MASTERRESULTFILE = $(ROOT)/src/$(DIR)/result.master
else
    MASTERRESULTFILE = $(ROOT)/src/$(DIR)/result.master.nocql
endif
MASTERRESULTFILE_EMAIL = $(ROOT)/src/$(DIR)/result.master.email
MASTERRESULTFILE_SYSLOG = $(ROOT)/src/$(DIR)/result.master.syslog

##
## Load the test MOF with cimmofl.
##
repository:
	@ $(ECHO) +++++ Loading CIM_Core$(CIM_SCHEMA_VER) into $(CIMSUBTESTNS) namespace ...
	@ cimmofl "-R$(REPOSITORY_DIR)" "-N$(REPOSITORY_NAME)" "-M$(REPOSITORY_MODE)" -n$(CIMSUBTESTNS) $(CIM_SCHEMA_DIR)/Core$(CIM_SCHEMA_VER)_Qualifiers.mof
	@ $(ECHO) +++++ Loading CIM_Core$(CIM_SCHEMA_VER) into $(CIMSUBTESTNS) namespace ...
	@ cimmofl "-R$(REPOSITORY_DIR)" "-N$(REPOSITORY_NAME)" "-M$(REPOSITORY_MODE)" "-I$(CIM_SCHEMA_DIR)" "-n$(CIMSUBTESTNS)" $(ALLOW_EXPERIMENTAL) $(CIM_SCHEMA_DIR)/CIM_Core$(CIM_SCHEMA_VER).mof
	@ $(ECHO) +++++ Loading CIM_Event$(CIM_SCHEMA_VER) into $(CIMSUBTESTNS) namespace ...
	@ cimmofl "-R$(REPOSITORY_DIR)" "-N$(REPOSITORY_NAME)" "-M$(REPOSITORY_MODE)" "-I$(CIM_SCHEMA_DIR)" "-n$(CIMSUBTESTNS)" $(ALLOW_EXPERIMENTAL) $(CIM_SCHEMA_DIR)/CIM_Event$(CIM_SCHEMA_VER).mof

	@ $(ECHO) +++++ Loading PG_Events$(PG_INTEROP_SCHEMA_VER) into $(CIMSUBTESTNS) namespace ...
	@ cimmofl "-R$(REPOSITORY_DIR)" "-N$(REPOSITORY_NAME)" "-M$(REPOSITORY_MODE)" "-I$(PG_INTEROP_MOF_PATH)" "-n$(CIMSUBTESTNS)" $(ALLOW_EXPERIMENTAL) $(PG_INTEROP_MOF_PATH)/PG_Events$(PG_INTEROP_SCHEMA_VER).mof

	@ $(ECHO) +++++ Loading Core$(CIM_SCHEMA_VER)_Qualifiers.mof into $(CIMSUBTESTNS1) namespace ...
	@ cimmofl "-R$(REPOSITORY_DIR)" "-N$(REPOSITORY_NAME)" "-M$(REPOSITORY_MODE)" -n$(CIMSUBTESTNS1) $(CIM_SCHEMA_DIR)/Core$(CIM_SCHEMA_VER)_Qualifiers.mof
	@ $(ECHO) +++++ Loading CIM_Core$(CIM_SCHEMA_VER) into $(CIMSUBTESTNS1) namespace ...
	@ cimmofl "-R$(REPOSITORY_DIR)" "-N$(REPOSITORY_NAME)" "-M$(REPOSITORY_MODE)" "-I$(CIM_SCHEMA_DIR)" "-n$(CIMSUBTESTNS1)" $(ALLOW_EXPERIMENTAL) $(CIM_SCHEMA_DIR)/CIM_Core$(CIM_SCHEMA_VER).mof
	@ $(ECHO) +++++ Loading CIM_Event$(CIM_SCHEMA_VER) into $(CIMSUBTESTNS1) namespace ...
	@ cimmofl "-R$(REPOSITORY_DIR)" "-N$(REPOSITORY_NAME)" "-M$(REPOSITORY_MODE)" "-I$(CIM_SCHEMA_DIR)" "-n$(CIMSUBTESTNS1)" $(ALLOW_EXPERIMENTAL) $(CIM_SCHEMA_DIR)/CIM_Event$(CIM_SCHEMA_VER).mof

	@ $(ECHO) +++++ Loading PG_Events$(PG_INTEROP_SCHEMA_VER) into $(CIMSUBTESTNS1) namespace ...
	@ cimmofl "-R$(REPOSITORY_DIR)" "-N$(REPOSITORY_NAME)" "-M$(REPOSITORY_MODE)" "-I$(PG_INTEROP_MOF_PATH)" "-n$(CIMSUBTESTNS1)" $(ALLOW_EXPERIMENTAL) $(PG_INTEROP_MOF_PATH)/PG_Events$(PG_INTEROP_SCHEMA_VER).mof

	@ $(ECHO) +++++ Loading Core$(CIM_SCHEMA_VER)_Qualifiers.mof into $(CIMSUBTESTNS2) namespace ...
	@ cimmofl "-R$(REPOSITORY_DIR)" "-N$(REPOSITORY_NAME)" "-M$(REPOSITORY_MODE)" -n$(CIMSUBTESTNS2) $(CIM_SCHEMA_DIR)/Core$(CIM_SCHEMA_VER)_Qualifiers.mof
	@ $(ECHO) +++++ Loading CIM_Core$(CIM_SCHEMA_VER) into $(CIMSUBTESTNS2) namespace ...
	@ cimmofl "-R$(REPOSITORY_DIR)" "-N$(REPOSITORY_NAME)" "-M$(REPOSITORY_MODE)" "-I$(CIM_SCHEMA_DIR)" "-n$(CIMSUBTESTNS2)" $(ALLOW_EXPERIMENTAL) $(CIM_SCHEMA_DIR)/CIM_Core$(CIM_SCHEMA_VER).mof
	@ $(ECHO) +++++ Loading CIM_Event$(CIM_SCHEMA_VER) into $(CIMSUBTESTNS2) namespace ...
	@ cimmofl "-R$(REPOSITORY_DIR)" "-N$(REPOSITORY_NAME)" "-M$(REPOSITORY_MODE)" "-I$(CIM_SCHEMA_DIR)" "-n$(CIMSUBTESTNS2)" $(ALLOW_EXPERIMENTAL) $(CIM_SCHEMA_DIR)/CIM_Event$(CIM_SCHEMA_VER).mof

	@ $(ECHO) +++++ Loading PG_Events$(PG_INTEROP_SCHEMA_VER) into $(CIMSUBTESTNS2) namespace ...
	@ cimmofl "-R$(REPOSITORY_DIR)" "-N$(REPOSITORY_NAME)" "-M$(REPOSITORY_MODE)" "-I$(PG_INTEROP_MOF_PATH)" "-n$(CIMSUBTESTNS2)" $(ALLOW_EXPERIMENTAL) $(PG_INTEROP_MOF_PATH)/PG_Events$(PG_INTEROP_SCHEMA_VER).mof

	@ $(ECHO) +++++ Loading Core$(CIM_SCHEMA_VER)_Qualifiers.mof into $(CIMSUBTESTNS3) namespace ...
	@ cimmofl "-R$(REPOSITORY_DIR)" "-N$(REPOSITORY_NAME)" "-M$(REPOSITORY_MODE)" -n$(CIMSUBTESTNS3) $(CIM_SCHEMA_DIR)/Core$(CIM_SCHEMA_VER)_Qualifiers.mof
	@ $(ECHO) +++++ Loading CIM_Core$(CIM_SCHEMA_VER) into $(CIMSUBTESTNS3) namespace ...
	@ cimmofl "-R$(REPOSITORY_DIR)" "-N$(REPOSITORY_NAME)" "-M$(REPOSITORY_MODE)" "-I$(CIM_SCHEMA_DIR)" "-n$(CIMSUBTESTNS3)" $(ALLOW_EXPERIMENTAL) $(CIM_SCHEMA_DIR)/CIM_Core$(CIM_SCHEMA_VER).mof
	@ $(ECHO) +++++ Loading CIM_Event$(CIM_SCHEMA_VER) into $(CIMSUBTESTNS3) namespace ...
	@ cimmofl "-R$(REPOSITORY_DIR)" "-N$(REPOSITORY_NAME)" "-M$(REPOSITORY_MODE)" "-I$(CIM_SCHEMA_DIR)" "-n$(CIMSUBTESTNS3)" $(ALLOW_EXPERIMENTAL) $(CIM_SCHEMA_DIR)/CIM_Event$(CIM_SCHEMA_VER).mof

	@ $(ECHO) +++++ Loading PG_Events$(PG_INTEROP_SCHEMA_VER) into $(CIMSUBTESTNS3) namespace ...
	@ cimmofl "-R$(REPOSITORY_DIR)" "-N$(REPOSITORY_NAME)" "-M$(REPOSITORY_MODE)" "-I$(PG_INTEROP_MOF_PATH)" "-n$(CIMSUBTESTNS3)" $(ALLOW_EXPERIMENTAL) $(PG_INTEROP_MOF_PATH)/PG_Events$(PG_INTEROP_SCHEMA_VER).mof

##
## Load the test MOF with cimmof.
##
repositoryServer:
	@ $(ECHO) +++++ Loading CIM_Core$(CIM_SCHEMA_VER) into $(CIMSUBTESTNS) namespace ...
	@ cimmof -n$(CIMSUBTESTNS) $(CIM_SCHEMA_DIR)/Core$(CIM_SCHEMA_VER)_Qualifiers.mof
	@ $(ECHO) +++++ Loading CIM_Core$(CIM_SCHEMA_VER) into $(CIMSUBTESTNS) namespace ...
	@ cimmof "-I$(CIM_SCHEMA_DIR)" "-n$(CIMSUBTESTNS)" $(ALLOW_EXPERIMENTAL) $(CIM_SCHEMA_DIR)/CIM_Core$(CIM_SCHEMA_VER).mof
	@ $(ECHO) +++++ Loading CIM_Event$(CIM_SCHEMA_VER) into $(CIMSUBTESTNS) namespace ...
	@ cimmof "-I$(CIM_SCHEMA_DIR)" "-n$(CIMSUBTESTNS)" $(ALLOW_EXPERIMENTAL) $(CIM_SCHEMA_DIR)/CIM_Event$(CIM_SCHEMA_VER).mof

	@ $(ECHO) +++++ Loading PG_Events$(PG_INTEROP_SCHEMA_VER) into $(CIMSUBTESTNS) namespace ...
	@ cimmof "-I$(PG_INTEROP_MOF_PATH)" "-n$(CIMSUBTESTNS)" $(ALLOW_EXPERIMENTAL) $(PG_INTEROP_MOF_PATH)/PG_Events$(PG_INTEROP_SCHEMA_VER).mof

	@ $(ECHO) +++++ Loading Core$(CIM_SCHEMA_VER)_Qualifiers.mof into $(CIMSUBTESTNS1) namespace ...
	@ cimmof -n$(CIMSUBTESTNS1) $(CIM_SCHEMA_DIR)/Core$(CIM_SCHEMA_VER)_Qualifiers.mof
	@ $(ECHO) +++++ Loading CIM_Core$(CIM_SCHEMA_VER) into $(CIMSUBTESTNS1) namespace ...
	@ cimmof "-I$(CIM_SCHEMA_DIR)" "-n$(CIMSUBTESTNS1)" $(ALLOW_EXPERIMENTAL) $(CIM_SCHEMA_DIR)/CIM_Core$(CIM_SCHEMA_VER).mof
	@ $(ECHO) +++++ Loading CIM_Event$(CIM_SCHEMA_VER) into $(CIMSUBTESTNS1) namespace ...
	@ cimmof "-I$(CIM_SCHEMA_DIR)" "-n$(CIMSUBTESTNS1)" $(ALLOW_EXPERIMENTAL) $(CIM_SCHEMA_DIR)/CIM_Event$(CIM_SCHEMA_VER).mof

	@ $(ECHO) +++++ Loading PG_Events$(PG_INTEROP_SCHEMA_VER) into $(CIMSUBTESTNS1) namespace ...
	@ cimmof "-I$(PG_INTEROP_MOF_PATH)" "-n$(CIMSUBTESTNS1)" $(ALLOW_EXPERIMENTAL) $(PG_INTEROP_MOF_PATH)/PG_Events$(PG_INTEROP_SCHEMA_VER).mof

	@ $(ECHO) +++++ Loading Core$(CIM_SCHEMA_VER)_Qualifiers.mof into $(CIMSUBTESTNS2) namespace ...
	@ cimmof -n$(CIMSUBTESTNS2) $(CIM_SCHEMA_DIR)/Core$(CIM_SCHEMA_VER)_Qualifiers.mof
	@ $(ECHO) +++++ Loading CIM_Core$(CIM_SCHEMA_VER) into $(CIMSUBTESTNS2) namespace ...
	@ cimmof "-I$(CIM_SCHEMA_DIR)" "-n$(CIMSUBTESTNS2)" $(ALLOW_EXPERIMENTAL) $(CIM_SCHEMA_DIR)/CIM_Core$(CIM_SCHEMA_VER).mof
	@ $(ECHO) +++++ Loading CIM_Event$(CIM_SCHEMA_VER) into $(CIMSUBTESTNS2) namespace ...
	@ cimmof "-I$(CIM_SCHEMA_DIR)" "-n$(CIMSUBTESTNS2)" $(ALLOW_EXPERIMENTAL) $(CIM_SCHEMA_DIR)/CIM_Event$(CIM_SCHEMA_VER).mof

	@ $(ECHO) +++++ Loading PG_Events$(PG_INTEROP_SCHEMA_VER) into $(CIMSUBTESTNS2) namespace ...
	@ cimmof "-I$(PG_INTEROP_MOF_PATH)" "-n$(CIMSUBTESTNS2)" $(ALLOW_EXPERIMENTAL) $(PG_INTEROP_MOF_PATH)/PG_Events$(PG_INTEROP_SCHEMA_VER).mof

	@ $(ECHO) +++++ Loading Core$(CIM_SCHEMA_VER)_Qualifiers.mof into $(CIMSUBTESTNS3) namespace ...
	@ cimmof -n$(CIMSUBTESTNS3) $(CIM_SCHEMA_DIR)/Core$(CIM_SCHEMA_VER)_Qualifiers.mof
	@ $(ECHO) +++++ Loading CIM_Core$(CIM_SCHEMA_VER) into $(CIMSUBTESTNS3) namespace ...
	@ cimmof "-I$(CIM_SCHEMA_DIR)" "-n$(CIMSUBTESTNS3)" $(ALLOW_EXPERIMENTAL) $(CIM_SCHEMA_DIR)/CIM_Core$(CIM_SCHEMA_VER).mof
	@ $(ECHO) +++++ Loading CIM_Event$(CIM_SCHEMA_VER) into $(CIMSUBTESTNS3) namespace ...
	@ cimmof "-I$(CIM_SCHEMA_DIR)" "-n$(CIMSUBTESTNS3)" $(ALLOW_EXPERIMENTAL) $(CIM_SCHEMA_DIR)/CIM_Event$(CIM_SCHEMA_VER).mof

	@ $(ECHO) +++++ Loading PG_Events$(PG_INTEROP_SCHEMA_VER) into $(CIMSUBTESTNS3) namespace ...
	@ cimmof "-I$(PG_INTEROP_MOF_PATH)" "-n$(CIMSUBTESTNS3)" $(ALLOW_EXPERIMENTAL) $(PG_INTEROP_MOF_PATH)/PG_Events$(PG_INTEROP_SCHEMA_VER).mof

tests:

cleanup:
	@TestCimsub cleanup

clean:
	@$(RM) $(RESULTFILE) $(RESULTFILE_EMAIL) $(RESULTFILE_SYSLOG)

#
# Create filters,handlers,and subscriptions required for running the tests
#
test_setup:

	@TestCimsub cleanup
	@TestCimsub register
	@TestCimsub test

poststarttests: test_setup
	@$(RM) $(RESULTFILE) $(RESULTFILE_EMAIL) $(RESULTFILE_SYSLOG)
	@$(MAKE) -i -s cimsubTest1_ignoreerror
	@$(STRIPCRS)
	$(COMPARERESULTS)
ifdef PEGASUS_ENABLE_EMAIL_HANDLER
	@TestCimsub test_email
	@$(MAKE) -s cimsubTestEmail RESULTFILE="$(RESULTFILE_EMAIL)" MASTERRESULTFILE="$(MASTERRESULTFILE_EMAIL)"
endif
ifdef PEGASUS_ENABLE_SYSTEM_LOG_HANDLER
	@TestCimsub cleanup
	@TestCimsub register
	@TestCimsub test
	@TestCimsub test_syslog
	@$(MAKE) -s cimsubTestSyslog RESULTFILE="$(RESULTFILE_SYSLOG)" MASTERRESULTFILE="$(MASTERRESULTFILE_SYSLOG)"
endif
	@$(ECHO) +++ Test passed +++
	@TestCimsub cleanup
	@$(RM) $(RESULTFILE) $(RESULTFILE_EMAIL) $(RESULTFILE_SYSLOG)

cimsubTest1_ignoreerror:
	@$(ECHO) ++++cimsub tests
	@$(ECHO)  +++++++++ Command line testing +++++++++ >> $(RESULTFILE)

	@$(ECHO) "Missing command line option." >> $(RESULTFILE)
	@cimsub -v >> $(RESULTFILE) $(REDIRECTERROR)
	@$(ECHO) >> $(RESULTFILE)

	@$(ECHO) "Duplicate list option." >> $(RESULTFILE)
	@cimsub -ls -lf >> $(RESULTFILE) $(REDIRECTERROR)
	@$(ECHO) >> $(RESULTFILE)

	@$(ECHO) "Missing list argument option." >> $(RESULTFILE)
	@cimsub -l >> $(RESULTFILE) $(REDIRECTERROR)
	@$(ECHO) >> $(RESULTFILE)

	@$(ECHO) "Invalid list argument option." >> $(RESULTFILE)
	@cimsub -lx >> $(RESULTFILE) $(REDIRECTERROR)
	@$(ECHO) >> $(RESULTFILE)

	@$(ECHO) "Duplicate verbose option." >> $(RESULTFILE)
	@cimsub -ls -v -v >> $(RESULTFILE) $(REDIRECTERROR)
	@$(ECHO) >> $(RESULTFILE)

	@$(ECHO) "Missing namespace value." >> $(RESULTFILE)
	@cimsub -ls -n >> $(RESULTFILE) $(REDIRECTERROR)
	@$(ECHO) >> $(RESULTFILE)

	@$(ECHO) "Duplicate namespace option." >> $(RESULTFILE)
	@cimsub -ls -n ns1 -n ns2>> $(RESULTFILE) $(REDIRECTERROR)
	@$(ECHO) >> $(RESULTFILE)

	@$(ECHO) "No command line options specified." >> $(RESULTFILE)
	@cimsub >> $(RESULTFILE) $(REDIRECTERROR)
	@$(ECHO) >> $(RESULTFILE)

	@$(ECHO) "No command line operation specified." >> $(RESULTFILE)
	@cimsub -F Filter01 >> $(RESULTFILE) $(REDIRECTERROR)
	@$(ECHO) >> $(RESULTFILE)

	@$(ECHO) "Invalid command line options specified." >> $(RESULTFILE)
	@cimsub -no >> $(RESULTFILE) $(REDIRECTERROR)
	@$(ECHO) >> $(RESULTFILE)

	@$(ECHO) "Invalid command line options specified." >> $(RESULTFILE)
	@cimsub no_such_option >> $(RESULTFILE) $(REDIRECTERROR)
	@$(ECHO) >> $(RESULTFILE)

	@$(ECHO) "Help option(long)." >> $(RESULTFILE)
	@cimsub --help >> $(RESULTFILE) $(REDIRECTERROR)
	@$(ECHO) >> $(RESULTFILE)

	@$(ECHO) "Duplicate help(long) option." >> $(RESULTFILE)
	@cimsub --help --help >> $(RESULTFILE) $(REDIRECTERROR)
	@$(ECHO) >> $(RESULTFILE)

	@$(ECHO) "Help option(short)." >> $(RESULTFILE)
	@cimsub -h >> $(RESULTFILE) $(REDIRECTERROR)
	@$(ECHO) >> $(RESULTFILE)

	@$(ECHO) "Duplicate help(short) option." >> $(RESULTFILE)
	@cimsub -h -h >> $(RESULTFILE) $(REDIRECTERROR)
	@$(ECHO) >> $(RESULTFILE)

	@$(ECHO) "Duplicate help(short & long) option." >> $(RESULTFILE)
	@cimsub -h --help >> $(RESULTFILE) $(REDIRECTERROR)
	@$(ECHO) >> $(RESULTFILE)

	@$(ECHO) "Duplicate help(long & short) option." >> $(RESULTFILE)
	@cimsub --help -h >> $(RESULTFILE) $(REDIRECTERROR)
	@$(ECHO) >> $(RESULTFILE)

	@$(ECHO) "Unexpected option, combine remove and help." >> $(RESULTFILE)
	@cimsub --help -rs >> $(RESULTFILE) $(REDIRECTERROR)
	@$(ECHO) >> $(RESULTFILE)

	@$(ECHO) "Unexpected option, combine list and help." >> $(RESULTFILE)
	@cimsub --help -ls >> $(RESULTFILE) $(REDIRECTERROR)
	@$(ECHO) >> $(RESULTFILE)

	@$(ECHO) "Unexpected option, combine enable and help." >> $(RESULTFILE)
	@cimsub --help -e >> $(RESULTFILE) $(REDIRECTERROR)
	@$(ECHO) >> $(RESULTFILE)

	@$(ECHO) "Unexpected option, combine verbose and help." >> $(RESULTFILE)
	@cimsub --help -v >> $(RESULTFILE) $(REDIRECTERROR)
	@$(ECHO) >> $(RESULTFILE)

	@$(ECHO) "Unexpected option, combine disable and help." >> $(RESULTFILE)
	@cimsub --help -d >> $(RESULTFILE) $(REDIRECTERROR)
	@$(ECHO) >> $(RESULTFILE)

	@$(ECHO) "Unexpected option, combine filter and help." >> $(RESULTFILE)
	@cimsub --help -Fx >> $(RESULTFILE) $(REDIRECTERROR)
	@$(ECHO) >> $(RESULTFILE)

	@$(ECHO) "Unexpected option, combine handler and help." >> $(RESULTFILE)
	@cimsub --help -Hx >> $(RESULTFILE) $(REDIRECTERROR)
	@$(ECHO) >> $(RESULTFILE)

	@$(ECHO) "Unexpected option, combine namespace and help." >> $(RESULTFILE)
	@cimsub --help -nx >> $(RESULTFILE) $(REDIRECTERROR)
	@$(ECHO) >> $(RESULTFILE)

	@$(ECHO) "Duplicate version option." >> $(RESULTFILE)
	@cimsub --version --version >> $(RESULTFILE) $(REDIRECTERROR)
	@$(ECHO) >> $(RESULTFILE)

	@$(ECHO) "Unexpected option, combine remove and version." >> $(RESULTFILE)
	@cimsub --version -rs >> $(RESULTFILE) $(REDIRECTERROR)
	@$(ECHO) >> $(RESULTFILE)

	@$(ECHO) "Unexpected option, combine list and version." >> $(RESULTFILE)
	@cimsub --version -ls >> $(RESULTFILE) $(REDIRECTERROR)
	@$(ECHO) >> $(RESULTFILE)

	@$(ECHO) "Unexpected option, combine enable and version." >> $(RESULTFILE)
	@cimsub --version -e >> $(RESULTFILE) $(REDIRECTERROR)
	@$(ECHO) >> $(RESULTFILE)

	@$(ECHO) "Unexpected option, combine verbose and version." >> $(RESULTFILE)
	@cimsub --version -v >> $(RESULTFILE) $(REDIRECTERROR)
	@$(ECHO) >> $(RESULTFILE)

	@$(ECHO) "Unexpected option, combine disable and version." >> $(RESULTFILE)
	@cimsub --version -d >> $(RESULTFILE) $(REDIRECTERROR)
	@$(ECHO) >> $(RESULTFILE)

	@$(ECHO) "Unexpected option, combine filter and version." >> $(RESULTFILE)
	@cimsub --version -Fx >> $(RESULTFILE) $(REDIRECTERROR)
	@$(ECHO) >> $(RESULTFILE)

	@$(ECHO) "Unexpected option, combine handler and version." >> $(RESULTFILE)
	@cimsub --version -Hx >> $(RESULTFILE) $(REDIRECTERROR)
	@$(ECHO) >> $(RESULTFILE)

	@$(ECHO) "Unexpected option, combine namespace and version." >> $(RESULTFILE)
	@cimsub --version -nx >> $(RESULTFILE) $(REDIRECTERROR)
	@$(ECHO) >> $(RESULTFILE)

	@$(ECHO) "Display version." >> $(RESULTFILE)
	@cimsub --version >> $(RESULTFILE) $(REDIRECTERROR)
	@$(ECHO) >> $(RESULTFILE)

	@$(ECHO) "Unexpected option, combine enable and disable." >> $(RESULTFILE)
	@cimsub -d -e >> $(RESULTFILE) $(REDIRECTERROR)
	@$(ECHO) >> $(RESULTFILE)

	@$(ECHO) "Duplicate -e option." >> $(RESULTFILE)
	@cimsub -e -e >> $(RESULTFILE) $(REDIRECTERROR)
	@$(ECHO) >> $(RESULTFILE)

	@$(ECHO) "Duplicate -d option." >> $(RESULTFILE)
	@cimsub -d -d >> $(RESULTFILE) $(REDIRECTERROR)
	@$(ECHO) >> $(RESULTFILE)

	@$(ECHO) "Duplicate -r option." >> $(RESULTFILE)
	@cimsub -r -r >> $(RESULTFILE) $(REDIRECTERROR)
	@$(ECHO) >> $(RESULTFILE)

	@$(ECHO) "Duplicate -F option." >> $(RESULTFILE)
	@cimsub -ls -F filter1 -F filter1 >> $(RESULTFILE) $(REDIRECTERROR)
	@$(ECHO) >> $(RESULTFILE)

	@$(ECHO) "Duplicate -H option." >> $(RESULTFILE)
	@cimsub -ls -H handler1 -H handler2 >> $(RESULTFILE) $(REDIRECTERROR)
	@$(ECHO) >> $(RESULTFILE)

	@$(ECHO) "Invalid -T option." >> $(RESULTFILE)
	@cimsub -ls -T >> $(RESULTFILE) $(REDIRECTERROR)
	@$(ECHO) >> $(RESULTFILE)

	@$(ECHO) "Required option missing, handler option not specified." >> $(RESULTFILE)
	@cimsub -e -F filter1 >> $(RESULTFILE) $(REDIRECTERROR)
	@$(ECHO) >> $(RESULTFILE)

	@$(ECHO) "Required option missing, filter option not specified." >> $(RESULTFILE)
	@cimsub -e -H handler1 >> $(RESULTFILE) $(REDIRECTERROR)
	@$(ECHO) >> $(RESULTFILE)

	@$(ECHO) "Required option missing, filter option not specified." >> $(RESULTFILE)
	@cimsub -e -H handler1 >> $(RESULTFILE) $(REDIRECTERROR)
	@$(ECHO) >> $(RESULTFILE)

	@$(ECHO) "Invalid option verbose specified for enable." >> $(RESULTFILE)
	@cimsub -e -v >> $(RESULTFILE) $(REDIRECTERROR)
	@$(ECHO) >> $(RESULTFILE)

	@$(ECHO) "Invalid option verbose specified for disable." >> $(RESULTFILE)
	@cimsub -d -v >> $(RESULTFILE) $(REDIRECTERROR)
	@$(ECHO) >> $(RESULTFILE)

	@$(ECHO) "Required option missing, handler option not specified." >> $(RESULTFILE)
	@cimsub -d -F filter1 >> $(RESULTFILE) $(REDIRECTERROR)
	@$(ECHO) >> $(RESULTFILE)

	@$(ECHO) "Required option missing, filter option not specified." >> $(RESULTFILE)
	@cimsub -d -H Handler01 >> $(RESULTFILE) $(REDIRECTERROR)
	@$(ECHO) >> $(RESULTFILE)

	@$(ECHO) "Missing argument for -H option." >> $(RESULTFILE)
	@cimsub -ls -H >> $(RESULTFILE) $(REDIRECTERROR)
	@$(ECHO) >> $(RESULTFILE)

	@$(ECHO) "Missing argument for -F option." >> $(RESULTFILE)
	@cimsub -ls -F >> $(RESULTFILE) $(REDIRECTERROR)
	@$(ECHO) >> $(RESULTFILE)

	@$(ECHO) "Missing namespace for -F option." >> $(RESULTFILE)
	@cimsub -ls -F :filter1 >> $(RESULTFILE) $(REDIRECTERROR)
	@$(ECHO) >> $(RESULTFILE)

	@$(ECHO) "Missing filter name -F option." >> $(RESULTFILE)
	@cimsub -ls -F test/CimsubTest1: >> $(RESULTFILE) $(REDIRECTERROR)
	@$(ECHO) >> $(RESULTFILE)

	@$(ECHO) "Missing namespace for -H option." >> $(RESULTFILE)
	@cimsub -ls -H :handler1 >> $(RESULTFILE) $(REDIRECTERROR)
	@$(ECHO) >> $(RESULTFILE)

	@$(ECHO) "Missing creation class for -H option." >> $(RESULTFILE)
	@cimsub -ls -H test/CimsubTest1:.handler1 >> $(RESULTFILE) $(REDIRECTERROR)
	@$(ECHO) >> $(RESULTFILE)

	@$(ECHO) "Missing handler name -H option." >> $(RESULTFILE)
	@cimsub -ls -H test/CimsubTest1: >> $(RESULTFILE) $(REDIRECTERROR)
	@$(ECHO) >> $(RESULTFILE)

	@$(ECHO) "Missing creation class and name -H option." >> $(RESULTFILE)
	@cimsub -ls -H test/CimsubTest1:. >> $(RESULTFILE) $(REDIRECTERROR)
	@$(ECHO) >> $(RESULTFILE)

	@$(ECHO) "Missing creation class -H option." >> $(RESULTFILE)
	@cimsub -ls -H test/CimsubTest1:.1 >> $(RESULTFILE) $(REDIRECTERROR)
	@$(ECHO) >> $(RESULTFILE)

	@$(ECHO) "Missing creation class and name -H option." >> $(RESULTFILE)
	@cimsub -ls -H :. >> $(RESULTFILE) $(REDIRECTERROR)
	@$(ECHO) >> $(RESULTFILE)

	@$(ECHO) "Missing creation class and name -H option." >> $(RESULTFILE)
	@cimsub -ls -H . >> $(RESULTFILE) $(REDIRECTERROR)
	@$(ECHO) >> $(RESULTFILE)

	@$(ECHO) "Missing namespace and name -H option." >> $(RESULTFILE)
	@cimsub -ls -H : >> $(RESULTFILE) $(REDIRECTERROR)
	@$(ECHO) >> $(RESULTFILE)

	@$(ECHO) "Missing handler name -H option." >> $(RESULTFILE)
	@cimsub -ls -H test/CimsubTest1:CIM_ListenerDestinationCIMXML. >> $(RESULTFILE) $(REDIRECTERROR)
	@$(ECHO) >> $(RESULTFILE)

	@$(ECHO) "List handler with namespace and class." >> $(RESULTFILE)
	@cimsub -lh -H test/CimsubTestNS2:CIM_ListenerDestinationCIMXML.ListenerDestination05 >> $(RESULTFILE) $(REDIRECTERROR)
	@$(ECHO) >> $(RESULTFILE)

	@$(ECHO) "List subscription with handler only." >> $(RESULTFILE)
	@cimsub -ls -H test/CimsubTestNS2:CIM_ListenerDestinationCIMXML.ListenerDestination05 >> $(RESULTFILE) $(REDIRECTERROR)
	@$(ECHO) >> $(RESULTFILE)

	@$(ECHO) "List subscription with filter only." >> $(RESULTFILE)
	@cimsub -ls -F Filter03 >> $(RESULTFILE) $(REDIRECTERROR)
	@$(ECHO) >> $(RESULTFILE)

	@$(ECHO) "Incorrect option for list handlers." >> $(RESULTFILE)
	@cimsub -lh -F filter01 >> $(RESULTFILE) $(REDIRECTERROR)
	@$(ECHO) >> $(RESULTFILE)

	@$(ECHO) "List handlers and version option." >> $(RESULTFILE)
	@cimsub -lh -H Handler01 --version >> $(RESULTFILE) $(REDIRECTERROR)

	@$(ECHO) "List handlers and help option." >> $(RESULTFILE)
	@cimsub -lh -H Handler01 --help >> $(RESULTFILE) $(REDIRECTERROR)
	@$(ECHO) >> $(RESULTFILE)

	@$(ECHO) "List handlers with no creation class." >> $(RESULTFILE)
	@cimsub -lh -H .1 >> $(RESULTFILE) $(REDIRECTERROR)
	@$(ECHO) >> $(RESULTFILE)

	@$(ECHO) "Incorrect option for list filters." >> $(RESULTFILE)
	@cimsub -lf -H handler1 >> $(RESULTFILE) $(REDIRECTERROR)
	@$(ECHO) >> $(RESULTFILE)

	@$(ECHO) "List filters with version option." >> $(RESULTFILE)
	@cimsub -lf --version >> $(RESULTFILE) $(REDIRECTERROR)
	@$(ECHO) >> $(RESULTFILE)

	@$(ECHO) "List filters with help option." >> $(RESULTFILE)
	@cimsub -lf --help >> $(RESULTFILE) $(REDIRECTERROR)
	@$(ECHO) >> $(RESULTFILE)

	@$(ECHO) "No command line argument for remove." >> $(RESULTFILE)
	@cimsub -r >> $(RESULTFILE) $(REDIRECTERROR)
	@$(ECHO) >> $(RESULTFILE)

	@$(ECHO) "Two removes." >> $(RESULTFILE)
	@cimsub -rs -rs >> $(RESULTFILE) $(REDIRECTERROR)
	@$(ECHO) >> $(RESULTFILE)

	@$(ECHO) "Invalid option verbose specified for remove." >> $(RESULTFILE)
	@cimsub -rs -v >> $(RESULTFILE) $(REDIRECTERROR)
	@$(ECHO) >> $(RESULTFILE)

	@$(ECHO) "No required options for remove subscription." >> $(RESULTFILE)
	@cimsub -rs >> $(RESULTFILE) $(REDIRECTERROR)
	@$(ECHO) >> $(RESULTFILE)

	@$(ECHO) "No required options for remove all." >> $(RESULTFILE)
	@cimsub -ra >> $(RESULTFILE) $(REDIRECTERROR)
	@$(ECHO) >> $(RESULTFILE)

	@$(ECHO) "No required option for remove filter." >> $(RESULTFILE)
	@cimsub -rf >> $(RESULTFILE) $(REDIRECTERROR)
	@$(ECHO) >> $(RESULTFILE)

	@$(ECHO) "No required option for remove handler." >> $(RESULTFILE)
	@cimsub -rh >> $(RESULTFILE) $(REDIRECTERROR)
	@$(ECHO) >> $(RESULTFILE)

	@$(ECHO) "Incorrect argument for remove." >> $(RESULTFILE)
	@cimsub -rx >> $(RESULTFILE) $(REDIRECTERROR)
	@$(ECHO) >> $(RESULTFILE)

	@$(ECHO) "Required option missing, handler not specified." >> $(RESULTFILE)
	@cimsub -rs -F filter1 >> $(RESULTFILE) $(REDIRECTERROR)
	@$(ECHO) >> $(RESULTFILE)

	@$(ECHO) "Required option missing, filter not specified." >> $(RESULTFILE)
	@cimsub -rs -H handler1 >> $(RESULTFILE) $(REDIRECTERROR)
	@$(ECHO) >> $(RESULTFILE)

	@$(ECHO) "Required option missing, handler not specified." >> $(RESULTFILE)
	@cimsub -ra -F filter1 >> $(RESULTFILE) $(REDIRECTERROR)
	@$(ECHO) >> $(RESULTFILE)

	@$(ECHO) "Required option missing, filter not specified." >> $(RESULTFILE)
	@cimsub -ra -H handler1 >> $(RESULTFILE) $(REDIRECTERROR)
	@$(ECHO) >> $(RESULTFILE)

	@$(ECHO) "Invalid option, handler is required." >> $(RESULTFILE)
	@cimsub -rh -F filter1 >> $(RESULTFILE) $(REDIRECTERROR)
	@$(ECHO) >> $(RESULTFILE)

	@$(ECHO) "Non-existent handler." >> $(RESULTFILE)
	@cimsub -rh -H no_handler_around >> $(RESULTFILE) $(REDIRECTERROR)
	@$(ECHO) >> $(RESULTFILE)

	@$(ECHO) "Remove handler with different creation class." >> $(RESULTFILE)
	@cimsub -rh -H test/CimsubTestNS0:Snmphandler3 >> $(RESULTFILE) $(REDIRECTERROR)
	@$(ECHO) >> $(RESULTFILE)

	@$(ECHO) "Invalid option, filter is required." >> $(RESULTFILE)
	@cimsub -rf -H handler1 >> $(RESULTFILE) $(REDIRECTERROR)
	@$(ECHO) >> $(RESULTFILE)

	@$(ECHO) "Try to remove non-existent filter." >> $(RESULTFILE)
	@cimsub -rf -F Filterxxx >> $(RESULTFILE) $(REDIRECTERROR)
	@$(ECHO) >> $(RESULTFILE)

	@$(ECHO) "Try to remove non-existent filter." >> $(RESULTFILE)
	@cimsub -rf -F test/static:Filter01 >> $(RESULTFILE) $(REDIRECTERROR)
	@$(ECHO) >> $(RESULTFILE)

	@$(ECHO) "List all subscriptions." >> $(RESULTFILE)
	@cimsub -ls >> $(RESULTFILE) $(REDIRECTERROR)
	@$(ECHO) >> $(RESULTFILE)

	@$(ECHO) "List all filters." >> $(RESULTFILE)
	@cimsub -lf  >> $(RESULTFILE) $(REDIRECTERROR)
	@$(ECHO) >> $(RESULTFILE)

	@$(ECHO) "List all handlers." >> $(RESULTFILE)
	@cimsub -lh >> $(RESULTFILE) $(REDIRECTERROR)
	@$(ECHO) >> $(RESULTFILE)

	@$(ECHO) "List all subscriptions(verbose)." >> $(RESULTFILE)
	@cimsub -ls -v >> $(RESULTFILE) $(REDIRECTERROR)
	@$(ECHO) >> $(RESULTFILE)

	@$(ECHO) "List all filters(verbose)." >> $(RESULTFILE)
	@cimsub -lf -v >> $(RESULTFILE) $(REDIRECTERROR)
	@$(ECHO) >> $(RESULTFILE)

	@$(ECHO) "List all handlers(verbose)." >> $(RESULTFILE)
	@cimsub -lh -v >> $(RESULTFILE)  $(REDIRECTERROR)
	@$(ECHO) >> $(RESULTFILE)
	@$(ECHO) >> $(RESULTFILE)

	@$(ECHO) "List subscription with filter and handler." >> $(RESULTFILE)
	@$(ECHO) "    Incorrect non-existent namespace." >> $(RESULTFILE)
	@cimsub -ls -nfoons/foons2 -F Filter01 -H ListenerDestination01  >> $(RESULTFILE) $(REDIRECTERROR)
	@$(ECHO) >> $(RESULTFILE)

	@$(ECHO) "List subscription with filter and handler." >> $(RESULTFILE)
	@$(ECHO) "    Subscription does not exist in non-default namespace ." >> $(RESULTFILE)
	@cimsub -ls -n test/CimsubTestNS1 -F Filter12 -H ListenerDestination06 >> $(RESULTFILE) $(REDIRECTERROR)
	@$(ECHO) >> $(RESULTFILE)

	@$(ECHO) "List subscription with valid namespace for filter." >> $(RESULTFILE)
	@$(ECHO) "    Subscription does not exist in non-default namespace ." >> $(RESULTFILE)
	@cimsub -ls -n test/CimsubTestNS1 -F test/CimsubTestNS1:Filter12 -H ListenerDestination06 >> $(RESULTFILE) $(REDIRECTERROR)
	@$(ECHO) >> $(RESULTFILE)

	@$(ECHO) "List subscription with valid namespace for handler." >> $(RESULTFILE)
	@$(ECHO) "    Subscription does not exist in non-default namespace ." >> $(RESULTFILE)
	@cimsub -ls -n test/CimsubTestNS1 -F Filter12 -H test/CimsubTestNS2:ListenerDestination06 >> $(RESULTFILE) $(REDIRECTERROR)
	@$(ECHO) >> $(RESULTFILE)

	@$(ECHO) "List subscription with valid namespace for filter and handler." >> $(RESULTFILE)
	@$(ECHO) "    Subscription exists in non-default namespace ." >> $(RESULTFILE)
	@cimsub -ls -n test/CimsubTestNS2 -F test/CimsubTestNS1:Filter12 -H test/CimsubTestNS3:ListenerDestination06 >> $(RESULTFILE) $(REDIRECTERROR)
	@$(ECHO) >> $(RESULTFILE)

	@$(ECHO) "List subscription with no namespace for filter." >> $(RESULTFILE)
	@cimsub -ls -n test/CimsubTestNS2 -F :Filter12 -H test/CimsubTestNS3:ListenerDestination06 >> $(RESULTFILE) $(REDIRECTERROR)
	@$(ECHO) >> $(RESULTFILE)

	@$(ECHO) "List subscription with no namespace for handler." >> $(RESULTFILE)
	@cimsub -ls -n test/CimsubTestNS2 -F test/CimsubTestNS1:Filter12 -H :ListenerDestination06 >> $(RESULTFILE) $(REDIRECTERROR)
	@$(ECHO) >> $(RESULTFILE)

	@$(ECHO) "List subscription with no creationclass for handler." >> $(RESULTFILE)
	@cimsub -ls -n test/CimsubTestNS2 -F test/CimsubTestNS1:Filter12 -H test/CimsubTestNS3.:ListenerDestination06 >> $(RESULTFILE) $(REDIRECTERROR)
	@$(ECHO) >> $(RESULTFILE)

	@$(ECHO) "List subscription with no name for handler." >> $(RESULTFILE)
	@cimsub -ls -n test/CimsubTestNS0 -F test/CimsubTestNS0:Filter01 -H test/CimsubTestNS0:CIM_IndicationHandlerCIMXML. >> $(RESULTFILE) $(REDIRECTERROR)
	@$(ECHO) >> $(RESULTFILE)

	@$(ECHO) "List subscription with no name for handler." >> $(RESULTFILE)
	@cimsub -ls -n test/CimsubTestNS2 -F test/CimsubTestNS1:Filter12 -H test/CimsubTestNS3: >> $(RESULTFILE) $(REDIRECTERROR)
	@$(ECHO) >> $(RESULTFILE)

	@$(ECHO) "List subscription with no name for filter." >> $(RESULTFILE)
	@cimsub -ls -n test/CimsubTestNS2 -F test/CimsubTestNS1: -H test/CimsubTestNS3:ListenerDestination06 >> $(RESULTFILE) $(REDIRECTERROR)
	@$(ECHO) >> $(RESULTFILE)

	@$(ECHO) "List subscription with full handler path." >> $(RESULTFILE)
	@cimsub -ls -n test/CimsubTestNS0 -F test/CimsubTestNS0:Filter01 -H test/CimsubTestNS0:CIM_IndicationHandlerCIMXML.Handler01 >> $(RESULTFILE) $(REDIRECTERROR)
	@$(ECHO) >> $(RESULTFILE)

	@$(ECHO) "List CIM_IndicationHandlerCIMXML handler with full handler path." >> $(RESULTFILE)
	@cimsub -lh -H test/CimsubTestNS0:CIM_IndicationHandlerCIMXML.Handler01 >> $(RESULTFILE) $(REDIRECTERROR)
	@$(ECHO) >> $(RESULTFILE)

	@$(ECHO) "List CIM_ListenerDestinationCIMXML handler with full handler path." >> $(RESULTFILE)
	@cimsub -lh -H test/CimsubTestNS3:CIM_ListenerDestinationCIMXML.ListenerDestination06 >> $(RESULTFILE) $(REDIRECTERROR)
	@$(ECHO) >> $(RESULTFILE)

	@$(ECHO) "List PG_IndicationHandlerSNMPMapper handler with full handler path." >> $(RESULTFILE)
	@cimsub -lh -H test/CimsubTestNS0:PG_IndicationHandlerSNMPMapper.Handler03 >> $(RESULTFILE) $(REDIRECTERROR)
	@$(ECHO) >> $(RESULTFILE)

	@$(ECHO) "List verbose CIM_IndicationHandlerCIMXML handler with full handler path." >> $(RESULTFILE)
	@cimsub -lh -v -H test/CimsubTestNS0:CIM_IndicationHandlerCIMXML.Handler01 >> $(RESULTFILE) $(REDIRECTERROR)
	@$(ECHO) >> $(RESULTFILE)

	@$(ECHO) "List verbose CIM_ListenerDestinationCIMXML handler with full handler path." >> $(RESULTFILE)
	@cimsub -lh -v -H test/CimsubTestNS3:CIM_ListenerDestinationCIMXML.ListenerDestination06 >> $(RESULTFILE) $(REDIRECTERROR)
	@$(ECHO) >> $(RESULTFILE)

	@$(ECHO) "List verbose PG_IndicationHandlerSNMPMapper handler with full handler path." >> $(RESULTFILE)
	@cimsub -lh -v -H test/CimsubTestNS0:PG_IndicationHandlerSNMPMapper.Handler03 >> $(RESULTFILE) $(REDIRECTERROR)
	@$(ECHO) >> $(RESULTFILE)

	@$(ECHO) "List test subscription." >> $(RESULTFILE)
	@cimsub -ls -n test/CimsubTestNS0 -F Filter01 -H ListenerDestination01 >> $(RESULTFILE) $(REDIRECTERROR)
	@$(ECHO) >> $(RESULTFILE)

	@$(ECHO) "Enable the already enabled test subscription." >> $(RESULTFILE)
	@cimsub -e -n test/CimsubTestNS0 -F Filter01 -H ListenerDestination01 >> $(RESULTFILE) $(REDIRECTERROR)
	@$(ECHO) >> $(RESULTFILE)

	@$(ECHO) "List test subscription." >> $(RESULTFILE)
	@cimsub -ls -n test/CimsubTestNS0 -F Filter01 -H ListenerDestination01 >> $(RESULTFILE) $(REDIRECTERROR)
	@$(ECHO) >> $(RESULTFILE)

	@$(ECHO) "Disable the test subscription." >> $(RESULTFILE)
	@cimsub -d -n test/CimsubTestNS0 -F Filter01 -H ListenerDestination01 >> $(RESULTFILE) $(REDIRECTERROR)
	@$(ECHO) >> $(RESULTFILE)

	@$(ECHO) "List test subscription." >> $(RESULTFILE)
	@cimsub -ls -n test/CimsubTestNS0 -F Filter01 -H ListenerDestination01 >> $(RESULTFILE) $(REDIRECTERROR)
	@$(ECHO) >> $(RESULTFILE)

	@$(ECHO) "Try to disable the test subscription again." >> $(RESULTFILE)
	@cimsub -d -n test/CimsubTestNS0 -F Filter01 -H ListenerDestination01 >> $(RESULTFILE) $(REDIRECTERROR)
	@$(ECHO) >> $(RESULTFILE)

	@$(ECHO) "List test subscription." >> $(RESULTFILE)
	@cimsub -ls -n test/CimsubTestNS0 -F Filter01 -H ListenerDestination01 >> $(RESULTFILE) $(REDIRECTERROR)
	@$(ECHO) >> $(RESULTFILE)

	@$(ECHO) "Enable the test subscription." >> $(RESULTFILE)
	@cimsub -e -n test/CimsubTestNS0 -F Filter01 -H ListenerDestination01 >> $(RESULTFILE) $(REDIRECTERROR)
	@$(ECHO) >> $(RESULTFILE)

	@$(ECHO) "List test subscription." >> $(RESULTFILE)
	@cimsub -ls -n test/CimsubTestNS0 -F Filter01 -H ListenerDestination01 >> $(RESULTFILE) $(REDIRECTERROR)
	@$(ECHO) >> $(RESULTFILE)

	@$(ECHO) "List test subscription." >> $(RESULTFILE)
	@cimsub -ls -n test/CimsubTestNS2 -F test/CimsubTestNS1:Filter12 -H test/CimsubTestNS3:ListenerDestination06 >> $(RESULTFILE) $(REDIRECTERROR)
	@$(ECHO) >> $(RESULTFILE)

	@$(ECHO) "Enable the already enabled test subscription." >> $(RESULTFILE)
	@cimsub -e -n test/CimsubTestNS2 -F test/CimsubTestNS1:Filter12 -H test/CimsubTestNS3:ListenerDestination06 >> $(RESULTFILE) $(REDIRECTERROR)
	@$(ECHO) >> $(RESULTFILE)

	@$(ECHO) "List test subscription." >> $(RESULTFILE)
	@cimsub -ls -n test/CimsubTestNS2 -F test/CimsubTestNS1:Filter12 -H test/CimsubTestNS3:ListenerDestination06 >> $(RESULTFILE) $(REDIRECTERROR)
	@$(ECHO) >> $(RESULTFILE)

	@$(ECHO) "Disable the test subscription." >> $(RESULTFILE)
	@cimsub -d -n test/CimsubTestNS2 -F test/CimsubTestNS1:Filter12 -H test/CimsubTestNS3:ListenerDestination06 >> $(RESULTFILE) $(REDIRECTERROR)
	@$(ECHO) >> $(RESULTFILE)

	@$(ECHO) "List test subscription." >> $(RESULTFILE)
	@cimsub -ls -n test/CimsubTestNS2 -F test/CimsubTestNS1:Filter12 -H test/CimsubTestNS3:ListenerDestination06 >> $(RESULTFILE) $(REDIRECTERROR)
	@$(ECHO) >> $(RESULTFILE)

	@$(ECHO) "Enable the test subscription." >> $(RESULTFILE)
	@cimsub -e -n test/CimsubTestNS2 -F test/CimsubTestNS1:Filter12 -H test/CimsubTestNS3:ListenerDestination06 >> $(RESULTFILE) $(REDIRECTERROR)
	@$(ECHO) >> $(RESULTFILE)

	@$(ECHO) "List test subscription." >> $(RESULTFILE)
	@cimsub -ls -n test/CimsubTestNS2 -F test/CimsubTestNS1:Filter12 -H test/CimsubTestNS3:ListenerDestination06 >> $(RESULTFILE) $(REDIRECTERROR)
	@$(ECHO) >> $(RESULTFILE)

	@$(ECHO) "List test subscription." >> $(RESULTFILE)
	@cimsub -ls -n test/CimsubTestNS0 -F test/CimsubTestNS1:Filter12 -H test/CimsubTestNS2:ListenerDestination06 >> $(RESULTFILE) $(REDIRECTERROR)
	@$(ECHO) >> $(RESULTFILE)

	@$(ECHO) "Disable the test subscription." >> $(RESULTFILE)
	@cimsub -d -n test/CimsubTestNS0 -F test/CimsubTestNS1:Filter12 -H test/CimsubTestNS2:ListenerDestination06 >> $(RESULTFILE) $(REDIRECTERROR)
	@$(ECHO) >> $(RESULTFILE)

	@$(ECHO) "List test subscription." >> $(RESULTFILE)
	@cimsub -ls -n test/CimsubTestNS0 -F test/CimsubTestNS1:Filter12 -H test/CimsubTestNS2:ListenerDestination06 >> $(RESULTFILE) $(REDIRECTERROR)
	@$(ECHO) >> $(RESULTFILE)

	@$(ECHO) "Re-enable the test subscription." >> $(RESULTFILE)
	@cimsub -e -n test/CimsubTestNS0 -F test/CimsubTestNS1:Filter12 -H test/CimsubTestNS2:ListenerDestination06 >> $(RESULTFILE) $(REDIRECTERROR)
	@$(ECHO) >> $(RESULTFILE)

	@$(ECHO) "List test subscription." >> $(RESULTFILE)
	@cimsub -ls -n test/CimsubTestNS0 -F test/CimsubTestNS1:Filter12 -H test/CimsubTestNS2:ListenerDestination06 >> $(RESULTFILE) $(REDIRECTERROR)
	@$(ECHO) >> $(RESULTFILE)

	@$(ECHO) "List test subscription." >> $(RESULTFILE)
	@cimsub -ls -n test/CimsubTestNS0 -F test/CimsubTestNS1:Filter12 -H test/CimsubTestNS2:ListenerDestination06 >> $(RESULTFILE) $(REDIRECTERROR)
	@$(ECHO) >> $(RESULTFILE)

	@$(ECHO) "Disable the test subscription." >> $(RESULTFILE)
	@$(ECHO) "    Specify subscription namespace." >> $(RESULTFILE)
	@cimsub -d -n test/CimsubTestNS0 -F test/CimsubTestNS1:Filter12 -H test/CimsubTestNS2:ListenerDestination06 >> $(RESULTFILE) $(REDIRECTERROR)
	@$(ECHO) >> $(RESULTFILE)

	@$(ECHO) "List test subscription." >> $(RESULTFILE)
	@cimsub -ls -n test/CimsubTestNS0 -F test/CimsubTestNS1:Filter12 -H test/CimsubTestNS2:ListenerDestination06 >> $(RESULTFILE) $(REDIRECTERROR)
	@$(ECHO) >> $(RESULTFILE)

	@$(ECHO) "Re-enable the test subscription." >> $(RESULTFILE)
	@$(ECHO) "    Specify subscription namespace." >> $(RESULTFILE)
	@cimsub -e -n test/CimsubTestNS0 -F test/CimsubTestNS1:Filter12 -H test/CimsubTestNS2:ListenerDestination06 >> $(RESULTFILE) $(REDIRECTERROR)
	@$(ECHO) >> $(RESULTFILE)

	@$(ECHO) "List test subscription." >> $(RESULTFILE)
	@cimsub -ls -n test/CimsubTestNS0 -F test/CimsubTestNS1:Filter12 -H test/CimsubTestNS2:ListenerDestination06 >> $(RESULTFILE) $(REDIRECTERROR)
	@$(ECHO) >> $(RESULTFILE)

	@$(ECHO) "List test subscription." >> $(RESULTFILE)
	@cimsub -ls -n test/CimsubTestNS2 -F test/CimsubTestNS1:Filter12 -H test/CimsubTestNS3:ListenerDestination06 >> $(RESULTFILE) $(REDIRECTERROR)
	@$(ECHO) >> $(RESULTFILE)

	@$(ECHO) "Disable the test subscription." >> $(RESULTFILE)
	@$(ECHO) "    Specify subscription namespace." >> $(RESULTFILE)
	@cimsub -d -n test/CimsubTestNS2 -F test/CimsubTestNS1:Filter12 -H test/CimsubTestNS3:ListenerDestination06 >> $(RESULTFILE) $(REDIRECTERROR)
	@$(ECHO) >> $(RESULTFILE)

	@$(ECHO) "List test subscription." >> $(RESULTFILE)
	@cimsub -ls -n test/CimsubTestNS2 -F test/CimsubTestNS1:Filter12 -H test/CimsubTestNS3:ListenerDestination06 >> $(RESULTFILE) $(REDIRECTERROR)
	@$(ECHO) >> $(RESULTFILE)

	@$(ECHO) "Re-enable the test subscription." >> $(RESULTFILE)
	@$(ECHO) "    Specify subscription namespace." >> $(RESULTFILE)
	@cimsub -e -n test/CimsubTestNS2 -F test/CimsubTestNS1:Filter12 -H test/CimsubTestNS3:ListenerDestination06 >> $(RESULTFILE) $(REDIRECTERROR)
	@$(ECHO) >> $(RESULTFILE)

	@$(ECHO) "List test subscription." >> $(RESULTFILE)
	@cimsub -ls -n test/CimsubTestNS2 -F test/CimsubTestNS1:Filter12 -H test/CimsubTestNS3:ListenerDestination06 >> $(RESULTFILE) $(REDIRECTERROR)
	@$(ECHO) >> $(RESULTFILE)

	@$(ECHO) "List test subscription." >> $(RESULTFILE)
	@$(ECHO) "   Specify all namespaces." >> $(RESULTFILE)
	@cimsub -ls -n test/CimsubTestNS3 -F test/CimsubTestNS1:Filter12 -H test/CimsubTestNS2:ListenerDestination06 >> $(RESULTFILE) $(REDIRECTERROR)
	@$(ECHO) >> $(RESULTFILE)

	@$(ECHO) "Remove test subscription." >> $(RESULTFILE)
	@$(ECHO) "    Specify all namespaces." >> $(RESULTFILE)
	@cimsub -rs -n test/CimsubTestNS3 -F test/CimsubTestNS1:Filter12 -H test/CimsubTestNS2:ListenerDestination06 >> $(RESULTFILE) $(REDIRECTERROR)
	@$(ECHO) >> $(RESULTFILE)

	@$(ECHO) "List test subscription." >> $(RESULTFILE)
	@$(ECHO) "    Specify all namespaces." >> $(RESULTFILE)
	@cimsub -ls -n test/CimsubTestNS3 -F test/CimsubTestNS1:Filter11 -H test/CimsubTestNS2:ListenerDestination06 >> $(RESULTFILE) $(REDIRECTERROR)
	@$(ECHO) >> $(RESULTFILE)

	@$(ECHO) "List test filter." >> $(RESULTFILE)
	@cimsub -lf -F test/CimsubTestNS0:Filter02 >> $(RESULTFILE) $(REDIRECTERROR)
	@$(ECHO) >> $(RESULTFILE)

	@$(ECHO) "Try to remove test filter." >> $(RESULTFILE)
	@$(ECHO) "    The filter is still referenced by a suscription." >> $(RESULTFILE)
	@cimsub -rf -F test/CimsubTestNS0:Filter02 >> $(RESULTFILE) $(REDIRECTERROR)
	@$(ECHO) >> $(RESULTFILE)

	@$(ECHO) "List test subscription." >> $(RESULTFILE)
	@$(ECHO) "    This is the only reference to the test filter." >> $(RESULTFILE)
	@cimsub -ls -n test/CimsubTestNS0 -F test/CimsubTestNS0:Filter02 -H test/CimsubTestNS0:CIM_IndicationHandlerCIMXML.Handler01 >> $(RESULTFILE) $(REDIRECTERROR)
	@$(ECHO) >> $(RESULTFILE)

	@$(ECHO) "Remove test subscription." >> $(RESULTFILE)
	@$(ECHO) "    This is the only reference to the test filter." >> $(RESULTFILE)
	@cimsub -rs -n test/CimsubTestNS0 -F test/CimsubTestNS0:Filter02 -H test/CimsubTestNS0:CIM_IndicationHandlerCIMXML.Handler01 >> $(RESULTFILE) $(REDIRECTERROR)
	@$(ECHO) >> $(RESULTFILE)

	@$(ECHO) "List test subscription." >> $(RESULTFILE)
	@$(ECHO) "    This is the only reference to the test filter." >> $(RESULTFILE)
	@cimsub -ls -n test/CimsubTestNS0 -F test/CimsubTestNS0:Filter02 -H test/CimsubTestNS0:CIM_IndicationHandlerCIMXML.Handler01 >> $(RESULTFILE) $(REDIRECTERROR)
	@$(ECHO) >> $(RESULTFILE)

	@$(ECHO) "Try to remove test filter." >> $(RESULTFILE)
	@$(ECHO) "    The filter should be removed." >> $(RESULTFILE)
	@cimsub -rf -F test/CimsubTestNS0:Filter02 >> $(RESULTFILE) $(REDIRECTERROR)
	@$(ECHO) >> $(RESULTFILE)

	@$(ECHO) "List test filter." >> $(RESULTFILE)
	@cimsub -lf -F test/CimsubTestNS0:Filter02 >> $(RESULTFILE) $(REDIRECTERROR)
	@$(ECHO) >> $(RESULTFILE)

	@$(ECHO) "List persistent test handler." >> $(RESULTFILE)
	@cimsub -lh -v -H test/CimsubTestNS0:CIM_IndicationHandlerCIMXML.Handler02p >> $(RESULTFILE) $(REDIRECTERROR)
	@$(ECHO) >> $(RESULTFILE)

	@$(ECHO) "Try to remove persistent test handler." >> $(RESULTFILE)
	@$(ECHO) "    The handler is still referenced by a subscription." >> $(RESULTFILE)
	@cimsub -rh -H test/CimsubTestNS0:CIM_IndicationHandlerCIMXML.Handler02p >> $(RESULTFILE) $(REDIRECTERROR)
	@$(ECHO) >> $(RESULTFILE)

	@$(ECHO) "List test subscription." >> $(RESULTFILE)
	@$(ECHO) "    This is the only reference to the persistent test handler." >> $(RESULTFILE)
	@cimsub -ls -n test/CimsubTestNS0 -F test/CimsubTestNS0:Filter01 -H test/CimsubTestNS0:CIM_IndicationHandlerCIMXML.Handler02p >> $(RESULTFILE) $(REDIRECTERROR)
	@$(ECHO) >> $(RESULTFILE)

	@$(ECHO) "Remove test subscription." >> $(RESULTFILE)
	@$(ECHO) "    This is the only reference to the persistent test handler." >> $(RESULTFILE)
	@cimsub -rs -n test/CimsubTestNS0 -F test/CimsubTestNS0:Filter01 -H test/CimsubTestNS0:CIM_IndicationHandlerCIMXML.Handler02p >> $(RESULTFILE) $(REDIRECTERROR)
	@$(ECHO) >> $(RESULTFILE)

	@$(ECHO) "List test subscription." >> $(RESULTFILE)
	@$(ECHO) "    This is the only reference to the persistent test handler." >> $(RESULTFILE)
	@cimsub -ls -n test/CimsubTestNS0 -F Filter01 -H CIM_IndicationHandlerCIMXML.Handler02p >> $(RESULTFILE) $(REDIRECTERROR)
	@$(ECHO) >> $(RESULTFILE)

	@$(ECHO) "Remove persistent test handler." >> $(RESULTFILE)
	@$(ECHO) "    The handler should be removed." >> $(RESULTFILE)
	@cimsub -rh -H test/CimsubTestNS0:CIM_IndicationHandlerCIMXML.Handler02p >> $(RESULTFILE) $(REDIRECTERROR)
	@$(ECHO) >> $(RESULTFILE)

	@$(ECHO) "List persistent test handler." >> $(RESULTFILE)
	@cimsub -lh -H test/CimsubTestNS0:CIM_IndicationHandlerCIMXML.Handler02p >> $(RESULTFILE) $(REDIRECTERROR)
	@$(ECHO) >> $(RESULTFILE)

	@$(ECHO) "List transient test handler." >> $(RESULTFILE)
	@cimsub -lh -v -H test/CimsubTestNS0:CIM_IndicationHandlerCIMXML.Handler02t >> $(RESULTFILE) $(REDIRECTERROR)
	@$(ECHO) >> $(RESULTFILE)

	@$(ECHO) "List test subscription." >> $(RESULTFILE)
	@$(ECHO) "    This is the only reference to the transient test handler." >> $(RESULTFILE)
	@cimsub -ls -n test/CimsubTestNS0 -F Filter01 -H CIM_IndicationHandlerCIMXML.Handler02t >> $(RESULTFILE) $(REDIRECTERROR)
	@$(ECHO) >> $(RESULTFILE)

	@$(ECHO) "Remove the transient test handler." >> $(RESULTFILE)
	@cimsub -rh -H test/CimsubTestNS0:CIM_IndicationHandlerCIMXML.Handler02t >> $(RESULTFILE) $(REDIRECTERROR)
	@$(ECHO) >> $(RESULTFILE)

	@$(ECHO) "List transient test handler." >> $(RESULTFILE)
	@$(ECHO) "    The transient test handler should have been removed." >> $(RESULTFILE)
	@cimsub -lh -H test/CimsubTestNS0:CIM_IndicationHandlerCIMXML.Handler02t >> $(RESULTFILE) $(REDIRECTERROR)
	@$(ECHO) >> $(RESULTFILE)

	@$(ECHO) "List test subscription." >> $(RESULTFILE)
	@$(ECHO) "    The test subscription should have been removed." >> $(RESULTFILE)
	@cimsub -ls -n test/CimsubTestNS0 -F Filter01 -H CIM_IndicationHandlerCIMXML.Handler02t >> $(RESULTFILE) $(REDIRECTERROR)
	@$(ECHO) >> $(RESULTFILE)

	@$(ECHO) "List transient test handler." >> $(RESULTFILE)
	@cimsub -lh -H test/CimsubTestNS0:ListenerDestination01t >> $(RESULTFILE) $(REDIRECTERROR)
	@$(ECHO) >> $(RESULTFILE)

	@$(ECHO) "List test subscription." >> $(RESULTFILE)
	@$(ECHO) "    This is the only reference to the transient test handler." >> $(RESULTFILE)
	@cimsub -ls -n test/CimsubTestNS0 -F Filter01 -H ListenerDestination01t >> $(RESULTFILE) $(REDIRECTERROR)
	@$(ECHO) >> $(RESULTFILE)

	@$(ECHO) "Remove the test subscription." >> $(RESULTFILE)
	@$(ECHO) "    The test handler is only referenced by this subscription." >> $(RESULTFILE)
	@cimsub -rs -n test/CimsubTestNS0 -F Filter01 -H ListenerDestination01t >> $(RESULTFILE) $(REDIRECTERROR)
	@$(ECHO) >> $(RESULTFILE)

	@$(ECHO) "List test subscription." >> $(RESULTFILE)
	@$(ECHO) "    The test subscription should have been removed." >> $(RESULTFILE)
	@cimsub -ls -n test/CimsubTestNS0 -F Filter01 -H ListenerDestination01t >> $(RESULTFILE) $(REDIRECTERROR)
	@$(ECHO) >> $(RESULTFILE)

	@$(ECHO) "List transient test handler." >> $(RESULTFILE)
	@$(ECHO) "    The transient test handler should still exist." >> $(RESULTFILE)
	@cimsub -lh -H test/CimsubTestNS0:ListenerDestination01t >> $(RESULTFILE) $(REDIRECTERROR)
	@$(ECHO) >> $(RESULTFILE)

	@$(ECHO) "List test subscription." >> $(RESULTFILE)
	@cimsub -ls -n test/CimsubTestNS0 -F Filter09 -H ListenerDestination03 >> $(RESULTFILE) $(REDIRECTERROR)
	@$(ECHO) >> $(RESULTFILE)

	@$(ECHO) "List test handler." >> $(RESULTFILE)
	@cimsub -lh -H test/CimsubTestNS0:ListenerDestination03 >> $(RESULTFILE) $(REDIRECTERROR)
	@$(ECHO) >> $(RESULTFILE)

	@$(ECHO) "List test filter." >> $(RESULTFILE)
	@cimsub -lf -F test/CimsubTestNS0:Filter09 >> $(RESULTFILE) $(REDIRECTERROR)
	@$(ECHO) >> $(RESULTFILE)

	@$(ECHO) "Remove the test subscription,filter and handler." >> $(RESULTFILE)
	@cimsub -ra -n test/CimsubTestNS0 -F Filter09  -H ListenerDestination03 >> $(RESULTFILE) $(REDIRECTERROR)
	@$(ECHO) >> $(RESULTFILE)

	@$(ECHO) "List test subscription." >> $(RESULTFILE)
	@$(ECHO) "    The test subscription should have been removed." >> $(RESULTFILE)
	@cimsub -ls -n test/CimsubTestNS0 -F Filter09 -H ListenerDestination03 >> $(RESULTFILE) $(REDIRECTERROR)
	@$(ECHO) >> $(RESULTFILE)

	@$(ECHO) "List test handler." >> $(RESULTFILE)
	@$(ECHO) "    The test handler should have been removed." >> $(RESULTFILE)
	@cimsub -lh -H test/CimsubTestNS0:ListenerDestination03 >> $(RESULTFILE) $(REDIRECTERROR)
	@$(ECHO) >> $(RESULTFILE)

	@$(ECHO) "List test filter." >> $(RESULTFILE)
	@$(ECHO) "    The test filter should have been removed." >> $(RESULTFILE)
	@cimsub -lf -F test/CimsubTestNS0:Filter09 >> $(RESULTFILE) $(REDIRECTERROR)
	@$(ECHO) >> $(RESULTFILE)

	@$(ECHO) "List test filter." >> $(RESULTFILE)
	@cimsub -lf -F test/CimsubTestNS1:Filter13 >> $(RESULTFILE) $(REDIRECTERROR)
	@$(ECHO) >> $(RESULTFILE)

	@$(ECHO) "Remove test filter." >> $(RESULTFILE)
	@$(ECHO) "    The filter should be removed." >> $(RESULTFILE)
	@cimsub -rf -F test/CimsubTestNS1:Filter13 >> $(RESULTFILE) $(REDIRECTERROR)
	@$(ECHO) >> $(RESULTFILE)

	@$(ECHO) "List test filter." >> $(RESULTFILE)
	@cimsub -lf -F test/CimsubTestNS1:Filter13 >> $(RESULTFILE) $(REDIRECTERROR)
	@$(ECHO) >> $(RESULTFILE)

	@$(ECHO) "Remove the test subscription." >> $(RESULTFILE)
	@$(ECHO) "    Valid filter and nonexistent handler." >> $(RESULTFILE)
	@cimsub -rs -n test/CimsubTestNS0 -F Filter01 -H invalidHandler >> $(RESULTFILE) $(REDIRECTERROR)
	@$(ECHO) >> $(RESULTFILE)

	@$(ECHO) "Remove the test subscription." >> $(RESULTFILE)
	@$(ECHO) "    Nonexistent filter and valid handler." >> $(RESULTFILE)
	@cimsub -rs -n test/CimsubTestNS0 -F invalidFilter -H Handler01 >> $(RESULTFILE) $(REDIRECTERROR)
	@$(ECHO) >> $(RESULTFILE)

	@$(ECHO) "List an expired subscription." >> $(RESULTFILE)
	@cimsub -ls -v -n test/CimsubTestNS0 -F Filter15 -H Handler15 >> $(RESULTFILE) $(REDIRECTERROR)
	@$(ECHO) >> $(RESULTFILE)

	@$(ECHO) "Disable an expired subscription." >> $(RESULTFILE)
	@cimsub -d -n test/CimsubTestNS0 -F Filter15 -H Handler15 >> $(RESULTFILE) $(REDIRECTERROR)
	@$(ECHO) >> $(RESULTFILE)

	@$(ECHO) "List an expired subscription." >> $(RESULTFILE)
	@$(ECHO) "The subscription should have been removed." >> $(RESULTFILE)
	@cimsub -ls -v -n test/CimsubTestNS0 -F Filter15 -H Handler15 >> $(RESULTFILE) $(REDIRECTERROR)
	@$(ECHO) >> $(RESULTFILE)

	@$(ECHO) " Execute batch file." >> $(RESULTFILE)
	@cimsub -b ../testbatchfile/batchfile.batch >> $(RESULTFILE) $(REDIRECTERROR)
	@$(ECHO) >> $(RESULTFILE)
	
	@$(ECHO) ++++cimsub tests completed


cimsubTestEmail:
	@$(ECHO) ++++cimsub email handler tests
	@$(ECHO)  +++++++++ Command line testing +++++++++ >> $(RESULTFILE_EMAIL)

	@$(ECHO) "List PG_ListenerDestinationEmail handler with full handler path." >> $(RESULTFILE_EMAIL)
	@cimsub -lh -H test/CimsubTestNS0:PG_ListenerDestinationEmail.EmailHandler01 >> $(RESULTFILE_EMAIL) $(REDIRECTERROR)
	@$(ECHO) >> $(RESULTFILE_EMAIL)

	@$(ECHO) "List verbose PG_ListenerDestinationEmail handler with full handler path." >> $(RESULTFILE_EMAIL)
	@cimsub -lh -v -H test/CimsubTestNS0:PG_ListenerDestinationEmail.EmailHandler01 >> $(RESULTFILE_EMAIL) $(REDIRECTERROR)
	@$(ECHO) >> $(RESULTFILE_EMAIL)

	@$(ECHO) "List subscriptions with email handler." >> $(RESULTFILE_EMAIL)
	@cimsub -ls -H test/CimsubTestNS0:PG_ListenerDestinationEmail.EmailHandler01 >> $(RESULTFILE_EMAIL) $(REDIRECTERROR)
	@$(ECHO) >> $(RESULTFILE_EMAIL)

	@$(ECHO) "List verbose subscriptions with email handler." >> $(RESULTFILE_EMAIL)
	@cimsub -ls -v -H test/CimsubTestNS0:PG_ListenerDestinationEmail.EmailHandler01 >> $(RESULTFILE_EMAIL) $(REDIRECTERROR)
	@$(ECHO) >> $(RESULTFILE_EMAIL)

	@$(ECHO) "List all subscriptions." >> $(RESULTFILE_EMAIL)
	@cimsub -ls >> $(RESULTFILE_EMAIL) $(REDIRECTERROR)
	@$(ECHO) >> $(RESULTFILE_EMAIL)

	@$(ECHO) "List verbose all subscriptions." >> $(RESULTFILE_EMAIL)
	@cimsub -ls -v >> $(RESULTFILE_EMAIL) $(REDIRECTERROR)
	@$(ECHO) >> $(RESULTFILE_EMAIL)
	@$(STRIPCRS)
	$(COMPARERESULTS)

	@$(ECHO) ++++cimsub email handler tests completed

cimsubTestSyslog:
	@$(ECHO) ++++cimsub syslog handler tests
	@$(ECHO)  +++++++++ Command line testing +++++++++ >> $(RESULTFILE_SYSLOG)
	@$(ECHO) "List PG_ListenerDestinationSystemLog handler with full handler path." >> $(RESULTFILE_SYSLOG)
	@cimsub -lh -H test/CimsubTestNS0:PG_ListenerDestinationSystemLog.SyslogHandler01 >> $(RESULTFILE_SYSLOG) $(REDIRECTERROR)
	@$(ECHO) >> $(RESULTFILE_SYSLOG)

	@$(ECHO) "List verbose PG_ListenerDestinationSystemLog handler with full handler path." >> $(RESULTFILE_SYSLOG)
	@cimsub -lh -v -H test/CimsubTestNS0:PG_ListenerDestinationSystemLog.SyslogHandler01 >> $(RESULTFILE_SYSLOG) $(REDIRECTERROR)
	@$(ECHO) >> $(RESULTFILE_SYSLOG)

	@$(ECHO) "List subscriptions with syslog handler." >> $(RESULTFILE_SYSLOG)
	@cimsub -ls -H test/CimsubTestNS0:PG_ListenerDestinationSystemLog.SyslogHandler01 >> $(RESULTFILE_SYSLOG) $(REDIRECTERROR)
	@$(ECHO) >> $(RESULTFILE_SYSLOG)

	@$(ECHO) "List verbose subscriptions with syslog handler." >> $(RESULTFILE_SYSLOG)
	@cimsub -ls -v -H test/CimsubTestNS0:PG_ListenerDestinationSystemLog.SyslogHandler01 >> $(RESULTFILE_SYSLOG) $(REDIRECTERROR)
	@$(ECHO) >> $(RESULTFILE_SYSLOG)

	@$(ECHO) "List all subscriptions." >> $(RESULTFILE_SYSLOG)
	@cimsub -ls >> $(RESULTFILE_SYSLOG) $(REDIRECTERROR)
	@$(ECHO) >> $(RESULTFILE_SYSLOG)

	@$(ECHO) "List verbose all subscriptions." >> $(RESULTFILE_SYSLOG)
	@cimsub -ls -v >> $(RESULTFILE_SYSLOG) $(REDIRECTERROR)
	@$(ECHO) >> $(RESULTFILE_SYSLOG)
	@$(STRIPCRS)
	$(COMPARERESULTS)

	@$(ECHO) ++++cimsub syslog handler tests completed

depend:

general:

strip-license:

prepend-license:
