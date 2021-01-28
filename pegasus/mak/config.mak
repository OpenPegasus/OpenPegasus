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
################################################################################
##
## Get external environment variables. Note that all external environment
## variables begin with "PEGASUS_".
##
################################################################################

#PEGASUS_ROOT is mandatory currently
ifdef PEGASUS_ROOT
  ifeq ($(wildcard $(PEGASUS_ROOT)),)
    $(error PEGASUS_ROOT = $(PEGASUS_ROOT) is incorrect, \
      Did you meant to set it to $(CURDIR)?)
  endif
  ROOT =  $(subst \,/,$(PEGASUS_ROOT))
else
  $(error PEGASUS_ROOT environment variable undefined)
endif


ifdef PEGASUS_ENVVAR_FILE
    include $(PEGASUS_ENVVAR_FILE)
else
    include $(ROOT)/env_var.status
endif

ifdef PEGASUS_HOME
    HOME_DIR = $(subst \,/,$(PEGASUS_HOME))
else
    $(error PEGASUS_HOME environment variable undefined)
endif


ifdef PEGASUS_TMP
    TMP_DIR = $(subst \,/,$(PEGASUS_TMP))
else
    TMP_DIR = .
endif

ifdef PEGASUS_DISPLAYCONSUMER_DIR
    DISPLAYCONSUMER_DIR = $(subst \,/,$(PEGASUS_DISPLAYCONSUMER_DIR))
else
    DISPLAYCONSUMER_DIR = $(subst \,/,$(PEGASUS_HOME))
endif

ifdef PEGASUS_DEBUG
     PEGASUS_USE_DEBUG_BUILD_OPTIONS = 1
endif

PLATFORM_FILES=$(wildcard $(ROOT)/mak/platform*.mak)
PLATFORM_TEMP=$(subst $(ROOT)/mak/platform_,, $(PLATFORM_FILES))
VALID_PLATFORMS=$(subst .mak,  , $(PLATFORM_TEMP))

ifndef PEGASUS_PLATFORM
    $(error PEGASUS_PLATFORM environment variable undefined. Please set to\
        one of the following:$(VALID_PLATFORMS))
endif

################################################################################
ifeq ($(findstring _GNU, $(PEGASUS_PLATFORM)), _GNU)
    ifdef CXX
      GCC_VERSION = $(shell $(CXX) -dumpversion)
    else
      GCC_VERSION = $(shell g++ -dumpversion)
    endif
else
    GCC_VERSION =
endif

#############################################################################
## As a general rule, the directory structure for the object files mirrors
## the directory structure of the source files.  E.g.,
## $PEGASUS_HOME/obj/Pegasus/Common contains the object files for the
## source files in $PEGASUS_ROOT/src/Pegasus/Common.  Each source-level
## Makefile includes a DIR value that defines this common path (e.g.,
## Pegasus/Common). In a small number of cases, source files are built
## multiple times with difference compile options.
## To handle this situation, the ALT_OBJ_DIR variable can be used to
## specify an alternative object directory for use in building the
## objects defined in the Makefile.
##

ifndef ALT_OBJ_DIR
  ifeq ($(PEGASUS_INTERNAL_ENABLE_32BIT_PROVIDER_SUPPORT),true)
    OBJ_DIR = $(HOME_DIR)/obj32/$(DIR)
  else
    OBJ_DIR = $(HOME_DIR)/obj/$(DIR)
  endif
else
  ifeq ($(PEGASUS_INTERNAL_ENABLE_32BIT_PROVIDER_SUPPORT),true)
    OBJ_DIR = $(HOME_DIR)/obj/$(ALT_OBJ_DIR)32
  else
    OBJ_DIR = $(HOME_DIR)/obj/$(ALT_OBJ_DIR)
  endif
endif

#############################################################################

ifdef PEGASUS_TEST_VALGRIND_LOG_DIR
    BIN_DIR = $(HOME_DIR)/bin_exe
    VALGRIND_SCRIPT_BIN_DIR = $(HOME_DIR)/bin
else
    BIN_DIR = $(HOME_DIR)/bin
endif

ifeq ($(PEGASUS_INTERNAL_ENABLE_32BIT_PROVIDER_SUPPORT),true)
  LIB_DIR = $(HOME_DIR)/lib32
else
  LIB_DIR = $(HOME_DIR)/lib
endif

# l10n
# define the location for the compiled messages
MSG_ROOT = $(HOME_DIR)/msg

#############################################################################
##  The following REPOSITORY_XXX variables are only used within the
## makefiles for building the repository (cimmofl) and running the tests.
## They have no effect on CIMconfig initial startup configuration.

#
# define the location for the repository
#
# NOTE: There is another variable efined in many of the test makefiles
# called REPOSITORYDIR. It is a local variable in each of those Makefiles
# to localally control where the temporay small repository they
# build, use and then delete is located. Most of the time it is set to TMP_DIR.
#

REPOSITORY_DIR = $(HOME_DIR)

#
# WARNING: The REPOSITORY_NAME varible is not used by all the test,
# many of them are still hardcoded to "repository".  What this means
# is that you can change the repository name and build it. But you
# cannot run the test without many of them failing
#

REPOSITORY_NAME = repository


REPOSITORY_ROOT = $(REPOSITORY_DIR)/$(REPOSITORY_NAME)

# define the repository mode
#       XML = XML format
#       BIN = Binary format
#
ifndef PEGASUS_REPOSITORY_MODE
   ## set to default value
   REPOSITORY_MODE = XML
else
   ## validate assigned value
   ifeq ($(PEGASUS_REPOSITORY_MODE),XML)
       REPOSITORY_MODE = XML
   else
     ifeq ($(PEGASUS_REPOSITORY_MODE),BIN)
      REPOSITORY_MODE = BIN
     else
      $(error PEGASUS_REPOSITORY_MODE ($(PEGASUS_REPOSITORY_MODE)) \
		 is invalid. It must be set to either XML or BIN)
     endif
   endif
endif


###########################################################################

# The two variables, CIM_SCHEMA_DIR and CIM_SCHEMA_VER,
# are used to control the version of the CIM Schema
# loaded into the Pegasus Internal, InterOp,
# root/cimv2 and various test namespaces.
#
# Update the following two environment variables to
# change the version.

# The environment variable PEGASUS_CIM_SCHEMA can be used
# to change the values of CIM_SCHEMA_DIR, CIM_SCHEMA_VER
# and ALLOW_EXPERIMENTAL.
#
# To use the PEGASUS_CIM_SCHEMA variable the Schema mof
# files must be placed in the directory
# $(PEGASUS_ROOT)/Schemas/$(PEGASUS_CIM_SCHEMA)
#
# The value of PEGASUS_CIM_SCHEMA must conform to the
# following syntax:
#
#        CIM[Prelim]<CIM_SCHEMA_VER>
#
# The string "Prelim" should be included if the
# Schema contains "Experimental" class definitions.
#
# The value of <CIM_SCHEMA_VER> must be the value
# of the version string included by the DMTF as
# part of the mof file names (e.g, CIM_Core27.mof).
# Therefore, for example, the value of <CIM_SCHEMA_VER>
# for CIM27 Schema directories MUST be 27.
#
# Examples of valid values of PEGASUS_CIM_SCHEMA
# include CIMPrelim27, CIM27, CIMPrelim28, and CIM28.
#
# Note the CIMPrelim271 would NOT be a valid value
# for PEGASUS_CIM_SCHEMA because the version string
# portion of the mof files (e.g., CIM_Core27.mof) in
# the CIMPrelimin271 directory is 27 not 271.

# ***** CIM_SCHEMA_DIR INFO ****
# If CIM_SCHEMA_DIR changes to use a preliminary schema which
# has experimentals make sure and change the path below to appopriate
# directory path.  Example:  CIMPrelim271 is preliminary and has
# experimental classes.  Since experimental classes exist the -aE
# option of the mof compiler needs to be set.
# *****

## Sets default CIM Schema if PEGASUS_CIM_SCHEMA not defined.
## NOTE: If the default below is changed, please update the definition
## of default for this variable in pegasus/doc/BuildAndReleaseOptions.html
ifndef PEGASUS_CIM_SCHEMA
    PEGASUS_CIM_SCHEMA=CIM241
endif

