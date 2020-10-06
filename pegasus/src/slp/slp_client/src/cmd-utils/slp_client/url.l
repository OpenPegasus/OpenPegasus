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
/* NOCHKSRC */
/*****************************************************************************
 *  Description: encode/decode attribute urls
 *
 *  Originated: February 25, 2000
 *	Original Author: Mike Day md@soft-hackle.net
 *                                mdd@us.ibm.com
 *
 *  Copyright (c) 2001 - 2003  IBM
 *  Copyright (c) 2000 - 2003 Michael Day
 *
 *  Permission is hereby granted, free of charge, to any person obtaining a
 *  copy of this software and associated documentation files (the "Software"),
 *  to deal in the Software without restriction, including without limitation
 *  the rights to use, copy, modify, merge, publish, distribute, sublicense,
 *  and/or sell copies of the Software, and to permit persons to whom the
 *  Software is furnished to do so, subject to the following conditions:
 *
 *  The above copyright notice and this permission notice shall be included in
 *  all copies or substantial portions of the Software.
 *
 *
 *  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 *  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 *  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
 *  THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 *  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 *  FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 *  DEALINGS IN THE SOFTWARE.
 *
 *****************************************************************************/


%{
#undef YYLMAX
#define YYLMAX 2048
#define YY_NEVER_INTERACTIVE 1
#undef YY_INPUT
#define YY_INPUT(b, r, m) abort()
#define exit(i) abort()
#include "slp_client.h"
#include "y_url.h"
#include <stdarg.h>

#define urlless yyless
void urlerror(const char *, ...);

static int16 heapIndex;
static char *heap;
static char *buf;
/* special counter to exit the appletalk state */
char atalk_state = 0;

static char *_lslp_strdup(const char *s);
void url_close_lexer(size_t handle);
size_t url_init_lexer(const char *s);

%}

 /* {}	*/
