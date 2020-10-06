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
/* NOCHKSRC */
/*****************************************************************************
 *  Description: encode/decode attribute urls
 *
 *  Originated: March 6, 2000
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

#include "slp_client.h"



/* prototypes and globals go here */
void urlerror(const char *, ...);
int32 urlwrap(void);
int32 urllex(void);
int32 urlparse(void);
void url_close_lexer(size_t handle);
size_t url_init_lexer(const char *s);


static lslpAtomList srvcHead = {&srvcHead, &srvcHead, TRUE, NULL, 0};
static lslpAtomList siteHead = {&siteHead, &siteHead, TRUE, NULL, 0};
static lslpAtomList pathHead = {&pathHead, &pathHead, TRUE, NULL, 0};
static lslpAtomList attrHead = {&attrHead, &attrHead, TRUE, NULL, 0};

lslpAtomizedURL urlHead =
{
    &urlHead,
    &urlHead,
    TRUE,
    NULL,
    0,
    {&srvcHead, &srvcHead, TRUE, NULL, 0},
    {&siteHead, &siteHead, TRUE, NULL, 0},
    {&pathHead, &pathHead, TRUE, NULL, 0},
    {&attrHead, &attrHead, TRUE, NULL, 0}
};

%}

/* definitions for ytab.h */
%name-prefix="url"

%union {
	int32 _i;
	char *_s;
	lslpAtomList *_atl;
	lslpAtomizedURL *_aturl;
}

%token<_i> _RESERVED
%token<_s> _HEXDIG _STAG _RESNAME _NAME _ELEMENT _IPADDR  _IPX
%token<_s> _AT _ZONE

/* typecast the non-terminals */

/* %type <_i> */
%type <_s> ip_site ipx_site at_site hostport host service_id
%type <_aturl> url
%type <_atl> service_list service site path_list path_el attr_list attr_el
%type <_atl> url_part sap

%%
url: service_list sap {
			if (NULL != ($$ = (lslpAtomizedURL *)calloc(1, sizeof(lslpAtomizedURL))))
			{
				int32 urlLen = 1;
				lslpAtomList *temp = srvcHead.next;
				while (! _LSLP_IS_HEAD(temp) && temp->str != NULL)
				{
					urlLen += strlen(temp->str);
					temp = temp->next;
				}
				temp = siteHead.next;
				while (! _LSLP_IS_HEAD(temp) && temp->str != NULL)
				{
					urlLen += strlen(temp->str);
					temp = temp->next;
				}
				temp = pathHead.next;
				while (! _LSLP_IS_HEAD(temp) && temp->str != NULL)
				{
					urlLen += strlen(temp->str) + 1;
					temp = temp->next;
				}
				temp = attrHead.next;
				while (! _LSLP_IS_HEAD(temp) && temp->str != NULL)
				{
					urlLen += strlen(temp->str) + 1;
					temp = temp->next;
				}
				if (NULL != ($$->url = (char *)calloc(urlLen, sizeof(char))))
				{
					temp = srvcHead.next;
					if (! _LSLP_IS_HEAD(temp) && temp->str != NULL)
					{
						strcpy($$->url, temp->str);
						temp = temp->next;
					}
					while (! _LSLP_IS_HEAD(temp) && temp->str != NULL)
					{
						strcat($$->url, temp->str);
						temp = temp->next;
					}
					temp = siteHead.next;
					while (! _LSLP_IS_HEAD(temp) && temp->str != NULL)
					{
						strcat($$->url, temp->str);
						temp = temp->next;
					}
					temp = pathHead.next;
					while (! _LSLP_IS_HEAD(temp) && temp->str != NULL)
					{
						strcat($$->url, "/");
						strcat($$->url, temp->str);
						temp = temp->next;
					}
					temp = attrHead.next;
					while (! _LSLP_IS_HEAD(temp) && temp->str != NULL)
					{
						strcat($$->url, ";");
						strcat($$->url, temp->str);
						temp = temp->next;
					}
					$$->urlHash = lslpCheckSum($$->url, (int16)strlen($$->url));
				}

				/* make certain the listheads are initialized */
				$$->srvcs.next = $$->srvcs.prev = &($$->srvcs);
				$$->srvcs.isHead = TRUE;
				$$->site.next = $$->site.prev = &($$->site);
				$$->site.isHead = TRUE;
				$$->path.next = $$->path.prev = &($$->path);
				$$->path.isHead = TRUE;
				$$->attrs.next = $$->attrs.prev = &($$->attrs);
				$$->attrs.isHead = TRUE;
				if (! _LSLP_IS_EMPTY(&srvcHead ))
				{
					_LSLP_LINK_HEAD(&($$->srvcs), &srvcHead);
				}
				if (! _LSLP_IS_EMPTY(&siteHead))
				{
					_LSLP_LINK_HEAD(&($$->site), &siteHead);
				}
				if (! _LSLP_IS_EMPTY(&pathHead))
				{
					_LSLP_LINK_HEAD(&($$->path), &pathHead);
				}
				if (! _LSLP_IS_EMPTY(&attrHead))
				{
					_LSLP_LINK_HEAD(&($$->attrs), &attrHead);
				}
				_LSLP_INSERT_BEFORE($$, &urlHead);
				lslpInitAtomLists();
			}
		}
	;
