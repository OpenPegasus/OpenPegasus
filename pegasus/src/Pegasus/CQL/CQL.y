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
%{
#include <Pegasus/Common/Config.h>
#include <Pegasus/Common/String.h>
#include <Pegasus/Common/CommonUTF.h>
#include <Pegasus/Query/QueryCommon/QueryException.h>
#include <Pegasus/Common/MessageLoader.h>
#include <Pegasus/CQL/CQLFactory.h>
#include "CQLObjects.h"
#include <stdio.h>

#define yyparse CQL_parse
#define CQLPREDICATE 0
#define CQLVALUE 1
#define CQLIDENTIFIER 2
#define CQLFUNCTION 3
#define CQLCHAINEDIDENTIFIER 4

int yylex();
#ifdef CQL_DEBUG_GRAMMAR
#define CQL_DEBUG_TRACE(X) \
    printf(X);
#define CQL_DEBUG_TRACE2(X,Y) \
    printf(X,Y);
#else
#define CQL_DEBUG_TRACE(X)
#define CQL_DEBUG_TRACE2(X,Y)
#endif


extern char * yytext;
int chain_state;
CQLFactory _factory = CQLFactory();
extern int CQL_error(const char *err);

enum CQLType { Id, CId, Val, Func, Fact, Trm, Expr, SPred, Pred, Str };

typedef struct CQLObjPtr {
        void* _ptr;
          CQLType type;
} CQLOBJPTR;

Array<CQLObjPtr> _ptrs;
CQLOBJPTR _ObjPtr;

PEGASUS_NAMESPACE_BEGIN

extern CQLParserState* CQL_globalParserState;
Array<CQLPredicate> _arglist;


PEGASUS_NAMESPACE_END


void CQL_Arglist_Cleanup()
{
    _arglist.clear();
}

void CQL_Bison_Cleanup(){
    for(Uint32 i = 0; i < _ptrs.size(); i++)
    {
      if(_ptrs[i]._ptr)
      {
        switch(_ptrs[i].type)
        {
            case Id:
                    delete (CQLIdentifier*)_ptrs[i]._ptr;
                    break;
            case CId:
                    delete (CQLChainedIdentifier*)_ptrs[i]._ptr;
                    break;
            case Val:
                    delete (CQLValue*)_ptrs[i]._ptr;
                    break;
            case Func:
                    delete (CQLFunction*)_ptrs[i]._ptr;
                    break;
            case Fact:
                    delete (CQLFactor*)_ptrs[i]._ptr;
                    break;
            case Trm:
                    delete (CQLTerm*)_ptrs[i]._ptr;
                    break;
            case Expr:
                    delete (CQLExpression*)_ptrs[i]._ptr;
                    break;
            case SPred:
                    delete (CQLSimplePredicate*)_ptrs[i]._ptr;
                    break;
            case Pred:
                    delete (CQLPredicate*)_ptrs[i]._ptr;
                    break;
            case Str:
                    delete (String*)_ptrs[i]._ptr;
        }
      }
    }
    CQL_Arglist_Cleanup();
    _ptrs.clear();
   _factory.cleanup();
    _factory = CQLFactory();
}

%}
%union {
    char * strValue;
    String * _string;
    CQLValue * _value;
    CQLIdentifier * _identifier;
    CQLChainedIdentifier * _chainedIdentifier;
    CQLPredicate * _predicate;
    ExpressionOpType _opType;
    void * _node;
}

/* terminals */
%token <strValue> TOK_IDENTIFIER
%token <strValue> TOK_STRING_LITERAL
%token <strValue> TOK_BINARY
%token <strValue> TOK_NEGATIVE_BINARY
%token <strValue> TOK_HEXADECIMAL
%token <strValue> TOK_NEGATIVE_HEXADECIMAL
%token <strValue> TOK_INTEGER
%token <strValue> TOK_NEGATIVE_INTEGER
%token <strValue> TOK_REAL
%token <strValue> TOK_NEGATIVE_REAL
%token <strValue> TOK_TRUE
%token <strValue> TOK_FALSE
%token <strValue> TOK_SCOPED_PROPERTY
%token <strValue> TOK_LPAR
%token <strValue> TOK_RPAR
%token <strValue> TOK_HASH
%token <strValue> TOK_DOT
%token <strValue> TOK_LBRKT
%token <strValue> TOK_RBRKT
%token <strValue> TOK_UNDERSCORE
%token <strValue> TOK_COMMA
%token <strValue> TOK_CONCAT
%token <strValue> TOK_DBL_PIPE
%token <strValue> TOK_PLUS
%token <strValue> TOK_MINUS
%token <strValue> TOK_TIMES
%token <strValue> TOK_DIV
%token <strValue> TOK_IS
%token <strValue> TOK_NULL
%token <_opType> TOK_EQ
%token <_opType> TOK_NE
%token <_opType> TOK_GT
%token <_opType> TOK_LT
%token <_opType> TOK_GE
%token <_opType> TOK_LE
%token <_opType> TOK_ISA
%token <_opType> TOK_LIKE
%token <strValue> TOK_NOT TOK_AND TOK_OR
%token <strValue> TOK_SCOPE
%token <strValue> TOK_ANY TOK_EVERY TOK_IN TOK_SATISFIES
%token <strValue> TOK_STAR
%token <strValue> TOK_DOTDOT
%token <strValue> TOK_SHARP TOK_DISTINCT
%token <strValue> TOK_SELECT
%token <strValue> TOK_FIRST
%token <strValue> TOK_FROM
%token <strValue> TOK_WHERE
%token <strValue> TOK_ORDER
%token <strValue> TOK_BY
%token <strValue> TOK_ASC
%token <strValue> TOK_DESC
%token <strValue> TOK_AS
%token <strValue> TOK_UNEXPECTED_CHAR


/* grammar - non terminals */
%type <_identifier> identifier
%type <_identifier> class_name
%type <_identifier> class_path
%type <_identifier> scoped_property
%type <_string> literal_string
%type <_value> binary_value
%type <_value> hex_value
%type <_value> decimal_value
%type <_value> real_value
%type <_value> literal
%type <_string> array_index
%type <_string> array_index_list
%type <_node> chain
%type <_predicate> concat
%type <_predicate> factor
%type <_predicate> term
%type <_predicate> arith
%type <_value> value_symbol
%type <_predicate> arith_or_value_symbol
%type <_opType> comp_op
%type <_predicate> comp
%type <_predicate> expr_factor
%type <_predicate> expr_term
%type <_predicate> expr
%type <_node> arg_list
%type <_node> from_specifier
%type <_node> from_criteria
%type <_chainedIdentifier> star_expr
%type <_node> selected_entry
%type <_node> select_list
%type <_node> select_list_tail
%type <_node> search_condition
%type <_node> optional_where
%type <_node> select_statement

