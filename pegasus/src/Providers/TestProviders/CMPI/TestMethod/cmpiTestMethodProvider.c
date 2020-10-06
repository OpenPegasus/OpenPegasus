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
#include <ctype.h>
#include <stdarg.h>

#include <Pegasus/Provider/CMPI/cmpidt.h>
#include <Pegasus/Provider/CMPI/cmpift.h>
#include <Pegasus/Provider/CMPI/cmpimacs.h>
#include <Providers/TestProviders/CMPI/TestUtilLib/cmpiUtilLib.h>

#define _ClassName "TestCMPI_Method"
#define _ClassName_size strlen(_ClassName)
#define _Namespace    "test/TestProvider"
#define _PersonClass  "CMPI_TEST_Person"

#define _ProviderLocation "/src/Providers/TestProviders/CMPI/TestMethod/tests/"

#ifdef CMPI_VER_100
static const CMPIBroker *_broker;
#else
static CMPIBroker *_broker;
#endif

#ifdef __GNUC__
# define UINT64_LITERAL(X) ((CMPIUint64)X##ULL)
#else
# define UINT64_LITERAL(X) ((CMPIUint64)X)
#endif

/* ---------------------------------------------------------------------------*/
/*                       CMPI Helper function                        */
/* ---------------------------------------------------------------------------*/

CMPIObjectPath * make_ObjectPath (
    const CMPIBroker *broker,
    const char *ns,
    const char *className)
{
    CMPIObjectPath *objPath = NULL;
    CMPIStatus rc = { CMPI_RC_OK, NULL };

    PROV_LOG ("++++  make_ObjectPath: CMNewObjectPath");
    objPath = CMNewObjectPath (broker, ns, className, &rc);

    PROV_LOG ("++++  CMNewObjectPath : (%s)", strCMPIStatus (rc));
    CMAddKey (objPath, "ElementName", (CMPIValue *) className, CMPI_chars);

    return objPath;
}

CMPIInstance * make_Instance (const CMPIObjectPath * op)
{
    CMPIStatus rc = { CMPI_RC_OK, NULL };
    CMPIInstance *ci = NULL;

    PROV_LOG ("++++ make_Instance: CMNewInstance");
    ci = CMNewInstance (_broker, op, &rc);
    PROV_LOG ("++++  CMNewInstance : (%s)", strCMPIStatus (rc));
    if (rc.rc == CMPI_RC_ERR_NOT_FOUND)
    {
        PROV_LOG (" ---- Class %s is not found in the %s namespace!",
            _ClassName, _Namespace);
        return NULL;
    }
    return ci;
}

/*
 * Test routines
 */

 int
_CDGetType (const void *o, char **result)
{

  CMPIStatus rc = { CMPI_RC_OK, NULL };
  CMPIString *type;

  PROV_LOG ("++++ #A CDGetType");
  type = CDGetType (_broker, o, &rc);
  if (type)
    {
      PROV_LOG ("++++ %s (%s)", CMGetCharsPtr (type,NULL), strCMPIStatus (rc));
      // The result of the call is put in 'result' parameter.
      *result = (char *) strdup ((const char *) CMGetCharsPtr (type,NULL));
      CMRelease (type);
    }
  else
    {
      PROV_LOG ("---- No type extracted.");
      *result = strdup ("No type extracted.");
    }
  if (rc.rc == CMPI_RC_OK)
    return 0;
  return 1;
}

 int
_CDToString (const void *o, char **result)
{
  CMPIStatus rc = { CMPI_RC_OK, NULL };
  CMPIString *type;
  const char *p, *q;

  PROV_LOG ("++++ #B CDToString");
  type = CDToString (_broker, o, &rc);
  if (type)
    {
      p = CMGetCharsPtr (type,NULL);
      // The pointer to the null value.
      q = p + strlen (p);
      // The first couple of bytes are the address of the data. We don't want
      // it here so we are going to skip over them.
      while ((*p != ':') && (p < q))
        p++;
      // Skip over the space after the ':'
      p++;
      *result = strdup (p);
      PROV_LOG ("++++ %s (%s)", *result, strCMPIStatus (rc));
      CMRelease (type);
    }
  else
    {
      PROV_LOG ("---- No string extracted.");
      *result = strdup ("No string extracted.");
    }

  if (rc.rc == CMPI_RC_OK)
    return 0;
  return 1;
}

 int
_CDIsOfType (const void *o, char **result)
{
  CMPIStatus rc = { CMPI_RC_OK, NULL };
  CMPIBoolean isOfType = 0;

  PROV_LOG ("++++ #D CDIsOfType ");

  isOfType = CDIsOfType (_broker, o, "CMPIInstance", &rc);
  if (isOfType)
    {
      *result = strdup ("++++ CMPIInstance = Yes");
      goto end;
    }

  isOfType = CDIsOfType (_broker, o, "CMPIObjectPath", &rc);
  if (isOfType)
    {
      *result = strdup ("++++ CMPIObjectPath= Yes");
      goto end;
    }

  isOfType = CDIsOfType (_broker, o, "CMPIBroker", &rc);
  if (isOfType)
    {
      *result = strdup ("++++ CMPIBroker = Yes");
      goto end;
    }

  isOfType = CDIsOfType (_broker, o, "CMPIString", &rc);
  if (isOfType)
    {
      *result = strdup ("++++ CMPIString = Yes");
      goto end;
    }

  isOfType = CDIsOfType (_broker, o, "CMPIDateTime", &rc);
  if (isOfType)
    {
      *result = strdup ("++++ CMPIDateTime  = Yes");
      goto end;
    }

  isOfType = CDIsOfType (_broker, o, "CMPIArgs", &rc);
  if (isOfType)
    {
      *result = strdup ("++++ CMPIArgs = Yes");
      goto end;
    }

  isOfType = CDIsOfType (_broker, o, "CMPISelectExp", &rc);
  if (isOfType)
    {
      *result = strdup ("++++ CMPISelectExp = Yes");
      goto end;
    }

  isOfType = CDIsOfType (_broker, o, "CMPISelectCond", &rc);
  if (isOfType)
    {
      *result = strdup ("++++ CMPISelectCond = Yes");
      goto end;
    }

  isOfType = CDIsOfType (_broker, o, "CMPISubCond", &rc);
  if (isOfType)
    {
      *result = strdup ("++++ CMPISubCond = Yes");
      goto end;
    }

  isOfType = CDIsOfType (_broker, o, "CMPIPredicate", &rc);
  if (isOfType)
    {
      *result = strdup ("++++ CMPIPredicate = Yes");
      goto end;
    }

  isOfType = CDIsOfType (_broker, o, "CMPIArray", &rc);
  if (isOfType)
    {
      *result = strdup ("++++ CMPIArray = Yes");
      goto end;
    }

  isOfType = CDIsOfType (_broker, o, "CMPIEnumeration", &rc);
  if (isOfType)
    {
      *result = strdup ("++++ CMPIEnumeration = Yes");
      goto end;
    }

  isOfType = CDIsOfType (_broker, o, "CMPIContext", &rc);
  if (isOfType)
    {
      *result = strdup ("++++ CMPIContext = Yes");
      goto end;
    }

  isOfType = CDIsOfType (_broker, o, "CMPIResult", &rc);
  if (isOfType)
    {
      *result = strdup ("++++ CMPIResult = Yes");
      goto end;
    }

end:

  PROV_LOG ("%s", *result);

  if (rc.rc == CMPI_RC_OK)
    return 0;
  return 1;
}

static int
_CMGetMessage (char **result)
{

  CMPIString *str = NULL;
  CMPIStatus rc = { CMPI_RC_OK, NULL };

  PROV_LOG ("++++ #A CMGetMessage");
  /* Gets the internationalized string */
  str =
    CMGetMessage (_broker, "Common.CIMStatusCode.CIM_ERR_SUCCESS",
                  "CIM_ERR_SUCCESS: Successful.", &rc, 0);
  PROV_LOG ("++++ (%s)", strCMPIStatus (rc));
  if (str)
    {
      PROV_LOG ("++++ [%s]", CMGetCharsPtr (str,NULL));
      *result = strdup (CMGetCharsPtr (str,NULL));
    }
  if (rc.rc == CMPI_RC_OK)
    return 0;
  return 1;
}

static int _CMGetMessage2 (char **result, const char* msgFile,
    const char* msgId, const char* constinsert1, CMPIUint32 insert2)
{
    CMPIString *str = NULL;
    CMPIStatus rc = { CMPI_RC_OK, NULL};
    CMPIMsgFileHandle msgFileHandle;
    char *insert1 = strdup(constinsert1);

    PROV_LOG ("++++ #A CMOpenMessageFile");
    rc = CMOpenMessageFile(_broker, msgFile, &msgFileHandle);
    PROV_LOG ("++++ (%s)", strCMPIStatus (rc));
    PROV_LOG ("++++ #B CMGetMessage2");
    /* Gets the internationalized string */
    str = CMGetMessage2 (_broker, msgId, msgFileHandle,
        "CIM_ERR_SUCCESS: Successful.", &rc,
        CMFmtArgs2(CMFmtChars(insert1),CMFmtUint(insert2)));
    free(insert1);
    PROV_LOG ("++++ (%s)", strCMPIStatus (rc));
    if (str)
    {
        PROV_LOG ("++++ [%s]", CMGetCharsPtr (str,NULL));
        *result = strdup (CMGetCharsPtr (str,NULL));
    }

    PROV_LOG ("++++ #C CMCloseMessageFile");
    rc = CMCloseMessageFile(_broker, msgFileHandle);
    PROV_LOG ("++++ (%s)", strCMPIStatus (rc));

    if (rc.rc == CMPI_RC_OK)
        return 0;
    return 1;
}

static int
_CMLogMessage (char **result)
{
  CMPIStatus rc = { CMPI_RC_OK, NULL };
  CMPIString *str = CMNewString (_broker, "CMLogMessage", &rc);

  PROV_LOG ("++++ #B CMLogMessage");
  rc.rc = CMPI_RC_OK;
  rc = CMLogMessage (_broker, 64, _ClassName, "Log message", NULL);
  PROV_LOG ("++++ (%s)", strCMPIStatus (rc));

  // Ths should return CMPI_RC_ERR_INVALID_PARAMETER
  rc.rc = CMPI_RC_OK;
  rc = CMLogMessage (_broker, 2, NULL, NULL, NULL);
  PROV_LOG ("++++ (%s)", strCMPIStatus (rc));

  // This should return CMPI_RC_OK
  rc.rc = CMPI_RC_OK;
  rc = CMLogMessage (_broker, 1, NULL, "LogMessage with NULL", NULL);
  PROV_LOG ("++++ (%s)", strCMPIStatus (rc));

  rc.rc = CMPI_RC_OK;
  rc = CMLogMessage (_broker, 1, _ClassName, NULL, str);
  PROV_LOG ("++++ (%s)", strCMPIStatus (rc));

  if (rc.rc == CMPI_RC_OK)
    return 0;
  return 1;
}

static int
_CMTraceMessage (char **result)
{
  CMPIStatus rc = { CMPI_RC_OK, NULL };
  CMPIString *str = CMNewString (_broker, "CMTraceMessage", &rc);

  PROV_LOG ("++++ #C CMLogTrace");

  // Ths should return CMPI_RC_ERR_INVALID_PARAMETER
  rc.rc = CMPI_RC_OK;
  rc = CMTraceMessage (_broker, 2, NULL, NULL, NULL);
  PROV_LOG ("++++ (%s)", strCMPIStatus (rc));

  rc.rc = CMPI_RC_OK;
  rc = CMTraceMessage (_broker, 1, _ClassName, "CMTraceMessage", NULL);
  PROV_LOG ("++++ (%s)", strCMPIStatus (rc));

  rc.rc = CMPI_RC_OK;
  rc = CMTraceMessage (_broker, 4, "Authorization", NULL, str);
  PROV_LOG ("++++ (%s)", strCMPIStatus (rc));

  if (rc.rc == CMPI_RC_OK)
    return 0;
  return 1;
}

static CMPIInstance *
_createInstance()
{
  CMPIStatus rc = { CMPI_RC_OK, NULL };
  CMPIObjectPath *objPath = NULL;
  CMPIObjectPath *fake_objPath  = NULL;
  CMPIObjectPath *temp_objPath = NULL;
  CMPIString *objName = NULL;
  CMPIInstance *inst = NULL;
  CMPIString *str;
  CMPIValue v;

  PROV_LOG("Calling CMNewObjectPath for %s", _ClassName );
  objPath  = CMNewObjectPath (_broker, _Namespace, "TestCMPI_Instance", &rc);
  CMAddKey (objPath, "ElementName", (CMPIValue *) "ObjPath", CMPI_chars);
  PROV_LOG ("---- (rc:%s)", strCMPIStatus (rc));

  PROV_LOG("Calling CMNewInstance ");
  inst = CMNewInstance(_broker, objPath, &rc);
  PROV_LOG ("---- (rc:%s)", strCMPIStatus (rc));

  str = CMNewString (_broker, "", 0);
  v.string = str;
  CMSetProperty(inst, "s", &v, CMPI_string);

  // Get the object path
  PROV_LOG("Calling CMGetObjectPath");
  temp_objPath = CMGetObjectPath(inst, &rc);
  PROV_LOG ("---- (rc:%s)", strCMPIStatus (rc));

  objName = CMObjectPathToString(temp_objPath, &rc);
  PROV_LOG ("---- Object path is %s (rc:%s)", CMGetCharsPtr(objName,NULL),
      strCMPIStatus (rc));

  // Create a new ObjectPath, in a different namespace.
  PROV_LOG("Calling CMNewObjectPath for %s", "TestCMPI_Instance");
  fake_objPath =
      CMNewObjectPath (_broker, "root/cimv2", "TestCMPI_Instance", &rc);
  CMAddKey (fake_objPath, "ElementName", (CMPIValue *) "Fake_ObjPath",
      CMPI_chars);

  PROV_LOG ("---- (%s)", strCMPIStatus (rc));

  objName = CMObjectPathToString(fake_objPath, &rc);
  PROV_LOG ("---- Object path: %s (rc:%s)", CMGetCharsPtr(objName,NULL),
      strCMPIStatus (rc));

  // Setting objPath to fake_ObjPath
  PROV_LOG("Calling CMSetObjectPath with object path: %s",
      CMGetCharsPtr(objName,NULL));
  rc = CMSetObjectPath(inst, fake_objPath);
  PROV_LOG ("---- (%s)", strCMPIStatus (rc));

  // Please note that this instance now has this objectPath
  PROV_LOG("Calling CMGetObjectPath");
  temp_objPath = CMGetObjectPath(inst, &rc);
  PROV_LOG ("---- (rc:%s)", strCMPIStatus (rc));
  objName = CMObjectPathToString(temp_objPath, &rc);
  PROV_LOG ("---- Object path is %s (rc:%s)", CMGetCharsPtr(objName,NULL),
      strCMPIStatus (rc));

  return inst;
}

