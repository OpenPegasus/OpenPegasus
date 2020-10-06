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

unsigned char CMPI_true = 1;
unsigned char CMPI_false = 0;

static int dataNext = 0;
typedef struct
{
  CMPIString *key;
  CMPIString *data;
} Data;

static Data store[64];

static const CMPIBroker *broker;

int
addToStore (CMPIString * k, CMPIString * d)
{
  int i;
  for (i = 0; i < dataNext; i++)
    if (strcmp (CMGetCharsPtr (k,NULL), CMGetCharsPtr (store[i].key,NULL)) == 0)
      return 0;
// We must clone the CMPIString, since after the CreateInstance function exits
// these CMPIString would be automaticly deleted. Thought this puts a
// responsibility on us to use CMRelease when removing/cleaning up.
  store[dataNext].key = CMClone (k, NULL);
  store[dataNext++].data = CMClone (d, NULL);
  return 1;
}

CMPIString *
getFromStore (CMPIString * k)
{
  int i;
  for (i = 0; i < dataNext; i++)
    if (strcmp (CMGetCharsPtr (k,NULL), CMGetCharsPtr (store[i].key,NULL)) == 0)
      return store[i].data;
  return NULL;
}

int
replInStore (CMPIString * k, CMPIString * d)
{
  int i;
  for (i = 0; i < dataNext; i++)
    if (strcmp (CMGetCharsPtr (k,NULL), CMGetCharsPtr (store[i].key,NULL)) == 0)
      {
        // We release the other CMPIString
        CMRelease (store[i].data);
        // And clone the new one
        store[i].data = CMClone (d, NULL);
        return 1;
      }
  return 0;
}

int
remFromStore (CMPIString * k)
{
  int i;
  for (i = 0; i < dataNext; i++)
    {
      if (strcmp(
              CMGetCharsPtr(k,NULL),
              CMGetCharsPtr(store[i].key,NULL)) == 0)
        {
          // Release the CMPIStrings
          CMRelease (store[i].key);
          CMRelease (store[i].data);
          // Move the pointers from the other CMPIString to overwrite this one.
          for (; i < dataNext; i++)
            {
              store[i].key = store[i + 1].key;
              store[i].data = store[i + 1].data;
            }
          --dataNext;
          return 1;
        }
    }
  return 0;
}



//----------------------------------------------------------
//---
//      Instance Provider
//---
//----------------------------------------------------------


CMPIStatus
testProvCleanup (CMPIInstanceMI * cThis, const CMPIContext * ctx,
                 CMPIBoolean term)
{

  int i;
#ifdef PEGASUS_DEBUG
  fprintf (stderr, "+++ testProvCleanup(). Removing %d items.\n", dataNext);
#endif
  for (i = 0; i < dataNext; i++)
    {
      // Releasing the CMPIString's
      CMRelease (store[i].key);
      CMRelease (store[i].data);
    }

  CMReturn (CMPI_RC_OK);
}

CMPIStatus testProvEnumInstanceNames
  (CMPIInstanceMI * cThis, const CMPIContext * ctx, const CMPIResult * rslt,
   const CMPIObjectPath * ref)
{
  int i;
  CMPIStatus rc;
  CMPIObjectPath *cop;
#ifdef PEGASUS_DEBUG
  fprintf (stderr, "+++ testProvEnumInstanceNames()\n");
#endif
  cop = CMNewObjectPath (broker, CMGetCharsPtr (CMGetNameSpace (ref, &rc),NULL),
                         CMGetCharsPtr (CMGetClassName (ref, &rc),NULL), &rc);
  for (i = 0; i < dataNext; i++)
    {
      CMAddKey (cop, "Identifier", &store[i].key, CMPI_string);
      CMReturnObjectPath (rslt, cop);
    }
  CMReturnDone (rslt);
  CMReturn (CMPI_RC_OK);
}

CMPIStatus testProvEnumInstances
  (CMPIInstanceMI * cThis, const CMPIContext * ctx, const CMPIResult * rslt,
   const CMPIObjectPath * ref, const char **properties)
{
  int i;
  CMPIStatus rc;
  CMPIObjectPath *cop;
  CMPIInstance *inst;

#ifdef PEGASUS_DEBUG
  fprintf (stderr, "+++ testProvEnumInstances()\n");
#endif
  cop = CMNewObjectPath(
            broker,
            CMGetCharsPtr(CMGetNameSpace(ref, &rc), NULL),
            CMGetCharsPtr(CMGetClassName (ref, &rc),NULL),
            &rc);
  for (i = 0; i < dataNext; i++)
    {
      CMAddKey (cop, "Identifier", &store[i].key, CMPI_string);
      inst = CMNewInstance (broker, cop, &rc);
      CMSetProperty (inst, "Identifier", &store[i].key, CMPI_string);
      CMSetProperty (inst, "data", &store[i].data, CMPI_string);
      CMReturnInstance (rslt, inst);
    }
  CMReturnDone (rslt);
  CMReturn (CMPI_RC_OK);
}