%destructor {CQL_Bison_Cleanup(); }  TOK_SELECT

%start select_statement

%%

    /**
        The general pattern:  We construct small objects first (CQLIdentifiers,
        CQLValues etc) which get forwared to more complex rules where more
        complex objects are made.  Eventually we have constructed a top level
        CQLPredicate that gets added to the CQL_globalParserState select
        statement.

        Along the way we keep track of which rule we are processing so that
        any errors encountered are specific enough to actually do us some good.

        The CQLFactory object is used primarily to save space/coding efforts.
        It enables us to build complex CQL objects with one simple call,
        or query complex objects through one simple call.

    */

/* CQLIdentifier */
identifier  : TOK_IDENTIFIER
    {
        CQL_globalParserState->currentRule = "identifier";
        CQL_DEBUG_TRACE("BISON::identifier\n");

        if(isUTF8Str(CQL_lval.strValue))
        {
            $$ = new CQLIdentifier(String(CQL_lval.strValue));
                _ObjPtr._ptr = $$;
                _ObjPtr.type = Id;
                _ptrs.append(_ObjPtr);
        }
        else
        {
            CQL_DEBUG_TRACE("BISON::identifier-> BAD UTF\n");

            throw CQLSyntaxErrorException(
                MessageLoaderParms("CQL.CQL_y.BAD_UTF8",
                "Bad UTF8 encountered parsing rule $0"
                   " in position $1.",
                "identifier",
                CQL_globalParserState->currentTokenPos) );
        }
    }
;

class_name : identifier
    {
        CQL_globalParserState->currentRule = "class_name";
        CQL_DEBUG_TRACE2("BISON::class_name = %s\n",
            (const char *)($1->getName().getString().getCString()));
        $$ = $1;
    }
;

class_path : class_name
    {
        CQL_globalParserState->currentRule = "class_path";
        CQL_DEBUG_TRACE("BISON::class_path\n");
        $$ = $1;
    }
;
/**********************************
property_scope : class_path TOK_SCOPE
    {
        CQL_DEBUG_TRACE2("BISON::property_scope = %s\n",$1);
    }
***********************************/
;

/* CQLIdentifier */
scoped_property : TOK_SCOPED_PROPERTY
    {
        /*
        TOK_SCOPED_PROPERTY can be:
        - "A::prop"
        - "A::class.prop"
        - "A::class.prop#'OK'
        - "A::class.prop[4]"
        */
        CQL_globalParserState->currentRule = "scoped_property";
        CQL_DEBUG_TRACE2("BISON::scoped_property = %s\n", CQL_lval.strValue);

        if(isUTF8Str(CQL_lval.strValue))
        {
            String tmp(CQL_lval.strValue);
            $$ = new CQLIdentifier(tmp);
            _ObjPtr._ptr = $$;
            _ObjPtr.type = Id;
            _ptrs.append(_ObjPtr);
        }
        else
        {
            CQL_DEBUG_TRACE("BISON::scoped_property-> BAD UTF\n");
            throw CQLSyntaxErrorException(
               MessageLoaderParms("CQL.CQL_y.BAD_UTF8",
                  "Bad UTF8 encountered parsing rule $0 in position $1.",
                  "scoped_property",
                  CQL_globalParserState->currentTokenPos) );
        }
    }
;

/* String */
literal_string : TOK_STRING_LITERAL
    {
        /*
        Make sure the literal is valid UTF8, then make a String
        */
        CQL_globalParserState->currentRule = "literal_string";
        CQL_DEBUG_TRACE2("BISON::literal_string-> %s\n", CQL_lval.strValue);

        if(isUTF8Str(CQL_lval.strValue))
        {
            $$ = new String(CQL_lval.strValue);
            _ObjPtr._ptr = $$;
            _ObjPtr.type = Str;
            _ptrs.append(_ObjPtr);
        }
        else
        {
            CQL_DEBUG_TRACE("BISON::literal_string-> BAD UTF\n");

            throw CQLSyntaxErrorException(
               MessageLoaderParms("CQL.CQL_y.BAD_UTF8",
                  "Bad UTF8 encountered parsing rule $0 in position $1.",
                  "literal_string",
                  CQL_globalParserState->currentTokenPos));
        }
    }
;

/* CQLValue */
binary_value : TOK_BINARY
    {
        CQL_globalParserState->currentRule = "binary_value->TOK_BINARY";
        CQL_DEBUG_TRACE2("BISON::binary_value-> %s\n", CQL_lval.strValue);

        $$ = new CQLValue(CQL_lval.strValue, CQLValue::Binary);
        _ObjPtr._ptr = $$;
        _ObjPtr.type = Val;
        _ptrs.append(_ObjPtr);
    }
| TOK_NEGATIVE_BINARY
    {
        CQL_globalParserState->currentRule =
            "binary_value->TOK_NEGATIVE_BINARY";
        CQL_DEBUG_TRACE2("BISON::binary_value-> %s\n", CQL_lval.strValue);

        $$ = new CQLValue(CQL_lval.strValue, CQLValue::Binary, false);
        _ObjPtr._ptr = $$;
        _ObjPtr.type = Val;
        _ptrs.append(_ObjPtr);
    }
;

/* CQLValue */
hex_value : TOK_HEXADECIMAL
    {
        CQL_globalParserState->currentRule = "hex_value->TOK_HEXADECIMAL";
        CQL_DEBUG_TRACE2("BISON::hex_value-> %s\n", CQL_lval.strValue);

        $$ = new CQLValue(CQL_lval.strValue, CQLValue::Hex);
        _ObjPtr._ptr = $$;
        _ObjPtr.type = Val;
        _ptrs.append(_ObjPtr);
    }
| TOK_NEGATIVE_HEXADECIMAL
    {
        CQL_globalParserState->currentRule =
            "hex_value->TOK_NEGATIVE_HEXADECIMAL";
        CQL_DEBUG_TRACE2("BISON::hex_value-> %s\n", CQL_lval.strValue);

        $$ = new CQLValue(CQL_lval.strValue, CQLValue::Hex, false);
        _ObjPtr._ptr = $$;
        _ObjPtr.type = Val;
        _ptrs.append(_ObjPtr);
    }