//This function is to test CMPIArray Clone function
static int _testArrayClone(const CMPIContext* ctx)
{
    struct array_types
    {
        //Type of the element in the array
        CMPIType element_type;
        // Name of the Array type in string format
        char* typeAName;
    }types_arr[] = {
        {CMPI_string,
        "CMPI_string"},

        {CMPI_dateTime,
        "CMPI_dateTime"},

        {CMPI_instance,
        "CMPI_instance"},

        {CMPI_args,
        "CMPI_args"},

        {CMPI_charsptr,
        "CMPI_charsptr"},

        {CMPI_enumeration,
        "CMPI_enumeration"},

        {CMPI_ref,
        "CMPI_ref"}};



    int i ,flag, size;
    CMPIStatus rc = { CMPI_RC_OK, NULL };
    CMPIStatus rc1 = { CMPI_RC_OK, NULL };
    CMPIArray *arr = NULL;
    CMPIArray *arrClone = NULL;
    CMPIArray *arr_ptr = NULL;
    CMPIArray *arrClone_ptr = NULL;
    CMPIString* retNamespace = NULL;
    CMPIString* retClassname = NULL;
    CMPIValue value, value1;
    CMPIData arr_data;
    CMPIData arrClone_data;
    CMPIType arr_type;
    CMPIType arrClone_type;
    CMPICount arr_size;
    CMPICount arrClone_size;
    CMPIData data, dataClone;
    CMPIObjectPath* objPath = make_ObjectPath(_broker,
        _Namespace,
        _ClassName);
    CMPIUint64 datetime1, datetime2;
    const char* str1;
    const char* str2;

//Size of the array_types array set at the time of preprocessing
    size = 7;

    PROV_LOG("++++  Entering testArrayClone");

    for ( i = 0 ; i < size; i++)
    {
//Initializing the elements that will constitute the array
        switch(types_arr[i].element_type)
        {
            case CMPI_string:
                value.string = CMNewString(_broker, "string", &rc);
                break;

            case CMPI_dateTime:
                value.dateTime = CMNewDateTime(_broker, &rc);
                break;

            case CMPI_ref:
                value.ref = CMNewObjectPath (_broker,
                    "root/cimv2",
                    "TestCMPI_Instance",
                    &rc);
                break;

            case CMPI_instance:
                value.inst = make_Instance(objPath);
                value1.uint32 = 20;
                rc = CMSetProperty(value.inst,
                    "Property1",
                    &value1,
                    CMPI_uint32);
                break;

            case CMPI_args:
                value.args = CMNewArgs(_broker, &rc);
                value1.uint8 = 32;
                CMAddArg(value.args, "Argument", &value1, CMPI_uint8);
                break;

            case CMPI_enumeration:
                objPath = make_ObjectPath(_broker, _Namespace, _PersonClass);
                value.Enum = CBEnumInstances(_broker, ctx, objPath, NULL, &rc);
                break;

            case CMPI_charsptr:
                value.dataPtr.ptr = "String";
                value.dataPtr.length =
                     (CMPICount)(strlen((value.dataPtr.ptr)) + 1);
                break;

        }
        PROV_LOG("++++  Testing for %s type", types_arr[i].typeAName);
//Allocating the space for the array and adding the initialized element to it
        arr = CMNewArray (_broker, 1, types_arr[i].element_type, &rc);
        PROV_LOG("++++ Status of creation of CMPIArray : (%s) of type (%s)",
            strCMPIStatus (rc),
            types_arr[i].typeAName);

        rc = CMSetArrayElementAt(arr, 0, &value, types_arr[i].element_type);
        PROV_LOG("++++ Status of CMSetArrayElementAt : (%s)",
            strCMPIStatus (rc));

        arrClone = CMClone(arr, &rc);
        PROV_LOG("++++ Status of Cloning CMPIArray : (%s) of type (%s)",
            strCMPIStatus (rc),
            types_arr[i].typeAName);

        flag = 1;

        arr_size = CMGetArrayCount(arr, &rc);
        PROV_LOG("++++ Status of CMGetArrayCount of Array: (%s)",
            strCMPIStatus (rc));

        arrClone_size = CMGetArrayCount(arrClone, &rc);
        PROV_LOG("++++ Status of CMGetArrayCount of Cloned Array: (%s)",
            strCMPIStatus (rc));

        arr_data = CMGetArrayElementAt(arr, 0, &rc);
        PROV_LOG("++++ Status of CMGetArrayElementAt of Array: (%s)",
            strCMPIStatus (rc));

        arrClone_data = CMGetArrayElementAt(arrClone, 0, &rc);
        PROV_LOG("++++ Status of CMGetArrayElementAt of Cloned Array: (%s)",
            strCMPIStatus (rc));

        arr_type = CMGetArrayType(arr, &rc);
        PROV_LOG("++++ Status of CMGetArrayType of Array: (%s)",
            strCMPIStatus (rc));

        arrClone_type = CMGetArrayType(arrClone, &rc);
        PROV_LOG("++++ Status of CMGetArrayType of Array: (%s)",
            strCMPIStatus (rc));

        switch(types_arr[i].element_type)
        {
            case CMPI_string:
                str1 = CMGetCharsPtr(arr_data.value.string, &rc);
                PROV_LOG("**** Value of string in original array is : %s ",
                    str1);
                str2 = CMGetCharsPtr(arrClone_data.value.string, &rc1);
                PROV_LOG("**** Value of string in cloned array is : %s ",
                    str2);
                if ((rc.rc != CMPI_RC_OK) ||
                    (rc1.rc != CMPI_RC_OK) ||
                    strcmp(str1, str2) ||
                    (arr_type != arrClone_type) ||
                    (arr_size != arrClone_size))
                {
                    flag = 0;
                    PROV_LOG("++++  Cloning of array of type %s is"
                        "UnSuccessful", types_arr[i].typeAName);
                }
                break;

            case CMPI_dateTime:
                datetime1 = CMGetBinaryFormat(arr_data.value.dateTime,
                    &rc);
                datetime2 = CMGetBinaryFormat(arrClone_data.value.dateTime,
                    &rc1);
                if ((rc.rc != CMPI_RC_OK) ||
                    (rc1.rc != CMPI_RC_OK) ||
                    (datetime1 != datetime2) ||
                    (arr_type != arrClone_type) ||
                    (arr_size != arrClone_size))
                {
                    flag = 0;
                    PROV_LOG("++++  Cloning of array of type %s"
                        " is UnSuccessful",
                        types_arr[i].typeAName);
                }
                rc = CMRelease(value.dateTime);
                PROV_LOG("++++ Status of CMRelease(value.dateTime) : (%s)",
                    strCMPIStatus(rc));
                break;

            case CMPI_ref:
                retNamespace = CMGetNameSpace(arrClone_data.value.ref, &rc);
                retClassname = CMGetClassName(arrClone_data.value.ref, &rc1);
                if ((rc.rc == CMPI_RC_OK) &&
                    (rc1.rc == CMPI_RC_OK) &&
                    (arr_type == arrClone_type) &&
                    (arr_size == arrClone_size) )
                {
                    str1 = CMGetCharsPtr(retNamespace, &rc);
                    str2 = CMGetCharsPtr(retClassname, &rc1);
                    if ((rc.rc == CMPI_RC_OK) &&
                        (rc1.rc == CMPI_RC_OK))
                    {
                        if ((strcmp(str1, "root/cimv2")) ||
                            (strcmp(str2, "TestCMPI_Instance")))
                        {
                            flag = 0;
                            PROV_LOG("++++  Cloning of array of type %s"
                                " is UnSuccessful",
                                types_arr[i].typeAName);
                        }
                    }
                    else
                    {
                        flag = 0;
                        PROV_LOG("++++  Cloning of array of type %s is "
                            "UnSuccessful",
                            types_arr[i].typeAName);
                    }
                }
                else
                {
                    flag = 0;
                    PROV_LOG("++++  Cloning of array of type %s is"
                        " UnSuccessful", types_arr[i].typeAName);
                }
                rc = CMRelease(value.ref);
                PROV_LOG("++++ Status of CMRelease(value.ref) : (%s)",
                    strCMPIStatus(rc));
                break;

            case CMPI_instance:
                data = CMGetProperty(arr_data.value.inst,
                    "Property1", &rc);
                dataClone = CMGetProperty(arrClone_data.value.inst,
                    "Property1",
                    &rc1);
                if ((rc.rc != CMPI_RC_OK) ||
                    (rc1.rc != CMPI_RC_OK) ||
                    (data.value.uint32 != dataClone.value.uint32) ||
                    (arr_type != arrClone_type) ||
                    (arr_size != arrClone_size))
                {
                    flag = 0;
                    PROV_LOG("++++  Cloning of array of type %s"
                        " is UnSuccessful",
                        types_arr[i].typeAName);
                }
                rc = CMRelease(value.inst);
                PROV_LOG("++++ Status of CMRelease(value.inst) : (%s)",
                    strCMPIStatus(rc));
                break;

            case CMPI_args:
                data = CMGetArg(arr_data.value.args, "Argument", &rc);
                dataClone = CMGetArg(arrClone_data.value.args, "Argument",
                    &rc1);
                 if ((rc.rc != CMPI_RC_OK) ||
                    (rc1.rc != CMPI_RC_OK) ||
                    (data.value.uint32 != dataClone.value.uint32) ||
                    (arr_type != arrClone_type) ||
                    (arr_size != arrClone_size))
                {
                    flag = 0;
                    PROV_LOG("++++  Cloning of array of type %s is"
                        "UnSuccessful", types_arr[i].typeAName);
                }
                rc = CMRelease(value.args);
                PROV_LOG("++++ Status of CMRelease(value.args) : (%s)",
                    strCMPIStatus(rc));
                break;

            case CMPI_charsptr:
                if (strcmp(arr_data.value.dataPtr.ptr,
                        arrClone_data.value.dataPtr.ptr) ||
                    (arr_type != arrClone_type) ||
                    (arr_size != arrClone_size) ||
                    (arr_data.value.dataPtr.length !=
                        arrClone_data.value.dataPtr.length))
                {
                    flag = 0;
                    PROV_LOG("++++  Cloning of array of type %s "
                        "is UnSuccessful",
                        types_arr[i].typeAName);
                }
                PROV_LOG("**** Value of charsptr in original array is : %s ",
                    arr_data.value.dataPtr.ptr);
                PROV_LOG("**** Value of charsptr in cloned array is : %s ",
                    arrClone_data.value.dataPtr.ptr);
                break;

            case CMPI_enumeration:
                arr_ptr = CMToArray(arr_data.value.Enum, &rc);
                arrClone_ptr = CMToArray(arrClone_data.value.Enum, &rc1);
                if (arr_ptr != NULL &&
                    arrClone_ptr != NULL &&
                    (rc.rc == CMPI_RC_OK) &&
                    (rc1.rc == CMPI_RC_OK))
                {
                    if (CMGetArrayCount(arr_ptr, &rc) ==
                        CMGetArrayCount(arrClone_ptr, &rc1))
                    {
                        PROV_LOG("**** Status of CMGetArrayCount for CMPI_enum"
                            " in original array (%s) ",
                            strCMPIStatus(rc));
                        PROV_LOG("**** Count returned for CMPI_enum"
                            "in original array (%d) ",
                            CMGetArrayCount(arr_ptr, &rc));
                        PROV_LOG("**** Status of CMGetArrayCount for CMPI_enum"
                            " in cloned array (%s) ",
                            strCMPIStatus(rc1));
                        PROV_LOG("**** Count returned for CMPI_enum"
                            "in cloned array (%d) ",
                            CMGetArrayCount(arrClone_ptr, &rc));

                        while(CMHasNext(arr_data.value.Enum,&rc))
                        {
                            data = CMGetNext(arr_data.value.Enum, &rc);
                            dataClone = CMGetNext(
                                arrClone_data.value.Enum,
                                &rc1);
                            if((rc.rc != CMPI_RC_OK) ||
                                (rc1.rc != CMPI_RC_OK) ||
                                (data.type != dataClone.type))
                            {
                                flag = 0;
                                PROV_LOG("++++  Cloning of array of type %s "
                                    "is UnSuccessful",
                                    types_arr[i].typeAName);
                            }
                        }
                    }
                    else
                    {
                        flag = 0;
                        PROV_LOG("++++  Cloning of array of type %s "
                            "is UnSuccessful",
                            types_arr[i].typeAName);
                    }
                }
                else
                {
                    flag = 0;
                    PROV_LOG("++++  Cloning of array of type %s "
                        "is UnSuccessful",
                        types_arr[i].typeAName);
                }
                rc = CMRelease(value.Enum);
                PROV_LOG("++++ Status of CMRelease(value.Enum) : (%s)",
                    strCMPIStatus(rc));
                break;

        }
        if (flag)
        {
            PROV_LOG("++++  Cloning of array of type %s is Successful",
                types_arr[i].typeAName);
        }

        rc = CMRelease(arr);
        PROV_LOG("++++ Status of CMRelease(arr) : (%s)", strCMPIStatus(rc));
        rc = CMRelease(arrClone);
        PROV_LOG("++++ Status of CMRelease(arrClone) : (%s)",
            strCMPIStatus(rc));
    }
    //Test Error Paths
    arr = CMNewArray (_broker, 1, CMPI_charsptr, &rc);
    PROV_LOG("++++ Status of creation of CMPIArray : (%s) of type"
        " CMPI_uint32",
        strCMPIStatus (rc));

    rc = CMSetArrayElementAt(arr, 0, NULL, CMPI_charsptr);
    PROV_LOG("++++ Error Status of CMSetArrayElementAt with NULL value: (%s)",
        strCMPIStatus (rc));

    value.dataPtr.ptr = "String";
    value.dataPtr.length = (CMPICount)(strlen((value.dataPtr.ptr)) + 1);

    rc = CMSetArrayElementAt(arr, 1, &value, CMPI_charsptr);
    PROV_LOG("++++ Error Status of CMSetArrayElementAt with wrong"
        " position: (%s)",
        strCMPIStatus (rc));

    CMRelease(arr);
    return flag;
}

