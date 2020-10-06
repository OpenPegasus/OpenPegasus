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
// Author: Konrad Rzeszutek <konradr@us.ibm.com>
//
// Modified By: David Dillard, VERITAS Software Corp.
//                  (david.dillard@veritas.com)
//
//%/////////////////////////////////////////////////////////////////////////////

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdarg.h>

#include <Pegasus/Provider/CMPI/cmpidt.h>
#include <Pegasus/Provider/CMPI/cmpift.h>
#include <Pegasus/Provider/CMPI/cmpimacs.h>

#include <Providers/TestProviders/CMPI/TestUtilLib/cmpiUtilLib.h>

#define _ClassName "TestCMPI_ExecQuery"
#define _ClassName_size strlen(_ClassName)
#define _Namespace    "test/TestProvider"
#define _ProviderLocation  \
    "/src/Providers/TestProviders/CMPI/TestExecQuery/tests/"

#ifdef CMPI_VER_100
static const CMPIBroker *_broker;
#else
static CMPIBroker *_broker;
#endif

/* ---------------------------------------------------------------------------*/
/*                       CMPI Helper function                        */
/* ---------------------------------------------------------------------------*/

static CMPIObjectPath *
make_ObjectPath (const CMPIBroker * broker, const char *ns, const char *class)
{
  CMPIObjectPath *objPath = NULL;
  CMPIStatus rc = { CMPI_RC_OK, NULL };

  PROV_LOG ("--- make_ObjectPath: CMNewObjectPath");
  objPath = CMNewObjectPath (broker, ns, class, &rc);
  //PEGASUS_TEST_ASSERT ( rc.rc == CMPI_RC_OK);
  PROV_LOG ("----- %s",  strCMPIStatus(rc));
  CMAddKey (objPath, "ElementName", (CMPIValue *) class, CMPI_chars);

  return objPath;
}

static CMPIInstance *
make_Instance (CMPIObjectPath * op)
{

  CMPIStatus rc = { CMPI_RC_OK, NULL };
  CMPIInstance *ci = NULL;


  PROV_LOG ("--- make_Instance: CMNewInstance");
  ci = CMNewInstance (_broker, op, &rc);
  PROV_LOG ("----- %s", strCMPIStatus (rc));
  if (rc.rc == CMPI_RC_ERR_NOT_FOUND)
    {
      PROV_LOG (" --- Class %s is not found in the %s namespace!",
                _ClassName, _Namespace);
      PROV_LOG (" --- Aborting!!! ");
      return NULL;
    }
  return ci;
}

 int
