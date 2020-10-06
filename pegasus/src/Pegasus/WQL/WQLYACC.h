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
     TOK_INTEGER = 258,
     TOK_DOUBLE = 259,
     TOK_STRING = 260,
     TOK_TRUE = 261,
     TOK_FALSE = 262,
     TOK_NULL = 263,
     TOK_ISA = 264,
     TOK_DOT = 265,
     TOK_EQ = 266,
     TOK_NE = 267,
     TOK_LT = 268,
     TOK_LE = 269,
     TOK_GT = 270,
     TOK_GE = 271,
     TOK_NOT = 272,
     TOK_OR = 273,
     TOK_AND = 274,
     TOK_IS = 275,
     TOK_IDENTIFIER = 276,
     TOK_SELECT = 277,
     TOK_WHERE = 278,
     TOK_FROM = 279,
     TOK_UNEXPECTED_CHAR = 280
   };
#endif
/* Tokens.  */
#define TOK_INTEGER 258
#define TOK_DOUBLE 259
#define TOK_STRING 260
#define TOK_TRUE 261
#define TOK_FALSE 262
#define TOK_NULL 263
#define TOK_ISA 264
#define TOK_DOT 265
#define TOK_EQ 266
#define TOK_NE 267
#define TOK_LT 268
#define TOK_LE 269
#define TOK_GT 270
#define TOK_GE 271
#define TOK_NOT 272
#define TOK_OR 273
#define TOK_AND 274
#define TOK_IS 275
#define TOK_IDENTIFIER 276
#define TOK_SELECT 277
#define TOK_WHERE 278
#define TOK_FROM 279
#define TOK_UNEXPECTED_CHAR 280




#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED
typedef union YYSTYPE
#line 87 "WQL.y"
{
   int intValue;
   double doubleValue;
   char* strValue;
   void* nodeValue;
}
/* Line 1489 of yacc.c.  */
#line 106 "WQLtemp.h"
	YYSTYPE;
# define yystype YYSTYPE /* obsolescent; will be withdrawn */
# define YYSTYPE_IS_DECLARED 1
# define YYSTYPE_IS_TRIVIAL 1
#endif

extern YYSTYPE WQL_lval;

