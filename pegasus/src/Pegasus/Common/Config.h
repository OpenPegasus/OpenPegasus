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
//
//%/////////////////////////////////////////////////////////////////////////////
*/

#ifndef Pegasus_Config_h
#define Pegasus_Config_h

#if defined(PEGASUS_PLATFORM_WIN32_IX86_MSVC)
# include <Pegasus/Common/Platform_WIN32_IX86_MSVC.h>
#elif defined(PEGASUS_PLATFORM_WIN64_IA64_MSVC)
# include <Pegasus/Common/Platform_WIN64_IA64_MSVC.h>
#elif defined(PEGASUS_PLATFORM_WIN64_X86_64_MSVC)
# include <Pegasus/Common/Platform_WIN64_X86_64_MSVC.h>
#elif defined (PEGASUS_PLATFORM_LINUX_IX86_GNU)
# include <Pegasus/Common/Platform_LINUX_IX86_GNU.h>
#elif defined (PEGASUS_PLATFORM_LINUX_XSCALE_GNU)
# include <Pegasus/Common/Platform_LINUX_XSCALE_GNU.h>
#elif defined (PEGASUS_PLATFORM_PASE_ISERIES_IBMCXX)
#include <Pegasus/Common/Platform_PASE_ISERIES_IBMCXX.h>
#elif defined (PEGASUS_PLATFORM_AIX_RS_IBMCXX)
# include <Pegasus/Common/Platform_AIX_RS_IBMCXX.h>
#elif defined (PEGASUS_PLATFORM_HPUX_PARISC_ACC)
# include <Pegasus/Common/Platform_HPUX_PARISC_ACC.h>
#elif defined (PEGASUS_PLATFORM_HPUX_PARISC_GNU)
# include <Pegasus/Common/Platform_HPUX_PARISC_GNU.h>
#elif defined (PEGASUS_PLATFORM_HPUX_IA64_ACC)
# include <Pegasus/Common/Platform_HPUX_IA64_ACC.h>
#elif defined (PEGASUS_PLATFORM_HPUX_IA64_GNU)
# include <Pegasus/Common/Platform_HPUX_IA64_GNU.h>
#elif defined (PEGASUS_PLATFORM_TRU64_ALPHA_DECCXX)
# include <Pegasus/Common/Platform_TRU64_ALPHA_DECCXX.h>
#elif defined (PEGASUS_PLATFORM_SOLARIS_SPARC_GNU)
# include <Pegasus/Common/Platform_SOLARIS_SPARC_GNU.h>
#elif defined (PEGASUS_PLATFORM_SOLARIS_SPARC_CC)
# include <Pegasus/Common/Platform_SOLARIS_SPARC_CC.h>
#elif defined (PEGASUS_PLATFORM_SOLARIS_SPARC_64_CC)
# include <Pegasus/Common/Platform_SOLARIS_SPARC_CC_64.h>
#elif defined (PEGASUS_PLATFORM_SOLARIS_IX86_CC)
# include <Pegasus/Common/Platform_SOLARIS_IX86_CC.h>
#elif defined (PEGASUS_PLATFORM_SOLARIS_X86_64_CC)
# include <Pegasus/Common/Platform_SOLARIS_X86_64_CC.h>
#elif defined (PEGASUS_PLATFORM_ZOS_ZSERIES_IBM)
# include <Pegasus/Common/Platform_ZOS_ZSERIES_IBM.h>
#elif defined (PEGASUS_PLATFORM_ZOS_ZSERIES64_IBM)
# include <Pegasus/Common/Platform_ZOS_ZSERIES64_IBM.h>
#elif defined (PEGASUS_PLATFORM_LINUX_IA64_GNU)
# include <Pegasus/Common/Platform_LINUX_IA64_GNU.h>
#elif defined (PEGASUS_PLATFORM_LINUX_X86_64_GNU)
# include <Pegasus/Common/Platform_LINUX_X86_64_GNU.h>
#elif defined (PEGASUS_PLATFORM_LINUX_PPC_GNU)
# include <Pegasus/Common/Platform_LINUX_PPC_GNU.h>
#elif defined (PEGASUS_PLATFORM_LINUX_PPC64_GNU)
# include <Pegasus/Common/Platform_LINUX_PPC64_GNU.h>
#elif defined (PEGASUS_PLATFORM_LINUX_PPC_E500_GNU)
# include <Pegasus/Common/Platform_LINUX_PPC_E500_GNU.h>
#elif defined (PEGASUS_PLATFORM_LINUX_ZSERIES_GNU)
# include <Pegasus/Common/Platform_LINUX_ZSERIES_GNU.h>
#elif defined (PEGASUS_PLATFORM_LINUX_ZSERIES64_GNU)
# include <Pegasus/Common/Platform_LINUX_ZSERIES64_GNU.h>
#elif defined (PEGASUS_PLATFORM_DARWIN_PPC_GNU)
# include <Pegasus/Common/Platform_DARWIN_PPC_GNU.h>
#elif defined (PEGASUS_PLATFORM_DARWIN_IX86_GNU)
# include <Pegasus/Common/Platform_DARWIN_IX86_GNU.h>
#elif defined (PEGASUS_PLATFORM_VMS_ALPHA_DECCXX)
# include <Pegasus/Common/Platform_VMS_ALPHA_DECCXX.h>
#elif defined (PEGASUS_PLATFORM_VMS_IA64_DECCXX)
# include <Pegasus/Common/Platform_VMS_IA64_DECCXX.h>
#elif defined (PEGASUS_PLATFORM_LINUX_XSCALE_GNU)
# include <Pegasus/Common/Platform_LINUX_XSCALE_GNU.h>
#elif defined (PEGASUS_PLATFORM_LINUX_AARCH64_GNU)
# include <Pegasus/Common/Platform_LINUX_AARCH64_GNU.h>
#elif defined (PEGASUS_PLATFORM_LINUX_X86_64_CLANG)
# include <Pegasus/Common/Platform_LINUX_X86_64_CLANG.h>
#elif defined (PEGASUS_PLATFORM_LINUX_IX86_CLANG)
# include <Pegasus/Common/Platform_LINUX_IX86_CLANG.h>
#elif defined (PEGASUS_PLATFORM_LINUX_SH4_GNU)
# include <Pegasus/Common/Platform_LINUX_SH4_GNU.h>
#else
# error "<Pegasus/Common/Config.h>: Unsupported Platform"
#endif

