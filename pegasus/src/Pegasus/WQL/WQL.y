//%LICENSE////////////////////////////////////////////////////////////////
//
// Licensed to The Open Group (TOG) under one or more contributor license
// agreements.  Refer to the OpenPegasusNOTICE.txt file distributed with
// this work for additional information regarding copyright ownership.
// Each contributor licenses this file to you under the OpenPegasus Open
// Source License; you may not use this file except in compliance with the
// License.
//
// Permission is hereby granted, free of charge, to any person obtaining a
// copy of this software and associated documentation files (the "Software"),
// to deal in the Software without restriction, including without limitation
// the rights to use, copy, modify, merge, publish, distribute, sublicense,
// and/or sell copies of the Software, and to permit persons to whom the
// Software is furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included
// in all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
// OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
// MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
// IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
// CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
// TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
// SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
//
//////////////////////////////////////////////////////////////////////////
/*
**==============================================================================
**
** Includes
**
**==============================================================================
*/

%{

#include <Pegasus/Common/Config.h>
#include <Pegasus/WQL/WQLOperation.h>
#include <Pegasus/WQL/WQLOperand.h>
#include <Pegasus/WQL/WQLParserState.h>
#include <Pegasus/WQL/WQLSelectStatement.h>
#include <string.h>
#include <stdlib.h>

#ifdef PEGASUS_OS_TYPE_WINDOWS
# include <malloc.h>
#endif

#if defined(PEGASUS_COMPILER_ACC) && defined(PEGASUS_OS_HPUX)
# include <alloca.h>
#endif

#if 0
# define WQL_TRACE(X) printf X
#else
# define WQL_TRACE(X)
#endif

extern int WQL_lex();
extern int WQL_error(const char*);

//
// Define the global parser state object:
//

PEGASUS_USING_PEGASUS;

PEGASUS_NAMESPACE_BEGIN

extern WQLParserState* globalParserState;

PEGASUS_NAMESPACE_END

%}

/*
**==============================================================================
**
** Union used to pass tokens from Lexer to this Parser.
**
**==============================================================================
*/

%union
{
   int intValue;
   double doubleValue;
   char* strValue;
   void* nodeValue;
}

/*
**==============================================================================
**
** Tokens, types, and associative rules.
**
**==============================================================================
*/

%token <intValue> TOK_INTEGER
%token <doubleValue> TOK_DOUBLE
%token <strValue> TOK_STRING
%token <intValue> TOK_TRUE
%token <intValue> TOK_FALSE
%token <intValue> TOK_NULL
%token <intValue> TOK_ISA
%token <intValue> TOK_DOT

%token <intValue> TOK_EQ
%token <intValue> TOK_NE
%token <intValue> TOK_LT
%token <intValue> TOK_LE
%token <intValue> TOK_GT
%token <intValue> TOK_GE

%token <intValue> TOK_NOT
%token <intValue> TOK_OR
%token <intValue> TOK_AND
%token <intValue> TOK_IS

%token <strValue> TOK_IDENTIFIER
%token <intValue> TOK_SELECT
%token <intValue> TOK_WHERE
%token <intValue> TOK_FROM

%token <intValue> TOK_UNEXPECTED_CHAR

%type <strValue> propertyName
%type <nodeValue> propertyList
%type <nodeValue> predicate
%type <nodeValue> comparisonPredicate
// %type <nodeValue> comparisonTerm
%type <nodeValue> nullPredicate
%type <nodeValue> searchCondition
%type <nodeValue> fromClause
%type <nodeValue> whereClause
%type <nodeValue> selectStatement
%type <nodeValue> selectList
%type <nodeValue> selectExpression
%type <strValue> className
%type <intValue> truthValue

%left TOK_OR
%left TOK_AND
%nonassoc TOK_NOT

%%

/*
**==============================================================================
**
** The grammar itself.
**
**==============================================================================
*/

start
    : selectStatement
    {
    WQL_TRACE(("YACC: start\n"));
    }

selectStatement
    : TOK_SELECT selectList selectExpression
    {

    }

selectList
    : '*'
    {
    globalParserState->statement->setAllProperties(true);
    }
    | propertyList
    {

    }

propertyList
    : propertyName
    {
    globalParserState->statement->appendSelectPropertyName(CIMName($1));
    }
    | propertyList ',' propertyName
    {
    globalParserState->statement->appendSelectPropertyName(CIMName($3));
    }

selectExpression
    : fromClause whereClause
    {

    }
    | fromClause
    {

    }

fromClause
    : TOK_FROM className
    {
    WQL_TRACE(("YACC: fromClause: TOK_FROM className(%s)\n", $2));
    globalParserState->statement->setClassName(CIMName($2));
    }

whereClause
    : TOK_WHERE searchCondition
    {

    }

