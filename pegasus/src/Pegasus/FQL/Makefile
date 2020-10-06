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
ROOT = ../../..

DIR = Pegasus/FQL

include $(ROOT)/mak/config.mak

LOCAL_DEFINES = -DPEGASUS_FQL_INTERNAL -DPEGASUS_INTERNALONLY

## Bison/yacc  and FQL diagnostic displays
## Enable by uncommenting the following. They mpact the both the
## grammar compile and diagnostics in the code.
##LOCAL_DEFINES += -DFQL_YYDEBUG
##LOCAL_DEFINES += -DFQL_DOTRACE

ifneq ($(OS), HPUX)
    ifneq ($(OS), zos)
        EXTRA_INCLUDES = -Int_includes
    endif
endif

STATIC=1
LIBRARY = pegfql

#### TODO: NOTE THAT THIS USES QUERY COMMON.
LIBRARIES = \
   pegcommon \
   pegquerycommon

SOURCES = \
	FQLQueryStatement.cpp \
	FQLQueryStatementRep.cpp \
	FQLYACC.cpp \
	FQLLEX.cpp \
	FQLPropertySource.cpp \
	FQLInstancePropertySource.cpp \
	FQLParserState.cpp \
	FQLOperation.cpp \
	FQLOperand.cpp \
	FQLParser.cpp \
	FQLRegularExpression.cpp \
    FQLQueryExpressionRep.cpp

SOURCES_X = \
	FQLLEX.cpp \

include $(ROOT)/mak/library.mak

##------------------------------------------------------------------------------
##
## Rules to build LEX AND YACC sources:
##
## To make these files type: "make grammar"
##
## NOTE: Normally developers should never need to compile the FQL grammar.
## The generate source files are committed to CVS as part of any changes
## to the lex or yacc defintions so only developers of the grammar itsel
## need execute the make grammar below.
##
##------------------------------------------------------------------------------


##
## This pushes BISON_SIMPLE into the environment so that bison will use this
## rather than looking for /usr/local/share/bison.simple.
##

grammar: yacc_rule lex_rule

export BISON_SIMPLE = bison.simple

##LEX_DEBUG = -T
##YACC_DEBUG = -t

##
## Lex rule:
##
lex_rule:
	$(LEX) -oFQLLEX.cpp $(LEX_DEBUG) -PFQL_ FQL.l
	$(TOUCH) FQLLEX.cpp
	mu prepend ../nochksrc.txt FQLLEX.cpp

##
## Yacc rule:
##
yacc_rule:
ifeq ($(YACC),yacc)
	$(YACC) -vd -p FQL_ FQL.y
	$(COPY) "y.tab.c" "FQLYACC.cpp"
	$(COPY) "y.tab.h" "FQLYACC.h"
	$(RM) y.tab.c y.tab.h
	$(TOUCH) FQLYACC.cpp FQLYACC.h
else
	$(YACC) -d -v -oFQLYACC $(YACC_DEBUG) --report=all -p FQL_ FQL.y
	$(COPY) "FQLYACC" "FQLYACC.cpp"
	$(RM) FQLYACC
endif
	mu prepend ../nochksrc.txt FQLYACC.h
	mu prepend ../nochksrc.txt FQLYACC.cpp

## bison --graph option. Prepare vcg graph output. No real display for vcg
## --report=all.  Note that we always did -vd
##  -v same as --report=state
## -t should define YYDEBUG if it is not already defined
## -v verbose extra output file.