;

/* CQLValue */
decimal_value : TOK_INTEGER
    {
        CQL_globalParserState->currentRule = "decimal_value->TOK_INTEGER";
        CQL_DEBUG_TRACE2("BISON::decimal_value-> %s\n", CQL_lval.strValue);

        $$ = new CQLValue(CQL_lval.strValue, CQLValue::Decimal);
        _ObjPtr._ptr = $$;
        _ObjPtr.type = Val;
        _ptrs.append(_ObjPtr);
    }
  | TOK_NEGATIVE_INTEGER
    {
        CQL_globalParserState->currentRule =
            "decimal_value->TOK_NEGATIVE_INTEGER";
        CQL_DEBUG_TRACE2("BISON::decimal_value-> %s\n", CQL_lval.strValue);

        $$ = new CQLValue(CQL_lval.strValue, CQLValue::Decimal, false);
        _ObjPtr._ptr = $$;
        _ObjPtr.type = Val;
        _ptrs.append(_ObjPtr);
    }
;

/* CQLValue */
real_value : TOK_REAL
    {
        CQL_globalParserState->currentRule = "real_value->TOK_REAL";
        CQL_DEBUG_TRACE2("BISON::real_value-> %s\n", CQL_lval.strValue);

        $$ = new CQLValue(CQL_lval.strValue, CQLValue::Real);
        _ObjPtr._ptr = $$;
        _ObjPtr.type = Val;
        _ptrs.append(_ObjPtr);
    }
  | TOK_NEGATIVE_REAL
    {
        CQL_globalParserState->currentRule = "real_value->TOK_NEGATIVE_REAL";
        CQL_DEBUG_TRACE2("BISON::real_value-> %s\n", CQL_lval.strValue);

        $$ = new CQLValue(CQL_lval.strValue, CQLValue::Real, false);
        _ObjPtr._ptr = $$;
        _ObjPtr.type = Val;
        _ptrs.append(_ObjPtr);
    }
;

/* CQLValue */
literal : literal_string
        {
            CQL_globalParserState->currentRule = "literal->literal_string";
            CQL_DEBUG_TRACE("BISON::literal->literal_string\n");
            $$ = new CQLValue(*$1);
            _ObjPtr._ptr = $$;
            _ObjPtr.type = Val;
            _ptrs.append(_ObjPtr);
        }
    | decimal_value
        {
            CQL_globalParserState->currentRule = "literal->decimal_value";
            CQL_DEBUG_TRACE("BISON::literal->decimal_value\n");
        }
    | binary_value
        {
            CQL_globalParserState->currentRule = "literal->binary_value";
            CQL_DEBUG_TRACE("BISON::literal->binary_value\n");
        }
    | hex_value
        {
            CQL_globalParserState->currentRule = "literal->hex_value";
            CQL_DEBUG_TRACE("BISON::literal->hex_value\n");
        }
    | real_value
        {
            CQL_globalParserState->currentRule = "literal->real_value";
            CQL_DEBUG_TRACE("BISON::literal->real_value\n");
        }
    | TOK_TRUE
        {
            CQL_globalParserState->currentRule = "literal->TOK_TRUE";
            CQL_DEBUG_TRACE("BISON::literal->TOK_TRUE\n");

            $$ = new CQLValue(Boolean(true));
            _ObjPtr._ptr = $$;
            _ObjPtr.type = Val;
            _ptrs.append(_ObjPtr);
        }
    | TOK_FALSE
        {
            CQL_globalParserState->currentRule = "literal->TOK_FALSE";
            CQL_DEBUG_TRACE("BISON::literal->TOK_FALSE\n");

            $$ = new CQLValue(Boolean(false));
            _ObjPtr._ptr = $$;
            _ObjPtr.type = Val;
            _ptrs.append(_ObjPtr);
        }
;

/* String */
array_index : expr
    {
        CQL_globalParserState->currentRule = "array_index->expr";
        CQL_DEBUG_TRACE("BISON::array_index->expr\n");

        CQLValue* _val = (CQLValue*)_factory.getObject($1,Predicate,Value);
        $$ = new String(_val->toString());
        _ObjPtr._ptr = $$;
        _ObjPtr.type = Str;
        _ptrs.append(_ObjPtr);
    }
;

/* String */
array_index_list : array_index
    {
        CQL_globalParserState->currentRule = "array_index_list->array_index";
        CQL_DEBUG_TRACE("BISON::array_index_list->array_index\n");

        $$ = $1;
    }
;