//Testing CMPIArrays with elements of different CMPITypes
//In this test case CMPIArrays of different CMPITypes are created.
//Then these arrays are added to CMPIArgs object as a new argument.
//Then array are retrieved from the CMPIArgs object using their unique name
// and verified for the expected values.
static int _testArrayTypes()
{
    struct array_types
    {
        //Type of the element in the array
        CMPIType element_type;
        // Array type
        CMPIType typeA;
        // Name of the element type in string format
        char* typeName;
        // Name of the Array type in string format
        char* typeAName;
        // Unique argument name to be used while adding the array
        // as an argument to CMPIArgs object
        char* args_name;
    }types_arr[] = {
        {CMPI_uint32,
        CMPI_uint32A,
        "CMPI_uint32",
        "CMPI_uint32A",
        "CMPI_uint32_array"},

        {CMPI_uint16,
        CMPI_uint16A,
        "CMPI_uint16",
        "CMPI_uint16A",
        "CMPI_uint16_array"},

        {CMPI_uint8,
        CMPI_uint8A,
        "CMPI_uint8",
        "CMPI_uint8A",
        "CMPI_uint8_array"},

        {CMPI_uint64,
        CMPI_uint64A,
        "CMPI_uint64",
        "CMPI_uint64A",
        "CMPI_uint64_array"},

        {CMPI_sint32,
        CMPI_sint32A,
        "CMPI_sint32",
        "CMPI_sint32A",
        "CMPI_sint32_array"},

        {CMPI_sint16,
        CMPI_sint16A,
        "CMPI_sint16",
        "CMPI_sint16A",
        "CMPI_sint16_array"},

        {CMPI_sint8,
        CMPI_sint8A,
        "CMPI_sint8",
        "CMPI_sint8A",
        "CMPI_sint8_array"},

        {CMPI_sint64,
        CMPI_sint64A,
        "CMPI_sint64",
        "CMPI_sint64A",
        "CMPI_sint64_array"},

        {CMPI_real32,
        CMPI_real32A,
        "CMPI_real32",
        "CMPI_real32A",
        "CMPI_real32_array"},

        {CMPI_real64,
        CMPI_real64A,
        "CMPI_real64",
        "CMPI_real64A",
        "CMPI_real64_array"},

        {CMPI_char16,
        CMPI_char16A,
        "CMPI_char16",
        "CMPI_char16A",
        "CMPI_char16_array"},

        {CMPI_boolean,
        CMPI_booleanA,
        "CMPI_boolean",
        "CMPI_booleanA",
        "CMPI_boolean_array"},

        {CMPI_string,
        CMPI_stringA,
        "CMPI_string",
        "CMPI_stringA",
        "CMPI_string_array"},

        {CMPI_dateTime,
        CMPI_dateTimeA,
        "CMPI_dateTime",
        "CMPI_dateTimeA",
        "CMPI_dateTime_array"},

        {CMPI_ref,
        CMPI_refA,
        "CMPI_ref",
        "CMPI_refA",
        "CMPI_ref_array"},

        {CMPI_instance,
        CMPI_instanceA,
        "CMPI_instance",
        "CMPI_instanceA",
        "CMPI_instance_array"},
//Test case for covering default case in CMPI_Value.cpp
// value2CIMValue() function
        {CMPI_null,
        CMPI_ARRAY,
        "Invalid",
        "InvalidArray",
        "Invalid_array"}};

    int i ,flag, size;
    CMPIStatus rc = { CMPI_RC_OK, NULL };
    CMPIStatus rc1 = { CMPI_RC_OK, NULL };
    CMPIArray *arr = NULL;
    CMPIString* retNamespace = NULL;
    CMPIString* retClassname = NULL;
    CMPIValue value, value1;
    CMPIData data;
    CMPIData arr_data;
    CMPIData dataInst;
    CMPIData retDataInst;
    CMPIArgs* args_ptr = NULL;
    CMPIObjectPath* objPath = make_ObjectPath(_broker,
        _Namespace,
        _ClassName);
    CMPIUint64 datetime1, datetime2;
    const char* str1;
    const char* str2;

//Size of the array_types array set at the time of preprocessing
    size = 17;

    PROV_LOG("++++  Entering testArrayTypes");

    for ( i = 0 ; i < size; i++)
    {
        args_ptr = CMNewArgs(_broker, &rc);
        PROV_LOG("++++ Status of CMNewArgs : (%s)",
            strCMPIStatus (rc));

//Initializing the elements that will constitute the array
        switch(types_arr[i].element_type)
        {
            case CMPI_uint32:
                value.uint32 = 56;
                break;

            case CMPI_uint16:
                value.uint16 = 32;
                break;

            case CMPI_uint8:
                value.uint8 = 56;
                break;

            case CMPI_uint64:
                value.uint64 = 32;
                break;

            case CMPI_sint32:
                value.sint32 = -56;
                break;

            case CMPI_sint16:
                value.sint16 = -32;
                break;

            case CMPI_sint8:
                value.sint8 = -56;
                break;

            case CMPI_sint64:
                value.sint64 = -32;
                break;

            case CMPI_real32:
                value.real32 = (CMPIReal32)-32.78;
                break;

            case CMPI_real64:
                value.real64 = -899.32;
                break;

            case CMPI_char16:
                value.char16 = 'k';
                break;

            case CMPI_string:
                value.string = CMNewString(_broker, "string", &rc);
                break;

            case CMPI_boolean:
                value.boolean = 1;
                break;

            case CMPI_dateTime:
                value.dateTime = CMNewDateTime(_broker, &rc);
                break;

            case CMPI_ref:
                value.ref = CMNewObjectPath (_broker,
                    "root/cimv2",
                    "TestCMPI_Instance",
                    &rc);
                break;

            case CMPI_null:
                value.args = NULL;
                break;

            case CMPI_instance:
                value.inst = make_Instance(objPath);
                value1.uint32 = 20;
                rc = CMSetProperty(value.inst,
                    "Property1",
                    &value1,
                    CMPI_uint32);
                break;
        }

        PROV_LOG("++++  Testing for %s type", types_arr[i].typeAName);
        arr = NULL;
//Testing for NULL array ;
        rc = CMAddArg (args_ptr,
            "EmptyArray",
            (CMPIValue *) &arr,
            types_arr[i].typeA);
        PROV_LOG("++++ Status of CMAddArg with name EmptyArray : (%s)",
            strCMPIStatus (rc));

//Allocating the space for the array and adding the initialized element to it
        arr = CMNewArray (_broker, 1, types_arr[i].element_type, &rc);
        PROV_LOG("++++ Status of creation of CMPIArray : (%s) of type (%s)",
            strCMPIStatus (rc),
            types_arr[i].typeAName);

        rc = CMSetArrayElementAt(arr, 0, &value, types_arr[i].element_type);
        PROV_LOG("++++ Status of CMSetArrayElementAt : (%s)",
            strCMPIStatus (rc));

//Adding the array as an argument to CMPIArgs
        rc = CMAddArg (args_ptr,
            types_arr[i].args_name,
            (CMPIValue *) &arr,
            types_arr[i].typeA);
        PROV_LOG("++++ Status of CMAddArg with name %s : (%s)",
            types_arr[i].args_name,
            strCMPIStatus (rc));

//Testing the retrieved arrays from CMPIArgs object
        flag = 1;
        if((types_arr[i].element_type) != CMPI_null)
        {
            data = CMGetArg(args_ptr, types_arr[i].args_name , &rc);
            PROV_LOG("++++ Status of CMGetArg with name %s : (%s)",
                types_arr[i].args_name,
                strCMPIStatus (rc));

            arr_data = CMGetArrayElementAt(data.value.array, 0, &rc);
            PROV_LOG("++++ Status of CMGetArrayElementAt : (%s)",
                strCMPIStatus (rc));

            switch(types_arr[i].element_type)
            {
                case CMPI_uint32:
                    if (arr_data.value.uint32 != value.uint32)
                    {
                        flag = 0;
                    }
                    break;

                case CMPI_uint16:
                    if (arr_data.value.uint16 != value.uint16)
                    {
                        flag = 0;
                    }
                    break;

                case CMPI_uint8:
                    if (arr_data.value.uint8 != value.uint8)
                    {
                        flag = 0;
                    }
                    break;

                case CMPI_uint64:
                    if (arr_data.value.uint64 != value.uint64)
                    {
                        flag = 0;
                    }
                    break;

                case CMPI_sint32:
                    if (arr_data.value.sint32 != value.sint32)
                    {
                        flag = 0;
                    }
                    break;

                case CMPI_sint16:
                    if (arr_data.value.sint16 != value.sint16)
                    {
                        flag = 0;
                    }
                    break;

                case CMPI_sint8:
                    if (arr_data.value.sint8 != value.sint8)
                    {
                        flag = 0;
                    }
                    break;

                case CMPI_sint64:
                    if (arr_data.value.sint64 != value.sint64)
                    {
                        flag = 0;
                    }
                    break;

                case CMPI_real32:
                    if (arr_data.value.real32 != value.real32)
                    {
                        flag = 0;
                    }
                    break;

                case CMPI_real64:
                    if (arr_data.value.real64 != value.real64)
                    {
                        flag = 0;
                    }
                    break;

                case CMPI_char16:
                    if (arr_data.value.char16 != value.char16)
                    {
                        flag = 0;
                    }
                    break;

                case CMPI_string:
                    str1 = CMGetCharsPtr(arr_data.value.string, &rc);
                    str2 = CMGetCharsPtr(value.string, &rc1);
                    if ((rc.rc != CMPI_RC_OK) ||
                        (rc1.rc != CMPI_RC_OK) ||
                        strcmp(str1, str2))
                    {
                        flag = 0;
                    }
                    break;

                case CMPI_boolean:
                    if (arr_data.value.boolean != value.boolean)
                    {
                        flag = 0;
                    }
                    break;

                case CMPI_dateTime:
                    datetime1 = CMGetBinaryFormat(arr_data.value.dateTime,
                        &rc);
                    datetime2 = CMGetBinaryFormat(value.dateTime, &rc1);
                    if ((rc.rc != CMPI_RC_OK) ||
                        (rc1.rc != CMPI_RC_OK) ||
                        (datetime1 != datetime2))
                    {
                        flag = 0;
                    }
                    rc = CMRelease(value.dateTime);
                    PROV_LOG("++++ Status of CMRelease(value.dateTime) : (%s)",
                        strCMPIStatus(rc));
                    break;

                case CMPI_ref:
                    retNamespace = CMGetNameSpace(arr_data.value.ref, &rc);
                    retClassname = CMGetClassName(arr_data.value.ref, &rc1);
                    if((rc.rc == CMPI_RC_OK) &&
                        (rc1.rc == CMPI_RC_OK))
                    {
                        str1 = CMGetCharsPtr(retNamespace, &rc);
                        str2 = CMGetCharsPtr(retClassname, &rc1);
                        if ((rc.rc == CMPI_RC_OK) &&
                            (rc1.rc == CMPI_RC_OK))
                        {
                            if ((strcmp(str1, "root/cimv2")) ||
                                (strcmp(str2, "TestCMPI_Instance")))
                            {
                                flag = 0;
                            }
                        }
                        else
                        {
                            flag = 0;
                        }
                    }
                    else
                    {
                        flag = 0;
                    }
                    rc = CMRelease(value.ref);
                    PROV_LOG("++++ Status of CMRelease(value.ref) : (%s)",
                        strCMPIStatus(rc));
                    break;

                case CMPI_instance:
                    retDataInst = CMGetProperty(arr_data.value.inst,
                        "Property1", &rc);
                    dataInst = CMGetProperty(value.inst, "Property1", &rc);
                    if (retDataInst.value.uint32 != dataInst.value.uint32)
                    {
                        flag = 0;
                    }
                    rc = CMRelease(value.inst);
                    PROV_LOG("++++ Status of CMRelease(value.inst) : (%s)",
                        strCMPIStatus(rc));
                    break;
            }
            if (data.type == types_arr[i].typeA && flag)
            {
                PROV_LOG("++++  CMGetArg : Name - %s is of type"
                    " %s ", types_arr[i].args_name, types_arr[i].typeAName);
            }
            PROV_LOG("++++ Status of CMGetArg : (%s)", strCMPIStatus (rc));
        }
        rc = CMRelease(arr);
        PROV_LOG("++++ Status of CMRelease(arr) : (%s)", strCMPIStatus(rc));
        rc = CMRelease(args_ptr);
        PROV_LOG("++++ Status of CMRelease(args_ptr) : (%s)",
            strCMPIStatus(rc));
    }
    return flag;
}// End of _testArrayTypes

//Testing CMPITypes other than CMPIArray
static int _testSimpleTypes()
{
    CMPIArgs* args_ptr = NULL;
    CMPIStatus rc = { CMPI_RC_OK, NULL };
    CMPIStatus rc1 = { CMPI_RC_OK, NULL };
    int i, flag, size;
    CMPIValue value;
    CMPIValue value1;
    CMPIData data;
    CMPIData dataInst;
    CMPIData retDataInst;
    CMPIString* retNamespace = NULL;
    CMPIString* retClassname = NULL;
    CMPIObjectPath* objPath = make_ObjectPath(_broker,
        _Namespace,
        _ClassName);
    const char* str1;
    const char* str2;

    struct array_types
    {
        CMPIType element_type;
        char* typeName;
        char* args_name;
    }types_arr[] = {

        {CMPI_instance,
        "CMPI_instance",
        "CMPI_instance"},
        {CMPI_ref,
        "CMPI_ref",
        "CMPI_ref"}};

    size = 2;

    flag = 1;
    for ( i = 0 ; i < size; i++)
    {
        args_ptr = CMNewArgs(_broker, &rc);
        PROV_LOG("++++ Status of CMNewArgs : (%s)",
            strCMPIStatus (rc));

        switch(types_arr[i].element_type)
        {
            case CMPI_ref:
                value.ref = CMNewObjectPath (_broker,
                    "root/cimv2",
                    "TestCMPI_Instance",
                    &rc);
                break;

            case CMPI_instance:
                value.inst = make_Instance(objPath);
                value1.uint32 = 20;
                rc = CMSetProperty(value.inst,
                    "Property1",
                    &value1,
                    CMPI_uint32);
                break;
        }
        PROV_LOG("++++  Testing for %s type", types_arr[i].typeName);
        rc = CMAddArg (args_ptr,
            types_arr[i].args_name,
            (CMPIValue *) &value,
            types_arr[i].element_type);
        PROV_LOG("++++ Status of CMAddArg with name %s : (%s)",
            types_arr[i].args_name,
            strCMPIStatus (rc));

        data = CMGetArg(args_ptr, types_arr[i].args_name , &rc);
        PROV_LOG("++++ Status of CMGetArg with name %s : (%s)",
            types_arr[i].args_name,
            strCMPIStatus (rc));

        switch(types_arr[i].element_type)
        {
            case CMPI_ref:
                retNamespace = CMGetNameSpace(data.value.ref, &rc);
                retClassname = CMGetClassName(data.value.ref, &rc1);

                if((rc.rc == CMPI_RC_OK) &&
                    (rc1.rc == CMPI_RC_OK))
                {
                    str1 = CMGetCharsPtr(retNamespace, &rc);
                    str2 = CMGetCharsPtr(retClassname, &rc1);
                    if ((rc.rc == CMPI_RC_OK) &&
                        (rc1.rc == CMPI_RC_OK))
                    {
                        if ((strcmp(str1, "root/cimv2")) ||
                            (strcmp(str2, "TestCMPI_Instance")))
                        {
                            flag = 0;
                        }
                    }
                    else
                    {
                        flag = 0;
                    }
                }
                else
                {
                    flag = 0;
                }
                rc = CMRelease(value.ref);
                PROV_LOG("++++ Status of CMRelease(value.ref) : (%s)",
                    strCMPIStatus(rc));
                break;

            case CMPI_instance:
                retDataInst = CMGetProperty(data.value.inst,
                    "Property1", &rc);
                dataInst = CMGetProperty(value.inst, "Property1", &rc);
                if (retDataInst.value.uint32 != dataInst.value.uint32)
                {
                    flag = 0;
                }
                rc = CMRelease(value.inst);
                PROV_LOG("++++ Status of CMRelease(value.inst) : (%s)",
                strCMPIStatus(rc));
                break;
        }
        if (data.type == types_arr[i].element_type && flag)
        {
            PROV_LOG("++++  CMGetArg : Name - %s is of type"
                " %s ", types_arr[i].args_name, types_arr[i].typeName);
        }
        PROV_LOG("++++ Status of CMGetArg : (%s)", strCMPIStatus (rc));

        rc = CMRelease(args_ptr);
        PROV_LOG("++++ Status of CMRelease(args_ptr) : (%s)",
            strCMPIStatus(rc));
    }
    return flag;
}//end _testSimpleTypes

//Testing for error cases to cover else portions
static int _testErrorPaths()
{
    CMPIArgs* args_ptr = NULL;
    CMPIStatus rc = { CMPI_RC_OK, NULL };
    CMPIValue value;
    char* str = NULL;
    value.inst = NULL;
    args_ptr = CMNewArgs(_broker, &rc);
    PROV_LOG("++++ Status of CMNewArgs : (%s)",
        strCMPIStatus (rc));

    PROV_LOG("++++  Testing for CMPI_instance type");
    rc = CMAddArg (args_ptr,
        "EmptyInstance",
        (CMPIValue *) &value,
        CMPI_instance);
    PROV_LOG("++++ Status of CMAddArg with name EmptyInstance : (%s)",
        strCMPIStatus (rc));

    value.ref = NULL;
    PROV_LOG("++++  Testing for CMPI_ref type");
    rc = CMAddArg (args_ptr,
        "EmptyRef",
        (CMPIValue *) &value,
        CMPI_ref);
    PROV_LOG("++++ Status of CMAddArg with name EmptyRef : (%s)",
        strCMPIStatus (rc));
    value.dateTime = NULL;
    PROV_LOG("++++  Testing for CMPI_datetime type");
    rc = CMAddArg (args_ptr,
        "EmptyDatetime",
        (CMPIValue *) &value,
        CMPI_dateTime);
    PROV_LOG("++++ Status of CMAddArg with name EmptyDatetime : (%s)",
        strCMPIStatus (rc));
    PROV_LOG("++++  Testing for CMPI_chars type");
    rc = CMAddArg (args_ptr,
        "EmptyChars",
        (CMPIValue *) str,
        CMPI_chars);
    PROV_LOG("++++ Status of CMAddArg with name EmptyChars : (%s)",
        strCMPIStatus (rc));

    PROV_LOG("++++  Testing for CMPI_charsptrA type");
    rc = CMAddArg (args_ptr,
        "EmptyCharsPtrA",
        NULL,
        CMPI_charsptrA);
    PROV_LOG("++++ Status of CMAddArg with name EmptyCharsPtrA : (%s)",
        strCMPIStatus (rc));

    value.chars = NULL;
    PROV_LOG("++++  Testing for CMPI_charsptr type");
    rc = CMAddArg (args_ptr,
        "EmptyCharsPtr",
        &value,
        CMPI_charsptr);
    PROV_LOG("++++ Status of CMAddArg with name EmptyCharsPtr : (%s)",
        strCMPIStatus (rc));

    value.args = NULL;
    PROV_LOG("++++  Testing for CMPI_args type");
    rc = CMAddArg (args_ptr,
        "EmptyArgs",
        (CMPIValue *) &value,
        CMPI_args);
    PROV_LOG("++++ Status of CMAddArg with name EmptyArgs : (%s)",
        strCMPIStatus (rc));

    rc = CMRelease(args_ptr);
    PROV_LOG("++++ Status of CMRelease(args_ptr) : (%s)", strCMPIStatus(rc));
    return 1;
}//End of _testErrorPaths


