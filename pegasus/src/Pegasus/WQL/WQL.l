/*
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
/////////////////////////////////////////////////////////////////////////
*/


%option never-interactive
%option nounput
%option noinput
%{

extern int WQLInput(char* buffer, int& numRead, int numRequested);
extern int WQL_error(const char*);

#define YY_INPUT(BUF, NREAD, NREQUESTED) WQLInput(BUF, NREAD, NREQUESTED)

#include <Pegasus/Common/Config.h>
#include <Pegasus/WQL/WQLParserState.h>
#include <cstring>
#include <cassert>
#include "WQLYACC.h"

#if 0
# define WQL_TRACE(X) printf X
#else
# define WQL_TRACE(X)
#endif

PEGASUS_NAMESPACE_BEGIN

extern WQLParserState* globalParserState;

PEGASUS_NAMESPACE_END

PEGASUS_USING_PEGASUS;

%}

POSITIVE_DECIMAL_DIGIT [1-9]
DECIMAL_DIGIT [0-9]
BLANK [ \t\n]
IDENT_CHAR [A-Za-z_]

%%

[Ss][Ee][Ll][Ee][Cc][Tt] {

    WQL_TRACE(("LEX: %s [TOK_SELECT]\n", yytext));
    return TOK_SELECT;
}

[Ff][Rr][Oo][Mm] {

    WQL_TRACE(("LEX: %s [TOK_FROM]\n", yytext));
    return TOK_FROM;
}

[Ww][Hh][Ee][Rr][Ee] {

    WQL_TRACE(("LEX: %s [TOK_WHERE]\n", yytext));
    return TOK_WHERE;
}

[Ii][Ss][Aa] {

   WQL_TRACE(("LEX: %s [TOK_ISA]\n", yytext));
   return TOK_ISA;
}

"\." {
  WQL_TRACE(("LEX: %s [TOK_DOT]\n", yytext));
  return TOK_DOT;
}



[Tt][Rr][Uu][Ee] {

    WQL_TRACE(("LEX: %s [TOK_TRUE]\n", yytext));
    return TOK_TRUE;
}

[Ff][Aa][Ll][Ss][Ee] {

    WQL_TRACE(("LEX: %s [TOK_FALSE]\n", yytext));
    return TOK_FALSE;
}

[Nn][Uu][Ll][Ll] {

    WQL_TRACE(("LEX: %s [TOK_NULL]\n", yytext));
    return TOK_NULL;
}

[Nn][Oo][Tt] {

    WQL_TRACE(("LEX: %s [TOK_NOT]\n", yytext));
    return TOK_NOT;
}

[Aa][Nn][Dd] {

    WQL_TRACE(("LEX: %s [TOK_AND]\n", yytext));
    return TOK_AND;
}

[Oo][Rr] {

    WQL_TRACE(("LEX: %s [TOK_OR]\n", yytext));
    return TOK_OR;
}

[Ii][Ss] {

    WQL_TRACE(("LEX: %s [TOK_IS]\n", yytext));
    return TOK_IS;
}

[-+]?{POSITIVE_DECIMAL_DIGIT}{DECIMAL_DIGIT}* {

    WQL_TRACE(("LEX: %s [TOK_INTEGER]\n", yytext));
    WQL_lval.intValue = strtol(yytext, (char**)0, 10);
    return TOK_INTEGER;
}

[+-]?0 {

    WQL_TRACE(("LEX: %s [TOK_INTEGER]\n", yytext));
    WQL_lval.intValue = 0;
    return TOK_INTEGER;
}

[-+]?{DECIMAL_DIGIT}*\.{DECIMAL_DIGIT}+([eE][+-]?{DECIMAL_DIGIT}+)? {

    WQL_TRACE(("LEX: %s [TOK_DOUBLE]\n", yytext));
    WQL_lval.doubleValue = strtod((char*)yytext, (char**)0);
    return TOK_DOUBLE;
}

\"[^\"\n]*\" {

    /* ATTN-B: handle long literals by using yyinput(). */
    /* ATTN-B: Handle expansion of special characters */

    WQL_TRACE(("LEX: %s [TOK_STRING]\n", yytext));

    /* Copy the string (but remove the surrounding quotes */

    {
    size_t n = strlen(yytext) - 2;
    char* strValue = new char[n + 1];
    memcpy(strValue, yytext + 1, n);
    strValue[n] = '\0';
    WQL_lval.strValue = strValue;
    globalParserState->outstandingStrings.append(strValue);
    }

    return TOK_STRING;
}

\"[^\"\n]*$ {

    WQL_error("Unterminated string");
}

[\*(),] {

    WQL_TRACE(("LEX: %c\n", yytext[0]));
    return yytext[0];
}

"=" {
    WQL_TRACE(("LEX: %s [TOK_EQ]\n", yytext));
    return TOK_EQ;
}

"!=" {

    WQL_TRACE(("LEX: %s [TOK_NE]\n", yytext));
    return TOK_NE;
}

"<=" {

    WQL_TRACE(("LEX: %s [TOK_LE]\n", yytext));
    return TOK_LE;
}

"<" {

    WQL_TRACE(("LEX: %s [TOK_LT]\n", yytext));
    return TOK_LT;
}

">=" {

    WQL_TRACE(("LEX: %s [TOK_GE]\n", yytext));
    return TOK_GE;
}

"<>" {

    WQL_TRACE(("LEX: %s [TOK_NE]\n", yytext));
    return TOK_NE;
}

">" {

    WQL_TRACE(("LEX: %s [TOK_GT]\n", yytext));
    return TOK_GT;
}

{IDENT_CHAR}({IDENT_CHAR}|{DECIMAL_DIGIT})*  {

    WQL_TRACE(("LEX: %s [TOK_IDENTIFIER]\n", yytext));

    {
    size_t n = strlen(yytext);
    char* strValue = new char[n + 1];
    memcpy(strValue, yytext, n);
    strValue[n] = '\0';
    WQL_lval.strValue = strValue;
    globalParserState->outstandingStrings.append(strValue);
    }

    return TOK_IDENTIFIER;
}

{BLANK}+ {

    /* Ignore blanks */
}

. {
    WQL_lval.intValue = 0;
    return TOK_UNEXPECTED_CHAR;
}

%%

extern "C" int WQL_wrap()
{
    return 1;
}