/* void* */
chain : literal
    {
        CQL_globalParserState->currentRule = "chain->literal";
        CQL_DEBUG_TRACE("BISON::chain->literal\n");

        chain_state = CQLVALUE;
        $$ = _factory.makeObject($1,Predicate);
    }

  | TOK_LPAR expr TOK_RPAR
    {
        CQL_globalParserState->currentRule = "chain-> ( expr )";
        CQL_DEBUG_TRACE("BISON::chain-> ( expr )\n");

        chain_state = CQLPREDICATE;
        $$ = $2;
    }

  | identifier
    {
        CQL_globalParserState->currentRule = "chain->identifier";
        CQL_DEBUG_TRACE("BISON::chain->identifier\n");

        chain_state = CQLIDENTIFIER;
        $$ = _factory.makeObject($1,Predicate);
    }

  | identifier TOK_HASH literal_string
    {
        CQL_globalParserState->currentRule =
            "chain->identifier#literal_string";
        CQL_DEBUG_TRACE("BISON::chain->identifier#literal_string\n");

        String tmp = $1->getName().getString();
        tmp.append("#").append(*$3);
        CQLIdentifier _id(tmp);
        $$ = _factory.makeObject(&_id,Predicate);
        chain_state = CQLIDENTIFIER;
    }

  | scoped_property
    {
        CQL_globalParserState->currentRule = "chain->scoped_property";
        CQL_DEBUG_TRACE("BISON::chain->scoped_property\n");

        chain_state = CQLIDENTIFIER;
        $$ = _factory.makeObject($1,Predicate);
    }

  | identifier TOK_LPAR arg_list TOK_RPAR
    {
        CQL_globalParserState->currentRule = "identifier( arg_list )";
        CQL_DEBUG_TRACE("BISON::identifier( arg_list )\n");

        chain_state = CQLFUNCTION;
        CQLFunction _func(*$1,_arglist);
        $$ = (CQLPredicate*)(_factory.makeObject(&_func,Predicate));
        CQL_Arglist_Cleanup();
    }

  | chain TOK_DOT scoped_property
    {
        CQL_globalParserState->currentRule = "chain->chain.scoped_property";
        CQL_DEBUG_TRACE2(
            "BISON::chain-> chain TOK_DOT scoped_property : chain_state = %d\n"
            ,chain_state);

        CQLIdentifier *_id;
        if(chain_state == CQLIDENTIFIER)
        {
            _id = ((CQLIdentifier*)(_factory.getObject(
                $1,Predicate,Identifier)));
            CQLChainedIdentifier _cid(*_id);
            _cid.append(*$3);
            $$ = _factory.makeObject(&_cid,Predicate);
        }
        else if(chain_state == CQLCHAINEDIDENTIFIER)
        {
            CQLChainedIdentifier *_cid;
            _cid = ((CQLChainedIdentifier*)(_factory.getObject(
                $1,Predicate,ChainedIdentifier)));
            _cid->append(*$3);
            _factory.setObject(((CQLPredicate*)$1),_cid,ChainedIdentifier);
            $$ = $1;
        }
        else  /* error */
        {
            String _msg("chain-> chain TOK_DOT scoped_property :"
                " chain state not CQLIDENTIFIER or CQLCHAINEDIDENTIFIER");

            throw CQLSyntaxErrorException(
                MessageLoaderParms("CQL.CQL_y.NOT_CHAINID_OR_IDENTIFIER",
                    "Chain state not a CQLIdentifier or a"
                        " CQLChainedIdentifier while parsing rule $0 in"
                        " position $1.",
                    "chain.scoped_property",
                    CQL_globalParserState->currentTokenPos));
        }

    chain_state = CQLCHAINEDIDENTIFIER;
    }

  | chain TOK_DOT identifier
    {
        CQL_globalParserState->currentRule = "chain->chain.identifier";
        CQL_DEBUG_TRACE2("BISON::chain->chain.identifier : chain_state = %d\n"
            ,chain_state);

        if(chain_state == CQLIDENTIFIER)
        {
            CQLIdentifier *_id = ((CQLIdentifier*)(_factory.getObject(
            $1,Predicate,Identifier)));
            CQLChainedIdentifier _cid(*_id);
            _cid.append(*$3);
            $$ = _factory.makeObject(&_cid,Predicate);
        }
        else if(chain_state == CQLCHAINEDIDENTIFIER)
        {
            CQLChainedIdentifier *_cid =
            ((CQLChainedIdentifier*)(_factory.getObject(
            $1,Predicate,ChainedIdentifier)));
            _cid->append(*$3);
            _factory.setObject(((CQLPredicate*)$1),_cid,ChainedIdentifier);
            $$ = $1;
        }
        else  /* error */
        {
            String _msg("chain-> chain TOK_DOT identifier : chain state not"
                " CQLIDENTIFIER or CQLCHAINEDIDENTIFIER");

            throw CQLSyntaxErrorException(
            MessageLoaderParms("CQL.CQL_y.NOT_CHAINID_OR_IDENTIFIER",
                "Chain state not a CQLIdentifier or a CQLChainedId"
                    "Identifier while parsing rule $0 in position $1.",
                "chain.identifier",
                CQL_globalParserState->currentTokenPos));
        }
        chain_state = CQLCHAINEDIDENTIFIER;
    }

  | chain TOK_DOT identifier TOK_HASH literal_string
    {
        CQL_globalParserState->currentRule =
            "chain->chain.identifier#literal_string";
        CQL_DEBUG_TRACE2(
            "BISON::chain->chain.identifier#literal_string : chain_state = %d\n"
            ,chain_state);

        if(chain_state == CQLIDENTIFIER)
        {
            CQLIdentifier *_id = ((CQLIdentifier*)(_factory.getObject(
                $1,Predicate,Identifier)));
            CQLChainedIdentifier _cid(*_id);
            String tmp($3->getName().getString());
            tmp.append("#").append(*$5);
            CQLIdentifier _id1(tmp);
            _cid.append(_id1);
            _factory.setObject(((CQLPredicate*)$1),&_cid,ChainedIdentifier);
            $$ = $1;
        }
        else if(chain_state == CQLCHAINEDIDENTIFIER)
        {
            CQLChainedIdentifier *_cid =  ((CQLChainedIdentifier*)(
                _factory.getObject($1,Predicate,ChainedIdentifier)));
            String tmp($3->getName().getString());
            tmp.append("#").append(*$5);
            CQLIdentifier _id1(tmp);
            _cid->append(_id1);
            _factory.setObject(((CQLPredicate*)$1),_cid,ChainedIdentifier);
            $$ = $1;
        }
        else  /* error */
        {
            String _msg("chain->chain.identifier#literal_string : chain"
                " state not CQLIDENTIFIER or CQLCHAINEDIDENTIFIER");

            throw CQLSyntaxErrorException(
            MessageLoaderParms("CQL.CQL_y.NOT_CHAINID_OR_IDENTIFIER",
                "Chain state not a CQLIdentifier or a CQLChained"
                    "Identifier while parsing rule $0 in position $1.",
                "chain.identifier#literal_string",
                CQL_globalParserState->currentTokenPos) );
            }
        chain_state = CQLCHAINEDIDENTIFIER;
    }

  | chain TOK_LBRKT array_index_list TOK_RBRKT
    {
        CQL_globalParserState->currentRule =
            "chain->chain[ array_index_list ]";
        CQL_DEBUG_TRACE2(
            "BISON::chain->chain[ array_index_list ] : chain_state = %d\n"
            ,chain_state);

        if(chain_state == CQLIDENTIFIER)
        {
            CQLIdentifier *_id = ((CQLIdentifier*)
                (_factory.getObject($1,Predicate,Identifier)));
            String tmp = _id->getName().getString();
            tmp.append("[").append(*$3).append("]");
            CQLIdentifier _id1(tmp);
            CQLChainedIdentifier _cid(_id1);
            _factory.setObject(((CQLPredicate*)$1),&_cid,ChainedIdentifier);
            $$ = $1;
        }
        else if(chain_state == CQLCHAINEDIDENTIFIER || chain_state == CQLVALUE)
        {
            CQLChainedIdentifier *_cid = ((CQLChainedIdentifier*)
                (_factory.getObject
                ($1,Predicate,ChainedIdentifier)));
            CQLIdentifier tmpid = _cid->getLastIdentifier();
            String tmp = tmpid.getName().getString();
            tmp.append("[").append(*$3).append("]");
            CQLIdentifier _id1(tmp);
            CQLChainedIdentifier _tmpcid(_id1);

            if(_cid->size() == 1)
            {
                _cid = &_tmpcid;
            }
            else
            {
                _cid->append(_id1);
            }
            _factory.setObject(((CQLPredicate*)$1),_cid,ChainedIdentifier);
            $$ = $1;
        }
        else  /* error */
        {
            String _msg("chain->chain[ array_index_list ] : chain state not"
                " CQLIDENTIFIER or CQLCHAINEDIDENTIFIER or CQLVALUE");

            throw CQLSyntaxErrorException(
               MessageLoaderParms(
                  "CQL.CQL_y.NOT_CHAINID_OR_IDENTIFIER_OR_VALUE",
                  "Chain state not a CQLIdentifier or a "
                    "CQLChainedIdentifier or a CQLValue while parsing rule"
                        " $0 in position $1.",
                  "chain->chain[ array_index_list ]",
                  CQL_globalParserState->currentTokenPos) );
        }
    }
