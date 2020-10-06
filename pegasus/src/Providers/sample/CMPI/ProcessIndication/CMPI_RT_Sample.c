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

#include <stdio.h>
#include <string.h>
#include <time.h>

#include <Pegasus/Provider/CMPI/cmpidt.h>
#include <Pegasus/Provider/CMPI/cmpift.h>
#include <Pegasus/Provider/CMPI/cmpimacs.h>

static const CMPIBroker *broker;

unsigned char CMPI_true = 1;
unsigned char CMPI_false = 0;

static int enabled = 0;
static int _nextUID = 0;


static void
generateIndication (const char *methodname, const CMPIContext * ctx)
{

  CMPIInstance *inst;
  CMPIObjectPath *cop;
  CMPIDateTime *dat;
  CMPIArray *ar;
  CMPIStatus rc;
  char buffer[32];

  if (enabled)
    {
#ifdef PEGASUS_DEBUG
      fprintf (stderr, "+++ generateIndication()\n");
#endif
      cop =
        CMNewObjectPath (broker, "root/SampleProvider",
                         "CMPI_RT_SampleIndication", &rc);
      inst = CMNewInstance (broker, cop, &rc);

      sprintf (buffer, "%d", _nextUID++);
      CMSetProperty (inst, "IndicationIdentifier", buffer, CMPI_chars);

      dat = CMNewDateTime (broker, &rc);
      CMSetProperty (inst, "IndicationTime", &dat, CMPI_dateTime);

      CMSetProperty (inst, "MethodName", methodname, CMPI_chars);

      ar = CMNewArray (broker, 0, CMPI_string, &rc);
      CMSetProperty (inst, "CorrelatedIndications", &ar, CMPI_stringA);

      CMAddContextEntry (ctx, "SnmpTrapOidContainer",
                         "1.3.6.1.4.1.900.2.3.9002.9600", CMPI_chars);

      rc = CBDeliverIndication (broker, ctx, "root/SampleProvider", inst);
      if (rc.rc != CMPI_RC_OK)
        {
          fprintf (stderr, "+++ Could not send the indication!\n");
        }
    }
#ifdef PEGASUS_DEBUG
  fprintf (stderr, "+++ generateIndication() done\n");
#endif
}

//----------------------------------------------------------
//---
//      Method Provider
//---
//----------------------------------------------------------


CMPIStatus
testProvMethodCleanup (CMPIMethodMI * cThis, const CMPIContext * ctx,
                       CMPIBoolean term)
{
  CMReturn (CMPI_RC_OK);
}

CMPIStatus testProvInvokeMethod
  (CMPIMethodMI * cThis, const CMPIContext * ctx, const CMPIResult * rslt,
   const CMPIObjectPath * cop, const char *method, const CMPIArgs * in,
   CMPIArgs * out)
{
  CMPIValue value;
#ifdef PEGASUS_DEBUG
  fprintf (stderr, "+++ testProvInvokeMethod()\n");
#endif

  if (enabled == 0)
    {
#ifdef PEGASUS_DEBUG
      fprintf (stderr, "+++ PROVIDER NOT ENABLED\n");
#endif
    }
  else
    {
      generateIndication (method, ctx);
    }

  value.uint32 = 0;
  CMReturnData (rslt, &value, CMPI_uint32);
  CMReturnDone (rslt);
  CMReturn (CMPI_RC_OK);
}

//----------------------------------------------------------
//---
//      Indication Provider
//---
//----------------------------------------------------------


CMPIStatus
testProvIndicationCleanup (CMPIIndicationMI * cThis, const CMPIContext * ctx,
                           CMPIBoolean term)
{
  CMReturn (CMPI_RC_OK);
}

CMPIStatus testProvAuthorizeFilter
  (CMPIIndicationMI * cThis, const CMPIContext * ctx,
   const CMPISelectExp * filter, const char *indType,
   const CMPIObjectPath * classPath, const char *owner)
{

  CMReturn (CMPI_RC_OK);
}

CMPIStatus testProvMustPoll
  (CMPIIndicationMI * cThis, const CMPIContext * ctx,
   const CMPISelectExp * filter, const char *indType,
   const CMPIObjectPath * classPath)
{

  CMReturn (CMPI_RC_OK);
}

CMPIStatus testProvActivateFilter
  (CMPIIndicationMI * cThis, const CMPIContext * ctx,
   const CMPISelectExp * exp, const char *clsName,
   const CMPIObjectPath * classPath, CMPIBoolean firstActivation)
{
/*
   CMPISelectCond* cond;
   CMPISubCond *subc;
   CMPIPredicate *prd;
   CMPIString* filter,*lhs,*rhs;
   CMPICount c;
   int type;

   fprintf(stderr,"+++ testProvActivateFilter()\n");

   filter=CMGetSelExpString(exp,NULL);
   fprintf(stderr,"--- %s\n",CMGetCharsPtr(filter,NULL));

   cond=CMGetDoc(exp,NULL);
   c=CMGetSubCondCountAndType(cond,&type,NULL);
   fprintf(stderr,"--- Count: %d type: %d\n",c,type);

//   for (int i=0,m=c; i<m; i++) {
      subc=CMGetSubCondAt(cond,0,NULL);
//   }
      fprintf(stderr,"--- Count: %d\n",CMGetPredicateCount(subc,NULL));
   prd=CMGetPredicateAt(subc,0,NULL);

   CMGetPredicateData(prd,NULL,NULL,&lhs,&rhs);
   fprintf(
       stderr,
       "--- %s %s\n",
       CMGetCharsPtr(lhs,NULL),
       CMGetCharsPtr(rhs,NULL));
*/

#ifdef PEGASUS_DEBUG
  fprintf (stderr, "+++ testProvActivateFilter()\n");
#endif

  enabled++;

#ifdef PEGASUS_DEBUG
  fprintf (stderr, "--- enabled: %d\n", enabled);
#endif

  CMReturn (CMPI_RC_OK);
}

CMPIStatus testProvDeActivateFilter
  (CMPIIndicationMI * cThis, const CMPIContext * ctx,
   const CMPISelectExp * filter, const char *clsName,
   const CMPIObjectPath * classPath, CMPIBoolean lastActivation)
{
#ifdef PEGASUS_DEBUG
  fprintf (stderr, "+++ testProvDeActivateFilter\n");
#endif
  enabled--;

#ifdef PEGASUS_DEBUG
  fprintf (stderr, "--- disabled: %d\n", enabled);
#endif
  CMReturn (CMPI_RC_OK);
}

CMPIStatus
testProvEnableIndications (CMPIIndicationMI * cThis, const CMPIContext * ctx)
{
#ifdef PEGASUS_DEBUG
  fprintf (stderr, "+++ testProvEnableIndications\n");
#endif
  CMReturn (CMPI_RC_OK);
}

CMPIStatus
testProvDisableIndications (CMPIIndicationMI * cThis, const CMPIContext * ctx)
{
#ifdef PEGASUS_DEBUG
  fprintf (stderr, "+++ testProvDisableIndications\n");
#endif
  CMReturn (CMPI_RC_OK);
}


//----------------------------------------------------------
//---
//      Provider Factory Stubs
//---
//----------------------------------------------------------

CMMethodMIStub (testProv, CMPI_RT_SampleProvider, broker, CMNoHook)
//----------------------------------------------------------


CMIndicationMIStub (testProv, CMPI_RT_SampleProvider, broker, CMNoHook)

//----------------------------------------------------------
