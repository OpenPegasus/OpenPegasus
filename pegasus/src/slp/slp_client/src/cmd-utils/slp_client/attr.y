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
//NOCHKSRC

/*****************************************************************************
 *  Description:   encode/decode attribute lists
 *
 *  Originated: March 06, 2000
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
void attrerror(const char *, ...);
int32 attrwrap(void);
int32 attrlex(void);
int32 attrparse(void);
BOOL bt = TRUE, bf = FALSE;
void attr_close_lexer(size_t handle);
size_t attr_init_lexer(const char *s);

lslpAttrList attrHead =
{
	&attrHead, &attrHead, TRUE, 0, 0, 0, 0, 0, {0}
};

lslpAttrList inProcessAttr =
{
	&inProcessAttr, &inProcessAttr, TRUE, 0, 0, 0, 0, 0, {0}
};

lslpAttrList inProcessTag =
{
	&inProcessTag, &inProcessTag, TRUE, 0, 0, 0, 0, 0, {0}
};


%}

/* definitions for ytab.h */
%name-prefix="attr"

%union {
	int32 _i;
	char *_s;
	lslpAttrList *_atl;
}

%token<_i> _TRUE _FALSE _MULTIVAL _INT
%token<_s> _ESCAPED _TAG _STRING _OPAQUE

/* typecast the non-terminals */

/* %type <_i> */
%type <_atl> attr_list attr attr_val_list attr_val

%%

attr_list: attr {
			while (! _LSLP_IS_HEAD(inProcessAttr.next))
			{
				$$ = inProcessAttr.next;
				_LSLP_UNLINK($$);
				_LSLP_INSERT_BEFORE($$, &attrHead);
			}
		/* all we really want to do here is link each attribute */
		/* to the global list head. */
		}
	| attr_list ',' attr {
		/* both of these non-terminals are really lists */
		/* ignore the first non-terminal */
			while (! _LSLP_IS_HEAD(inProcessAttr.next))
			{
				$$ = inProcessAttr.next;
				_LSLP_UNLINK($$);
				_LSLP_INSERT_BEFORE($$, &attrHead);
			}
		}
	;

attr: _TAG 	{
			$$ =  lslpAllocAttr($1, tag, NULL, 0);
			if (NULL != $$)
			{
				_LSLP_INSERT_BEFORE($$, &inProcessAttr);
			}
		}
	| '(' _TAG ')' 	{
			$$ =  lslpAllocAttr($2, tag, NULL, 0);
			if (NULL != $$)
			{
				_LSLP_INSERT_BEFORE($$, &inProcessAttr);
			}
		}
        | '(' _TAG '=' ')' {
  			$$ =  lslpAllocAttr($2, tag, NULL, 0);
			if (NULL != $$)
			{
				_LSLP_INSERT_BEFORE($$, &inProcessAttr);
			}
               }

	| '(' _TAG '=' attr_val_list ')' {
			$$ = inProcessTag.next;
			while (! _LSLP_IS_HEAD($$))
			{
				$$->name = strdup($2);
				_LSLP_UNLINK($$);
				_LSLP_INSERT_BEFORE($$, &inProcessAttr);
				$$ = inProcessTag.next;
			}
		}
	;

attr_val_list: attr_val {

			if(NULL != $1)
			{
				_LSLP_INSERT($1, &inProcessTag);
			}
		}
	| attr_val_list _MULTIVAL attr_val {
			if (NULL != $3)
			{
				_LSLP_INSERT_BEFORE($3, &inProcessTag);
			}
		}
	;
attr_val: _TRUE {
			$$ = lslpAllocAttr(NULL, bool_type,  &bt, sizeof(BOOL));
		}
	|     _FALSE {
			$$ = lslpAllocAttr(NULL, bool_type,  &bf, sizeof(BOOL));
		}
	|     _ESCAPED {
			$$ = lslpAllocAttr(NULL, opaque, $1, (int16)(strlen($1) + 1));
		}
	|	  _STRING {
	                     if(strlen($1) > 5 ) {
				if( *($1) == '\\' && ((*($1 + 1) == 'f') || (*($1 + 1) == 'F')) &&  ((*($1 + 2) == 'f') || (*($1 + 2) == 'F'))) {
				       $$ = lslpAllocAttr(NULL, opaque, $1, (int16)(strlen($1) + 1));
                                     } else {
				       $$ = lslpAllocAttr(NULL, string, $1, (int16)(strlen($1) + 1));
				     }
                                  }
			     else {

			       $$ = lslpAllocAttr(NULL, string, $1, (int16)(strlen($1) + 1));
			     }
               }

	|     _INT {
			$$ = lslpAllocAttr(NULL, integer, &($1), sizeof(int32));
		}
	;