/*
//
// COMPILER Checks
//
// This is to allow a check for GCC > 3.2
// It needs to be the first thing we check because the next lines load
// further source files
*/

#if defined(__GNUC__)
#define GCC_VERSION (__GNUC__ * 10000 \
                               + __GNUC_MINOR__ * 100 \
                               + __GNUC_PATCHLEVEL__)

/*
// To test for GCC > 3.2.0:
//     #if GCC_VERSION > 30200
*/

#endif


#ifdef __cplusplus
#include <cstdlib>
#endif

#ifndef PEGASUS_IMPORT
# define PEGASUS_IMPORT /* empty */
#endif

#ifndef PEGASUS_EXPORT
# define PEGASUS_EXPORT /* empty */
#endif

/*
**==============================================================================
**
** PEGASUS_HIDDEN_LINKAGE
**
**     This macro suppresses the export of a symbol in contexts in which the
**     default linkage would have otherwise exported the symbol. Consider the
**     following example:
**
**         class PEGASUS_COMMON_LINKAGE MyClass
**         {
**         public:
**
**             // Is exported (due to PEGASUS_COMMON_LINKAGE macro).
**             void f();
**
**             // Is not exported (despite the PEGASUS_COMMON_LINKAGE macro).
**             PEGASUS_HIDDEN_LINKAGE void g(); // not-exported.
**         };
**
**     Notes: this macro has an empty expansion on many platforms since this
**     feature is compiler specific.
**
**==============================================================================
*/
#ifndef PEGASUS_HIDDEN_LINKAGE
# define PEGASUS_HIDDEN_LINKAGE /* empty */
#endif

