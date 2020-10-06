/* To be inserted in other files to bypass the chksrc tests */
/* NOCHKSRC */

/* A Bison parser, made by GNU Bison 2.3.  */

/* Skeleton implementation for Bison's Yacc-like parsers in C

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

/* C LALR(1) parser skeleton written by Richard Stallman, by
   simplifying the original so-called "semantic" parser.  */

/* All symbols defined below should begin with yy or YY, to avoid
   infringing on user name space.  This should be done even for local
   variables, as they might otherwise be expanded by user macros.
   There are some unavoidable exceptions within include files to
   define necessary library symbols; they are noted "INFRINGES ON
   USER NAME SPACE" below.  */

/* Identify Bison output.  */
#define YYBISON 1

/* Bison version.  */
#define YYBISON_VERSION "2.3"

/* Skeleton name.  */
#define YYSKELETON_NAME "yacc.c"

/* Pure parsers.  */
#define YYPURE 0

/* Using locations.  */
#define YYLSP_NEEDED 0

/* Substitute the variable and function names.  */
#define yyparse CQL_parse
#define yylex   CQL_lex
#define yyerror CQL_error
#define yylval  CQL_lval
#define yychar  CQL_char
#define yydebug CQL_debug
#define yynerrs CQL_nerrs


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




/* Copy the first part of user declarations.  */
#line 29 "CQL.y"

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



/* Enabling traces.  */
#ifndef YYDEBUG
# define YYDEBUG 0
#endif

/* Enabling verbose error messages.  */
#ifdef YYERROR_VERBOSE
# undef YYERROR_VERBOSE
# define YYERROR_VERBOSE 1
#else
# define YYERROR_VERBOSE 0
#endif

/* Enabling the token table.  */
#ifndef YYTOKEN_TABLE
# define YYTOKEN_TABLE 0
#endif

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
/* Line 187 of yacc.c.  */
#line 342 "CQLtemp"
	YYSTYPE;
# define yystype YYSTYPE /* obsolescent; will be withdrawn */
# define YYSTYPE_IS_DECLARED 1
# define YYSTYPE_IS_TRIVIAL 1
#endif



/* Copy the second part of user declarations.  */


/* Line 216 of yacc.c.  */
#line 355 "CQLtemp"

#ifdef short
# undef short
#endif

#ifdef YYTYPE_UINT8
typedef YYTYPE_UINT8 yytype_uint8;
#else
typedef unsigned char yytype_uint8;
#endif

#ifdef YYTYPE_INT8
typedef YYTYPE_INT8 yytype_int8;
#elif (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
typedef signed char yytype_int8;
#else
typedef short int yytype_int8;
#endif

#ifdef YYTYPE_UINT16
typedef YYTYPE_UINT16 yytype_uint16;
#else
typedef unsigned short int yytype_uint16;
#endif

#ifdef YYTYPE_INT16
typedef YYTYPE_INT16 yytype_int16;
#else
typedef short int yytype_int16;
#endif

#ifndef YYSIZE_T
# ifdef __SIZE_TYPE__
#  define YYSIZE_T __SIZE_TYPE__
# elif defined size_t
#  define YYSIZE_T size_t
# elif ! defined YYSIZE_T && (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
#  include <stddef.h> /* INFRINGES ON USER NAME SPACE */
#  define YYSIZE_T size_t
# else
#  define YYSIZE_T unsigned int
# endif
#endif

#define YYSIZE_MAXIMUM ((YYSIZE_T) -1)

#ifndef YY_
# if YYENABLE_NLS
#  if ENABLE_NLS
#   include <libintl.h> /* INFRINGES ON USER NAME SPACE */
#   define YY_(msgid) dgettext ("bison-runtime", msgid)
#  endif
# endif
# ifndef YY_
#  define YY_(msgid) msgid
# endif
#endif

/* Suppress unused-variable warnings by "using" E.  */
#if ! defined lint || defined __GNUC__
# define YYUSE(e) ((void) (e))
#else
# define YYUSE(e) /* empty */
#endif

/* Identity function, used to suppress warnings about constant conditions.  */
#ifndef lint
# define YYID(n) (n)
#else
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static int
YYID (int i)
#else
static int
YYID (i)
    int i;
#endif
{
  return i;
}
#endif

#if ! defined yyoverflow || YYERROR_VERBOSE

/* The parser invokes alloca or malloc; define the necessary symbols.  */

# ifdef YYSTACK_USE_ALLOCA
#  if YYSTACK_USE_ALLOCA
#   ifdef __GNUC__
#    define YYSTACK_ALLOC __builtin_alloca
#   elif defined __BUILTIN_VA_ARG_INCR
#    include <alloca.h> /* INFRINGES ON USER NAME SPACE */
#   elif defined _AIX
#    define YYSTACK_ALLOC __alloca
#   elif defined _MSC_VER
#    include <malloc.h> /* INFRINGES ON USER NAME SPACE */
#    define alloca _alloca
#   else
#    define YYSTACK_ALLOC alloca
#    if ! defined _ALLOCA_H && ! defined _STDLIB_H && (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
#     include <stdlib.h> /* INFRINGES ON USER NAME SPACE */
#     ifndef _STDLIB_H
#      define _STDLIB_H 1
#     endif
#    endif
#   endif
#  endif
# endif

# ifdef YYSTACK_ALLOC
   /* Pacify GCC's `empty if-body' warning.  */
#  define YYSTACK_FREE(Ptr) do { /* empty */; } while (YYID (0))
#  ifndef YYSTACK_ALLOC_MAXIMUM
    /* The OS might guarantee only one guard page at the bottom of the stack,
       and a page size can be as small as 4096 bytes.  So we cannot safely
       invoke alloca (N) if N exceeds 4096.  Use a slightly smaller number
       to allow for a few compiler-allocated temporary stack slots.  */
#   define YYSTACK_ALLOC_MAXIMUM 4032 /* reasonable circa 2006 */
#  endif
# else
#  define YYSTACK_ALLOC YYMALLOC
#  define YYSTACK_FREE YYFREE
#  ifndef YYSTACK_ALLOC_MAXIMUM
#   define YYSTACK_ALLOC_MAXIMUM YYSIZE_MAXIMUM
#  endif
#  if (defined __cplusplus && ! defined _STDLIB_H \
       && ! ((defined YYMALLOC || defined malloc) \
	     && (defined YYFREE || defined free)))
#   include <stdlib.h> /* INFRINGES ON USER NAME SPACE */
#   ifndef _STDLIB_H
#    define _STDLIB_H 1
#   endif
#  endif
#  ifndef YYMALLOC
#   define YYMALLOC malloc
#   if ! defined malloc && ! defined _STDLIB_H && (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
void *malloc (YYSIZE_T); /* INFRINGES ON USER NAME SPACE */
#   endif
#  endif
#  ifndef YYFREE
#   define YYFREE free
#   if ! defined free && ! defined _STDLIB_H && (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
void free (void *); /* INFRINGES ON USER NAME SPACE */
#   endif
#  endif
# endif
#endif /* ! defined yyoverflow || YYERROR_VERBOSE */


#if (! defined yyoverflow \
     && (! defined __cplusplus \
	 || (defined YYSTYPE_IS_TRIVIAL && YYSTYPE_IS_TRIVIAL)))

/* A type that is properly aligned for any stack member.  */
union yyalloc
{
  yytype_int16 yyss;
  YYSTYPE yyvs;
  };

/* The size of the maximum gap between one aligned stack and the next.  */
# define YYSTACK_GAP_MAXIMUM (sizeof (union yyalloc) - 1)

/* The size of an array large to enough to hold all stacks, each with
   N elements.  */
# define YYSTACK_BYTES(N) \
     ((N) * (sizeof (yytype_int16) + sizeof (YYSTYPE)) \
      + YYSTACK_GAP_MAXIMUM)

/* Copy COUNT objects from FROM to TO.  The source and destination do
   not overlap.  */
# ifndef YYCOPY
#  if defined __GNUC__ && 1 < __GNUC__
#   define YYCOPY(To, From, Count) \
      __builtin_memcpy (To, From, (Count) * sizeof (*(From)))
#  else
#   define YYCOPY(To, From, Count)		\
      do					\
	{					\
	  YYSIZE_T yyi;				\
	  for (yyi = 0; yyi < (Count); yyi++)	\
	    (To)[yyi] = (From)[yyi];		\
	}					\
      while (YYID (0))
#  endif
# endif

/* Relocate STACK from its old location to the new one.  The
   local variables YYSIZE and YYSTACKSIZE give the old and new number of
   elements in the stack, and YYPTR gives the new location of the
   stack.  Advance YYPTR to a properly aligned location for the next
   stack.  */
# define YYSTACK_RELOCATE(Stack)					\
    do									\
      {									\
	YYSIZE_T yynewbytes;						\
	YYCOPY (&yyptr->Stack, Stack, yysize);				\
	Stack = &yyptr->Stack;						\
	yynewbytes = yystacksize * sizeof (*Stack) + YYSTACK_GAP_MAXIMUM; \
	yyptr += yynewbytes / sizeof (*yyptr);				\
      }									\
    while (YYID (0))

#endif

/* YYFINAL -- State number of the termination state.  */
#define YYFINAL  41
/* YYLAST -- Last index in YYTABLE.  */
#define YYLAST   140

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  62
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  35
/* YYNRULES -- Number of rules.  */
#define YYNRULES  77
/* YYNRULES -- Number of states.  */
#define YYNSTATES  108

/* YYTRANSLATE(YYLEX) -- Bison symbol number corresponding to YYLEX.  */
#define YYUNDEFTOK  2
#define YYMAXUTOK   316

#define YYTRANSLATE(YYX)						\
  ((unsigned int) (YYX) <= YYMAXUTOK ? yytranslate[YYX] : YYUNDEFTOK)

/* YYTRANSLATE[YYLEX] -- Bison symbol number corresponding to YYLEX.  */
static const yytype_uint8 yytranslate[] =
{
       0,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     1,     2,     3,     4,
       5,     6,     7,     8,     9,    10,    11,    12,    13,    14,
      15,    16,    17,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    27,    28,    29,    30,    31,    32,    33,    34,
      35,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    49,    50,    51,    52,    53,    54,
      55,    56,    57,    58,    59,    60,    61
};

#if YYDEBUG
/* YYPRHS[YYN] -- Index of the first RHS symbol of rule number YYN in
   YYRHS.  */
static const yytype_uint8 yyprhs[] =
{
       0,     0,     3,     5,     7,     9,    11,    13,    15,    17,
      19,    21,    23,    25,    27,    29,    31,    33,    35,    37,
      39,    41,    43,    45,    47,    49,    53,    55,    59,    61,
      66,    70,    74,    80,    85,    87,    91,    93,    95,    97,
     100,   102,   104,   106,   108,   110,   112,   114,   116,   118,
     123,   127,   131,   135,   139,   143,   145,   148,   150,   154,
     156,   160,   161,   163,   165,   169,   172,   174,   176,   180,
     182,   184,   187,   188,   192,   194,   195,   198
};

