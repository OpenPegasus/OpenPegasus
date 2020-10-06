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

OS = linux

ECHO = echo
MKDIRHIER = mkdir -p
RM = rm -f
TOUCH = touch

ifeq ($(findstring _CLANG, $(PEGASUS_PLATFORM)), _CLANG)
    COMPILER = clang
    COMPILE_CXX_COMMAND = clang++
    COMPILE_C_COMMAND = clang
else
    COMPILER = gnu
    COMPILE_CXX_COMMAND = g++
    COMPILE_C_COMMAND = gcc
endif
LIBRARY_LINK_COMMAND = $(COMPILE_CXX_COMMAND) -shared
PROGRAM_LINK_COMMAND = $(COMPILE_CXX_COMMAND)

SYS_LIBS = -ldl -lpthread -lcrypt

OBJ_SUFFIX=.o

DEFINES = -DPEGASUS_PLATFORM_$(PEGASUS_PLATFORM)
ifeq ($(COMPILER)),clang)
    PROGRAM_COMPILE_OPTIONS = -O2 -W -Wall -Wno-unused-parameter  -Wno-unused-value -D_GNU_SOURCE \
        -DTHREAD_SAFE -D_REENTRANT -Wno-unused-variable  -Wno-unused-function $(CXX_MACHINE_OPTIONS)
else
    PROGRAM_COMPILE_OPTIONS = -O2 -W -Wall -Wno-unused -D_GNU_SOURCE \
        -DTHREAD_SAFE -D_REENTRANT -fPIC $(CXX_MACHINE_OPTIONS)
endif

LIBRARY_COMPILE_OPTIONS = $(PROGRAM_COMPILE_OPTIONS)

PROGRAM_LINK_OPTIONS = -Xlinker -rpath -Xlinker $(PEGASUS_DEST_LIB_DIR) \
     $(LINK_MACHINE_OPTIONS)
LIBRARY_LINK_OPTIONS = $(PROGRAM_LINK_OPTIONS)
LINK_OUT = -o
SYM_LINK_LIB = $(PEGASUS_PROVIDER_LIB_DIR)/lib$(LIBRARY)
PLATFORM_LIB_SUFFIX = so