CMPIStatus testProvGetInstance
  (CMPIInstanceMI * cThis, const CMPIContext * ctx, const CMPIResult * rslt,
   const CMPIObjectPath * cop, const char **properties)
{
  CMPIString *d, *k;
  CMPIStatus rc;
  CMPIInstance *inst;

  k = cop->ft->getKey (cop, "Identifier", &rc).value.string;
#ifdef PEGASUS_DEBUG
  fprintf (stderr, "+++ testProvGetInstance()\n");
#endif

  if ((d = getFromStore (k)) != NULL)
    {
      inst = CMNewInstance (broker, cop, &rc);
      CMSetProperty (inst, "Identifier", &k, CMPI_string);
      CMSetProperty (inst, "data", &d, CMPI_string);
      CMReturnInstance (rslt, inst);
    }
  else
    CMReturn (CMPI_RC_ERR_NOT_FOUND);

  CMReturnDone (rslt);
  CMReturn (CMPI_RC_OK);
}

CMPIStatus testProvCreateInstance
  (CMPIInstanceMI * cThis, const CMPIContext * ctx, const CMPIResult * rslt,
   const CMPIObjectPath * cop, const CMPIInstance * inst)
{
  CMPIString *k, *d;
  CMPIStatus rc;

#ifdef PEGASUS_DEBUG
  fprintf (stderr, "+++ testProvCreateInstance()\n");
#endif
  k = CMGetProperty (inst, "Identifier", &rc).value.string;
  d = CMGetProperty (inst, "data", &rc).value.string;

  if (addToStore (k, d) == 0)
    {
#ifdef PEGASUS_DEBUG
      fprintf (stderr, "+++ testProvCreateInstance() already exists\n");
#endif
      CMReturn (CMPI_RC_ERR_ALREADY_EXISTS);
    }

  CMReturnObjectPath (rslt, cop);
  CMReturnDone (rslt);
  CMReturn (CMPI_RC_OK);
}

CMPIStatus testProvModifyInstance
  (CMPIInstanceMI * cThis, const CMPIContext * ctx, const CMPIResult * rslt,
   const CMPIObjectPath * cop, const CMPIInstance * inst,
   const char **properties)
{
  CMReturn (CMPI_RC_ERR_NOT_SUPPORTED);
}

CMPIStatus testProvDeleteInstance
  (CMPIInstanceMI * cThis, const CMPIContext * ctx, const CMPIResult * rslt,
   const CMPIObjectPath * cop)
{
  CMPIString *k;
  CMPIStatus rc = { CMPI_RC_OK , 0};

  k = CMGetKey (cop, "Identifier", &rc).value.string;
  if (!remFromStore (k))
  {
    rc.rc = CMPI_RC_ERR_NOT_FOUND;
  }

  return rc;
}

CMPIStatus testProvExecQuery
  (CMPIInstanceMI * cThis, const CMPIContext * ctx, const CMPIResult * rslt,
   const CMPIObjectPath * cop, const char *lang, const char *query)
{
  CMReturn (CMPI_RC_OK);
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
  CMPIStatus rc;
  char data[128];
#ifdef PEGASUS_DEBUG
  fprintf (stderr, "+++ testProvInvokeMethod()\n");
#endif
  strcpy (data, "hallo! dir: ");
  strcat (data, (char *) (CMGetArg (in, "dir", &rc).value.string->hdl));
  strcat (data, " type: ");
  strcat (data, (char *) (CMGetArg (in, "type", &rc).value.string->hdl));

  CMReturnData (rslt, data, CMPI_chars);
  CMReturnDone (rslt);

  CMReturn (CMPI_RC_OK);
}

//----------------------------------------------------------
//---
//      Provider Factory Stubs
//---
//----------------------------------------------------------

CMInstanceMIStub (testProv, CMPISample, broker, CMNoHook)

//----------------------------------------------------------

CMMethodMIStub (testProv, CMPISample, broker, CMNoHook)

//----------------------------------------------------------
//----------------------------------------------------------