;

concat : chain
    {
        CQL_globalParserState->currentRule = "concat->chain";
        CQL_DEBUG_TRACE("BISON::concat->chain\n");

        $$ = ((CQLPredicate*)$1);
    }

  | concat TOK_DBL_PIPE literal_string
    {
        CQL_globalParserState->currentRule =
            "concat->concat || literal_string";
        CQL_DEBUG_TRACE("BISON::concat->concat || literal_string\n");

        CQLValue* tmpval = new CQLValue(*$3);
        _ObjPtr._ptr = tmpval;
        _ObjPtr.type = Val;
        _ptrs.append(_ObjPtr);

        if((CQLPredicate*)$1->isSimple())
        {
            CQLSimplePredicate sp = ((CQLPredicate*)$1)->getSimplePredicate();
            if(sp.isSimple())
            {
                CQLExpression exp = sp.getLeftExpression();
                if(exp.isSimple())
                {
                    CQLTerm* _term = ((CQLTerm*)
                        (_factory.getObject($1, Predicate, Term)));
                    // check for simple literal values
                    Array<CQLFactor> factors = _term->getFactors();
                    for(Uint32 i = 0; i < factors.size(); i++)
                    {
                        if(!factors[i].isSimpleValue())
                        {
                            MessageLoaderParms mparms(
                                "CQL.CQL_y.CONCAT_PRODUCTION_FACTORS_NOT"
                                "_SIMPLE",
                                "The CQLFactors are not simple while"
                                " processing rule $0.",
                                CQL_globalParserState->currentRule);
                            throw CQLSyntaxErrorException(mparms);
                        }
                        else /* !factors[i].isSimpleValue() */
                        {
                            CQLValue val = factors[i].getValue();
                            if(val.getValueType() != CQLValue::String_type)
                            {
                                MessageLoaderParms mparms(
                                 "CQL.CQL_y."
                                     "CONCAT_PRODUCTION_VALUE_NOT_LITERAL",
                                 "The CQLValue is not a string literal while"
                                     " processing rule $0.",
                                 CQL_globalParserState->currentRule);
                                throw CQLSyntaxErrorException(mparms);
                            }
                        }
                    }  // End For factors.size() Loop

                    CQLFactor* _fctr2 = ((CQLFactor*)
                        (_factory.makeObject(tmpval, Factor)));
                    _term->appendOperation(concat,*_fctr2);
                    $$ = (CQLPredicate*)(_factory.makeObject
                        (_term,Predicate));
                }
                else /* exp.isSimple() */
                {
                    MessageLoaderParms mparms(
                        "CQL.CQL_y.CONCAT_PRODUCTION_NOT_SIMPLE",
                        "The $0 is not simple while processing rule $1.",
                        "CQLExpression",
                        CQL_globalParserState->currentRule);
                    throw CQLSyntaxErrorException(mparms);
                }
            }
            else /*sp.isSimple() */
            {
                 MessageLoaderParms mparms(
                    "CQL.CQL_y.CONCAT_PRODUCTION_NOT_SIMPLE",
                    "The $0 is not simple while processing rule $1.",
                    "CQLSimplePredicate",
                    CQL_globalParserState->currentRule);
                 throw CQLSyntaxErrorException(mparms);
            }
        }
        else /* end if((CQLPredicate*)$1->isSimple()) */
        {
            MessageLoaderParms mparms(
                "CQL.CQL_y.CONCAT_PRODUCTION_NOT_SIMPLE",
                "The $0 is not simple while processing rule $1.",
                "CQLPredicate",
                CQL_globalParserState->currentRule);
            throw CQLSyntaxErrorException(mparms);
        }
    }
;

factor : concat
    {
        CQL_globalParserState->currentRule = "factor->concat";
        CQL_DEBUG_TRACE("BISON::factor->concat\n");

        $$ = $1;
    }
    /*
  | TOK_PLUS concat
    {
        add enum instead of _invert to CQLFactor,has to be either nothing,
        + or -
        get the factor and set the optype appropriately
        CQL_globalParserState->currentRule = "concat->TOK_PLUS concat";
        printf("BISON::factor->TOK_PLUS concat\n");
        ...
        $$ = new CQLFactor(*(CQLValue*)$2);
    }
  | TOK_MINUS concat
    {
        get the factor and set the optype appropriately
        CQL_globalParserState->currentRule = "concat->TOK_MINUS concat";
        printf("BISON::factor->TOK_MINUS concat\n");
        ...
        CQLValue *tmp = (CQLValue*)$2;
        tmp->invert();
        $$ = new CQLFactor(*tmp);
    }
    */
;