// Test for CMPIEnumeration and its error paths.
static int _testCMPIEnumeration (const CMPIContext* ctx)
{
    CMPIStatus rc = { CMPI_RC_OK, NULL };
    CMPIEnumeration *enum_ptr = NULL;
    CMPIData data;
    unsigned int initCount = 0;
    CMPIObjectPath* objPath = NULL;
    CMPIArray* arr_ptr = NULL;
    CMPICount returnedArraySize;
    void *eptr;
    PROV_LOG("++++ _testCMPIEnumeration");

    objPath = make_ObjectPath(_broker, _Namespace, _PersonClass);
    enum_ptr = CBEnumInstances(_broker, ctx, objPath, NULL, &rc);
    PROV_LOG ("++++  CBEnumInstances : (%s)", strCMPIStatus (rc));
    if (enum_ptr == NULL)
    {
        PROV_LOG("---- CBEnumInstances failed  ----");
        return 1;
    }

    arr_ptr = CMToArray(enum_ptr, &rc);
    PROV_LOG ("++++  CMToArray : (%s)", strCMPIStatus (rc));
    if (arr_ptr == NULL)
    {
        PROV_LOG("---- CMToArray failed  ----");
        return 1;
    }

    returnedArraySize = CMGetArrayCount(arr_ptr, &rc);
    PROV_LOG ("++++ ReturnedArraySize :%d", returnedArraySize);
    PROV_LOG("++++ Calling CMHasNext");
    while (CMHasNext(enum_ptr, &rc))
    {
        PROV_LOG("++++ Calling CMGetNext");
        data = CMGetNext(enum_ptr, &rc);
        PROV_LOG ("++++  CMGetNext : (%s)", strCMPIStatus (rc));
        if (data.type != CMPI_instance)
        {
            return 1;
        }
        initCount++;
    }
    PROV_LOG("++++ Enum count %d ", initCount);

    //Error Paths

    eptr = enum_ptr->hdl;
    enum_ptr->hdl = NULL;

    PROV_LOG ("++++  Testing Error Path for CMToArray():  " );
    CMToArray(enum_ptr, &rc);
    PROV_LOG ("++++ CMToArray Error Path: (%s)", strCMPIStatus (rc));
    if (rc.rc != CMPI_RC_ERR_INVALID_HANDLE)
    {
        return 1;
    }

    PROV_LOG ("++++  Testing Error Path : Invalid Handle test for CMGetNext " );
    CMGetNext(enum_ptr, &rc);
    PROV_LOG ("CMGetNext Error Path-Invalid Handle: (%s)", strCMPIStatus (rc));
    if (rc.rc != CMPI_RC_ERR_INVALID_HANDLE)
    {
        return 1;
    }

    PROV_LOG ("++++  Testing Error Path : Invalid Handle test for CMHasNext " );
    CMHasNext(enum_ptr, &rc);
    PROV_LOG ("++++  CMHasNext Error Path : (%s)", strCMPIStatus (rc));
    if (rc.rc != CMPI_RC_ERR_INVALID_HANDLE)
    {
        return 1;
    }
    enum_ptr->hdl = eptr;
    rc = CMRelease (enum_ptr);
    if (rc.rc != CMPI_RC_OK)
    {
        return 1;
    }

    return 0;
}

// Test for CMPIArray and its error paths.
static int _testCMPIArray ()
{
  CMPIStatus rc = { CMPI_RC_OK, NULL };
    CMPIArray *arr_ptr = NULL;
    CMPIArray *new_arr_ptr = NULL;
    CMPIData data[3];
    CMPIData clonedData[3];
    CMPIValue value;
    CMPIType initArrayType = CMPI_uint32;
    CMPIType initErrArrayType = CMPI_REAL;
    CMPIType returnedArrayType;
    CMPICount initArraySize = 3;
    CMPICount returnedArraySize;
    CMPIUint32 i;
    CMPIBoolean cloneSuccessful = 0;
    CMPIBoolean getDataSuccessful;
    void *aptr;

    PROV_LOG ("++++ _testCMPIArray" );

    PROV_LOG ("++++ creating CMPIArray : CMNewArray");
    arr_ptr = CMNewArray(_broker, initArraySize, initArrayType, &rc);
    if (arr_ptr == NULL)
    {
        PROV_LOG("---- CMNewArray failed ----");
        return 1;
    }
    PROV_LOG ("++++  CMNewArray : (%s)", strCMPIStatus (rc));
    returnedArraySize = CMGetArrayCount(arr_ptr, &rc);
    PROV_LOG ("++++  CMGetArrayCount : (%s)", strCMPIStatus (rc));
    PROV_LOG ("++++ returnedArraySize :%d", returnedArraySize);
    returnedArrayType = CMGetArrayType(arr_ptr, &rc);
    PROV_LOG ("++++  CMGetArrayType : (%s)", strCMPIStatus (rc));
    PROV_LOG ("++++ returnedArrayType :%d", returnedArrayType);

    value.uint32 = 10;
    rc = CMSetArrayElementAt(arr_ptr, 0, &value, initArrayType);
    PROV_LOG ("++++  CMSetArrayElementAt : (%s)", strCMPIStatus (rc));
    value.uint32 = 20;
    rc = CMSetArrayElementAt(arr_ptr, 1, &value, initArrayType);
    PROV_LOG ("++++  CMSetArrayElementAt : (%s)", strCMPIStatus (rc));
    value.uint32 = 30;
    rc = CMSetArrayElementAt(arr_ptr, 2, &value, initArrayType);
    PROV_LOG ("++++  CMSetArrayElementAt : (%s)", strCMPIStatus (rc));

    i = 0;
    while (i < 3)
    {
        data[i] = CMGetArrayElementAt(arr_ptr, i, &rc);
        PROV_LOG("++++ Value of data[%d]:%d", i, data[i].value.uint32);
        i++;
    }

    i = 0;
    getDataSuccessful = 1;
    while (i < 3)
    {
        if (data[i].value.uint32 != (i + 1) * 10)
        {
            getDataSuccessful = 0;
            break;
        }
        i++;
    }
    PROV_LOG("++++ Getting Data Successful : %d ", getDataSuccessful);

    // Testing CMPIArrayFT.clone()
    PROV_LOG("++++ cloning Array");
    new_arr_ptr = arr_ptr->ft->clone(arr_ptr, &rc);
    PROV_LOG ("++++  ArrayClone : (%s)", strCMPIStatus (rc));

    i = 0;
    while (i < 3)
    {
        clonedData[i] = CMGetArrayElementAt(new_arr_ptr, i, &rc);
        i++;
    }

    //Compare the Contents of data and clonedData
    cloneSuccessful = 1;
    for (i = 0; i < initArraySize; i++)
    {
        if (data[i].value.uint32 != clonedData[i].value.uint32)
        {
            cloneSuccessful = 0;
            break;
        }
    }
    PROV_LOG("++++ clone Successful: %d", cloneSuccessful);
    rc = new_arr_ptr->ft->release(new_arr_ptr);
    PROV_LOG ("++++ Cloned Array Release : (%s)", strCMPIStatus (rc));

    //Error Paths

    aptr = arr_ptr->hdl;
    arr_ptr->hdl = NULL;

    returnedArraySize = CMGetArrayCount(arr_ptr, &rc);
    PROV_LOG ("++++  CMGetArrayCount Error Path : (%s)", strCMPIStatus (rc));
    if (rc.rc != CMPI_RC_ERR_INVALID_HANDLE)
    {
        return 1;
    }

    returnedArrayType = CMGetArrayType(arr_ptr, &rc);
    PROV_LOG ("++++  CMGetArrayType Error Path: (%s)", strCMPIStatus (rc));
    if (rc.rc != CMPI_RC_ERR_INVALID_HANDLE)
    {
        return 1;
    }

    rc = CMSetArrayElementAt(arr_ptr, 2, &value, initArrayType);
    PROV_LOG ("++++  CMSetArrayElementAt Error Path : (%s)",
        strCMPIStatus (rc));
    if (rc.rc != CMPI_RC_ERR_INVALID_HANDLE)
    {
        return 1;
    }

    CMGetArrayElementAt(arr_ptr, 5, &rc);
    PROV_LOG ("++++  CMGetArrayElementAt Error Path : (%s)",
        strCMPIStatus (rc));
    if (rc.rc != CMPI_RC_ERR_INVALID_HANDLE)
    {
        return 1;
    }
    arr_ptr->ft->clone(arr_ptr, &rc);
    PROV_LOG ("++++  ArrayClone Error Path : (%s)", strCMPIStatus (rc));
    if (rc.rc != CMPI_RC_ERR_INVALID_HANDLE)
    {
        return 1;
    }

    rc = arr_ptr->ft->release(arr_ptr);
    PROV_LOG ("++++  Array Release Error Path : (%s)", strCMPIStatus (rc));
    if (rc.rc != CMPI_RC_ERR_INVALID_HANDLE)
    {
        return 1;
    }

    arr_ptr->hdl = aptr;

    CMGetArrayElementAt(arr_ptr, 5, &rc);
    PROV_LOG ("++++  CMGetArrayElementAt Error Path: (%s)", strCMPIStatus (rc));
    if (rc.rc != CMPI_RC_ERR_NO_SUCH_PROPERTY)
    {
        return 1;
    }

    rc = CMSetArrayElementAt(arr_ptr, 2, &value, initErrArrayType);
    PROV_LOG ("++++  CMSetArrayElementAt Error Path : (%s)",
        strCMPIStatus (rc));
    if (rc.rc != CMPI_RC_ERR_TYPE_MISMATCH)
    {
        return 1;
    }

    rc = arr_ptr->ft->release(arr_ptr);
    PROV_LOG ("++++  Array Release : (%s)", strCMPIStatus (rc));

    return 0;
}

// Test for CMPIContext and its error paths.
static int _testCMPIcontext (const CMPIContext* ctx)
{
    CMPIStatus rc = { CMPI_RC_OK, NULL };
    CMPIValue value;
    CMPIData data;
    CMPIUint32 count = 0;
    CMPIUint32  count_for_new_context = 0;
    PROV_LOG ("++++ _testCMPIContext");

    count = CMGetContextEntryCount(ctx, &rc);
    value.uint32 = 40;
    PROV_LOG ("++++ CMAddContextEntry");
    rc = CMAddContextEntry(ctx, "name1", &value, CMPI_uint32);
    PROV_LOG ("++++ CMAddContextEntry : (%s)", strCMPIStatus (rc));

    value.real32 = (CMPIReal32)40.123;
    PROV_LOG ("++++ CMAddContextEntry");
    rc = CMAddContextEntry(ctx, "name2", &value, CMPI_real32);
    PROV_LOG ("++++ CMAddContextEntry : (%s)", strCMPIStatus (rc));

    rc = CMAddContextEntry (ctx, "SnmpTrapOidContainer",
                         "1.3.6.1.4.1.900.2.3.9002.9600", CMPI_chars);
    PROV_LOG ("++++  CMAddContextEntry 2: (%s)", strCMPIStatus (rc));

    data = CMGetContextEntry(ctx, "name1", &rc);
    PROV_LOG ("++++ CMGetContextEntry : (%s)", strCMPIStatus (rc));
    PROV_LOG ("++++ Data added : %d ", data.value.uint32 );
    PROV_LOG ("++++ Getting data sucessful : %s", strCMPIStatus (rc));

    data = CMGetContextEntry(ctx, "name2", &rc);
    PROV_LOG ("++++ CMGetContextEntry : (%s)", strCMPIStatus (rc));
    PROV_LOG ("++++ Data added : %.3f ", data.value.real32 );
    PROV_LOG ("++++ Getting data sucessful : %s", strCMPIStatus (rc));

    count_for_new_context = CMGetContextEntryCount(ctx, &rc);
    PROV_LOG ("++++  CMGetContextEntryCount : (%s)", strCMPIStatus (rc));
    PROV_LOG ("++++ Total number of data added : %d",
              count_for_new_context - count);

    //Error Paths
    CMGetContextEntry(ctx, "noEntry", &rc);
    PROV_LOG ("++++ CMGetContextEntry Error Path : (%s)", strCMPIStatus (rc));
    if (rc.rc != CMPI_RC_ERR_NO_SUCH_PROPERTY)
    {
        return 1;
    }

    return 0;
}

// Test for CMPIDateTime and its error paths.
static int _testCMPIDateTime ()
{
    CMPIStatus rc = { CMPI_RC_OK, NULL };

    CMPIBoolean isInterval = 0;
    CMPIBoolean interval = 0;
    CMPIBoolean cloneSuccessful = 0;
    CMPIBoolean binaryDateTimeEqual = 0;

    CMPIDateTime *dateTime = NULL;
    CMPIDateTime *new_dateTime = NULL;
    CMPIDateTime *clonedDateTime = NULL;
    CMPIDateTime *dateTimeFromBinary = NULL;

    CMPIUint64 dateTimeInBinary = UINT64_LITERAL(1170892800000000); //07/02/2007
    CMPIUint64 returnedDateTimeInBinary = 0;

    CMPIString* stringDate = NULL;
    CMPIString* clonedStringDate = NULL;

    const char *normalString = NULL;
    const char *clonedString = NULL;
    void *dtptr;

    PROV_LOG("++++ _testCMPIDateTime" );

    PROV_LOG("++++ creating CMPIDateTime : CMNewDateTime");

    dateTime = CMNewDateTime(_broker, &rc);
    PROV_LOG ("++++  CMNewDateTime : (%s)", strCMPIStatus (rc));
    if (dateTime == NULL)
    {
        PROV_LOG("---- CMNewDateTime failed ----");
        return 1;
    }

    PROV_LOG("++++ creating CMPIDateTime from binary: CMNewDateTimeFromBinary");

    dateTimeFromBinary = CMNewDateTimeFromBinary(
        _broker, dateTimeInBinary, interval, &rc);
    PROV_LOG ("++++  CMNewDateTimeFromBinary : (%s)", strCMPIStatus (rc));

    returnedDateTimeInBinary = CMGetBinaryFormat(dateTimeFromBinary, &rc);
    PROV_LOG ("++++  CMGetBinaryFormat : (%s)", strCMPIStatus (rc));
    if (dateTimeInBinary == returnedDateTimeInBinary)
    {
        binaryDateTimeEqual = 1;
    }
    PROV_LOG("++++ Created time sucessfully : %d", binaryDateTimeEqual);
    isInterval = CMIsInterval(dateTime, &rc);
    PROV_LOG ("++++  CMIsInterval : (%s)", strCMPIStatus (rc));
    PROV_LOG("++++ is interval : %d", isInterval);

    //Create new dateTime with interval = true
    PROV_LOG("++++ Setting interval as true");
    interval = 1;
    new_dateTime = CMNewDateTimeFromBinary(
        _broker, dateTimeInBinary, interval,&rc);
    PROV_LOG ("++++  CMNewDateTimeFromBinary : (%s)", strCMPIStatus (rc));

    isInterval = CMIsInterval(new_dateTime, &rc);
    PROV_LOG ("++++  CMIsInterval-Binary : (%s)", strCMPIStatus (rc));
    PROV_LOG("++++ is interval : %d", isInterval);

    PROV_LOG("++++ cloning DateTime");
    clonedDateTime = dateTime->ft->clone(dateTime, &rc);
    PROV_LOG ("++++  Cloning DateTime : (%s)", strCMPIStatus (rc));

    stringDate = CMGetStringFormat(dateTime, &rc);
    clonedStringDate = CMGetStringFormat(clonedDateTime, &rc);
    rc = clonedDateTime->ft->release(clonedDateTime);
    PROV_LOG ("++++  Cloned DateTime Release : (%s)", strCMPIStatus (rc));
    normalString = CMGetCharsPtr(stringDate, &rc);
    clonedString = CMGetCharsPtr(clonedStringDate, &rc);

    if (strcmp(normalString,clonedString) == 0)
    {
        cloneSuccessful = 1;
    }
    PROV_LOG("++++ clone Successful : %d ",cloneSuccessful);

    //Error Paths

    dtptr = dateTime->hdl;
    dateTime->hdl = NULL;

    CMGetBinaryFormat(dateTime, &rc);
    PROV_LOG ("++++  CMGetBinaryFormat ErrorPath: (%s)", strCMPIStatus (rc));
    if (rc.rc != CMPI_RC_ERR_INVALID_HANDLE)
    {
        return 1;
    }

    dateTime->ft->clone(dateTime, &rc);
    PROV_LOG ("++++  DateTimeClone Error Path: (%s)", strCMPIStatus (rc));
    if (rc.rc != CMPI_RC_ERR_INVALID_HANDLE)
    {
        return 1;
    }

    CMGetStringFormat(dateTime, &rc);
    PROV_LOG ("DateTime getStringFormat Error Path: (%s)", strCMPIStatus (rc));
    if (rc.rc != CMPI_RC_ERR_INVALID_HANDLE)
    {
        return 1;
                  }

    rc = dateTime->ft->release(dateTime);
    PROV_LOG ("++++  DateTimeRelease Error Path: (%s)", strCMPIStatus (rc));
    if (rc.rc != CMPI_RC_ERR_INVALID_HANDLE)
    {
        return 1;
    }

    dateTime->hdl = dtptr;
    rc = dateTime->ft->release(dateTime);
    PROV_LOG ("++++  DateTime Release : (%s)", strCMPIStatus (rc));

    return 0;
}

