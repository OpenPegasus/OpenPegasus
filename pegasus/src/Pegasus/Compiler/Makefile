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
#
#  NOTE:  The grammar and parser are no longer built in this makefile,
#  and in fact flex and bison are assumed to already have been run
#  and their output committed to CVS.  To rebuild the compiler grammar, run
#      make grammar
#           which executes the grammar makefile
#  to generate cimmof_tab.cpp, cimmof_tab.h, and cimmof_lex.cpp.
#  Remember to  commit them afterwards.
#  Please read the warnings in Makefile.grammar before trying to commit
#  changes to the compiler.
#

ROOT = ../../..

DIR = Pegasus/Compiler

include $(ROOT)/mak/config.mak

EXTRA_INCLUDES = $(SYS_INCLUDES)

ifeq ($(OS_TYPE),windows)
    EXTRA_INCLUDES += -I./nt_includes
endif

LOCAL_CLEAN_TARGET = local_clean_target

LOCAL_DEFINES = -DPEGASUS_COMPILER_INTERNAL -DPEGASUS_INTERNALONLY

ifeq ($(OS_TYPE),vms)
    STATIC=1
endif

LIBRARY = pegcompiler

LIBRARIES = \
    pegclient  \
    pegrepository \
    pegconfig \
    peggeneral \
    pegcommon

PRE_DEPEND_INCLUDES = -I./depends

SOURCES = \
    cimmof_lex_wrapper.cpp \
    cimmofParser.cpp \
    cimmofMessages.cpp \
    cimmofRepositoryInterface.cpp \
    cimmofClient.cpp \
    cimmofRepository.cpp \
    cimmof_tab_wrapper.cpp \
    parser.cpp \
    valueFactory.cpp \
    compilerDeclContext.cpp

ifdef PEGASUS_ENABLE_MRR_GENERATION
  SOURCES += cimmofMRR.cpp
  SOURCES += Closure.cpp
  LOCAL_DEFINES += -DPEGASUS_ENABLE_MRR_GENERATION
endif

include $(ROOT)/mak/dynamic-library.mak

$(OBJ_DIR)/cimmof_tab_wrapper$(OBJ) : cimmof_tab.cpp

$(OBJ_DIR)/cimmof_lex_wrapper$(OBJ) : cimmof_lex.cpp

local_clean_target:
	$(RM) cimmoftemp.out cimmoftemp.output
	
## Make the grammar files by executing Makefile.grammar
grammar:
	make -f Makefile.grammar
