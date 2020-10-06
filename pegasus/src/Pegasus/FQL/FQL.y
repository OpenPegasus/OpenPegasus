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
#include <Pegasus/FQL/FQLOperation.h>
#include <Pegasus/FQL/FQLOperand.h>
#include <Pegasus/FQL/FQLParserState.h>
#include <Pegasus/FQL/FQLQueryStatement.h>
#include <string.h>
#include <stdlib.h>

#ifdef PEGASUS_OS_TYPE_WINDOWS
# include <malloc.h>
#endif

#if defined(PEGASUS_COMPILER_ACC) && defined(PEGASUS_OS_HPUX)
# include <alloca.h>
#endif


/* Debugging the parser.  Debugging is provided through
   1. debug functions in Bison that are controlled by a compile time
      flag (YYDEBUG) and a runtime flag (yydebug) which is redefined
      to cimmof_debug.
   2. Debug functions defined through FQL_TRACE, a macro defined
      in cimmofparser.h and turned on and off manually.
   All debugging must be turned on manually at this point by
   setting the YYDEBUG compile flag and also setting YACCTRACE.
*/
// Enable this define to compile Bison/Yacc tracing
#ifdef FQL_YYDEBUG
#define YYDEBUG 1
/// extern int yydebug;
/// yydebug = 1;
#endif

#ifdef FQL_DOTRACE
# define FQL_TRACE(X) printf X
#else
# define FQL_TRACE(X)
#endif

extern int FQL_lex();
extern int FQL_error(const char*);

//
// Define the global parser state object:
//

PEGASUS_USING_PEGASUS;

PEGASUS_NAMESPACE_BEGIN

extern FQLParserState* globalParserState;
extern Array<FQLOperand*> _argList;
//// extern Array<String*> _propertyList;

PEGASUS_NAMESPACE_END

%}

// defines the use of the glr parser because there are multiple places in
// the grammar where there is multiple-symbol lookahead and the glr parser
// solves those problems.
%glr-parser
////%expect-rr 1

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

// This does not seem to do anything
%error-verbose
/*
**==============================================================================
**
** Tokens, types, and associative rules.
**
**==============================================================================
*/

%token <intValue> TOK_INTEGER
%token <intValue> TOK_UNSIGNED_INTEGER
%token <doubleValue> TOK_DOUBLE
%token <intValue> TOK_HEXADECIMAL
%token <intValue> TOK_BINARY

%token <strValue> TOK_STRING
%token <strValue> TOK_DATETIME
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
%token <intValue> TOK_LIKE

%token <intValue> TOK_NOT
%token <intValue> TOK_OR
%token <intValue> TOK_AND
%token <intValue> TOK_IS
%token <intValue> TOK_ANY
%token <intValue> TOK_EVERY
%token <strValue> TOK_IDENTIFIER
%token <strValue> TOK_REFERENCE_LIT

%token <strValue> TOK_LBRKT
%token <strValue> TOK_RBRKT
%token <strValue> TOK_LPAREN
%token <strValue> TOK_RPAREN
%token <strValue> TOK_LBRACE
%token <strValue> TOK_RBRACE

%token <strValue> TOK_COMMA

%token <intValue> TOK_UNEXPECTED_CHAR

%type <intValue> booleanLiteral
%type <intValue> binaryLiteral
%type <intValue> hexLiteral
%type <intValue> decimalLiteral
%type <intValue> integerLiteral
%type <nodeValue> scalarProperty
%type <strValue> arrayProperty
%type <strValue> property
%type <strValue> indexedArrayProperty
%type <intValue> index

%type <nodeValue> FQLQueryStatement
%type <nodeValue> fqlExpression
%type <intValue> scalarCompOp
%type <nodeValue> fqlExpressionList
%type <nodeValue> scalarLiteral

%left TOK_OR TOK_AND
%left TOK_LIKE TOK_EQ TOK_NE TOK_LE TOK_GE
%left TOK_ANY
%left TOK_EVERY
%nonassoc TOK_NOT

%%

