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

ARCHITECTURE = sparc

COMPILER = gnu

SYS_INCLUDES =

DEFINES = -DPEGASUS_PLATFORM_$(PEGASUS_PLATFORM) -D_POSIX_PTHREAD_SEMANTICS -D BSD_COMP -D_REENTRANT

DEFINES += -DPEGASUS_OS_SOLARIS

SUNOS_VERSION = $(shell uname -r)


# Pegasus requires the kernel LWP thread model.
# It doesn't exist on SunOS 5.6 or 5.7 so thery are no longer supported.
#
ifeq ($(SUNOS_VERSION), 5.6)
DEFINES += -DSUNOS_5_6
    $(error SunOS version 5.6 is not supportted)
endif

# Pegasus requires the kernel LWP thread model.
# It doesn't exist on SunOS 5.6 or 5.7 so thery are no longer supported.
#
ifeq ($(SUNOS_VERSION), 5.7)
DEFINES += -DSUNOS_5_7
    $(error SunOS version 5.7 is not supportted)
endif

ifeq ($(SUNOS_VERSION), 5.8)
DEFINES += -DSUNOS_5_8
endif

ifdef PEGASUS_USE_DEBUG_BUILD_OPTIONS
FLAGS = -g -W -Wall -Wno-unused -fPIC
else
FLAGS = -O2 -W -Wall -Wno-unused -fPIC
endif

SYS_LIBS = -lpthread -ldl -lsocket -lnsl -lxnet -lrt

# on SunOS 5.8 use the alternate (kernel LWP) thread model that is standard on
# SunOS 5.9 and 5.10
#
ifeq ($(SUNOS_VERSION), 5.8)
SYS_LIBS += -R /usr/lib/lwp
endif

CXX = g++

CC = gcc

SH = sh

YACC = yacc

RM = rm -f

DIFF = diff

SORT = sort

COPY = cp

MOVE = mv

LIB_SUFFIX = .so

PEGASUS_SUPPORTS_DYNLIB = yes
