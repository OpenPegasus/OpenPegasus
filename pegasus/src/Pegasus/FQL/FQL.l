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

extern int FQLInput(char* buffer, int& numRead, int numRequested);
extern int FQL_error(const char*);

#define YY_INPUT(BUF, NREAD, NREQUESTED) FQLInput(BUF, NREAD, NREQUESTED)

// set no unistd.h flag
#define YY_NO_UNISTD_H

#include <Pegasus/Common/Config.h>
#include <Pegasus/FQL/FQLParserState.h>
#include <cstring>
#include <cassert>
#include "FQLYACC.h"

#ifdef FQL_DOTRACE
# define FQL_TRACE(X) printf X
#else
# define FQL_TRACE(X)
#endif

// Define the globalParserState update for each new token as a single
// macro so that we can use this or not.
// KS_TODO need to tie this to existence of the parameters, displays, etc.
// so we could consider a simplified version without the extras.
# define FQL_STATE_UPDATE  globalParserState->currentTokenPos+=yyleng; \
    globalParserState->tokenCount++

PEGASUS_NAMESPACE_BEGIN

extern FQLParserState* globalParserState;

PEGASUS_NAMESPACE_END

PEGASUS_USING_PEGASUS;

%}

/* TODO issues with unsignedInt, etc and the 0 one.*/
/* TODO sort out what this one is.*/
IDENT_CHAR [A-Za-z_]

SIGN [+-]
BINARY_DIGIT [01]
HEX_DIGIT [A-Fa-f0-9]
HEX_IDENT 0[Xx]
POSITIVE_DECIMAL_DIGIT [1-9]
DECIMAL_DIGIT [0-9]
BLANK [ \t\n]
/*IDENT_CHAR [0-9A-Za-z_\x80-\xFF]
IDENT_CHAR_1 [0-9A-Za-z_\x80-\xFF]
IDENT_CHAR_NO_NUM [A-Za-z_\x80-\xFF]*/
PROP_CHAR [A-Za-z0-9_\[\]\-\#\']

A [Aa]
B [Bb]
C [Cc]
D [Dd]
E [Ee]
F [Ff]
G [Gg]
H [Hh]
I [Ii]
J [Jj]
K [Kk]
L [Ll]
M [Mm]
N [Nn]
O [Oo]
P [Pp]
Q [Qq]
R [Rr]
S [Ss]
T [Tt]
U [Uu]
V [Vv]
W [Ww]
X [Xx]
Y [Yy]
Z [Zz]

%%

"\." {
    FQL_TRACE(("LEX: %s [TOK_DOT]\n", yytext));
    FQL_STATE_UPDATE;
    return TOK_DOT;
}

{T}{R}{U}{E} {
    FQL_TRACE(("LEX: %s [TOK_TRUE]\n", yytext));
    FQL_STATE_UPDATE;
    return TOK_TRUE;
}

{F}{A}{L}{S}{E} {
    FQL_TRACE(("LEX: %s [TOK_FALSE]\n", yytext));
    FQL_STATE_UPDATE;
    return TOK_FALSE;
}

{N}{U}{L}{L} {
    FQL_TRACE(("LEX: %s [TOK_NULL]\n", yytext));
    FQL_STATE_UPDATE;
    return TOK_NULL;
}

{N}{O}{T} {
    FQL_TRACE(("LEX: %s [TOK_NOT]\n", yytext));
    FQL_STATE_UPDATE;
    return TOK_NOT;
}

{A}{N}{D} {
    FQL_TRACE(("LEX: %s [TOK_AND]\n", yytext));
    FQL_STATE_UPDATE;
    return TOK_AND;
}

{O}{R} {

    FQL_TRACE(("LEX: %s [TOK_OR]\n", yytext));
    FQL_STATE_UPDATE;
    return TOK_OR;
}

{I}{S} {

    FQL_TRACE(("LEX: %s [TOK_IS]\n", yytext));
    FQL_STATE_UPDATE;
    return TOK_IS;
}

{A}{N}{Y} {

    FQL_TRACE(("LEX: %s [TOK_ANY]\n", yytext));
    FQL_STATE_UPDATE;
    return TOK_ANY;
}

{E}{V}{E}{R}{Y} {

    FQL_TRACE(("LEX: %s [TOK_EVERY]\n", yytext));
    FQL_STATE_UPDATE;
    return TOK_EVERY;
}

{L}{I}{K}{E} {

    FQL_TRACE(("LEX: %s [TOK_LIKE]\n", yytext));
    FQL_STATE_UPDATE;
    return TOK_LIKE;
}

[-+]?[0][Xx]{HEX_DIGIT}{HEX_DIGIT}* {
    FQL_TRACE(("LEX: %s [TOK_HEXADECIMAL]\n", yytext));
    FQL_STATE_UPDATE;
    FQL_lval.intValue = strtol(yytext, (char**)0, 16);
    return TOK_HEXADECIMAL;
}

[-+]?{BINARY_DIGIT}{BINARY_DIGIT}*{B} {
    FQL_TRACE(("LEX: %s [TOK_BINARY]\n", yytext));
    FQL_STATE_UPDATE;
    FQL_lval.intValue = strtol(yytext, (char**)0, 2);
    return TOK_BINARY;
}


[-+]?{POSITIVE_DECIMAL_DIGIT}{DECIMAL_DIGIT}* {

    FQL_TRACE(("LEX: %s [TOK_INTEGER]\n", yytext));
    FQL_STATE_UPDATE;
    FQL_lval.intValue = strtol(yytext, (char**)0, 10);
    FQL_TRACE(("LEX: return TOK_UNSIGNED_INTEGER %d\n", FQL_lval.intValue));
    return TOK_INTEGER;
}

[+-]?0 {

    FQL_TRACE(("LEX: %s [TOK_INTEGER]\n", yytext));
    FQL_STATE_UPDATE;
    FQL_lval.intValue = 0;
    return TOK_INTEGER;
}

[-+]?{DECIMAL_DIGIT}*\.{DECIMAL_DIGIT}+([eE][+-]?{DECIMAL_DIGIT}+)? {

    FQL_TRACE(("LEX: %s [TOK_DOUBLE]\n", yytext));
    FQL_STATE_UPDATE;
    FQL_lval.doubleValue = strtod((char*)yytext, (char**)0);
    return TOK_DOUBLE;
}

\'[^\'\n]*\' {

    /* ATTN-B: handle long literals by using yyinput(). */
    /* ATTN-B: Handle expansion of special characters */

    FQL_TRACE(("LEX: %s [TOK_STRING]\n", yytext));
    FQL_STATE_UPDATE;

    /* Copy the string (but remove the surrounding quotes */

    {
       size_t n = strlen(yytext) - 2;
       char* strValue = new char[n + 1];
       memcpy(strValue, yytext + 1, n);
       strValue[n] = '\0';
       FQL_lval.strValue = strValue;
       globalParserState->outstandingStrings.append(strValue);
    }

    return TOK_STRING;
}

