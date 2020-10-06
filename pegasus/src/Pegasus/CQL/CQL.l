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
//////////////////////////////////////////////////////////////////////////
*/

/*
   This file describes the language tokens possible for CQL. When a token is
   matched, the token postion is updated in the CQL_globalParserState, copying
   from the lex buffer  to a shared buffer (shared with the lexer and parser)
   may happen, then a token identifier is returned to the parser.

*/

%option never-interactive
%option nounput
%option noinput
%{
extern int CQLInput(char* buffer, int& numRead, int numRequested);
extern int CQL_error(const char*);

#ifdef CQLINPUT
#define YY_INPUT(BUF, NREAD, NREQUESTED) CQLInput(BUF, NREAD, NREQUESTED)
#endif

#include <Pegasus/Common/Config.h>
#include <Pegasus/Common/CommonUTF.h>
#include <Pegasus/Common/MessageLoader.h>
#include "CQLParserState.h"
#include <stdlib.h>
#include <stdio.h>
#include <cstring>
#include "CQLObjects.h"
#include "CQLYACC.h"
PEGASUS_NAMESPACE_BEGIN

extern CQLParserState* CQL_globalParserState;

PEGASUS_NAMESPACE_END


#ifdef CQL_DEBUG_LEXER
#define CQL_DEBUG_TRACE(X) \
    printf(X);
#define CQL_DEBUG_TRACE2(X,Y) \
    printf(X,Y);
#else
#define CQL_DEBUG_TRACE(X)
#define CQL_DEBUG_TRACE2(X,Y)
#endif

int lineno;

%}
SIGN [+-]
BINARY_DIGIT [01]
HEX_DIGIT [A-Fa-f0-9]
HEX_IDENT 0[Xx]
POSITIVE_DECIMAL_DIGIT [1-9]
DECIMAL_DIGIT [0-9]
BLANK [ \t]
IDENT_CHAR [0-9A-Za-z_\x80-\xFF]
IDENT_CHAR_1 [0-9A-Za-z_\x80-\xFF]
IDENT_CHAR_NO_NUM [A-Za-z_\x80-\xFF]
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

{S}{E}{L}{E}{C}{T} {

    CQL_DEBUG_TRACE2("LEX: %s [TOK_SELECT] ", yytext);
    CQL_globalParserState->currentTokenPos+=yyleng;
    CQL_globalParserState->tokenCount++;
    return TOK_SELECT;
}

{F}{R}{O}{M} {

    CQL_DEBUG_TRACE2("LEX: %s [TOK_FROM] ", yytext);
    CQL_globalParserState->currentTokenPos+=yyleng;
    CQL_globalParserState->tokenCount++;
    return TOK_FROM;
}

{W}{H}{E}{R}{E} {

    CQL_DEBUG_TRACE2("LEX: %s [TOK_WHERE] ", yytext);
    CQL_globalParserState->currentTokenPos+=yyleng;
    CQL_globalParserState->tokenCount++;
    return TOK_WHERE;
}

{A}{N}{Y} {

    CQL_DEBUG_TRACE2("LEX: %s [TOK_ANY] ", yytext);
    CQL_globalParserState->currentTokenPos+=yyleng;
    CQL_globalParserState->tokenCount++;
    return TOK_ANY;
}

{A}{S} {
    CQL_DEBUG_TRACE2("LEX: %s [TOK_AS] ", yytext);
    CQL_globalParserState->currentTokenPos+=yyleng;
    CQL_globalParserState->tokenCount++;
    return TOK_AS;
}

{A}{S}{C} {
    CQL_DEBUG_TRACE2("LEX: %s [TOK_ASC] ", yytext);
    CQL_globalParserState->currentTokenPos+=yyleng;
    CQL_globalParserState->tokenCount++;
    return TOK_ASC;
}

{B}{Y} {
    CQL_DEBUG_TRACE2("LEX: %s [TOK_BY] ", yytext);
    CQL_globalParserState->currentTokenPos+=yyleng;
    CQL_globalParserState->tokenCount++;
    return TOK_BY;
}