/* YYRHS -- A `-1'-separated list of the rules' RHS.  */
static const yytype_int8 yyrhs[] =
{
      96,     0,    -1,     3,    -1,    63,    -1,    64,    -1,    15,
      -1,     4,    -1,     5,    -1,     6,    -1,     7,    -1,     8,
      -1,     9,    -1,    10,    -1,    11,    -1,    12,    -1,    67,
      -1,    70,    -1,    68,    -1,    69,    -1,    71,    -1,    13,
      -1,    14,    -1,    86,    -1,    73,    -1,    72,    -1,    16,
      86,    17,    -1,    63,    -1,    63,    18,    67,    -1,    66,
      -1,    63,    16,    87,    17,    -1,    75,    19,    66,    -1,
      75,    19,    63,    -1,    75,    19,    63,    18,    67,    -1,
      75,    20,    74,    21,    -1,    75,    -1,    76,    25,    67,
      -1,    76,    -1,    77,    -1,    78,    -1,    18,    67,    -1,
      79,    -1,    80,    -1,    32,    -1,    33,    -1,    34,    -1,
      35,    -1,    36,    -1,    37,    -1,    79,    -1,    79,    30,
      40,    31,    -1,    79,    30,    31,    -1,    79,    82,    81,
      -1,    80,    82,    79,    -1,    79,    38,    63,    -1,    79,
      39,    67,    -1,    83,    -1,    40,    83,    -1,    84,    -1,
      85,    41,    84,    -1,    85,    -1,    86,    42,    85,    -1,
      -1,    86,    -1,    65,    -1,    65,    60,    63,    -1,    65,
      63,    -1,    88,    -1,    48,    -1,    75,    19,    48,    -1,
      86,    -1,    90,    -1,    91,    93,    -1,    -1,    23,    91,
      93,    -1,    86,    -1,    -1,    55,    94,    -1,    52,    92,
      54,    89,    95,    -1
};

/* YYRLINE[YYN] -- source line where rule number YYN was defined.  */
static const yytype_uint16 yyrline[] =
{
       0,   259,   259,   285,   294,   310,   343,   372,   382,   396,
     406,   420,   430,   444,   454,   467,   476,   481,   486,   491,
     496,   506,   519,   533,   543,   552,   561,   570,   583,   592,
     603,   645,   683,   729,   787,   795,   884,   915,   944,   975,
     990,   997,  1009,  1016,  1022,  1029,  1035,  1041,  1049,  1056,
    1068,  1079,  1109,  1139,  1156,  1172,  1179,  1189,  1196,  1211,
    1218,  1232,  1251,  1310,  1317,  1329,  1342,  1349,  1358,  1375,
    1397,  1406,  1415,  1417,  1427,  1436,  1438,  1449
};
#endif

#if YYDEBUG || YYERROR_VERBOSE || YYTOKEN_TABLE
/* YYTNAME[SYMBOL-NUM] -- String name of the symbol SYMBOL-NUM.
   First, the terminals, then, starting at YYNTOKENS, nonterminals.  */
static const char *const yytname[] =
{
  "$end", "error", "$undefined", "TOK_IDENTIFIER", "TOK_STRING_LITERAL",
  "TOK_BINARY", "TOK_NEGATIVE_BINARY", "TOK_HEXADECIMAL",
  "TOK_NEGATIVE_HEXADECIMAL", "TOK_INTEGER", "TOK_NEGATIVE_INTEGER",
  "TOK_REAL", "TOK_NEGATIVE_REAL", "TOK_TRUE", "TOK_FALSE",
  "TOK_SCOPED_PROPERTY", "TOK_LPAR", "TOK_RPAR", "TOK_HASH", "TOK_DOT",
  "TOK_LBRKT", "TOK_RBRKT", "TOK_UNDERSCORE", "TOK_COMMA", "TOK_CONCAT",
  "TOK_DBL_PIPE", "TOK_PLUS", "TOK_MINUS", "TOK_TIMES", "TOK_DIV",
  "TOK_IS", "TOK_NULL", "TOK_EQ", "TOK_NE", "TOK_GT", "TOK_LT", "TOK_GE",
  "TOK_LE", "TOK_ISA", "TOK_LIKE", "TOK_NOT", "TOK_AND", "TOK_OR",
  "TOK_SCOPE", "TOK_ANY", "TOK_EVERY", "TOK_IN", "TOK_SATISFIES",
  "TOK_STAR", "TOK_DOTDOT", "TOK_SHARP", "TOK_DISTINCT", "TOK_SELECT",
  "TOK_FIRST", "TOK_FROM", "TOK_WHERE", "TOK_ORDER", "TOK_BY", "TOK_ASC",
  "TOK_DESC", "TOK_AS", "TOK_UNEXPECTED_CHAR", "$accept", "identifier",
  "class_name", "class_path", "scoped_property", "literal_string",
  "binary_value", "hex_value", "decimal_value", "real_value", "literal",
  "array_index", "array_index_list", "chain", "concat", "factor", "term",
  "arith", "value_symbol", "arith_or_value_symbol", "comp_op", "comp",
  "expr_factor", "expr_term", "expr", "arg_list", "from_specifier",
  "from_criteria", "star_expr", "selected_entry", "select_list",
  "select_list_tail", "search_condition", "optional_where",
  "select_statement", 0
};
#endif

# ifdef YYPRINT
/* YYTOKNUM[YYLEX-NUM] -- Internal token number corresponding to
   token YYLEX-NUM.  */
static const yytype_uint16 yytoknum[] =
{
       0,   256,   257,   258,   259,   260,   261,   262,   263,   264,
     265,   266,   267,   268,   269,   270,   271,   272,   273,   274,
     275,   276,   277,   278,   279,   280,   281,   282,   283,   284,
     285,   286,   287,   288,   289,   290,   291,   292,   293,   294,
     295,   296,   297,   298,   299,   300,   301,   302,   303,   304,
     305,   306,   307,   308,   309,   310,   311,   312,   313,   314,
     315,   316
};
# endif

/* YYR1[YYN] -- Symbol number of symbol that rule YYN derives.  */
static const yytype_uint8 yyr1[] =
{
       0,    62,    63,    64,    65,    66,    67,    68,    68,    69,
      69,    70,    70,    71,    71,    72,    72,    72,    72,    72,
      72,    72,    73,    74,    75,    75,    75,    75,    75,    75,
      75,    75,    75,    75,    76,    76,    77,    78,    79,    80,
      81,    81,    82,    82,    82,    82,    82,    82,    83,    83,
      83,    83,    83,    83,    83,    84,    84,    85,    85,    86,
      86,    87,    87,    88,    88,    88,    89,    90,    90,    91,
      91,    92,    93,    93,    94,    95,    95,    96
};

/* YYR2[YYN] -- Number of symbols composing right hand side of rule YYN.  */
static const yytype_uint8 yyr2[] =
{
       0,     2,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     3,     1,     3,     1,     4,
       3,     3,     5,     4,     1,     3,     1,     1,     1,     2,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     4,
       3,     3,     3,     3,     3,     1,     2,     1,     3,     1,
       3,     0,     1,     1,     3,     2,     1,     1,     3,     1,
       1,     2,     0,     3,     1,     0,     2,     5
};

/* YYDEFACT[STATE-NAME] -- Default rule to reduce with in state
   STATE-NUM when YYTABLE doesn't specify something else to do.  Zero
   means the default is an error.  */
static const yytype_uint8 yydefact[] =
{
       0,     0,     0,     2,     6,     7,     8,     9,    10,    11,
      12,    13,    14,    20,    21,     5,     0,     0,     0,    67,
      26,    28,    15,    17,    18,    16,    19,    24,    34,    36,
      37,    38,    48,     0,    55,    57,    59,    69,    70,    72,
       0,     1,    34,     0,    39,    56,    61,     0,     0,     0,
       0,     0,    42,    43,    44,    45,    46,    47,     0,     0,
       0,     0,     0,     0,     0,    71,     0,     0,    25,    62,
       0,    27,    68,    31,    30,    23,     0,    22,    35,    50,
       0,    53,    54,    40,    41,    51,    52,    58,    60,    72,
       3,     4,    63,    66,    75,    29,     0,    33,    49,    73,
       0,    65,     0,    77,    32,    64,    74,    76
};

/* YYDEFGOTO[NTERM-NUM].  */
static const yytype_int8 yydefgoto[] =
{
      -1,    20,    91,    92,    21,    22,    23,    24,    25,    26,
      27,    75,    76,    42,    29,    30,    31,    32,    33,    85,
      60,    34,    35,    36,    37,    70,    93,    94,    38,    39,
      40,    65,   107,   103,     2
};

/* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
   STATE-NUM.  */
#define YYPACT_NINF -46
static const yytype_int8 yypact[] =
{
     -36,    62,    11,   -46,   -46,   -46,   -46,   -46,   -46,   -46,
     -46,   -46,   -46,   -46,   -46,   -46,    85,    15,   108,   -46,
      -6,   -46,   -46,   -46,   -46,   -46,   -46,   -46,    -5,    20,
     -46,   -46,     4,    72,   -46,   -46,   -12,   -17,   -46,    23,
      -3,   -46,     8,   -11,   -46,   -46,    85,    15,     2,    85,
      15,   -22,   -46,   -46,   -46,   -46,   -46,   -46,    45,    15,
     108,   124,    85,    85,    62,   -46,    45,     5,   -46,   -17,
      32,   -46,   -46,    34,   -46,   -46,    33,   -17,   -46,   -46,
      22,   -46,   -46,   -46,   -46,   -46,   -46,   -46,   -12,    23,
     -46,   -46,     1,   -46,     3,   -46,    15,   -46,   -46,   -46,
      45,   -46,    85,   -46,   -46,   -46,   -17,   -46
};

/* YYPGOTO[NTERM-NUM].  */
static const yytype_int8 yypgoto[] =
{
     -46,   -45,   -46,   -46,   -41,   -15,   -46,   -46,   -46,   -46,
     -46,   -46,   -46,     0,   -46,   -46,   -46,   -37,    -4,   -46,
      24,    41,    -2,    -1,   -16,   -46,   -46,   -46,   -46,    18,
     -46,   -26,   -46,   -46,   -46
};

/* YYTABLE[YYPACT[STATE-NUM]].  What to do in state STATE-NUM.  If
   positive, shift that token.  If negative, reduce the rule which
   number is the opposite.  If zero, do what YYDEFACT says.
   If YYTABLE_NINF, syntax error.  */
#define YYTABLE_NINF -1
static const yytype_uint8 yytable[] =
{
      43,    28,    44,    73,     3,     3,    68,    74,     3,    79,
      46,    41,    47,    81,    48,    49,     1,    15,    80,     4,
      15,    90,    73,    83,    86,    63,    74,    67,    49,    62,
      69,    63,    71,    77,    51,    78,    52,    53,    54,    55,
      56,    57,    58,    59,    82,    50,    64,   101,     3,    95,
      72,    66,    96,    98,    97,   105,    84,    61,   102,    45,
      87,   100,    88,    99,    28,     3,     4,     5,     6,     7,
       8,     9,    10,    11,    12,    13,    14,    15,    16,     0,
      17,   104,    89,     0,     0,     0,   106,     0,     3,     4,
       5,     6,     7,     8,     9,    10,    11,    12,    13,    14,
      15,    16,    18,    17,    52,    53,    54,    55,    56,    57,
      19,     3,     4,     5,     6,     7,     8,     9,    10,    11,
      12,    13,    14,    15,    16,    18,    17,     3,     4,     5,
       6,     7,     8,     9,    10,    11,    12,    13,    14,    15,
      16
};

