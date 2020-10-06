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
#define yyparse cimmof_parse
#define yylex   cimmof_lex
#define yyerror cimmof_error
#define yylval  cimmof_lval
#define yychar  cimmof_char
#define yydebug cimmof_debug
#define yynerrs cimmof_nerrs


/* Tokens.  */
#ifndef YYTOKENTYPE
# define YYTOKENTYPE
   /* Put the tokens into the symbol table, so that GDB and other debuggers
      know about them.  */
   enum yytokentype {
     TOK_ALIAS_IDENTIFIER = 258,
     TOK_ANY = 259,
     TOK_AS = 260,
     TOK_ASSOCIATION = 261,
     TOK_BINARY_VALUE = 262,
     TOK_CHAR_VALUE = 263,
     TOK_CLASS = 264,
     TOK_COLON = 265,
     TOK_COMMA = 266,
     TOK_DISABLEOVERRIDE = 267,
     TOK_DQUOTE = 268,
     TOK_DT_BOOL = 269,
     TOK_DT_CHAR16 = 270,
     TOK_DT_CHAR8 = 271,
     TOK_DT_DATETIME = 272,
     TOK_DT_REAL32 = 273,
     TOK_DT_REAL64 = 274,
     TOK_DT_SINT16 = 275,
     TOK_DT_SINT32 = 276,
     TOK_DT_SINT64 = 277,
     TOK_DT_SINT8 = 278,
     TOK_DT_STR = 279,
     TOK_DT_UINT16 = 280,
     TOK_DT_UINT32 = 281,
     TOK_DT_UINT64 = 282,
     TOK_DT_UINT8 = 283,
     TOK_ENABLEOVERRIDE = 284,
     TOK_END_OF_FILE = 285,
     TOK_EQUAL = 286,
     TOK_FALSE = 287,
     TOK_FLAVOR = 288,
     TOK_HEX_VALUE = 289,
     TOK_INCLUDE = 290,
     TOK_INDICATION = 291,
     TOK_INSTANCE = 292,
     TOK_LEFTCURLYBRACE = 293,
     TOK_LEFTPAREN = 294,
     TOK_LEFTSQUAREBRACKET = 295,
     TOK_METHOD = 296,
     TOK_NULL_VALUE = 297,
     TOK_OCTAL_VALUE = 298,
     TOK_OF = 299,
     TOK_PARAMETER = 300,
     TOK_PERIOD = 301,
     TOK_POSITIVE_DECIMAL_VALUE = 302,
     TOK_PRAGMA = 303,
     TOK_PROPERTY = 304,
     TOK_QUALIFIER = 305,
     TOK_REAL_VALUE = 306,
     TOK_REF = 307,
     TOK_REFERENCE = 308,
     TOK_RESTRICTED = 309,
     TOK_RIGHTCURLYBRACE = 310,
     TOK_RIGHTPAREN = 311,
     TOK_RIGHTSQUAREBRACKET = 312,
     TOK_SCOPE = 313,
     TOK_SEMICOLON = 314,
     TOK_SIGNED_DECIMAL_VALUE = 315,
     TOK_SIMPLE_IDENTIFIER = 316,
     TOK_STRING_VALUE = 317,
     TOK_TOSUBCLASS = 318,
     TOK_TRANSLATABLE = 319,
     TOK_TRUE = 320,
     TOK_UNEXPECTED_CHAR = 321
   };
#endif
/* Tokens.  */
#define TOK_ALIAS_IDENTIFIER 258
#define TOK_ANY 259
#define TOK_AS 260
#define TOK_ASSOCIATION 261
#define TOK_BINARY_VALUE 262
#define TOK_CHAR_VALUE 263
#define TOK_CLASS 264
#define TOK_COLON 265
#define TOK_COMMA 266
#define TOK_DISABLEOVERRIDE 267
#define TOK_DQUOTE 268
#define TOK_DT_BOOL 269
#define TOK_DT_CHAR16 270
#define TOK_DT_CHAR8 271
#define TOK_DT_DATETIME 272
#define TOK_DT_REAL32 273
#define TOK_DT_REAL64 274
#define TOK_DT_SINT16 275
#define TOK_DT_SINT32 276
#define TOK_DT_SINT64 277
#define TOK_DT_SINT8 278
#define TOK_DT_STR 279
#define TOK_DT_UINT16 280
#define TOK_DT_UINT32 281
#define TOK_DT_UINT64 282
#define TOK_DT_UINT8 283
#define TOK_ENABLEOVERRIDE 284
#define TOK_END_OF_FILE 285
#define TOK_EQUAL 286
#define TOK_FALSE 287
#define TOK_FLAVOR 288
#define TOK_HEX_VALUE 289
#define TOK_INCLUDE 290
#define TOK_INDICATION 291
#define TOK_INSTANCE 292
#define TOK_LEFTCURLYBRACE 293
#define TOK_LEFTPAREN 294
#define TOK_LEFTSQUAREBRACKET 295
#define TOK_METHOD 296
#define TOK_NULL_VALUE 297
#define TOK_OCTAL_VALUE 298
#define TOK_OF 299
#define TOK_PARAMETER 300
#define TOK_PERIOD 301
#define TOK_POSITIVE_DECIMAL_VALUE 302
#define TOK_PRAGMA 303
#define TOK_PROPERTY 304
#define TOK_QUALIFIER 305
#define TOK_REAL_VALUE 306
#define TOK_REF 307
#define TOK_REFERENCE 308
#define TOK_RESTRICTED 309
#define TOK_RIGHTCURLYBRACE 310
#define TOK_RIGHTPAREN 311
#define TOK_RIGHTSQUAREBRACKET 312
#define TOK_SCOPE 313
#define TOK_SEMICOLON 314
#define TOK_SIGNED_DECIMAL_VALUE 315
#define TOK_SIMPLE_IDENTIFIER 316
#define TOK_STRING_VALUE 317
#define TOK_TOSUBCLASS 318
#define TOK_TRANSLATABLE 319
#define TOK_TRUE 320
#define TOK_UNEXPECTED_CHAR 321




/* Copy the first part of user declarations.  */
#line 29 "cimmof.y"


  /* Flex grammar created from CIM Specification Version 2.2 Appendix A */

  /*
     Note the following implementation details:

       1. The MOF specification has a production of type assocDeclaration,
       but an association is just a type of classDeclaration with a few
       special rules.  At least for the first pass, I'm treating an
       associationDeclaration as a classDeclaration and applying its
       syntactical rules outside of the grammar definition.

       2. Same with the indicationDeclaration.  It appears to be a normal
       classDeclaration with the INDICATION qualifier and no special
       syntactical rules.

       3. The Parser uses String objects throughout to represent
       character data.  However, the tokenizer underneath is probably
       working with 8-bit chars.  If we later use an extended character
       compatible tokenizer, I anticipate NO CHANGE to this parser.

       4. Besides the tokenizer, this parser uses 2 sets of outside
       services:
          1)Class valueFactory.  This has a couple of static methods
      that assist in creating CIMValue objects from Strings.
      2)Class cimmofParser.  This has a wide variety of methods
      that fall into these catagories:
            a) Interfaces to the Repository.  You call cimmofParser::
            methods to query and store compiled CIM elements.
        b) Error handling.
            c) Data format conversions.
            d) Tokenizer manipulation
            e) Pragma handling
            f) Alias Handling
  */


#define YYSTACKSIZE 2000

#include <cstdlib>
#if !defined(PEGASUS_OS_ZOS) && !defined(PEGASUS_OS_VMS)
#if defined(PEGASUS_OS_DARWIN)
#include <sys/malloc.h>
#else
#include <malloc.h>
#endif
#endif
#include <cstdio>
#include <cstring>
#include <Pegasus/Common/String.h>
#include <Pegasus/Common/CIMName.h>
#include <Pegasus/Common/StringConversion.h>
#include "cimmofParser.h"
#include "valueFactory.h"
#include "memobjs.h"
#include <Pegasus/Common/CIMQualifierList.h>
#include <Pegasus/Common/AutoPtr.h>

//include any useful debugging stuff here

/* Debugging the parser.  Debugging is provided through
   1. debug functions in Bison that are controlled by a compile time
      flag (YYDEBUG) and a runtime flag (yydebug) which is redefined
      to cimmof_debug.
   2. Debug functions defined through YACCTRACE, a macro defined
      in cimmofparser.h and turned on and off manually.
   All debugging must be turned on manually at this point by
   setting the YYDEBUG compile flag and also setting YACCTRACE.
*/
// Enable this define to compile Bison/Yacc tracing
// #define YYDEBUG 1

extern int   cimmof_lex(void);
extern int   cimmof_error(...);
extern char *cimmof_text;
extern void  cimmof_yy_less(int n);
extern int   cimmof_leng;
extern char * metaQualifierName;


/* ------------------------------------------------------------------- */
/* These globals provide continuity between various pieces of a        */
/* declaration.  They are usually interpreted as "these modifiers were */
/* encountered and need to be applied to the finished object".  For    */
/* example, qualifiers are accumulated in g_qualifierList[] as they    */
/* encountered, then applied to the production they qualify when it    */
/* is completed.                                                       */
/* ------------------------------------------------------------------- */
  CIMFlavor g_flavor = CIMFlavor (CIMFlavor::NONE);
  CIMScope g_scope = CIMScope ();
  CIMQualifierList g_qualifierList;
  CIMMethod *g_currentMethod = 0;
  CIMClass *g_currentClass = 0;
  CIMInstance *g_currentInstance = 0;
  String g_currentAliasRef; // Alias reference
  String g_currentAliasDecl; // Alias declaration
  CIMName g_referenceClassName = CIMName();
  Array<CIMKeyBinding> g_KeyBindingArray; // it gets created empty
  TYPED_INITIALIZER_VALUE g_typedInitializerValue;
  int g_strValType;

