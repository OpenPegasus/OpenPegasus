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

ARCHITECTURE = alpha

COMPILER = deccxx

SYS_VMSINCLUDES = -I$(ROOT)/src/stdcxx/cwrappers

DEFINES = -DPEGASUS_OS_VMS -DPEGASUS_PLATFORM_$(PEGASUS_PLATFORM)

DEPEND_INCLUDES =

TMP_OBJDIR = $(subst ../,,$(DIR))
OBJ_DIR = $(HOME_DIR)/obj/$(TMP_OBJDIR)
BIN_DIR = $(HOME_DIR)/bin
LIB_DIR = $(HOME_DIR)/lib
OPT_DIR = $(HOME_DIR)/opt

LFLAGS = /Threads_Enable=Upcalls
LFLAGS += /symbol_table=$(BIN_VMSDIRA)]$(PROGRAM)
LFLAGS += /map=$(BIN_VMSDIRA)]$(PROGRAM)/full/cross_ref/section_binding
CFLAGS = /main=POSIX_EXIT
CFLAGS += /names=(uppercase,shortened)
CFLAGS += /repos=$(CXXREPOSITORY_VMSROOT)/template_def=time

CCFLAGS = /main=POSIX_EXIT
CCFLAGS += /names=(uppercase,shortened)
CCFLAGS += /OPT=INLINE
CCFLAGS += /nowarn

ifdef PEGASUS_USE_DEBUG_BUILD_OPTIONS
CFLAGS += /debug/noopt/show=include/lis=$(OBJ_VMSDIRA)]
CCFLAGS += /debug/noopt
CCFLAGS += /show=include/lis=$(OBJ_VMSDIRA)]
LFLAGS += /debug
endif

# SSL support
OPENSSL_VMSHOME = /Pegasus_Tools
OPENSSL_HOME = $(OPENSSL_VMSHOME)
OPENSSL_LIB = PEGASUS_OPENSSLLIB
PEGASUS_HAS_SSL = yes
OPENSSL_SET_SERIAL_SUPPORTED = true

PEGASUS_ENABLE_CMPI_PROVIDER_MANAGER = true
PEGASUS_ENABLE_SYSTEM_LOG_HANDLER = true
PEGASUS_ENABLE_EMAIL_HANDLER = true
# Local domain sockets, or an equivalent,
# is not currently supported on OpenVMS. Bug 2147
PEGASUS_DISABLE_LOCAL_DOMAIN_SOCKET=1
# The Provider User Context feature (PEP 197) is not yet supported on OpenVMS
PEGASUS_DISABLE_PROV_USERCTXT=1
PEGASUS_USE_STATIC_LIBRARIES = true
PEGASUS_ENABLE_PROTOCOL_WSMAN = true

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

