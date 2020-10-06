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
ROOT = ../../../../../..

DIR = slp/slp_client/src/cmd-utils/slp_client

include $(ROOT)/mak/config.mak

ifeq ($(OS_TYPE),windows)
EXTRA_INCLUDES += -I./nt_includes
endif

# Treat all the SOURCES as C source only
#ifeq ($(OS_TYPE),windows)
#FLAGS += /TC
#endif
#ifeq ($(PEGASUS_PLATFORM),LINUX_IX86_GNU)
#FLAGS += -x c 
#endif

LIBRARY = pegslp_client

SOURCES = \
    slp_utils.cpp \
    slp_client.cpp \
    lslp.cpp \
    y_attr.cpp \
    l_attr.cpp \
    y_url.cpp \
    l_url.cpp \
    y_filter.cpp \
    l_filter.cpp \

ifeq ($(OS_TYPE),windows)
  SYS_LIBS = ws2_32.lib advapi32.lib
endif

include $(ROOT)/mak/library.mak

## Help on make usage of lex and yac grammar.
##
## make -f Makefile.grammar -- when both lex and yacc files are modified
## make -f Makefile.grammar y_grammar -- when only yacc files are modified
## make -f Makefile.grammar l_grammar -- when only lex files are modified
##
## Note that this compiles the lex or yacc files.
## However you should never need to use it unless
## you have modified these files.  Also, the generated files are committed
## to the CVS so that any updates to the generated files must be committed 
## also.
grammar:
	make -f Makefile.grammar

# DO NOT DELETE