term : factor
    {
        CQL_globalParserState->currentRule = "term->factor";
        CQL_DEBUG_TRACE("BISON::term->factor\n");

        $$ = $1;
    }
 /*
  | term TOK_STAR factor
    {
        get factor out of $1, get factor out of $3, appendoperation,
        then construct predicate and forward it
        printf("BISON::term->term TOK_STAR factor\n");
        CQL_globalParserState->currentRule = "term->term TOK_STAR factor";
        $1->appendOperation(mult, *$3);
        $$ = $1;
    }
  | term TOK_DIV factor
    {
        get factor out of $1, get factor out of $3, appendoperation,
        then construct predicate and forward it
        CQL_globalParserState->currentRule = "term->term TOK_DIV factor";
        printf("BISON::term->term TOK_DIV factor\n");
        $1->appendOperation(divide, *$3);
        $$ = $1;
    }
 */
;

arith : term
    {
        CQL_globalParserState->currentRule = "arith->term";
        CQL_DEBUG_TRACE("BISON::arith->term\n");

        //CQLPredicate* _pred = new CQLPredicate(*$1);
        //      _factory._predicates.append(_pred);
        $$ = $1;
    }
/***********************8
   | arith TOK_PLUS term
   {
         get term out of $1, get term out of $3, appendoperation, then
         construct predicate and forward it
         CQL_globalParserState->currentRule = "arith->arith TOK_PLUS term";
         printf("BISON::arith->arith TOK_PLUS term\n");
         $1->appendOperation(TERM_ADD, *$3);
         $$ = $1;
    }
   | arith TOK_MINUS term
    {
         get term out of $1, get term out of $3, append operation, then
         construct predicate and forward it
         CQL_globalParserState->currentRule = "arith->arith TOK_MINUS term";
         printf("BISON::arith->arith TOK_MINUS term\n");
         $1->appendOperation(TERM_SUBTRACT, *$3);
         $$ = $1;
    }
************************/
;

value_symbol : TOK_HASH literal_string
    {
        CQL_globalParserState->currentRule = "value_symbol->#literal_string";
        CQL_DEBUG_TRACE("BISON::value_symbol->#literal_string\n");

        String tmp("#");
        tmp.append(*$2);
        CQLIdentifier tmpid(tmp);
        $$ = new CQLValue(tmpid);
        _ObjPtr._ptr = $$;
        _ObjPtr.type = Val;
        _ptrs.append(_ObjPtr);
    }
;

arith_or_value_symbol : arith
    {
        CQL_globalParserState->currentRule = "arith_or_value_symbol->arith";
        CQL_DEBUG_TRACE("BISON::arith_or_value_symbol->arith\n");

        $$ = $1;
    }
  | value_symbol
    {
        /* make into predicate */
        CQL_globalParserState->currentRule =
            "arith_or_value_symbol->value_symbol";
        CQL_DEBUG_TRACE("BISON::arith_or_value_symbol->value_symbol\n");

        CQLFactor _fctr(*$1);
        $$ = (CQLPredicate*)(_factory.makeObject(&_fctr, Predicate));
    }
;

comp_op : TOK_EQ
    {
        CQL_globalParserState->currentRule = "comp_op->TOK_EQ";
        CQL_DEBUG_TRACE("BISON::comp_op->TOK_EQ\n");

        $$ = EQ;
    }
  | TOK_NE
    {
        CQL_globalParserState->currentRule = "comp_op->TOK_NE";
        CQL_DEBUG_TRACE("BISON::comp_op->TOK_NE\n");
        $$ = NE;
    }
  | TOK_GT
    {
        CQL_globalParserState->currentRule = "comp_op->TOK_GT";
        CQL_DEBUG_TRACE("BISON::comp_op->TOK_GT\n");

        $$ = GT;
    }
  | TOK_LT
    {
        CQL_globalParserState->currentRule = "comp_op->TOK_LT";
        CQL_DEBUG_TRACE("BISON::comp_op->TOK_LT\n");
        $$ = LT;
    }
  | TOK_GE
    {
        CQL_globalParserState->currentRule = "comp_op->TOK_GE";
        CQL_DEBUG_TRACE("BISON::comp_op->TOK_GE\n");
        $$ = GE;
    }
  | TOK_LE
    {
        CQL_globalParserState->currentRule = "comp_op->TOK_LE";
        CQL_DEBUG_TRACE("BISON::comp_op->TOK_LE\n");
        $$ = LE;
    }
;