searchCondition
    : searchCondition TOK_OR searchCondition
    {
    WQL_TRACE(("YACC: TOK_OR\n"));
    globalParserState->statement->appendOperation(WQL_OR);
    }
    | searchCondition TOK_AND searchCondition
    {
    WQL_TRACE(("YACC: TOK_AND\n"));
    globalParserState->statement->appendOperation(WQL_AND);
    }
    | TOK_NOT searchCondition
    {
    WQL_TRACE(("YACC: TOK_NOT\n"));

    globalParserState->statement->appendOperation(WQL_NOT);
    }
    | '(' searchCondition ')'
    {

    }
    | predicate
    {

    }
    | predicate TOK_IS truthValue
    {
    WQLOperation op = $3 ? WQL_IS_TRUE : WQL_IS_FALSE;
    globalParserState->statement->appendOperation(op);
    }
    | predicate TOK_IS TOK_NOT truthValue
    {
    WQLOperation op = $4 ? WQL_IS_NOT_TRUE : WQL_IS_NOT_FALSE;
    globalParserState->statement->appendOperation(op);
    }

/******************************************************************************/

predicate
    : comparisonPredicate
    {

    }
    | nullPredicate
    {

    }

comparisonPredicate
    : comparisonTerm TOK_LT comparisonTerm
    {
    WQL_TRACE(("YACC: TOK_LT\n"));
    globalParserState->statement->appendOperation(WQL_LT);
    }
    | comparisonTerm TOK_GT comparisonTerm
    {
    WQL_TRACE(("YACC: TOK_GT\n"));
    globalParserState->statement->appendOperation(WQL_GT);
    }
    | comparisonTerm TOK_LE comparisonTerm
    {
    WQL_TRACE(("YACC: TOK_LE\n"));
    globalParserState->statement->appendOperation(WQL_LE);
    }
    | comparisonTerm TOK_GE comparisonTerm
    {
    WQL_TRACE(("YACC: TOK_GE\n"));
    globalParserState->statement->appendOperation(WQL_GE);
    }
    | comparisonTerm TOK_EQ comparisonTerm
    {
    WQL_TRACE(("YACC: TOK_EQ\n"));
    globalParserState->statement->appendOperation(WQL_EQ);
    }
    | comparisonTerm TOK_NE comparisonTerm
    {
    WQL_TRACE(("YACC: TOK_NE\n"));
    globalParserState->statement->appendOperation(WQL_NE);
    }
    | propertyName TOK_ISA className
    {
    WQL_TRACE(("YACC: TOK_ISA\n"));
#ifndef PEGASUS_SNIA_EXTENSIONS
        // If SNIA tests, allow the ISA but do not pass className
        yyerror("ISA Token Not Supported");
#endif
    }

nullPredicate
    : comparisonTerm TOK_IS TOK_NULL
    {
    WQL_TRACE(("YACC: nullPredicate : comparisonTerm IS NULL\n"));
    globalParserState->statement->appendOperation(WQL_IS_NULL);
    }
    | comparisonTerm TOK_IS TOK_NOT TOK_NULL
    {
    WQL_TRACE(("YACC: nullPredicate : comparisonTerm IS NOT NULL\n"));
    globalParserState->statement->appendOperation(WQL_IS_NOT_NULL);
    }

truthValue
    : TOK_TRUE
    {
    $$ = 1;
    }
    | TOK_FALSE
    {
    $$ = 0;
    }
/**************
propertyName
    : TOK_IDENTIFIER
    {
    WQL_TRACE(("YACC: propertyName : TOK_IDENTIFIER(%s)\n", $1));
    $$ = $1;
    }
*****************/
propertyName
    : TOK_IDENTIFIER
    {
        WQL_TRACE(("YACC: propertyName : TOK_IDENTIFIER(%s)\n", $1));
        $$ = $1;
    }
    | TOK_IDENTIFIER TOK_DOT TOK_IDENTIFIER
    {
        WQL_TRACE(("YACC: propertyName : TOK_IDENTIFIER(%s.%s)\n", $1, $3));
#ifdef PEGASUS_SNIA_EXTENSIONS
        // Pass anything as a property name to fool parser for SNIA testing
        $$ = strdup("dummy");
#else
        yyerror("Scoped (dotted) property names not supported");
#endif
    }


className : TOK_IDENTIFIER
    {
    WQL_TRACE(("YACC: TOK_IDENTIFIER %s\n", $1));
    $$ = $1;
    }

comparisonTerm
    : propertyName
    {
    globalParserState->statement->appendOperand(
        WQLOperand($1, WQL_PROPERTY_NAME_TAG));
    globalParserState->statement->appendWherePropertyName(CIMName($1));
    }
    | TOK_INTEGER
    {
    globalParserState->statement->appendOperand(
        WQLOperand($1, WQL_INTEGER_VALUE_TAG));
    }
    | TOK_DOUBLE
    {
    globalParserState->statement->appendOperand(
        WQLOperand($1, WQL_DOUBLE_VALUE_TAG));
    }
    | TOK_STRING
    {
    globalParserState->statement->appendOperand(
        WQLOperand($1, WQL_STRING_VALUE_TAG));
    }
    | truthValue
    {
    globalParserState->statement->appendOperand(
        WQLOperand($1 != 0, WQL_BOOLEAN_VALUE_TAG));
    }

%%