CIM_SCHEMA_DIR=$(PEGASUS_ROOT)/Schemas/$(PEGASUS_CIM_SCHEMA)
ifeq ($(findstring $(patsubst CIM%,%,$(patsubst CIMPrelim%,%,$(PEGASUS_CIM_SCHEMA))),1 2 3 4 5 6 7 8 9 10 11 12 13 14 15 16 17 18 19 20 21 22 23 24 25 26 27 271 28),)
    CIM_SCHEMA_VER=
else
    CIM_SCHEMA_VER=$(patsubst CIM%,%,$(patsubst CIMPrelim%,%,$(PEGASUS_CIM_SCHEMA)))
endif

ifneq (, $(findstring Prelim, $(CIM_SCHEMA_DIR)))
    ALLOW_EXPERIMENTAL = -aE
else
    ALLOW_EXPERIMENTAL =
endif

LEX = flex

## ======================================================================
##
## PEGASUS_ENABLE_SORTED_DIFF
## This controls if the DIFFSORT function is used rather than a simple DIFF of
##  of the test results files to the static results file.
##
##   Set to "true" enables the sorted diffs of results to static results files.
##   otherwise results in regular diffs of results to static results files.
##   see bug 2283 for background information concerning this config variable.
##
##  Defaults to true.
##
##
ifndef PEGASUS_ENABLE_SORTED_DIFF
PEGASUS_ENABLE_SORTED_DIFF=true
endif

## ========================================================================
## DIFFSORT function definition
## Here is an example using the DIFFSORT function:
##
## difftest: FORCE
##      @ test > result
##      @ $(call DIFFSORT,result,standard_result)
##      @ $(ECHO) +++++ all test passed
##

define NL


endef

ifndef FORCE_NOCASE

DIFFSORT = $(SORT) $(1) > $(1).tmp $(NL) \
$(SORT) $(2) > $(2).tmp $(NL) \
$(DIFF) $(1).tmp $(2).tmp $(NL) \
$(RM) -f $(1).tmp $(NL) \
$(RM) -f $(2).tmp $(NL)

else

DIFFSORT = $(SORT) -f $(1) > $(1).tmp $(NL) \
$(SORT) -f $(2) > $(2).tmp $(NL) \
$(DIFF) -i $(1).tmp $(2).tmp $(NL) \
$(RM) -f $(1).tmp $(NL) \
$(RM) -f $(2).tmp $(NL)

endif

DIFFSORTNOCASE = $(SORT) $(1) > $(1).tmp $(NL) \
$(SORT) $(2) > $(2).tmp $(NL) \
$(DIFF) -i $(1).tmp $(2).tmp $(NL) \
$(RM) -f $(1).tmp $(NL) \
$(RM) -f $(2).tmp $(NL)

#
# The following is used to define the usage message for MakeFile
#
# See the pegasus/Makfile for an exampleof its usage.
#
USAGE = @$(ECHO) " $(1)"

################################################################################
##
## Attempt to include a platform configuration file:
##
################################################################################

PLATFORM_FILE = $(ROOT)/mak/platform_$(PEGASUS_PLATFORM).mak
ifneq ($(wildcard $(PLATFORM_FILE)), )
    include $(PLATFORM_FILE)
else
  $(error  PEGASUS_PLATFORM environment variable must be set to one of\
        the following:$(VALID_PLATFORMS))
endif

################################################################################
##
##  Set up any platform independent compile conditionals by adding them to
##  precreated FLAGS parameter.
##  Assumes that the basic flags have been setup in FLAGS.
##  Assumes that compile time flags are controlled with -D CLI option.
##
################################################################################

############################################################################
# OpenPegasus relies on the existence of an external set of libraries to
# support localized messages.  Today, the only supported package is
# the International Components for Unicode (ICU) OSS project,
# http://oss.software.ibm.com/icu.  If PEGASUS_HAS_ICU is true,
# OpenPegasus will use the ICU library.
#
# ICU_INSTALL points to the directory containing the ICU installation.
# If set, the OpenPegasus will use this variable to locate the ICU
# include files and libraries.  If not set, the ICU libraries are expected
# to be installed in a directory that is searched by default.
#
# If PEGASUS_HAS_ICU is not set and ICU_INSTALL is set, the value of
# PEGASUS_HAS_ICU will be set to true.
############################################################################

ifdef PEGASUS_HAS_ICU
    ifneq ($(PEGASUS_HAS_ICU),true)
        ifneq ($(PEGASUS_HAS_ICU),false)
            $(error PEGASUS_HAS_ICU ($(PEGASUS_HAS_ICU)) \
                invalid, must be true or false)
        endif
    endif
else
    ifdef ICU_INSTALL
        PEGASUS_HAS_ICU = true
    endif
endif

ifdef PEGASUS_HAS_MESSAGES
    DEFINES += -DPEGASUS_HAS_MESSAGES

    ifneq ($(PEGASUS_HAS_ICU),true)
        $(error Support for localized messages in OpenPegasus \
            requires PEGASUS_HAS_ICU to be true)
    endif
endif


ifeq ($(PEGASUS_HAS_ICU),true)
    DEFINES += -DPEGASUS_HAS_ICU

    ####################################
    ##
    ## ICU_ROOT_BUNDLE_LANG if set, specifies the language that the root
    ## resource bundle will be generated from.  Defaults to _en if not set.
    ## If set, for any directory containing resource bundles, there must
    ## exist a file name:  package($ICU_ROOT_BUNDLE_LANG).txt or the make
    ## messages target will fail.
    ##
    ####################################

    ifdef ICU_ROOT_BUNDLE_LANG
        MSG_ROOT_SOURCE = $(ICU_ROOT_BUNDLE_LANG)
    else
        MSG_ROOT_SOURCE = _en
    endif

    ifdef ICU_INSTALL
        MSG_COMPILE = $(ICU_INSTALL)/bin/genrb
    else
        MSG_COMPILE = genrb
    endif

    MSG_FLAGS =
    MSG_SOURCE_EXT = .txt
    MSG_COMPILE_EXT = .res

    ifeq ($(OS),linux)
        CNV_ROOT_CMD = $(BIN_DIR)/cnv2rootbundle
    else
        CNV_ROOT_CMD = cnv2rootbundle
    endif

    # The library path option is needed on all link commands with some ICU
    # builds, because internal ICU library dependencies are resolved
    # dynamically.
    ifdef ICU_INSTALL
        ifeq ($(OS),zos)
            # On z/OS, the -L option must appear before the -o option and
            # the object (.o) and sidedeck (.x) files in the link command.
            FLAGS += -L$(ICU_INSTALL)/lib
            PR_FLAGS += -L$(ICU_INSTALL)/lib
        else
            EXTRA_LIBRARIES += -L$(ICU_INSTALL)/lib
        endif
    endif

    # The ICU include path and library dependencies are restricted to
    # where they are specifically needed.
    ifeq ($(HAS_ICU_DEPENDENCY),true)
        ifdef ICU_INSTALL
            SYS_INCLUDES += -I$(ICU_INSTALL)/include
        endif

        ifeq ($(OS),windows)
            EXTRA_LIBRARIES += \
                $(ICU_INSTALL)/lib/icuuc.lib \
                $(ICU_INSTALL)/lib/icuin.lib \
                $(ICU_INSTALL)/lib/icudt.lib
        else
            ifeq ($(OS),zos)
                EXTRA_LIBRARIES += \
                    $(ICU_INSTALL)/lib/libicui18n$(DYNLIB_SUFFIX) \
                    $(ICU_INSTALL)/lib/libicuuc$(DYNLIB_SUFFIX)
            else
                EXTRA_LIBRARIES += -licuuc -licui18n
                ifeq ($(OS),linux)
                    EXTRA_LIBRARIES += -licudata
                endif
            endif
        endif
    endif
endif

################################################################################
##
## PEGASUS_MAX_THREADS_PER_SVC_QUEUE
##
## Controls the maximum number of threads allowed per message service queue.
##     It is allowed to range between 1 and MAX_THREADS_PER_SVC_QUEUE_LIMIT
##     as set in pegasus/src/Pegasus/Common/MessageQueueService.cpp.  If the
##     specified value is out of range, MAX_THREADS_PER_SVC_QUEUE_LIMIT is
##     used.  The default value is MAX_THREADS_PER_SVC_QUEUE_DEFAULT, as
##     defined in pegasus/src/Pegasus/Common/MessageQueueService.cpp.
##
##	Label					Current value
##	--------------------------------------  -------------
##      MAX_THREADS_PER_SVC_QUEUE_LIMIT	        5000
##      MAX_THREADS_PER_SVC_QUEUE_DEFAULT       5
##
##

