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

DIR = Pegasus/CQL

include $(ROOT)/mak/config.mak

LOCAL_DEFINES = -DPEGASUS_CQL_INTERNAL -DPEGASUS_INTERNALONLY -DCQLINPUT

# The next line can be uncommented to turn on debug output from CQL.l
#LOCAL_DEFINES += -DCQL_DEBUG_LEXER

# The next line can be uncommented to turn on debug output from CQL.y
#LOCAL_DEFINES += -DCQL_DEBUG_GRAMMAR

ifneq ($(OS), HPUX)
ifneq ($(OS), zos)
EXTRA_INCLUDES = -Int_includes
endif
endif

STATIC=1
LIBRARY = pegcql

LIBRARIES = \
    pegquerycommon \
    pegcommon

SOURCES = \
   CQLYACC.cpp \
   CQLLEX.cpp \
   CQLChainedIdentifier.cpp \
   CQLChainedIdentifierRep.cpp \
   CQLIdentifier.cpp \
   CQLIdentifierRep.cpp \
   CQLParser.cpp \
   CQLParserState.cpp \
   CQLSimplePredicate.cpp \
   CQLSimplePredicateRep.cpp \
   CQLPredicate.cpp \
   CQLPredicateRep.cpp \
   CQLTerm.cpp \
   CQLTermRep.cpp \
   CQLFunction.cpp \
   CQLFunctionRep.cpp \
   CQLExpression.cpp \
   CQLExpressionRep.cpp \
   CQLQueryExpressionRep.cpp \
   CQLFactor.cpp \
   CQLFactorRep.cpp \
   CQLFactory.cpp \
   CQLSelectStatement.cpp \
   CQLSelectStatementRep.cpp \
   CQLValue.cpp \
   CQLValueRep.cpp \
   CQLRegularExpression.cpp \
   CQLUtilities.cpp \
   Cql2Dnf.cpp

include $(ROOT)/mak/library.mak


##-----------------------------------------------------------------------------
##
## Rules to build LEX AND YACC sources:
##
## To make these files type: "make grammar"
##
## NOTE: Normally developers should never need to compile the CQL grammar.
## The generate source files are committed to CVS as part of any changes
## to the lex or yacc defintions so only developers of the grammar itsel
## need execute the make grammar below.
##------------------------------------------------------------------------------


##
## This pushes BISON_SIMPLE into the environment so that bison will use this
## rather than looking for /usr/local/share/bison.simple.
##

grammar: yacc_rule lex_rule

export BISON_SIMPLE = bison.simple

##    WARNING: using bison and yacc generates slightly different
##    output messages which means that at least the Queries
##    Misc test resgood file must be changed if you move from
##    one to another.See bug 6351.
##YACC = bison
##YACC=yacc
##
## Lex rule:
##
lex_rule:
	$(LEX) -PCQL_ CQL.l
	$(COPY) lex.CQL_.c CQLLEX.cpp
	$(RM) lex.CQL_.c
	$(TOUCH) CQLLEX.cpp
	mu prepend ../nochksrc.txt CQLLEX.cpp
##
## Yacc rule:
##
yacc_rule:
ifeq ($(YACC),yacc)
	$(YACC) -vd -p CQL_ CQL.y
	$(COPY) y.tab.c CQLYACC.cpp
	$(COPY) y.tab.h CQLYACC.h
	$(RM) y.tab.c y.tab.h
	$(TOUCH) CQLYACC.cpp
else
	$(YACC) -vd -oCQLtemp -p CQL_ CQL.y
	$(COPY) CQLtemp CQLYACC.cpp
	$(COPY) CQLtemp.h CQLYACC.h
	$(RM) CQLtemp CQLtemp.h
	$(TOUCH) CQLYACC.cpp CQLYACC.h
endif
	mu prepend ../nochksrc.txt CQLYACC.h
	mu prepend ../nochksrc.txt CQLYACC.cpp
	
