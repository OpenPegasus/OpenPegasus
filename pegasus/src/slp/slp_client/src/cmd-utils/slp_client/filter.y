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
 *  Description: encode/decode/eval LDAP Filters
 *
 *  Originated: April 04, 2002
 *	Original Author: Mike Day md@soft-hackle.net
 *                                mdd@us.ibm.com
 *
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
  void filtererror(const char *, ...);

  int32 filterlex(void);

int32 filterparse(void);
void filter_close_lexer(size_t handle);
size_t filter_init_lexer(const char *s);

/* have a place to put attributes and the filter while the parser is working */
/* on them makes it easier to recover from parser errors - all the memory we  */
/* need to free is available from the list heads below.  */

/* listhead for reduced filters until the parser is finished */
 filterHead reducedFilters = { &reducedFilters, &reducedFilters, TRUE, FALSE};
 int nesting_level;

%}

%name-prefix="filter"
/* definitions for ytab.h */

%union {
  int32 filter_int;
  char *filter_string;
  lslpLDAPFilter *filter_filter;
}


%token<filter_int> L_PAREN R_PAREN OP_AND OP_OR OP_NOT OP_EQU OP_GT OP_LT OP_PRESENT OP_APPROX
%token<filter_int> VAL_INT VAL_BOOL
%token<filter_string> OPERAND

/* typecast the non-terminals */

%type <filter_filter> filter filter_list expression
%type <filter_int> exp_operator filter_op filter_open filter_close
%%

/* grammar */

filter_list: filter
         | filter_list filter
         ;

filter: filter_open filter_op filter_list filter_close {
            if(NULL != ($$ = lslpAllocFilter($2))) {
	      $$->nestingLevel = nesting_level;
	      if(! _LSLP_IS_EMPTY(&reducedFilters) ) {
		lslpLDAPFilter *temp = (lslpLDAPFilter *)reducedFilters.next;
		while(! _LSLP_IS_HEAD(temp)) {
		  if(temp->nestingLevel == nesting_level + 1) {
		    lslpLDAPFilter *nest = temp;
		    temp = temp->next;
		    _LSLP_UNLINK(nest);
		    _LSLP_INSERT_BEFORE(nest, (lslpLDAPFilter *)&($$->children)) ;
		  } else {temp = temp->next; }
		}
		_LSLP_INSERT_BEFORE( (filterHead *)$$, &reducedFilters);
	      } else { lslpFreeFilter($$) ; $$ = NULL ; }
            }
         }

         | filter_open expression filter_close {
	   $$ = $2;
	   if($2 != NULL) {
	     $2->nestingLevel = nesting_level;
	     _LSLP_INSERT_BEFORE((filterHead *)$2, &reducedFilters) ;
	   }
	 }
         ;

filter_open: L_PAREN { nesting_level++; }
         ;

filter_close: R_PAREN { nesting_level--; }
         ;

filter_op: OP_AND
         | OP_OR
         | OP_NOT
         { $$ = filterlval.filter_int; }

         ;

expression: OPERAND OP_PRESENT {      /* presence test binds to single operand */
             if(NULL != ($$ = lslpAllocFilter(expr_present))) {
	       lslpAttrList *attr = lslpAllocAttr($1, string, "*", (int16)strlen("*") + 1);
	       if(attr != NULL) {
		 _LSLP_INSERT(attr, &($$->attrs));
	       } else { lslpFreeFilter($$); $$ = NULL; }
	     }
         }

         | OPERAND exp_operator VAL_INT  {  /* must be an int or a bool */
	   /* remember to touch up the token values to match the enum in lslp.h */
	   if(NULL != ($$ = lslpAllocFilter($2))) {
	     lslpAttrList *attr = lslpAllocAttr($1, integer, &($3), sizeof($3));
	     if(attr != NULL) {
	       _LSLP_INSERT(attr, &($$->attrs));
	     } else { lslpFreeFilter($$); $$ = NULL ; }
	   }
	 }

         | OPERAND exp_operator VAL_BOOL  {  /* must be an int or a bool */
	   /* remember to touch up the token values to match the enum in lslp.h */
	   if(NULL != ($$ = lslpAllocFilter($2))) {
	     lslpAttrList *attr = lslpAllocAttr($1, bool_type, &($3), sizeof($3));
	     if(attr != NULL) {
	       _LSLP_INSERT(attr, &($$->attrs));
	     } else { lslpFreeFilter($$); $$ = NULL ; }
	   }
	 }

         | OPERAND exp_operator OPERAND  {   /* both operands are strings */
	   if(NULL != ($$ = lslpAllocFilter($2))) {
	     lslpAttrList *attr = lslpAllocAttr($1, string, $3, (int16)strlen($3) + 1 );
	     if(attr != NULL) {
	       _LSLP_INSERT(attr, &($$->attrs));
	     } else { lslpFreeFilter($$); $$ = NULL ; }
	   }
	 }

         ;