static const yytype_int8 yycheck[] =
{
      16,     1,    17,    48,     3,     3,    17,    48,     3,    31,
      16,     0,    18,    58,    19,    20,    52,    15,    40,     4,
      15,    66,    67,    60,    61,    42,    67,    19,    20,    41,
      46,    42,    47,    49,    30,    50,    32,    33,    34,    35,
      36,    37,    38,    39,    59,    25,    23,    92,     3,    17,
      48,    54,    18,    31,    21,   100,    60,    33,    55,    18,
      62,    60,    63,    89,    64,     3,     4,     5,     6,     7,
       8,     9,    10,    11,    12,    13,    14,    15,    16,    -1,
      18,    96,    64,    -1,    -1,    -1,   102,    -1,     3,     4,
       5,     6,     7,     8,     9,    10,    11,    12,    13,    14,
      15,    16,    40,    18,    32,    33,    34,    35,    36,    37,
      48,     3,     4,     5,     6,     7,     8,     9,    10,    11,
      12,    13,    14,    15,    16,    40,    18,     3,     4,     5,
       6,     7,     8,     9,    10,    11,    12,    13,    14,    15,
      16
};

/* YYSTOS[STATE-NUM] -- The (internal number of the) accessing
   symbol of state STATE-NUM.  */
static const yytype_uint8 yystos[] =
{
       0,    52,    96,     3,     4,     5,     6,     7,     8,     9,
      10,    11,    12,    13,    14,    15,    16,    18,    40,    48,
      63,    66,    67,    68,    69,    70,    71,    72,    75,    76,
      77,    78,    79,    80,    83,    84,    85,    86,    90,    91,
      92,     0,    75,    86,    67,    83,    16,    18,    19,    20,
      25,    30,    32,    33,    34,    35,    36,    37,    38,    39,
      82,    82,    41,    42,    23,    93,    54,    19,    17,    86,
      87,    67,    48,    63,    66,    73,    74,    86,    67,    31,
      40,    63,    67,    79,    80,    81,    79,    84,    85,    91,
      63,    64,    65,    88,    89,    17,    18,    21,    31,    93,
      60,    63,    55,    95,    67,    63,    86,    94
};

#define yyerrok		(yyerrstatus = 0)
#define yyclearin	(yychar = YYEMPTY)
#define YYEMPTY		(-2)
#define YYEOF		0

#define YYACCEPT	goto yyacceptlab
#define YYABORT		goto yyabortlab
#define YYERROR		goto yyerrorlab


/* Like YYERROR except do call yyerror.  This remains here temporarily
   to ease the transition to the new meaning of YYERROR, for GCC.
   Once GCC version 2 has supplanted version 1, this can go.  */

#define YYFAIL		goto yyerrlab

#define YYRECOVERING()  (!!yyerrstatus)

#define YYBACKUP(Token, Value)					\
do								\
  if (yychar == YYEMPTY && yylen == 1)				\
    {								\
      yychar = (Token);						\
      yylval = (Value);						\
      yytoken = YYTRANSLATE (yychar);				\
      YYPOPSTACK (1);						\
      goto yybackup;						\
    }								\
  else								\
    {								\
      yyerror (YY_("syntax error: cannot back up")); \
      YYERROR;							\
    }								\
while (YYID (0))


#define YYTERROR	1
#define YYERRCODE	256


/* YYLLOC_DEFAULT -- Set CURRENT to span from RHS[1] to RHS[N].
   If N is 0, then set CURRENT to the empty location which ends
   the previous symbol: RHS[0] (always defined).  */

#define YYRHSLOC(Rhs, K) ((Rhs)[K])
#ifndef YYLLOC_DEFAULT
# define YYLLOC_DEFAULT(Current, Rhs, N)				\
    do									\
      if (YYID (N))                                                    \
	{								\
	  (Current).first_line   = YYRHSLOC (Rhs, 1).first_line;	\
	  (Current).first_column = YYRHSLOC (Rhs, 1).first_column;	\
	  (Current).last_line    = YYRHSLOC (Rhs, N).last_line;		\
	  (Current).last_column  = YYRHSLOC (Rhs, N).last_column;	\
	}								\
      else								\
	{								\
	  (Current).first_line   = (Current).last_line   =		\
	    YYRHSLOC (Rhs, 0).last_line;				\
	  (Current).first_column = (Current).last_column =		\
	    YYRHSLOC (Rhs, 0).last_column;				\
	}								\
    while (YYID (0))
#endif


/* YY_LOCATION_PRINT -- Print the location on the stream.
   This macro was not mandated originally: define only if we know
   we won't break user code: when these are the locations we know.  */

#ifndef YY_LOCATION_PRINT
# if YYLTYPE_IS_TRIVIAL
#  define YY_LOCATION_PRINT(File, Loc)			\
     fprintf (File, "%d.%d-%d.%d",			\
	      (Loc).first_line, (Loc).first_column,	\
	      (Loc).last_line,  (Loc).last_column)
# else
#  define YY_LOCATION_PRINT(File, Loc) ((void) 0)
# endif
#endif


/* YYLEX -- calling `yylex' with the right arguments.  */

#ifdef YYLEX_PARAM
# define YYLEX yylex (YYLEX_PARAM)
#else
# define YYLEX yylex ()
#endif

/* Enable debugging if requested.  */
#if YYDEBUG

# ifndef YYFPRINTF
#  include <stdio.h> /* INFRINGES ON USER NAME SPACE */
#  define YYFPRINTF fprintf
# endif

# define YYDPRINTF(Args)			\
do {						\
  if (yydebug)					\
    YYFPRINTF Args;				\
} while (YYID (0))

# define YY_SYMBOL_PRINT(Title, Type, Value, Location)			  \
do {									  \
  if (yydebug)								  \
    {									  \
      YYFPRINTF (stderr, "%s ", Title);					  \
      yy_symbol_print (stderr,						  \
		  Type, Value); \
      YYFPRINTF (stderr, "\n");						  \
    }									  \
} while (YYID (0))


/*--------------------------------.
| Print this symbol on YYOUTPUT.  |
`--------------------------------*/

/*ARGSUSED*/
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static void
yy_symbol_value_print (FILE *yyoutput, int yytype, YYSTYPE const * const yyvaluep)
#else
static void
yy_symbol_value_print (yyoutput, yytype, yyvaluep)
    FILE *yyoutput;
    int yytype;
    YYSTYPE const * const yyvaluep;
#endif
{
  if (!yyvaluep)
    return;
# ifdef YYPRINT
  if (yytype < YYNTOKENS)
    YYPRINT (yyoutput, yytoknum[yytype], *yyvaluep);
# else
  YYUSE (yyoutput);
# endif
  switch (yytype)
    {
      default:
	break;
    }
}


/*--------------------------------.
| Print this symbol on YYOUTPUT.  |
`--------------------------------*/

#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static void
yy_symbol_print (FILE *yyoutput, int yytype, YYSTYPE const * const yyvaluep)
#else
static void
yy_symbol_print (yyoutput, yytype, yyvaluep)
    FILE *yyoutput;
    int yytype;
    YYSTYPE const * const yyvaluep;
#endif
{
  if (yytype < YYNTOKENS)
    YYFPRINTF (yyoutput, "token %s (", yytname[yytype]);
  else
    YYFPRINTF (yyoutput, "nterm %s (", yytname[yytype]);

  yy_symbol_value_print (yyoutput, yytype, yyvaluep);
  YYFPRINTF (yyoutput, ")");
}

/*------------------------------------------------------------------.
| yy_stack_print -- Print the state stack from its BOTTOM up to its |
| TOP (included).                                                   |
`------------------------------------------------------------------*/

#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static void
yy_stack_print (yytype_int16 *bottom, yytype_int16 *top)
#else
static void
yy_stack_print (bottom, top)
    yytype_int16 *bottom;
    yytype_int16 *top;
#endif
{
  YYFPRINTF (stderr, "Stack now");
  for (; bottom <= top; ++bottom)
    YYFPRINTF (stderr, " %d", *bottom);
  YYFPRINTF (stderr, "\n");
}

# define YY_STACK_PRINT(Bottom, Top)				\
do {								\
  if (yydebug)							\
    yy_stack_print ((Bottom), (Top));				\
} while (YYID (0))


/*------------------------------------------------.
| Report that the YYRULE is going to be reduced.  |
`------------------------------------------------*/

#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static void
yy_reduce_print (YYSTYPE *yyvsp, int yyrule)
#else
static void
yy_reduce_print (yyvsp, yyrule)
    YYSTYPE *yyvsp;
    int yyrule;
#endif
{
  int yynrhs = yyr2[yyrule];
  int yyi;
  unsigned long int yylno = yyrline[yyrule];
  YYFPRINTF (stderr, "Reducing stack by rule %d (line %lu):\n",
	     yyrule - 1, yylno);
  /* The symbols being reduced.  */
  for (yyi = 0; yyi < yynrhs; yyi++)
    {
      fprintf (stderr, "   $%d = ", yyi + 1);
      yy_symbol_print (stderr, yyrhs[yyprhs[yyrule] + yyi],
		       &(yyvsp[(yyi + 1) - (yynrhs)])
		       		       );
      fprintf (stderr, "\n");
    }
}

# define YY_REDUCE_PRINT(Rule)		\
do {					\
  if (yydebug)				\
    yy_reduce_print (yyvsp, Rule); \
} while (YYID (0))

/* Nonzero means print parse trace.  It is left uninitialized so that
   multiple parsers can coexist.  */
int yydebug;
#else /* !YYDEBUG */
# define YYDPRINTF(Args)
# define YY_SYMBOL_PRINT(Title, Type, Value, Location)
# define YY_STACK_PRINT(Bottom, Top)
# define YY_REDUCE_PRINT(Rule)
#endif /* !YYDEBUG */


/* YYINITDEPTH -- initial size of the parser's stacks.  */
#ifndef	YYINITDEPTH
# define YYINITDEPTH 200
#endif

/* YYMAXDEPTH -- maximum size the stacks can grow to (effective only
   if the built-in stack extension method is used).

   Do not make this value too large; the results are undefined if
   YYSTACK_ALLOC_MAXIMUM < YYSTACK_BYTES (YYMAXDEPTH)
   evaluated with infinite-precision integer arithmetic.  */

#ifndef YYMAXDEPTH
# define YYMAXDEPTH 10000
#endif



#if YYERROR_VERBOSE

# ifndef yystrlen
#  if defined __GLIBC__ && defined _STRING_H
#   define yystrlen strlen
#  else
/* Return the length of YYSTR.  */
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static YYSIZE_T
yystrlen (const char *yystr)
#else
static YYSIZE_T
yystrlen (yystr)
    const char *yystr;
#endif
{
  YYSIZE_T yylen;
  for (yylen = 0; yystr[yylen]; yylen++)
    continue;
  return yylen;
}
#  endif
# endif

# ifndef yystpcpy
#  if defined __GLIBC__ && defined _STRING_H && defined _GNU_SOURCE
#   define yystpcpy stpcpy
#  else
/* Copy YYSRC to YYDEST, returning the address of the terminating '\0' in
   YYDEST.  */
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static char *
yystpcpy (char *yydest, const char *yysrc)
#else
static char *
yystpcpy (yydest, yysrc)
    char *yydest;
    const char *yysrc;
#endif
{
  char *yyd = yydest;
  const char *yys = yysrc;

  while ((*yyd++ = *yys++) != '\0')
    continue;

  return yyd - 1;
}
#  endif
# endif

# ifndef yytnamerr
/* Copy to YYRES the contents of YYSTR after stripping away unnecessary
   quotes and backslashes, so that it's suitable for yyerror.  The
   heuristic is that double-quoting is unnecessary unless the string
   contains an apostrophe, a comma, or backslash (other than
   backslash-backslash).  YYSTR is taken from yytname.  If YYRES is
   null, do not copy; instead, return the length of what the result
   would have been.  */