digit			[0-9]
upalpha			[A-Z]
lowalpha		[a-z]
alpha			[a-zA-Z]
alphanum 		[a-zA-Z0-9]
hex			[a-fA-F0-9]
safe			[-$_.~]
escaped			("%"{hex}{2} | "\\"{hex}{2})
mark 			[-_.!~*'()]
unreserved		({alphanum}|{mark})
reserved		[;/?:@&=+$,]
uric			({reserved} | {unreserved} | {escaped})

 /* IPv6 definitions */
hex4			({hex}{1,4})
hexseq			{hex4}|{hex4}(":"{hex4})*
hexpart			{hexseq}|{hexseq}"::"({hexseq})*|"::"({hexseq})*
ip6prefix		{hexpart}"/"{1}{digit}{2}
ip4addr			({digit}{1,3})"."({digit}{1,3})"."({digit}{1,3})"."({digit}{1,3})
ip6addr			{hexpart}|{hexpart}":"{ip4addr}

%x IP_SITE
%x AT_SITE
%x ATTRIBUTE
%p 	 9000
%n    300
%e    700
%option noyywrap prefix="url"
%option nounput
%option noinput
%%

 /* the ip user @ host syntax is a special state because some reserved */
 /* characters are valid - but user the string itself is optional so we can't */
 /* build it into a full-time rule */

"//"			{
                          BEGIN IP_SITE;
			  urlless(0);
			}
 /* return the slash as a token */
<IP_SITE>"/"		{urllval._i = *urltext; return(urllval._i);}

 /* this next rule needs to kick in even if it matches a zero-length string */
 /* i.e., it needs to be guaranteed even if there is no user @ host production */
<IP_SITE>[-_.!~*'()a-zA-Z0-9;+&=\%]*|"["{ip6addr}"]" {
			BEGIN INITIAL;
			if (urlleng > 0)
			{
                            if(NULL != (urllval._s = _lslp_strdup(urltext)))
                                return(_RESNAME);
			    else
				return(0L);
			}
		}

 /* appletalk syntax includes three reserved characters - give the lexer a chance */
 /* to match an appletalk production before it starts returning reserved */
 /* chars as tokens. */

"/"[aA]{1}[tT]{1}"/" 	{
			BEGIN AT_SITE;
 			if(NULL != (urllval._s	= _lslp_strdup(urltext)))
				return(_AT);
			else
				return(0L);
			}

<AT_SITE>(([-a-zA-Z0-9$_.~])|("%"([a-fA-F0-9]{2,2}))){1,31} {
 			atalk_state++;
			if (atalk_state == 3)
				{BEGIN INITIAL;}
 			if(NULL != (urllval._s	= _lslp_strdup(urltext)))
				return(_ZONE);
			else
				return(0L);
			}

<AT_SITE>":"		{urllval._i = *urltext; return(urllval._i);}
<AT_SITE>"/"		{BEGIN INITIAL; urlless(0);}

 /* ipx syntax includes two reserved characters - give the lexer a chance */
 /* to match an ipx production before it starts returning reserved chars */
 /* as tokens */

"/"[iI]{1}[pP]{1}[xX]{1}"/"({hex}{8})":"({hex}{12})":"({hex}{4}) {
 			if(NULL != (urllval._s	= _lslp_strdup(urltext)))
				return(_IPX);
			else
				return(0L);
			}

 /* reserved characters  - always a token in normal state unless part of */
 /* one of the productions above */
";"			{BEGIN ATTRIBUTE; urllval._i = *urltext; return(urllval._i);}
"/"			{urllval._i = *urltext; return(urllval._i);}
"?"			{urllval._i = *urltext; return(urllval._i);}
":"			{urllval._i = *urltext; return(urllval._i);}
"@"			{urllval._i = *urltext; return(urllval._i);}
"&"			{urllval._i = *urltext; return(urllval._i);}
"="			{urllval._i = *urltext; return(urllval._i);}
"+"			{urllval._i = *urltext; return(urllval._i);}
"$"			{urllval._i = *urltext; return(urllval._i);}
","			{urllval._i = *urltext; return(urllval._i);}

 /* a string consisting of only hex digits */
{hex}+			{
 			if(NULL != (urllval._s	= _lslp_strdup(urltext)))
				return(_HEXDIG);
			else
				return(0L);
			}

 /* an ipv4 address */
({hex}{1,3})"."({hex}{1,3})"."({hex}{1,3})"."({hex}{1,3})|"["{ip6addr}"]" {
 			if(NULL != (urllval._s	= _lslp_strdup(urltext)))
				return(_IPADDR);
			else
				return(0L);
			}

 /* resource names start with alpha and include alphanum and '+' or '-' */
 /* but '+' is reserved and must be escaped */
[-a-zA-Z0-9.]* 	{
 			if(NULL != (urllval._s	= _lslp_strdup(urltext)))
				return(_RESNAME);
			else
				return(0L);
			}


 /* anything else that is not reserved */
<ATTRIBUTE>[!-~]+ {
 			BEGIN INITIAL;
 			if(NULL != (urllval._s	= _lslp_strdup(urltext)))
				return(_ELEMENT);
			else
				return(0L);
			}

 /* anything else that is not reserved */
[^;/?:@&=+$,]+		{
 			if(NULL != (urllval._s	= _lslp_strdup(urltext)))
				return(_ELEMENT);
			else
				return(0L);
			}

 /* anything else is an error */

%%

static char *_lslp_strdup(const char *s)
{
  char *p = &heap[heapIndex];

  while ((heap[heapIndex++] = *s++))
      ;
  return p;
}

void url_close_lexer(size_t handle)
{
  PEGASUS_ASSERT(handle);
  PEGASUS_ASSERT(buf);
  PEGASUS_ASSERT(heap);
  url_delete_buffer((YY_BUFFER_STATE)handle);
  free(buf);
  free(heap);
  buf = heap = 0;
}

size_t url_init_lexer(const char *s)
{
  size_t len = strlen(s) + 2;

  if (!(heap = (char*)malloc(len)))
  {
      return 0;
  }
  if (!(buf = (char*)malloc(len)))
  {
      free (heap);
      return 0;
  }
  heapIndex = 0;
  strcpy(buf, s);
  buf[len - 1] = 0;

  return ((size_t) url_scan_buffer(buf, len));
}

void urlerror(const char *s, ...)
{
 return;
}	