ifdef PEGASUS_MAX_THREADS_PER_SVC_QUEUE
  DEFINES += -DMAX_THREADS_PER_SVC_QUEUE=$(PEGASUS_MAX_THREADS_PER_SVC_QUEUE)
endif

# Allow PEGASUS_ASSERT statements to be disabled.
ifdef PEGASUS_NOASSERTS
    DEFINES += -DNDEBUG -DPEGASUS_NOASSERTS
endif

# do not compile trace code. sometimes it causes problems debugging
ifdef PEGASUS_REMOVE_TRACE
    DEFINES += -DPEGASUS_REMOVE_TRACE
endif


ifdef PEGASUS_PLATFORM_FOR_32BIT_PROVIDER_SUPPORT
   PLATFORM_FILE_32 = $(ROOT)/mak/platform_$(PEGASUS_PLATFORM_FOR_32BIT_PROVIDER_SUPPORT).mak
   ifeq ($(wildcard $(PLATFORM_FILE_32)), )
      $(error  PEGASUS_PLATFORM_FOR_32BIT_PROVIDER_SUPPORT  environment variable must be set to one of\
        the following:$(VALID_PLATFORMS))
   endif
     DEFINES += -DPEGASUS_PLATFORM_FOR_32BIT_PROVIDER_SUPPORT
   ifdef PEGASUS_PROVIDER_MANAGER_32BIT_LIB_DIR
       DEFINES += -DPEGASUS_PROVIDER_MANAGER_32BIT_LIB_DIR=\"$(PEGASUS_PROVIDER_MANAGER_32BIT_LIB_DIR)\"
   endif
endif
  
# PEP 315
# Control whether compile with or without method entertexit trace code.
# A value other than 'true' or 'false' will cause a make error.
ifdef PEGASUS_REMOVE_METHODTRACE
  ifeq ($(PEGASUS_REMOVE_METHODTRACE),true)
    DEFINES += -DPEGASUS_REMOVE_METHODTRACE
  else
    ifneq ($(PEGASUS_REMOVE_METHODTRACE),false)
      $(error PEGASUS_REMOVE_METHODTRACE ($(PEGASUS_REMOVE_METHODTRACE)) invalid, must be true or false)
    endif
  endif
endif

# Control whether the class definitions in the repository contain elements
# propagated from superclass definitions.

ifndef PEGASUS_REPOSITORY_STORE_COMPLETE_CLASSES
    PEGASUS_REPOSITORY_STORE_COMPLETE_CLASSES = false
endif

ifeq ($(PEGASUS_REPOSITORY_STORE_COMPLETE_CLASSES),true)
    DEFINES += -DPEGASUS_REPOSITORY_STORE_COMPLETE_CLASSES
else
    ifneq ($(PEGASUS_REPOSITORY_STORE_COMPLETE_CLASSES),false)
        $(error PEGASUS_REPOSITORY_STORE_COMPLETE_CLASSES ($(PEGASUS_REPOSITORY_STORE_COMPLETE_CLASSES)) invalid, must be true or false)
    endif
endif

# SQLite repository support

ifndef PEGASUS_USE_SQLITE_REPOSITORY
    PEGASUS_USE_SQLITE_REPOSITORY = false
endif

ifeq ($(PEGASUS_USE_SQLITE_REPOSITORY),true)
    ifeq ($(PEGASUS_REPOSITORY_STORE_COMPLETE_CLASSES),true)
        $(error PEGASUS_REPOSITORY_STORE_COMPLETE_CLASSES may not be set to true when PEGASUS_USE_SQLITE_REPOSITORY is true)
    endif
    DEFINES += -DPEGASUS_USE_SQLITE_REPOSITORY
else
    ifneq ($(PEGASUS_USE_SQLITE_REPOSITORY),false)
        $(error PEGASUS_USE_SQLITE_REPOSITORY ($(PEGASUS_USE_SQLITE_REPOSITORY)) invalid, must be true or false)
    endif
endif

# PEP 161
# Control whether utf-8 filenames are supported by the repository.
# Note: These options only apply to the file-based repository, not SQLite.
ifdef PEGASUS_SUPPORT_UTF8_FILENAME
    DEFINES += -DPEGASUS_SUPPORT_UTF8_FILENAME

    # Control whether utf-8 filenames in the repository are escaped
    ifdef PEGASUS_REPOSITORY_ESCAPE_UTF8
        DEFINES += -DPEGASUS_REPOSITORY_ESCAPE_UTF8
    endif
endif

#
# PEP 142
# The following flag need to be set to enable
# user group authorization functionality.
#
ifdef PEGASUS_ENABLE_USERGROUP_AUTHORIZATION
    DEFINES += -DPEGASUS_ENABLE_USERGROUP_AUTHORIZATION
endif

############################################################################
#
# PEGASUS_ENABLE_CQL and PEGASUS_DISABLE_CQL
# Set to enable CQL processor in indication subscriptions and query execution
# PEGASUS_DISABLE_CQL (PEP 193) has been depracated. New use model is:
#
# Use PEGASUS_ENABLE_CQL=true  to enable  compilation of CQL functions.
#
# Use PEGASUS_ENABLE_CQL=false to disable compilation of CQL functions.
#
# Default is PEGASUS_ENABLE_CQL=true if not defined external to config.mak
#

ifdef PEGASUS_DISABLE_CQL
    $(error PEGASUS_DISABLE_CQL has been deprecated. Please use PEGASUS_ENABLE_CQL=[true/false])
endif

ifndef PEGASUS_ENABLE_CQL
    # Default is true. CQL is enabled normally on all platforms unless specifically defined
    PEGASUS_ENABLE_CQL=true
endif

ifeq ($(PEGASUS_ENABLE_CQL),true)
    DEFINES += -DPEGASUS_ENABLE_CQL
else
    ifneq ($(PEGASUS_ENABLE_CQL),false)
        $(error PEGASUS_ENABLE_CQL ($(PEGASUS_ENABLE_CQL)) invalid, must be true or false)
    endif
endif

############################################################################
#
# PEGASUS_ENABLE_FQL
# The use model is:
#
# Use PEGASUS_ENABLE_FQL=true  to enable  compilation of FQL functions.
#
# Use PEGASUS_ENABLE_FQL=false to disable compilation of FQL functions.
#
# Default is PEGASUS_ENABLE_FQL=true if not defined external to config.mak
#

ifndef PEGASUS_ENABLE_FQL
    # Default is true. CQL is enabled normally on all platforms unless specifically defined
    PEGASUS_ENABLE_FQL=true
endif

ifeq ($(PEGASUS_ENABLE_FQL),true)
    DEFINES += -DPEGASUS_ENABLE_FQL
else
    ifneq ($(PEGASUS_ENABLE_FQL),false)
        $(error PEGASUS_ENABLE_FQL ($(PEGASUS_ENABLE_FQL)) invalid, must be true or false)
    endif
endif

############################################################################
#
# PEGASUS_OVERRIDE_PRODUCT_ID
# PEP 186
# Allow override of product name/version/status.  A file
# pegasus/src/Pegasus/Common/ProductVersion.h must exist when this
# flag is defined.
#
ifdef PEGASUS_OVERRIDE_PRODUCT_ID
    DEFINES += -DPEGASUS_OVERRIDE_PRODUCT_ID
endif

#
# PEP 72
# Allow Out-of-Process Providers to be disabled by default
#
ifdef PEGASUS_DEFAULT_ENABLE_OOP
  ifeq ($(PEGASUS_DEFAULT_ENABLE_OOP),true)
    DEFINES += -DPEGASUS_DEFAULT_ENABLE_OOP
  else
    ifneq ($(PEGASUS_DEFAULT_ENABLE_OOP),false)
      $(error PEGASUS_DEFAULT_ENABLE_OOP ($(PEGASUS_DEFAULT_ENABLE_OOP)) invalid, must be true or false)
    endif
  endif
endif