service_list: service	{
			$$ = &srvcHead;
			if ($1 != NULL)
			{
				_LSLP_INSERT_BEFORE($1, $$);
			}
		}

	| service_list service {
			$$ = &srvcHead;
			if ($2 != NULL)
			{
				_LSLP_INSERT_BEFORE($2, $$);
			}
		}
	;

service:  _RESNAME ':'	{
			if (NULL != ($$ = (lslpAtomList *)calloc(1, sizeof(lslpAtomList))))
			{
				$$->next = $$->prev = $$;
				if (NULL != ($$->str = (char *)calloc(2 + strlen($1), sizeof(char))))
				{
					strcpy($$->str, $1);
					strcat($$->str, ":");	
					$$->hash = lslpCheckSum($$->str, (int16)strlen($$->str));
				}
				else
				{
					free($$);
					$$ = NULL;
				}
			}

		}
	|	  _RESNAME '.' _RESNAME ':'	{
			if (NULL != ($$ = (lslpAtomList *)calloc(1, sizeof(lslpAtomList))))
			{
				$$->next = $$->prev = $$;
				if (NULL != ($$->str = (char *)calloc(3 + strlen($1) + strlen($3), sizeof(char))))
				{
					strcpy($$->str, $1);
					strcat($$->str, ".");
					strcat($$->str, $3);
					strcat($$->str, ":");	
					$$->hash = lslpCheckSum($$->str, (int16)strlen($$->str));
				}
				else
				{
					free($$);
					$$ = NULL;
				}
			}

		}
	;

sap:  site {
			$$ = &siteHead;
			if ($1 != NULL)
			{
				_LSLP_INSERT_BEFORE($1, $$);
			}
		}
	| 	site url_part 	{
			$$ = &siteHead;
			if ($1 != NULL)
			{
					_LSLP_INSERT_BEFORE($1, $$);
			}
		}
	;

site:  ip_site {
			if ($1 != NULL)
			{
				if(NULL != ($$ = (lslpAtomList *)calloc(1, sizeof(lslpAtomList))))
				{
					$$->next = $$->prev = $$;
					$$->str = $1;
					$$->hash = lslpCheckSum($$->str, (int16)strlen($$->str));
				}
			}
			else
				$$ = NULL;
		}
	|	ipx_site {
			if ($1 != NULL)
			{
				if(NULL != ($$ = (lslpAtomList *)calloc(1, sizeof(lslpAtomList))))
				{
					$$->next = $$->prev = $$;
					$$->str = $1;
					$$->hash = lslpCheckSum($$->str, (int16)strlen($$->str));
				}
			}
			else
				$$ = NULL;
		}
	|	at_site {
			if ($1 != NULL)
			{
				if(NULL != ($$ = (lslpAtomList *)calloc(1, sizeof(lslpAtomList))))
				{
					$$->next = $$->prev = $$;
					$$->str = $1;
					$$->hash = lslpCheckSum($$->str, (int16)strlen($$->str));
				}
			}
			else
				$$ = NULL;
		}

|       service_id {

			if ($1 != NULL)
			{
				if(NULL != ($$ = (lslpAtomList *)calloc(1, sizeof(lslpAtomList))))
				{
					$$->next = $$->prev = $$;
					$$->str = $1;
					$$->hash = lslpCheckSum($$->str, (int16)strlen($$->str));
				}
			}
			else
				$$ = NULL;

}

	;

ip_site: '/''/' {
			$$ = strdup("//");
		}
	|   '/''/' hostport {
			if(NULL != $3 && (NULL !=($$ = (char *)calloc(3 + strlen($3), sizeof(char)))))
			{
				strcpy($$, "//");
				strcat($$, $3);
				free($3);
			}

		}
	|   '/''/' _RESNAME '@' hostport {
			if(NULL != $5 && (NULL !=($$ = (char *)calloc(4 + strlen($3) + strlen($5), sizeof(char)))))
			{
				strcpy($$, "//");
				strcat($$, $3);
				strcat($$, "@");
				strcat($$, $5);
			}
		}
	;

service_id: _RESNAME {
	  $$ = strdup($1);
	}
        ;