%%

void _lslpInitInternalAttrList(void)
{
	attrHead.next = attrHead.prev = &attrHead;
	attrHead.isHead = TRUE;
	inProcessAttr.next =  inProcessAttr.prev = &inProcessAttr;
	inProcessAttr.isHead = TRUE;
	inProcessTag.next =  inProcessTag.prev = &inProcessTag;
	inProcessTag.isHead = TRUE;
	return;
}

lslpAttrList *_lslpDecodeAttrString(char *s)
{
  size_t lexer = 0;
  lslpAttrList *temp = NULL;
  PEGASUS_ASSERT(s != NULL);
  _lslpInitInternalAttrList();
  if (s != NULL) {
    if(NULL != (temp = lslpAllocAttrList()))  {
      if ((0 != (lexer = attr_init_lexer(s))) &&  attrparse()) {
	lslpFreeAttrList(temp, TRUE);
	while (! _LSLP_IS_HEAD(inProcessTag.next))  {
	  temp = inProcessTag.next;
	    _LSLP_UNLINK(temp);
	    lslpFreeAttr(temp);
	}
	while (! _LSLP_IS_HEAD(inProcessAttr.next))  {
	  temp = inProcessAttr.next;
	  _LSLP_UNLINK(temp);
	  lslpFreeAttr(temp);
	}
	while (! _LSLP_IS_HEAD(attrHead.next))  {
	  temp = attrHead.next;
	  _LSLP_UNLINK(temp);
	  lslpFreeAttr(temp);
	}
	attr_close_lexer(lexer);
	return(NULL);
      }

      if (! _LSLP_IS_EMPTY(&attrHead)) {
	temp->attr_string_len = strlen(s);
	temp->attr_string = (char *)malloc(temp->attr_string_len + 1);
	if(temp->attr_string != NULL) {
	  memcpy(temp->attr_string, s, temp->attr_string_len);
	  temp->attr_string[temp->attr_string_len] = 0x00;
	}
	_LSLP_LINK_HEAD(temp, &attrHead);
      }
      if(lexer != 0)
	attr_close_lexer(lexer);
    }
  }

  return(temp);
}


lslpAttrList *lslpAllocAttr(const char *name, char type, const void *val, int16 len)
{
  lslpAttrList *attr;
  if (NULL != (attr = (lslpAttrList *)calloc(1, sizeof(lslpAttrList))))
    {
      if (name != NULL)
	{
	  if (NULL == (attr->name = strdup(name)))
	    {
	      free(attr);
	      return(NULL);
	    }
	}
      attr->type = type;
      if (type == head)	/* listhead */
	return(attr);
      if (val != NULL)
	{
	  attr->attr_len = len;
	  switch (type)	    {
	    case string:
	      if ( NULL == (attr->val.stringVal = strdup((const char *)val)))
              {
                  lslpFreeAttr(attr);
                  return NULL;
              }
	      break;
	    case integer:
	      attr->val.intVal = *(const uint32 *)val;
	      break;
	    case bool_type:
	      attr->val.boolVal = *(const BOOL *)val;
	      break;
	    case opaque:
	      if ( NULL == (attr->val.opaqueVal = strdup((const char *)val)))
              {
                  lslpFreeAttr(attr);
                  return NULL;
              }
	      break;
	    default:
                  lslpFreeAttr(attr);
                  return NULL;
	    }
	}
    }
  return(attr);
}

lslpAttrList *lslpAllocAttrList(void)
{
  lslpAttrList *temp;
  if (NULL != (temp = lslpAllocAttr(NULL, head, NULL, 0)))
    {
      temp->next = temp->prev = temp;
      temp->isHead = TRUE;
    }
  return(temp);
}

/* attr MUST be unlinked from its list ! */
void lslpFreeAttr(lslpAttrList *attr)
{
  PEGASUS_ASSERT(attr != NULL);
  if (attr->name != NULL)
    free(attr->name);
  if(attr->attr_string != NULL)
    free(attr->attr_string);
  if (attr->type == string && attr->val.stringVal != NULL)
    free(attr->val.stringVal);
  else if (attr->type == opaque && attr->val.opaqueVal != NULL)
    free(attr->val.opaqueVal);
  free(attr);
}

void lslpFreeAttrList(lslpAttrList *list, BOOL staticFlag)
{
  lslpAttrList *temp;

  PEGASUS_ASSERT(list != NULL);
  PEGASUS_ASSERT(_LSLP_IS_HEAD(list));
  while(! (_LSLP_IS_EMPTY(list)))
    {
      temp = list->next;
      _LSLP_UNLINK(temp);
      lslpFreeAttr(temp);
    }
  if(staticFlag == TRUE)
    lslpFreeAttr(list);
  return;

}