comp : arith
    {
        CQL_globalParserState->currentRule = "comp->arith";
        CQL_DEBUG_TRACE("BISON::comp->arith\n");

        $$ = $1;
    }
  | arith TOK_IS TOK_NOT TOK_NULL
    {
        CQL_globalParserState->currentRule =
            "comp->arith TOK_IS TOK_NOT TOK_NULL";
        CQL_DEBUG_TRACE("BISON::comp->arith TOK_IS TOK_NOT TOK_NULL\n");

        CQLExpression *_expr =
            (CQLExpression*)(_factory.getObject($1,Expression));
        CQLSimplePredicate _sp(*_expr, IS_NOT_NULL);
        _factory.setObject($1,&_sp,SimplePredicate);
        $$ = $1;
    }
  | arith TOK_IS TOK_NULL
    {
        CQL_globalParserState->currentRule = "comp->arith TOK_IS TOK_NULL";
        CQL_DEBUG_TRACE("BISON::comp->arith TOK_IS TOK_NULL\n");

        CQLExpression *_expr =
            (CQLExpression*)(_factory.getObject($1,Expression));
        CQLSimplePredicate _sp(*_expr, IS_NULL);
        _factory.setObject($1,&_sp,SimplePredicate);
        $$ = $1;
    }
  | arith comp_op arith_or_value_symbol
    {
        CQL_globalParserState->currentRule =
            "comp->arith comp_op arith_or_value_symbol";
        CQL_DEBUG_TRACE("BISON::comp->arith comp_op arith_or_value_symbol\n");

        if($1->isSimple() && $3->isSimple())
        {
            CQLExpression* _exp1 = (CQLExpression*)
            (_factory.getObject($1,Predicate,Expression));
            CQLExpression* _exp2 = (CQLExpression*)
            (_factory.getObject($3,Predicate,Expression));
            CQLSimplePredicate _sp(*_exp1, *_exp2, $2);
            $$ = new CQLPredicate(_sp);
            _ObjPtr._ptr = $$;
            _ObjPtr.type = Pred;
            _ptrs.append(_ObjPtr);
        }
        else  /* error */
        {
           String _msg("comp->arith comp_op arith_or_value_symbol : $1 is "
               "not simple OR $3 is not simple");
           throw CQLSyntaxErrorException(
               MessageLoaderParms("CQL.CQL_y.NOT_SIMPLE",
                   "The CQLSimplePredicate is not simple while parsing "
                   "rule $0 in position $1.",
                       "comp->arith comp_op arith_or_value_symbol",
                   CQL_globalParserState->currentTokenPos) );
        }
    }
  | value_symbol comp_op arith
    {
        CQL_globalParserState->currentRule =
            "comp->value_symbol comp_op arith";
        CQL_DEBUG_TRACE("BISON::comp->value_symbol comp_op arith\n");

        if($3->isSimple())
        {
            CQLExpression* _exp1 = (CQLExpression*)
                (_factory.makeObject($1, Expression));
            CQLExpression* _exp2 = (CQLExpression*)
                (_factory.getObject($3,Predicate,Expression));
            CQLSimplePredicate _sp(*_exp1, *_exp2, $2);
            $$ = new CQLPredicate(_sp);
            _ObjPtr._ptr = $$;
            _ObjPtr.type = Pred;
            _ptrs.append(_ObjPtr);
        }
        else  /* error */
        {
            String _msg("comp->value_symbol comp_op arith : $3 is not simple");
            throw CQLSyntaxErrorException(
                MessageLoaderParms("CQL.CQL_y.NOT_SIMPLE",
                    "The CQLSimplePredicate is not simple while parsing"
                        " rule $0 in position $1.",
                    "comp->value_symbol comp_op arith",
                CQL_globalParserState->currentTokenPos) );

        }
    }
  | arith TOK_ISA identifier
    {
        /* make sure $1 isSimple(), get its expression, make
           simplepred->predicate
        */
        CQL_globalParserState->currentRule = "comp->arith _TOK_ISA identifier";
        CQL_DEBUG_TRACE("BISON::comp->arith _TOK_ISA identifier\n");

        CQLExpression *_expr1 = (CQLExpression*)
        (_factory.getObject($1,Predicate,Expression));
        CQLChainedIdentifier _cid(*$3);
        CQLExpression *_expr2 = (CQLExpression*)
        (_factory.makeObject(&_cid,Expression));
        CQLSimplePredicate _sp(*_expr1, *_expr2, ISA);
        _factory.setObject($1,&_sp,SimplePredicate);
        $$ = $1;
    }
  | arith TOK_LIKE literal_string
    {
        CQL_globalParserState->currentRule =
            "comp->arith TOK_LIKE literal_string";
        CQL_DEBUG_TRACE("BISON::comp->arith TOK_LIKE literal_string\n");

        CQLExpression *_expr1 = (CQLExpression*)
            (_factory.getObject($1,Predicate,Expression));
        CQLValue _val(*$3);
        CQLExpression *_expr2 = (CQLExpression*)
            (_factory.makeObject(&_val,Expression));
        CQLSimplePredicate _sp(*_expr1, *_expr2, LIKE);
        _factory.setObject($1,&_sp,SimplePredicate);
        $$ = $1;
    }
;
expr_factor : comp
    {
        CQL_globalParserState->currentRule = "expr_factor->comp";
        CQL_DEBUG_TRACE("BISON::expr_factor->comp\n");

        $$ = $1;
    }
  | TOK_NOT comp
    {
        CQL_globalParserState->currentRule = "expr_factor->TOK_NOT comp";
        CQL_DEBUG_TRACE("BISON::expr_factor->TOK_NOT comp\n");

        $2->setInverted(!($2->getInverted()));
        $$ = $2;
    }
;

expr_term : expr_factor
    {
        CQL_globalParserState->currentRule = "expr_term->expr_factor";
        CQL_DEBUG_TRACE("BISON::expr_term->expr_factor\n");

        $$ = $1;
    }
| expr_term TOK_AND expr_factor
    {
        CQL_globalParserState->currentRule =
            "expr_term->expr_term AND expr_factor";
        CQL_DEBUG_TRACE("BISON::expr_term->expr_term AND expr_factor\n");

        $$ = new CQLPredicate();
        $$->appendPredicate(*$1);
        $$->appendPredicate(*$3, AND);
         _ObjPtr._ptr = $$;
        _ObjPtr.type = Pred;
        _ptrs.append(_ObjPtr);
    }
;

expr : expr_term
    {
        CQL_globalParserState->currentRule = "expr->expr_term";
        CQL_DEBUG_TRACE("BISON::expr->expr_term\n");

        $$ = $1;
        }
| expr TOK_OR expr_term
    {
        CQL_globalParserState->currentRule = "expr->expr OR expr_term";
        CQL_DEBUG_TRACE("BISON::expr->expr OR expr_term\n");

        $$ = new CQLPredicate();
        $$->appendPredicate(*$1);
        $$->appendPredicate(*$3, OR);
        _ObjPtr._ptr = $$;
        _ObjPtr.type = Pred;
        _ptrs.append(_ObjPtr);
    }
;

arg_list : {;}
/****************
   | TOK_STAR
      {
        CQL_globalParserState->currentRule = "arg_list->TOK_STAR";
        CQL_DEBUG_TRACE("BISON::arg_list->TOK_STAR\n");

         CQLIdentifier _id("*");
         CQLPredicate* _pred = (CQLPredicate*)
            (_factory.makeObject(&_id,Predicate));
         _arglist.append(*_pred);
                    since arg_list can loop back on itself,
                    we need to store away previous solutions
                    production.  We keep track of previous productions
                    in the _arglist array and later pass that to CQLFunction
                    as part of chain: identifier TOK_LPAR arg_list TOK_RPAR

      }
*******************/
    | expr
    {
        CQL_globalParserState->currentRule = "arg_list->arg_list_sub->expr";
        CQL_DEBUG_TRACE("BISON::arg_list->arg_list_sub->expr\n");

        _arglist.append(*$1);
        /*
        since arg_list can loop back on itself,
        we need to store away previous solutions
        production.  We keep track of previous productions
        in the _arglist array and later pass that to CQLFunction
        as part of chain: identifier TOK_LPAR arg_list TOK_RPAR
        */
    }
