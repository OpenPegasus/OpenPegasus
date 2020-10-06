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

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "cmpiPerf_TestClassA.h"
#include <Pegasus/Provider/CMPI/cmpimacs.h>

/* ---------------------------------------------------------------------------*/
/*                            Factory functions                               */
/* ---------------------------------------------------------------------------*/

/* ---------- method to create a CMPIObjectPath of this class ----------------*/

CMPIObjectPath * _makePath_TestClassA(
    const CMPIBroker * _broker,
    const CMPIContext * ctx,
    const CMPIObjectPath * cop)
{
   CMPIObjectPath * op = NULL;
   CMPIValue theKey;
   theKey.uint32 = 2;

   op=CMNewObjectPath(
       _broker,
       CMGetCharsPtr(CMGetNameSpace(cop,NULL), NULL),
       _ClassName,
       NULL);

   CMAddKey(op, "theKey", &theKey, CMPI_uint32);
   return op;
}

/* ----------- method to create a CMPIInstance of this class ----------------*/

CMPIInstance * _makeInst_TestClassA(
    const CMPIBroker * _broker,
    const CMPIContext * ctx,
    const CMPIObjectPath * cop,
    const char ** properties)
{
   CMPIArray       *array  = NULL;
   CMPIValue opstatus;
   CMPIValue status;
   CMPIValue theKey;
   CMPIObjectPath* op=NULL;
   CMPIInstance* ci=NULL;
   opstatus.uint16 = 0; /* Unknown */
   status.uint16 = 2;  /* Enabled */
   theKey.uint32 = 2;

   //
   // Construct ObjectPath
   //
   op=_makePath_TestClassA(_broker,ctx,cop);

   //
   // Create a new instance and fill it's properties
   //
   ci = CMNewInstance( _broker, op, NULL);
   CMRelease(op);
   CMSetPropertyFilter(ci,properties,NULL);

   //
   // Properties of CIM_ManagedElement
   //
   CMSetProperty(ci,"Caption","Test class A",CMPI_chars);
   CMSetProperty(
       ci,
       "Description",
       "Test class used for all kinds of testing",
       CMPI_chars);
   CMSetProperty(ci,"ElementName","Instance1",CMPI_chars);

   //
   // Properties of CIM_ManagedSystemElement
   //
   array = CMNewArray(_broker,1,CMPI_uint16,NULL);
   CMSetArrayElementAt(array,0,&opstatus,CMPI_uint16);
   CMSetProperty(ci,"OperationalStatus",(CMPIValue*)&(array),CMPI_uint16A);

   //
   // Properties of CIM_EnabledLogicalElement
   //
   CMSetProperty(ci,"EnabledState",&status,CMPI_uint16);
   CMSetProperty(ci,"OtherEnabledState","NULL",CMPI_chars);
   CMSetProperty(ci,"RequestedState",&status,CMPI_uint16);
   CMSetProperty(ci,"EnabledDefault",&status,CMPI_uint16);

   //
   // Properties of CIM_TestClass
   //
   CMSetProperty(ci,"theKey",&theKey,CMPI_uint32 );
   CMSetProperty(ci,"theData",&theKey,CMPI_uint32 );
   CMSetProperty(ci,"theString","Test Instance Number One",CMPI_chars);

   return ci;
}

/* -----------------------------------------------*/
/*          end of cmpiPerf_TestClassA.c          */
/* -----------------------------------------------*/