// Test for CMPIInstance and its error paths.
static int _testCMPIInstance ()
{
    CMPIStatus rc = { CMPI_RC_OK, NULL };

    CMPIInstance* instance = NULL;
    CMPIInstance* clonedInstance = NULL;
    CMPIObjectPath* objPath = NULL;
    CMPIObjectPath* newObjPath = NULL;
    CMPIObjectPath* returnedObjPath = NULL;

    CMPIData returnedData1;
    CMPIData returnedData2;
    CMPIData clonedData1;

    CMPIString* returnedName = NULL;
    unsigned int count = 0;
    const char* name1 = "firstPropertyName";
    CMPIValue value1;
    const char* name2 = "secondPropertyName";
    CMPIValue value2;
    CMPIType type = CMPI_uint64;
    CMPIBoolean dataEqual = 0;
    CMPIBoolean objectPathEqual = 0;
    CMPIBoolean cloneSuccessful = 0;
    CMPIString* beforeObjPath = NULL;
    CMPIString* afterObjPath = NULL;
    const char* beforeString = NULL;
    const char* afterString = NULL;
    void *instptr;

    PROV_LOG("++++ _testCMPIInstance" );

    objPath = make_ObjectPath(_broker, _Namespace, _ClassName);
    instance = make_Instance(objPath);

    // Check for default null values
    returnedData1 = CMGetProperty(instance, name1, &rc);
    PROV_LOG("++++  CMGetProperty, Default Nullvalue. rc: (%s)",
        strCMPIStatus (rc));
    PROV_LOG("++++  CMGetProperty value state: (%s)",
        (returnedData1.state == CMPI_nullValue) ?
            "CMPI_nullValue" : "Unexpected" );

    // Set Null value tests
    rc = CMSetProperty(instance, name1, 0, type);
    returnedData1 = CMGetProperty(instance, name1, &rc);
    PROV_LOG("++++  CMGetProperty, Set Nullvalue. rc: (%s)",
        strCMPIStatus (rc));
    PROV_LOG("++++  CMGetProperty value state: (%s)",
        returnedData1.state == CMPI_nullValue ?
            "CMPI_nullValue" : "Unexpected" );

    // Property not found in schema tests
    returnedData1 = CMGetProperty(instance, "NotAProp", &rc);
    PROV_LOG("++++  CMGetProperty, not a valid property. rc: (%s)",
        strCMPIStatus (rc));
    PROV_LOG("++++  CMGetProperty, not a valid property. Value state: (%s)",
        returnedData1.state == (CMPI_nullValue|CMPI_notFound) ?
            "CMPI_nullValue|CMPI_notFound" : "Unexpected" );

    value1.uint32 = 10;
    rc = CMSetProperty(instance, name1, &value1, type);
    PROV_LOG("++++  CMSetProperty-1 : (%s)", strCMPIStatus (rc));
    value2.uint32 = 20;
    rc = CMSetProperty(instance, name2, &value2, type);
    PROV_LOG("++++  CMSetProperty-2 : (%s)", strCMPIStatus (rc));
    count = CMGetPropertyCount(instance, &rc);
    PROV_LOG("++++  CMGetPropertyCount : (%s)", strCMPIStatus (rc));
    PROV_LOG("++++ Number of values added : %d", count);

    returnedData1 = CMGetProperty(instance, name1, &rc);
    PROV_LOG("++++  CMGetProperty : (%s)", strCMPIStatus (rc));
    PROV_LOG("++++ First value  : %d", returnedData1.value.uint32);
    if (returnedData1.value.uint32 == 10)
    {
        dataEqual = 1 ;
    }
    PROV_LOG("++++ CMSetProperty successful : %d ", dataEqual);

    returnedData2 = CMGetPropertyAt(instance, 2, &returnedName, &rc);
    PROV_LOG("++++  CMGetPropertyAt : (%s)", strCMPIStatus (rc));
    PROV_LOG("++++ Second value : %d", returnedData2.value.uint32);
    if (returnedData2.value.uint32 == 20)
    {
        dataEqual = 1 ;
    }
    PROV_LOG("++++ CMGetProperty successful : %d ", dataEqual);

    PROV_LOG("++++ Setting new ObjectPath");

    newObjPath = make_ObjectPath(_broker, _Namespace, _ClassName);
    returnedObjPath = CMGetObjectPath(instance, &rc);
    beforeObjPath = CMObjectPathToString(returnedObjPath, &rc);
    beforeString = CMGetCharsPtr(beforeObjPath, &rc);
    PROV_LOG("++++ Before ObjectPath was : %s", beforeString);

    rc = CMSetNameSpace(newObjPath, "newNamespace");
    PROV_LOG("++++  CMSetNameSpace : (%s)", strCMPIStatus (rc));

    rc = CMSetObjectPath(instance, newObjPath);
    PROV_LOG("++++  CMSetObjectPath : (%s)", strCMPIStatus (rc));

    returnedObjPath = CMGetObjectPath(instance, &rc);
    afterObjPath = CMObjectPathToString(returnedObjPath, &rc);
    afterString = CMGetCharsPtr(afterObjPath,&rc);
    PROV_LOG("++++ New ObjectPath is : %s", afterString);

    //Get namespace of this object path
    afterString = CMGetCharsPtr(CMGetNameSpace(returnedObjPath, &rc), &rc);
    if (strcmp("newNamespace",afterString) == 0)
    {
        objectPathEqual = 1;
    }
    PROV_LOG("++++ New ObjectPath set successfully : %d" , objectPathEqual);

    PROV_LOG("++++ cloning CMPIInstance");
    clonedInstance = instance->ft->clone(instance, &rc);
    PROV_LOG("++++  Cloning CMPIInstance : (%s)", strCMPIStatus (rc));

    clonedData1 = CMGetProperty(clonedInstance, name1, &rc);
    PROV_LOG("++++ cloned data value %d : ", clonedData1.value.uint32);
    rc = clonedInstance->ft->release(clonedInstance);
    PROV_LOG("++++ Cloned Instance Release : (%s)", strCMPIStatus (rc));
    if (returnedData1.value.uint32 == clonedData1.value.uint32)
    {
        cloneSuccessful = 1;
    }
    else
    {
        cloneSuccessful = 0;
    }
    PROV_LOG("++++ clone Successful : %d ",cloneSuccessful);

    //Error Paths

    instptr = instance->hdl;
    instance->hdl = NULL;

    rc = CMSetProperty(instance, name2, &value2, type);
    PROV_LOG("++++  CMSetProperty Error Path : (%s)", strCMPIStatus (rc));
    if (rc.rc != CMPI_RC_ERR_INVALID_HANDLE)
    {
        return 1;
    }

    CMGetPropertyCount(instance, &rc);
    PROV_LOG("++++  CMGetPropertyCount Error Path: (%s)", strCMPIStatus (rc));
    if (rc.rc != CMPI_RC_ERR_INVALID_HANDLE)
    {
        return 1;
    }

    CMGetProperty(instance, name1, &rc);
    PROV_LOG("++++  CMGetProperty : (%s)", strCMPIStatus (rc));
    if (rc.rc != CMPI_RC_ERR_INVALID_HANDLE)
    {
        return 1;
    }

    CMGetPropertyAt(instance, 1, &returnedName, &rc);
    PROV_LOG("++++  CMGetPropertyAt Error Path : (%s)", strCMPIStatus (rc));
    if (rc.rc != CMPI_RC_ERR_INVALID_HANDLE)
    {
        return 1;
    }

    CMGetObjectPath(instance, &rc);
    PROV_LOG("++++  CMPIInstance setOP Error Path : (%s)", strCMPIStatus (rc));
    if (rc.rc != CMPI_RC_ERR_INVALID_HANDLE)
    {
        return 1;
    }

    CMGetObjectPath(instance, &rc);
    PROV_LOG("++++  CMPIInstance getOP Error Path : (%s)", strCMPIStatus (rc));
    if (rc.rc != CMPI_RC_ERR_INVALID_HANDLE)
    {
        return 1;
    }

    instance->ft->clone(instance, &rc);
    PROV_LOG("++++  InstanceClone Error Path: (%s)", strCMPIStatus (rc));
    if (rc.rc != CMPI_RC_ERR_INVALID_HANDLE)
    {
        return 1;
    }

    rc = instance->ft->release(instance);
    PROV_LOG("++++   Instance Release Error Path : (%s)", strCMPIStatus (rc));
    if (rc.rc != CMPI_RC_ERR_INVALID_HANDLE)
    {
        return 1;
    }

    instance->hdl = (CMPIInstance *)instptr;
    CMGetProperty(instance, "noProperty", &rc);
    PROV_LOG("++++  CMGetProperty : (%s)", strCMPIStatus (rc));
    if (rc.rc != CMPI_RC_ERR_NO_SUCH_PROPERTY)
    {
        return 1;
    }

    CMGetPropertyAt(instance, 100, &returnedName, &rc);
    PROV_LOG("++++  CMGetPropertyAt Error Path : (%s)", strCMPIStatus (rc));
    if (rc.rc != CMPI_RC_ERR_NO_SUCH_PROPERTY)
    {
        return 1;
    }
    rc = instance->ft->release(instance);
    PROV_LOG("++++   Instance Release : (%s)", strCMPIStatus (rc));

    return 0;
}