{D}{E}{S}{C} {
    CQL_DEBUG_TRACE2("LEX: %s [TOK_DESC] ", yytext);
    CQL_globalParserState->currentTokenPos+=yyleng;
    CQL_globalParserState->tokenCount++;
    return TOK_DESC;
}


{D}{I}{S}{T}{I}{N}{C}{T} {
    CQL_DEBUG_TRACE2("LEX: %s [TOK_DISTINCT] ", yytext);
    CQL_globalParserState->currentTokenPos+=yyleng;
    CQL_globalParserState->tokenCount++;
    return TOK_DISTINCT;
}

{E}{V}{E}{R}{Y} {
    CQL_DEBUG_TRACE2("LEX: %s [TOK_EVERY] ", yytext);
    CQL_globalParserState->currentTokenPos+=yyleng;
    CQL_globalParserState->tokenCount++;
    return TOK_EVERY;
}

{F}{I}{R}{S}{T} {
    CQL_DEBUG_TRACE2("LEX: %s [TOK_FIRST] ", yytext);
    CQL_globalParserState->currentTokenPos+=yyleng;
    CQL_globalParserState->tokenCount++;
    return TOK_FIRST;
}

{I}{N} {
    CQL_DEBUG_TRACE2("LEX: %s [TOK_IN] ", yytext);
    CQL_globalParserState->currentTokenPos+=yyleng;
    CQL_globalParserState->tokenCount++;
    return TOK_IN;
}

{I}{S} {
    CQL_DEBUG_TRACE2("LEX: %s [TOK_IS] ", yytext);
    CQL_globalParserState->currentTokenPos+=yyleng;
    CQL_globalParserState->tokenCount++;
    return TOK_IS;
}

{I}{S}{A} {
    CQL_DEBUG_TRACE2("LEX: %s [ISA] ", yytext);
    CQL_globalParserState->currentTokenPos+=yyleng;
    CQL_globalParserState->tokenCount++;
    return TOK_ISA;
}

{L}{I}{K}{E} {
    CQL_DEBUG_TRACE2("LEX: %s [TOKE_LIKE] ", yytext);
    CQL_globalParserState->currentTokenPos+=yyleng;
    CQL_globalParserState->tokenCount++;
    return TOK_LIKE;
}

{O}{R}{D}{E}{R} {
    CQL_DEBUG_TRACE2("LEX: %s [TOK_ORDER] ", yytext);
    CQL_globalParserState->currentTokenPos+=yyleng;
    CQL_globalParserState->tokenCount++;
    return TOK_ORDER;
}

{S}{A}{T}{I}{S}{F}{I}{E}{S} {
    CQL_DEBUG_TRACE2("LEX: %s [TOK_SATISFIES] ", yytext);
    CQL_globalParserState->currentTokenPos+=yyleng;
    CQL_globalParserState->tokenCount++;
    return TOK_SATISFIES;
}

{T}{R}{U}{E} {
    CQL_DEBUG_TRACE2("LEX: %s [TOK_TRUE] ", yytext);
    CQL_globalParserState->currentTokenPos+=yyleng;
    CQL_globalParserState->tokenCount++;
    return TOK_TRUE;
}

{F}{A}{L}{S}{E} {
    CQL_DEBUG_TRACE2("LEX: %s [TOK_FALSE] ", yytext);
    CQL_globalParserState->currentTokenPos+=yyleng;
    CQL_globalParserState->tokenCount++;
    return TOK_FALSE;
}

{N}{U}{L}{L} {
    CQL_DEBUG_TRACE2("LEX: %s [TOK_NULL] ", yytext);
    CQL_globalParserState->currentTokenPos+=yyleng;
    CQL_globalParserState->tokenCount++;
    return TOK_NULL;
}

{N}{O}{T} {
    CQL_DEBUG_TRACE2("LEX: %s [TOK_NOT] ", yytext);
    CQL_globalParserState->currentTokenPos+=yyleng;
    CQL_globalParserState->tokenCount++;
    return TOK_NOT;
}

{A}{N}{D} {
    CQL_DEBUG_TRACE2("LEX: %s [TOK_AND] ", yytext);
    CQL_globalParserState->currentTokenPos+=yyleng;
    CQL_globalParserState->tokenCount++;
    return TOK_AND;
}

