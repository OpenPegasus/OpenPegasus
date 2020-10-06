/* To be inserted in other files to bypass the chksrc tests */
/* NOCHKSRC */

/* A Bison parser, made by GNU Bison 2.3.  */

/* Skeleton interface for Bison's Yacc-like parsers in C

   Copyright (C) 1984, 1989, 1990, 2000, 2001, 2002, 2003, 2004, 2005, 2006
   Free Software Foundation, Inc.

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2, or (at your option)
   any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.  */

/* As a special exception, you may create a larger work that contains
   part or all of the Bison parser skeleton and distribute that work
   under terms of your choice, so long as that work isn't itself a
   parser generator using the skeleton or a modified version thereof
   as a parser skeleton.  Alternatively, if you modify or redistribute
   the parser skeleton itself, you may (at your option) remove this
   special exception, which will cause the skeleton and the resulting
   Bison output files to be licensed under the GNU General Public
   License without this special exception.

   This special exception was added by the Free Software Foundation in
   version 2.2 of Bison.  */

/* Tokens.  */
#ifndef YYTOKENTYPE
# define YYTOKENTYPE
   /* Put the tokens into the symbol table, so that GDB and other debuggers
      know about them.  */
   enum yytokentype {
     TOK_IDENTIFIER = 258,
     TOK_STRING_LITERAL = 259,
     TOK_BINARY = 260,
     TOK_NEGATIVE_BINARY = 261,
     TOK_HEXADECIMAL = 262,
     TOK_NEGATIVE_HEXADECIMAL = 263,
     TOK_INTEGER = 264,
     TOK_NEGATIVE_INTEGER = 265,
     TOK_REAL = 266,
     TOK_NEGATIVE_REAL = 267,
     TOK_TRUE = 268,
     TOK_FALSE = 269,
     TOK_SCOPED_PROPERTY = 270,
     TOK_LPAR = 271,
     TOK_RPAR = 272,
     TOK_HASH = 273,
     TOK_DOT = 274,
     TOK_LBRKT = 275,
     TOK_RBRKT = 276,
     TOK_UNDERSCORE = 277,
     TOK_COMMA = 278,
     TOK_CONCAT = 279,
     TOK_DBL_PIPE = 280,
     TOK_PLUS = 281,
     TOK_MINUS = 282,
     TOK_TIMES = 283,
     TOK_DIV = 284,
     TOK_IS = 285,
     TOK_NULL = 286,
     TOK_EQ = 287,
     TOK_NE = 288,
     TOK_GT = 289,
     TOK_LT = 290,
     TOK_GE = 291,
     TOK_LE = 292,
     TOK_ISA = 293,
     TOK_LIKE = 294,
     TOK_NOT = 295,
     TOK_AND = 296,
     TOK_OR = 297,
     TOK_SCOPE = 298,
     TOK_ANY = 299,
     TOK_EVERY = 300,
     TOK_IN = 301,
     TOK_SATISFIES = 302,
     TOK_STAR = 303,
     TOK_DOTDOT = 304,
     TOK_SHARP = 305,
     TOK_DISTINCT = 306,
     TOK_SELECT = 307,
     TOK_FIRST = 308,
     TOK_FROM = 309,
     TOK_WHERE = 310,
     TOK_ORDER = 311,
     TOK_BY = 312,
     TOK_ASC = 313,
     TOK_DESC = 314,
     TOK_AS = 315,
     TOK_UNEXPECTED_CHAR = 316
   };
#endif
/* Tokens.  */
#define TOK_IDENTIFIER 258
#define TOK_STRING_LITERAL 259
#define TOK_BINARY 260
#define TOK_NEGATIVE_BINARY 261
#define TOK_HEXADECIMAL 262
#define TOK_NEGATIVE_HEXADECIMAL 263
#define TOK_INTEGER 264
#define TOK_NEGATIVE_INTEGER 265
#define TOK_REAL 266
#define TOK_NEGATIVE_REAL 267
#define TOK_TRUE 268
#define TOK_FALSE 269
#define TOK_SCOPED_PROPERTY 270
#define TOK_LPAR 271
#define TOK_RPAR 272
#define TOK_HASH 273
#define TOK_DOT 274
#define TOK_LBRKT 275
#define TOK_RBRKT 276
#define TOK_UNDERSCORE 277
#define TOK_COMMA 278
#define TOK_CONCAT 279
#define TOK_DBL_PIPE 280
#define TOK_PLUS 281
#define TOK_MINUS 282
#define TOK_TIMES 283
#define TOK_DIV 284
#define TOK_IS 285
#define TOK_NULL 286
#define TOK_EQ 287
#define TOK_NE 288
#define TOK_GT 289
#define TOK_LT 290
#define TOK_GE 291
#define TOK_LE 292
#define TOK_ISA 293
#define TOK_LIKE 294
#define TOK_NOT 295
#define TOK_AND 296
#define TOK_OR 297
#define TOK_SCOPE 298
#define TOK_ANY 299
#define TOK_EVERY 300
#define TOK_IN 301
#define TOK_SATISFIES 302
#define TOK_STAR 303
#define TOK_DOTDOT 304
#define TOK_SHARP 305
#define TOK_DISTINCT 306
#define TOK_SELECT 307
#define TOK_FIRST 308
#define TOK_FROM 309
#define TOK_WHERE 310
#define TOK_ORDER 311
#define TOK_BY 312
#define TOK_ASC 313
#define TOK_DESC 314
#define TOK_AS 315
#define TOK_UNEXPECTED_CHAR 316




#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED
typedef union YYSTYPE
#line 133 "CQL.y"
{
    char * strValue;
    String * _string;
    CQLValue * _value;
    CQLIdentifier * _identifier;
    CQLChainedIdentifier * _chainedIdentifier;
    CQLPredicate * _predicate;
    ExpressionOpType _opType;
    void * _node;
}
/* Line 1489 of yacc.c.  */
#line 182 "CQLtemp.h"
	YYSTYPE;
# define yystype YYSTYPE /* obsolescent; will be withdrawn */
# define YYSTYPE_IS_DECLARED 1
# define YYSTYPE_IS_TRIVIAL 1
#endif

extern YYSTYPE CQL_lval;

