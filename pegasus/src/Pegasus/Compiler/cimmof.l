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

%option never-interactive
%option nounput
%option noyy_top_state
%option stack
%option yylineno
%{


#include "cimmofParser.h"
#include "memobjs.h"
#include "cimmof_tab.h"

/* Avoid warn_unused_result warnings in Linux RPM build */
#ifndef ECHO
# define ECHO do { fwrite(yytext, yyleng, 1, yyout); } while(0)
#endif

/*---------------------------------------------------------------*/
/* These routines wrap certain flex functions, keeping down the  */
/* number of external dependencies                               */
/*---------------------------------------------------------------*/
int get_yy_buf_size_wrapper() { return YY_BUF_SIZE; }

void * get_cimmof__current_buffer_wrapper() {
  return (void *)YY_CURRENT_BUFFER;
}

void * create_cimmof_buffer_wrapper(const FILE *f, int size) {
  return (void *)yy_create_buffer((FILE *)f, size);
}

int switch_to_buffer_wrapper(void *buffstate, Boolean closeCurrent) {
  if (closeCurrent)
  {
     if (yyin != NULL)
     {
        fflush(yyin);
        fclose(yyin);
        yyin = NULL;
     }
     yy_delete_buffer(YY_CURRENT_BUFFER);
  }
  yy_switch_to_buffer((YY_BUFFER_STATE)buffstate);
  return 0;
}

char  strbuf[1024], *strbuf_p, slashx[2];

/* Pointer to name for metaQualifiers communicated between lexer and parser */
char * metaQualifierName;


%}


%x myStr myHex
%%


\n             { cimmofParser::Instance()->increment_lineno(); }
\/\/.*[\r\n]?  { cimmofParser::Instance()->increment_lineno(); }
[ \t\r]        { /* skip white space */ }
class          { return(TOK_CLASS); }
\{             { return(TOK_LEFTCURLYBRACE); }
\}             { return(TOK_RIGHTCURLYBRACE); }
\[             { return(TOK_LEFTSQUAREBRACKET); }
\]             { return(TOK_RIGHTSQUAREBRACKET); }
\(             { return(TOK_LEFTPAREN); }
\)             { return(TOK_RIGHTPAREN); }
\:             { return(TOK_COLON); }
;              { return(TOK_SEMICOLON); }
[+-]?[0-9]*\.[0-9]+([eE][+-]?[0-9]+)? {
                   cimmof_lval.strval = new String(yytext);
                   return(TOK_REAL_VALUE); }
[+-]?0[0-7]+   { cimmof_lval.strval = new String(yytext);
                   return(TOK_OCTAL_VALUE); }
[+-]?0x[0-9A-Fa-f]+ { cimmof_lval.strval = new String(yytext);
                   return(TOK_HEX_VALUE); }
[+-]?[01]+[Bb] { cimmof_lval.strval = new String(yytext);
                   return(TOK_BINARY_VALUE); }
[1-9][0-9]*    { cimmof_lval.strval = new String(yytext);
                   return(TOK_POSITIVE_DECIMAL_VALUE); }
[+-]?[0-9]+    { /* Note: All non-zero unsigned integers are handled above */
                 cimmof_lval.strval = new String(yytext);
                   return(TOK_SIGNED_DECIMAL_VALUE); }
\=             { return(TOK_EQUAL); }
,              { return(TOK_COMMA); }
\'.+\'         { cimmof_lval.strval = new String(yytext);
                   return(TOK_CHAR_VALUE); }

    /*--- bugzilla 544 fix; handle string escapes ---

       an impl. decision: invalid escape sequences
       are simply ignored, and left in the string.

       an impl. decision: yyleng is set to the len
       of resulting string, not the len of the matched
       token(s), since they are different, with escapes.
       (but note; cimmof.y does not use yyleng!)

       an impl. req't: strings are returned to parser
       without enclosing "s.
    */

\"             { BEGIN(myStr); strbuf_p = strbuf;
               /**strbuf_p++ = '"';*/ }