{O}{R} {
    CQL_DEBUG_TRACE2("LEX: %s [TOK_OR] ", yytext);
    CQL_globalParserState->currentTokenPos+=yyleng;
    CQL_globalParserState->tokenCount++;
    return TOK_OR;
}
    /* NOCHKSRC */
{IDENT_CHAR_NO_NUM}{IDENT_CHAR}*\:\:{IDENT_CHAR_NO_NUM}({IDENT_CHAR}*|{IDENT_CHAR}*\[.*\]|{IDENT_CHAR}*#\'.*\') {
    CQL_globalParserState->currentTokenPos+=yyleng;
    CQL_globalParserState->tokenCount++;
     /* remove any single quotes surrounding embedded literals */
     size_t n;
     if(CQL_lval.strValue)
             delete [] CQL_lval.strValue;
     /* CHKSRC */
    // chuck
    if (!isUTF8Str(yytext))
    {
        CQL_DEBUG_TRACE("LEX: [STRING]-> BAD UTF\n");
        throw CQLSyntaxErrorException(
            MessageLoaderParms("CQL.CQL_y.BAD_UTF8",
            "Bad UTF8 encountered parsing rule $0 in position $1.",
            "identifier",
            CQL_globalParserState->currentTokenPos));
    }

     String s(yytext);
     Uint32 index = s.find("'");
     if(index != PEG_NOT_FOUND){
        s.remove(index,1);
        s.remove(s.size()-1,1);
        CString cstr = s.getCString();
        const char* string = (const char*)cstr;
        n = strlen(string);
        CQL_lval.strValue = new char[n+1];
        memcpy(CQL_lval.strValue, string, n);
     }else{
        n = strlen(yytext);
        CQL_lval.strValue = new char[n+1];
      memcpy(CQL_lval.strValue, yytext, n);
     }
    CQL_lval.strValue[n] = '\0';
    CQL_DEBUG_TRACE2("LEX: %s [SCOPED_PROPERTY] ", CQL_lval.strValue);

    return TOK_SCOPED_PROPERTY;
}

[-]{1}[0][Xx]{HEX_DIGIT}{HEX_DIGIT}* {
    CQL_DEBUG_TRACE2("LEX: %s [TOK_NEGATIVE_HEXADECIMAL] ", yytext);
    CQL_globalParserState->currentTokenPos+=yyleng;
    CQL_globalParserState->tokenCount++;
    /* copy the hex value */
    {
        size_t n = strlen(yytext);
    if(CQL_lval.strValue)
        delete [] CQL_lval.strValue;
        CQL_lval.strValue = new char[n + 1];
        memcpy(CQL_lval.strValue, yytext, n);
        CQL_lval.strValue[n] = '\0';
    }
    return TOK_NEGATIVE_HEXADECIMAL;
}

[+]?[0][Xx]{HEX_DIGIT}{HEX_DIGIT}* {
    CQL_DEBUG_TRACE2("LEX: %s [TOK_HEXADECIMAL] ", yytext);
    CQL_globalParserState->currentTokenPos+=yyleng;
    CQL_globalParserState->tokenCount++;
    /* copy the hex value */
    {
        size_t n = strlen(yytext);
        if(CQL_lval.strValue)
                delete [] CQL_lval.strValue;
        CQL_lval.strValue = new char[n + 1];
        memcpy(CQL_lval.strValue, yytext, n);
        CQL_lval.strValue[n] = '\0';
    }
    return TOK_HEXADECIMAL;
}

[-]{1}{BINARY_DIGIT}{BINARY_DIGIT}*{B} {
    CQL_DEBUG_TRACE2("LEX: %s [TOK_NEGATIVE_BINARY] ", yytext);
    CQL_globalParserState->currentTokenPos+=yyleng;
    CQL_globalParserState->tokenCount++;
    /* copy the bin value */
    {
        size_t n = strlen(yytext);
    if(CQL_lval.strValue)
                delete [] CQL_lval.strValue;
        CQL_lval.strValue = new char[n + 1];
        memcpy(CQL_lval.strValue, yytext, n);
        CQL_lval.strValue[n] = '\0';
    }
    return TOK_NEGATIVE_BINARY;
}

