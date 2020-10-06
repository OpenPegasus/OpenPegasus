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
//
//%/////////////////////////////////////////////////////////////////////////////
#ifndef Pegasus_Assert_h
#define Pegasus_Assert_h

#ifdef PEGASUS_OS_ZOS
  // zOS specific implementation of assert macros.
# include <Pegasus/Common/PegasusAssertZOS.h>

#else
// NOTE:
//
//  This is the common implementation for the assert macros.
//  If platform specific implementation are needed they have to
//  be placed in a platform specific include file and included before.
//
// All built and tested OK with <assert.h>
// but <cassert> is more appropriate for c++ files
// however if this should be a problem for any c fileswithin the tree
// it is possible to revert back to assert.h
//     J Wunderlich 11/21/2005
//

#if (__cplusplus)
#include <cassert>
#else
#include <assert.h>
#endif

/** define PEGASUS_ASSERT assertion statement.  This statement tests the
    condition defined by the parameters and if not True executes an

   It only generates code if NDEBUG is not defined.
   See also the man page for assert().

   NOTE: if NDEBUG is set then the assert() macro will generate no code,
         and hence do nothing at all.

    <pre>
    assert()
    </pre>

    defining the file, line and condition that was tested.
*/

#define PEGASUS_ASSERT(COND) assert(COND)


/* define PEGASUS_DEBUG_ASSERT() assertion statement. This statement tests the
   condition defined by the parameters and if not True executes an assert.
   It only generates code if PEGASUS_DEBUG is defined and NDEBUG is not
   defined.
   See also the man page for assert().

   NOTE: if NDEBUG is set then the assert() macro will generate no code,
         and hence do nothing at all.

*/

#ifdef PEGASUS_DEBUG
# define PEGASUS_DEBUG_ASSERT(COND) assert(COND)
#else
# define PEGASUS_DEBUG_ASSERT(COND)
#endif

//print the condition string with PEGASUS_TEST_ASSERT failure so that user will 
//know the failure scenario also and the usage example is mentioned below , 
//if some one want to mention the failure reason.
//PEGASUS_TEST_ASSERT( 0 && (bool)"failure reason");
#define PEGASUS_TEST_ASSERT(COND)                                              \
    do                                                                         \
    {                                                                          \
        if (!(COND))                                                           \
        {                                                                      \
            printf("PEGASUS_TEST_ASSERT: Assertion `%s` in %s:%d failed \n",   \
                #COND,__FILE__, __LINE__);                                     \
            abort();                                                           \
        }                                                                      \
    } while (0)

# endif /* PEGASUS_OS_ZOS */


/* define PEGASUS_FCT_EXECUTE_AND_ASSERT assertion statement.

   Use this macro to avoid unused variables instead of PEGASUS_ASSERT when
   the return value of a function is only used to do an assert check.
   
   This statement compares the return value of function against VALUE for
   equalness but only if assertion is enabled. The Function FCT will always be
   called (equal if assertion is enabled or disabled).
      
   Do this:
   
       PEGASUS_FCT_EXECUTE_AND_ASSERT(true, f());
   
   Not this:
       bool returnCode = f();
       PEGASUS_ASSERT(true == returnCode);
*/
#if defined(PEGASUS_NOASSERTS) || defined(NDEBUG)
# define PEGASUS_FCT_EXECUTE_AND_ASSERT(VALUE,FCT) FCT
#else
# define PEGASUS_FCT_EXECUTE_AND_ASSERT(VALUE,FCT) PEGASUS_ASSERT(VALUE == FCT)
#endif

/**
    Defines PEGASUS_DISABLED_TEST_ASSERT(COND) assertion statement.
    This assert generates an error message and executes an exit(0) so that
    the tests may continue.
    Used for tests that we want to become permanent but today they may fail
    It generates code independent of PEGASUS_DEBUG.
    This macro is to be used ONLY by pegasus test components to temporarily
    bypass asserts for tests that are defined but do not currently work
    correctly.

   See also the man page for assert().

   NOTE: This macro is not linked to NDEBUG and also independent of
         platform definition
*/

#define PEGASUS_DISABLED_TEST_ASSERT(COND)                                 \
    do                                                                     \
    {                                                                      \
        if (!(COND))                                                       \
        {                                                                  \
            printf("PEGASUS_DISABLED_TEST_ASSERT failed in file %s"        \
                " line %d.\n Exit 0 from program so tests can continue\n", \
                __FILE__, __LINE__);                                       \
            exit(0);                                                       \
        }                                                                  \
    } while (0)

/**
 * Same as PEGASUS_DISABLED_TEST_ASSERT except that this macro does not
 * exit the program, simply generates a message and lets the program
 * continue.
 */
#define PEGASUS_DISABLED_TEST_ASSERT_CONTINUE(COND)                        \
    do                                                                     \
    {                                                                      \
        if (!(COND))                                                       \
        {                                                                  \
            printf("PEGASUS_DISABLED_TEST_ASSERT failed in file %s"        \
                " line %d.\n Exit 0 from program so tests can continue\n", \
                __FILE__, __LINE__);                                       \
        }                                                                  \
    } while (0)
#endif  /* Pegasus_Assert_h */