// Test for CMPIObjectPath and its error paths.
static int _testCMPIObjectPath ()
{
    CMPIStatus rc = { CMPI_RC_OK, NULL };

    CMPIObjectPath* objPath = NULL;
    CMPIObjectPath* clonedObjPath = NULL;
    CMPIObjectPath* otherObjPath = NULL;
    CMPIObjectPath *fakeObjPath  = NULL;

    const char* hostName = "HOSTNAME";
    const char* nameSpace = "root/dummy";
    const char* className = "classname";

    CMPIString* returnedHostname = NULL;
    CMPIBoolean equalHostname = 0;
    CMPIString* returnedNamespace = NULL;
    CMPIBoolean equalNamespace = 0;
    CMPIString* returnedClassName;
    CMPIBoolean equalClassName = 0;
    CMPIString* returnedObjectPath;
    CMPIBoolean cloneSuccessful = 0;
    CMPIBoolean getKeySuccessful = 0;
    CMPIBoolean getKeyCountSuccessful = 0;
    CMPIBoolean getKeyAtSuccessful = 0;
    CMPIBoolean getKeyAtErrorPathSuccessful = 0;
    const char* objectPath1 = NULL;
    const char* objectPath2 = NULL;
    CMPIData data;
    CMPIValue value;
    unsigned int keyCount = 0;
    void *opptr;

    PROV_LOG("++++ _testCMPIObjectPath" );

    objPath = make_ObjectPath(_broker, _Namespace, _ClassName);
    PROV_LOG("++++ setting Hostname");
    rc = CMSetHostname(objPath, hostName);
    PROV_LOG ("++++  CMSetHostname : (%s)", strCMPIStatus (rc));

    returnedHostname = CMGetHostname(objPath, &rc);
    PROV_LOG ("++++  CMGetHostname : (%s)", strCMPIStatus (rc));
    PROV_LOG("++++ Hostname : %s", CMGetCharsPtr(returnedHostname,&rc));
    if (strcmp(hostName,CMGetCharsPtr(returnedHostname,&rc)) == 0)
    {
        equalHostname = 1;
    }
    PROV_LOG("++++ Hostname set successfuly : %d", equalHostname);

    PROV_LOG("++++ setting Namespace");
    rc = CMSetNameSpace(objPath, nameSpace);
    PROV_LOG("++++  CMSetNameSpace : (%s)", strCMPIStatus (rc));
    returnedNamespace = CMGetNameSpace(objPath, &rc);
    PROV_LOG ("++++  CMGetNameSpace : (%s)", strCMPIStatus (rc));
    PROV_LOG("++++ Namespace : %s", CMGetCharsPtr(returnedNamespace, &rc));
    if (strcmp(nameSpace, CMGetCharsPtr(returnedNamespace, &rc)) == 0)
    {
        equalNamespace = 1;
    }
    PROV_LOG("++++ Namespace set successfuly : %d", equalNamespace);

    PROV_LOG("++++ setting classname");
    rc = CMSetClassName(objPath, className);
    PROV_LOG ("++++  CMSetClassName : (%s)", strCMPIStatus (rc));

    returnedClassName = CMGetClassName(objPath, &rc);
    PROV_LOG ("++++  CMGetClassName : (%s)", strCMPIStatus (rc));
    PROV_LOG("++++ Classname : %s", CMGetCharsPtr(returnedClassName, &rc));
    if (strcmp(className,CMGetCharsPtr(returnedClassName, &rc)) == 0)
    {
        equalClassName = 1;
    }
    PROV_LOG("++++ Classname set successfuly : %d", equalClassName);

    PROV_LOG("++++ calling CMSetNameSpaceFromObjectPath");
    otherObjPath = make_ObjectPath(_broker, _Namespace, _ClassName);
    returnedNamespace = CMGetNameSpace(otherObjPath, &rc);

    PROV_LOG("++++ Before:Namespace:%s",CMGetCharsPtr(returnedNamespace, &rc));
    rc = CMSetNameSpaceFromObjectPath(otherObjPath, objPath);
    PROV_LOG ("++++  CMSetNameSpaceFromObjectPath : (%s)", strCMPIStatus (rc));

    returnedNamespace = CMGetNameSpace(otherObjPath, &rc);
    PROV_LOG("++++ After:Namespace: %s",CMGetCharsPtr(returnedNamespace, &rc));
    if (strcmp(nameSpace,CMGetCharsPtr(returnedNamespace, &rc)) == 0)
    {
        equalNamespace = 1;
    }
    PROV_LOG("++++ Namespace set successfuly : %d", equalNamespace);

    PROV_LOG("++++ calling CMSetHostAndNameSpaceFromObjectPath");
    returnedHostname = CMGetHostname(otherObjPath, &rc);
    PROV_LOG("++++ Before:Hostname: %s ",CMGetCharsPtr(returnedHostname, &rc));
    rc = CMSetHostAndNameSpaceFromObjectPath(otherObjPath,objPath);
    PROV_LOG ("++++  CMSetHostAndNameSpaceFromObjectPath : (%s)",
        strCMPIStatus (rc));

    returnedHostname = CMGetHostname(otherObjPath, &rc);
    PROV_LOG("++++ After:Hostname: %s ",CMGetCharsPtr(returnedHostname, &rc));
    if (strcmp(hostName,CMGetCharsPtr(returnedHostname,&rc)) == 0)
    {
        equalHostname = 1;
    }
    PROV_LOG("++++ Hostname set successfuly : %d", equalHostname);

    returnedObjectPath = CMObjectPathToString(objPath, &rc);

    PROV_LOG("++++ cloning objectpath");
    objectPath1 = CMGetCharsPtr(returnedObjectPath, &rc);

    clonedObjPath = objPath->ft->clone(objPath, &rc);
    PROV_LOG ("++++  Cloning Objectpath : (%s)", strCMPIStatus (rc));
    returnedObjectPath = CMObjectPathToString(clonedObjPath, &rc);
    rc = clonedObjPath->ft->release(clonedObjPath);
    PROV_LOG ("++++  Cloned ObjectPath Release : (%s)", strCMPIStatus (rc));
    objectPath2 = CMGetCharsPtr(returnedObjectPath, &rc);

    if (strcmp(objectPath1,objectPath2) == 0)
    {
        cloneSuccessful = 1;
    }
    else
    {
        cloneSuccessful = 0;
    }
    PROV_LOG("++++ clone Successful : %d ",cloneSuccessful);

    // Create a new ObjectPath, in a different namespace.

    fakeObjPath = CMNewObjectPath (_broker, "root/cimv2",
        "TestCMPI_Instance", &rc);
    PROV_LOG ("++++  CMNewObjectPath : (%s)", strCMPIStatus (rc));
    rc = CMAddKey (fakeObjPath, "ElementName",
        (CMPIValue *) "Fake", CMPI_chars);
    PROV_LOG ("++++  CMAddKey : (%s)", strCMPIStatus (rc));
    rc = CMAddKey (otherObjPath, "ElementName1",
        (CMPIValue *) "otherObjPath", CMPI_chars);
    PROV_LOG ("++++  CMAddKey : (%s)", strCMPIStatus (rc));


    PROV_LOG("++++ calling CMGetKey");
    data = CMGetKey(fakeObjPath, "ElementName", &rc);
    PROV_LOG ("++++  CMGetKey : (%s)", strCMPIStatus (rc));

    if (strcmp(CMGetCharsPtr(data.value.string, &rc),"Fake") == 0)
    {
        getKeySuccessful = 1;
    }
    PROV_LOG("++++ CMGetKey Successful : %d ",getKeySuccessful);

    PROV_LOG("++++ calling CMGetKeyCount");
    keyCount = CMGetKeyCount(fakeObjPath, &rc);
    PROV_LOG ("++++  CMGetKeyCount : (%d)", keyCount);
    if (keyCount == 1)
    {
        getKeyCountSuccessful = 1;
    }
    PROV_LOG("++++ CMGetKeyCount Successful : %d ",getKeyCountSuccessful);

    PROV_LOG("++++ calling CMGetKeyAt");
    data = CMGetKeyAt(fakeObjPath, 0, NULL, &rc);
    PROV_LOG ("++++  CMGetKeyCount : (%s)", strCMPIStatus (rc));
    if (strcmp(strCMPIStatus (rc), "CMPI_RC_OK") == 0)
    {
        getKeyAtSuccessful = 1;
    }
    PROV_LOG("++++ CMGetKeyAt Successful : %d ",getKeyAtSuccessful);

    PROV_LOG("++++ calling CMGetKeyAt");
    data = CMGetKeyAt(fakeObjPath, 1, NULL, &rc);
    PROV_LOG ("++++  CMGetKeyCount : (%s)", strCMPIStatus (rc));
    if (strcmp(strCMPIStatus (rc), "CMPI_RC_ERR_NOT_FOUND") == 0)
    {
        getKeyAtErrorPathSuccessful = 1;
    }
    PROV_LOG("++++ CMGetKeyAtErrorPath Successful : %d ",
        getKeyAtErrorPathSuccessful);

    value.uint16 = 67;
    rc = CMAddKey (fakeObjPath, "Numeric_key_unsigned",
        (CMPIValue *) &value, CMPI_uint16);
    PROV_LOG ("++++  Status of CMAddKey of type CMPI_uint16: (%s)",
        strCMPIStatus (rc));
    data = CMGetKey(fakeObjPath, "Numeric_key_unsigned", &rc);
    // Check for uint64, CMGetKey promotes all unsigned numeric types to
    // Uint64
    if (data.value.uint64 == (CMPIUint64)value.uint16)
    {
        PROV_LOG ("++++  Status of CMGetKey of type CMPI_uint16: (%s)",
        strCMPIStatus (rc));
    }

    value.sint16 = -67;
    rc = CMAddKey (fakeObjPath, "Numeric_key_signed",
        (CMPIValue *) &value, CMPI_sint16);
    PROV_LOG ("++++  Status of CMAddKey of type CMPI_sint16: (%s)",
        strCMPIStatus (rc));
    data = CMGetKey(fakeObjPath, "Numeric_key_signed", &rc);
    // Check for sint64, CMGetKey promotes all signed numeric types to
    // Sint64
    if (data.value.sint64 == (CMPISint64)value.sint16)
    {
        PROV_LOG ("++++  Status of CMGetKey of type CMPI_sint16: (%s)",
        strCMPIStatus (rc));
    }

    value.boolean = 1;
    rc = CMAddKey (fakeObjPath, "Boolean_key",
        (CMPIValue *) &value, CMPI_boolean);
    PROV_LOG ("++++  Status of CMAddKey of type CMPI_boolean: (%s)",
        strCMPIStatus (rc));
    data = CMGetKey(fakeObjPath, "Boolean_key", &rc);
    if(data.value.boolean == value.boolean)
    {
        PROV_LOG ("++++  Status of CMGetKey of type CMPI_boolean: (%s)",
        strCMPIStatus (rc));
    }

    //Error Paths

    opptr = objPath->hdl;
    objPath->hdl = NULL;

    rc = CMSetHostname(objPath, "host");
    PROV_LOG ("ObjectPath SetHostname Error Path: (%s)", strCMPIStatus (rc));
    if (rc.rc != CMPI_RC_ERR_INVALID_HANDLE)
    {
        return 1;
    }

    rc = CMSetNameSpace(objPath, nameSpace);
    PROV_LOG ("ObjectPath setNameSpace Error Path: (%s)", strCMPIStatus (rc));
    if (rc.rc != CMPI_RC_ERR_INVALID_HANDLE)
    {
        return 1;
    }

    rc = CMSetClassName(objPath, className);
    PROV_LOG ("ObjectPath SetClassName Error Path: (%s)", strCMPIStatus (rc));
    if (rc.rc != CMPI_RC_ERR_INVALID_HANDLE)
    {
        return 1;
    }

    CMGetClassName(objPath, &rc);
    PROV_LOG ("ObjectPath GetClassName Error Path: (%s)", strCMPIStatus (rc));
    if (rc.rc != CMPI_RC_ERR_INVALID_HANDLE)
    {
        return 1;
    }

    rc = CMSetNameSpaceFromObjectPath(otherObjPath, objPath);
    PROV_LOG ("ObjectPath SetNameSpaceFromObjectPath Error Path: (%s)",
    strCMPIStatus (rc));
    if (rc.rc != CMPI_RC_ERR_INVALID_HANDLE)
    {
        return 1;
    }

    CMGetNameSpace(objPath, &rc);
    PROV_LOG ("ObjectPath GetNameSpace Error Path: (%s)", strCMPIStatus (rc));
    if (rc.rc != CMPI_RC_ERR_INVALID_HANDLE)
    {
        return 1;
    }

    rc = CMSetHostAndNameSpaceFromObjectPath(otherObjPath, objPath);
    PROV_LOG ("ObjectPath SetHostAndNameSpaceFromObjectPath Error Path: (%s)",
        strCMPIStatus (rc));
    if (rc.rc != CMPI_RC_ERR_INVALID_HANDLE)
    {
        return 1;
    }

    CMGetHostname(objPath, &rc);
    PROV_LOG ("ObjectPath GetHostName Error Path: (%s)", strCMPIStatus (rc));
    if (rc.rc != CMPI_RC_ERR_INVALID_HANDLE)
    {
        return 1;
    }

    rc = CMAddKey (objPath, "ElementName",
        (CMPIValue *) "Fake", CMPI_chars);
    PROV_LOG ("++++  ObjectPathAddKey Error Path: (%s)", strCMPIStatus (rc));
    if (rc.rc != CMPI_RC_ERR_INVALID_HANDLE)
    {
        return 1;
    }

    CMGetKey (objPath, "ElementName", &rc);
    PROV_LOG ("++++  ObjectPathGetKey Error Path : (%s)", strCMPIStatus (rc));
    if (rc.rc != CMPI_RC_ERR_INVALID_HANDLE)
    {
        return 1;
    }

    CMGetKeyCount (objPath, &rc);
    PROV_LOG ("ObjectPath GetKeycount Error Path : (%s)", strCMPIStatus (rc));
    if (rc.rc != CMPI_RC_ERR_INVALID_HANDLE)
    {
        return 1;
    }

    CMGetKeyAt(objPath, 0, NULL, &rc);
    PROV_LOG ("++++  ObjectPath getKeyAt Error Path : (%s)",
        strCMPIStatus (rc));
    if (rc.rc != CMPI_RC_ERR_INVALID_HANDLE)
    {
        return 1;
    }

    objPath->ft->clone(objPath, &rc);
    PROV_LOG ("++++  ObjectPathClone Error Path: (%s)", strCMPIStatus (rc));
    if (rc.rc != CMPI_RC_ERR_INVALID_HANDLE)
    {
        return 1;
    }

    rc = objPath->ft->release(objPath);
    PROV_LOG ("++++   ObjectPath Release Error Path : (%s)",
        strCMPIStatus (rc));
    if (rc.rc != CMPI_RC_ERR_INVALID_HANDLE)
    {
        return 1;
    }
    objPath->hdl = (CMPIObjectPath *)opptr;

    CMGetKeyAt(objPath, 500, NULL, &rc);
    PROV_LOG ("++++  ObjectPathGetKeyAt Error Path : (%s)", strCMPIStatus (rc));
    if (rc.rc != CMPI_RC_ERR_NO_SUCH_PROPERTY)
    {
        return 1;
    }
    rc = objPath->ft->release(objPath);
    PROV_LOG ("++++  ObjectPath Release : (%s)", strCMPIStatus (rc));

    rc = fakeObjPath->ft->release(fakeObjPath);
    PROV_LOG ("++++  Fake ObjectPath Release : (%s)", strCMPIStatus (rc));

    return 0;
}

// Test for CMPIResult and its error paths.
static int _testCMPIResult (const CMPIResult *rslt)
{
    CMPIStatus rc = { CMPI_RC_OK, NULL };

    CMPIValue value;
    CMPIType type;
    const CMPIObjectPath* objPath = NULL;
    CMPIBoolean returnDataSuccessful = 0;

    PROV_LOG("++++ _testCMPIResult" );

    value.uint32 = 10;
    type = CMPI_uint32;

    rc = CMReturnData(rslt, &value, type);
    PROV_LOG("++++  CMReturnData : (%s)", strCMPIStatus (rc));
    if (rc.rc == CMPI_RC_OK)
    {
        returnDataSuccessful = 1;
    }
    PROV_LOG("++++ CMReturnData Successful : %d ", returnDataSuccessful);

    objPath = make_ObjectPath(_broker, _Namespace, _ClassName);
    rc = CMReturnObjectPath(rslt, objPath);
    PROV_LOG("++++  CMReturnObjectPath : (%s)", strCMPIStatus (rc));

    rc = CMReturnDone(rslt);
    PROV_LOG("++++  CMReturnDone : (%s)", strCMPIStatus (rc));

    //Error Paths
    rc = CMReturnData(rslt, NULL, type);
    PROV_LOG("++++  CMReturnData : (%s)", strCMPIStatus (rc));
    if (rc.rc != CMPI_RC_ERR_INVALID_PARAMETER)
    {
        return 1;
    }

    return 0;
}

// Test for CMPIString and its error paths.
static int _testCMPIString()
{
    CMPIStatus rc = { CMPI_RC_OK, NULL };

    CMPIString* string = NULL;
    CMPIString* clonedString = NULL;
    const char* actual_string = NULL;
    const char* cloned_string = NULL;
    const char *data = "dataString";
    CMPIBoolean cloneSuccessful = 0;
    void *string_ptr;

    PROV_LOG("++++ _testCMPIString" );

    PROV_LOG("++++ creating CMPIString : CMNewString");
    string = CMNewString(_broker, data, &rc);
    PROV_LOG ("++++  CMNewString : (%s)", strCMPIStatus (rc));
    actual_string = CMGetCharsPtr(string, &rc);
    PROV_LOG("++++ CString is : %s", actual_string);

    PROV_LOG("++++ Clone on CMPIString");
    clonedString = string->ft->clone(string, &rc);

    PROV_LOG ("++++  Cloning String status : (%s)", strCMPIStatus (rc));

    cloned_string = CMGetCharsPtr(clonedString, &rc);
    PROV_LOG("++++ Cloned String is : %s", cloned_string);

    if (strcmp(actual_string,cloned_string) == 0)
    {
        cloneSuccessful = 1;
    }
    PROV_LOG("++++ clone Successful : %d ",cloneSuccessful);

    rc = clonedString->ft->release(clonedString);
    PROV_LOG ("++++ Cloned String Release : (%s)", strCMPIStatus (rc));
    //Error Paths

    string_ptr = string->hdl;
    string->hdl = NULL;

    string->ft->clone(string, &rc);
    PROV_LOG ("++++ Error Cloning String : (%s)", strCMPIStatus (rc));
    if (rc.rc != CMPI_RC_ERR_INVALID_HANDLE)
    {
        return 1;
    }

    rc = string->ft->release(string);
    PROV_LOG ("++++  Error String Release : (%s)", strCMPIStatus (rc));
    if (rc.rc != CMPI_RC_ERR_INVALID_HANDLE)
    {
        return 1;
    }

    CMGetCharsPtr(string, &rc);
    PROV_LOG ("++++  Error String getCharsPtr : (%s)", strCMPIStatus (rc));
    if (rc.rc != CMPI_RC_ERR_INVALID_HANDLE)
    {
        return 1;
    }

    string->hdl = string_ptr;
    rc = string->ft->release(string);
    PROV_LOG ("++++  String Release : (%s)", strCMPIStatus (rc));

    return 0;
}

// Test for CMPIArgs and its error paths.
static int _testCMPIArgs()
{
    CMPIStatus rc = { CMPI_RC_OK, NULL };

    CMPIArgs* args = NULL;
    CMPIArgs* clonedArgs = NULL;
    CMPIUint32 count = 0;
    CMPIType type = CMPI_uint32;
    char *arg1 = "arg1";
    char *arg2 = "arg2";
    CMPIValue value;
    CMPIData data;
    CMPIData clonedData;
    CMPIBoolean cloneSuccessful = 0;
    CMPIBoolean getArgCountSuccessful = 0;
    void *args_ptr;

    PROV_LOG("++++ _testCMPIArgs" );

    PROV_LOG("++++ creating CMPIArgs : CMNewArgs");
    args = CMNewArgs(_broker, &rc);
    PROV_LOG ("++++  CMNewArgs : (%s)", strCMPIStatus (rc));

    value.uint32 = 10;
    rc = CMAddArg(args, arg1, &value, type);
    PROV_LOG("++++  CMAddArg : (%s)", strCMPIStatus (rc));

    count = CMGetArgCount(args, &rc);
    PROV_LOG("++++  CMGetArgCount : (%s)", strCMPIStatus (rc));
    PROV_LOG("++++ Number of value added %d", count);
    if (count == 1)
    {
        getArgCountSuccessful = 1;
    }
    PROV_LOG("++++ GetArgCount Successful : %d ",getArgCountSuccessful);

    value.uint32 = 20;
    rc = CMAddArg(args, arg2, &value, type);
    PROV_LOG("++++  CMAddArg-2 : (%s)", strCMPIStatus (rc));

    count = CMGetArgCount(args, &rc);
    PROV_LOG("++++  CMGetArgCount-2 : (%s)", strCMPIStatus (rc));
    PROV_LOG("++++ Number of value added %d", count);
    if (count == 2)
    {
        getArgCountSuccessful = 1;
    }
    PROV_LOG("++++ GetArgCount Successful : %d ", getArgCountSuccessful);

    data = CMGetArg(args, arg2, &rc);
    PROV_LOG("++++  CMGetArg : (%s)", strCMPIStatus (rc));
    PROV_LOG("++++ Added data value is %d" , data.value.uint32);
    rc = CMAddArg(args, arg1, &value, type);

    PROV_LOG("++++ cloning Args");
    clonedArgs = args->ft->clone(args, &rc);
    PROV_LOG("++++  Cloning Args : (%s)", strCMPIStatus (rc));
    clonedData = CMGetArg(clonedArgs, arg2, &rc);
    rc = clonedArgs->ft->release(clonedArgs);
    PROV_LOG("++++  Cloned Args Release : (%s)", strCMPIStatus (rc));
    PROV_LOG("++++  CMGetArg-2 : (%s)", strCMPIStatus (rc));
    if (data.value.uint32 == clonedData.value.uint32)
    {
        cloneSuccessful = 1;
    }
    PROV_LOG("++++ clone Successful : %d ",cloneSuccessful);

    // Error Paths

    args_ptr = args->hdl;
    args->hdl = NULL;
    args->ft->clone(args, &rc);
    PROV_LOG("++++ Error Cloning Args : (%s)", strCMPIStatus (rc));
    if (rc.rc != CMPI_RC_ERR_INVALID_HANDLE)
    {
        return 1;
    }

    args->ft->release(args);
    PROV_LOG("++++  Error Args Release : (%s)", strCMPIStatus (rc));
    if (rc.rc != CMPI_RC_ERR_INVALID_HANDLE)
    {
        return 1;
    }

    CMGetArgCount(args, &rc);
    PROV_LOG(" CMGetArgCount : (%s)", strCMPIStatus (rc));
    if (rc.rc != CMPI_RC_ERR_INVALID_HANDLE)
    {
        return 1;
    }

    args->hdl = (CMPIArgs *)args_ptr;
    rc = args->ft->release(args);
    PROV_LOG("++++  Args Release : (%s)", strCMPIStatus (rc));

    return 0;
}