[+]?{BINARY_DIGIT}{BINARY_DIGIT}*{B} {
    CQL_DEBUG_TRACE2("LEX: %s [TOK_BINARY] ", yytext);
    CQL_globalParserState->currentTokenPos+=yyleng;
    CQL_globalParserState->tokenCount++;
    /* copy the bin value */
    {
        size_t n = strlen(yytext);
        if(CQL_lval.strValue)
                delete [] CQL_lval.strValue;
        CQL_lval.strValue = new char[n + 1];
        memcpy(CQL_lval.strValue, yytext, n);
        CQL_lval.strValue[n] = '\0';
    }
    return TOK_BINARY;
}


[-]{1}{POSITIVE_DECIMAL_DIGIT}{DECIMAL_DIGIT}* {

    CQL_DEBUG_TRACE2("LEX: %s [TOK_NEGATIVE_INTEGER] ", yytext);
    CQL_globalParserState->currentTokenPos+=yyleng;
    CQL_globalParserState->tokenCount++;
    size_t n = strlen(yytext);
    if(CQL_lval.strValue)
        delete [] CQL_lval.strValue;
    CQL_lval.strValue = new char[n + 1];
    memcpy(CQL_lval.strValue, yytext, n);
    CQL_lval.strValue[n] = '\0';

    /*CQL_lval.intValue = strtol(yytext, (char**)0, 10);*/
    return TOK_NEGATIVE_INTEGER;
}

[+]?{DECIMAL_DIGIT}+ {

    CQL_DEBUG_TRACE2("LEX: %s [TOK_INTEGER] ", yytext);
    CQL_globalParserState->currentTokenPos+=yyleng;
    CQL_globalParserState->tokenCount++;
    size_t n = strlen(yytext);
    if(CQL_lval.strValue)
            delete [] CQL_lval.strValue;
    CQL_lval.strValue = new char[n + 1];
    memcpy(CQL_lval.strValue, yytext, n);
    CQL_lval.strValue[n] = '\0';

    /*CQL_lval.intValue = strtol(yytext, (char**)0, 10);*/
    return TOK_INTEGER;
}

[-]{1}{DECIMAL_DIGIT}*\.{DECIMAL_DIGIT}+([eE][+-]?{DECIMAL_DIGIT}+)? {

    CQL_DEBUG_TRACE2("LEX: %s [TOK_NEGATIVE_REAL] ", yytext);
    CQL_globalParserState->currentTokenPos+=yyleng;
    CQL_globalParserState->tokenCount++;
    size_t n = strlen(yytext);
    if(CQL_lval.strValue)
            delete [] CQL_lval.strValue;
    CQL_lval.strValue = new char[n + 1];
    memcpy(CQL_lval.strValue, yytext, n);
    CQL_lval.strValue[n] = '\0';

    /*CQL_lval.realValue = strtod((char*)yytext, (char**)0);*/
    return TOK_NEGATIVE_REAL;
}
    /* NOCHKSRC */
([+]?{DECIMAL_DIGIT}*\.{DECIMAL_DIGIT}+([eE][+-]?{DECIMAL_DIGIT}+)?)|([+]?{DECIMAL_DIGIT}+\.{DECIMAL_DIGIT}*([eE][+-]?{DECIMAL_DIGIT}+)?) {
    CQL_DEBUG_TRACE2("LEX: %s [TOK_REAL] ", yytext);
    CQL_globalParserState->currentTokenPos+=yyleng;
    CQL_globalParserState->tokenCount++;
    size_t n = strlen(yytext);
    if(CQL_lval.strValue)
            delete [] CQL_lval.strValue;
    CQL_lval.strValue = new char[n + 1];
    memcpy(CQL_lval.strValue, yytext, n);
    CQL_lval.strValue[n] = '\0';

    /*CQL_lval.realValue = strtod((char*)yytext, (char**)0);*/
    return TOK_REAL;
}
    /* CHKSRC */
