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
// Author:      Mark Hamzy, hamzy@us.ibm.com
//
//%/////////////////////////////////////////////////////////////////////////////

#ifndef _JMPI_Convert_h
#define _JMPI_Convert_h

#ifndef PEGASUS_DEBUG

// Retail build

#define DEBUG_ConvertJavaToC(j,c,p)   (c)p
#define DEBUG_ConvertCToJava(c,j,p)   (j)p
#define DEBUG_ConvertCleanup(c,p)

#else

// Debug build

#define CONVERT_NONE      1
#define CONVERT_BIG_PAD   0
#define CONVERT_SMALL_PAD 0

#include <Pegasus/Common/Config.h>
#include <iostream>
#include <cstdlib>

PEGASUS_USING_STD;
PEGASUS_USING_PEGASUS;

#if CONVERT_NONE

#define DEBUG_ConvertJavaToC(j,c,p)   (c)p
#define DEBUG_ConvertCToJava(c,j,p)   (j)p
#define DEBUG_ConvertCleanup(c,p)

#elif CONVERT_BIG_PAD

#define DEBUG_ConvertJavaToC(j,c,p)   TemplateConvertJavaToC <j,c> (p)
#define DEBUG_ConvertCToJava(c,j,p)   TemplateConvertCToJava <c,j> (p)
#define DEBUG_ConvertCleanup(j,p)     TemplateConvertCleanup <j>   (p)

#define CONVERT_SIGNATURE_BEGIN PEGASUS_UINT64_LITERAL(0x0123456789ABCDEF)
#define CONVERT_SIGNATURE_END   PEGASUS_UINT64_LITERAL(0xFEDCBA9876543210)
#define CONVERT_SIZE            (4 * sizeof (Uint64))

#define ConvertEndianSwap(n) \
    ((((n) & PEGASUS_UINT64_LITERAL(0xff00000000000000)) >> 56) | \
     (((n) & PEGASUS_UINT64_LITERAL(0x00ff000000000000)) >> 40) | \
     (((n) & PEGASUS_UINT64_LITERAL(0x0000ff0000000000)) >> 24) | \
     (((n) & PEGASUS_UINT64_LITERAL(0x000000ff00000000)) >> 8)  | \
     (((n) & PEGASUS_UINT64_LITERAL(0x00000000ff000000)) << 8)  | \
     (((n) & PEGASUS_UINT64_LITERAL(0x0000000000ff0000)) << 24) | \
     (((n) & PEGASUS_UINT64_LITERAL(0x000000000000ff00)) << 40) | \
     (((n) & PEGASUS_UINT64_LITERAL(0x00000000000000ff)) << 56) )

template <typename J, typename C>
C TemplateConvertJavaToC (J p)
{
   Uint64 *pui64 = (Uint64 *)p;
   Uint64 *ptrap = 0;

   if (pui64[0] != CONVERT_SIGNATURE_BEGIN)
   {
      PEGASUS_STD(cout)<<"TemplateConvertJavaToC "<<
          PEGASUS_STD(hex)<<(void*)p<<PEGASUS_STD(dec)<<PEGASUS_STD(endl);
      *ptrap = 0;
   }
   if (pui64[1] != ConvertEndianSwap (pui64[2]))
   {
      *ptrap = 0;
   }
   if (pui64[3] != CONVERT_SIGNATURE_END)
   {
      PEGASUS_STD(cout)<<"TemplateConvertJavaToC "<<
          PEGASUS_STD(hex)<<(void*)p<<PEGASUS_STD(dec)<<PEGASUS_STD(endl);
      *ptrap = 0;
   }

   PEGASUS_STD(cout)<<"TemplateConvertJavaToC "<<
       PEGASUS_STD(hex)<<(void*)p<<" -> "<<(void*)pui64[1]<<
           PEGASUS_STD(dec)<<PEGASUS_STD(endl);

   return (C)pui64[1];
}

template <typename C, typename J>
J TemplateConvertCToJava (C p)
{
   Uint64 *pui64 = (Uint64 *)::malloc (CONVERT_SIZE);
   Uint64 *ptrap = 0;

   if (pui64)
   {
      pui64[0] = CONVERT_SIGNATURE_BEGIN;
      pui64[1] = (Uint64)p;
      pui64[2] = ConvertEndianSwap ((Uint64)p);
      pui64[3] = CONVERT_SIGNATURE_END;
   }
   else
   {
      PEGASUS_STD(cout)<<"TemplateConvertCToJava"<<
          PEGASUS_STD(hex)<<(void*)p<<PEGASUS_STD(dec)<<PEGASUS_STD(endl);
      *ptrap = 0;
   }

   PEGASUS_STD(cout)<<"TemplateConvertCToJava "<<
       PEGASUS_STD(hex)<<(void*)p<<" -> "<<(void*)pui64<<
           PEGASUS_STD(dec)<<PEGASUS_STD(endl);

   return (J)pui64;
}