// Test for CMPIBroker and its error paths.
static int _testCMPIBroker (const CMPIContext* ctx)
{
    CMPIStatus rc = { CMPI_RC_OK, NULL };

    const char *properties_for_assoc[] = {"name", NULL};
    CMPIInstance* instOfAssocClass = NULL;
    unsigned int count = 0;
    const char* brokerName = NULL;

    //Boolean Variables used to Cross-Check the Functions' Success
    CMPIBoolean associatorsObjectPathSuccessful = 0;
    CMPIBoolean associatorNamesObjectPathSuccessful = 0;
    CMPIBoolean referenceObjectPathSuccessful = 0;
    CMPIBoolean referenceNamesObjectPathSuccessful = 0;

    //Enumeration Variables
    CMPIEnumeration* testEnumerationForAssociators = NULL;
    CMPIEnumeration* testErrorEnumerationForAssociators = NULL;
    CMPIEnumeration* testEnumerationForAssociatorNames = NULL;
    CMPIEnumeration* testErrorEnumerationForAssociatorNames = NULL;
    CMPIEnumeration* testEnumerationForReferences = NULL;
    CMPIEnumeration* testErrorEnumerationForReferences = NULL;
    CMPIEnumeration* testEnumerationForReferenceNames = NULL;

    //data Arrays to store the values
    CMPIData data[10];
    CMPIData dataAN[10];
    CMPIData dataR[10];
    CMPIData dataRN[10];

    //ObjectPaths for Associator-related Functions
    CMPIObjectPath* opForAssociatorFunctions = NULL;
    CMPIObjectPath* opForErrorAssociatorFunctions = NULL;
    CMPIObjectPath* opAssociators = NULL;
    CMPIObjectPath* opAssociatorNames = NULL;
    CMPIObjectPath* opReferences = NULL;
    CMPIObjectPath* opReferenceNames = NULL;

    //String Variables
    CMPIString* objPathAssociators = NULL;
    CMPIString* objPathAssociatorNames = NULL;
    CMPIString* objPathReferences = NULL;
    CMPIString* objPathReferenceNames = NULL;

    const char* getInstanceStringAssociators = NULL;
    const char* getInstanceStringAssociatorNames = NULL;
    const char* getInstanceStringReferences = NULL;
    const char* getInstanceStringReferenceNames = NULL;
    const char* errorCheck = "CMPI_TEST_erson";

    PROV_LOG("++++ _testCMPIBroker" );

    //Getting ObjectPaths for different Classes
    opForAssociatorFunctions = make_ObjectPath(_broker, _Namespace,
        _PersonClass);

    opForErrorAssociatorFunctions = make_ObjectPath(_broker, _Namespace,
        _PersonClass);

    opReferences = make_ObjectPath(_broker, _Namespace, "CMPI_TEST_Racing");

    //Getting Instances of Class
    instOfAssocClass = make_Instance(opForAssociatorFunctions);

    PROV_LOG("++++ BrokerName-Broker : CBBrokerName");
    brokerName = CBBrokerName(_broker);
    PROV_LOG("++++ Broker Name %s ", brokerName);

    //==============================CBAssociators==============================

    PROV_LOG("++++ Associators-Broker : CBAssociators");

    // Close log file before making upcall to provider that uses same PROV_LOG
    // functions.
    PROV_LOG_CLOSE();

    testEnumerationForAssociators = CBAssociators( _broker, ctx,
        opForAssociatorFunctions,"CMPI_TEST_Racing", NULL, NULL, NULL,
        properties_for_assoc, &rc);

    // Reopen our log file.
    PROV_LOG_OPEN (_ClassName, _ProviderLocation);

    PROV_LOG("++++  CBAssociators : (rc:%s)", strCMPIStatus (rc));

    while(CMHasNext(testEnumerationForAssociators, &rc))
    {
        data[count] = CMGetNext(testEnumerationForAssociators, &rc);
        opAssociators = CMGetObjectPath(data[count].value.inst, &rc);
        objPathAssociators = CMObjectPathToString(opAssociators, &rc);;
        getInstanceStringAssociators = CMGetCharsPtr(objPathAssociators,&rc);
        count++;
    }

    PROV_LOG("++++ Number of Associators : %d)", count);

    if (count == 2)
    {
        associatorsObjectPathSuccessful = 1;
    }
    PROV_LOG("++++ Associators success status : %d",
        associatorsObjectPathSuccessful);

    // Close log file before making upcall to provider that uses same PROV_LOG
    // functions.
    PROV_LOG_CLOSE();

    // testing with an Invalid Class Name
    testErrorEnumerationForAssociators = CBAssociators( _broker, ctx,
        opForAssociatorFunctions,"CMPI_TEST_acing", NULL, NULL, NULL,
        properties_for_assoc, &rc);

    // Reopen our log file.
    PROV_LOG_OPEN (_ClassName, _ProviderLocation);

    if (rc.rc != CMPI_RC_ERR_INVALID_PARAMETER)
    {
        return 1;
    }
    PROV_LOG("++++  CBAssociators Error Path 1: (rc:%s)", strCMPIStatus (rc));

    // Close log file before making upcall to provider that uses same PROV_LOG
    // functions.
    PROV_LOG_CLOSE();

    // testing with a class that is not an ASSOCIATION class
    testEnumerationForAssociators = CBAssociators( _broker, ctx,
        opForAssociatorFunctions,_PersonClass, NULL, NULL, NULL,
        properties_for_assoc, &rc);

    // Reopen our log file.
    PROV_LOG_OPEN (_ClassName, _ProviderLocation);

    if (rc.rc != CMPI_RC_OK)
    {
        return 1;
    }
    PROV_LOG("++++  CBAssociators Error Path 2: (rc:%s)", strCMPIStatus (rc));


    //============================CBAssociatorNames============================

    PROV_LOG("++++ Associators-Broker : CBAssociatorNames");

    // Close log file before making upcall to provider that uses same PROV_LOG
    // functions.
    PROV_LOG_CLOSE();

    testEnumerationForAssociatorNames = CBAssociatorNames
        (_broker, ctx, opForAssociatorFunctions, "CMPI_TEST_Racing", NULL,
        NULL, NULL, &rc);

    // Reopen our log file.
    PROV_LOG_OPEN (_ClassName, _ProviderLocation);

    PROV_LOG("++++  CBAssociatorNames : (rc:%s)", strCMPIStatus (rc));

    count = 0;
    while(CMHasNext(testEnumerationForAssociatorNames, &rc))
    {
        dataAN[count] = CMGetNext(testEnumerationForAssociatorNames, &rc);
        opAssociatorNames = dataAN[count].value.ref;
        objPathAssociatorNames = CMObjectPathToString(opAssociatorNames, &rc);
        getInstanceStringAssociatorNames = CMGetCharsPtr(
            objPathAssociatorNames,&rc);
        count++;
    }

    PROV_LOG("++++ Number of AssociatorNames : %d)", count);
    if (count == 2)
    {
        associatorNamesObjectPathSuccessful = 1;
    }
    PROV_LOG("++++ AssociatorNames success status : %d",
        associatorNamesObjectPathSuccessful);

    //Checking for Error Paths

    // Close log file before making upcall to provider that uses same PROV_LOG
    // functions.
    PROV_LOG_CLOSE();

    testErrorEnumerationForAssociatorNames = CBAssociatorNames
        (_broker, ctx, opForAssociatorFunctions, "CMPI_TEST_acing", NULL,
        NULL, NULL, &rc);

    // Reopen our log file.
    PROV_LOG_OPEN (_ClassName, _ProviderLocation);

    if (testErrorEnumerationForAssociatorNames == NULL)
    {
        PROV_LOG("++++ CBAssociatorNames - Error Path : NULL value returned");
    }
    else
    {
        PROV_LOG("++++ CBAssociatorNames - Error Path not reached");
    }

    //==============================CBReferences===============================

    PROV_LOG("++++ Associators-Broker : CBReferences");

    // Close log file before making upcall to provider that uses same PROV_LOG
    // functions.
    PROV_LOG_CLOSE();

    testEnumerationForReferences = CBReferences
        (_broker, ctx, opReferences, "CMPI_TEST_Racing", NULL,
        properties_for_assoc, &rc);

    // Reopen our log file.
    PROV_LOG_OPEN (_ClassName, _ProviderLocation);

    PROV_LOG("++++  CBReferences : (rc:%s)", strCMPIStatus (rc));

    count = 0;
    while(CMHasNext(testEnumerationForReferences, &rc))
    {
        dataR[count] = CMGetNext(testEnumerationForReferences, &rc);
        opReferences = dataR[count].value.ref;
        objPathReferences = CMObjectPathToString(opReferences, &rc);
        getInstanceStringReferences = CMGetCharsPtr(objPathReferences, &rc);
        count++;
    }

    PROV_LOG("++++ Number of References : %d)", count);

    if (count == 0)
    {
        referenceObjectPathSuccessful = 1;
    }
    PROV_LOG("++++ References success status : %d",
        referenceObjectPathSuccessful);

    //Checking for Error Paths

    // Close log file before making upcall to provider that uses same PROV_LOG
    // functions.
    PROV_LOG_CLOSE();

    testErrorEnumerationForReferences = CBReferences
        (_broker, ctx, opReferences, "CMPI_TEST_acing", NULL,
        properties_for_assoc, &rc);

    // Reopen our log file.
    PROV_LOG_OPEN (_ClassName, _ProviderLocation);

    if (testErrorEnumerationForReferences != NULL)
    {
        return 1;
    }

    //============================CBReferenceNames=============================

    PROV_LOG("++++ Associators-Broker : CBReferenceNames");

    // Close log file before making upcall to provider that uses same PROV_LOG
    // functions.
    PROV_LOG_CLOSE();

    testEnumerationForReferenceNames = CBReferenceNames
        (_broker, ctx, opForAssociatorFunctions, NULL, NULL, &rc);

    // Reopen our log file.
    PROV_LOG_OPEN (_ClassName, _ProviderLocation);

    PROV_LOG("++++  CBReferenceNames : (rc:%s)", strCMPIStatus (rc));

    count = 0;
    while(CMHasNext(testEnumerationForReferenceNames, &rc))
    {
        dataRN[count] = CMGetNext(testEnumerationForReferenceNames, &rc);
        opReferenceNames = dataRN[count].value.ref;
        objPathReferenceNames = CMObjectPathToString(opReferenceNames, &rc);
        getInstanceStringReferenceNames = CMGetCharsPtr(
             objPathReferenceNames,&rc);
        count++;
    }

    PROV_LOG("++++ Number of ReferenceNames : %d)", count);

    if (count == 2)
    {
        referenceNamesObjectPathSuccessful = 1;
    }
    else
    {
        referenceNamesObjectPathSuccessful = 0;
    }
    PROV_LOG("++++ ReferenceNames success status : %d",
        referenceNamesObjectPathSuccessful);

    //Checking for Error Paths

    // Close log file before making upcall to provider that uses same PROV_LOG
    // functions.
    PROV_LOG_CLOSE();

    testEnumerationForReferenceNames = CBReferenceNames
        (_broker, ctx, opForErrorAssociatorFunctions, errorCheck, NULL, &rc);

    // Reopen our log file.
    PROV_LOG_OPEN (_ClassName, _ProviderLocation);

    if (testErrorEnumerationForReferences != NULL)
    {
        return 1;
    }

    return 0;
}
          /* and many more soon to come */

/* ---------------------------------------------------------------------------*/
/*                        Method Provider Interface                           */
/* ---------------------------------------------------------------------------*/
#ifdef CMPI_VER_100
CMPIStatus
TestCMPIMethodProviderMethodCleanup (CMPIMethodMI * mi,
                                     const CMPIContext * ctx,
                                     CMPIBoolean  term)
#else
CMPIStatus
TestCMPIMethodProviderMethodCleanup (CMPIMethodMI * mi, CMPIContext * ctx)
#endif
{
  CMReturn (CMPI_RC_OK);
}

#ifdef CMPI_VER_100
CMPIStatus
TestCMPIMethodProviderInvokeMethod (CMPIMethodMI * mi,
                                    const CMPIContext * ctx,
                                    const CMPIResult * rslt,
                                    const CMPIObjectPath * ref,
                                    const char *methodName,
                                    const CMPIArgs * in, CMPIArgs * out)
#else
CMPIStatus
TestCMPIMethodProviderInvokeMethod (CMPIMethodMI * mi,
                                    CMPIContext * ctx,
                                    CMPIResult * rslt,
                                    CMPIObjectPath * ref,
                                    char *methodName,
                                    CMPIArgs * in, CMPIArgs * out)