\'((\\')|(\\\\)|[^'(\\')])*\' {
    /* \'[^\'\n]*\' */
    /* ATTN-B: handle long literals by using yyinput(). */
    /* ATTN-B: Handle expansion of special characters */

    CQL_DEBUG_TRACE2("LEX: %s [TOK_STRING_LITERAL] ", yytext);
    CQL_globalParserState->currentTokenPos+=yyleng;
    CQL_globalParserState->tokenCount++;
    /* remove any escaped \ OR escaped ' */

    String _esc_doubleslash = "\\\\";
    String _esc_singlequote = "\\'";

    if (!isUTF8Str(yytext))
    {
        CQL_DEBUG_TRACE("LEX: [STRING]-> BAD UTF\n");
        throw CQLSyntaxErrorException(
            MessageLoaderParms("CQL.CQL_y.BAD_UTF8",
                "Bad UTF8 encountered parsing rule $0 in position $1.",
                "literal_string",
                CQL_globalParserState->currentTokenPos));
    }

    String s(yytext);

    Uint32 index = 1;
    while((index = s.find(_esc_doubleslash)) != PEG_NOT_FOUND ||
           (index = s.find(_esc_singlequote)) != PEG_NOT_FOUND)
     {
         // make sure we don't remove the slasy from say 'abc\'
         if(index == s.size() - 2)
                break;
         s.remove(index,1);
    }
    CString cstr = s.getCString();
    const char* string = (const char*)cstr;

    /* Copy the string (but remove the surrounding quotes */

    {
    size_t n = strlen(string) - 2;
    if(CQL_lval.strValue)
        delete [] CQL_lval.strValue;
    CQL_lval.strValue = new char[n + 1];
    memcpy(CQL_lval.strValue, string + 1, n);
    CQL_lval.strValue[n] = '\0';
    }
    return TOK_STRING_LITERAL;
}

\'[^\'\n]*$ {

    CQL_DEBUG_TRACE("Unterminated string");
    CQL_globalParserState->currentTokenPos+=yyleng;
    CQL_globalParserState->tokenCount++;
}

\*{1} {
   CQL_DEBUG_TRACE("LEX: [TOK_STAR] ");
   CQL_globalParserState->currentTokenPos+=yyleng;
   CQL_globalParserState->tokenCount++;
   return TOK_STAR;
}

\/{1} {
   CQL_DEBUG_TRACE("LEX: [TOK_DIV] ");
   CQL_globalParserState->currentTokenPos+=yyleng;
   CQL_globalParserState->tokenCount++;
   return TOK_DIV;
}

\+{1} {
   CQL_DEBUG_TRACE("LEX: [TOK_PLUS] ");
   CQL_globalParserState->currentTokenPos+=yyleng;
   CQL_globalParserState->tokenCount++;
   return TOK_PLUS;
}

\-{1} {
   CQL_DEBUG_TRACE("LEX: [TOK_MINUS] ");
   CQL_globalParserState->currentTokenPos+=yyleng;
   CQL_globalParserState->tokenCount++;
   return TOK_MINUS;
}

\,{1} {
   CQL_DEBUG_TRACE("LEX: [TOK_COMMA] ");
   CQL_globalParserState->currentTokenPos+=yyleng;
   CQL_globalParserState->tokenCount++;
   return TOK_COMMA;
}

\.{2} {
    CQL_DEBUG_TRACE("LEX: [TOK_DOTDOT] ");
    CQL_globalParserState->currentTokenPos+=yyleng;
    CQL_globalParserState->tokenCount++;
    return TOK_DOTDOT;
}

\#{1} {
    CQL_DEBUG_TRACE("LEX: [TOK_HASH] ");
    CQL_globalParserState->currentTokenPos+=yyleng;
    CQL_globalParserState->tokenCount++;
    return TOK_HASH;
}

\.{1} {
    CQL_DEBUG_TRACE("LEX: [TOK_DOT] ");
    CQL_globalParserState->currentTokenPos+=yyleng;
    CQL_globalParserState->tokenCount++;
    return TOK_DOT;
}

