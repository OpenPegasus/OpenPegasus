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

include $(ROOT)/mak/config-unix.mak

OS = solaris

COMPILER = CC

CC = cc

CXX = CC

SH = sh

YACC = yacc

RM = rm -f

DIFF = diff

SORT = sort

COPY = cp

MOVE = mv

LIB_SUFFIX = .so

PEGASUS_SUPPORTS_DYNLIB = yes

SYS_INCLUDES =

##==============================================================================
##
## DEFINES
##
##==============================================================================

DEFINES += -DPEGASUS_PLATFORM_$(PEGASUS_PLATFORM)
DEFINES += -D_POSIX_PTHREAD_SEMANTICS
DEFINES += -DPEGASUS_OS_SOLARIS

##==============================================================================
##
## FLAGS
##
##==============================================================================

ifdef PEGASUS_USE_DEBUG_BUILD_OPTIONS
  FLAGS = -g -xs
else
  FLAGS = -O2 -s
endif

ifdef PEGASUS_SOLARIS_NEED_PTO
  FLAGS += -pto
endif

FLAGS += +w -KPIC -mt -xildoff

##==============================================================================
##
## COMMON_SYS_LIBS
##
##==============================================================================

COMMON_SYS_LIBS = -lpthread -ldl -lsocket -lnsl -lxnet -lCstd -lrt

##==============================================================================
##
## SYS_LIBS (system libraries needed to build programs)
##
##==============================================================================

SYS_LIBS = $(COMMON_SYS_LIBS) $(EXTRA_LIBRARIES)

##==============================================================================
##
## LIBRARY_SYS_LIBS (system libraries needed to build other libraries)
##
##==============================================================================

LIBRARY_SYS_LIBS = $(COMMON_SYS_LIBS)

##==============================================================================
##
## OpenSSL Settings
##
##     To build for SSL, add PEGASUS_HAS_SSL=1 to environment
##
##==============================================================================

ifndef OPENSSL_HOME
  OPENSSL_HOME=/usr/sfw
endif

ifndef OPENSSL_BIN
  OPENSSL_BIN=/usr/sfw/bin
endif

ifndef OPENSSL_COMMAND
  OPENSSL_COMMAND=/usr/sfw/bin/openssl
endif

ifndef PEGASUS_ENABLE_SSL_CRL_VERIFICATION
  PEGASUS_ENABLE_SSL_CRL_VERIFICATION=true
endif

ifndef PEGASUS_USE_SSL_RANDOMFILE
  PEGASUS_USE_SSL_RANDOMFILE=true
endif

ifdef PEGASUS_HAS_SSL
  FLAGS += -L$(OPENSSL_HOME)/$(LIBBASE)
  EXTRA_LIBRARIES += -L$(OPENSSL_HOME)/$(LIBBASE)
  LD_LIBRARY_PATH+=:$(OPENSSL_HOME)/$(LIBBASE)
  export LD_LIBRARY_PATH
endif

##==============================================================================
##
## OpenSPL Settings
##
##     Build for either OpenSlp or SolarisSLP.
##
##==============================================================================

ifndef PEGASUS_EXTERNAL_SLP_HOME
  PEGASUS_EXTERNAL_SLP_HOME=/usr
endif

ifeq ($(PEGASUS_USE_EXTERNAL_SLP),openslp)
  FLAGS += -L$(PEGASUS_EXTERNAL_SLP_HOME)/$(LIBBASE)
  LD_LIBRARY_PATH+=:$(PEGASUS_EXTERNAL_SLP_HOME)/$(LIBBASE)
  export LD_LIBRARY_PATH
endif