/****************************
   | TOK_DISTINCT TOK_STAR
      {
        CQL_globalParserState->currentRule = "arg_list->TOK_DISTINCT TOK_STAR";
        CQL_DEBUG_TRACE("BISON::arg_list->TOK_DISTINCT TOK_STAR\n");

         CQLIdentifier _id("DISTINCTSTAR");
         CQLPredicate* _pred = (CQLPredicate*)
            (_factory.makeObject(&_id,Predicate));
         _arglist.append(*_pred);
      }
   | arg_list_sub arg_list_tail
      {
         CQL_DEBUG_TRACE("BISON::arg_list->arg_list_sub arg_list_tail\n");
      }
;

arg_list_sub : expr
      {
         CQL_DEBUG_TRACE("BISON::arg_list_sub->expr\n");

         _arlist.append(*$1);
      }
   | TOK_DISTINCT expr
      {
         CQL_DEBUG_TRACE("BISON::arg_list_sub->TOK_DISTINCT expr\n");

         String tmp1("TOK_DISTINCT");
         CQLIdentifier* _id = (CQLIdentifier*)
            (_factory.getObject($1,Predicate,Identifier));
         String tmp2(_id->getName().getString());
         tmp1.append(tmp2);
         CQLIdentifier _id1(tmp1);
      }
;

arg_list_tail : {;}
   | TOK_COMMA arg_list_sub arg_list_tail
      {
         CQL_DEBUG_TRACE("BISON::arg_list_tail->TOK_COMMA arg_list_sub"
             " arg_list_tail\n");
      }
;
*******************/

from_specifier : class_path
    {
        CQL_globalParserState->currentRule = "from_specifier->class_path";
        CQL_DEBUG_TRACE("BISON::from_specifier->class_path\n");

        CQL_globalParserState->statement->appendClassPath(*$1);
    }
  | class_path TOK_AS identifier
    {
        CQL_globalParserState->currentRule =
            "from_specifier->class_path TOK_AS identifier";
        CQL_DEBUG_TRACE(
            "BISON::from_specifier->class_path TOK_AS identifier\n");

        CQLIdentifier _class(*$1);
        String _alias($3->getName().getString());
        CQL_globalParserState->statement->insertClassPathAlias(_class,_alias);
        CQL_globalParserState->statement->appendClassPath(_class);
    }
  | class_path identifier
    {
        CQL_globalParserState->currentRule =
            "from_specifier->class_path identifier";
        CQL_DEBUG_TRACE("BISON::from_specifier->class_path identifier\n");

        CQLIdentifier _class(*$1);
        String _alias($2->getName().getString());
        CQL_globalParserState->statement->insertClassPathAlias(_class,_alias);
        CQL_globalParserState->statement->appendClassPath(_class);
    }
;

from_criteria : from_specifier
    {
        CQL_globalParserState->currentRule = "from_criteria->from_specifier";
        CQL_DEBUG_TRACE("BISON::from_criteria->from_specifier\n");
    }
;

star_expr : TOK_STAR
    {
        CQL_globalParserState->currentRule = "star_expr->TOK_STAR";
        CQL_DEBUG_TRACE("BISON::star_expr->TOK_STAR\n");

        CQLIdentifier _id("*");
        $$ = (CQLChainedIdentifier*)
             (_factory.makeObject(&_id,ChainedIdentifier));
    }
  | chain TOK_DOT TOK_STAR
    {
        CQL_globalParserState->currentRule = "star_expr->chain.*";
        CQL_DEBUG_TRACE("BISON::star_expr->chain.*\n");

        CQLChainedIdentifier* _tmp = (CQLChainedIdentifier*)
        (_factory.getObject($1,Predicate,ChainedIdentifier));
        CQLChainedIdentifier* _cid = new CQLChainedIdentifier(*_tmp);
        CQLIdentifier _id("*");
        _cid->append(_id);
        $$ = _cid;
        _ObjPtr._ptr = $$;
        _ObjPtr.type = CId;
        _ptrs.append(_ObjPtr);
    }
;

selected_entry : expr
    {
        CQL_globalParserState->currentRule = "selected_entry->expr";
        CQL_DEBUG_TRACE("BISON::selected_entry->expr\n");

        if($1->isSimpleValue())
        {
            CQLChainedIdentifier *_cid = (CQLChainedIdentifier*)
                (_factory.getObject($1,Predicate,ChainedIdentifier));
            CQL_globalParserState->statement->appendSelectIdentifier(*_cid);
        }
        else  /* errot*/
        {
            String _msg("selected_entry->expr : $1 is not a simple value");
            throw CQLSyntaxErrorException(
               MessageLoaderParms("CQL.CQL_y.NOT_SIMPLE_VALUE",
                  "The CQLPredicate is not a simple value while "
                      "parsing rule $0 in position $1.",
                  "selected_entry->expr",
                  CQL_globalParserState->currentTokenPos) );
        }
    }
| star_expr
    {
        CQL_globalParserState->currentRule = "selected_entry->star_expr";
        CQL_DEBUG_TRACE("BISON::selected_entry->star_expr\n");

        CQL_globalParserState->statement->appendSelectIdentifier(*$1);
    }
;

select_list : selected_entry select_list_tail
    {
        CQL_globalParserState->currentRule =
            "select_list->selected_entry select_list_tail";
        CQL_DEBUG_TRACE(
            "BISON::select_list->selected_entry select_list_tail\n");
    }
;

select_list_tail : {;} /* empty */

  | TOK_COMMA selected_entry select_list_tail
    {
        CQL_globalParserState->currentRule =
            "select_list_tail->TOK_COMMA selected_entry select_list_tail";
        CQL_DEBUG_TRACE(
           "BISON::select_list_tail->TOK_COMMA"
           " selected_entry select_list_tail\n");
    }
;

search_condition : expr
    {
        CQL_globalParserState->currentRule = "search_condition->expr";
        CQL_DEBUG_TRACE("BISON::search_condition->expr\n");

        CQL_globalParserState->statement->setPredicate(*$1);
    }
;

optional_where : {}

  | TOK_WHERE search_condition
    {
        CQL_globalParserState->currentRule =
            "optional_where->TOK_WHERE search_condition";
        CQL_DEBUG_TRACE(
            "BISON::optional_where->TOK_WHERE search_condition\n");

        CQL_globalParserState->statement->setHasWhereClause();
    }
;

select_statement : TOK_SELECT select_list TOK_FROM from_criteria optional_where
    {
        CQL_globalParserState->currentRule = "select_statement";
        CQL_DEBUG_TRACE("select_statement\n");

        CQL_Bison_Cleanup();
    }

;

%%
/*int yyerror(char * err){yyclearin; yyerrok;throw Exception(String(err));
return 1;}*/