ipx_site: _IPX {
			$$ = $1;
		}
	;

at_site: _AT _ZONE ':' _ZONE ':' _ZONE {
			if(NULL != ($$ = (char *)calloc(strlen($1) + strlen($2) + strlen($4) + strlen($6) + 3, sizeof(char))))
			{
				strcpy($$, $1);
				strcat($$, $2);
				strcat($$, ":");
				strcat($$, $4);
				strcat($$, ":");
				strcat($$, $6);
			}
		}
	;

hostport: host {
                        if (NULL != ($$ = (char *)calloc(strlen($1) + 2, sizeof(char))))
                        {
                            strcpy($$, $1);
                        }
		}
	|	host ':' _HEXDIG {
			if ($1 != NULL)
			{
				if(NULL != ($$ = (char *)calloc(strlen($1) + strlen($3) + 2, sizeof(char))))
				{
					strcpy($$, $1);
					strcat($$, ":");
					strcat($$, $3);
				}
			}
			else
				$$ = NULL;	
		}
	;

host: _RESNAME {
			$$ = $1;
		}
	|  _IPADDR {
			$$ = $1;
		}
	;

url_part: path_list {
			;	
		}
	|	attr_list {
			;		
		}
	|	path_list attr_list {
			;
		}
	;

path_list:	path_el {
			$$ = &pathHead;
			if ($1 != NULL)
			{
				_LSLP_INSERT_BEFORE($1, $$);
			}
		}
	| path_list path_el {
			$$ = &pathHead;
			if ($2 != NULL)
			{
				_LSLP_INSERT_BEFORE($2, $$);
			}
		}

	;

path_el: '/' {
			 /* dangling path slash - do nothing */
			 $$ = NULL;
			 }
	|    '/' _ELEMENT  {
			if(NULL != ($$ = (lslpAtomList *)calloc(1, sizeof(lslpAtomList))))
			{
				$$->prev = $$->next = $$;
				if(NULL != ($$->str = (char *)calloc(1 + strlen($2), sizeof(char))))
				{
					strcpy($$->str, $2);
					$$->hash = lslpCheckSum($$->str, (int16)strlen($$->str));

				}
				else
				{
					free($$);
					$$ = NULL;
				}
			}

		}
	|	 '/' _RESNAME  {
			if(NULL != ($$ = (lslpAtomList *)calloc(1, sizeof(lslpAtomList))))
			{
				$$->prev = $$->next = $$;
				if(NULL != ($$->str = (char *)calloc(1 + strlen($2), sizeof(char))))
				{
					strcpy($$->str, $2);
					$$->hash = lslpCheckSum($$->str, (int16)strlen($$->str));
				}
				else
				{
					free($$);
					$$ = NULL;
				}
			}

		}
	| 	'/' _HEXDIG {
			if(NULL != ($$ = (lslpAtomList *)calloc(1, sizeof(lslpAtomList))))
			{
				$$->prev = $$->next = $$;
				if(NULL != ($$->str = (char *)calloc(1 + strlen($2), sizeof(char))))
				{
					strcpy($$->str, $2);
					$$->hash = lslpCheckSum($$->str, (int16)strlen($$->str));
				}
				else
				{
					free($$);
					$$ = NULL;
				}
			}
		}
	;

attr_list: attr_el {
			$$ = &attrHead;
			if ($1 != NULL)
			{
				_LSLP_INSERT_BEFORE($1, $$);
			}
		}
	| attr_list attr_el {
			$$ = &attrHead;
			if ($2 != NULL)
			{
				_LSLP_INSERT_BEFORE($2, $$);
			}
		}
	;

attr_el: ';' _ELEMENT  {
			if(NULL != ($$ = (lslpAtomList *)calloc(1, sizeof(lslpAtomList))))
			{
				$$->prev = $$->next = $$;
				if(NULL != ($$->str = (char *)calloc(1 + strlen($2), sizeof(char))))
				{
					strcpy($$->str, $2);
					$$->hash = lslpCheckSum($$->str, (int16)strlen($$->str));
				}
				else
				{
					free($$);
					$$ = NULL;
				}
			}
		}

	|  ';' _ELEMENT '=' _ELEMENT {
			if(NULL != ($$ = (lslpAtomList *)calloc(1, sizeof(lslpAtomList))))
			{
				$$->prev = $$->next = $$;
				if(NULL != ($$->str = (char *)calloc(2 + strlen($2) + strlen($4), sizeof(char))))
				{
					strcpy($$->str, $2);
					strcat($$->str, "=");
					strcat($$->str, $4);
					$$->hash = lslpCheckSum($$->str, (int16)strlen($$->str));
				}
				else
				{
					free($$);
					$$ = NULL;
				}
			}
		}
	;