exp_operator: OP_EQU
         | OP_GT
         | OP_LT
         | OP_APPROX
         { $$ = filterlval.filter_int; }
         ;

%%


lslpLDAPFilter *lslpAllocFilter(int filterOperator)
{
  lslpLDAPFilter *filter = (lslpLDAPFilter *)calloc(1, sizeof(lslpLDAPFilter));
  if(filter  != NULL) {
    filter->next = filter->prev = filter;
    if(filterOperator == head) {
      filter->isHead = TRUE;
    } else {
      filter->children.next = filter->children.prev = &(filter->children);
      filter->children.isHead = 1;
      filter->attrs.next = filter->attrs.prev = &(filter->attrs);
      filter->attrs.isHead = 1;
      filter->_operator = filterOperator;
    }
  }
  return(filter);
}


void lslpFreeFilter(lslpLDAPFilter *filter)
{
  if(filter->children.next != NULL) {
    while(! (_LSLP_IS_EMPTY((lslpLDAPFilter *)&(filter->children)))) {
      lslpLDAPFilter *child = (lslpLDAPFilter *)filter->children.next;
      _LSLP_UNLINK(child);
      lslpFreeFilter(child);
    }
  }
  if(filter->attrs.next != NULL) {
    while(! (_LSLP_IS_EMPTY(&(filter->attrs)))) {
      lslpAttrList *attrs = filter->attrs.next;
      _LSLP_UNLINK(attrs);
      lslpFreeAttr(attrs);
    }
  }
}

void lslpFreeFilterList(lslpLDAPFilter *head, BOOL static_flag)
{
  PEGASUS_ASSERT((head != NULL) && (_LSLP_IS_HEAD(head)));
  while(! (_LSLP_IS_EMPTY(head))) {
    lslpLDAPFilter *temp = head->next;
    _LSLP_UNLINK(temp);
    lslpFreeFilter(temp);
  }

  if( static_flag == TRUE)
    lslpFreeFilter(head);
  return;
}

void lslpInitFilterList(void )
{
  reducedFilters.next = reducedFilters.prev = &reducedFilters;
  reducedFilters.isHead = TRUE;
  return;
}

void lslpCleanUpFilterList(void)
{
  lslpFreeFilterList( (lslpLDAPFilter *)&reducedFilters, FALSE);
}

lslpLDAPFilter *_lslpDecodeLDAPFilter(char *filter)
{

  lslpLDAPFilter *temp = NULL;
  size_t lexer = 0;
  PEGASUS_ASSERT(filter != NULL && strlen(filter));
  lslpInitFilterList();
  nesting_level = 1;
  if(0 != (lexer = filter_init_lexer(filter))) {
    if(filterparse()) { lslpCleanUpFilterList(); }
    filter_close_lexer(lexer);
  }
  if (! _LSLP_IS_EMPTY(&reducedFilters)) {
    if(NULL != (temp  = lslpAllocFilter(ldap_and))) {
      _LSLP_LINK_HEAD(&(temp->children), &reducedFilters);
    }
  }
  lslpCleanUpFilterList();

  return(temp);
}