/* ------------------------------------------------------------------- */
/* Pragmas, except for the Include pragma, are not handled yet         */
/* I don't understand them, so it may be a while before they are       */
/* ------------------------------------------------------------------- */
  struct pragma {
    String name;
    String value;
  };

/* ---------------------------------------------------------------- */
/* Use our general wrap manager to handle end-of-file               */
/* ---------------------------------------------------------------- */
extern "C" {
int
cimmof_wrap() {
  return cimmofParser::Instance()->wrapCurrentBuffer();
}
}

/* ---------------------------------------------------------------- */
/* Pass errors to our general log manager.                          */
/* Appends the current text and msg parameter                       */
/* ---------------------------------------------------------------- */
void cimmof_error(const char *msg) {
  cimmofParser::Instance()->log_parse_error(cimmof_text, msg);
  // printf("Error: %s\n", msg);
}

static void MOF_error(const char * str, const char * S);

#ifdef DEBUG_cimmof
static void MOF_trace(const char* str);
static void MOF_trace2(const char * str, const char * S);
#else
#define MOF_Trace(X)
#define MOF_trace2(X, Y)
#endif



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
#line 176 "cimmof.y"
{
  //char                     *strval;
  CIMClass                 *mofclass;
  CIMFlavor                *flavor;
  CIMInstance              *instance;
  CIMKeyBinding            *keybinding;
  CIMMethod                *method;
  CIMName                  *cimnameval;
  CIMObjectPath            *reference;
  CIMProperty              *property;
  CIMQualifier             *qualifier;
  CIMQualifierDecl         *mofqualifier;
  CIMScope                 *scope;
  CIMType                   datatype;
  CIMValue                 *value;
  int                       ival;
  CIMObjectPath            *modelpath;
  String                   *strptr;
  String                   *strval;
  struct pragma            *pragma;
  TYPED_INITIALIZER_VALUE  *typedinitializer;
}
/* Line 187 of yacc.c.  */
#line 401 "cimmoftemp"
	YYSTYPE;
# define yystype YYSTYPE /* obsolescent; will be withdrawn */
# define YYSTYPE_IS_DECLARED 1
# define YYSTYPE_IS_TRIVIAL 1
#endif



/* Copy the second part of user declarations.  */


/* Line 216 of yacc.c.  */
#line 414 "cimmoftemp"

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
#define YYFINAL  25
/* YYLAST -- Last index in YYTABLE.  */
#define YYLAST   282

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  67
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  84
/* YYNRULES -- Number of rules.  */
#define YYNRULES  162
/* YYNRULES -- Number of states.  */
#define YYNSTATES  240

/* YYTRANSLATE(YYLEX) -- Bison symbol number corresponding to YYLEX.  */
#define YYUNDEFTOK  2
#define YYMAXUTOK   321

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
      55,    56,    57,    58,    59,    60,    61,    62,    63,    64,
      65,    66
};

#if YYDEBUG
/* YYPRHS[YYN] -- Index of the first RHS symbol of rule number YYN in
   YYRHS.  */
static const yytype_uint16 yyprhs[] =
{
       0,     0,     3,     5,     8,     9,    11,    13,    15,    17,
      20,    26,    28,    31,    32,    37,    41,    43,    46,    48,
      50,    52,    57,    60,    64,    66,    68,    70,    74,    75,
      80,    82,    84,    88,    93,    95,   102,   104,   106,   109,
     110,   113,   115,   117,   121,   124,   125,   128,   129,   131,
     133,   135,   137,   139,   141,   143,   145,   149,   151,   153,
     155,   157,   159,   161,   163,   165,   167,   169,   171,   173,
     175,   177,   179,   182,   184,   188,   191,   193,   195,   200,
     202,   205,   206,   208,   212,   214,   218,   222,   224,   227,
     228,   230,   233,   239,   244,   246,   249,   255,   257,   259,
     265,   267,   273,   280,   285,   289,   290,   294,   296,   300,
     302,   304,   306,   308,   310,   312,   314,   316,   321,   322,
     325,   327,   331,   333,   335,   337,   339,   341,   343,   344,
     346,   349,   351,   353,   355,   357,   359,   361,   363,   365,
     367,   369,   371,   373,   375,   377,   379,   381,   385,   386,
     388,   390,   394,   398,   400,   402,   404,   406,   410,   414,
     416,   417,   419
};

/* YYRHS -- A `-1'-separated list of the rules' RHS.  */
static const yytype_int16 yyrhs[] =
{
      68,     0,    -1,    69,    -1,    70,    69,    -1,    -1,   124,
      -1,   128,    -1,    71,    -1,   119,    -1,    72,    75,    -1,
     143,     9,    73,   117,    74,    -1,    61,    -1,    10,    73,
      -1,    -1,    38,    76,    55,    59,    -1,    38,    55,    59,
      -1,    77,    -1,    76,    77,    -1,    86,    -1,    78,    -1,
      89,    -1,   143,    79,    80,    81,    -1,   140,    82,    -1,
      39,    83,    56,    -1,    59,    -1,    61,    -1,    84,    -1,
      83,    11,    84,    -1,    -1,   143,    85,    94,    96,    -1,
     140,    -1,    93,    -1,   143,    87,    88,    -1,   140,    95,
      96,    97,    -1,    59,    -1,   143,    90,    52,    91,    92,
      59,    -1,    61,    -1,    61,    -1,    31,   106,    -1,    -1,
      73,    52,    -1,    61,    -1,    61,    -1,    40,    47,    57,
      -1,    40,    57,    -1,    -1,    31,    99,    -1,    -1,   101,
      -1,   107,    -1,   108,    -1,   102,    -1,    42,    -1,   107,
      -1,   108,    -1,   101,    -1,   100,    11,   101,    -1,   102,
      -1,    42,    -1,   103,    -1,    51,    -1,     8,    -1,   105,
      -1,   104,    -1,    47,    -1,    43,    -1,    34,    -1,     7,
      -1,    60,    -1,    32,    -1,    65,    -1,   106,    -1,   105,
     106,    -1,    62,    -1,    38,   100,    55,    -1,    38,    55,
      -1,   109,    -1,   110,    -1,    13,   111,   113,    13,    -1,
     118,    -1,   112,    10,    -1,    -1,   106,    -1,    73,    46,
     114,    -1,   115,    -1,   114,    11,   115,    -1,   116,    31,
      98,    -1,    61,    -1,     5,   118,    -1,    -1,     3,    -1,
     120,   121,    -1,   143,    37,    44,    73,   117,    -1,    38,
     122,    55,    59,    -1,   123,    -1,   122,   123,    -1,   143,
      61,    31,    99,    59,    -1,   125,    -1,   127,    -1,    48,
      35,    39,   126,    56,    -1,   106,    -1,    48,   149,    39,
     150,    56,    -1,    50,   147,   129,   130,   134,    59,    -1,
      10,   140,    96,    97,    -1,   131,   132,    56,    -1,    -1,
      11,    58,    39,    -1,   133,    -1,   132,    11,   133,    -1,
       9,    -1,     6,    -1,    36,    -1,    49,    -1,    53,    -1,
      41,    -1,    45,    -1,     4,    -1,    11,   135,   136,    56,
      -1,    -1,    33,    39,    -1,   137,    -1,   136,    11,   137,
      -1,    29,    -1,    12,    -1,    54,    -1,    63,    -1,    64,
      -1,   139,    -1,    -1,   137,    -1,   139,   137,    -1,   141,
      -1,   142,    -1,    24,    -1,    14,    -1,    17,    -1,    28,
      -1,    23,    -1,    25,    -1,    20,    -1,    26,    -1,    21,
      -1,    27,    -1,    22,    -1,    15,    -1,    18,    -1,    19,
      -1,   144,   145,    57,    -1,    -1,    40,    -1,   146,    -1,
     145,    11,   146,    -1,   147,   148,   138,    -1,     6,    -1,
      36,    -1,    53,    -1,    61,    -1,    39,   102,    56,    -1,
      39,    42,    56,    -1,   107,    -1,    -1,    61,    -1,    62,
      -1
};

/* YYRLINE[YYN] -- source line where rule number YYN was defined.  */
static const yytype_uint16 yyrline[] =
{
       0,   311,   311,   314,   315,   319,   320,   325,   329,   341,
     353,   375,   378,   384,   389,   390,   393,   394,   397,   403,
     408,   426,   436,   454,   456,   458,   470,   471,   472,   478,
     507,   508,   522,   538,   563,   576,   601,   603,   605,   606,
     609,   616,   623,   640,   647,   652,   660,   662,   674,   676,
     678,   697,   705,   710,   717,   735,   739,   759,   761,   780,
     785,   790,   795,   799,   806,   807,   808,   809,   810,   817,
     819,   825,   826,   832,   871,   875,   881,   882,   885,   906,
     937,   939,   942,   945,   963,   965,   973,  1000,  1003,  1013,
    1019,  1030,  1051,  1070,  1075,  1076,  1082,  1139,  1143,  1149,
    1156,  1159,  1186,  1198,  1213,  1218,  1224,  1231,  1235,  1250,
    1251,  1254,  1257,  1258,  1259,  1260,  1261,  1271,  1274,  1280,
    1286,  1287,  1296,  1298,  1299,  1300,  1301,  1304,  1309,  1314,
    1315,  1320,  1321,  1322,  1323,  1324,  1327,  1328,  1329,  1330,
    1331,  1332,  1333,  1334,  1335,  1338,  1340,  1350,  1352,  1356,
    1360,  1362,  1370,  1389,  1398,  1408,  1413,  1422,  1429,  1434,
    1441,  1447,  1450
};
#endif