template <typename J>
void TemplateConvertCleanup (J p)
{
   Uint64 *pui64 = (Uint64 *)p;
   Uint64 *ptrap = 0;

   PEGASUS_STD(cout)<<"TemplateConvertCleanup "<<
       PEGASUS_STD(hex)<<(void*)p<<PEGASUS_STD(dec)<<PEGASUS_STD(endl);

   if (pui64[0] != CONVERT_SIGNATURE_BEGIN)
   {
      *ptrap = 0;
   }
   if (pui64[1] != ConvertEndianSwap (pui64[2]))
   {
      *ptrap = 0;
   }
   if (pui64[3] != CONVERT_SIGNATURE_END)
   {
      *ptrap = 0;
   }

   ::memset (pui64, 0, CONVERT_SIZE);
   ::free ((void *)pui64);
}

#elif CONVERT_SMALL_PAD

#define DEBUG_ConvertJavaToC(j,c,p)   TemplateConvertJavaToC <j,c> (p)
#define DEBUG_ConvertCToJava(c,j,p)   TemplateConvertCToJava <c,j> (p)
#define DEBUG_ConvertCleanup(j,p)     TemplateConvertCleanup <j>   (p)

#define CONVERT_SIGNATURE_BEGIN 0xFE
#define CONVERT_SIGNATURE_END   0x12
#define CONVERT_SIZE (sizeof(unsigned char)+ \
                      sizeof(Uint64)+ \
                      sizeof(unsigned char))

template <typename J, typename C>
C TemplateConvertJavaToC (J p)
{
   unsigned char *puch  = (unsigned char *)p;
   unsigned char *ptrap = 0;

   if (puch[0] != CONVERT_SIGNATURE_BEGIN)
   {
      PEGASUS_STD(cout)<<"TemplateConvertJavaToC "<<
          PEGASUS_STD(hex)<<(void*)p<<PEGASUS_STD(dec)<<PEGASUS_STD(endl);
      *ptrap = 0;
   }
   if (puch[sizeof (unsigned char) + sizeof (Uint64)] != CONVERT_SIGNATURE_END)
   {
      PEGASUS_STD(cout)<<"TemplateConvertJavaToC "<<
          PEGASUS_STD(hex)<<(void*)p<<PEGASUS_STD(dec)<<PEGASUS_STD(endl);
      *ptrap = 0;
   }

   PEGASUS_STD(cout)<<"TemplateConvertJavaToC "<<
       PEGASUS_STD(hex)<<(void*)p<<" -> "<<(void*)(*((long *)(puch + 1)))<<
           PEGASUS_STD(dec)<<PEGASUS_STD(endl);

   return (C)(void *)(*((Uint64 *)(puch + 1)));
}

template <typename C, typename J>
J TemplateConvertCToJava (C p)
{
   unsigned char *puch  = (unsigned char *)::malloc(CONVERT_SIZE);
   unsigned char *ptrap = 0;

   if (puch)
   {
      puch[0]                   = CONVERT_SIGNATURE_BEGIN;
      *(Uint64 *)(puch + 1)     = (Uint64)p;
      puch[1 + sizeof (Uint64)] = CONVERT_SIGNATURE_END;
   }
   else
   {
      PEGASUS_STD(cout)<<"TemplateConvertCToJava"<<
          PEGASUS_STD(hex)<<(void*)p<<PEGASUS_STD(dec)<<PEGASUS_STD(endl);
      *ptrap = 0;
   }

   PEGASUS_STD(cout)<<"TemplateConvertCToJava "<<
       PEGASUS_STD(hex)<<(void*)p<<" -> "<<(void *)puch<<
           PEGASUS_STD(dec)<<PEGASUS_STD(endl);

   return (J)puch;
}

template <typename J>
void TemplateConvertCleanup (J p)
{
   unsigned char *puch  = (unsigned char *)p;
   unsigned char *ptrap = 0;

   PEGASUS_STD(cout)<<"TemplateConvertCleanup "<<
       PEGASUS_STD(hex)<<(void*)p<<PEGASUS_STD(dec)<<PEGASUS_STD(endl);

   if (puch[0] != CONVERT_SIGNATURE_BEGIN)
   {
      *ptrap = 0;
   }
   if (puch[1 + sizeof (Uint64)] != CONVERT_SIGNATURE_END)
   {
      *ptrap = 0;
   }

   ::memset(puch, 0, CONVERT_SIZE);
   ::free((void *)puch);
}

#else

#error "Unsupported conversion case in \
           src/Pegasus/ProviderManager2/JMPI/Convert.h"

#endif

#endif

#endif
