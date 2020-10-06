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

OS = solaris

ECHO = echo
MKDIRHIER = mkdir -p
RM = rm -f
TOUCH = touch


SYM_LINK_LIB = $(PEGASUS_PROVIDER_LIB_DIR)/lib$(LIBRARY)

SYS_LIBS = -lpthread -lrt

OBJ_SUFFIX = .o
LINK_OUT = -o
PLATFORM_LIB_SUFFIX = so

ifdef PEGASUS_DEBUG
FLAGS = -g -KPIC -mt -xs -xildoff +w $(CXX_MACHINE_OPTIONS)
else
FLAGS = -O4 -KPIC -mt -xildoff -s -xipo=1 +w $(CXX_MACHINE_OPTIONS)
endif

DEFINES += -DPEGASUS_PLATFORM_$(PEGASUS_PLATFORM)

PROGRAM_COMPILE_OPTIONS = $(FLAGS)

PROGRAM_LINK_OPTIONS = $(PROGRAM_COMPILE_OPTIONS)

COMPILE_CXX_COMMAND = CC
COMPILE_C_COMMAND = cc
LIBRARY_LINK_COMMAND = $(COMPILE_CXX_COMMAND)
PROGRAM_LINK_COMMAND =  $(COMPILE_CXX_COMMAND)

LIBRARY_COMPILE_OPTIONS = $(PROGRAM_COMPILE_OPTIONS)
LIBRARY_LINK_OPTIONS = $(PROGRAM_LINK_OPTIONS) -G