\'[^\"\n]*$ {

    FQL_error("Unterminated string");
    FQL_STATE_UPDATE;
}


"=" {
    FQL_TRACE(("LEX: %s [TOK_EQ]\n", yytext));
    FQL_STATE_UPDATE;
    return TOK_EQ;
}

"<=" {
    FQL_TRACE(("LEX: %s [TOK_LE]\n", yytext));
    FQL_STATE_UPDATE;
    return TOK_LE;
}

"<" {
    FQL_TRACE(("LEX: %s [TOK_LT]\n", yytext));
    FQL_STATE_UPDATE;
    return TOK_LT;
}

">=" {
    FQL_TRACE(("LEX: %s [TOK_GE]\n", yytext));
    FQL_STATE_UPDATE;
    return TOK_GE;
}

"<>" {
    FQL_TRACE(("LEX: %s [TOK_NE]\n", yytext));
    FQL_STATE_UPDATE;
    return TOK_NE;
}

">" {
    FQL_TRACE(("LEX: %s [TOK_GT]\n", yytext));
    FQL_STATE_UPDATE;
    return TOK_GT;
}

"[" {
    FQL_TRACE(("LEX: [TOK_LBRKT]\n "));
    FQL_STATE_UPDATE;
    return TOK_LBRKT;
}

"]" {
    FQL_TRACE(("LEX: [TOK_RBRKT]\n "));
    FQL_STATE_UPDATE;
    return TOK_RBRKT;
}

"{" {
    FQL_TRACE(("LEX: [TOK_LBRACE] \n"));
    FQL_STATE_UPDATE;
    return TOK_LBRACE;
}

"}" {
    FQL_TRACE(("LEX: [TOK_RBRACE]\n "));
    FQL_STATE_UPDATE;
    return TOK_RBRACE;
}

\({1} {
    FQL_TRACE(("LEX: [TOK_LPAREN] \n"));
    FQL_STATE_UPDATE;
    return TOK_LPAREN;
}

\){1} {
    FQL_TRACE(("LEX: [TOK_RPAREN]\n "));
    FQL_STATE_UPDATE;
    return TOK_RPAREN;
}

"," {
    FQL_TRACE(("LEX: [TOK_COMMA]\n "));
    FQL_STATE_UPDATE;
    return TOK_COMMA;
}

{IDENT_CHAR}({IDENT_CHAR}|{DECIMAL_DIGIT})*  {

    FQL_TRACE(("LEX: %s [TOK_IDENTIFIER]\n", yytext));
    FQL_STATE_UPDATE;
    {
       size_t n = strlen(yytext);
       char* strValue = new char[n + 1];
       memcpy(strValue, yytext, n);
       strValue[n] = '\0';
       FQL_lval.strValue = strValue;
       globalParserState->outstandingStrings.append(strValue);
    }

    return TOK_IDENTIFIER;
}



{BLANK}+ {

    /* Ignore blanks */
}

. {
    FQL_lval.intValue = 0;
    return TOK_UNEXPECTED_CHAR;
}

%%

extern "C" int FQL_wrap()
{
    return 1;
}

/*
{POSITIVE_DECIMAL_DIGIT}{DECIMAL_DIGIT}* {

    FQL_TRACE(("LEX: %s [TOK_INTEGER]\n", yytext));
    printf("LEX: found POSITIVEDECIMAL %s\n", yytext);
    FQL_lval.intValue = strtol(yytext, (char**)0, 10);
    // TODO returns in, not long
    FQL_TRACE("LEX: return TOK_UNSIGNED_INTEGER %d\n", FQL_lval.intValue);
    return TOK_UNSIGNED_INTEGER;
}

// removed comma from this one
[\*(),] {

    FQL_TRACE(("LEX: %c\n", yytext[0]));
    return yytext[0];
}

 */