#
# Allow to define the default value for the Provider User Context
# property as REQUESTOR.
# If is set and true use REQUESTOR
# If is not set or false use PRIVILEGED
#
ifdef PEGASUS_DEFAULT_USERCTXT_REQUESTOR
  ifeq ($(PEGASUS_DEFAULT_USERCTXT_REQUESTOR),true)
    DEFINES += -DPEGASUS_DEFAULT_USERCTXT_REQUESTOR
  else
    ifneq ($(PEGASUS_DEFAULT_USERCTXT_REQUESTOR),false)
      $(error PEGASUS_DEFAULT_USERCTXT_REQUESTOR ($(PEGASUS_DEFAULT_USERCTXT_REQUESTOR)) invalid, must be true or false)
    endif
  endif
endif

#
# PEP 197
# Allow the Provider User Context feature to be disabled.
#
ifdef PEGASUS_DISABLE_PROV_USERCTXT
    DEFINES += -DPEGASUS_DISABLE_PROV_USERCTXT
endif

# Bug 2147
# Allow local domain socket usage to be disabled.
ifdef PEGASUS_DISABLE_LOCAL_DOMAIN_SOCKET
    DEFINES += -DPEGASUS_DISABLE_LOCAL_DOMAIN_SOCKET
endif

# PEP 211
# Controls object normalization support.
ifdef PEGASUS_ENABLE_OBJECT_NORMALIZATION
    DEFINES += -DPEGASUS_ENABLE_OBJECT_NORMALIZATION
endif

# Allow ExecQuery functionality to be enabled
ifndef PEGASUS_ENABLE_EXECQUERY
    DEFINES += -DPEGASUS_DISABLE_EXECQUERY
endif

# Allow System Log Handler to be enabled
ifdef PEGASUS_ENABLE_SYSTEM_LOG_HANDLER
  DEFINES += -DPEGASUS_ENABLE_SYSTEM_LOG_HANDLER
endif

# Allow Email Handler to be enabled
ifdef PEGASUS_ENABLE_EMAIL_HANDLER
  DEFINES += -DPEGASUS_ENABLE_EMAIL_HANDLER
endif

# Allow qualifiers to be disabled for instance operations
ifdef PEGASUS_DISABLE_INSTANCE_QUALIFIERS
  DEFINES += -DPEGASUS_DISABLE_INSTANCE_QUALIFIERS
endif

# Controls snmp indication handler to use NET-SNMP to deliver trap
ifdef PEGASUS_USE_NET_SNMP   
   ifeq ($(PEGASUS_USE_NET_SNMP),true)
      DEFINES += -DPEGASUS_USE_NET_SNMP
   else
      ifneq ($(PEGASUS_USE_NET_SNMP),false)
         $(error PEGASUS_USE_NET_SNMP ($(PEGASUS_USE_NET_SNMP)) invalid, must be true or false)
      endif
   endif
endif
# Controls snmp indication handler to use NET-SNMP V3 features. 
ifndef PEGASUS_ENABLE_NET_SNMPV3
    ifeq ($(PEGASUS_USE_NET_SNMP),true)
       PEGASUS_ENABLE_NET_SNMPV3=true
    else
        PEGASUS_ENABLE_NET_SNMPV3=false
    endif
endif

ifeq ($(PEGASUS_ENABLE_NET_SNMPV3),true)
    ifneq ($(PEGASUS_USE_NET_SNMP),true)
        $(error PEGASUS_USE_NET_SNMP should be set to true when PEGASUS_ENABLE_NET_SNMPV3 is true)
    endif
    DEFINES += -DPEGASUS_ENABLE_NET_SNMPV3
else
    ifneq ($(PEGASUS_ENABLE_NET_SNMPV3),false)
        $(error PEGASUS_ENABLE_NET_SNMPV3 ($(PEGASUS_ENABLE_NET_SNMPV3)) invalid, must be true or false)
    endif
endif

ifdef PEGASUS_HAS_SSL
    DEFINES += -DPEGASUS_HAS_SSL

    # Enable SSL Random file by default.
    ifndef PEGASUS_USE_SSL_RANDOMFILE
        PEGASUS_USE_SSL_RANDOMFILE = true
    endif

    # Allow SSL Random file functionality to be optionally disabled.
    ifdef PEGASUS_USE_SSL_RANDOMFILE
        ifeq ($(PEGASUS_USE_SSL_RANDOMFILE), true)
            DEFINES += -DPEGASUS_SSL_RANDOMFILE
        else
            ifneq ($(PEGASUS_USE_SSL_RANDOMFILE), false)
                $(error PEGASUS_USE_SSL_RANDOMFILE\
                     ($(PEGASUS_USE_SSL_RANDOMFILE)) invalid, \
                      must be true or false)
            endif
        endif
    endif

    ifndef OPENSSL_COMMAND
        ifdef OPENSSL_BIN
            OPENSSL_COMMAND = $(OPENSSL_BIN)/openssl
        else
            OPENSSL_COMMAND = openssl
        endif
    endif
    ifndef OPENSSL_SET_SERIAL_SUPPORTED
        ifneq (, $(findstring 0.9.6, $(shell $(OPENSSL_COMMAND) version)))
            OPENSSL_SET_SERIAL_SUPPORTED = false
        else
            OPENSSL_SET_SERIAL_SUPPORTED = true
        endif
    endif

    # Enable CRL verification
    ifndef PEGASUS_ENABLE_SSL_CRL_VERIFICATION
        PEGASUS_ENABLE_SSL_CRL_VERIFICATION = true
    endif

    # Check for Enable SSL CRL verification
    ifdef PEGASUS_ENABLE_SSL_CRL_VERIFICATION
        ifeq ($(PEGASUS_ENABLE_SSL_CRL_VERIFICATION), true)
            DEFINES += -DPEGASUS_ENABLE_SSL_CRL_VERIFICATION
        else
            ifneq ($(PEGASUS_ENABLE_SSL_CRL_VERIFICATION), false)
                $(error PEGASUS_ENABLE_SSL_CRL_VERIFICATION\
                     ($(PEGASUS_ENABLE_SSL_CRL_VERIFICATION)) invalid, \
                      must be true or false)
            endif
        endif
    endif
endif

#
# PEP 258
# Allow Audit Logger to be disabled.  It is enabled by default.
#

ifndef PEGASUS_ENABLE_AUDIT_LOGGER
    PEGASUS_ENABLE_AUDIT_LOGGER = true
endif

ifdef PEGASUS_ENABLE_AUDIT_LOGGER
    ifeq ($(PEGASUS_ENABLE_AUDIT_LOGGER),true)
        DEFINES += -DPEGASUS_ENABLE_AUDIT_LOGGER
    else
        ifneq ($(PEGASUS_ENABLE_AUDIT_LOGGER),false)
            $(error PEGASUS_ENABLE_AUDIT_LOGGER \
              ($(PEGASUS_ENABLE_AUDIT_LOGGER)) invalid, must be true or false)
        endif
    endif
endif

# Check for use of deprecated variable
ifdef PEGASUS_DISABLE_AUDIT_LOGGER
    $(error The PEGASUS_DISABLE_AUDIT_LOGGER variable is deprecated. \
        Use PEGASUS_ENABLE_AUDIT_LOGGER=false instead)
endif


#
# PEP 291
# Enable IPv6 support
#

ifndef PEGASUS_ENABLE_IPV6
    PEGASUS_ENABLE_IPV6 = true
endif

# Check for Enable IPv6 support
ifdef PEGASUS_ENABLE_IPV6
  ifeq ($(PEGASUS_ENABLE_IPV6),true)
    DEFINES += -DPEGASUS_ENABLE_IPV6
  else
    ifneq ($(PEGASUS_ENABLE_IPV6),false)
      $(error PEGASUS_ENABLE_IPV6 ($(PEGASUS_ENABLE_IPV6)) \
       invalid, must be true or false)
    endif
  endif
endif

# Verify Test IPv6 support
# If PEGASUS_ENABLE_IPV6 is defined and PEGASUS_TEST_IPV6 is not defined, we set
# PEGASUS_TEST_IPV6 to the same value as PEGASUS_ENABLE_IPV6.
# You can explicitly set PEGASUS_TEST_IPV6 to false if you don't want to run the
# IPv6 tests (for example, on an IPv4 system that is running an IPv6-enabled
# version of Pegasus).
#
ifdef PEGASUS_TEST_IPV6
  ifneq ($(PEGASUS_TEST_IPV6),true)
    ifneq ($(PEGASUS_TEST_IPV6),false)
      $(error PEGASUS_TEST_IPV6 ($(PEGASUS_TEST_IPV6)) \
       invalid, must be true or false)
    endif
  endif
