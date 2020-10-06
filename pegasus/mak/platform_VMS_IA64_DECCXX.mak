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
include $(ROOT)/mak/config-vms.mak

OS = VMS

ARCHITECTURE = ia64

COMPILER = deccxx

DEFINES = -DPEGASUS_OS_VMS -DPEGASUS_PLATFORM_$(PEGASUS_PLATFORM)

ifeq ($(PEGASUS_USE_STATIC_LIBRARIES),false)
    PEGASUS_VMS_GENERATE_EXPORT_SYMBOLS = true
endif

ifeq ($(PEGASUS_VMS_GENERATE_EXPORT_SYMBOLS),true)
    DEFINES += -DPEGASUS_VMS_GENERATE_EXPORT_SYMBOLS
endif

DEPEND_INCLUDES =

TMP_OBJDIR = $(subst ../,,$(DIR))
OBJ_DIR = $(HOME_DIR)/obj/$(TMP_OBJDIR)
BIN_DIR = $(HOME_DIR)/bin
LIB_DIR = $(HOME_DIR)/lib
OPT_DIR = $(HOME_DIR)/opt
LFLAGS =  /Threads_Enable=(Upcalls,Multiple_Kernel_Threads)
LFLAGS +=  /symbol_table=$(BIN_VMSDIRA)]$(PROGRAM)
LFLAGS +=  /map=$(BIN_VMSDIRA)]$(PROGRAM)/full/cross_reference
CFLAGS =  /main=POSIX_EXIT
CFLAGS += /names=(uppercase,shortened)
CFLAGS += /repos=$(CXXREPOSITORY_VMSROOT)

CCFLAGS = /main=POSIX_EXIT
CCFLAGS += /names=(uppercase,shortened)

ifdef PEGASUS_LIST_MACHINE_CODE
CFLAGS += /show=include/list=$(OBJ_VMSDIRA)]/debug/noopt
CCFLAGS += /show=include/list=$(OBJ_VMSDIRA)]/debug/noopt
CFLAGS += /machine_code
CCFLAGS += /machine_code
endif

ifdef PEGASUS_USE_DEBUG_BUILD_OPTIONS
CFLAGS += /show=include/list=$(OBJ_VMSDIRA)]/debug/noopt
CCFLAGS += /show=include/list=$(OBJ_VMSDIRA)]/debug/noopt
# generate run-time .dsf files (Debug Symbol Table files)
LFLAGS +=  /dsf=$(BIN_VMSDIRA)]$(PROGRAM)
#Do not link with /debug so that debug images can be "run"
#during the build without invoking the debugger which stops automated
#tests due to the user interface.
#Use $set image/flag=call_debug to debug an image.
#Note: This assumes the build is using /dsf= which gives the debugger debug
#information.
else
# Note: No advantage to /debug in a release build unless
#       LFLAGS += /dsf= is used.
# generate run-time .dsf files (Debug Symbol Table files)
LFLAGS +=  /dsf=$(BIN_VMSDIRA)]$(PROGRAM)
CFLAGS += /debug
CCFLAGS += /debug
CCFLAGS += /OPT=INLINE
endif

# SSL support
OPENSSL_VMSHOME = /Pegasus_Tools
OPENSSL_HOME = $(OPENSSL_VMSHOME)
OPENSSL_LIB = PEGASUS_OPENSSLLIB
PEGASUS_HAS_SSL = yes
OPENSSL_SET_SERIAL_SUPPORTED = true

PEGASUS_ARCHITECTURE_64BIT = yes

# PEGASUS_TEST_USER_DEFINED assumes user "pegtest"
PEGASUS_TEST_USER_DEFINED = true
PEGASUS_ENABLE_CMPI_PROVIDER_MANAGER = true
PEGASUS_ENABLE_SYSTEM_LOG_HANDLER = true
PEGASUS_ENABLE_EMAIL_HANDLER = true
PEGASUS_ENABLE_COMPRESSED_REPOSITORY = true
PEGASUS_ENABLE_PROTOCOL_WSMAN = true

# Local domain sockets, or an equivalent,
# is not currently supported on OpenVMS. Bug 2147
PEGASUS_DISABLE_LOCAL_DOMAIN_SOCKET=1

# The Provider User Context feature (PEP 197) is not yet supported on OpenVMS
PEGASUS_DISABLE_PROV_USERCTXT=1

CXX = cxx

SH = sh

YACC = bison

RM = mu rm

DIFF = mu compare

SORT = mu sort

COPY = mu copy

MOVE = mu move

PEGASUS_SUPPORTS_DYNLIB = yes

LIB_SUFFIX =.olb
