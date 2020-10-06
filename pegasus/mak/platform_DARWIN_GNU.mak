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

OS = darwin

COMPILER = gnu

DEFINES += -DPEGASUS_PLATFORM_$(PEGASUS_PLATFORM)
DEFINES += -D_POSIX_PTHREAD_SEMANTICS
DEFINES += -D BSD_COMP
DEFINES += -D_REENTRANT
DEFINES += -DPEGASUS_OS_DARWIN

FLAGS = -W -Wall -Wno-unused -fPIC -bind_at_load

ifdef PEGASUS_USE_DEBUG_BUILD_OPTIONS
  FLAGS += -g
else
  FLAGS += -O2
endif

SYS_LIBS += -lpthread
SYS_LIBS += -ldl

ifndef CXX
  CXX = g++
endif

ifndef CC
  CC = cc
endif

SH = sh

YACC = yacc

RM = rm -f

DIFF = diff

SORT = sort

COPY = cp

MOVE = mv

LIB_SUFFIX = .dylib

PEGASUS_SUPPORTS_DYNLIB = yes

ifdef PEGASUS_USE_RELEASE_DIRS
  PEGASUS_DEST_LIB_DIR=/usr/lib/pegasus
endif