else
  PEGASUS_TEST_IPV6 = $(PEGASUS_ENABLE_IPV6)
endif

#
# PEP 322
# Allow tracking generated indications data to be disabled.  It is enabled
# by default.
#

ifndef PEGASUS_ENABLE_INDICATION_COUNT
    PEGASUS_ENABLE_INDICATION_COUNT = true
endif

ifdef PEGASUS_ENABLE_INDICATION_COUNT
    ifeq ($(PEGASUS_ENABLE_INDICATION_COUNT),true)
        DEFINES += -DPEGASUS_ENABLE_INDICATION_COUNT
    else
        ifneq ($(PEGASUS_ENABLE_INDICATION_COUNT),false)
            $(error PEGASUS_ENABLE_INDICATION_COUNT \
              ($(PEGASUS_ENABLE_INDICATION_COUNT)) invalid, must be true or false)
        endif
    endif
endif

############################################################################
#
# PEGASUS_ENABLE_SLP and PEGASUS_DISABLE_SLP
#
# PEGASUS_DISABLE_SLP has been deprecated. New use model is:
#
# Use PEGASUS_ENABLE_SLP=true  to enable  compilation of SLP functions.
#
# Use PEGASUS_ENABLE_SLP=false to disable compilation of SLP functions.
#
# NOTE. Effective with Bug # 2633 some platforms enable SLP.
# To see which platforms look for platform make files that set
# the variable PEGASUS_ENABLE_SLP.
#
#

ifdef PEGASUS_ENABLE_SLP
  ifdef PEGASUS_DISABLE_SLP
    $(error Conflicting defines PEGASUS_ENABLE_SLP and PEGASUS_DISABLE_SLP both set)
  endif
endif

ifdef PEGASUS_DISABLE_SLP
    $(error PEGASUS_DISABLE_SLP has been deprecated. Please use PEGASUS_ENABLE_SLP=[true/false] )

PEGASUS_ENABLE_SLP=false

endif

ifdef PEGASUS_ENABLE_SLP
  ifeq ($(PEGASUS_ENABLE_SLP),true)
    DEFINES += -DPEGASUS_ENABLE_SLP
  else
    ifneq ($(PEGASUS_ENABLE_SLP),false)
      $(error PEGASUS_ENABLE_SLP ($(PEGASUS_ENABLE_SLP)) invalid, must be true or false)
    endif
  endif
endif


############################################################################
#
# PEGASUS_USE_OPENSLP

## NOTE: This variable has been deprecated and superceeded by the use of
## PEGASUS_USE_EXTERNAL_SLP. The use of this variable may be
## removed from Pegasus in future releases. It is converted to the
## PEGASUS_USE_EXTERNAL_SLP variable in the following function.
## Do not allow PEGASUS_USE_EXTERNAL_SLP if PEGASUS_ENABLE_SLP not set and
## insure that PEGASUS_USE_OPENSLP and PEGASUS_USE_EXTERNAL_SLP are not used
## simultaneously
#
# Environment variable to set openslp as SLP environment to use
# for SLP Directory and User Agents.
#
# Allows enabling use of openslp interfaces for slp instead of the
# internal pegasus slp agent.  Note that this does not disable the
# compilation of the internal agent code, etc.  However, it assumes
# openslp is installed on the platform and changes the interfaces
# to match this.
#
# Use this variable in conjunction with PEGASUS_OPENSLP_HOME
# to enable OpenSlp as the slp implementation.
#
# NOTE that it has no affect if the PEGASUS_ENABLE_SLP etc. flags are not set.
#

ifdef PEGASUS_USE_OPENSLP
  ifdef PEGASUS_USE_EXTERNAL_SLP
      $(error Both PEGASUS_USE_OPENSLP and PEGASUS_USE_EXTERNAL_SLP defined. \
          Please use PEGASUS_USE_EXTERNAL_SLP)
  endif
  ## if PEGASUS_USE_OPENSLP used, convert to EXTERNAL_SLP variables
  ## as defined for openslp
  ifeq ($(PEGASUS_USE_OPENSLP),true)
    ifeq ($(PEGASUS_ENABLE_SLP),true)
      export PEGASUS_USE_EXTERNAL_SLP=openslp
      PEGASUS_USE_OPENSLP=
    else
      $(error PEGASUS_USE_OPENSLP defined but PEGASUS_ENABLE_SLP is not true. \
        Please correct this inconsistency)
    endif
  else
    ifneq ($(PEGASUS_USE_OPENSLP), false)
      $(error PEGASUS_USE_OPENSLP \
            ($(PEGASUS_USE_OPENSLP)) \
            invalid, must be true or false)
    endif
  endif
endif

#########################################################################
# PEP 267
# SLP reregistration support.
# PEGASUS_SLP_REG_TIMEOUT is defined as the SLP registration timeout
# interval, in minutes.
ifdef PEGASUS_SLP_REG_TIMEOUT
    ifeq ($(PEGASUS_ENABLE_SLP),true)
        DEFINES += -DPEGASUS_SLP_REG_TIMEOUT=$(PEGASUS_SLP_REG_TIMEOUT)
     else
        $(error PEGASUS_SLP_REG_TIMEOUT defined but PEGASUS_ENABLE_SLP is not true. \
            Please correct this inconsistency)
     endif
 endif

############################################################################
#
# PEGASUS_USE_EXTERNAL_SLP

# Environment variable to set an external slp implementation as the SLP
# environment to use for SLP Directory Agents.

# This allows setting any one of several possible external SLP SAs as the
# interface for the SLP provider to communicate with in managing SLP templates
# in place of the internal SLP agent provided with Pegasus.
# Note: This does not disable the compilation of the internal agent code since
# this is used for the SLP UA defined with the pegasus client.
# Pegasus assumes that the external SLP defined is installed on the platform
# and running when pegasus is started. It changes the interfaces from the SLP
# provider to match the defined SLP implementation.

# This environment variable superceedes the use of PEGASUS_USE_OPENSLP since
# openslp is considered one of the valid external SLP environments usable by
# pegasus.

# The variable uses the value component to define a name for the external SLP
# environment that must match one of the names defined below.

# This variable is not allowed if the PEGASUS_ENABLE_SLP flag is not set.

# Allow only predefined string values for the variable corresponding
# to external slp types that pegasus knows.
# Valid types are openslp (1) and solarisslp (2)
EXTERNAL_SLP_TYPES = openslp solarisslp

ifdef PEGASUS_USE_EXTERNAL_SLP
  ifeq ($(PEGASUS_ENABLE_SLP),true)
    ifeq ($(PEGASUS_USE_EXTERNAL_SLP),openslp)
      DEFINES += -DPEGASUS_USE_EXTERNAL_SLP_TYPE=1
    else
      ifeq ($(PEGASUS_USE_EXTERNAL_SLP),solarisslp)
         DEFINES += -DPEGASUS_USE_EXTERNAL_SLP_TYPE=2
      else
        $(error PEGASUS_USE_EXTERNAL_SLP value ($(PEGASUS_USE_EXTERNAL_SLP)) \
          invalid. It must be one of valid SLP external types \
          ($(EXTERNAL_SLP_TYPES)) )
      endif
    endif
  endif
endif

############################################################################
#
# PEGASUS_OPENSLP_HOME
#
# PEGASUS_OPENSLP_HOME superceeded by PEGASUS_OPEN_EXTERNAL_SLP_HOME. If
# PEGASUS_OPENSLP_HOME is encountered it will create PEGASUS_OPEN_EXTERNAL_SLP_HOME
#
# Environment variable to set home location for OpenSLP include and library
# files if they are located somewhere other than /usr/include and /usr/lib.
#
# PEGASUS_USE_OPENSLP must also be defined for this environment variable
# to have any effect.
#
# This is the directory level within which both the include and lib
# directories holding the OpenSLP files will be found.
#
# EG: If the are located in /opt/OpenSLP/include and /opt/OpenSLP/lib
#     then this environment variable should be set to /opt/OpenSLP.
#