\[{1} {
    CQL_DEBUG_TRACE("LEX: [TOK_LBRKT] ");
    CQL_globalParserState->currentTokenPos+=yyleng;
    CQL_globalParserState->tokenCount++;
    return TOK_LBRKT;
}

\]{1} {
    CQL_DEBUG_TRACE("LEX: [TOK_RBRKT] ");
    CQL_globalParserState->currentTokenPos+=yyleng;
    CQL_globalParserState->tokenCount++;
    return TOK_RBRKT;
}

\({1} {
    CQL_DEBUG_TRACE("LEX: [TOK_LPAR] ");
    CQL_globalParserState->currentTokenPos+=yyleng;
    CQL_globalParserState->tokenCount++;
    return TOK_LPAR;
}

\){1} {
    CQL_DEBUG_TRACE("LEX: [TOK_RPAR] ");
    CQL_globalParserState->currentTokenPos+=yyleng;
    CQL_globalParserState->tokenCount++;
    return TOK_RPAR;
}

"||" {
    CQL_DEBUG_TRACE2("LEX: %s [TOK_DBL_PIPE] ", yytext);
    CQL_globalParserState->currentTokenPos+=yyleng;
    CQL_globalParserState->tokenCount++;
    return TOK_DBL_PIPE;
}

"_" {
    CQL_DEBUG_TRACE2("LEX: %s [TOK_UNDERSCORE] ", yytext);
    CQL_globalParserState->currentTokenPos+=yyleng;
    CQL_globalParserState->tokenCount++;
    return TOK_UNDERSCORE;
}

"=" {
    CQL_DEBUG_TRACE2("LEX: %s [TOK_EQ] ", yytext);
    CQL_globalParserState->currentTokenPos+=yyleng;
    CQL_globalParserState->tokenCount++;
    return TOK_EQ;
}


"<>" {

    CQL_DEBUG_TRACE2("LEX: %s [TOK_NE] ", yytext);
    CQL_globalParserState->currentTokenPos+=yyleng;
    CQL_globalParserState->tokenCount++;
    return TOK_NE;
}


"<=" {

    CQL_DEBUG_TRACE2("LEX: %s [TOK_LE] ", yytext);
    CQL_globalParserState->currentTokenPos+=yyleng;
    CQL_globalParserState->tokenCount++;
    return TOK_LE;
}

"<" {

    CQL_DEBUG_TRACE2("LEX: %s [TOK_LT] ", yytext);
    CQL_globalParserState->currentTokenPos+=yyleng;
    CQL_globalParserState->tokenCount++;
    return TOK_LT;
}

">=" {

    CQL_DEBUG_TRACE2("LEX: %s [TOK_GE] ", yytext);
    CQL_globalParserState->currentTokenPos+=yyleng;
    CQL_globalParserState->tokenCount++;
    return TOK_GE;
}

">" {

    CQL_DEBUG_TRACE2("LEX: %s [TOK_GT] ", yytext);
    CQL_globalParserState->currentTokenPos+=yyleng;
    CQL_globalParserState->tokenCount++;
    return TOK_GT;
}

{IDENT_CHAR_NO_NUM}({IDENT_CHAR})*  {

    CQL_DEBUG_TRACE2("LEX: %s [TOK_IDENTIFIER] ", yytext);
    CQL_globalParserState->currentTokenPos+=yyleng;
    CQL_globalParserState->tokenCount++;
    {
        size_t n = strlen(yytext);
        if(CQL_lval.strValue)
            delete [] CQL_lval.strValue;
        CQL_lval.strValue = new char[n + 1];
        memcpy(CQL_lval.strValue, yytext, n);
        CQL_lval.strValue[n] = '\0';
    }

    return TOK_IDENTIFIER;
}

{BLANK}+ {

    /* Ignore blanks */
    CQL_globalParserState->currentTokenPos+=yyleng;
}

\n {
    CQL_globalParserState->currentTokenPos=0;
    return 0;
   }

<<EOF>> {
             return 0;
        }

. {
    delete [] CQL_lval.strValue;
    CQL_lval.strValue = 0;
    CQL_DEBUG_TRACE("LEX::TOK_UNEXPECTED_CHAR\n");

    return TOK_UNEXPECTED_CHAR;
}

%%
extern "C" int CQL_wrap()
{
    return 1;
}