#ifdef PEGASUS_HAVE_NAMESPACES
#ifdef __cplusplus
# define PEGASUS_NAMESPACE_BEGIN namespace Pegasus {
# define PEGASUS_NAMESPACE_END }
# define PEGASUS_NAMESPACE(X) Pegasus::X

#ifndef PEGASUS_HAVE_NO_STD
# define PEGASUS_STD(X) std::X
# define PEGASUS_USING_STD using namespace std
#else
# define PEGASUS_STD(X) X
# define PEGASUS_USING_STD
#endif
# define PEGASUS_USING_PEGASUS using namespace Pegasus
#else
# define PEGASUS_NAMESPACE_BEGIN /* empty */
# define PEGASUS_NAMESPACE_END /* empty */
# define PEGASUS_NAMESPACE(X) X
# define PEGASUS_STD(X) X
# define PEGASUS_USING_STD
# define PEGASUS_USING_PEGASUS
#endif
#else
# define PEGASUS_NAMESPACE_BEGIN /* empty */
# define PEGASUS_NAMESPACE_END /* empty */
# define PEGASUS_NAMESPACE(X) X
# define PEGASUS_STD(X) X
# define PEGASUS_USING_STD
# define PEGASUS_USING_PEGASUS
#endif

#ifdef PEGASUS_HAVE_EXPLICIT
#ifdef __cplusplus
# define PEGASUS_EXPLICIT explicit
#else
# define PEGASUS_EXPLICIT /* empty */
#endif
#else
# define PEGASUS_EXPLICIT /* empty */
#endif

#ifndef PEGASUS_HAVE_FOR_SCOPE
# define for if (0) ; else for
#endif

#ifdef PEGASUS_HAVE_TEMPLATE_SPECIALIZATION
#ifdef __cplusplus
# define PEGASUS_TEMPLATE_SPECIALIZATION template <>
#else
# define PEGASUS_TEMPLATE_SPECIALIZATION
#endif
#else
# define PEGASUS_TEMPLATE_SPECIALIZATION
#endif

#ifdef PEGASUS_HAVE_IOS_BINARY
#ifdef __cplusplus
#define PEGASUS_IOS_BINARY ,std::ios::binary
#define PEGASUS_OR_IOS_BINARY | std::ios::binary
#else
#define PEGASUS_IOS_BINARY /* empty */
#define PEGASUS_OR_IOS_BINARY /* empty */
#endif
#else
#define PEGASUS_IOS_BINARY /* empty */
#define PEGASUS_OR_IOS_BINARY /* empty */
#endif

#ifndef PEGASUS_SINT64_LITERAL
#define PEGASUS_SINT64_LITERAL(X) (Sint64 (X##LL))
#endif
#ifndef PEGASUS_UINT64_LITERAL
#define PEGASUS_UINT64_LITERAL(X) (Uint64 (X##ULL))
#endif

/* Used in printf and scanf conversion strings for Uint64 and Sint64 args */
#ifndef PEGASUS_64BIT_CONVERSION_WIDTH
#define PEGASUS_64BIT_CONVERSION_WIDTH "ll"
#endif

#ifndef PEGASUS_MAXHOSTNAMELEN
# ifdef _POSIX_HOST_NAME_MAX
#  define PEGASUS_MAXHOSTNAMELEN _POSIX_HOST_NAME_MAX
# else
#  define PEGASUS_MAXHOSTNAMELEN MAXHOSTNAMELEN
# endif
#endif

#ifdef __cplusplus
PEGASUS_NAMESPACE_BEGIN

typedef bool Boolean;
typedef unsigned char Uint8;
typedef signed char Sint8;
typedef unsigned short Uint16;
typedef signed short Sint16;
typedef unsigned int Uint32;
typedef signed int Sint32;
typedef float Real32;
typedef double Real64;
typedef PEGASUS_UINT64 Uint64;
typedef PEGASUS_SINT64 Sint64;

#define PEG_NOT_FOUND Uint32(-1)

PEGASUS_NAMESPACE_END
#endif