ifdef PEGASUS_OPENSLP_HOME
  ifdef PEGASUS_OPEN_EXTERNAL_SLP_HOME
    $(error Both PEGASUS_OPENSLP_HOME and PEGASUS_OPEN_EXTERNAL_SLP_HOME defined. \
      Please use PEGASUS_OPEN_EXTERNAL_SLP_HOME)
  else
      export PEGASUS_EXTERNAL_SLP_HOME=$(PEGASUS_OPENSLP_HOME)
   endif
endif
############################################################################
#
# PEGASUS_EXTERNAL_SLP_HOME
#
# Environment variable to set home location for External SLP include and library
# files if they are located somewhere other than /usr/include and /usr/lib.
#
# This variable superceeds PEGASUS_OPENSLP_HOME to match the use of
# PEGASUS_USE_EXTERNAL_SLP variable.
#
# PEGASUS_USE_EXTERNAL_SLP must also be defined for this environment variable
# to have any effect.
#
# This is the directory level within which both the include and lib
# directories holding the OpenSLP files will be found.
#
# EG: If the are located in /opt/OpenSLP/include and /opt/OpenSLP/lib
#     then this environment variable should be set to /opt/OpenSLP.
#

############################################################################
#
# Enable this flag to allow the handshake to continue regardless of verification result
#
ifdef PEGASUS_OVERRIDE_SSL_CERT_VERIFICATION_RESULT
  DEFINES += -DPEGASUS_OVERRIDE_SSL_CERT_VERIFICATION_RESULT
endif

############################################################################
#
# PEGASUS_ENABLE_INTEROP_PROVIDER
# Enables the interop provider AND the server profile.
# initially this was activated by setting either the perfinst or slp enable
# flags.  This allows activating this function without any either perfinst or
# slp enabled.  Note that if either of these are enabled, this funtion is also
# enabled

## if either slp or perfinst are enabled and this is false, flag error
## This gets messy because should account for both postive and negative on
## interop so we don't get multiples.

ifdef PEGASUS_ENABLE_SLP
    ifeq ($(PEGASUS_ENABLE_SLP),true)
        ifndef PEGASUS_ENABLE_INTEROP_PROVIDER
            PEGASUS_ENABLE_INTEROP_PROVIDER = true
        else
            ifeq ($(PEGASUS_ENABLE_INTEROP_PROVIDER),false)
                $(error PEGASUS_ENABLE_INTEROP_PROVIDER ($(PEGASUS_ENABLE_INTEROP_PROVIDER)) invalid, must be true if SLP enabled)
            endif
        endif
    endif
endif

## if PERFINST enabled, set to force interop.
ifndef PEGASUS_DISABLE_PERFINST
    ifndef PEGASUS_ENABLE_INTEROP_PROVIDER
        PEGASUS_ENABLE_INTEROP_PROVIDER = true
    else
        ifeq ($(PEGASUS_ENABLE_INTEROP_PROVIDER),false)
            $(error PEGASUS_ENABLE_INTEROP_PROVIDER ($(PEGASUS_ENABLE_INTEROP_PROVIDER)) invalid, must be true if PERFINST enabled)
        endif
    endif
endif

#
## PEP 323, DMTF Indications Profile support, stage 1
#
ifdef PEGASUS_ENABLE_DMTF_INDICATION_PROFILE_SUPPORT
    ifeq ($(PEGASUS_ENABLE_DMTF_INDICATION_PROFILE_SUPPORT), true)
        ifndef PEGASUS_ENABLE_INTEROP_PROVIDER
            PEGASUS_ENABLE_INTEROP_PROVIDER = true
        else
            ifeq ($(PEGASUS_ENABLE_INTEROP_PROVIDER),false)
                $(error PEGASUS_ENABLE_INTEROP_PROVIDER ($(PEGASUS_ENABLE_INTEROP_PROVIDER)) invalid, must be true if DMTF Indications profile support is enabled)
            endif
        endif
        DEFINES += -DPEGASUS_ENABLE_DMTF_INDICATION_PROFILE_SUPPORT -DPEGASUS_ENABLE_INDICATION_ORDERING
    else
        ifneq ($(PEGASUS_ENABLE_DMTF_INDICATION_PROFILE_SUPPORT),false)
            $(error PEGASUS_ENABLE_DMTF_INDICATION_PROFILE_SUPPORT ($(PEGASUS_ENABLE_DMTF_INDICATION_PROFILE_SUPPORT)) invalid, must be true or false)
        endif
    endif
endif

ifdef PEGASUS_ENABLE_INTEROP_PROVIDER
    ifeq ($(PEGASUS_ENABLE_INTEROP_PROVIDER),true)
        DEFINES += -DPEGASUS_ENABLE_INTEROP_PROVIDER
    else
        ifneq ($(PEGASUS_ENABLE_INTEROP_PROVIDER),false)
            $(error PEGASUS_ENABLE_INTEROP_PROVIDER ($(PEGASUS_ENABLE_INTEROP_PROVIDER)) invalid, must be true or false)
        endif
    endif
endif


############################################################################
# set PEGASUS_DEBUG into the DEFINES if it exists.
# Note that this flag is the general separator between
# debug compiles and non-debug compiles and controls both
# the use of any debug options on compilers and linkers
# and general debug support that we want to be turned on in
# debug mode.
ifdef PEGASUS_DEBUG
    DEFINES += -DPEGASUS_DEBUG

    # Indications debugging options
    ifdef PEGASUS_INDICATION_PERFINST
        DEFINES += -DPEGASUS_INDICATION_PERFINST
    endif

    ifdef PEGASUS_INDICATION_HASHTRACE
        DEFINES += -DPEGASUS_INDICATION_HASHTRACE
    endif

endif

# compile in the experimental APIs
DEFINES += -DPEGASUS_USE_EXPERIMENTAL_INTERFACES

# Ensure that the deprecated interfaces are defined in the Pegasus libraries.
# One may wish to disable these interfaces if binary compatibility with
# previous Pegasus releases is not required.
ifndef PEGASUS_DISABLE_DEPRECATED_INTERFACES
    DEFINES += -DPEGASUS_USE_DEPRECATED_INTERFACES
endif

# Set compile flag to control compilation of CIMOM statistics
ifdef PEGASUS_DISABLE_PERFINST
    DEFINES += -DPEGASUS_DISABLE_PERFINST
endif

# Set compile flag to control compilation of SNIA Extensions
ifdef PEGASUS_SNIA_EXTENSIONS
    DEFINES += -DPEGASUS_SNIA_EXTENSIONS
endif

ifdef PEGASUS_ENABLE_CMPI_PROVIDER_MANAGER
    ifeq ($(PEGASUS_ENABLE_CMPI_PROVIDER_MANAGER), true)
        DEFINES += -DPEGASUS_ENABLE_CMPI_PROVIDER_MANAGER
    else
        ifneq ($(PEGASUS_ENABLE_CMPI_PROVIDER_MANAGER), false)
            $(error PEGASUS_ENABLE_CMPI_PROVIDER_MANAGER \
                 ($(PEGASUS_ENABLE_CMPI_PROVIDER_MANAGER)) invalid, \
                  must be true or false)
        endif
    endif
endif

ifdef PEGASUS_ENABLE_JMPI_PROVIDER_MANAGER
    ifeq ($(PEGASUS_ENABLE_JMPI_PROVIDER_MANAGER), true)
        DEFINES += -DPEGASUS_ENABLE_JMPI_PROVIDER_MANAGER
        ifndef PEGASUS_JAVA_CLASSPATH_DELIMITER
            PEGASUS_JAVA_CLASSPATH_DELIMITER = :
        endif

        ifndef PEGASUS_JVM
            PEGASUS_JVM = sun
        endif
        ifeq ($(PEGASUS_JVM),gcj)
            PEGASUS_JAVA_COMPILER           = gcj -C
            PEGASUS_JAVA_JAR                = fastjar
            PEGASUS_JAVA_INTERPRETER        = gij
        else
            PEGASUS_JAVA_COMPILER           = javac -target 1.4 -source 1.4
            PEGASUS_JAVA_JAR                = jar
            PEGASUS_JAVA_INTERPRETER        = java
        endif

    else
        ifneq ($(PEGASUS_ENABLE_JMPI_PROVIDER_MANAGER), false)
            $(error PEGASUS_ENABLE_JMPI_PROVIDER_MANAGER \
                 ($(PEGASUS_ENABLE_JMPI_PROVIDER_MANAGER)) invalid, \
                  must be true or false)
        endif
    endif
endif