#if YYDEBUG || YYERROR_VERBOSE || YYTOKEN_TABLE
/* YYTNAME[SYMBOL-NUM] -- String name of the symbol SYMBOL-NUM.
   First, the terminals, then, starting at YYNTOKENS, nonterminals.  */
static const char *const yytname[] =
{
  "$end", "error", "$undefined", "TOK_ALIAS_IDENTIFIER", "TOK_ANY",
  "TOK_AS", "TOK_ASSOCIATION", "TOK_BINARY_VALUE", "TOK_CHAR_VALUE",
  "TOK_CLASS", "TOK_COLON", "TOK_COMMA", "TOK_DISABLEOVERRIDE",
  "TOK_DQUOTE", "TOK_DT_BOOL", "TOK_DT_CHAR16", "TOK_DT_CHAR8",
  "TOK_DT_DATETIME", "TOK_DT_REAL32", "TOK_DT_REAL64", "TOK_DT_SINT16",
  "TOK_DT_SINT32", "TOK_DT_SINT64", "TOK_DT_SINT8", "TOK_DT_STR",
  "TOK_DT_UINT16", "TOK_DT_UINT32", "TOK_DT_UINT64", "TOK_DT_UINT8",
  "TOK_ENABLEOVERRIDE", "TOK_END_OF_FILE", "TOK_EQUAL", "TOK_FALSE",
  "TOK_FLAVOR", "TOK_HEX_VALUE", "TOK_INCLUDE", "TOK_INDICATION",
  "TOK_INSTANCE", "TOK_LEFTCURLYBRACE", "TOK_LEFTPAREN",
  "TOK_LEFTSQUAREBRACKET", "TOK_METHOD", "TOK_NULL_VALUE",
  "TOK_OCTAL_VALUE", "TOK_OF", "TOK_PARAMETER", "TOK_PERIOD",
  "TOK_POSITIVE_DECIMAL_VALUE", "TOK_PRAGMA", "TOK_PROPERTY",
  "TOK_QUALIFIER", "TOK_REAL_VALUE", "TOK_REF", "TOK_REFERENCE",
  "TOK_RESTRICTED", "TOK_RIGHTCURLYBRACE", "TOK_RIGHTPAREN",
  "TOK_RIGHTSQUAREBRACKET", "TOK_SCOPE", "TOK_SEMICOLON",
  "TOK_SIGNED_DECIMAL_VALUE", "TOK_SIMPLE_IDENTIFIER", "TOK_STRING_VALUE",
  "TOK_TOSUBCLASS", "TOK_TRANSLATABLE", "TOK_TRUE", "TOK_UNEXPECTED_CHAR",
  "$accept", "mofSpec", "mofProductions", "mofProduction",
  "classDeclaration", "classHead", "className", "superClass", "classBody",
  "classFeatures", "classFeature", "methodDeclaration", "methodHead",
  "methodBody", "methodEnd", "methodName", "parameterList", "parameter",
  "parameterType", "propertyDeclaration", "propertyBody", "propertyEnd",
  "referenceDeclaration", "referencedObject", "referenceName",
  "referencePath", "objectRef", "parameterName", "propertyName", "array",
  "typedDefaultValue", "initializer", "typedInitializer",
  "constantValueList", "constantValue", "nonNullConstantValue",
  "integerValue", "booleanValue", "stringValues", "stringValue",
  "arrayInitializer", "referenceInitializer", "objectHandle",
  "aliasInitializer", "namespaceHandleRef", "namespaceHandle", "modelPath",
  "keyValuePairList", "keyValuePair", "keyValuePairName", "alias",
  "aliasIdentifier", "instanceDeclaration", "instanceHead", "instanceBody",
  "valueInitializerList", "valueInitializer", "compilerDirective",
  "compilerDirectiveInclude", "fileName", "compilerDirectivePragma",
  "qualifierDeclaration", "qualifierType", "scope", "scope_begin",
  "metaElements", "metaElement", "defaultFlavor", "flavorHead",
  "explicitFlavors", "explicitFlavor", "flavor", "overrideFlavors",
  "dataType", "intDataType", "realDataType", "qualifierList",
  "qualifierListBegin", "qualifiers", "qualifier", "qualifierName",
  "typedQualifierParameter", "pragmaName", "pragmaVal", 0
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
     315,   316,   317,   318,   319,   320,   321
};
# endif

/* YYR1[YYN] -- Symbol number of symbol that rule YYN derives.  */
static const yytype_uint8 yyr1[] =
{
       0,    67,    68,    69,    69,    70,    70,    70,    70,    71,
      72,    73,    74,    74,    75,    75,    76,    76,    77,    77,
      77,    78,    79,    80,    81,    82,    83,    83,    83,    84,
      85,    85,    86,    87,    88,    89,    90,    91,    92,    92,
      93,    94,    95,    96,    96,    96,    97,    97,    98,    98,
      98,    99,    99,    99,    99,   100,   100,   101,   101,   102,
     102,   102,   102,   102,   103,   103,   103,   103,   103,   104,
     104,   105,   105,   106,   107,   107,   108,   108,   109,   110,
     111,   111,   112,   113,   114,   114,   115,   116,   117,   117,
     118,   119,   120,   121,   122,   122,   123,   124,   124,   125,
     126,   127,   128,   129,   130,   130,   131,   132,   132,   133,
     133,   133,   133,   133,   133,   133,   133,   134,   134,   135,
     136,   136,   137,   137,   137,   137,   137,   138,   138,   139,
     139,   140,   140,   140,   140,   140,   141,   141,   141,   141,
     141,   141,   141,   141,   141,   142,   142,   143,   143,   144,
     145,   145,   146,   147,   147,   147,   147,   148,   148,   148,
     148,   149,   150
};

/* YYR2[YYN] -- Number of symbols composing right hand side of rule YYN.  */
static const yytype_uint8 yyr2[] =
{
       0,     2,     1,     2,     0,     1,     1,     1,     1,     2,
       5,     1,     2,     0,     4,     3,     1,     2,     1,     1,
       1,     4,     2,     3,     1,     1,     1,     3,     0,     4,
       1,     1,     3,     4,     1,     6,     1,     1,     2,     0,
       2,     1,     1,     3,     2,     0,     2,     0,     1,     1,
       1,     1,     1,     1,     1,     1,     3,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     2,     1,     3,     2,     1,     1,     4,     1,
       2,     0,     1,     3,     1,     3,     3,     1,     2,     0,
       1,     2,     5,     4,     1,     2,     5,     1,     1,     5,
       1,     5,     6,     4,     3,     0,     3,     1,     3,     1,
       1,     1,     1,     1,     1,     1,     1,     4,     0,     2,
       1,     3,     1,     1,     1,     1,     1,     1,     0,     1,
       2,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     3,     0,     1,
       1,     3,     3,     1,     1,     1,     1,     3,     3,     1,
       0,     1,     1
};

/* YYDEFACT[STATE-NAME] -- Default rule to reduce with in state
   STATE-NUM when YYTABLE doesn't specify something else to do.  Zero
   means the default is an error.  */
static const yytype_uint8 yydefact[] =
{
     148,   149,     0,     0,     0,     2,   148,     7,     0,     8,
       0,     5,    97,    98,     6,     0,     0,     0,   161,     0,
     153,   154,   155,   156,     0,     1,     3,   148,     9,   148,
      91,     0,     0,     0,   150,   160,     0,     0,     0,   105,
       0,   148,    16,    19,    18,    20,     0,   148,    94,     0,
      11,    89,     0,     0,   147,     0,     0,   159,   128,    73,
     100,     0,   162,     0,   134,   144,   135,   145,   146,   139,
     141,   143,   137,   133,   138,   140,   142,   136,    45,   131,
     132,     0,   118,     0,    15,     0,    17,    36,     0,     0,
       0,     0,     0,    95,     0,     0,    13,    89,   151,    67,
      61,    69,    66,    58,    65,    64,    60,    75,    68,    70,
       0,    55,    57,    59,    63,    62,    71,     0,     0,   123,
     122,   124,   125,   126,   129,   152,   127,    99,   101,     0,
      47,     0,     0,     0,   116,   110,   109,   111,   114,   115,
     112,   113,     0,   107,    14,   148,     0,    34,    32,     0,
      42,    22,    45,    93,     0,    90,    88,     0,    10,    92,
       0,    74,    72,   158,   157,   130,     0,    44,     0,   103,
     106,     0,     0,   102,     0,   104,     0,    26,     0,    24,
      21,    37,    39,    47,    81,    52,     0,    51,    53,    54,
      76,    77,    79,    12,    56,    43,    46,   119,     0,   120,
     108,   148,    23,     0,     0,    31,    30,     0,     0,    33,
      82,     0,     0,    96,     0,   117,    27,    40,    41,    45,
      38,    35,     0,     0,    80,   121,    29,     0,    78,    87,
      83,    84,     0,     0,     0,    85,    86,    48,    49,    50
};