/*
 *PEGASUS_UNREACHABLE implies unreachable code in pegasus.
 *Should be used in places where the control should not reached.
 *Please use in this way
 * PEGASUS_UNREACHABLE( expression;)
 *not in this way
 * PEGASUS_UNREACHABLE(expression);
 *
 *Though both are same, Former will prevent ;;(double semicolon)
 *
 */

#ifdef PEGASUS_SUPPRESS_UNREACHABLE_STATEMENTS
# define PEGASUS_UNREACHABLE(CODE)
#else
# if defined(__clang__ )
#  define PEGASUS_UNREACHABLE(CODE) __builtin_unreachable();
# elif defined(GCC_VERSION)
#  if GCC_VERSION >= 40500 //Unreachable supported only for gcc 4.5 and above
#   define PEGASUS_UNREACHABLE(CODE) __builtin_unreachable();
#  else
#   define PEGASUS_UNREACHABLE(CODE) CODE
#  endif
# elif defined(_MSC_VER) //PEGASUS_OS_TYPE_WINDOWS
#  define PEGASUS_UNREACHABLE(CODE) __assume(0);
# else
#  define PEGASUS_UNREACHABLE(CODE) CODE
# endif
#endif

/*
**==============================================================================
**
** PEGASUS_FORMAT
**
**     This macro allows the compiler to check the arguments to format
**     specifiers in the printf family of functions. For example, the following
**     enables format checking for the foo function.
**
**         PEGASUS_FORMAT(1, 2)
**         void foo(const char* format, ...);
**
**     On GCC, the following error is detected  below (when using this macro).
**
**         foo("%s %d", 99, "hello"); // mismatch specifiers!
**
**==============================================================================
*/

#if defined(__GNUC__) &&  (__GNUC__ >= 4)
# define PEGASUS_FORMAT(A1, A2) __attribute__((format (printf, A1, A2)))
#else
# define PEGASUS_FORMAT(A1, A2) /* not implemented */
#endif

/*
** PEGASUS_INITIAL_THREADSTACK_SIZE
**
** This macro is used to restrict the maximal stack size used per thread.
** Since OpenPegasus makes heavy use of multi-threading reducing the system
** default is espescially important on embedded systems.
**
** This value can be overridden platform specific by defining the macro in
** the corresponding PEGASUS_PLATFORM_*.h file.
*/
#ifndef PEGASUS_INITIAL_THREADSTACK_SIZE
# define PEGASUS_INITIAL_THREADSTACK_SIZE 256*1024
#endif

/*
** PEGASUS_PG_OBJECTMANAGER_ELEMENTNAME
**
** This macro is used to set ElementName property on the PG_ObjectManager
** instance. This value is also used in SLP announcment for service-hi-name.
**
** This value can be overridden platform specific by defining the macro in
** the corresponding PEGASUS_PLATFORM_*.h file.
*/
#ifndef PEGASUS_PG_OBJECTMANAGER_ELEMENTNAME
# ifdef PEGASUS_FLAVOR
#  define PEGASUS_PG_OBJECTMANAGER_ELEMENTNAME PEGASUS_FLAVOR"-pegasus"
# else
#  define PEGASUS_PG_OBJECTMANAGER_ELEMENTNAME "Pegasus"
# endif
#endif

/*
** PEGASUS_INSTANCEID_GLOBAL_PREFIX
** This macro is used to create the Name property on the PG_ObjectManager
** instance. This value is also used in SLP announcement for service-id.
**
** This value can be overridden platform specific by defining the macro in
** the corresponding PEGASUS_PLATFORM_*.h file.
*/
#ifndef PEGASUS_INSTANCEID_GLOBAL_PREFIX
# ifdef PEGASUS_FLAVOR
#  define PEGASUS_INSTANCEID_GLOBAL_PREFIX "PG-"PEGASUS_FLAVOR
# else
#  define PEGASUS_INSTANCEID_GLOBAL_PREFIX "PG"
# endif
#endif

#endif  /* Pegasus_Config_h */