start
    : FQLQueryStatement
    {
        FQL_TRACE(("YACC: start\n"));
    }

// fql = fql-expr / "(" fql-expr ")" *( bool-op "(" fql-expr ")" )
// TODO where do we get this into the various tables.?????

FQLQueryStatement : fqlExpressionList

fqlExpressionList : fqlExpression
    | fqlExpressionList fqlExpression;

// fql-expr = property-comp *( bool-op property-comp )

fqlExpression
   :
   fqlExpression TOK_AND fqlExpression
   {
      FQL_TRACE(("YACC: fqlExpression:TOK_AND fqlExpression\n"));
      globalParserState->statement->appendOperation(FQL_AND);
   }
   | fqlExpression TOK_OR fqlExpression
   {
      FQL_TRACE(("YACC: fqlExpression: TOK_OR fqlExpression\n"));
      globalParserState->statement->appendOperation(FQL_OR);
   }
   | TOK_LPAREN fqlExpression TOK_RPAREN
   {
      FQL_TRACE(("YACC:  fqlExpression: ( fqlExpression )\n"));
   }
   | propertyComparison
   {
      FQL_TRACE(("YACC: fqlExpression: propertyComparision\n"));
   }

;

propertyComparison
    : arrayProperty arrayCompOp arrayLiteral
    {
        FQL_TRACE(("YACC: propertyComparison :arrayProperty arrayCompOp"
                    "arrayLiteral\n"));
         //// Want to move arrayCompOp put to here also
    }

    | scalarProperty scalarCompOp scalarLiteral
    {
        FQL_TRACE(("YACC: propertyComparison : scalarProperty scalarCompOp"
                   " scalarLiteral\n"));
    }
    // resolve property type and requirement to match scalar to scalar
    // at runtime.
    // This is general and conflicts with scalar vs array but right now we
    // have no way to separate scalar vs. array.
    | property scalarCompOp property
    {
      FQL_TRACE(("YACC: propertyComparison : property scalarCompOp"
                 " property\n"));
    }

    | indexedArrayProperty scalarCompOp scalarLiteral
    {
      FQL_TRACE(("YACC: propertyComparison : arrayProperty [index] scalarCompOp"
                 " scalarLiteral\n"));
    }

    | indexedArrayProperty scalarCompOp scalarProperty
    {
      FQL_TRACE(("YACC: propertyComparison : arrayProperty [index] scalarCompOp"
                 " scalarLProperty\n"));
    }

    | indexedArrayProperty scalarCompOp indexedArrayProperty
    {
      FQL_TRACE(("YACC: propertyComparison : arrayProperty [index] scalarCompOp"
                 " arrayProperty [index]\n"));
    }

    | arrayOp arrayProperty scalarCompOp scalarLiteral
    {
       FQL_TRACE(("YACC: propertyComparison :arrayOp arrayProperty scalarCompOp"
                  " scalarLiteral\n"));
    }

    | arrayOp arrayProperty scalarCompOp scalarProperty
    {
       FQL_TRACE(("YACC: propertyComparison :arrayOp arrayProperty scalarCompOp"
                  " scalarProperty\n"));
    }

    | arrayOp arrayProperty scalarCompOp indexedArrayProperty
    {
       FQL_TRACE(("YACC: propertyComparison :arrayOp arrayProperty scalarCompOp"
                  " arrayProperty TOK_LBRKT index TOK_RBRKT\n"));
    }

    | scalarProperty likeOp likePattern
    {
       FQL_TRACE(("YACC: propertyComparison :scalarProperty likeOp"
                  " likePattern\n"));
    }

    | indexedArrayProperty likeOp likePattern
    {
       FQL_TRACE(("YACC: propertyComparison :arrayProperty TOK_LBRKT index"
                  " TOK_RBRKT likeOp likePattern\n"));
    }

    | arrayOp arrayProperty likeOp  likePattern
    {
       FQL_TRACE(("YACC: propertyComparison :arrayOp arrayProperty likeOp"
                  " likePattern\n"));
    }
;