/* YYDEFGOTO[NTERM-NUM].  */
static const yytype_int16 yydefgoto[] =
{
      -1,     4,     5,     6,     7,     8,    51,   158,    28,    41,
      42,    43,    88,   146,   180,   151,   176,   177,   204,    44,
      89,   148,    45,    90,   182,   208,   205,   219,   152,   130,
     169,   236,   186,   110,   111,   112,   113,   114,   115,   116,
     188,   189,   190,   191,   211,   212,   223,   230,   231,   232,
      96,   192,     9,    10,    30,    47,    48,    11,    12,    61,
      13,    14,    39,    82,    83,   142,   143,   133,   172,   198,
     124,   125,   126,    78,    79,    80,    15,    16,    33,    34,
      35,    58,    19,    63
};

/* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
   STATE-NUM.  */
#define YYPACT_NINF -153
static const yytype_int16 yypact[] =
{
      18,  -153,   -24,     4,    28,  -153,    18,  -153,    -9,  -153,
      -7,  -153,  -153,  -153,  -153,     5,     4,     7,  -153,    11,
    -153,  -153,  -153,  -153,    52,  -153,  -153,   -20,  -153,    23,
    -153,    17,    43,     2,  -153,    -5,    29,    31,   237,    83,
      37,   -16,  -153,  -153,  -153,  -153,   189,   -10,  -153,    36,
    -153,    93,    17,     4,  -153,    78,   119,  -153,    26,  -153,
    -153,    44,  -153,    46,  -153,  -153,  -153,  -153,  -153,  -153,
    -153,  -153,  -153,  -153,  -153,  -153,  -153,  -153,    59,  -153,
    -153,    47,    96,   184,  -153,    49,  -153,  -153,    72,    54,
      62,    55,    56,  -153,    86,   120,   112,    93,  -153,  -153,
    -153,  -153,  -153,  -153,  -153,  -153,  -153,  -153,  -153,  -153,
      12,  -153,  -153,  -153,  -153,    29,  -153,    68,    75,  -153,
    -153,  -153,  -153,  -153,  -153,  -153,    26,  -153,  -153,   -21,
      97,    95,    99,    76,  -153,  -153,  -153,  -153,  -153,  -153,
    -153,  -153,     8,  -153,  -153,    21,    77,  -153,  -153,    80,
     100,  -153,    59,  -153,     9,  -153,  -153,    17,  -153,  -153,
     140,  -153,  -153,  -153,  -153,  -153,    85,  -153,     9,  -153,
    -153,   105,    26,  -153,   184,  -153,    14,  -153,   221,  -153,
    -153,  -153,   114,    97,    29,  -153,    87,  -153,  -153,  -153,
    -153,  -153,  -153,  -153,  -153,  -153,  -153,  -153,    16,  -153,
    -153,    23,  -153,   102,    88,  -153,  -153,    29,    98,  -153,
    -153,    17,   142,  -153,    26,  -153,  -153,  -153,  -153,    59,
    -153,  -153,   109,   143,  -153,  -153,  -153,   103,  -153,  -153,
     147,  -153,   128,   103,    41,  -153,  -153,  -153,  -153,  -153
};

/* YYPGOTO[NTERM-NUM].  */
static const yytype_int16 yypgoto[] =
{
    -153,  -153,   154,  -153,  -153,  -153,   -48,  -153,  -153,  -153,
     124,  -153,  -153,  -153,  -153,  -153,  -153,   -33,  -153,  -153,
    -153,  -153,  -153,  -153,  -153,  -153,  -153,  -153,  -153,  -143,
     -14,  -153,    -1,  -153,  -152,   -50,  -153,  -153,  -153,   -34,
     -35,   -63,  -153,  -153,  -153,  -153,  -153,  -153,   -57,  -153,
      81,    82,  -153,  -153,  -153,  -153,   133,  -153,  -153,  -153,
    -153,  -153,  -153,  -153,  -153,  -153,    15,  -153,  -153,  -153,
    -119,  -153,  -153,   -41,  -153,  -153,   -26,  -153,  -153,   132,
     183,  -153,  -153,  -153
};

/* YYTABLE[YYPACT[STATE-NUM]].  What to do in state STATE-NUM.  If
   positive, shift that token.  If negative, reduce the rule which
   number is the opposite.  If zero, do what YYDEFACT says.
   If YYTABLE_NINF, syntax error.  */
#define YYTABLE_NINF -29
static const yytype_int16 yytable[] =
{
      57,    46,    60,    49,    97,    91,   118,   165,   194,   183,
      20,    17,   155,    53,    31,    46,    99,   100,    -4,   174,
       1,    49,   184,   160,     1,   201,   166,   214,    25,    27,
       1,    29,   -28,    55,    56,    40,   167,    18,   119,    85,
      21,   101,    32,   102,   155,    92,    36,    55,    99,   100,
      37,   185,   104,   199,   184,   120,   105,    22,     1,    54,
     106,     1,    38,     1,   175,    23,     2,   161,     3,   108,
     202,    59,   215,   101,   109,   102,   226,   -28,    50,    55,
     121,   162,   237,   103,   104,    99,   100,    52,   105,   122,
     123,    59,   106,    62,    81,   225,    84,    94,    95,   129,
     127,   108,   128,    59,   187,   131,   109,   132,   144,   193,
     101,   145,   102,   147,   149,   153,   150,   154,   187,   178,
     103,   104,   157,   155,   163,   105,    99,   100,   168,   106,
     203,   164,   171,   107,   170,   173,   179,   206,   108,   -25,
      59,   181,   195,   109,   197,   207,   213,    99,   100,   218,
     210,   101,   224,   102,   217,   227,   228,   221,   233,   234,
      26,   117,   104,   222,   229,    86,   105,   196,   216,   209,
     106,   239,   101,   220,   102,   178,   235,   156,   159,   108,
      93,    59,   103,   104,   109,    98,    24,   105,   134,   200,
     135,   106,     0,   136,     0,     0,     0,     0,     0,   238,
     108,     0,    59,    64,    65,   109,    66,    67,    68,    69,
      70,    71,    72,    73,    74,    75,    76,    77,     0,     0,
     137,     0,     0,     0,     0,   138,     0,     0,     0,   139,
       0,     0,     0,   140,     0,    64,    65,   141,    66,    67,
      68,    69,    70,    71,    72,    73,    74,    75,    76,    77,
      87,    64,    65,     0,    66,    67,    68,    69,    70,    71,
      72,    73,    74,    75,    76,    77,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    50
};

static const yytype_int16 yycheck[] =
{
      35,    27,    36,    29,    52,    46,    56,   126,   160,   152,
       6,    35,     3,    11,     9,    41,     7,     8,     0,    11,
      40,    47,    13,    11,    40,    11,    47,    11,     0,    38,
      40,    38,    11,    38,    39,    55,    57,    61,    12,    55,
      36,    32,    37,    34,     3,    55,    39,    38,     7,     8,
      39,    42,    43,   172,    13,    29,    47,    53,    40,    57,
      51,    40,    10,    40,    56,    61,    48,    55,    50,    60,
      56,    62,    56,    32,    65,    34,   219,    56,    61,    38,
      54,   115,   234,    42,    43,     7,     8,    44,    47,    63,
      64,    62,    51,    62,    11,   214,    59,    61,     5,    40,
      56,    60,    56,    62,   154,    58,    65,    11,    59,   157,
      32,    39,    34,    59,    52,    59,    61,    31,   168,   145,
      42,    43,    10,     3,    56,    47,     7,     8,    31,    51,
     178,    56,    33,    55,    39,    59,    59,   178,    60,    39,
      62,    61,    57,    65,    39,    31,    59,     7,     8,    61,
     184,    32,    10,    34,    52,    46,    13,    59,    11,    31,
       6,    42,    43,   211,    61,    41,    47,   168,   201,   183,
      51,   234,    32,   207,    34,   201,   233,    95,    97,    60,
      47,    62,    42,    43,    65,    53,     3,    47,     4,   174,
       6,    51,    -1,     9,    -1,    -1,    -1,    -1,    -1,   234,
      60,    -1,    62,    14,    15,    65,    17,    18,    19,    20,
      21,    22,    23,    24,    25,    26,    27,    28,    -1,    -1,
      36,    -1,    -1,    -1,    -1,    41,    -1,    -1,    -1,    45,
      -1,    -1,    -1,    49,    -1,    14,    15,    53,    17,    18,
      19,    20,    21,    22,    23,    24,    25,    26,    27,    28,
      61,    14,    15,    -1,    17,    18,    19,    20,    21,    22,
      23,    24,    25,    26,    27,    28,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    61
};

/* YYSTOS[STATE-NUM] -- The (internal number of the) accessing
   symbol of state STATE-NUM.  */