static YYSIZE_T
yytnamerr (char *yyres, const char *yystr)
{
  if (*yystr == '"')
    {
      YYSIZE_T yyn = 0;
      char const *yyp = yystr;

      for (;;)
	switch (*++yyp)
	  {
	  case '\'':
	  case ',':
	    goto do_not_strip_quotes;

	  case '\\':
	    if (*++yyp != '\\')
	      goto do_not_strip_quotes;
	    /* Fall through.  */
	  default:
	    if (yyres)
	      yyres[yyn] = *yyp;
	    yyn++;
	    break;

	  case '"':
	    if (yyres)
	      yyres[yyn] = '\0';
	    return yyn;
	  }
    do_not_strip_quotes: ;
    }

  if (! yyres)
    return yystrlen (yystr);

  return yystpcpy (yyres, yystr) - yyres;
}
# endif

/* Copy into YYRESULT an error message about the unexpected token
   YYCHAR while in state YYSTATE.  Return the number of bytes copied,
   including the terminating null byte.  If YYRESULT is null, do not
   copy anything; just return the number of bytes that would be
   copied.  As a special case, return 0 if an ordinary "syntax error"
   message will do.  Return YYSIZE_MAXIMUM if overflow occurs during
   size calculation.  */
static YYSIZE_T
yysyntax_error (char *yyresult, int yystate, int yychar)
{
  int yyn = yypact[yystate];

  if (! (YYPACT_NINF < yyn && yyn <= YYLAST))
    return 0;
  else
    {
      int yytype = YYTRANSLATE (yychar);
      YYSIZE_T yysize0 = yytnamerr (0, yytname[yytype]);
      YYSIZE_T yysize = yysize0;
      YYSIZE_T yysize1;
      int yysize_overflow = 0;
      enum { YYERROR_VERBOSE_ARGS_MAXIMUM = 5 };
      char const *yyarg[YYERROR_VERBOSE_ARGS_MAXIMUM];
      int yyx;

# if 0
      /* This is so xgettext sees the translatable formats that are
	 constructed on the fly.  */
      YY_("syntax error, unexpected %s");
      YY_("syntax error, unexpected %s, expecting %s");
      YY_("syntax error, unexpected %s, expecting %s or %s");
      YY_("syntax error, unexpected %s, expecting %s or %s or %s");
      YY_("syntax error, unexpected %s, expecting %s or %s or %s or %s");
# endif
      char *yyfmt;
      char const *yyf;
      static char const yyunexpected[] = "syntax error, unexpected %s";
      static char const yyexpecting[] = ", expecting %s";
      static char const yyor[] = " or %s";
      char yyformat[sizeof yyunexpected
		    + sizeof yyexpecting - 1
		    + ((YYERROR_VERBOSE_ARGS_MAXIMUM - 2)
		       * (sizeof yyor - 1))];
      char const *yyprefix = yyexpecting;

      /* Start YYX at -YYN if negative to avoid negative indexes in
	 YYCHECK.  */
      int yyxbegin = yyn < 0 ? -yyn : 0;

      /* Stay within bounds of both yycheck and yytname.  */
      int yychecklim = YYLAST - yyn + 1;
      int yyxend = yychecklim < YYNTOKENS ? yychecklim : YYNTOKENS;
      int yycount = 1;

      yyarg[0] = yytname[yytype];
      yyfmt = yystpcpy (yyformat, yyunexpected);

      for (yyx = yyxbegin; yyx < yyxend; ++yyx)
	if (yycheck[yyx + yyn] == yyx && yyx != YYTERROR)
	  {
	    if (yycount == YYERROR_VERBOSE_ARGS_MAXIMUM)
	      {
		yycount = 1;
		yysize = yysize0;
		yyformat[sizeof yyunexpected - 1] = '\0';
		break;
	      }
	    yyarg[yycount++] = yytname[yyx];
	    yysize1 = yysize + yytnamerr (0, yytname[yyx]);
	    yysize_overflow |= (yysize1 < yysize);
	    yysize = yysize1;
	    yyfmt = yystpcpy (yyfmt, yyprefix);
	    yyprefix = yyor;
	  }

      yyf = YY_(yyformat);
      yysize1 = yysize + yystrlen (yyf);
      yysize_overflow |= (yysize1 < yysize);
      yysize = yysize1;

      if (yysize_overflow)
	return YYSIZE_MAXIMUM;

      if (yyresult)
	{
	  /* Avoid sprintf, as that infringes on the user's name space.
	     Don't have undefined behavior even if the translation
	     produced a string with the wrong number of "%s"s.  */
	  char *yyp = yyresult;
	  int yyi = 0;
	  while ((*yyp = *yyf) != '\0')
	    {
	      if (*yyp == '%' && yyf[1] == 's' && yyi < yycount)
		{
		  yyp += yytnamerr (yyp, yyarg[yyi++]);
		  yyf += 2;
		}
	      else
		{
		  yyp++;
		  yyf++;
		}
	    }
	}
      return yysize;
    }
}
#endif /* YYERROR_VERBOSE */


/*-----------------------------------------------.
| Release the memory associated to this symbol.  |
`-----------------------------------------------*/

/*ARGSUSED*/
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static void
yydestruct (const char *yymsg, int yytype, YYSTYPE *yyvaluep)
#else
static void
yydestruct (yymsg, yytype, yyvaluep)
    const char *yymsg;
    int yytype;
    YYSTYPE *yyvaluep;
#endif
{
  YYUSE (yyvaluep);

  if (!yymsg)
    yymsg = "Deleting";
  YY_SYMBOL_PRINT (yymsg, yytype, yyvaluep, yylocationp);

  switch (yytype)
    {
      case 52: /* "TOK_SELECT" */
#line 236 "CQL.y"
	{CQL_Bison_Cleanup(); };
#line 1372 "CQLtemp"
	break;

      default:
	break;
    }
}


/* Prevent warnings from -Wmissing-prototypes.  */

#ifdef YYPARSE_PARAM
#if defined __STDC__ || defined __cplusplus
int yyparse (void *YYPARSE_PARAM);
#else
int yyparse ();
#endif
#else /* ! YYPARSE_PARAM */
#if defined __STDC__ || defined __cplusplus
int yyparse (void);
#else
int yyparse ();
#endif
#endif /* ! YYPARSE_PARAM */



/* The look-ahead symbol.  */
int yychar;

/* The semantic value of the look-ahead symbol.  */
YYSTYPE yylval;

/* Number of syntax errors so far.  */
int yynerrs;



/*----------.
| yyparse.  |
`----------*/

#ifdef YYPARSE_PARAM
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
int
yyparse (void *YYPARSE_PARAM)
#else
int
yyparse (YYPARSE_PARAM)
    void *YYPARSE_PARAM;
#endif
#else /* ! YYPARSE_PARAM */
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
int
yyparse (void)
#else
int
yyparse ()