# Allow remote CMPI functionality to be enabled
ifdef PEGASUS_ENABLE_REMOTE_CMPI
    DEFINES += -DPEGASUS_ENABLE_REMOTE_CMPI
endif


############################################################
#
# Set up other Make Variables that depend on platform config files
#
############################################################

# This is temporary until we end up with a better place to
# put this variable
# Makefiles can do directory remove with
# $(RMREPOSITORY) repositoryname
#
RMREPOSITORY = $(RMDIRHIER)

ifndef PEGASUS_USE_RELEASE_CONFIG_OPTIONS
   PEGASUS_USE_RELEASE_CONFIG_OPTIONS=false
endif

ifeq ($(PEGASUS_USE_RELEASE_CONFIG_OPTIONS),true)
   DEFINES += -DPEGASUS_USE_RELEASE_CONFIG_OPTIONS
else
   ifneq ($(PEGASUS_USE_RELEASE_CONFIG_OPTIONS),false)
      $(error PEGASUS_USE_RELEASE_CONFIG_OPTIONS \
          ($(PEGASUS_USE_RELEASE_CONFIG_OPTIONS)) invalid, must be true or false)
   endif
endif

ifdef PEGASUS_USE_RELEASE_DIRS
    DEFINES += -DPEGASUS_USE_RELEASE_DIRS
endif

ifdef PEGASUS_OVERRIDE_DEFAULT_RELEASE_DIRS
    DEFINES += -DPEGASUS_OVERRIDE_DEFAULT_RELEASE_DIRS
endif

# Unless otherwise specified, Pegasus libraries go in $(PEGASUS_HOME)/lib
ifndef PEGASUS_DEST_LIB_DIR
    PEGASUS_DEST_LIB_DIR = lib
endif

ifeq ($(OS),VMS)
 DEFINES += -DPEGASUS_DEST_LIB_DIR="""$(PEGASUS_DEST_LIB_DIR)"""
else
 DEFINES += -DPEGASUS_DEST_LIB_DIR=\"$(PEGASUS_DEST_LIB_DIR)\"
endif

################################################################################
##
## PEGASUS_CLASS_CACHE_SIZE
##
##     This environment variable gives the size of the class cache used by
##     the CIM repository. When it is undefined, the size defaults to something
##     relatively small (see src/Pegasus/Repository/CIMRepository.cpp). If
##     defined, it gives the size of the class cache. If it is 0 , the class
##     cache is not defined compiled in at all.
##
################################################################################

ifdef PEGASUS_CLASS_CACHE_SIZE
DEFINES += -DPEGASUS_CLASS_CACHE_SIZE=$(PEGASUS_CLASS_CACHE_SIZE)
endif

################################################################################
##
## Additional build flags passed in through environment variables.
## These flags are added to the compile/link commands.
##
################################################################################

ifdef PEGASUS_EXTRA_CXX_FLAGS
    EXTRA_CXX_FLAGS = $(PEGASUS_EXTRA_CXX_FLAGS)
endif

ifdef PEGASUS_EXTRA_C_FLAGS
    EXTRA_C_FLAGS = $(PEGASUS_EXTRA_C_FLAGS)
endif

ifdef PEGASUS_EXTRA_LINK_FLAGS
    EXTRA_LINK_FLAGS = $(PEGASUS_EXTRA_LINK_FLAGS)
endif

##==============================================================================
##
## By definining PEGASUS_USE_STATIC_LIBRARIES in the environment and STATIC
## in the Makefile, a static library is produced rather than a shared one.
## PEGASUS_USE_STATIC_LIBRARIES should be "true" or "false".
##
##==============================================================================

ifdef PEGASUS_USE_STATIC_LIBRARIES
    ifeq ($(OS_TYPE),windows)
        $(error PEGASUS_USE_STATIC_LIBRARIES is not support on windows.)
    else
        ifneq ($(PEGASUS_USE_STATIC_LIBRARIES),true)
            $(error PEGASUS_USE_STATIC_LIBRARIES \
                ($(PEGASUS_USE_STATIC_LIBRARIES)) invalid, must be true.)
        endif
    endif
endif

##==============================================================================
##
## PEGASUS_ENABLE_PRIVILEGE_SEPARATION
##
##     Enables privilege separation support (uses the executor process to
##     perform privileged operations).
##
##==============================================================================

ifdef PEGASUS_ENABLE_PRIVILEGE_SEPARATION
  ifeq ($(PEGASUS_ENABLE_PRIVILEGE_SEPARATION),true)
    DEFINES += -DPEGASUS_ENABLE_PRIVILEGE_SEPARATION
  else
    ifneq ($(PEGASUS_ENABLE_PRIVILEGE_SEPARATION),false)
      $(error PEGASUS_ENABLE_PRIVILEGE_SEPARATION \
        ($(PEGASUS_ENABLE_PRIVILEGE_SEPARATION)) invalid, must be true or false)
    endif
  endif

  ## Defines the user context of the cimservermain process when privilege
  ## separation is enabled.
  PEGASUS_CIMSERVERMAIN_USER = cimsrvr
  DEFINES += -DPEGASUS_CIMSERVERMAIN_USER=\"$(PEGASUS_CIMSERVERMAIN_USER)\"
endif


##==============================================================================
##
## PEGASUS_ENABLE_PROTOCOL_WSMAN
##
##     Enables the WS-Management protocol in the CIM Server.
##
##==============================================================================

ifndef PEGASUS_ENABLE_PROTOCOL_WSMAN
    PEGASUS_ENABLE_PROTOCOL_WSMAN = false
endif

ifeq ($(PEGASUS_ENABLE_PROTOCOL_WSMAN),true)
    DEFINES += -DPEGASUS_ENABLE_PROTOCOL_WSMAN
else
    ifneq ($(PEGASUS_ENABLE_PROTOCOL_WSMAN),false)
        $(error PEGASUS_ENABLE_PROTOCOL_WSMAN ($(PEGASUS_ENABLE_PROTOCOL_WSMAN)) invalid, must be true or false)
    endif
endif

##==============================================================================
##
## PEGASUS_PAM_SESSION_SECURITY
##
## This is a new method to handle authentication with PAM in case it is required
## to keep the PAM session established by pam_start() open across an
## entire CIM request.
##
## This feature contradicts PEGASUS_PAM_AUTHENTICATION and
## PEGASUS_USE_PAM_STANDALONE_PROC
## Because of the additional process this feature is not compatible with
## Privilege Separation.
##
##==============================================================================

ifeq ($(PEGASUS_PAM_SESSION_SECURITY),true)
    ifdef PEGASUS_PAM_AUTHENTICATION
        $(error "PEGASUS_PAM_AUTHENTICATION must NOT be defined when PEGASUS_PAM_SESSION_SECURITY is defined")
    endif
    ifdef PEGASUS_USE_PAM_STANDALONE_PROC
        $(error "PEGASUS_USE_PAM_STANDALONE_PROC must NOT be defined when PEGASUS_PAM_SESSION_SECURITY is defined")
    endif
    ifdef PEGASUS_ENABLE_PRIVILEGE_SEPARATION
        $(error "PEGASUS_ENABLE_PRIVILEGE_SEPARATION must NOT be defined when PEGASUS_PAM_SESSION_SECURITY is defined")
    endif
    # Compile in the code required for PAM 
    # and compile out the code that uses the password file.
    DEFINES += -DPEGASUS_PAM_SESSION_SECURITY -DPEGASUS_NO_PASSWORDFILE
    # Link with libpam only where it is needed.
    ifeq ($(HAS_PAM_DEPENDENCY),true)
        SYS_LIBS += -lpam
    endif
endif


##==============================================================================
##
## PEGASUS_PAM_AUTHENTICATION
##
##==============================================================================

ifdef PEGASUS_PAM_AUTHENTICATION
    # Compile in the code required for PAM authentication
    # and compile out the code that uses the password file.
    DEFINES += -DPEGASUS_PAM_AUTHENTICATION -DPEGASUS_NO_PASSWORDFILE

    # Link with libpam only where it is needed.
    ifeq ($(HAS_PAM_DEPENDENCY),true)
        SYS_LIBS += -lpam
    endif
endif

##==============================================================================
##
## PEGASUS_NEGOTIATE_AUTHENTICATION
##
##==============================================================================

ifndef PEGASUS_NEGOTIATE_AUTHENTICATION
  PEGASUS_NEGOTIATE_AUTHENTICATION=false