%%


lslpAtomList *lslpAllocAtom(void)
{
	lslpAtomList *temp = (lslpAtomList *)calloc(1, sizeof(lslpAtomList));
	if (temp != NULL)
	{
		temp->next = temp->prev = temp;
	}
	return(temp);
}	

lslpAtomList *lslpAllocAtomList(void)
{
	lslpAtomList *temp =lslpAllocAtom();
	if (temp != NULL)
		temp->isHead = TRUE;
	return(temp);
}	

void lslpFreeAtom(lslpAtomList *a )
{
	PEGASUS_ASSERT(a != NULL);
	if (a->str != NULL)
		free(a->str);
	free(a);
	return;
}	

void lslpFreeAtomList(lslpAtomList *l, int32 flag)
{
	lslpAtomList *temp;
	PEGASUS_ASSERT(l != NULL);
	PEGASUS_ASSERT(_LSLP_IS_HEAD(l));
	while (! _LSLP_IS_EMPTY(l))
	{
		temp = l->next;
		temp->prev->next = temp->next;
		temp->next->prev = temp->prev;
		lslpFreeAtom(temp);
	}
	if (flag)
		lslpFreeAtom(l);
	return;
}	

lslpAtomizedURL *lslpAllocAtomizedURL(void)
{
	lslpAtomizedURL *temp = (lslpAtomizedURL *)calloc(1, sizeof(lslpAtomizedURL));
	if (temp != NULL)
	{
		temp->prev = temp->next = temp;
	}
	return(temp);
}	

lslpAtomizedURL *lslpAllocAtomizedURLList(void)
{
	lslpAtomizedURL *temp = lslpAllocAtomizedURL();
	if (temp != NULL)
	{
		temp->isHead = TRUE;
	}
	return(temp);
}	

void lslpFreeAtomizedURL(lslpAtomizedURL *u)
{
	PEGASUS_ASSERT(u != NULL);
	if (u->url != NULL)
		free(u->url);
	if (! _LSLP_IS_EMPTY(&(u->srvcs)))
		lslpFreeAtomList(&(u->srvcs), 0);
	if (! _LSLP_IS_EMPTY(&(u->site)))
		lslpFreeAtomList(&(u->site), 0);
	if (! _LSLP_IS_EMPTY(&(u->path)))
		lslpFreeAtomList(&(u->path), 0);
	if (!  _LSLP_IS_EMPTY(&(u->attrs)))
		lslpFreeAtomList(&(u->attrs), 0);
	free(u);
	return;
}	

void lslpFreeAtomizedURLList(lslpAtomizedURL *l, int32 flag)
{
	lslpAtomizedURL *temp;
	PEGASUS_ASSERT(l != NULL);
	PEGASUS_ASSERT(_LSLP_IS_HEAD(l));
	while (! (_LSLP_IS_HEAD(l->next)))
	{
		temp = l->next;
		temp->prev->next = temp->next;
		temp->next->prev = temp->prev;
		lslpFreeAtomizedURL(temp);
	}
	if (flag)
		free(l);
	return;
}	

void lslpInitAtomLists(void)
{
	srvcHead.next = srvcHead.prev = &srvcHead;
	siteHead.next = siteHead.prev = &siteHead;
	pathHead.next = pathHead.prev = &pathHead;
	attrHead.next = attrHead.prev = &attrHead;
	return;
}	

void lslpInitURLList(void)
{
	urlHead.next = urlHead.prev = &urlHead;
	return;
}	



void lslpCleanUpURLLists(void)
{

  lslpFreeAtomList(&srvcHead, 0);
  lslpFreeAtomList(&siteHead, 0);
  lslpFreeAtomList(&pathHead, 0);
  lslpFreeAtomList(&attrHead, 0);
  lslpFreeAtomizedURLList(&urlHead, 0);
}

lslpAtomizedURL *_lslpDecodeURLs(char *u[], int32 count)
{
  int32 i;

  size_t lexer = 0;
  lslpAtomizedURL *temp = NULL;
  PEGASUS_ASSERT(u != NULL && u[count - 1] != NULL);
  lslpInitURLList();
  lslpInitAtomLists();

  for (i = 0; i < count; i++) {
    if (NULL == u[i])
      break;
    if((0 != (lexer = url_init_lexer(u[i])))) {
      if(urlparse())
	lslpCleanUpURLLists();
      url_close_lexer(lexer);
    }
  }
  if (! _LSLP_IS_EMPTY(&urlHead)) {
    if(NULL != (temp = lslpAllocAtomizedURLList())) {
      _LSLP_LINK_HEAD(temp, &urlHead);
    }
  }

  return(temp);	
}	