_setProperty (CMPIInstance * ci, const char *p)
{
  CMPIValue val;
  const char *property;
  CMPIStatus rc = { CMPI_RC_OK, NULL };
  /*
     WQL just gives the property names, such as :"ElementName".
     CQL is more complex and gives the class from which the
     property belongs, such as "TestCMPIExecQuery.ElementName".
     So we find the classname in the property name we just
     ignore it.
*/
  if (strncmp (p, _ClassName, _ClassName_size) == 0)
    {
      // The first couple of bytes are the ClassName, then
      // move the pointer (+1 for the dot)
      property = p + _ClassName_size + 1;
    }
  else
    property = p;

  PROV_LOG ("--- _setProperty: %s -> %s", p, property);
  if ((strncmp (property, "ElementName", 11) == 0)
      && (strlen (property) == 11))
    {
      rc =
        CMSetProperty (ci, "ElementName", (CMPIValue *) _ClassName,
                       CMPI_chars);
    }

  else if ((strncmp (property, "s", 1) == 0) && (strlen (property) == 1))
    {
      rc = CMSetProperty (ci, "s", (CMPIValue *) "s", CMPI_chars);
    }
  else if ((strncmp (property, "c", 1) == 0) && (strlen (property) == 1))
    {
      rc = CMSetProperty (ci, "c", (CMPIValue *) "c", CMPI_char16);
    }

  else if ((strncmp (property, "n32", 3) == 0) && (strlen (property) == 3))
    {
      val.uint32 = 32;
      rc = CMSetProperty (ci, "n32", &val, CMPI_uint32);
    }
  else if ((strncmp (property, "n64", 3) == 0) && (strlen (property) == 3))
    {
      val.uint64 = 64;
      rc = CMSetProperty (ci, "n64", &val, CMPI_uint64);
    }
  else if ((strncmp (property, "n16", 3) == 0) && (strlen (property) == 3))
    {
      val.uint16 = 16;
      rc = CMSetProperty (ci, "n16", &val, CMPI_uint16);
    }
  else if ((strncmp (property, "n8", 2) == 0) && (strlen (property) == 2))
    {
      val.uint8 = 8;
      rc = CMSetProperty (ci, "n8", &val, CMPI_uint8);
    }

  else if ((strncmp (property, "r32", 3) == 0) && (strlen (property) == 3))
    {
      val.real32 = (CMPIReal32)1.23;
      rc = CMSetProperty (ci, "r32", &val, CMPI_real32);
    }
  else if ((strncmp (property, "r64", 3) == 0) && (strlen (property) == 3))
    {
      val.real64 = 3.1415678928283;
      rc = CMSetProperty (ci, "r64", &val, CMPI_real64);
    }

  else if ((strncmp (property, "s64", 3) == 0) && (strlen (property) == 3))
    {
      val.sint64 = 0xFFFFFFF;
      rc = CMSetProperty (ci, "s64", &val, CMPI_sint64);
    }
  else if ((strncmp (property, "s32", 3) == 0) && (strlen (property) == 3))
    {
      val.sint32 = 0xDEADBEEF;
      rc = CMSetProperty (ci, "s32", &val, CMPI_sint32);
    }
  else if ((strncmp (property, "s16", 3) == 0) && (strlen (property) == 3))
    {
      val.sint16 = (CMPISint16)0xFFFF;
      rc = CMSetProperty (ci, "s16", &val, CMPI_sint16);
    }
  else if ((strncmp (property, "s8", 2) == 0) && (strlen (property) == 2))
    {
      val.sint8 = (CMPISint8)0xFF;
      rc = CMSetProperty (ci, "s8", &val, CMPI_sint8);
    }

  else if ((strncmp (property, "b", 1) == 0) && (strlen (property) == 1))
    {
      val.boolean = 1;
      rc = CMSetProperty (ci, "b", &val, CMPI_boolean);
    }
  else if ((strncmp (property, "d", 1) == 0) && (strlen (property) == 1))
    {
      PROV_LOG ("---- CMNewDateTime");
      val.dateTime = CMNewDateTime (_broker, &rc);
      rc = CMSetProperty (ci, "d", &val, CMPI_dateTime);
    }
  // This means include _all_ of the properties
  // This means include _all_ of the properties
  else if ((strncmp (property, "*", 1) == 0) && (strlen (property) == 1))
    {
      PROV_LOG ("---- All properties");
      _setProperty (ci, "ElementName");
      _setProperty (ci, "s");
      _setProperty (ci, "c");
      _setProperty (ci, "n64");
      _setProperty (ci, "n32");
      _setProperty (ci, "n16");
      _setProperty (ci, "n8");
      _setProperty (ci, "s64");
      _setProperty (ci, "s32");
      _setProperty (ci, "s16");
      _setProperty (ci, "s8");
      _setProperty (ci, "r64");
      _setProperty (ci, "r32");
      _setProperty (ci, "d");
      _setProperty (ci, "b");
    }
  else
    {
      PROV_LOG ("---- No implementation for property: %s", property);
      return 1;
    }
  return 0;
}
          /* and many more .. */
