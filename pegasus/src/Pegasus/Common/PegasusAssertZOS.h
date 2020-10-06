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
#ifndef Pegasus_Assert_ZOS_h
#define Pegasus_Assert_ZOS_h

#include <ctest.h>
#include <signal.h>
#include <unistd.h>
#include <stdio.h>

/*
   This function is executing a number of assertaion task.
   In differnet to the standard assert() function,
   this function does not abort the server!

   1) Print a statement to stderr
   2) Create a stack trace
   3) Create a system dump

*/

void __pegasus_assert_zOS(const char* file, int line, const char* cond);

/*
   Define PEGASUS_ASSERT assertion statement.  This statement tests the
   condition defined by the parameters and if not True executes an number
   of tasks.
*/

# define PEGASUS_ASSERT(COND)                                               \
      do                                                                    \
      {                                                                     \
          if (!(COND))                                                      \
          {                                                                 \
             __pegasus_assert_zOS( __FILE__, __LINE__, #COND );             \
          }                                                                 \
      } while (0)

/*
   Define PEGASUS_DEBUG_ASSERT assertion statement.
   It only generates code if PEGASUS_DEBUG is defined
   This statement tests the condition defined by the parameters and
   if not True executes an number of tasks and abort the server !
*/

#ifdef PEGASUS_DEBUG
# define PEGASUS_DEBUG_ASSERT(COND)                                         \
      do                                                                    \
      {                                                                     \
          if (!(COND))                                                      \
          {                                                                 \
             fprintf(stderr,                                                \
                  "\nPEGASUS_DEBUG_ASSERT: Assertation \'%s\' failed "      \
                  "in file %s ,line %d\n",#COND,__FILE__,__LINE__);         \
             ctrace("PEGASUS_DEBUG_ASSERT: Assertation \'"                  \
                  #COND"\' failed.");                                       \
             kill(getpid(),SIGDUMP);                                        \
             abort();                                                       \
          }                                                                 \
      } while (0)
#else
# define PEGASUS_DEBUG_ASSERT(COND)
#endif

#define PEGASUS_TEST_ASSERT(COND)                                          \
    do                                                                     \
    {                                                                      \
        if (!(COND))                                                       \
        {                                                                  \
            fprintf(stderr,                                                \
                 "\nPEGASUS_TEST_ASSERT: Assertation \'%s\' failed "       \
                 "in file %s ,line %d\n",#COND,__FILE__,__LINE__);         \
            ctrace("PEGASUS_TEST_ASSERT: Assertation \'"                   \
                 #COND"\' failed.");                                       \
            kill(getpid(),SIGDUMP);                                        \
            abort();                                                       \
        }                                                                  \
    } while (0)
#endif  /* Pegasus_Assert_ZOS_h */