<myStr>\"      { BEGIN(INITIAL);
               /**strbuf_p++ = '"';*/  *strbuf_p = '\0';
                  cimmof_leng = strbuf_p - strbuf;
                  cimmof_lval.strval = new String(strbuf);
                  return TOK_STRING_VALUE; }
<myStr><<EOF>> |
<myStr>\n      { BEGIN(INITIAL);
               /**strbuf_p++ = '"';*/  *strbuf_p = '\0';
                  cimmof_leng = strbuf_p - strbuf;
                  cimmof_lval.strval = new String(strbuf);
                  return TOK_UNEXPECTED_CHAR; }
<myStr>\\b     { /* backspace */        *strbuf_p++ = '\b'; }
<myStr>\\f     { /* form feed */        *strbuf_p++ = '\f'; }
<myStr>\\n     { /* line feed */        *strbuf_p++ = '\n'; }
<myStr>\\r     { /* carriage return */  *strbuf_p++ = '\r'; }
<myStr>\\t     { /* horizontal tab */   *strbuf_p++ = '\t'; }
<myStr>\\'     { /* single quote */     *strbuf_p++ = '\''; }
<myStr>\\\"    { /* double quote */     *strbuf_p++ = '"';  }
<myStr>\\\\    { /* backslash */        *strbuf_p++ = '\\'; }
<myStr>\\[xX]  { yy_push_state(myHex);
                  memcpy(slashx,yytext,2);
               }
<myStr>\\(.|\n)         { *strbuf_p++ = '\\';
                        *strbuf_p++ = yytext[1]; }
<myStr>[^\\\n\"]+       { char *t = yytext;
                            while( *t ) *strbuf_p++ = *t++;
                            *strbuf_p = '\0'; }
<myHex>[0-9A-Fa-f]{1,4} { /* get escaped hex digits */
                            char  hexfmt[4];   unsigned int n;
                            sprintf( hexfmt, "%%%dx", yyleng );
                            sscanf(yytext, hexfmt, &n);
                            if (n>255) {
                                memcpy(strbuf_p,slashx,2);
                                strbuf_p += 2;
                                char *t = yytext;
                                while( *t ) *strbuf_p++ = *t++;
                            }
                            else *strbuf_p++ = (char)n;
                                yy_pop_state();
                        }
<myHex>.                { memcpy(strbuf_p,slashx,2);
                              strbuf_p += 2;
                              *strbuf_p++ = *yytext;
                              yy_pop_state();
                        }
                        /* --- end of bugzilla 544 fix --- */


\/\*                    {   int c1=0, c2=yyinput();
                                for(;;) {
                                     if (c2==EOF) break;
                                     if (c1=='*' && c2=='/') break;
                                     c1=c2;
                                     c2=yyinput();
                                }
                        }
<<EOF>>                              { yyterminate(); }
[Nn][Uu][Ll][Ll]                     { return(TOK_NULL_VALUE); }
[Tt][Rr][Uu][Ee]                     { return(TOK_TRUE); }
[Ff][Aa][Ll][Ss][Ee]                 { return(TOK_FALSE); }
\.                                   { return(TOK_PERIOD); }
\#[Pp][Rr][Aa][Gg][Mm][Aa]           { return(TOK_PRAGMA); }
[Ii][Nn][Ss][Tt][Aa][Nn][Cc][Ee]     { return(TOK_INSTANCE); }
[Oo][Ff]                             { return(TOK_OF); }
[Qq][Uu][Aa][Ll][Ii][Ff][Ii][Ee][Rr] {return(TOK_QUALIFIER); }
[Ss][Cc][Oo][Pp][Ee]                 { return(TOK_SCOPE); }

[Aa][Ss][Ss][Oo][Cc][Ii][Aa][Tt][Ii][Oo][Nn] {
                                       metaQualifierName = strdup(yytext);
                                       return(TOK_ASSOCIATION); }
