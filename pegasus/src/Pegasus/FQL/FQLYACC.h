/* To be inserted in other files to bypass the chksrc tests */
/* NOCHKSRC */

/* A Bison parser, made by GNU Bison 3.0.2.  */

/* Skeleton interface for Bison GLR parsers in C

   Copyright (C) 2002-2013 Free Software Foundation, Inc.

   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.  */

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

#ifndef YY_FQL_FQLYACC_H_INCLUDED
# define YY_FQL_FQLYACC_H_INCLUDED
/* Debug traces.  */
#ifndef YYDEBUG
# define YYDEBUG 0
#endif
#if YYDEBUG
extern int FQL_debug;
#endif

/* Token type.  */
#ifndef YYTOKENTYPE
# define YYTOKENTYPE
  enum yytokentype
  {
    TOK_INTEGER = 258,
    TOK_UNSIGNED_INTEGER = 259,
    TOK_DOUBLE = 260,
    TOK_HEXADECIMAL = 261,
    TOK_BINARY = 262,
    TOK_STRING = 263,
    TOK_DATETIME = 264,
    TOK_TRUE = 265,
    TOK_FALSE = 266,
    TOK_NULL = 267,
    TOK_ISA = 268,
    TOK_DOT = 269,
    TOK_EQ = 270,
    TOK_NE = 271,
    TOK_LT = 272,
    TOK_LE = 273,
    TOK_GT = 274,
    TOK_GE = 275,
    TOK_LIKE = 276,
    TOK_NOT = 277,
    TOK_OR = 278,
    TOK_AND = 279,
    TOK_IS = 280,
    TOK_ANY = 281,
    TOK_EVERY = 282,
    TOK_IDENTIFIER = 283,
    TOK_REFERENCE_LIT = 284,
    TOK_LBRKT = 285,
    TOK_RBRKT = 286,
    TOK_LPAREN = 287,
    TOK_RPAREN = 288,
    TOK_LBRACE = 289,
    TOK_RBRACE = 290,
    TOK_COMMA = 291,
    TOK_UNEXPECTED_CHAR = 292
  };
#endif

/* Value type.  */
#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED
typedef union YYSTYPE YYSTYPE;
union YYSTYPE
{
#line 112 "FQL.y" /* glr.c:2555  */

   int intValue;
   double doubleValue;
   char* strValue;
   void* nodeValue;

#line 99 "FQLYACC.h" /* glr.c:2555  */
};
# define YYSTYPE_IS_TRIVIAL 1
# define YYSTYPE_IS_DECLARED 1
#endif


extern YYSTYPE FQL_lval;

int FQL_parse (void);

#endif /* !YY_FQL_FQLYACC_H_INCLUDED  */