#endif
{
  CMPIString *class = NULL;
  CMPIStatus rc = { CMPI_RC_OK, NULL };
  CMPIData data;
  CMPIString *argName = NULL;
  CMPIInstance *instance = NULL;
  CMPIInstance *paramInst = NULL;
  unsigned int arg_cnt = 0, index = 0;
  CMPIValue value;
  char *result = NULL;

   // This dummy method, which tests InvokeMethod UP call.

  if (strncmp ("testReturn", methodName, strlen ("testReturn")) == 0)
  {
      value.uint32 = 2;
      CMReturnData (rslt, &value, CMPI_uint32);
      CMReturnDone (rslt);
      return rc;
  }

  PROV_LOG_OPEN (_ClassName, _ProviderLocation);

  PROV_LOG ("--- %s CMPI InvokeMethod() called", _ClassName);

  class = CMGetClassName (ref, &rc);

  PROV_LOG ("InvokeMethod: checking for correct classname [%s]",
            CMGetCharsPtr (class,NULL));

  PROV_LOG ("Calling CMGetArgCount");
  arg_cnt = CMGetArgCount (in, &rc);
  PROV_LOG ("++++ (%s)", strCMPIStatus (rc));

  PROV_LOG ("InvokeMethod: We have %d arguments for operation [%s]: ",
            arg_cnt, methodName);
  if (arg_cnt > 0)
    {
      PROV_LOG ("Calling CMGetArgAt");
      for (index = 0; index < arg_cnt; index++)
        {
          data = CMGetArgAt (in, index, &argName, &rc);
          if (data.type == CMPI_uint32)
            {
              PROV_LOG ("#%d: %s (uint32), value: %d", index,
                        CMGetCharsPtr (argName,NULL), data.value.uint32);
            }
          else if (data.type == CMPI_string)
            {
              PROV_LOG ("#%d: %s (string) value: %s", index,
                        CMGetCharsPtr (argName,NULL),
                        CMGetCharsPtr (data.value.string,NULL));
            }
          else
          {
              PROV_LOG ("#%d: %s (type: %x)",
                  index,
                  CMGetCharsPtr (argName,NULL),
                  data.type);
          }
          CMRelease (argName);
        }
    }
  if (strncmp(
      CMGetCharsPtr (class,NULL),
      _ClassName,
      strlen (_ClassName)) == 0)
    {
      if (strncmp ("TestCMPIBroker", methodName, strlen ("TestCMPIBroker")) ==
          0)
        {
          // Parse the CMPIArgs in to figure out which operation it is.
          // There are six of them:
          //   ValueMap { "1", "2", "3", "4", "5", "6", "7"},
          //   Values {"CDGetType", "CDToString", "CDIsOfType", "CMGetMessage",
          //           "CMLogMessage","CDTraceMessage","CMGetMessage2"}]
          //    uint32 Operation,
          //    [OUT]string Result);
          PROV_LOG ("++++ Calling CMGetArg");
          data = CMGetArg (in, "Operation", &rc);
          PROV_LOG ("++++ (%s)", strCMPIStatus (rc));

          if (data.type == CMPI_uint32)
            {
              switch (data.value.uint32)
                {
                case 1:
                  value.uint32 = _CDGetType (in, &result);
                  break;
                case 2:
                  value.uint32 = _CDToString (in, &result);
                  break;
                case 3:
                  value.uint32 = _CDIsOfType (in, &result);
                  break;
                case 4:
                  value.uint32 = _CMGetMessage (&result);
                  break;
                case 5:
                  value.uint32 = _CMLogMessage (&result);
                  break;
                case 6:
                  value.uint32 = _CMTraceMessage (&result);
                  break;
                case 7:
                  {
                      CMPIData msgFileData;
                      CMPIData msgIdData;
                      CMPIData insert1Data;
                      CMPIData insert2Data;
                      PROV_LOG ("++++ Calling CMGetArg for msgFile");
                      msgFileData = CMGetArg (in, "msgFile", &rc);
                      PROV_LOG ("++++ (%s)", strCMPIStatus (rc));
                      PROV_LOG ("++++ Calling CMGetArg for msgId");
                      msgIdData = CMGetArg (in, "msgId", &rc);
                      PROV_LOG ("++++ (%s)", strCMPIStatus (rc));
                      PROV_LOG ("++++ Calling CMGetArg for insert1");
                      insert1Data = CMGetArg (in, "insert1", &rc);
                      PROV_LOG ("++++ (%s)", strCMPIStatus (rc));
                      PROV_LOG ("++++ Calling CMGetArg for insert2");
                      insert2Data = CMGetArg (in, "insert2", &rc);
                      PROV_LOG ("++++ (%s)", strCMPIStatus (rc));
                      value.uint32 = _CMGetMessage2 (&result,
                          CMGetCharsPtr(msgFileData.value.string, NULL),
                          CMGetCharsPtr(msgIdData.value.string, NULL),
                          CMGetCharsPtr(insert1Data.value.string, NULL),
                          insert2Data.value.uint32
                          );
                      break;
                  }
                case 8:
                    value.uint32 = _testCMPIEnumeration (ctx);
                    break;
                case 9:
                    value.uint32 = _testCMPIArray ();
                    break;
                case 10:
                    value.uint32 = _testCMPIcontext (ctx);
                    break;
                case 11:
                    value.uint32 = _testCMPIDateTime ();
                    break;
                case 12:
                    value.uint32 = _testCMPIInstance ();
                    break;
                case 13:
                    value.uint32 = _testCMPIObjectPath ();
                    break;
                case 14:
                    value.uint32 = _testCMPIResult (rslt);
                    break;
                case 15:
                    value.uint32 = _testCMPIString ();
                    break;
                case 16:
                    value.uint32 = _testCMPIArgs ();
                    break;
                case 17:
                    value.uint32 = _testCMPIBroker(ctx);
                default:
                  break;
                }
              // Return the value via Result
              PROV_LOG ("++++ Calling CMReturnData+Done");
              CMReturnData (rslt, &value, CMPI_uint32);
              CMReturnDone (rslt);

              // Return the string value value via putting it on the out
              // parameter.
              PROV_LOG ("++++ Calling CMAddArg");
              rc = CMAddArg (out, "Result", (CMPIValue *) result, CMPI_chars);
              PROV_LOG ("++++ (%s)", strCMPIStatus (rc));
              free (result);
            }
          else                  // type of data
            {
              PROV_LOG
                ("Unknown type of data [%X] for the Operation parameter.",
                 data.type);
              value.uint32 = 1;
              PROV_LOG ("++++ Calling CMReturnData+Done");
              CMReturnData (rslt, &value, CMPI_uint32);
              CMReturnDone (rslt);
            }
        }

      else if (strncmp ("returnString", methodName, strlen ("returnString"))
               == 0)
        {
          result = strdup ("Returning string");
          PROV_LOG
            ("++++ Calling CMReturnData+Done on returnString operation");
          CMReturnData (rslt, (CMPIValue *) result, CMPI_chars);
          CMReturnDone (rslt);
          free(result);
        }
      else if (strncmp ("returnUint32", methodName, strlen ("returnUint32"))
               == 0)
        {
          value.uint32 = 42;

          PROV_LOG
            ("++++ Calling CMReturnData+Done on returnUint32 operation");

          CMReturnData (rslt, &value, CMPI_uint32);
          CMReturnDone (rslt);
        }
      else if (strncmp ("returnInstance", methodName, strlen ("returnInstance"))
               == 0)
        {
          instance = _createInstance();
          PROV_LOG
            ("++++ Calling CMReturnData+Done on returnInstance operation");
          CMReturnData (rslt, (CMPIValue *) & instance, CMPI_instance);
          CMReturnDone (rslt);
          CMRelease(instance);
        }
      else if (
          strncmp("returnDateTime", methodName, strlen("returnDateTime")) == 0)
        {
          CMPIUint64 ret_val = 0;
          CMPIStatus dateTimeRc={CMPI_RC_OK, NULL};

          CMPIDateTime *dateTime = CMNewDateTime(_broker, &dateTimeRc);
          PROV_LOG ("---- (rc:%s)", strCMPIStatus (dateTimeRc));
          // Checking the date.
          ret_val = CMGetBinaryFormat (dateTime, &dateTimeRc);
          PROV_LOG ("---- (rc:%s)", strCMPIStatus (dateTimeRc));
          if (ret_val == 0)
              PROV_LOG("Invalid conversion of date to CMPIDateTime");

          PROV_LOG
              ("++++ Calling CMReturnData+Done on returnDateTime operation");

          CMReturnData (rslt, (CMPIValue *) & dateTime, CMPI_dateTime);
          CMReturnDone (rslt);
        }
    else if(strncmp("processEmbeddedInstance", methodName,
      strlen ("processEmbeddedInstance"))== 0)
    {
        CMPIData d;

        PROV_LOG("++++ Creating instance for processEmbeddedInstance");
        instance = _createInstance();
        PROV_LOG("++++ Getting inputInstance arg");
        data = CMGetArg(in, "inputInstance", &rc);
        PROV_LOG("++++ (%s)", strCMPIStatus (rc));
        
        d  = CMGetProperty(data.value.inst, "s", &rc);
        if (rc.rc == CMPI_RC_OK && d.state == CMPI_nullValue)
        {
           PROV_LOG("++++ Error, Null value for empty string");
        }

        PROV_LOG("++++ Cloning inputInstance arg");
        paramInst = data.value.inst->ft->clone(
          data.value.inst, &rc);
        PROV_LOG("++++ (%s)", strCMPIStatus (rc));

        PROV_LOG("++++ Setting outputInstance arg");
        rc = CMAddArg (out, "outputInstance",
          (CMPIValue *) &paramInst, CMPI_instance);
        PROV_LOG("++++ (%s)", strCMPIStatus (rc));

        PROV_LOG(
            "++++ Calling CMReturnData on processEmbeddedInstance operation");
        CMReturnData (rslt, (CMPIValue *) &instance, CMPI_instance);

        PROV_LOG(
            "++++ Calling CMReturnDone on processEmbeddedInstance operation");
        CMReturnDone (rslt);
        paramInst->ft->release(paramInst);
    }
    else if(strncmp("processArrayEmbeddedInstance", methodName,
        strlen ("processArrayEmbeddedInstance"))== 0)
    {
        CMPIData data1, data2, data3;
        CMPIInstance *inst1, *inst2, *inst3;
        CMPIInstance *emInst1, *emInst2, *emInst3;
        CMPIObjectPath *objPath;
        CMPIValue value;
        CMPIArray *outArray = CMNewArray (_broker, 3, CMPI_instance, NULL);
        PROV_LOG("++++ Creating instance for processArryEmbeddedInstance");
        instance = _createInstance();
        PROV_LOG("++++ Getting inputInstance arg");
        data = CMGetArg(in, "inputInstances", &rc);
        PROV_LOG("++++ (%s)", strCMPIStatus (rc));

        PROV_LOG("++++ Getting Array elements.");
        data1 = CMGetArrayElementAt(data.value.array,0,&rc);
        PROV_LOG("++++ 1 (%s)", strCMPIStatus (rc));
        data2 = CMGetArrayElementAt(data.value.array,1,&rc);
        PROV_LOG("++++ 2 (%s)", strCMPIStatus (rc));
        data3 = CMGetArrayElementAt(data.value.array,2,&rc);
        PROV_LOG("++++ 3 (%s)", strCMPIStatus (rc));
        PROV_LOG("++++ Cloning input arg array elements");
        inst1 = data1.value.inst->ft->clone(
          data1.value.inst, &rc);
        PROV_LOG("++++ 1 (%s)", strCMPIStatus (rc));
        inst2 = data2.value.inst->ft->clone(
          data2.value.inst, &rc);
        PROV_LOG("++++ 2 (%s)", strCMPIStatus (rc));
        inst3 = data3.value.inst->ft->clone(
          data3.value.inst, &rc);
        PROV_LOG("++++ 3 (%s)", strCMPIStatus (rc));
        PROV_LOG("++++ Creating output  arg array elements");
        rc = CMSetArrayElementAt(outArray, 0,&inst1, CMPI_instance);
        PROV_LOG("++++ 1 (%s)", strCMPIStatus (rc));
        rc = CMSetArrayElementAt(outArray, 1, &inst2, CMPI_instance);
        PROV_LOG("++++ 2 (%s)", strCMPIStatus (rc));
        rc = CMSetArrayElementAt(outArray, 2, &inst3, CMPI_instance);
        PROV_LOG("++++ 3 (%s)", strCMPIStatus (rc));

        PROV_LOG ("++++  Creating ObjectPath for TestCMPI_Embedded instance");
        objPath = CMNewObjectPath (
            _broker,
            "test/TestProvider",
            "TestCMPI_Embedded",
            &rc);
        PROV_LOG("CMNewObjectPath status (%s)", strCMPIStatus (rc));
        value.uint32 = 1;
        rc = CMAddKey(objPath,"id",&value, CMPI_uint32);
        PROV_LOG("++++ CMAddKey (%s)", strCMPIStatus (rc));
        PROV_LOG ("++++  Creating TestCMPI_Embedded instance");
        emInst1 = CMNewInstance(_broker, objPath, &rc);
        PROV_LOG("CMNewInstance status (%s)", strCMPIStatus (rc));
        PROV_LOG("++++ Setting TestCMPI_Embedded instance properties");
        rc = CMSetProperty(emInst1, "id", 0, CMPI_uint32);
        rc = CMSetProperty(emInst1, "id", &value, CMPI_uint32);
        PROV_LOG("++++ CMSetProperty  (%s)", strCMPIStatus (rc));
        value.inst = 0;
        rc = CMSetProperty(emInst1, "emInstance", &value, CMPI_instance);
        value.inst = inst1;
        rc = CMSetProperty(emInst1, "emInstance", &value, CMPI_instance);
        PROV_LOG("++++ CMSetProperty  (%s)", strCMPIStatus (rc));
        rc = CMSetProperty(emInst1, "emObject", &value, CMPI_instance);
        PROV_LOG("++++ CMSetProperty  (%s)", strCMPIStatus (rc));
        emInst2 = emInst1->ft->clone(emInst1, &rc);
        PROV_LOG("++++ clone 1 (%s)", strCMPIStatus (rc));
        value.uint32 = 2;
        rc = CMSetProperty(emInst2, "id", &value, CMPI_uint32);
        PROV_LOG("++++ CMSetProperty  (%s)", strCMPIStatus (rc));
        rc = CMAddKey(objPath,"id",&value, CMPI_uint32);
        PROV_LOG("++++ CMAddKey (%s)", strCMPIStatus (rc));
        rc = CMSetObjectPath(emInst2, objPath);
        PROV_LOG("++++ CMSetObjectPath  (%s)", strCMPIStatus (rc));
        emInst3 = emInst1->ft->clone(emInst1, &rc);
        PROV_LOG("++++ clone 2 (%s)", strCMPIStatus (rc));
        value.uint32 = 3;
        rc = CMSetProperty(emInst3, "id", &value, CMPI_uint32);
        PROV_LOG("++++ CMSetProperty  (%s)", strCMPIStatus (rc));
        rc = CMAddKey(objPath,"id",&value, CMPI_uint32);
        PROV_LOG("++++ CMAddKey (%s)", strCMPIStatus (rc));
        rc = CMSetObjectPath(emInst3, objPath);
        PROV_LOG("++++ CMSetObjectPath  (%s)", strCMPIStatus (rc));

        PROV_LOG("++++ Setting outputInstance arg");
        rc = CMAddArg (out, "outputInstances",
          (CMPIValue *) &outArray, CMPI_instanceA);
        PROV_LOG("++++ (%s)", strCMPIStatus (rc));

        PROV_LOG("++++ Setting outputObject arg");
        rc = CMAddArg (out, "outputObjects",
          (CMPIValue *) &outArray, CMPI_instanceA);
        PROV_LOG("++++ (%s)", strCMPIStatus (rc));

        PROV_LOG("++++ Setting outputEmbeddedObject arg");

        PROV_LOG("++++ Creating output  arg array elements");
        rc = CMSetArrayElementAt(outArray, 0,&emInst1, CMPI_instance);
        PROV_LOG("++++ 1 (%s)", strCMPIStatus (rc));
        rc = CMSetArrayElementAt(outArray, 1, &emInst2, CMPI_instance);
        PROV_LOG("++++ 2 (%s)", strCMPIStatus (rc));
        rc = CMSetArrayElementAt(outArray, 2, &emInst3, CMPI_instance);
        PROV_LOG("++++ 3 (%s)", strCMPIStatus (rc));

        rc = CMAddArg (out, "outputEmbeddedObjects",
          (CMPIValue *) &outArray, CMPI_instanceA);
        PROV_LOG("++++ (%s)", strCMPIStatus (rc));

        PROV_LOG(
            "++++ Calling CMReturnData on processArrayEmbeddedInstance"
            " operation");
        CMReturnData (rslt, (CMPIValue *) &instance, CMPI_instance);

        PROV_LOG(
            "++++ Calling CMReturnDone on processArrayEmbeddedInstance"
            " operation");
        CMReturnDone (rslt);
        inst1->ft->release(inst1);
        inst2->ft->release(inst2);
        inst3->ft->release(inst3);
        emInst2->ft->release(emInst2);
        emInst3->ft->release(emInst3);
    }
    else if (
        strncmp("testArrayTypes", methodName, strlen ("testArrayTypes"))== 0)
    {
        value.uint32 = _testArrayTypes();
        CMReturnData (rslt, &value, CMPI_uint32);
        CMReturnDone (rslt);
    }
    else if (
        strncmp("testErrorPaths", methodName, strlen ("testErrorPaths")) == 0)
    {
        value.uint32 = _testErrorPaths();
        CMReturnData (rslt, &value, CMPI_uint32);
        CMReturnDone (rslt);
    }
    else if (
        strncmp("testSimpleTypes", methodName, strlen ("testSimpleTypes")) == 0)
    {
        value.uint32 = _testSimpleTypes();
        CMReturnData (rslt, &value, CMPI_uint32);
        CMReturnDone (rslt);
    }
    else if (
        strncmp("testArrayClone", methodName, strlen ("testArrayClone")) == 0 )
    {
        value.uint32 = _testArrayClone(ctx);
        CMReturnData (rslt, &value, CMPI_uint32);
        CMReturnDone (rslt);
    }
    else if (strncmp ("methodNotInMof", methodName, strlen ("methodNotInMof"))
        == 0)
    {
        value.uint32 = 42;
        PROV_LOG
            ("++++ Calling CMReturnData+Done on methodNotInMof operation");
        CMReturnData (rslt, &value, CMPI_uint32);
        CMReturnDone (rslt);
    }
      else
        {
          PROV_LOG ("++++ Could not find the %s operation", methodName);
          rc.rc = CMPI_RC_ERR_NOT_FOUND;
          rc.msg=_broker->eft->newString(_broker,methodName,0);
        }
    }
  PROV_LOG ("--- %s CMPI InvokeMethod() exited", _ClassName);
  PROV_LOG_CLOSE();
  return rc;
}

/* ---------------------------------------------------------------------------*/
/*                              Provider Factory                              */
/* ---------------------------------------------------------------------------*/


CMMethodMIStub (TestCMPIMethodProvider,
                TestCMPIMethodProvider, _broker, CMNoHook)

/* ---------------------------------------------------------------------------*/
/*             end of TestCMPIProvider                      */
/* ---------------------------------------------------------------------------*/