#endif
#endif
{
  
  int yystate;
  int yyn;
  int yyresult;
  /* Number of tokens to shift before error messages enabled.  */
  int yyerrstatus;
  /* Look-ahead token as an internal (translated) token number.  */
  int yytoken = 0;
#if YYERROR_VERBOSE
  /* Buffer for error messages, and its allocated size.  */
  char yymsgbuf[128];
  char *yymsg = yymsgbuf;
  YYSIZE_T yymsg_alloc = sizeof yymsgbuf;
#endif

  /* Three stacks and their tools:
     `yyss': related to states,
     `yyvs': related to semantic values,
     `yyls': related to locations.

     Refer to the stacks thru separate pointers, to allow yyoverflow
     to reallocate them elsewhere.  */

  /* The state stack.  */
  yytype_int16 yyssa[YYINITDEPTH];
  yytype_int16 *yyss = yyssa;
  yytype_int16 *yyssp;

  /* The semantic value stack.  */
  YYSTYPE yyvsa[YYINITDEPTH];
  YYSTYPE *yyvs = yyvsa;
  YYSTYPE *yyvsp;



#define YYPOPSTACK(N)   (yyvsp -= (N), yyssp -= (N))

  YYSIZE_T yystacksize = YYINITDEPTH;

  /* The variables used to return semantic value and location from the
     action routines.  */
  YYSTYPE yyval;


  /* The number of symbols on the RHS of the reduced rule.
     Keep to zero when no symbol should be popped.  */
  int yylen = 0;

  YYDPRINTF ((stderr, "Starting parse\n"));

  yystate = 0;
  yyerrstatus = 0;
  yynerrs = 0;
  yychar = YYEMPTY;		/* Cause a token to be read.  */

  /* Initialize stack pointers.
     Waste one element of value and location stack
     so that they stay on the same level as the state stack.
     The wasted elements are never initialized.  */

  yyssp = yyss;
  yyvsp = yyvs;

  goto yysetstate;

/*------------------------------------------------------------.
| yynewstate -- Push a new state, which is found in yystate.  |
`------------------------------------------------------------*/
 yynewstate:
  /* In all cases, when you get here, the value and location stacks
     have just been pushed.  So pushing a state here evens the stacks.  */
  yyssp++;

 yysetstate:
  *yyssp = yystate;

  if (yyss + yystacksize - 1 <= yyssp)
    {
      /* Get the current used size of the three stacks, in elements.  */
      YYSIZE_T yysize = yyssp - yyss + 1;

#ifdef yyoverflow
      {
	/* Give user a chance to reallocate the stack.  Use copies of
	   these so that the &'s don't force the real ones into
	   memory.  */
	YYSTYPE *yyvs1 = yyvs;
	yytype_int16 *yyss1 = yyss;


	/* Each stack pointer address is followed by the size of the
	   data in use in that stack, in bytes.  This used to be a
	   conditional around just the two extra args, but that might
	   be undefined if yyoverflow is a macro.  */
	yyoverflow (YY_("memory exhausted"),
		    &yyss1, yysize * sizeof (*yyssp),
		    &yyvs1, yysize * sizeof (*yyvsp),

		    &yystacksize);

	yyss = yyss1;
	yyvs = yyvs1;
      }
#else /* no yyoverflow */
# ifndef YYSTACK_RELOCATE
      goto yyexhaustedlab;
# else
      /* Extend the stack our own way.  */
      if (YYMAXDEPTH <= yystacksize)
	goto yyexhaustedlab;
      yystacksize *= 2;
      if (YYMAXDEPTH < yystacksize)
	yystacksize = YYMAXDEPTH;

      {
	yytype_int16 *yyss1 = yyss;
	union yyalloc *yyptr =
	  (union yyalloc *) YYSTACK_ALLOC (YYSTACK_BYTES (yystacksize));
	if (! yyptr)
	  goto yyexhaustedlab;
	YYSTACK_RELOCATE (yyss);
	YYSTACK_RELOCATE (yyvs);

#  undef YYSTACK_RELOCATE
	if (yyss1 != yyssa)
	  YYSTACK_FREE (yyss1);
      }
# endif
#endif /* no yyoverflow */

      yyssp = yyss + yysize - 1;
      yyvsp = yyvs + yysize - 1;


      YYDPRINTF ((stderr, "Stack size increased to %lu\n",
		  (unsigned long int) yystacksize));

      if (yyss + yystacksize - 1 <= yyssp)
	YYABORT;
    }

  YYDPRINTF ((stderr, "Entering state %d\n", yystate));

  goto yybackup;

/*-----------.
| yybackup.  |
`-----------*/
yybackup:

  /* Do appropriate processing given the current state.  Read a
     look-ahead token if we need one and don't already have one.  */

  /* First try to decide what to do without reference to look-ahead token.  */
  yyn = yypact[yystate];
  if (yyn == YYPACT_NINF)
    goto yydefault;

  /* Not known => get a look-ahead token if don't already have one.  */

  /* YYCHAR is either YYEMPTY or YYEOF or a valid look-ahead symbol.  */
  if (yychar == YYEMPTY)
    {
      YYDPRINTF ((stderr, "Reading a token: "));
      yychar = YYLEX;
    }

  if (yychar <= YYEOF)
    {
      yychar = yytoken = YYEOF;
      YYDPRINTF ((stderr, "Now at end of input.\n"));
    }
  else
    {
      yytoken = YYTRANSLATE (yychar);
      YY_SYMBOL_PRINT ("Next token is", yytoken, &yylval, &yylloc);
    }

  /* If the proper action on seeing token YYTOKEN is to reduce or to
     detect an error, take that action.  */
  yyn += yytoken;
  if (yyn < 0 || YYLAST < yyn || yycheck[yyn] != yytoken)
    goto yydefault;
  yyn = yytable[yyn];
  if (yyn <= 0)
    {
      if (yyn == 0 || yyn == YYTABLE_NINF)
	goto yyerrlab;
      yyn = -yyn;
      goto yyreduce;
    }

  if (yyn == YYFINAL)
    YYACCEPT;

  /* Count tokens shifted since error; after three, turn off error
     status.  */
  if (yyerrstatus)
    yyerrstatus--;

  /* Shift the look-ahead token.  */
  YY_SYMBOL_PRINT ("Shifting", yytoken, &yylval, &yylloc);

  /* Discard the shifted token unless it is eof.  */
  if (yychar != YYEOF)
    yychar = YYEMPTY;

  yystate = yyn;
  *++yyvsp = yylval;

  goto yynewstate;


/*-----------------------------------------------------------.
| yydefault -- do the default action for the current state.  |
`-----------------------------------------------------------*/
yydefault:
  yyn = yydefact[yystate];
  if (yyn == 0)
    goto yyerrlab;
  goto yyreduce;


/*-----------------------------.
| yyreduce -- Do a reduction.  |
`-----------------------------*/
yyreduce:
  /* yyn is the number of a rule to reduce with.  */
  yylen = yyr2[yyn];

  /* If YYLEN is nonzero, implement the default value of the action:
     `$$ = $1'.

     Otherwise, the following line sets YYVAL to garbage.
     This behavior is undocumented and Bison
     users should not rely upon it.  Assigning to YYVAL
     unconditionally makes the parser a bit smaller, and it avoids a
     GCC warning that YYVAL may be used uninitialized.  */
  yyval = yyvsp[1-yylen];


  YY_REDUCE_PRINT (yyn);
  switch (yyn)
    {
        case 2:
#line 260 "CQL.y"
    {
        CQL_globalParserState->currentRule = "identifier";
        CQL_DEBUG_TRACE("BISON::identifier\n");

        if(isUTF8Str(CQL_lval.strValue))
        {
            (yyval._identifier) = new CQLIdentifier(String(CQL_lval.strValue));
                _ObjPtr._ptr = (yyval._identifier);
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
    ;}
    break;

  case 3:
#line 286 "CQL.y"
    {
        CQL_globalParserState->currentRule = "class_name";
        CQL_DEBUG_TRACE2("BISON::class_name = %s\n",
            (const char *)((yyvsp[(1) - (1)]._identifier)->getName().getString().getCString()));
        (yyval._identifier) = (yyvsp[(1) - (1)]._identifier);
    ;}
    break;

  case 4:
#line 295 "CQL.y"
    {
        CQL_globalParserState->currentRule = "class_path";
        CQL_DEBUG_TRACE("BISON::class_path\n");
        (yyval._identifier) = (yyvsp[(1) - (1)]._identifier);
    ;}
    break;

  case 5:
#line 311 "CQL.y"
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
            (yyval._identifier) = new CQLIdentifier(tmp);
            _ObjPtr._ptr = (yyval._identifier);
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
    ;}
    break;

  case 6:
#line 344 "CQL.y"
    {
        /*
        Make sure the literal is valid UTF8, then make a String
        */
        CQL_globalParserState->currentRule = "literal_string";
        CQL_DEBUG_TRACE2("BISON::literal_string-> %s\n", CQL_lval.strValue);

        if(isUTF8Str(CQL_lval.strValue))
        {
            (yyval._string) = new String(CQL_lval.strValue);
            _ObjPtr._ptr = (yyval._string);
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
    ;}
    break;

  case 7:
#line 373 "CQL.y"
    {
        CQL_globalParserState->currentRule = "binary_value->TOK_BINARY";
        CQL_DEBUG_TRACE2("BISON::binary_value-> %s\n", CQL_lval.strValue);

        (yyval._value) = new CQLValue(CQL_lval.strValue, CQLValue::Binary);
        _ObjPtr._ptr = (yyval._value);
        _ObjPtr.type = Val;
        _ptrs.append(_ObjPtr);
    ;}
    break;

  case 8:
#line 383 "CQL.y"
    {
        CQL_globalParserState->currentRule =
            "binary_value->TOK_NEGATIVE_BINARY";
        CQL_DEBUG_TRACE2("BISON::binary_value-> %s\n", CQL_lval.strValue);

        (yyval._value) = new CQLValue(CQL_lval.strValue, CQLValue::Binary, false);
        _ObjPtr._ptr = (yyval._value);
        _ObjPtr.type = Val;
        _ptrs.append(_ObjPtr);
    ;}
    break;

  case 9:
#line 397 "CQL.y"
    {
        CQL_globalParserState->currentRule = "hex_value->TOK_HEXADECIMAL";
        CQL_DEBUG_TRACE2("BISON::hex_value-> %s\n", CQL_lval.strValue);

        (yyval._value) = new CQLValue(CQL_lval.strValue, CQLValue::Hex);
        _ObjPtr._ptr = (yyval._value);
        _ObjPtr.type = Val;
        _ptrs.append(_ObjPtr);
    ;}
    break;

  case 10:
#line 407 "CQL.y"
    {
        CQL_globalParserState->currentRule =
            "hex_value->TOK_NEGATIVE_HEXADECIMAL";
        CQL_DEBUG_TRACE2("BISON::hex_value-> %s\n", CQL_lval.strValue);

        (yyval._value) = new CQLValue(CQL_lval.strValue, CQLValue::Hex, false);
        _ObjPtr._ptr = (yyval._value);
        _ObjPtr.type = Val;
        _ptrs.append(_ObjPtr);
    ;}
    break;

  case 11:
#line 421 "CQL.y"
    {
        CQL_globalParserState->currentRule = "decimal_value->TOK_INTEGER";
        CQL_DEBUG_TRACE2("BISON::decimal_value-> %s\n", CQL_lval.strValue);

        (yyval._value) = new CQLValue(CQL_lval.strValue, CQLValue::Decimal);
        _ObjPtr._ptr = (yyval._value);
        _ObjPtr.type = Val;
        _ptrs.append(_ObjPtr);
    ;}
    break;

  case 12:
#line 431 "CQL.y"
    {
        CQL_globalParserState->currentRule =
            "decimal_value->TOK_NEGATIVE_INTEGER";
        CQL_DEBUG_TRACE2("BISON::decimal_value-> %s\n", CQL_lval.strValue);

        (yyval._value) = new CQLValue(CQL_lval.strValue, CQLValue::Decimal, false);
        _ObjPtr._ptr = (yyval._value);
        _ObjPtr.type = Val;
        _ptrs.append(_ObjPtr);
    ;}
    break;

  case 13:
#line 445 "CQL.y"
    {
        CQL_globalParserState->currentRule = "real_value->TOK_REAL";
        CQL_DEBUG_TRACE2("BISON::real_value-> %s\n", CQL_lval.strValue);

        (yyval._value) = new CQLValue(CQL_lval.strValue, CQLValue::Real);
        _ObjPtr._ptr = (yyval._value);
        _ObjPtr.type = Val;
        _ptrs.append(_ObjPtr);
    ;}
    break;

  case 14:
#line 455 "CQL.y"
    {
        CQL_globalParserState->currentRule = "real_value->TOK_NEGATIVE_REAL";
        CQL_DEBUG_TRACE2("BISON::real_value-> %s\n", CQL_lval.strValue);

        (yyval._value) = new CQLValue(CQL_lval.strValue, CQLValue::Real, false);
        _ObjPtr._ptr = (yyval._value);
        _ObjPtr.type = Val;
        _ptrs.append(_ObjPtr);
    ;}
    break;

  case 15:
#line 468 "CQL.y"
    {
            CQL_globalParserState->currentRule = "literal->literal_string";
            CQL_DEBUG_TRACE("BISON::literal->literal_string\n");
            (yyval._value) = new CQLValue(*(yyvsp[(1) - (1)]._string));
            _ObjPtr._ptr = (yyval._value);
            _ObjPtr.type = Val;
            _ptrs.append(_ObjPtr);
        ;}
    break;

  case 16:
#line 477 "CQL.y"
    {
            CQL_globalParserState->currentRule = "literal->decimal_value";
            CQL_DEBUG_TRACE("BISON::literal->decimal_value\n");
        ;}
    break;

  case 17:
#line 482 "CQL.y"
    {
            CQL_globalParserState->currentRule = "literal->binary_value";
            CQL_DEBUG_TRACE("BISON::literal->binary_value\n");
        ;}
    break;

  case 18:
#line 487 "CQL.y"
    {
            CQL_globalParserState->currentRule = "literal->hex_value";
            CQL_DEBUG_TRACE("BISON::literal->hex_value\n");
        ;}
    break;

  case 19:
#line 492 "CQL.y"
    {
            CQL_globalParserState->currentRule = "literal->real_value";
            CQL_DEBUG_TRACE("BISON::literal->real_value\n");
        ;}
    break;

  case 20:
#line 497 "CQL.y"
    {
            CQL_globalParserState->currentRule = "literal->TOK_TRUE";
            CQL_DEBUG_TRACE("BISON::literal->TOK_TRUE\n");

            (yyval._value) = new CQLValue(Boolean(true));
            _ObjPtr._ptr = (yyval._value);
            _ObjPtr.type = Val;
            _ptrs.append(_ObjPtr);
        ;}
    break;

  case 21:
#line 507 "CQL.y"
    {
            CQL_globalParserState->currentRule = "literal->TOK_FALSE";
            CQL_DEBUG_TRACE("BISON::literal->TOK_FALSE\n");

            (yyval._value) = new CQLValue(Boolean(false));
            _ObjPtr._ptr = (yyval._value);
            _ObjPtr.type = Val;
            _ptrs.append(_ObjPtr);
        ;}
    break;

  case 22:
#line 520 "CQL.y"
    {
        CQL_globalParserState->currentRule = "array_index->expr";
        CQL_DEBUG_TRACE("BISON::array_index->expr\n");

        CQLValue* _val = (CQLValue*)_factory.getObject((yyvsp[(1) - (1)]._predicate),Predicate,Value);
        (yyval._string) = new String(_val->toString());
        _ObjPtr._ptr = (yyval._string);
        _ObjPtr.type = Str;
        _ptrs.append(_ObjPtr);
    ;}
    break;

  case 23:
#line 534 "CQL.y"
    {
        CQL_globalParserState->currentRule = "array_index_list->array_index";
        CQL_DEBUG_TRACE("BISON::array_index_list->array_index\n");

        (yyval._string) = (yyvsp[(1) - (1)]._string);
    ;}
    break;

  case 24:
#line 544 "CQL.y"
    {
        CQL_globalParserState->currentRule = "chain->literal";
        CQL_DEBUG_TRACE("BISON::chain->literal\n");

        chain_state = CQLVALUE;
        (yyval._node) = _factory.makeObject((yyvsp[(1) - (1)]._value),Predicate);
    ;}
    break;

  case 25:
#line 553 "CQL.y"
    {
        CQL_globalParserState->currentRule = "chain-> ( expr )";
        CQL_DEBUG_TRACE("BISON::chain-> ( expr )\n");

        chain_state = CQLPREDICATE;
        (yyval._node) = (yyvsp[(2) - (3)]._predicate);
    ;}
    break;

  case 26:
#line 562 "CQL.y"
    {
        CQL_globalParserState->currentRule = "chain->identifier";
        CQL_DEBUG_TRACE("BISON::chain->identifier\n");

        chain_state = CQLIDENTIFIER;
        (yyval._node) = _factory.makeObject((yyvsp[(1) - (1)]._identifier),Predicate);
    ;}
    break;

  case 27:
#line 571 "CQL.y"
    {
        CQL_globalParserState->currentRule =
            "chain->identifier#literal_string";
        CQL_DEBUG_TRACE("BISON::chain->identifier#literal_string\n");

        String tmp = (yyvsp[(1) - (3)]._identifier)->getName().getString();
        tmp.append("#").append(*(yyvsp[(3) - (3)]._string));
        CQLIdentifier _id(tmp);
        (yyval._node) = _factory.makeObject(&_id,Predicate);
        chain_state = CQLIDENTIFIER;
    ;}
    break;

  case 28:
#line 584 "CQL.y"
    {
        CQL_globalParserState->currentRule = "chain->scoped_property";
        CQL_DEBUG_TRACE("BISON::chain->scoped_property\n");

        chain_state = CQLIDENTIFIER;
        (yyval._node) = _factory.makeObject((yyvsp[(1) - (1)]._identifier),Predicate);
    ;}
    break;

  case 29:
#line 593 "CQL.y"
    {
        CQL_globalParserState->currentRule = "identifier( arg_list )";
        CQL_DEBUG_TRACE("BISON::identifier( arg_list )\n");

        chain_state = CQLFUNCTION;
        CQLFunction _func(*(yyvsp[(1) - (4)]._identifier),_arglist);
        (yyval._node) = (CQLPredicate*)(_factory.makeObject(&_func,Predicate));
        CQL_Arglist_Cleanup();
    ;}
    break;

  case 30:
#line 604 "CQL.y"
    {
        CQL_globalParserState->currentRule = "chain->chain.scoped_property";
        CQL_DEBUG_TRACE2(
            "BISON::chain-> chain TOK_DOT scoped_property : chain_state = %d\n"
            ,chain_state);

        CQLIdentifier *_id;
        if(chain_state == CQLIDENTIFIER)
        {
            _id = ((CQLIdentifier*)(_factory.getObject(
                (yyvsp[(1) - (3)]._node),Predicate,Identifier)));
            CQLChainedIdentifier _cid(*_id);
            _cid.append(*(yyvsp[(3) - (3)]._identifier));
            (yyval._node) = _factory.makeObject(&_cid,Predicate);
        }
        else if(chain_state == CQLCHAINEDIDENTIFIER)
        {
            CQLChainedIdentifier *_cid;
            _cid = ((CQLChainedIdentifier*)(_factory.getObject(
                (yyvsp[(1) - (3)]._node),Predicate,ChainedIdentifier)));
            _cid->append(*(yyvsp[(3) - (3)]._identifier));
            _factory.setObject(((CQLPredicate*)(yyvsp[(1) - (3)]._node)),_cid,ChainedIdentifier);
            (yyval._node) = (yyvsp[(1) - (3)]._node);
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
    ;}
    break;

  case 31:
#line 646 "CQL.y"
    {
        CQL_globalParserState->currentRule = "chain->chain.identifier";
        CQL_DEBUG_TRACE2("BISON::chain->chain.identifier : chain_state = %d\n"
            ,chain_state);

        if(chain_state == CQLIDENTIFIER)
        {
            CQLIdentifier *_id = ((CQLIdentifier*)(_factory.getObject(
            (yyvsp[(1) - (3)]._node),Predicate,Identifier)));
            CQLChainedIdentifier _cid(*_id);
            _cid.append(*(yyvsp[(3) - (3)]._identifier));
            (yyval._node) = _factory.makeObject(&_cid,Predicate);
        }
        else if(chain_state == CQLCHAINEDIDENTIFIER)
        {
            CQLChainedIdentifier *_cid =
            ((CQLChainedIdentifier*)(_factory.getObject(
            (yyvsp[(1) - (3)]._node),Predicate,ChainedIdentifier)));
            _cid->append(*(yyvsp[(3) - (3)]._identifier));
            _factory.setObject(((CQLPredicate*)(yyvsp[(1) - (3)]._node)),_cid,ChainedIdentifier);
            (yyval._node) = (yyvsp[(1) - (3)]._node);
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
    ;}
    break;

  case 32:
#line 684 "CQL.y"
    {
        CQL_globalParserState->currentRule =
            "chain->chain.identifier#literal_string";
        CQL_DEBUG_TRACE2(
            "BISON::chain->chain.identifier#literal_string : chain_state = %d\n"
            ,chain_state);

        if(chain_state == CQLIDENTIFIER)
        {
            CQLIdentifier *_id = ((CQLIdentifier*)(_factory.getObject(
                (yyvsp[(1) - (5)]._node),Predicate,Identifier)));
            CQLChainedIdentifier _cid(*_id);
            String tmp((yyvsp[(3) - (5)]._identifier)->getName().getString());
            tmp.append("#").append(*(yyvsp[(5) - (5)]._string));
            CQLIdentifier _id1(tmp);
            _cid.append(_id1);
            _factory.setObject(((CQLPredicate*)(yyvsp[(1) - (5)]._node)),&_cid,ChainedIdentifier);
            (yyval._node) = (yyvsp[(1) - (5)]._node);
        }
        else if(chain_state == CQLCHAINEDIDENTIFIER)
        {
            CQLChainedIdentifier *_cid =  ((CQLChainedIdentifier*)(
                _factory.getObject((yyvsp[(1) - (5)]._node),Predicate,ChainedIdentifier)));
            String tmp((yyvsp[(3) - (5)]._identifier)->getName().getString());
            tmp.append("#").append(*(yyvsp[(5) - (5)]._string));
            CQLIdentifier _id1(tmp);
            _cid->append(_id1);
            _factory.setObject(((CQLPredicate*)(yyvsp[(1) - (5)]._node)),_cid,ChainedIdentifier);
            (yyval._node) = (yyvsp[(1) - (5)]._node);
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
    ;}
    break;

  case 33:
#line 730 "CQL.y"
    {
        CQL_globalParserState->currentRule =
            "chain->chain[ array_index_list ]";
        CQL_DEBUG_TRACE2(
            "BISON::chain->chain[ array_index_list ] : chain_state = %d\n"
            ,chain_state);

        if(chain_state == CQLIDENTIFIER)
        {
            CQLIdentifier *_id = ((CQLIdentifier*)
                (_factory.getObject((yyvsp[(1) - (4)]._node),Predicate,Identifier)));
            String tmp = _id->getName().getString();
            tmp.append("[").append(*(yyvsp[(3) - (4)]._string)).append("]");
            CQLIdentifier _id1(tmp);
            CQLChainedIdentifier _cid(_id1);
            _factory.setObject(((CQLPredicate*)(yyvsp[(1) - (4)]._node)),&_cid,ChainedIdentifier);
            (yyval._node) = (yyvsp[(1) - (4)]._node);
        }
        else if(chain_state == CQLCHAINEDIDENTIFIER || chain_state == CQLVALUE)
        {
            CQLChainedIdentifier *_cid = ((CQLChainedIdentifier*)
                (_factory.getObject
                ((yyvsp[(1) - (4)]._node),Predicate,ChainedIdentifier)));
            CQLIdentifier tmpid = _cid->getLastIdentifier();
            String tmp = tmpid.getName().getString();
            tmp.append("[").append(*(yyvsp[(3) - (4)]._string)).append("]");
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
            _factory.setObject(((CQLPredicate*)(yyvsp[(1) - (4)]._node)),_cid,ChainedIdentifier);
            (yyval._node) = (yyvsp[(1) - (4)]._node);
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
    ;}
    break;

  case 34:
#line 788 "CQL.y"
    {
        CQL_globalParserState->currentRule = "concat->chain";
        CQL_DEBUG_TRACE("BISON::concat->chain\n");

        (yyval._predicate) = ((CQLPredicate*)(yyvsp[(1) - (1)]._node));
    ;}
    break;

  case 35:
#line 796 "CQL.y"
    {
        CQL_globalParserState->currentRule =
            "concat->concat || literal_string";
        CQL_DEBUG_TRACE("BISON::concat->concat || literal_string\n");

        CQLValue* tmpval = new CQLValue(*(yyvsp[(3) - (3)]._string));
        _ObjPtr._ptr = tmpval;
        _ObjPtr.type = Val;
        _ptrs.append(_ObjPtr);

        if((CQLPredicate*)(yyvsp[(1) - (3)]._predicate)->isSimple())
        {
            CQLSimplePredicate sp = ((CQLPredicate*)(yyvsp[(1) - (3)]._predicate))->getSimplePredicate();
            if(sp.isSimple())
            {
                CQLExpression exp = sp.getLeftExpression();
                if(exp.isSimple())
                {
                    CQLTerm* _term = ((CQLTerm*)
                        (_factory.getObject((yyvsp[(1) - (3)]._predicate), Predicate, Term)));
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
                    (yyval._predicate) = (CQLPredicate*)(_factory.makeObject
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
    ;}
    break;

  case 36:
#line 885 "CQL.y"
    {
        CQL_globalParserState->currentRule = "factor->concat";
        CQL_DEBUG_TRACE("BISON::factor->concat\n");

        (yyval._predicate) = (yyvsp[(1) - (1)]._predicate);
    ;}
    break;

  case 37:
#line 916 "CQL.y"
    {
        CQL_globalParserState->currentRule = "term->factor";
        CQL_DEBUG_TRACE("BISON::term->factor\n");

        (yyval._predicate) = (yyvsp[(1) - (1)]._predicate);
    ;}
    break;

  case 38:
#line 945 "CQL.y"
    {
        CQL_globalParserState->currentRule = "arith->term";
        CQL_DEBUG_TRACE("BISON::arith->term\n");

        //CQLPredicate* _pred = new CQLPredicate(*$1);
        //      _factory._predicates.append(_pred);
        (yyval._predicate) = (yyvsp[(1) - (1)]._predicate);
    ;}
    break;

  case 39:
#line 976 "CQL.y"
    {
        CQL_globalParserState->currentRule = "value_symbol->#literal_string";
        CQL_DEBUG_TRACE("BISON::value_symbol->#literal_string\n");

        String tmp("#");
        tmp.append(*(yyvsp[(2) - (2)]._string));
        CQLIdentifier tmpid(tmp);
        (yyval._value) = new CQLValue(tmpid);
        _ObjPtr._ptr = (yyval._value);
        _ObjPtr.type = Val;
        _ptrs.append(_ObjPtr);
    ;}
    break;

  case 40:
#line 991 "CQL.y"
    {
        CQL_globalParserState->currentRule = "arith_or_value_symbol->arith";
        CQL_DEBUG_TRACE("BISON::arith_or_value_symbol->arith\n");

        (yyval._predicate) = (yyvsp[(1) - (1)]._predicate);
    ;}
    break;

  case 41:
#line 998 "CQL.y"
    {
        /* make into predicate */
        CQL_globalParserState->currentRule =
            "arith_or_value_symbol->value_symbol";
        CQL_DEBUG_TRACE("BISON::arith_or_value_symbol->value_symbol\n");

        CQLFactor _fctr(*(yyvsp[(1) - (1)]._value));
        (yyval._predicate) = (CQLPredicate*)(_factory.makeObject(&_fctr, Predicate));
    ;}
    break;

  case 42:
#line 1010 "CQL.y"
    {
        CQL_globalParserState->currentRule = "comp_op->TOK_EQ";
        CQL_DEBUG_TRACE("BISON::comp_op->TOK_EQ\n");

        (yyval._opType) = EQ;
    ;}
    break;

  case 43:
#line 1017 "CQL.y"
    {
        CQL_globalParserState->currentRule = "comp_op->TOK_NE";
        CQL_DEBUG_TRACE("BISON::comp_op->TOK_NE\n");
        (yyval._opType) = NE;
    ;}
    break;

  case 44:
#line 1023 "CQL.y"
    {
        CQL_globalParserState->currentRule = "comp_op->TOK_GT";
        CQL_DEBUG_TRACE("BISON::comp_op->TOK_GT\n");

        (yyval._opType) = GT;
    ;}
    break;

  case 45:
#line 1030 "CQL.y"
    {
        CQL_globalParserState->currentRule = "comp_op->TOK_LT";
        CQL_DEBUG_TRACE("BISON::comp_op->TOK_LT\n");
        (yyval._opType) = LT;
    ;}
    break;

  case 46:
#line 1036 "CQL.y"
    {
        CQL_globalParserState->currentRule = "comp_op->TOK_GE";
        CQL_DEBUG_TRACE("BISON::comp_op->TOK_GE\n");
        (yyval._opType) = GE;
    ;}
    break;

  case 47:
#line 1042 "CQL.y"
    {
        CQL_globalParserState->currentRule = "comp_op->TOK_LE";
        CQL_DEBUG_TRACE("BISON::comp_op->TOK_LE\n");
        (yyval._opType) = LE;
    ;}
    break;

  case 48:
#line 1050 "CQL.y"
    {
        CQL_globalParserState->currentRule = "comp->arith";
        CQL_DEBUG_TRACE("BISON::comp->arith\n");

        (yyval._predicate) = (yyvsp[(1) - (1)]._predicate);
    ;}
    break;

  case 49:
#line 1057 "CQL.y"
    {
        CQL_globalParserState->currentRule =
            "comp->arith TOK_IS TOK_NOT TOK_NULL";
        CQL_DEBUG_TRACE("BISON::comp->arith TOK_IS TOK_NOT TOK_NULL\n");

        CQLExpression *_expr =
            (CQLExpression*)(_factory.getObject((yyvsp[(1) - (4)]._predicate),Expression));
        CQLSimplePredicate _sp(*_expr, IS_NOT_NULL);
        _factory.setObject((yyvsp[(1) - (4)]._predicate),&_sp,SimplePredicate);
        (yyval._predicate) = (yyvsp[(1) - (4)]._predicate);
    ;}
    break;

  case 50:
#line 1069 "CQL.y"
    {
        CQL_globalParserState->currentRule = "comp->arith TOK_IS TOK_NULL";
        CQL_DEBUG_TRACE("BISON::comp->arith TOK_IS TOK_NULL\n");

        CQLExpression *_expr =
            (CQLExpression*)(_factory.getObject((yyvsp[(1) - (3)]._predicate),Expression));
        CQLSimplePredicate _sp(*_expr, IS_NULL);
        _factory.setObject((yyvsp[(1) - (3)]._predicate),&_sp,SimplePredicate);
        (yyval._predicate) = (yyvsp[(1) - (3)]._predicate);
    ;}
    break;

  case 51:
#line 1080 "CQL.y"
    {
        CQL_globalParserState->currentRule =
            "comp->arith comp_op arith_or_value_symbol";
        CQL_DEBUG_TRACE("BISON::comp->arith comp_op arith_or_value_symbol\n");

        if((yyvsp[(1) - (3)]._predicate)->isSimple() && (yyvsp[(3) - (3)]._predicate)->isSimple())
        {
            CQLExpression* _exp1 = (CQLExpression*)
            (_factory.getObject((yyvsp[(1) - (3)]._predicate),Predicate,Expression));
            CQLExpression* _exp2 = (CQLExpression*)
            (_factory.getObject((yyvsp[(3) - (3)]._predicate),Predicate,Expression));
            CQLSimplePredicate _sp(*_exp1, *_exp2, (yyvsp[(2) - (3)]._opType));
            (yyval._predicate) = new CQLPredicate(_sp);
            _ObjPtr._ptr = (yyval._predicate);
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
    ;}
    break;

  case 52:
#line 1110 "CQL.y"
    {
        CQL_globalParserState->currentRule =
            "comp->value_symbol comp_op arith";
        CQL_DEBUG_TRACE("BISON::comp->value_symbol comp_op arith\n");

        if((yyvsp[(3) - (3)]._predicate)->isSimple())
        {
            CQLExpression* _exp1 = (CQLExpression*)
                (_factory.makeObject((yyvsp[(1) - (3)]._value), Expression));
            CQLExpression* _exp2 = (CQLExpression*)
                (_factory.getObject((yyvsp[(3) - (3)]._predicate),Predicate,Expression));
            CQLSimplePredicate _sp(*_exp1, *_exp2, (yyvsp[(2) - (3)]._opType));
            (yyval._predicate) = new CQLPredicate(_sp);
            _ObjPtr._ptr = (yyval._predicate);
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
    ;}
    break;

  case 53:
#line 1140 "CQL.y"
    {
        /* make sure $1 isSimple(), get its expression, make
           simplepred->predicate
        */
        CQL_globalParserState->currentRule = "comp->arith _TOK_ISA identifier";
        CQL_DEBUG_TRACE("BISON::comp->arith _TOK_ISA identifier\n");

        CQLExpression *_expr1 = (CQLExpression*)
        (_factory.getObject((yyvsp[(1) - (3)]._predicate),Predicate,Expression));
        CQLChainedIdentifier _cid(*(yyvsp[(3) - (3)]._identifier));
        CQLExpression *_expr2 = (CQLExpression*)
        (_factory.makeObject(&_cid,Expression));
        CQLSimplePredicate _sp(*_expr1, *_expr2, ISA);
        _factory.setObject((yyvsp[(1) - (3)]._predicate),&_sp,SimplePredicate);
        (yyval._predicate) = (yyvsp[(1) - (3)]._predicate);
    ;}
    break;

  case 54:
#line 1157 "CQL.y"
    {
        CQL_globalParserState->currentRule =
            "comp->arith TOK_LIKE literal_string";
        CQL_DEBUG_TRACE("BISON::comp->arith TOK_LIKE literal_string\n");

        CQLExpression *_expr1 = (CQLExpression*)
            (_factory.getObject((yyvsp[(1) - (3)]._predicate),Predicate,Expression));
        CQLValue _val(*(yyvsp[(3) - (3)]._string));
        CQLExpression *_expr2 = (CQLExpression*)
            (_factory.makeObject(&_val,Expression));
        CQLSimplePredicate _sp(*_expr1, *_expr2, LIKE);
        _factory.setObject((yyvsp[(1) - (3)]._predicate),&_sp,SimplePredicate);
        (yyval._predicate) = (yyvsp[(1) - (3)]._predicate);
    ;}
    break;

  case 55:
#line 1173 "CQL.y"
    {
        CQL_globalParserState->currentRule = "expr_factor->comp";
        CQL_DEBUG_TRACE("BISON::expr_factor->comp\n");

        (yyval._predicate) = (yyvsp[(1) - (1)]._predicate);
    ;}
    break;

  case 56:
#line 1180 "CQL.y"
    {
        CQL_globalParserState->currentRule = "expr_factor->TOK_NOT comp";
        CQL_DEBUG_TRACE("BISON::expr_factor->TOK_NOT comp\n");

        (yyvsp[(2) - (2)]._predicate)->setInverted(!((yyvsp[(2) - (2)]._predicate)->getInverted()));
        (yyval._predicate) = (yyvsp[(2) - (2)]._predicate);
    ;}
    break;

  case 57:
#line 1190 "CQL.y"
    {
        CQL_globalParserState->currentRule = "expr_term->expr_factor";
        CQL_DEBUG_TRACE("BISON::expr_term->expr_factor\n");

        (yyval._predicate) = (yyvsp[(1) - (1)]._predicate);
    ;}
    break;

  case 58:
#line 1197 "CQL.y"
    {
        CQL_globalParserState->currentRule =
            "expr_term->expr_term AND expr_factor";
        CQL_DEBUG_TRACE("BISON::expr_term->expr_term AND expr_factor\n");

        (yyval._predicate) = new CQLPredicate();
        (yyval._predicate)->appendPredicate(*(yyvsp[(1) - (3)]._predicate));
        (yyval._predicate)->appendPredicate(*(yyvsp[(3) - (3)]._predicate), AND);
         _ObjPtr._ptr = (yyval._predicate);
        _ObjPtr.type = Pred;
        _ptrs.append(_ObjPtr);
    ;}
    break;

  case 59:
#line 1212 "CQL.y"
    {
        CQL_globalParserState->currentRule = "expr->expr_term";
        CQL_DEBUG_TRACE("BISON::expr->expr_term\n");

        (yyval._predicate) = (yyvsp[(1) - (1)]._predicate);
        ;}
    break;

  case 60:
#line 1219 "CQL.y"
    {
        CQL_globalParserState->currentRule = "expr->expr OR expr_term";
        CQL_DEBUG_TRACE("BISON::expr->expr OR expr_term\n");

        (yyval._predicate) = new CQLPredicate();
        (yyval._predicate)->appendPredicate(*(yyvsp[(1) - (3)]._predicate));
        (yyval._predicate)->appendPredicate(*(yyvsp[(3) - (3)]._predicate), OR);
        _ObjPtr._ptr = (yyval._predicate);
        _ObjPtr.type = Pred;
        _ptrs.append(_ObjPtr);
    ;}
    break;

  case 61:
#line 1232 "CQL.y"
    {;;}
    break;

  case 62:
#line 1252 "CQL.y"
    {
        CQL_globalParserState->currentRule = "arg_list->arg_list_sub->expr";
        CQL_DEBUG_TRACE("BISON::arg_list->arg_list_sub->expr\n");

        _arglist.append(*(yyvsp[(1) - (1)]._predicate));
        /*
        since arg_list can loop back on itself,
        we need to store away previous solutions
        production.  We keep track of previous productions
        in the _arglist array and later pass that to CQLFunction
        as part of chain: identifier TOK_LPAR arg_list TOK_RPAR
        */
    ;}
    break;

  case 63:
#line 1311 "CQL.y"
    {
        CQL_globalParserState->currentRule = "from_specifier->class_path";
        CQL_DEBUG_TRACE("BISON::from_specifier->class_path\n");

        CQL_globalParserState->statement->appendClassPath(*(yyvsp[(1) - (1)]._identifier));
    ;}
    break;

  case 64:
#line 1318 "CQL.y"
    {
        CQL_globalParserState->currentRule =
            "from_specifier->class_path TOK_AS identifier";
        CQL_DEBUG_TRACE(
            "BISON::from_specifier->class_path TOK_AS identifier\n");

        CQLIdentifier _class(*(yyvsp[(1) - (3)]._identifier));
        String _alias((yyvsp[(3) - (3)]._identifier)->getName().getString());
        CQL_globalParserState->statement->insertClassPathAlias(_class,_alias);
        CQL_globalParserState->statement->appendClassPath(_class);
    ;}
    break;

  case 65:
#line 1330 "CQL.y"
    {
        CQL_globalParserState->currentRule =
            "from_specifier->class_path identifier";
        CQL_DEBUG_TRACE("BISON::from_specifier->class_path identifier\n");

        CQLIdentifier _class(*(yyvsp[(1) - (2)]._identifier));
        String _alias((yyvsp[(2) - (2)]._identifier)->getName().getString());
        CQL_globalParserState->statement->insertClassPathAlias(_class,_alias);
        CQL_globalParserState->statement->appendClassPath(_class);
    ;}
    break;

  case 66:
#line 1343 "CQL.y"
    {
        CQL_globalParserState->currentRule = "from_criteria->from_specifier";
        CQL_DEBUG_TRACE("BISON::from_criteria->from_specifier\n");
    ;}
    break;

  case 67:
#line 1350 "CQL.y"
    {
        CQL_globalParserState->currentRule = "star_expr->TOK_STAR";
        CQL_DEBUG_TRACE("BISON::star_expr->TOK_STAR\n");

        CQLIdentifier _id("*");
        (yyval._chainedIdentifier) = (CQLChainedIdentifier*)
             (_factory.makeObject(&_id,ChainedIdentifier));
    ;}
    break;

  case 68:
#line 1359 "CQL.y"
    {
        CQL_globalParserState->currentRule = "star_expr->chain.*";
        CQL_DEBUG_TRACE("BISON::star_expr->chain.*\n");

        CQLChainedIdentifier* _tmp = (CQLChainedIdentifier*)
        (_factory.getObject((yyvsp[(1) - (3)]._node),Predicate,ChainedIdentifier));
        CQLChainedIdentifier* _cid = new CQLChainedIdentifier(*_tmp);
        CQLIdentifier _id("*");
        _cid->append(_id);
        (yyval._chainedIdentifier) = _cid;
        _ObjPtr._ptr = (yyval._chainedIdentifier);
        _ObjPtr.type = CId;
        _ptrs.append(_ObjPtr);
    ;}
    break;

  case 69:
#line 1376 "CQL.y"
    {
        CQL_globalParserState->currentRule = "selected_entry->expr";
        CQL_DEBUG_TRACE("BISON::selected_entry->expr\n");

        if((yyvsp[(1) - (1)]._predicate)->isSimpleValue())
        {
            CQLChainedIdentifier *_cid = (CQLChainedIdentifier*)
                (_factory.getObject((yyvsp[(1) - (1)]._predicate),Predicate,ChainedIdentifier));
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
    ;}
    break;

  case 70:
#line 1398 "CQL.y"
    {
        CQL_globalParserState->currentRule = "selected_entry->star_expr";
        CQL_DEBUG_TRACE("BISON::selected_entry->star_expr\n");

        CQL_globalParserState->statement->appendSelectIdentifier(*(yyvsp[(1) - (1)]._chainedIdentifier));
    ;}
    break;

  case 71:
#line 1407 "CQL.y"
    {
        CQL_globalParserState->currentRule =
            "select_list->selected_entry select_list_tail";
        CQL_DEBUG_TRACE(
            "BISON::select_list->selected_entry select_list_tail\n");
    ;}
    break;

  case 72:
#line 1415 "CQL.y"
    {;;}
    break;

  case 73:
#line 1418 "CQL.y"
    {
        CQL_globalParserState->currentRule =
            "select_list_tail->TOK_COMMA selected_entry select_list_tail";
        CQL_DEBUG_TRACE(
           "BISON::select_list_tail->TOK_COMMA"
           " selected_entry select_list_tail\n");
    ;}
    break;

  case 74:
#line 1428 "CQL.y"
    {
        CQL_globalParserState->currentRule = "search_condition->expr";
        CQL_DEBUG_TRACE("BISON::search_condition->expr\n");

        CQL_globalParserState->statement->setPredicate(*(yyvsp[(1) - (1)]._predicate));
    ;}
    break;

  case 75:
#line 1436 "CQL.y"
    {;}
    break;

  case 76:
#line 1439 "CQL.y"
    {
        CQL_globalParserState->currentRule =
            "optional_where->TOK_WHERE search_condition";
        CQL_DEBUG_TRACE(
            "BISON::optional_where->TOK_WHERE search_condition\n");

        CQL_globalParserState->statement->setHasWhereClause();
    ;}
    break;

  case 77:
#line 1450 "CQL.y"
    {
        CQL_globalParserState->currentRule = "select_statement";
        CQL_DEBUG_TRACE("select_statement\n");

        CQL_Bison_Cleanup();
    ;}
    break;


/* Line 1267 of yacc.c.  */
#line 2893 "CQLtemp"
      default: break;
    }
  YY_SYMBOL_PRINT ("-> $$ =", yyr1[yyn], &yyval, &yyloc);

  YYPOPSTACK (yylen);
  yylen = 0;
  YY_STACK_PRINT (yyss, yyssp);

  *++yyvsp = yyval;


  /* Now `shift' the result of the reduction.  Determine what state
     that goes to, based on the state we popped back to and the rule
     number reduced by.  */

  yyn = yyr1[yyn];

  yystate = yypgoto[yyn - YYNTOKENS] + *yyssp;
  if (0 <= yystate && yystate <= YYLAST && yycheck[yystate] == *yyssp)
    yystate = yytable[yystate];
  else
    yystate = yydefgoto[yyn - YYNTOKENS];

  goto yynewstate;


/*------------------------------------.
| yyerrlab -- here on detecting error |
`------------------------------------*/
yyerrlab:
  /* If not already recovering from an error, report this error.  */
  if (!yyerrstatus)
    {
      ++yynerrs;
#if ! YYERROR_VERBOSE
      yyerror (YY_("syntax error"));
#else
      {
	YYSIZE_T yysize = yysyntax_error (0, yystate, yychar);
	if (yymsg_alloc < yysize && yymsg_alloc < YYSTACK_ALLOC_MAXIMUM)
	  {
	    YYSIZE_T yyalloc = 2 * yysize;
	    if (! (yysize <= yyalloc && yyalloc <= YYSTACK_ALLOC_MAXIMUM))
	      yyalloc = YYSTACK_ALLOC_MAXIMUM;
	    if (yymsg != yymsgbuf)
	      YYSTACK_FREE (yymsg);
	    yymsg = (char *) YYSTACK_ALLOC (yyalloc);
	    if (yymsg)
	      yymsg_alloc = yyalloc;
	    else
	      {
		yymsg = yymsgbuf;
		yymsg_alloc = sizeof yymsgbuf;
	      }
	  }

	if (0 < yysize && yysize <= yymsg_alloc)
	  {
	    (void) yysyntax_error (yymsg, yystate, yychar);
	    yyerror (yymsg);
	  }
	else
	  {
	    yyerror (YY_("syntax error"));
	    if (yysize != 0)
	      goto yyexhaustedlab;
	  }
      }
#endif
    }



  if (yyerrstatus == 3)
    {
      /* If just tried and failed to reuse look-ahead token after an
	 error, discard it.  */

      if (yychar <= YYEOF)
	{
	  /* Return failure if at end of input.  */
	  if (yychar == YYEOF)
	    YYABORT;
	}
      else
	{
	  yydestruct ("Error: discarding",
		      yytoken, &yylval);
	  yychar = YYEMPTY;
	}
    }

  /* Else will try to reuse look-ahead token after shifting the error
     token.  */
  goto yyerrlab1;


/*---------------------------------------------------.
| yyerrorlab -- error raised explicitly by YYERROR.  |
`---------------------------------------------------*/
yyerrorlab:

  /* Pacify compilers like GCC when the user code never invokes
     YYERROR and the label yyerrorlab therefore never appears in user
     code.  */
  if (/*CONSTCOND*/ 0)
     goto yyerrorlab;

  /* Do not reclaim the symbols of the rule which action triggered
     this YYERROR.  */
  YYPOPSTACK (yylen);
  yylen = 0;
  YY_STACK_PRINT (yyss, yyssp);
  yystate = *yyssp;
  goto yyerrlab1;


/*-------------------------------------------------------------.
| yyerrlab1 -- common code for both syntax error and YYERROR.  |
`-------------------------------------------------------------*/
yyerrlab1:
  yyerrstatus = 3;	/* Each real token shifted decrements this.  */

  for (;;)
    {
      yyn = yypact[yystate];
      if (yyn != YYPACT_NINF)
	{
	  yyn += YYTERROR;
	  if (0 <= yyn && yyn <= YYLAST && yycheck[yyn] == YYTERROR)
	    {
	      yyn = yytable[yyn];
	      if (0 < yyn)
		break;
	    }
	}

      /* Pop the current state because it cannot handle the error token.  */
      if (yyssp == yyss)
	YYABORT;


      yydestruct ("Error: popping",
		  yystos[yystate], yyvsp);
      YYPOPSTACK (1);
      yystate = *yyssp;
      YY_STACK_PRINT (yyss, yyssp);
    }

  if (yyn == YYFINAL)
    YYACCEPT;

  *++yyvsp = yylval;


  /* Shift the error token.  */
  YY_SYMBOL_PRINT ("Shifting", yystos[yyn], yyvsp, yylsp);

  yystate = yyn;
  goto yynewstate;


/*-------------------------------------.
| yyacceptlab -- YYACCEPT comes here.  |
`-------------------------------------*/
yyacceptlab:
  yyresult = 0;
  goto yyreturn;

/*-----------------------------------.
| yyabortlab -- YYABORT comes here.  |
`-----------------------------------*/
yyabortlab:
  yyresult = 1;
  goto yyreturn;

#ifndef yyoverflow
/*-------------------------------------------------.
| yyexhaustedlab -- memory exhaustion comes here.  |
`-------------------------------------------------*/
yyexhaustedlab:
  yyerror (YY_("memory exhausted"));
  yyresult = 2;
  /* Fall through.  */
#endif

yyreturn:
  if (yychar != YYEOF && yychar != YYEMPTY)
     yydestruct ("Cleanup: discarding lookahead",
		 yytoken, &yylval);
  /* Do not reclaim the symbols of the rule which action triggered
     this YYABORT or YYACCEPT.  */
  YYPOPSTACK (yylen);
  YY_STACK_PRINT (yyss, yyssp);
  while (yyssp != yyss)
    {
      yydestruct ("Cleanup: popping",
		  yystos[*yyssp], yyvsp);
      YYPOPSTACK (1);
    }
#ifndef yyoverflow
  if (yyss != yyssa)
    YYSTACK_FREE (yyss);
#endif
#if YYERROR_VERBOSE
  if (yymsg != yymsgbuf)
    YYSTACK_FREE (yymsg);
#endif
  /* Make sure YYID is used.  */
  return YYID (yyresult);
}


#line 1459 "CQL.y"

/*int yyerror(char * err){yyclearin; yyerrok;throw Exception(String(err));
return 1;}*/