//// indexedArrayProperty: TOK_IDENTIFIER TOK_LBRKT index TOK_RBRKT
//// TODO want to make this into a single call to appendIndexedProperty.
indexedArrayProperty: TOK_IDENTIFIER TOK_LBRKT index TOK_RBRKT
{
    //// yydebug = 1;
    FQL_TRACE(("YACC: property : TOK_IDENTIFIER TOK_LBRKT index TOK_RBRKT\n"));
    globalParserState->statement->appendOperand(
        FQLOperand($1, FQL_PROPERTY_NAME_TAG, $3));
}

arrayProperty: property
   {
      FQL_TRACE(("YACC: arrayProperty: property\n"));
   }
;

scalarProperty: property
   {
      FQL_TRACE(("YACC: scalarProperty: property\n"));
      //{ $$ = $1; }
   }
;

// ABNF like-pattern = like-literal
// TODO should we be limiting this. Right now TOK_STRING
// Specified in
//     http://www.dmtf.org/standards/published_documents/DSP1001_1.1.pdf
likePattern : TOK_STRING
   {
      FQL_TRACE(("YACC: likePattern: TOK_STRING\n"));
      globalParserState->statement->appendOperand(
           FQLOperand($1, FQL_STRING_VALUE_TAG));
   }
;

// property = property-name *( "." property-name )
// property-name is name of a property in a CIM Class
// todo expand to cover the second .property-name

;
property
    : TOK_IDENTIFIER
    {
          FQL_TRACE(("YACC: property:TOK_IDENTIFIER\n"));
          globalParserState->statement->appendOperand(
             FQLOperand($1, FQL_PROPERTY_NAME_TAG));
          globalParserState->statement->appendQueryPropertyName(CIMName($1));
    }
    | property TOK_DOT TOK_IDENTIFIER
    {
          FQL_TRACE(("YACC: property:TOK_IDENTIFIER\n"));
          globalParserState->statement->chainOperand(
             FQLOperand($3, FQL_PROPERTY_NAME_TAG));
    }
;


// ABNF scalar-comp-op: "=" / "<>" / "<" / ">" / "<=" / ">="

scalarCompOp
   : TOK_EQ
   {
      globalParserState->statement->appendOperation(FQL_EQ);
   }
   | TOK_NE
   {
      globalParserState->statement->appendOperation(FQL_NE);
   }
   | TOK_LT
   {
      globalParserState->statement->appendOperation(FQL_LT);
   }
   | TOK_GT
   {
      globalParserState->statement->appendOperation(FQL_GT);
   }
   | TOK_LE
   {
      globalParserState->statement->appendOperation(FQL_LE);
   }
   | TOK_GE
   {
      globalParserState->statement->appendOperation(FQL_GE);
   }
;

// ABNF array-comp-op = "=" / "<>"
arrayCompOp
   : TOK_EQ
   {
      globalParserState->statement->appendOperation(FQL_EQ);
   }
   | TOK_NE
   {
      globalParserState->statement->appendOperation(FQL_NE);
   }
;

// like-op = [NOT] LIKE
likeOp
    : TOK_LIKE
    {
        FQL_TRACE(("YACC: likeOp: TOK_LIKE\n"));
        globalParserState->statement->appendOperation(FQL_LIKE);
    }
    |
    TOK_NOT TOK_LIKE
    {
        FQL_TRACE(("YACC: likeOp: TOK_NOT TOK_LIKE\n"));

        globalParserState->statement->appendOperation(FQL_NOT_LIKE);
    }
;

// array-op = [NOT] ( ANY / EVERY )
arrayOp
    : TOK_ANY
    {
        FQL_TRACE(("YACC: arrayOp: TOK_ANY\n"));
        globalParserState->statement->appendOperation(FQL_ANY);
    }
    | TOK_EVERY
    {
        FQL_TRACE(("YACC: arrayOp: TOK_EVERY\n"));
        globalParserState->statement->appendOperation(FQL_EVERY);
    }
    | TOK_NOT TOK_ANY
    {
        FQL_TRACE(("YACC: arrayOp: TOK_NOT TOK_ANY\n"));
        globalParserState->statement->appendOperation(FQL_NOT_ANY);
    }
    | TOK_NOT TOK_EVERY
    {
        FQL_TRACE(("YACC: arrayOp: TOK_NOT TOK_EVERY\n"));
        globalParserState->statement->appendOperation(FQL_NOT_EVERY);
    }