[Ii][Nn][Dd][Ii][Cc][Aa][Tt][Ii][Oo][Nn] {
                                       metaQualifierName = strdup(yytext);
                                       return(TOK_INDICATION); }

[Pp][Rr][Oo][Pp][Ee][Rr][Tt][Yy]     { return(TOK_PROPERTY); }
[Rr][Ee][Ff][Ee][Rr][Ee][Nn][Cc][Ee] { return(TOK_REFERENCE); }
[Mm][Ee][Tt][Hh][Oo][Dd]             { return(TOK_METHOD); }
[Pp][Aa][Rr][Aa][Mm][Ee][Tt][Ee][Rr] {return(TOK_PARAMETER); }
[Aa][Nn][Yy]                         { return(TOK_ANY); }

[Rr][Ee][Ff]                         { return(TOK_REF); }
[Ff][Ll][Aa][Vv][Oo][Rr]             { return(TOK_FLAVOR); }
[Ee][Nn][Aa][Bb][Ll][Ee][Oo][Vv][Ee][Rr][Rr][Ii][Dd][Ee] {
                                         return(TOK_ENABLEOVERRIDE); }
[Dd][Ii][Ss][Aa][Bb][Ll][Ee][Oo][Vv][Ee][Rr][Rr][Ii][Dd][Ee] {
                                         return(TOK_DISABLEOVERRIDE); }
[Rr][Ee][Ss][Tt][Rr][Ii][Cc][Tt][Ee][Dd] { return(TOK_RESTRICTED); }
[Tt][Oo][Ss][Uu][Bb][Cc][Ll][Aa][Ss][Ss] { return(TOK_TOSUBCLASS); }
[Tt][Rr][Aa][Nn][Ss][Ll][Aa][Tt][Aa][Bb][Ll][Ee] { return(TOK_TRANSLATABLE); }
[Ii][Nn][Cc][Ll][Uu][Dd][Ee]         { return(TOK_INCLUDE); }
$[_$0-9a-zA-Z]+                      { cimmof_lval.strval = new String(yytext);
                                       return TOK_ALIAS_IDENTIFIER; }
[Aa][Ss]                             { return TOK_AS; }
[Ss][Tt][Rr][Ii][Nn][Gg]             { return(TOK_DT_STR); }
[Bb][Oo][Oo][Ll][Ee][Aa][Nn]         { return(TOK_DT_BOOL); }
[Dd][Aa][Tt][Ee][Tt][Ii][Mm][Ee]     { return(TOK_DT_DATETIME); }
[Uu][Ii][Nn][Tt]8                    { return(TOK_DT_UINT8); }
[Uu][Ii][Nn][Tt]16                   { return(TOK_DT_UINT16); }
[Uu][Ii][Nn][Tt]32                   { return(TOK_DT_UINT32); }
[Uu][Ii][Nn][Tt]64                   { return(TOK_DT_UINT64); }
[Ss][Ii][Nn][Tt]8                    { return(TOK_DT_SINT8); }
[Ss][Ii][Nn][Tt]16                   { return(TOK_DT_SINT16); }
[Ss][Ii][Nn][Tt]32                   { return(TOK_DT_SINT32); }
[Ss][Ii][Nn][Tt]64                   { return(TOK_DT_SINT64); }
[Rr][Ee][Aa][Ll]32                   { return(TOK_DT_REAL32); }
[Rr][Ee][Aa][Ll]64                   { return(TOK_DT_REAL64); }
[Cc][Hh][Aa][Rr]8                    { return(TOK_DT_CHAR8); }
[Cc][Hh][Aa][Rr]16                   { return(TOK_DT_CHAR16); }
[_a-zA-Z][_$0-9a-zA-Z]*              { cimmof_lval.strval = new String(yytext);
                                         return(TOK_SIMPLE_IDENTIFIER); }
.                                    { cimmof_lval.strval = new String(yytext);
                                         return(TOK_UNEXPECTED_CHAR); }

%%

void cimmof_yy_less(int n) { yyless(n); }