endif

ifeq ($(PEGASUS_NEGOTIATE_AUTHENTICATION),true)
    DEFINES += -DPEGASUS_NEGOTIATE_AUTHENTICATION
    # Link with MIT Kerberos
    SYS_LIBS += -lgssapi_krb5
else
  ifneq ($(PEGASUS_NEGOTIATE_AUTHENTICATION),false)
    $(error "PEGASUS_NEGOTIATE_AUTHENTICATION must be true or false")
  endif
endif

##==============================================================================
##
## PEGASUS_USE_PAM_STANDALONE_PROC
##
##==============================================================================

ifdef PEGASUS_USE_PAM_STANDALONE_PROC
   ifndef PEGASUS_PAM_AUTHENTICATION
       $(error "PEGASUS_PAM_AUTHENTICATION must be defined when PEGASUS_USE_PAM_STANDALONE_PROC is defined")
   endif
   DEFINES += -DPEGASUS_USE_PAM_STANDALONE_PROC
endif

##==============================================================================

# Disable client timeouts when we're doing a valgrind build
ifdef PEGASUS_TEST_VALGRIND_LOG_DIR
    DEFINES += -DPEGASUS_DISABLE_CLIENT_TIMEOUT -DPEGASUS_TEST_VALGRIND
endif

## ======================================================================
##
## PEGASUS_ALLOW_ABSOLUTEPATH_IN_PROVIDERMODULE
## This controls allowing the path specified in the Location property of
## PG_ProviderModule class.
##
##   Set to "true", It allows the absolute path specified in the Location property
##   of PG_ProviderModule class. Otherwise it does not allow the absolute path.
##   see bug 7289 for background information concerning this config variable.
##

ifndef PEGASUS_ALLOW_ABSOLUTEPATH_IN_PROVIDERMODULE
    PEGASUS_ALLOW_ABSOLUTEPATH_IN_PROVIDERMODULE=false
endif

ifdef PEGASUS_ALLOW_ABSOLUTEPATH_IN_PROVIDERMODULE
  ifeq ($(PEGASUS_ALLOW_ABSOLUTEPATH_IN_PROVIDERMODULE),true)
    DEFINES += -DPEGASUS_ALLOW_ABSOLUTEPATH_IN_PROVIDERMODULE
  else
    ifneq ($(PEGASUS_ALLOW_ABSOLUTEPATH_IN_PROVIDERMODULE),false)
      $(error PEGASUS_ALLOW_ABSOLUTEPATH_IN_PROVIDERMODULE \
            ($(PEGASUS_ALLOW_ABSOLUTEPATH_IN_PROVIDERMODULE)) \
            invalid, must be true or false)
    endif
  endif
endif

##==============================================================================
##
## PEGASUS_ENABLE_PROTOCOL_BINARY
##
##     Enables the binary protocol between clients and cimserver. With provider
##     agent, both requests and responses are binary. For "ordinary" clients,
##     requests are XML and responses are binary. By default, this only affects
##     the protocol used over local domain sockets.
##
##==============================================================================

ifndef PEGASUS_ENABLE_PROTOCOL_BINARY
  PEGASUS_ENABLE_PROTOCOL_BINARY=false
endif

ifeq ($(PEGASUS_ENABLE_PROTOCOL_BINARY),true)
  DEFINES += -DPEGASUS_ENABLE_PROTOCOL_BINARY
else
  ifneq ($(PEGASUS_ENABLE_PROTOCOL_BINARY),false)
    $(error "PEGASUS_ENABLE_PROTOCOL_BINARY must be true or false")
  endif
endif


##==============================================================================
##
## PEGASUS_ENABLE_PROTOCOL_WEB
##
##     Enables the GET-Method for files in order to act as a web-server
##
##
##
##
##==============================================================================
ifndef PEGASUS_ENABLE_PROTOCOL_WEB
  PEGASUS_ENABLE_PROTOCOL_WEB = true
endif

ifeq ($(PEGASUS_ENABLE_PROTOCOL_WEB),true)
  DEFINES += -DPEGASUS_ENABLE_PROTOCOL_WEB
else
  ifneq ($(PEGASUS_ENABLE_PROTOCOL_WEB),false)
    $(error "PEGASUS_ENABLE_PROTOCOL_WEB must be true or false")
  endif
endif

## ======================================================================
##
## PLATFORM_CORE_PATTERN
## This variable describes the pattern used to search for core files.
## If not defined, this variable will be set to core*.
## Only wildcard characters supported by the make wildcard function
## may be used.

ifndef PLATFORM_CORE_PATTERN
    PLATFORM_CORE_PATTERN = core*
endif

ifdef PEGASUS_FLAVOR
  ifdef PEGASUS_USE_RELEASE_DIRS
    ifndef PEGASUS_OVERRIDE_DEFAULT_RELEASE_DIRS
      $(error "PEGASUS_OVERRIDE_DEFAULT_RELEASE_DIRS must be defined when both PEGASUS_FLAVOR and PEGASUS_USE_RELEASE_DIRS options are used")
    endif
  endif
  ifneq ($(PEGASUS_FLAVOR), tog)
      DEFINES += -DPEGASUS_FLAVOR=\"$(PEGASUS_FLAVOR)\"
  endif
endif

ifdef PEGASUS_EXTRA_PROVIDER_LIB_DIR
   ifndef PEGASUS_OVERRIDE_DEFAULT_RELEASE_DIRS
      $(error "PEGASUS_OVERRIDE_DEFAULT_RELEASE_DIRS must be defined when PEGASUS_EXTRA_PROVIDER_LIB_DIR defined.")
   endif
   DEFINES += -DPEGASUS_EXTRA_PROVIDER_LIB_DIR=\"$(PEGASUS_EXTRA_PROVIDER_LIB_DIR):\"
else
   DEFINES += -DPEGASUS_EXTRA_PROVIDER_LIB_DIR=\"\"
endif

################################################################################
##
## PEGASUS_INITIAL_THREADSTACK_SIZE
##
##     This environment variable sets the initial size of the stack on new threads.
##     When it is undefined, the size defaults to something relatively small
##     (see src/Pegasus/Common/Config.h or if overridden by platform see
##      src/Pegasus/Common/Platform_$(PEGASUS_PLATFORM).h).
##     Value is specified in number of bytes.
##
################################################################################

ifdef PEGASUS_INITIAL_THREADSTACK_SIZE
DEFINES += -DPEGASUS_INITIAL_THREADSTACK_SIZE=$(PEGASUS_INITIAL_THREADSTACK_SIZE)
endif

ifndef PEGASUS_INTEROP_NAMESPACE
    PEGASUS_INTEROP_NAMESPACE=root/PG_InterOp
else
ifeq ($(PEGASUS_INTEROP_NAMESPACE),root/interop)
DEFINES += -DNS_ROOT_INTEROP
    endif
ifeq ($(PEGASUS_INTEROP_NAMESPACE),interop)
DEFINES += -DNS_INTEROP
endif
endif

##These namespaces will be used in Makefiles.

NAMESPACE_INTEROP = interop

NAMESPACE_ROOT_INTEROP = root/interop


##==============================================================================
##
## PEGASUS_ENABLE_SESSION_COOKIES
##
##==============================================================================

# Cookies are enabled by defaut when HAS_SSL is defined _or_ on zOS
ifndef PEGASUS_ENABLE_SESSION_COOKIES
  ifdef PEGASUS_HAS_SSL
    PEGASUS_ENABLE_SESSION_COOKIES=true
  else
    ifeq ($(OS),zos)
      PEGASUS_ENABLE_SESSION_COOKIES=true
    else
      PEGASUS_ENABLE_SESSION_COOKIES=false
    endif
  endif
endif

ifeq ($(PEGASUS_ENABLE_SESSION_COOKIES),true)
  ifndef PEGASUS_HAS_SSL
    ifneq ($(OS),zos)
      $(error "PEGASUS_ENABLE_SESSION_COOKIES can be set to 'true' only when PEGASUS_HAS_SSL is 'true' or on zOS platform")
    endif
  endif
  DEFINES += -DPEGASUS_ENABLE_SESSION_COOKIES
else
  ifneq ($(PEGASUS_ENABLE_SESSION_COOKIES),false)
    $(error "PEGASUS_ENABLE_SESSION_COOKIES must be true or false")
  endif
endif