static CMPISelectExp* construct_instance(
    const CMPIBroker* _broker,
    const char* query,
    const char* lang,
    CMPIInstance* inst)
{
  CMPIStatus rc = { CMPI_RC_OK, NULL };
  CMPISelectExp *se_def = NULL;
  unsigned int idx;
  CMPIData data;
  CMPIArray *projection = NULL;
  CMPICount cnt = 0;
  int rc_setProperty = 0;

  /* This is used to figure what properties to construct against */
  se_def = CMNewSelectExp (_broker, query, lang, &projection, &rc);
  check_CMPIStatus(rc);
  if (se_def)
    {
      PROV_LOG ("--- Projection list is: ");
      if (projection)
        {
          cnt = CMGetArrayCount (projection, &rc);
  check_CMPIStatus(rc);
          PROV_LOG ("---- CMGetArrayCount, %d", cnt);
          for (idx = 0; idx < cnt; idx++)
            {
              data = CMGetArrayElementAt (projection, idx, &rc);
  check_CMPIStatus(rc);
              PROV_LOG ("--- CMGetArrayElementAt (%d), type is %d",
                  idx, data.type);
              if (data.type == CMPI_chars)
              {
                  PROV_LOG ("---- %s (chars)", data.value.chars);
                  rc_setProperty = _setProperty (inst, data.value.chars);
                  if (rc_setProperty)
                  {
                    PROV_LOG ("--- Error finding the property");
                  // At which point we would leave the function - as we cannot
                  // satisfy the request. But this is a test-case provider so
                  // we are continuing on and we just won't send the instance.
                  // Wait you say, won't CMEvaluteSelExp figure this too - yes,
                  // but only the CQL one. The WQL is not smart enough
                  goto error;
                  }
              }
              if (data.type == CMPI_string)
                {
                  PROV_LOG ("---- %s (string)",
                            CMGetCharsPtr (data.value.string, &rc));
                  // The _setProperty is a simple function to set
                  // _only_ properties that are needed.
                  rc_setProperty =
                    _setProperty (inst, CMGetCharsPtr (data.value.string, &rc));
                  if (rc_setProperty)
                  {
                      PROV_LOG ("--- Error finding the property");
                      goto error;
                  }
                }
            }

        }
      else
        {
          // Just add all of them then.
          PROV_LOG ("-- #4.2 Projection list is NULL. Meaning *");
          _setProperty (inst, "*");
        }
    }
exit:
  return se_def;
error:
  CMRelease(se_def);
  se_def = NULL;
  goto exit;
}

/* ---------------------------------------------------------------------------*/
/*                      Instance Provider Interface                           */
/* ---------------------------------------------------------------------------*/

#ifdef CMPI_VER_100
CMPIStatus
TestCMPIExecQueryProviderCleanup (CMPIInstanceMI * mi,
                                  const CMPIContext * ctx, CMPIBoolean term)
#else
CMPIStatus
TestCMPIExecQueryProviderCleanup (CMPIInstanceMI * mi, CMPIContext * ctx)
#endif
{

  //PROV_LOG("--- %s CMPI Cleanup() called",_ClassName);
  //PROV_LOG("--- %s CMPI Cleanup() exited",_ClassName);
  CMReturn (CMPI_RC_OK);
}

#ifdef CMPI_VER_100
CMPIStatus
TestCMPIExecQueryProviderEnumInstanceNames (CMPIInstanceMI * mi,
                                            const CMPIContext * ctx,
                                            const CMPIResult * rslt,
                                            const CMPIObjectPath * ref)
#else
CMPIStatus
TestCMPIExecQueryProviderEnumInstanceNames (CMPIInstanceMI * mi,
                                            CMPIContext * ctx,
                                            CMPIResult * rslt,
                                            CMPIObjectPath * ref)
#endif
{

  CMReturn (CMPI_RC_ERR_NOT_SUPPORTED);
}

#ifdef CMPI_VER_100
CMPIStatus
TestCMPIExecQueryProviderEnumInstances (CMPIInstanceMI * mi,
                                            const CMPIContext * ctx,
                                            const CMPIResult * rslt,
                                            const CMPIObjectPath * ref,
                                            const char **properties)
#else
CMPIStatus
TestCMPIExecQueryProviderEnumInstances (CMPIInstanceMI * mi,
                                            CMPIContext * ctx,
                                            CMPIResult * rslt,
                                            CMPIObjectPath * ref,
                                            char **properties)
#endif
{

  CMReturn (CMPI_RC_ERR_NOT_SUPPORTED);
}

#ifdef CMPI_VER_100
CMPIStatus
TestCMPIExecQueryProviderGetInstance (CMPIInstanceMI * mi,
                                      const CMPIContext * ctx,
                                      const CMPIResult * rslt,
                                      const CMPIObjectPath * cop,
                                      const char **properties)
#else
CMPIStatus
TestCMPIExecQueryProviderGetInstance (CMPIInstanceMI * mi,
                                      CMPIContext * ctx,
                                      CMPIResult * rslt,
                                      CMPIObjectPath * cop, char **properties)
#endif
{
  CMReturn (CMPI_RC_ERR_NOT_SUPPORTED);
}

#ifdef CMPI_VER_100
CMPIStatus
TestCMPIExecQueryProviderCreateInstance (CMPIInstanceMI * mi,
                                         const CMPIContext * ctx,
                                         const CMPIResult * rslt,
                                         const CMPIObjectPath * cop,
                                         const CMPIInstance * ci)