static const yytype_uint8 yystos[] =
{
       0,    40,    48,    50,    68,    69,    70,    71,    72,   119,
     120,   124,   125,   127,   128,   143,   144,    35,    61,   149,
       6,    36,    53,    61,   147,     0,    69,    38,    75,    38,
     121,     9,    37,   145,   146,   147,    39,    39,    10,   129,
      55,    76,    77,    78,    86,    89,   143,   122,   123,   143,
      61,    73,    44,    11,    57,    38,    39,   107,   148,    62,
     106,   126,    62,   150,    14,    15,    17,    18,    19,    20,
      21,    22,    23,    24,    25,    26,    27,    28,   140,   141,
     142,    11,   130,   131,    59,    55,    77,    61,    79,    87,
      90,   140,    55,   123,    61,     5,   117,    73,   146,     7,
       8,    32,    34,    42,    43,    47,    51,    55,    60,    65,
     100,   101,   102,   103,   104,   105,   106,    42,   102,    12,
      29,    54,    63,    64,   137,   138,   139,    56,    56,    40,
      96,    58,    11,   134,     4,     6,     9,    36,    41,    45,
      49,    53,   132,   133,    59,    39,    80,    59,    88,    52,
      61,    82,    95,    59,    31,     3,   118,    10,    74,   117,
      11,    55,   106,    56,    56,   137,    47,    57,    31,    97,
      39,    33,   135,    59,    11,    56,    83,    84,   143,    59,
      81,    61,    91,    96,    13,    42,    99,   102,   107,   108,
     109,   110,   118,    73,   101,    57,    99,    39,   136,   137,
     133,    11,    56,    73,    85,    93,   140,    31,    92,    97,
     106,   111,   112,    59,    11,    56,    84,    52,    61,    94,
     106,    59,    73,   113,    10,   137,    96,    46,    13,    61,
     114,   115,   116,    11,    31,   115,    98,   101,   107,   108
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
        case 5:
#line 319 "cimmof.y"
    { /* FIXME: Where do we put directives? */ ;}
    break;

  case 6:
#line 321 "cimmof.y"
    {
            cimmofParser::Instance()->addQualifier((yyvsp[(1) - (1)].mofqualifier));
            delete (yyvsp[(1) - (1)].mofqualifier);
        ;}
    break;

  case 7:
#line 326 "cimmof.y"
    {
            cimmofParser::Instance()->addClass((yyvsp[(1) - (1)].mofclass));
        ;}
    break;

  case 8:
#line 330 "cimmof.y"
    {
        cimmofParser::Instance()->addInstance((yyvsp[(1) - (1)].instance));
        ;}
    break;

  case 9:
#line 342 "cimmof.y"
    {
    YACCTRACE("classDeclaration:");
    if (g_currentAliasDecl != String::EMPTY)
    {
        cimmofParser::Instance()->addClassAlias(g_currentAliasDecl, (yyval.mofclass));
    }
;}
    break;

  case 10:
#line 354 "cimmof.y"
    {
    // create new instance of class with className and superclassName
    // put returned class object on stack
    YACCTRACE("classHead:");
    (yyval.mofclass) = cimmofParser::Instance()->newClassDecl(*(yyvsp[(3) - (5)].cimnameval), *(yyvsp[(5) - (5)].cimnameval));

    // put list of qualifiers into class
    applyQualifierList(g_qualifierList, *(yyval.mofclass));

    g_currentAliasRef = *(yyvsp[(4) - (5)].strval);
    if (g_currentClass)
    {
        delete g_currentClass;
    }
    g_currentClass = (yyval.mofclass);
    delete (yyvsp[(3) - (5)].cimnameval);
    delete (yyvsp[(4) - (5)].strval);
    delete (yyvsp[(5) - (5)].cimnameval);
;}
    break;

  case 11:
#line 375 "cimmof.y"
    {;}
    break;

  case 12:
#line 379 "cimmof.y"
    {
            (yyval.cimnameval) = new CIMName(*(yyvsp[(2) - (2)].cimnameval));
            delete (yyvsp[(2) - (2)].cimnameval);
        ;}
    break;

  case 13:
#line 384 "cimmof.y"
    {
            (yyval.cimnameval) = new CIMName();
        ;}
    break;

  case 18:
#line 398 "cimmof.y"
    {
        YACCTRACE("classFeature:applyProperty");
        cimmofParser::Instance()->applyProperty(*g_currentClass, *(yyvsp[(1) - (1)].property));
        delete (yyvsp[(1) - (1)].property);
    ;}
    break;

  case 19:
#line 404 "cimmof.y"
    {
        YACCTRACE("classFeature:applyMethod");
        cimmofParser::Instance()->applyMethod(*g_currentClass, *(yyvsp[(1) - (1)].method));
    ;}
    break;

  case 20:
#line 409 "cimmof.y"
    {
        YACCTRACE("classFeature:applyProperty");
        cimmofParser::Instance()->applyProperty(*g_currentClass, *(yyvsp[(1) - (1)].property));
        delete (yyvsp[(1) - (1)].property);
    ;}
    break;

  case 21:
#line 427 "cimmof.y"
    {
    YACCTRACE("methodDeclaration");
    (yyval.method) = (yyvsp[(2) - (4)].method);
;}
    break;

  case 22:
#line 437 "cimmof.y"
    {
    YACCTRACE("methodHead");
    if (g_currentMethod)
        delete g_currentMethod;

    // create new method instance with pointer to method name and datatype
    g_currentMethod = cimmofParser::Instance()->newMethod(*(yyvsp[(2) - (2)].cimnameval), (yyvsp[(1) - (2)].datatype)) ;

    // put new method on stack
    (yyval.method) = g_currentMethod;

    // apply the method qualifier list.
    applyQualifierList(g_qualifierList, *(yyval.method));

    delete (yyvsp[(2) - (2)].cimnameval);
;}
    break;

  case 25:
#line 459 "cimmof.y"
    {
    (yyval.cimnameval) = new CIMName(*(yyvsp[(1) - (1)].strval));
    delete (yyvsp[(1) - (1)].strval);
;}
    break;

  case 29:
#line 479 "cimmof.y"
    {
    // ATTN: P2 2002 Question Need to create default value including type?

    YACCTRACE("parameter:");
    CIMParameter *p = 0;
    cimmofParser *cp = cimmofParser::Instance();

    // Create new parameter with name, type, isArray, array, referenceClassName
    if ((yyvsp[(4) - (4)].ival) == -1)
    {
        p = cp->newParameter(*(yyvsp[(3) - (4)].cimnameval), (yyvsp[(2) - (4)].datatype), false, 0, g_referenceClassName);
    }
    else
    {
        p = cp->newParameter(*(yyvsp[(3) - (4)].cimnameval), (yyvsp[(2) - (4)].datatype), true, (yyvsp[(4) - (4)].ival), g_referenceClassName);
    }

    g_referenceClassName = CIMName();

    YACCTRACE("parameter:applyQualifierList");
    applyQualifierList(g_qualifierList, *p);

    cp->applyParameter(*g_currentMethod, *p);
    delete p;
    delete (yyvsp[(3) - (4)].cimnameval);
;}
    break;

  case 30:
#line 507 "cimmof.y"
    { (yyval.datatype) = (yyvsp[(1) - (1)].datatype); ;}
    break;

  case 31:
#line 508 "cimmof.y"
    { (yyval.datatype) = CIMTYPE_REFERENCE; ;}
    break;

  case 32:
#line 523 "cimmof.y"
    {
    // set body to stack and apply qualifier list
    // ATTN: the apply qualifer only works here because
    // there are not lower level qualifiers.  We do productions
    // that might have lower level qualifiers differently by
    // setting up a xxxHead production where qualifiers are
    // applied.
    YACCTRACE("propertyDeclaration:");
    (yyval.property) = (yyvsp[(2) - (3)].property);
    applyQualifierList(g_qualifierList, *(yyval.property));
;}
    break;

  case 33:
#line 539 "cimmof.y"
    {
    CIMValue *v = valueFactory::createValue((yyvsp[(1) - (4)].datatype),
        (yyvsp[(3) - (4)].ival),
        ((yyvsp[(4) - (4)].typedinitializer)->type == typedInitializerValue::NULL_VALUE),
        (yyvsp[(4) - (4)].typedinitializer)->nonNullParserType,
        (yyvsp[(4) - (4)].typedinitializer)->value);

    // set the value in the new property for the current instance
    // if scalar, $3 == -1 so set arrayDimension to zero.
    if ((yyvsp[(3) - (4)].ival) == CIMMOF_EMPTY_ARRAY)
    {
        (yyval.property) = cimmofParser::Instance()->newProperty(*(yyvsp[(2) - (4)].cimnameval), *v, 0);
    }
    else
    {
        (yyval.property) = cimmofParser::Instance()->newProperty(*(yyvsp[(2) - (4)].cimnameval), *v, (yyvsp[(3) - (4)].ival));
    }

    delete (yyvsp[(2) - (4)].cimnameval);
    delete (yyvsp[(4) - (4)].typedinitializer)->value;
    delete v;
;}
    break;

  case 35:
#line 578 "cimmof.y"
    {
    String s(*(yyvsp[(2) - (6)].strval));
    if (!String::equal(*(yyvsp[(5) - (6)].strval), String::EMPTY))
    {
        s.append("." + *(yyvsp[(5) - (6)].strval));
    }

    CIMValue *v = valueFactory::createValue(
        CIMTYPE_REFERENCE,
        CIMMOF_EMPTY_ARRAY,
        true,
        strValTypeNS::NULL_VALUE,
        &s);

    (yyval.property) = cimmofParser::Instance()->newProperty(*(yyvsp[(4) - (6)].strval), *v, 0, *(yyvsp[(2) - (6)].strval));
    applyQualifierList(g_qualifierList, *(yyval.property));
    delete (yyvsp[(2) - (6)].strval);
    delete (yyvsp[(4) - (6)].strval);
    delete (yyvsp[(5) - (6)].strval);
    delete v;
;}
    break;

  case 36:
#line 601 "cimmof.y"
    { (yyval.strval) = (yyvsp[(1) - (1)].strval); ;}
    break;

  case 37:
#line 603 "cimmof.y"
    { (yyval.strval) = (yyvsp[(1) - (1)].strval); ;}
    break;

  case 38:
#line 605 "cimmof.y"
    { (yyval.strval) = (yyvsp[(2) - (2)].strval); ;}
    break;

  case 39:
#line 606 "cimmof.y"
    { (yyval.strval) = new String(String::EMPTY); ;}
    break;

  case 40:
#line 610 "cimmof.y"
    {
    g_referenceClassName = *(yyvsp[(1) - (2)].cimnameval);
    delete (yyvsp[(1) - (2)].cimnameval);
;}
    break;

  case 41:
#line 617 "cimmof.y"
    {
    (yyval.cimnameval) = new CIMName(*(yyvsp[(1) - (1)].strval));
    delete (yyvsp[(1) - (1)].strval);
;}
    break;

  case 42:
#line 624 "cimmof.y"
    {
    (yyval.cimnameval) = new CIMName(*(yyvsp[(1) - (1)].strval));
    delete (yyvsp[(1) - (1)].strval);
;}
    break;

  case 43:
#line 641 "cimmof.y"
    {
            (yyval.ival) = (Uint32) valueFactory::stringToUint(*(yyvsp[(2) - (3)].strval), CIMTYPE_UINT32);
            delete (yyvsp[(2) - (3)].strval);
        ;}
    break;

  case 44:
#line 648 "cimmof.y"
    { (yyval.ival) = 0; ;}
    break;

  case 45:
#line 652 "cimmof.y"
    { (yyval.ival) = CIMMOF_EMPTY_ARRAY; ;}
    break;

  case 46:
#line 660 "cimmof.y"
    { (yyval.typedinitializer) = (yyvsp[(2) - (2)].typedinitializer); ;}
    break;

  case 47:
#line 662 "cimmof.y"
    {
            g_typedInitializerValue.setNull();
            (yyval.typedinitializer) = &g_typedInitializerValue;
        ;}
    break;

  case 48:
#line 675 "cimmof.y"
    { (yyval.strval) = (yyvsp[(1) - (1)].strval); ;}
    break;

  case 49:
#line 677 "cimmof.y"
    { (yyval.strval) = (yyvsp[(1) - (1)].strval); ;}
    break;

  case 50:
#line 679 "cimmof.y"
    { (yyval.strval) = (yyvsp[(1) - (1)].strval); ;}
    break;

  case 51:
#line 698 "cimmof.y"
    {
            YACCTRACE("typedInitializer:nonNullConstantValue= " << *(yyvsp[(1) - (1)].strval));
            g_typedInitializerValue.set(
                typedInitializerValue::CONSTANT_VALUE, (yyvsp[(1) - (1)].strval));
            g_typedInitializerValue.nonNullParserType = g_strValType;
            (yyval.typedinitializer) = &g_typedInitializerValue;
        ;}
    break;

  case 52:
#line 706 "cimmof.y"
    {
            g_typedInitializerValue.setNull();
            (yyval.typedinitializer) = &g_typedInitializerValue;
        ;}
    break;

  case 53:
#line 711 "cimmof.y"
    {
            g_typedInitializerValue.set(
                typedInitializerValue::ARRAY_VALUE, (yyvsp[(1) - (1)].strval));
            g_typedInitializerValue.nonNullParserType = g_strValType;
            (yyval.typedinitializer) = &g_typedInitializerValue;
        ;}
    break;

  case 54:
#line 718 "cimmof.y"
    {
            g_typedInitializerValue.set(
                typedInitializerValue::REFERENCE_VALUE, (yyvsp[(1) - (1)].strval));
            g_typedInitializerValue.nonNullParserType = g_strValType;
            (yyval.typedinitializer) = &g_typedInitializerValue;
        ;}
    break;

  case 55:
#line 736 "cimmof.y"
    {
        *(yyval.strval) = valueFactory::stringEscapeComma(String(*(yyvsp[(1) - (1)].strval)));
    ;}
    break;

  case 56:
#line 740 "cimmof.y"
    {
            YACCTRACE("constantValueList:constantValueList TOK_COMMA "
                      "constantValue, Value= " << *(yyvsp[(3) - (3)].strval));
            (*(yyval.strval)).append(",");

            // Append the new constantValue after escaping internal commas
            (*(yyval.strval)).append(valueFactory::stringEscapeComma(String(*(yyvsp[(3) - (3)].strval))));
            delete (yyvsp[(3) - (3)].strval);
        ;}
    break;

  case 57:
#line 760 "cimmof.y"
    {(yyval.strval) = (yyvsp[(1) - (1)].strval);;}
    break;

  case 58:
#line 762 "cimmof.y"
    {
            g_typedInitializerValue.nonNullParserType =
                strValTypeNS::NULL_VALUE;
            (yyval.strval) = new String(String::EMPTY);
        ;}
    break;

  case 59:
#line 781 "cimmof.y"
    {
        (yyval.strval) = (yyvsp[(1) - (1)].strval);
        g_strValType = strValTypeNS::INTEGER_VALUE;
    ;}
    break;

  case 60:
#line 786 "cimmof.y"
    {
        (yyval.strval) = (yyvsp[(1) - (1)].strval);
        g_strValType = strValTypeNS::REAL_VALUE;
    ;}
    break;

  case 61:
#line 791 "cimmof.y"
    {
         (yyval.strval) =  (yyvsp[(1) - (1)].strval);
        g_strValType = strValTypeNS::CHAR_VALUE;
    ;}
    break;

  case 62:
#line 796 "cimmof.y"
    {
        g_strValType = strValTypeNS::STRING_VALUE;
    ;}
    break;

  case 63:
#line 800 "cimmof.y"
    {
        g_strValType = strValTypeNS::BOOLEAN_VALUE;
        (yyval.strval) = new String((yyvsp[(1) - (1)].ival) ? "T" : "F");
    ;}
    break;

  case 69:
#line 818 "cimmof.y"
    { (yyval.ival) = 0; ;}
    break;

  case 70:
#line 820 "cimmof.y"
    { (yyval.ival) = 1; ;}
    break;

  case 71:
#line 825 "cimmof.y"
    { (yyval.strval) = (yyvsp[(1) - (1)].strval); ;}
    break;

  case 72:
#line 827 "cimmof.y"
    {
        (*(yyval.strval)).append(*(yyvsp[(2) - (2)].strval));  delete (yyvsp[(2) - (2)].strval);
    ;}
    break;

  case 73:
#line 833 "cimmof.y"
    {
   //String oldrep = *$1;
   //String s(oldrep), s1(String::EMPTY);
   // Handle quoted quote
   //int len = s.size();
   //if (s[len] == '\n') {
       //error: new line inside a string constant unless it is quoted
       //if (s[len - 2] == '\\') {
           //if (len > 3)
            //s1 = s.subString(1, len-3);
       //} else {
           //cimmof_error("New line in string constant");
           //}
       //cimmofParser::Instance()->increment_lineno();
   //} else { // Can only be a quotation mark
       //if (s[len - 2] == '\\') {  // if it is quoted
           //if (len > 3) s1 = s.subString(1, len-3);
           //s1.append('\"');
           //cimmof_yy_less(len-1);
       //} else { // This is the normal case:  real quotes on both end
           //s1 = s.subString(1, len - 2) ;
           //}
       //}
   //delete $1;
   //$$ = new String(s1);
   (yyval.strval) = new String(*(yyvsp[(1) - (1)].strval));
   delete (yyvsp[(1) - (1)].strval);
;}
    break;

  case 74:
#line 872 "cimmof.y"
    {
            (yyval.strval) = (yyvsp[(2) - (3)].strval);
        ;}
    break;

  case 75:
#line 876 "cimmof.y"
    {
            (yyval.strval) = new String(String::EMPTY);
        ;}
    break;

  case 76:
#line 881 "cimmof.y"
    {;}
    break;

  case 77:
#line 882 "cimmof.y"
    {  ;}
    break;

  case 78:
#line 886 "cimmof.y"
    {
    // The objectName string is decomposed for syntactical validation purposes
    // and reassembled here for later parsing in creation of an CIMObjectPath
    // instance
    String *s = new String(*(yyvsp[(2) - (4)].strval));
    if (!String::equal(*s, String::EMPTY) && (yyvsp[(3) - (4)].modelpath))
    {
        (*s).append(":");
    }
    if ((yyvsp[(3) - (4)].modelpath))
    {
        (*s).append((yyvsp[(3) - (4)].modelpath)->toString());
    }
    (yyval.strval) = s;
    delete (yyvsp[(2) - (4)].strval);
    delete (yyvsp[(3) - (4)].modelpath);
    MOF_trace2 ("objectHandle done $$ = ", (yyval.strval)->getCString());
;}
    break;

  case 79:
#line 907 "cimmof.y"
    {

    CIMObjectPath AOP;

    MOF_trace2("aliasInitializer $$ = ", (yyval.strval)->getCString());
    MOF_trace2("aliasInitializer $1 = ", (yyvsp[(1) - (1)].strval)->getCString());

    g_currentAliasRef = *(yyval.strval);

    MOF_trace2("aliasInitializer g_currentAliasRef = ",
        g_currentAliasRef.getCString());
    if (cimmofParser::Instance()->getInstanceAlias(g_currentAliasRef, AOP) == 0)
    {
         MOF_error("ERROR: aliasIdentifier NOT FOUND: aliasIdentifier = ",
             g_currentAliasRef.getCString());
        yyerror("aliasInitializer - 'aliasIdentifier' NOT FOUND");
        YYABORT;
    }

    String *s = new String(AOP.toString());

    (yyval.strval) = s;

    delete (yyvsp[(1) - (1)].strval);

    MOF_trace2 ("aliasInitializer done $$ = ", (yyval.strval)->getCString());

;}
    break;

  case 80:
#line 938 "cimmof.y"
    { ;}
    break;

  case 81:
#line 939 "cimmof.y"
    { (yyval.strval) = new String(String::EMPTY); ;}
    break;

  case 82:
#line 942 "cimmof.y"
    {;}
    break;

  case 83:
#line 946 "cimmof.y"
    {
      CIMObjectPath * m = new CIMObjectPath(String::EMPTY,
           CIMNamespaceName(),
           (*(yyvsp[(1) - (3)].cimnameval)).getString(),
           g_KeyBindingArray);

      g_KeyBindingArray.clear();
      delete (yyvsp[(1) - (3)].cimnameval);

      (yyval.modelpath) = m;

      MOF_trace2 ("modelPath done $$ = ", (yyval.modelpath)->toString().getCString());
   ;}
    break;

  case 84:
#line 964 "cimmof.y"
    { (yyval.ival) = 0; ;}
    break;

  case 85:
#line 966 "cimmof.y"
    { (yyval.ival) = 0; ;}
    break;

  case 86:
#line 974 "cimmof.y"
    {
        CIMKeyBinding::Type keyBindingType;
        Char16 firstChar = (*(yyvsp[(3) - (3)].strval))[0];
        // test for string type. TODO NOTSURE THIS IS CORRECT ANY MORE.
        if (firstChar == '\"')
        {
            keyBindingType = CIMKeyBinding::STRING;
        }
        // test for boolean type (i.e. first char value)
        else if ((firstChar == 'T') || (firstChar == 't') ||
                 (firstChar == 'F') || (firstChar == 'f'))
        {
            keyBindingType = CIMKeyBinding::BOOLEAN;
        }
        else
        {
            keyBindingType = CIMKeyBinding::NUMERIC;
        }
        CIMKeyBinding *kb = new CIMKeyBinding(*(yyvsp[(1) - (3)].strval), *(yyvsp[(3) - (3)].strval), keyBindingType);
        g_KeyBindingArray.append(*kb);
        delete kb;
        delete (yyvsp[(1) - (3)].strval);
        delete (yyvsp[(3) - (3)].strval);
    ;}
    break;

  case 88:
#line 1004 "cimmof.y"
    {
        (yyval.strval) = (yyvsp[(2) - (2)].strval);
        g_currentAliasDecl = *(yyvsp[(2) - (2)].strval);
        MOF_trace2("aliasIdentifier $$ = ", (yyval.strval)->getCString());
        MOF_trace2("aliasIdentifier g_currentAliasDecl = ",
            g_currentAliasDecl.getCString());

    ;}
    break;

  case 89:
#line 1013 "cimmof.y"
    {
            (yyval.strval) = new String(String::EMPTY);
            g_currentAliasDecl = String::EMPTY;
        ;}
    break;

  case 91:
#line 1031 "cimmof.y"
    {
    (yyval.instance) = g_currentInstance;
    if (g_currentAliasDecl != String::EMPTY)
    {
        MOF_trace2("instanceDeclaration g_currentAliasDecl = ",
                  g_currentAliasDecl.getCString());

        if (cimmofParser::Instance()->addInstanceAlias(
            g_currentAliasDecl, (yyval.instance)) == 0)
        {
            // Error alias already exist
            MOF_error("ERROR: alias ALREADY EXISTS: aliasIdentifier = ",
                g_currentAliasDecl.getCString());
            yyerror("instanceDeclaration - 'aliasIdentifier' ALREADY EXISTS");
            YYABORT;
        }
    }
;}
    break;

  case 92:
#line 1052 "cimmof.y"
    {
    if (g_currentInstance)
        delete g_currentInstance;
    g_currentAliasDecl = *(yyvsp[(5) - (5)].strval);
    g_currentInstance = cimmofParser::Instance()->newInstance(*(yyvsp[(4) - (5)].cimnameval));
    // apply the qualifierlist to the current instance
    (yyval.instance) = g_currentInstance;
    applyQualifierList(g_qualifierList, *(yyval.instance));
    delete (yyvsp[(4) - (5)].cimnameval);
    delete (yyvsp[(5) - (5)].strval);
    if (g_currentAliasDecl != String::EMPTY)
    {
        MOF_trace2("instanceHead g_currentAliasDecl = ",
            g_currentAliasDecl.getCString());
    }
;}
    break;

  case 96:
#line 1084 "cimmof.y"
    {
    AutoPtr<String> identifier((yyvsp[(2) - (5)].strval));
    cimmofParser *cp = cimmofParser::Instance();
    // ATTN: P1 InstanceUpdate function 2001 BB  Instance update needs
    // work here and CIMOM
    // a property.  It must be fixed in the Common code first.
    // What we have to do here is create a CIMProperty  and initialize it with
    // the value provided.  The name of the property is $2 and it belongs
    // to the class whose name is in g_currentInstance->getClassName().
    // The steps are
    //   2. Get  property declaration's value object
    CIMProperty *oldprop =
        cp->PropertyFromInstance(*g_currentInstance, *identifier);

    CIMValue *oldv = cp->ValueFromProperty(*oldprop);

    //   3. create the new Value object of the same type

    // We want createValue to interpret a value as an array if is enclosed
    // in {}s (e.g., { 2 } or {2, 3, 5}) or it is NULL and the property is
    // defined as an array. createValue is responsible for the actual
    // validation.

    CIMValue *v = valueFactory::createValue(oldv->getType(),
            (((yyvsp[(4) - (5)].typedinitializer)->type == typedInitializerValue::ARRAY_VALUE) ||
             (((yyvsp[(4) - (5)].typedinitializer)->type == typedInitializerValue::NULL_VALUE)
                    && (oldprop->isArray()))? 0 : CIMMOF_EMPTY_ARRAY),
            ((yyvsp[(4) - (5)].typedinitializer)->type == typedInitializerValue::NULL_VALUE),
            (yyvsp[(4) - (5)].typedinitializer)->nonNullParserType,
            (yyvsp[(4) - (5)].typedinitializer)->value);

    //   4. create a clone property with the new value
    CIMProperty *newprop = cp->copyPropertyWithNewValue(*oldprop, *v);

    //   5. apply the qualifiers;
    applyQualifierList(g_qualifierList, *newprop);

    //   6. and apply the CIMProperty to g_currentInstance.
    cp->applyProperty(*g_currentInstance, *newprop);
    delete (yyvsp[(4) - (5)].typedinitializer)->value;
    delete oldprop;
    delete oldv;
    delete v;
    delete newprop;
;}
    break;

  case 97:
#line 1140 "cimmof.y"
    {
        //printf("compilerDirectiveInclude ");
    ;}
    break;

  case 98:
#line 1144 "cimmof.y"
    {
        //printf("compilerDirectivePragma ");
    ;}
    break;

  case 99:
#line 1151 "cimmof.y"
    {
      cimmofParser::Instance()->enterInlineInclude(*(yyvsp[(4) - (5)].strval)); delete (yyvsp[(4) - (5)].strval);
    ;}
    break;

  case 100:
#line 1156 "cimmof.y"
    { (yyval.strval) = (yyvsp[(1) - (1)].strval); ;}
    break;

  case 101:
#line 1161 "cimmof.y"
    {
        cimmofParser::Instance()->processPragma(*(yyvsp[(2) - (5)].strval), *(yyvsp[(4) - (5)].strval));
        delete (yyvsp[(2) - (5)].strval);
        delete (yyvsp[(4) - (5)].strval);
    ;}
    break;

  case 102:
#line 1188 "cimmof.y"
    {
    (yyval.mofqualifier) = cimmofParser::Instance()->newQualifierDecl(*(yyvsp[(2) - (6)].strval), (yyvsp[(3) - (6)].value), *(yyvsp[(4) - (6)].scope), *(yyvsp[(5) - (6)].flavor));
    delete (yyvsp[(2) - (6)].strval);  // String created in qualifierName
    delete (yyvsp[(3) - (6)].value);  // CIMValue object created in qualifierValue production
    delete (yyvsp[(4) - (6)].scope);  // CIMScope object created in scope/metaElements production
;}
    break;

  case 103:
#line 1199 "cimmof.y"
    {
       (yyval.value) = valueFactory::createValue(
            (yyvsp[(2) - (4)].datatype),
            (yyvsp[(3) - (4)].ival),
            ((yyvsp[(4) - (4)].typedinitializer)->type == typedInitializerValue::NULL_VALUE),
            (yyvsp[(4) - (4)].typedinitializer)->nonNullParserType,
            (yyvsp[(4) - (4)].typedinitializer)->value);
       delete (yyvsp[(4) - (4)].typedinitializer)->value;
   ;}
    break;

  case 104:
#line 1214 "cimmof.y"
    {
        (yyval.scope) = (yyvsp[(2) - (3)].scope);
    ;}
    break;

  case 105:
#line 1218 "cimmof.y"
    {
       // terminate in error, no scope statement
          yyerror("\"scope\" definition required on Qualifier Declaration");
    ;}
    break;

  case 106:
#line 1225 "cimmof.y"
    {
        g_scope = CIMScope (CIMScope::NONE);
    ;}
    break;

  case 107:
#line 1232 "cimmof.y"
    {   /* empty */
            (yyval.scope) = (yyvsp[(1) - (1)].scope);
        ;}
    break;

  case 108:
#line 1236 "cimmof.y"
    {
            (yyval.scope)->addScope(*(yyvsp[(3) - (3)].scope));
            delete (yyvsp[(3) - (3)].scope);
        ;}
    break;

  case 109:
#line 1250 "cimmof.y"
    { (yyval.scope) = new CIMScope(CIMScope::CLASS);        ;}
    break;

  case 110:
#line 1251 "cimmof.y"
    { free(metaQualifierName);
                               metaQualifierName = 0;
                               (yyval.scope) = new CIMScope(CIMScope::ASSOCIATION);  ;}
    break;

  case 111:
#line 1254 "cimmof.y"
    { free(metaQualifierName);
                               metaQualifierName = 0;
                               (yyval.scope) = new CIMScope(CIMScope::INDICATION);   ;}
    break;

  case 112:
#line 1257 "cimmof.y"
    { (yyval.scope) = new CIMScope(CIMScope::PROPERTY);     ;}
    break;

  case 113:
#line 1258 "cimmof.y"
    { (yyval.scope) = new CIMScope(CIMScope::REFERENCE);    ;}
    break;

  case 114:
#line 1259 "cimmof.y"
    { (yyval.scope) = new CIMScope(CIMScope::METHOD);       ;}
    break;

  case 115:
#line 1260 "cimmof.y"
    { (yyval.scope) = new CIMScope(CIMScope::PARAMETER);    ;}
    break;

  case 116:
#line 1261 "cimmof.y"
    { (yyval.scope) = new CIMScope(CIMScope::ANY);          ;}
    break;

  case 117:
#line 1272 "cimmof.y"
    { (yyval.flavor) = &g_flavor; ;}
    break;

  case 118:
#line 1274 "cimmof.y"
    {
            g_flavor = CIMFlavor (CIMFlavor::NONE);
            (yyval.flavor) = &g_flavor;
        ;}
    break;

  case 119:
#line 1281 "cimmof.y"
    {
        g_flavor = CIMFlavor (CIMFlavor::NONE);
    ;}
    break;

  case 122:
#line 1297 "cimmof.y"
    { g_flavor.addFlavor (CIMFlavor::ENABLEOVERRIDE); ;}
    break;

  case 123:
#line 1298 "cimmof.y"
    { g_flavor.addFlavor (CIMFlavor::DISABLEOVERRIDE); ;}
    break;

  case 124:
#line 1299 "cimmof.y"
    { g_flavor.addFlavor (CIMFlavor::RESTRICTED); ;}
    break;

  case 125:
#line 1300 "cimmof.y"
    { g_flavor.addFlavor (CIMFlavor::TOSUBCLASS); ;}
    break;

  case 126:
#line 1301 "cimmof.y"
    { g_flavor.addFlavor (CIMFlavor::TRANSLATABLE); ;}
    break;

  case 127:
#line 1305 "cimmof.y"
    {
          (yyval.flavor) = &g_flavor;
        ;}
    break;

  case 128:
#line 1309 "cimmof.y"
    {
           g_flavor = CIMFlavor (CIMFlavor::NONE);
           (yyval.flavor) = &g_flavor;
        ;}
    break;

  case 131:
#line 1320 "cimmof.y"
    { (yyval.datatype) = (yyvsp[(1) - (1)].datatype); ;}
    break;

  case 132:
#line 1321 "cimmof.y"
    { (yyval.datatype) = (yyvsp[(1) - (1)].datatype); ;}
    break;

  case 133:
#line 1322 "cimmof.y"
    { (yyval.datatype) = CIMTYPE_STRING;   ;}
    break;

  case 134:
#line 1323 "cimmof.y"
    { (yyval.datatype) = CIMTYPE_BOOLEAN;  ;}
    break;

  case 135:
#line 1324 "cimmof.y"
    { (yyval.datatype) = CIMTYPE_DATETIME; ;}
    break;

  case 136:
#line 1327 "cimmof.y"
    { (yyval.datatype) = CIMTYPE_UINT8;  ;}
    break;

  case 137:
#line 1328 "cimmof.y"
    { (yyval.datatype) = CIMTYPE_SINT8;  ;}
    break;

  case 138:
#line 1329 "cimmof.y"
    { (yyval.datatype) = CIMTYPE_UINT16; ;}
    break;

  case 139:
#line 1330 "cimmof.y"
    { (yyval.datatype) = CIMTYPE_SINT16; ;}
    break;

  case 140:
#line 1331 "cimmof.y"
    { (yyval.datatype) = CIMTYPE_UINT32; ;}
    break;

  case 141:
#line 1332 "cimmof.y"
    { (yyval.datatype) = CIMTYPE_SINT32; ;}
    break;

  case 142:
#line 1333 "cimmof.y"
    { (yyval.datatype) = CIMTYPE_UINT64; ;}
    break;

  case 143:
#line 1334 "cimmof.y"
    { (yyval.datatype) = CIMTYPE_SINT64; ;}
    break;

  case 144:
#line 1335 "cimmof.y"
    { (yyval.datatype) = CIMTYPE_CHAR16; ;}
    break;

  case 145:
#line 1339 "cimmof.y"
    { (yyval.datatype) =CIMTYPE_REAL32; ;}
    break;

  case 146:
#line 1341 "cimmof.y"
    { (yyval.datatype) =CIMTYPE_REAL64; ;}
    break;

  case 148:
#line 1352 "cimmof.y"
    {
            //yydebug = 1; stderr = stdout;
        ;}
    break;

  case 149:
#line 1356 "cimmof.y"
    {
    YACCTRACE("qualifierListbegin");
    g_qualifierList.clear(); ;}
    break;

  case 150:
#line 1361 "cimmof.y"
    { ;}
    break;

  case 151:
#line 1363 "cimmof.y"
    { ;}
    break;

  case 152:
#line 1371 "cimmof.y"
    {
    cimmofParser *p = cimmofParser::Instance();
    // The qualifier value can't be set until we know the contents of the
    // QualifierDeclaration.  That's what QualifierValue() does.
    CIMValue *v = p->QualifierValue(*(yyvsp[(1) - (3)].strval),
                  ((yyvsp[(2) - (3)].typedinitializer)->type == typedInitializerValue::NULL_VALUE),
                  g_strValType,
                  *(yyvsp[(2) - (3)].typedinitializer)->value);
    (yyval.qualifier) = p->newQualifier(*(yyvsp[(1) - (3)].strval), *v, g_flavor);
    g_qualifierList.add(*(yyval.qualifier));
    delete (yyval.qualifier);
    delete (yyvsp[(1) - (3)].strval);
    delete (yyvsp[(2) - (3)].typedinitializer)->value;
    delete v;
;}
    break;

  case 153:
#line 1392 "cimmof.y"
    {
            g_flavor = CIMFlavor (CIMFlavor::NONE);
            (yyval.strval) = new String(metaQualifierName);
            free(metaQualifierName);
            metaQualifierName = 0;
        ;}
    break;

  case 154:
#line 1399 "cimmof.y"
    {
            g_flavor = CIMFlavor (CIMFlavor::NONE);
            (yyval.strval) = new String(metaQualifierName);
            free(metaQualifierName);
            metaQualifierName = 0;
        ;}
    break;

  case 155:
#line 1409 "cimmof.y"
    {
            g_flavor = CIMFlavor (CIMFlavor::NONE);
            (yyval.strval) = new String("Reference");
        ;}
    break;

  case 156:
#line 1414 "cimmof.y"
    {
            g_flavor = CIMFlavor (CIMFlavor::NONE);
            (yyval.strval) = (yyvsp[(1) - (1)].strval);
        ;}
    break;

  case 157:
#line 1423 "cimmof.y"
    {
            g_typedInitializerValue.set(
                typedInitializerValue::CONSTANT_VALUE, (yyvsp[(2) - (3)].strval));
            g_typedInitializerValue.nonNullParserType = g_strValType;
            (yyval.typedinitializer) = &g_typedInitializerValue;
        ;}
    break;

  case 158:
#line 1430 "cimmof.y"
    {
            g_typedInitializerValue.setNull();
            (yyval.typedinitializer) = &g_typedInitializerValue;
        ;}
    break;

  case 159:
#line 1435 "cimmof.y"
    {
            g_typedInitializerValue.set(
                typedInitializerValue::ARRAY_VALUE, (yyvsp[(1) - (1)].strval));
            g_typedInitializerValue.nonNullParserType = g_strValType;
            (yyval.typedinitializer) = &g_typedInitializerValue;
        ;}
    break;

  case 160:
#line 1441 "cimmof.y"
    {   /* empty */
            g_typedInitializerValue.setNull();
            (yyval.typedinitializer) = &g_typedInitializerValue;
        ;}
    break;

  case 161:
#line 1447 "cimmof.y"
    { (yyval.strval) = (yyvsp[(1) - (1)].strval); ;}
    break;

  case 162:
#line 1450 "cimmof.y"
    { (yyval.strval) = (yyvsp[(1) - (1)].strval); ;}
    break;


/* Line 1267 of yacc.c.  */
#line 3059 "cimmoftemp"
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


#line 1452 "cimmof.y"


/*
**==============================================================================
**
** MOF_error():
**
**==============================================================================
*/
static void MOF_error(const char * str, const char * S)
{
    printf("%s %s\n", str, S);
}

/*
**==============================================================================
**
** MOF_trace():
**
**==============================================================================
*/
// #define DEBUG_cimmof 1

#ifdef DEBUG_cimmof
static void MOF_trace(const char* str)
{
    printf("MOF_trace(): %s \n", str);
}

static void MOF_trace2(const char * str, const char * S)
{
    printf("MOF_trace2(): %s %s\n", str, S);
}

#endif // DEBUG_cimmof