;

// ABNF array-literal = "{" [scalar-literal *( "," scalar-literal ) ] "}"
arrayLiteral
    : TOK_LBRACE arrayLiteralList TOK_RBRACE
    {
        FQL_TRACE(("YACC: { arrayLiteralList }\n"));

        FQLOperand x = FQLOperand(_argList);
        globalParserState->statement->appendOperand(x);

        for (Uint32 i = 0; i < _argList.size(); i++)
        {
            delete _argList[i];
        }
        _argList.clear();
    }
;

arrayLiteralList
   : arrayLiteralEntry
   {
      FQL_TRACE(("YACC: arrayLiteralList : arrayLiteralEntry\n"));
   }
   | arrayLiteralList TOK_COMMA arrayLiteralEntry
   {
      FQL_TRACE(("YACC: arrayLiteralList: arrayLiteralList COMMA"
                 " arrayLiteralEntry\n"));
   }
;


arrayLiteralEntry
   : booleanLiteral
   {
      FQL_TRACE(("YACC: arrayLiteralEntry:booleanLiteral\n"));
         FQLOperand* _op = new FQLOperand($1, FQL_BOOLEAN_VALUE_TAG);
         _argList.append(_op);
   }
   | TOK_STRING
   {
      FQL_TRACE(("YACC: arrayLiteralEntry:TOK_STRING\n"));
      FQLOperand* _op = new FQLOperand($1, FQL_STRING_VALUE_TAG);
      // cout << "TOK_STRING in arrayLiteralEntry = " << $1 << endl;
      _argList.append(_op);
   }
   /// TODO Think this should be integerLiteral
   | TOK_INTEGER    // was integerLiteral
   {
      FQL_TRACE(("YACC: arrayLiteralEntry:TOK_INTEGER\n"));
      FQLOperand* _op = new FQLOperand($1, FQL_INTEGER_VALUE_TAG);
      _argList.append(_op);
   }
   // TODO why unsigned is separate here.
   | TOK_UNSIGNED_INTEGER    // was integerLiteral
   {
      FQL_TRACE(("YACC: arrayLiteralEntry:TOK_INTEGER\n"));
      FQLOperand* _op = new FQLOperand($1, FQL_INTEGER_VALUE_TAG);
      _argList.append(_op);
   }
   | TOK_DOUBLE
   {
      FQL_TRACE(("YACC: arrayLiteralEntry:TOK_DOUBLE\n"));
      FQLOperand* _op = new FQLOperand($1, FQL_DOUBLE_VALUE_TAG);
      _argList.append(_op);
   }
   // The following two should never occur because the lexer cannot parse
   // the difference with strings.
   | TOK_DATETIME
   {
      FQL_TRACE(("YACC: arrayLiteralEntry: dateTimeLiteral\n"));
      FQLOperand* _op = new FQLOperand($1, FQL_DATETIME_VALUE_TAG);
      _argList.append(_op);
   }

   | TOK_REFERENCE_LIT
   {
       FQL_TRACE(("YACC: scalarLiteral: referenceLiteral\n"));
       globalParserState->statement->appendOperand(
          FQLOperand($1, FQL_REFERENCE_VALUE_TAG));
   }
/// | TOK_NULL
/// {
///    //// TODO
/// }
;

index: TOK_INTEGER
{
      FQL_TRACE(("YACC: index:TOK_UNSIGNED_INTEGER\n"));
}
;