#else
CMPIStatus
TestCMPIExecQueryProviderCreateInstance (CMPIInstanceMI * mi,
                                         CMPIContext * ctx,
                                         CMPIResult * rslt,
                                         CMPIObjectPath * cop,
                                         CMPIInstance * ci)
#endif
{
  CMReturn (CMPI_RC_ERR_NOT_SUPPORTED);
}

#ifdef CMPI_VER_100
CMPIStatus
TestCMPIExecQueryProviderModifyInstance (CMPIInstanceMI * mi,
                                         const CMPIContext * ctx,
                                         const CMPIResult * rslt,
                                         const CMPIObjectPath * cop,
                                         const CMPIInstance * ci,
                                         const char **properties)
#else
CMPIStatus
TestCMPIExecQueryProviderSetInstance (CMPIInstanceMI * mi,
                                      CMPIContext * ctx,
                                      CMPIResult * rslt,
                                      CMPIObjectPath * cop,
                                      CMPIInstance * ci, char **properties)
#endif
{
  CMReturn (CMPI_RC_ERR_NOT_SUPPORTED);
}

#ifdef CMPI_VER_100
CMPIStatus
TestCMPIExecQueryProviderDeleteInstance (CMPIInstanceMI * mi,
                                         const CMPIContext * ctx,
                                         const CMPIResult * rslt,
                                         const CMPIObjectPath * cop)
#else
CMPIStatus
TestCMPIExecQueryProviderDeleteInstance (CMPIInstanceMI * mi,
                                         CMPIContext * ctx,
                                         CMPIResult * rslt,
                                         CMPIObjectPath * cop)
#endif
{
  CMReturn (CMPI_RC_ERR_NOT_SUPPORTED);
}

#ifdef CMPI_VER_100
CMPIStatus
TestCMPIExecQueryProviderExecQuery (CMPIInstanceMI * mi,
                                    const CMPIContext * ctx,
                                    const CMPIResult * rslt,
                                    const CMPIObjectPath * ref,
                                    const char *lang, const char *query)
#else
CMPIStatus
TestCMPIExecQueryProviderExecQuery (CMPIInstanceMI * mi,
                                    CMPIContext * ctx,
                                    CMPIResult * rslt,
                                    CMPIObjectPath * ref,
                                    char *lang, char *query)
#endif
{

  CMPISelectExp *se_def = NULL;
  CMPIBoolean evalRes;
  CMPIInstance *inst = NULL;
  CMPIObjectPath *objPath = NULL;

  PROV_LOG_OPEN (_ClassName, _ProviderLocation);

  PROV_LOG ("--- %s CMPI ExecQuery() called", _ClassName);

  PROV_LOG ("-- #1 MakeObjectPath");
  // Create instance

  objPath = make_ObjectPath (_broker, _Namespace, _ClassName);


  inst = make_Instance (objPath);

  // Try some bogus ones first.
  PROV_LOG ("--- Query: [%s], language: [%s]", query, "DMTF:CQL");
  se_def=construct_instance(_broker, query, "DMTF:CQL", inst);

  if (evaluate(se_def, inst,  instance_accessor, (void *)_broker)==CMPI_true)
  {
      PROV_LOG("Query (%s) returns true when using DMTF:CQL query language.",
          query);
  }
  if (se_def)
  {
      CMRelease(se_def); se_def = NULL;
  }

  PROV_LOG ("--- Query: [%s], language: [%s]", query, lang);
  se_def=construct_instance(_broker, query, lang, inst);

  evalRes = evaluate(se_def, inst,  instance_accessor, (void *)_broker);
  if (evalRes)
  {
       PROV_LOG("Returning instance for query:.%s",query);
       CMReturnInstance (rslt, inst);
       CMReturnDone (rslt);
  }

  PROV_LOG ("--- %s CMPI ExecQuery() exited", _ClassName);
  PROV_LOG_CLOSE ();
  CMReturn (CMPI_RC_OK);
}


/* ---------------------------------------------------------------------------*/
/*                              Provider Factory                              */
/* ---------------------------------------------------------------------------*/

CMInstanceMIStub (TestCMPIExecQueryProvider,
                  TestCMPIExecQueryProvider, _broker, CMNoHook);


/* ---------------------------------------------------------------------------*/
/*             end of TestCMPIProvider                      */
/* ---------------------------------------------------------------------------*/