scalarLiteral
    : booleanLiteral
    {
        FQL_TRACE(("YACC: scalarLiteral:booleanLiteral\n"));
        globalParserState->statement->appendOperand(
           FQLOperand($1 != 0, FQL_BOOLEAN_VALUE_TAG));
    }
    | TOK_STRING
    {
        FQL_TRACE(("YACC: scalarLiteral:TOK_STRING\n"));
        globalParserState->statement->appendOperand(
          FQLOperand($1, FQL_STRING_VALUE_TAG));
    }
    /// TODO Think this should be integerLiteral
    | integerLiteral
    {
        FQL_TRACE(("YACC: scalarLiteral:TOK_INTEGER\n"));
        globalParserState->statement->appendOperand(
          FQLOperand($1, FQL_INTEGER_VALUE_TAG));
    }
    // TODO why unsigned is separate here.
    | TOK_UNSIGNED_INTEGER    // was integerLiteral
    {
        FQL_TRACE(("YACC: scalarLiteral:TOK_UNSIGNED_INTEGER\n"));
        globalParserState->statement->appendOperand(
          FQLOperand($1, FQL_INTEGER_VALUE_TAG));
    }
    | TOK_DOUBLE
    {
        FQL_TRACE(("YACC: scalarLiteral:TOK_DOUBLE\n"));
        globalParserState->statement->appendOperand(
          FQLOperand($1, FQL_DOUBLE_VALUE_TAG));
    }
    // The following two should never occur because the lexer
    // cannot parse them out of strings. The parser and evaluator will
    // separate DateTime and References from Strings based on the
    // comparison property type
    | TOK_DATETIME
    {
        FQL_TRACE(("YACC: scalarLiteral: DateTime literal\n"));

        globalParserState->statement->appendOperand(
          FQLOperand($1, FQL_DATETIME_VALUE_TAG));
    }

    | TOK_REFERENCE_LIT
    {
        FQL_TRACE(("YACC: scalarLiteral: Reference literal\n"));
          globalParserState->statement->appendOperand(
            FQLOperand(FQL_REFERENCE_VALUE_TAG));
    }
    | TOK_NULL
    {
      globalParserState->statement->appendOperand(
          FQLOperand(FQL_NULL_VALUE_TAG));
    }
;

// The following ABNF rules shall be interpreted to combine their terminals as
// stated, without implicitly inserting any whitespace characters.

//Some alphabetic characters shall be treated case insensitively, as stated.
// All other alphabetic characters shall be treated case sensitively.
// ABNF boolean-literal = TRUE | FALSE
booleanLiteral
    : TOK_TRUE
    {
        FQL_TRACE(("YACC: booleanLiteral:TOK_TRUE\n"));
        $$ = 1;
    }
    | TOK_FALSE
    {
        FQL_TRACE(("YACC: booleanLiteral:TOK_FALSE\n"));
        $$ = 0;
    }

///charEscape = "\" ( "\" | singleQuote | "b" | "t" | "n" | "f" | "r" /
///              "u" 4*6(hex-digit) )

integerLiteral
    : decimalLiteral
    { }
    | binaryLiteral
    {}
    | hexLiteral
    { }

// ABNF decimal-literal: [sign] unsigned-integer
//      unsigned-integer: 1*(decimal-digit)
decimalLiteral : TOK_INTEGER
    {
      $$ = $1;
    }

// This is defined in lex as easily as here.
// ABNF binary-literal: [sign] 1*(binary-digit) "B" ; case insensitive
binaryLiteral: TOK_BINARY {$$ = $1;}

// ABNF hex-literal = [sign] "0X" 1*( hex-digit hex-digit ) ; case insensitive
hexLiteral: TOK_HEXADECIMAL
    {
      $$ = $1;
    }

// The following are treated in the lexer.
// ABNF exact-numeric=  unsigned-integer "." [unsigned-integer] /
// ABNF               "." unsigned-integer
// ABNF               sign = "+" / "-"

// ABNF binary-digit: "0" / "1"

//ABNF decimal-digit: binary-digit / "2" / "3" / "4" / "5" / "6" / "7" / "8"
// / "9"

// ABNF hex-digit =  decimal-digit / "A" / "B" / "C" / "D" / "E" / "F"
//  ; case insensitive

%%
