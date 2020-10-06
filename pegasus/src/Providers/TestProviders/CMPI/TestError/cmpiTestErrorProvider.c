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

#include <Pegasus/Provider/CMPI/cmpidt.h>
#include <Pegasus/Provider/CMPI/cmpift.h>
#include <Pegasus/Provider/CMPI/cmpimacs.h>
#include <Providers/TestProviders/CMPI/TestUtilLib/cmpiUtilLib.h>

#define _ClassName "TestCMPI_Error"
#define _ClassName_size strlen(_ClassName)
#define _Namespace    "test/TestProvider"

#define _ProviderLocation "/src/Providers/TestProviders/CMPI/TestError/tests/"

static const CMPIBroker *_broker;

/* ---------------------------------------------------------------------------*/
/*                        Method Provider Interface                           */
/* ---------------------------------------------------------------------------*/
CMPIStatus
TestCMPIErrorProviderMethodCleanup (CMPIMethodMI * mi,
                                     const CMPIContext * ctx,
                                     CMPIBoolean  term)
{
  CMReturn (CMPI_RC_OK);
}

CMPIStatus
TestCMPIErrorProviderInvokeMethod (CMPIMethodMI * mi,
                                    const CMPIContext * ctx,
                                    const CMPIResult * rslt,
                                    const CMPIObjectPath * ref,
                                    const char *methodName,
                                    const CMPIArgs * in, CMPIArgs * out)
{
  CMPIString *class_name = NULL;
  CMPIStatus rc = { CMPI_RC_OK, NULL };
  CMPIError *cmpiError;
  CMPIError *cmpiErrorClone;
  CMPICount i, arrSize;
  CMPIObjectPath *objPath = NULL;


  CMPIUint32 brokerCapabilities;
  /* CMPIError data */
  const char* inOwningEntity = "ACME";
  CMPIString* outOwningEntity;
  const char* inMsgID = "ACME0023";
  CMPIString* outMsgID;
  const char* inMsg = "ACME0023 Need to restock Road Runner bait and traps.";
  CMPIString* outMsg;
  const CMPIErrorSeverity inSev = ErrorSevMedium;
  CMPIErrorSeverity outSev;
  const CMPIErrorProbableCause inPc = Equipment_Malfunction;
  CMPIErrorProbableCause outPc;
  const CMPIrc inCIMStatusCode = CMPI_RC_ERR_FAILED;
  CMPIrc outCIMStatusCode;
  const char * inCIMStatusCodeDesc = "another failed attempt at road "
      "runner elimination";
  CMPIString* outCIMStatusCodeDesc;
  const CMPIErrorType inErrType = OtherErrorType;
  CMPIErrorType outErrType;
  const char* inOtherErrType = "Pervasive Coyote Incompetance";
  CMPIString* outOtherErrType;
  const char* inPcd = "Incorrect use of equipment";
  CMPIString* outPcd;
  CMPIArray *inRecActions;
  CMPIArray *outRecActions;
  CMPIArray *inMsgArgs;
  CMPIArray *outMsgArgs;
  CMPIValue raOne;
  CMPIValue raTwo;
  const char* inErrSource = "Acme bomb";
  CMPIString* outErrSource;
  const CMPIErrorSrcFormat inErrSourceFormat = CMPIErrSrcOther;
  CMPIErrorSrcFormat outErrSourceFormat;
  const char* inOtherErrSourceFormat = "no idea";
  CMPIString* outOtherErrSourceFormat;
  CMPIData retData;
  void* handle;
  PROV_LOG_OPEN (_ClassName, _ProviderLocation);

  PROV_LOG ("Calling CBGetBrokerCapabilities");
  brokerCapabilities = CBGetBrokerCapabilities(_broker);
  if (brokerCapabilities & CMPI_MB_Supports_Extended_Error)
  {
      PROV_LOG("CMPI_MB_Supports_Extended_Error Support : True");
  }
  else
  {
      PROV_LOG("CMPI_MB_Supports_Extended_Error Support : False");
      CMReturnWithString(CMPI_RC_ERR_NOT_SUPPORTED,
          CMNewString(_broker,
          "Extended error support not avilable", NULL));
  }

  PROV_LOG ("--- %s CMPI InvokeMethod() called", _ClassName);

  class_name = CMGetClassName (ref, &rc);

  PROV_LOG ("InvokeMethod: checking for correct classname [%s]",
            CMGetCharsPtr (class_name,NULL));

  PROV_LOG ("Calling CMNewCMPIError");
  cmpiError = CMNewCMPIError(_broker, inOwningEntity, inMsgID, inMsg,
      inSev, inPc, inCIMStatusCode, &rc);
  PROV_LOG ("++++ (%s) CMNewCMPIError", strCMPIStatus (rc));

  PROV_LOG ("Calling CMSetErrorType");
  rc = CMSetErrorType(cmpiError, inErrType);
  PROV_LOG ("++++ (%s) CMSetErrorType", strCMPIStatus (rc));

  PROV_LOG ("Calling CMSetOtherErrorType");
  rc = CMSetOtherErrorType(cmpiError, inOtherErrType);
  PROV_LOG ("++++ (%s) CMSetOtherErrorType", strCMPIStatus (rc));

  PROV_LOG ("Calling CMSetProbableCauseDescription");
  rc = CMSetProbableCauseDescription(cmpiError, inPcd);
  PROV_LOG ("++++ (%s) CMSetProbableCauseDescription", strCMPIStatus (rc));

  inRecActions = CMNewArray(_broker, 2, CMPI_string, &rc);
  PROV_LOG ("++++ (%s) CMNewArray", strCMPIStatus (rc));
  raOne.string = CMNewString(_broker, "Fire coyote.", &rc);
  PROV_LOG ("++++ (%s) CMNewString", strCMPIStatus (rc));
  rc = CMSetArrayElementAt(inRecActions, 0, &raOne, CMPI_string);
  PROV_LOG ("++++ (%s) CMSetArrayElementAt [0]", strCMPIStatus (rc));
  raTwo.string = CMNewString(_broker, "Give peace a chance.", &rc);
  PROV_LOG ("++++ (%s) CMNewString", strCMPIStatus (rc));
  rc = CMSetArrayElementAt(inRecActions, 1, &raTwo, CMPI_string);
  PROV_LOG ("++++ (%s) CMSetArrayElementAt [1]", strCMPIStatus (rc));

  PROV_LOG ("Calling CMSetRecommendedActions");
  rc = CMSetRecommendedActions(cmpiError, inRecActions);
  PROV_LOG ("++++ (%s) CMSetRecommendedActions", strCMPIStatus (rc));

  PROV_LOG ("Calling CMSetErrorSource");
  rc = CMSetErrorSource(cmpiError, inErrSource);
  PROV_LOG ("++++ (%s) CMSetErrorSource", strCMPIStatus (rc));

  PROV_LOG ("Calling CMSetErrorSourceFormat");
  rc = CMSetErrorSourceFormat(cmpiError, inErrSourceFormat);
  PROV_LOG ("++++ (%s) CMSetErrorSourceFormat", strCMPIStatus (rc));

  PROV_LOG ("Calling CMSetOtherErrorSourceFormat");
  rc = CMSetOtherErrorSourceFormat(cmpiError, inOtherErrSourceFormat);
  PROV_LOG ("++++ (%s) CMSetOtherErrorSourceFormat", strCMPIStatus (rc));

  PROV_LOG ("Calling CMSetCIMStatusCodeDescription");
  rc = CMSetCIMStatusCodeDescription(cmpiError, inCIMStatusCodeDesc);
  PROV_LOG ("++++ (%s) CMSetCIMStatusCodeDescription", strCMPIStatus (rc));

  inMsgArgs = CMNewArray(_broker, 1, CMPI_string, &rc);
  PROV_LOG ("++++ (%s) CMNewArray", strCMPIStatus (rc));
  raOne.string = CMNewString(_broker, "no real inserts", &rc);
  PROV_LOG ("++++ (%s) CMNewString", strCMPIStatus (rc));
  rc = CMSetArrayElementAt(inMsgArgs, 0, &raOne, CMPI_string);
  PROV_LOG ("++++ (%s) CMSetArrayElementAt [0]", strCMPIStatus (rc));

  PROV_LOG ("Calling CMSetMessageArguments");
  rc = CMSetMessageArguments(cmpiError, inMsgArgs);
  PROV_LOG ("++++ (%s) CMSetMessageArguments", strCMPIStatus (rc));

  PROV_LOG ("First clone the CMPIError.");
  cmpiErrorClone = CMClone(cmpiError, &rc);
  PROV_LOG ("++++ (%s) CMClone", strCMPIStatus (rc));

  PROV_LOG ("Ok, reading back fields to verify");

  PROV_LOG ("Calling CMGetErrorType");
  outErrType = CMGetErrorType(cmpiErrorClone, &rc);
  PROV_LOG ("++++ (%s) CMGetErrorType (%d)", strCMPIStatus (rc), outErrType);

  PROV_LOG ("Calling CMGetOtherErrorType");
  outOtherErrType = CMGetOtherErrorType(cmpiErrorClone, &rc);
  PROV_LOG ("++++ (%s) CMGetOtherErrorType (%s)", strCMPIStatus (rc),
      CMGetCharsPtr(outOtherErrType,NULL));

  PROV_LOG ("Calling CMGetOwningEntity");
  outOwningEntity = CMGetOwningEntity(cmpiErrorClone, &rc);
  PROV_LOG ("++++ (%s) CMGetOwningEntity (%s)", strCMPIStatus (rc),
      CMGetCharsPtr(outOwningEntity,NULL));

  PROV_LOG ("Calling CMGetMessageID");
  outMsgID = CMGetMessageID(cmpiErrorClone, &rc);
  PROV_LOG ("++++ (%s) CMGetMessageID (%s)", strCMPIStatus (rc),
      CMGetCharsPtr(outMsgID,NULL));

  PROV_LOG ("Calling CMGetErrorMessage");
  outMsg = CMGetErrorMessage(cmpiErrorClone, &rc);
  PROV_LOG ("++++ (%s) CMGetErrorMessage (%s)", strCMPIStatus (rc),
      CMGetCharsPtr(outMsg,NULL));

  PROV_LOG ("Calling CMGetPerceivedSeverity");
  outSev = CMGetPerceivedSeverity(cmpiErrorClone, &rc);
  PROV_LOG ("++++ (%s) CMGetPerceivedSeverity (%d)", strCMPIStatus (rc),
      outSev);

  PROV_LOG ("Calling CMGetProbableCause");
  outPc = CMGetProbableCause(cmpiErrorClone, &rc);
  PROV_LOG ("++++ (%s) CMGetProbableCause (%d)", strCMPIStatus (rc),
      outPc);

  PROV_LOG ("Calling CMGetProbableCauseDescription");
  outPcd = CMGetProbableCauseDescription(cmpiErrorClone, &rc);
  PROV_LOG ("++++ (%s) CMGetProbableCauseDescription (%s)",
      strCMPIStatus (rc),
      CMGetCharsPtr(outPcd,NULL));

  PROV_LOG ("Calling CMGetRecommendedActions");
  outRecActions = CMGetRecommendedActions(cmpiErrorClone, &rc);
  PROV_LOG ("++++ (%s) CMGetRecommendedActions", strCMPIStatus (rc));
  arrSize = CMGetArrayCount(outRecActions, &rc);
  PROV_LOG ("++++ (%s) CMGetArrayCount (%d)", strCMPIStatus (rc), arrSize);
  for (i = 0 ; i < arrSize ; i++)
  {
      CMPIData dta = CMGetArrayElementAt(outRecActions, i, &rc);
      PROV_LOG ("++++ (%s) CMGetArrayElementAt (%d:%s)", strCMPIStatus (rc),
          i, CMGetCharsPtr(dta.value.string,NULL));
  }

  PROV_LOG ("Calling CMGetErrorSource");
  outErrSource = CMGetErrorSource(cmpiErrorClone, &rc);
  PROV_LOG ("++++ (%s) CMGetErrorSource (%s)", strCMPIStatus (rc),
      CMGetCharsPtr(outErrSource,NULL));

  PROV_LOG ("Calling CMGetErrorSourceFormat");
  outErrSourceFormat = CMGetErrorSourceFormat(cmpiErrorClone, &rc);
  PROV_LOG ("++++ (%s) CMGetErrorSourceFormat (%d)", strCMPIStatus (rc),
      outErrSourceFormat);

  PROV_LOG ("Calling CMGetOtherErrorSourceFormat");
  outOtherErrSourceFormat = CMGetOtherErrorSourceFormat(cmpiErrorClone, &rc);
  PROV_LOG ("++++ (%s) CMGetOtherErrorSourceFormat (%s)", strCMPIStatus (rc),
      CMGetCharsPtr(outOtherErrSourceFormat,NULL));

  PROV_LOG ("Calling CMGetCIMStatusCode");
  outCIMStatusCode = CMGetCIMStatusCode(cmpiErrorClone, &rc);
  PROV_LOG ("++++ (%s) CMGetCIMStatusCode (%d)", strCMPIStatus (rc),
      outCIMStatusCode);

  PROV_LOG ("Calling CMGetCIMStatusCodeDescription");
  outCIMStatusCodeDesc = CMGetCIMStatusCodeDescription(cmpiErrorClone, &rc);
  PROV_LOG ("++++ (%s) CMGetCIMStatusCodeDescription (%s)", strCMPIStatus (rc),
      CMGetCharsPtr(outCIMStatusCodeDesc,NULL));

  PROV_LOG ("Calling CMGetMessageArguments");
  outMsgArgs = CMGetMessageArguments(cmpiErrorClone, &rc);
  PROV_LOG ("++++ (%s) CMGetMessageArguments", strCMPIStatus (rc));
  arrSize = CMGetArrayCount(outMsgArgs, &rc);
  PROV_LOG ("++++ (%s) CMGetArrayCount (%d)", strCMPIStatus (rc), arrSize);
  for (i = 0 ; i < arrSize ; i++)
  {
      CMPIData dta = CMGetArrayElementAt(outMsgArgs, i, &rc);
      PROV_LOG ("++++ (%s) CMGetArrayElementAt (%d:%s)", strCMPIStatus (rc),
          i, CMGetCharsPtr(dta.value.string,NULL));
  }

  PROV_LOG ("Calling CMPIResultFT.returnData");
  rc = (rslt)->ft->returnError (rslt, cmpiErrorClone);
  PROV_LOG ("++++ (%s) returnData", strCMPIStatus (rc));

  PROV_LOG ("Releasing cloned CMPIError");
  rc = CMRelease(cmpiErrorClone);
  PROV_LOG ("++++ (%s) CMClone", strCMPIStatus (rc));

//Test Error Paths
  PROV_LOG ("Testing for Error Paths in CMPI_Error.cpp");
  PROV_LOG ("Calling CMNewCMPIError");
  cmpiError = CMNewCMPIError(_broker,
      inOwningEntity,
      inMsgID,
      inMsg,
      inSev,
      inPc,
      inCIMStatusCode,
      &rc);
  PROV_LOG ("++++ (%s) CMNewCMPIError", strCMPIStatus (rc));

  PROV_LOG ("Calling CMGetErrorType without setting the ErrorType");
  outErrType = CMGetErrorType(cmpiError, &rc);
  if (!outErrType)
  {
      PROV_LOG ("++++ (%s) CMGetErrorType (%d)", strCMPIStatus (rc),
          outErrType);
  }

  PROV_LOG ("Calling CMGetOtherErrorType without setting the OtherErrorType");
  outOtherErrType = CMGetOtherErrorType(cmpiError, &rc);
  if (outOtherErrType == NULL)
  {
      PROV_LOG ("++++ (%s) CMGetOtherErrorType", strCMPIStatus (rc));
  }

  PROV_LOG ("Testing CMSetMessageArguments");
  CMRelease(inMsgArgs);
  inMsgArgs = CMNewArray(_broker, 1, CMPI_uint32, &rc);
  PROV_LOG ("++++ (%s) CMNewArray inMsgArgs of CMPI_uint32 type",
      strCMPIStatus (rc));
  raOne.uint32 = 32;
  rc = CMSetArrayElementAt(inMsgArgs, 0, &raOne, CMPI_uint32);
  PROV_LOG ("++++ (%s) CMSetArrayElementAt [0]", strCMPIStatus (rc));
  PROV_LOG ("Calling CMSetMessageArguments with input array of uint32");
  rc = CMSetMessageArguments(cmpiError, inMsgArgs);
  PROV_LOG ("++++ (%s) CMSetMessageArguments", strCMPIStatus (rc));

  handle = inMsgArgs->hdl;
  inMsgArgs->hdl = NULL;
  PROV_LOG ("Calling CMSetMessageArguments with input array with NULL handle");
  rc = CMSetMessageArguments(cmpiError, inMsgArgs);
  PROV_LOG ("++++ (%s) CMSetMessageArguments", strCMPIStatus (rc));
  inMsgArgs->hdl = handle;

  PROV_LOG ("Testing CMSetRecommendedActions");
  CMRelease(inRecActions);
  inRecActions = CMNewArray(_broker, 1, CMPI_uint32, &rc);
  PROV_LOG ("++++ (%s) CMNewArray inRecActions of CMPI_uint32",
      strCMPIStatus (rc));
  raOne.uint32 = 32;
  rc = CMSetArrayElementAt(inRecActions, 0, &raOne, CMPI_uint32);
  PROV_LOG ("++++ (%s) CMSetArrayElementAt [0]", strCMPIStatus (rc));
  PROV_LOG ("Calling CMSetRecommendedActions with input array of CMPI_uint32");
  rc = CMSetRecommendedActions(cmpiError, inRecActions);
  PROV_LOG ("++++ (%s) CMSetRecommendedActions", strCMPIStatus (rc));
  handle = inRecActions->hdl;
  inRecActions->hdl = NULL;
  PROV_LOG ("Calling CMSetRecommendedActions with input array"
      " with NULL handle");
  rc = CMSetRecommendedActions(cmpiError, inRecActions);
  PROV_LOG ("++++ (%s) CMSetRecommendedActions", strCMPIStatus (rc));
  inRecActions->hdl = handle;
  /* Cases when CMPIError object handle is NULL*/
  handle =  cmpiError->hdl;
  cmpiError->hdl = NULL;

  PROV_LOG ("Clone the CMPIError with NULL handle.");
  cmpiErrorClone = CMClone(cmpiError, &rc);
  if (cmpiErrorClone == NULL)
  {
      PROV_LOG ("++++ (%s) CMClone", strCMPIStatus (rc));
  }

  PROV_LOG ("Calling CMGetErrorType with NULL handle");
  outErrType = CMGetErrorType(cmpiError, &rc);
  if (!outErrType)
  {
      PROV_LOG ("++++ (%s) CMGetErrorType (%d)", strCMPIStatus (rc),
          outErrType);
  }

  PROV_LOG ("Calling CMGetOtherErrorType with NULL handle");
  outOtherErrType = CMGetOtherErrorType(cmpiError, &rc);
  if (outOtherErrType == NULL)
  {
      PROV_LOG ("++++ (%s) CMGetOtherErrorType ", strCMPIStatus (rc));
  }

  PROV_LOG ("Calling CMGetOwningEntity with NULL handle");
  outOwningEntity = CMGetOwningEntity(cmpiError, &rc);
  if (outOwningEntity == NULL)
  {
      PROV_LOG ("++++ (%s) CMGetOwningEntity ", strCMPIStatus (rc));
  }

  PROV_LOG ("Calling CMGetMessageID with NULL handle");
  outMsgID = CMGetMessageID(cmpiError, &rc);
  if (!outMsgID)
  {
      PROV_LOG ("++++ (%s) CMGetMessageID", strCMPIStatus (rc));
  }

  PROV_LOG ("Calling CMGetErrorMessage with NULL handle");
  outMsg = CMGetErrorMessage(cmpiError, &rc);
  if (!outMsg)
  {
      PROV_LOG ("++++ (%s) CMGetErrorMessage", strCMPIStatus (rc));
  }
  PROV_LOG ("Calling CMGetPerceivedSeverity with NULL handle");
  outSev = CMGetPerceivedSeverity(cmpiError, &rc);
  if (!outSev)
  {
      PROV_LOG ("++++ (%s) CMGetPerceivedSeverity (%d)", strCMPIStatus (rc),
          outSev);
  }

  PROV_LOG ("Calling CMGetProbableCause with NULL handle");
  outPc = CMGetProbableCause(cmpiError, &rc);
  if (!outPc)
  {
      PROV_LOG ("++++ (%s) CMGetProbableCause (%d)", strCMPIStatus (rc),
          outPc);
  }

  PROV_LOG ("Calling CMGetProbableCauseDescription with NULL handle");
  outPcd = CMGetProbableCauseDescription(cmpiError, &rc);
  if (!outPcd)
  {
      PROV_LOG ("++++ (%s) CMGetProbableCauseDescription ",
          strCMPIStatus (rc));
  }

  PROV_LOG ("Calling CMGetRecommendedActions with NULL handle");
  outRecActions = CMGetRecommendedActions(cmpiError, &rc);
  if (!outRecActions)
  {
      PROV_LOG ("++++ (%s) CMGetRecommendedActions", strCMPIStatus (rc));
  }

  PROV_LOG ("Calling CMGetErrorSource with NULL handle");
  outErrSource = CMGetErrorSource(cmpiError, &rc);
  if (!outErrSource)
  {
      PROV_LOG ("++++ (%s) CMGetErrorSource", strCMPIStatus (rc));
  }

  PROV_LOG ("Calling CMGetErrorSourceFormat with NULL handle");
  outErrSourceFormat = CMGetErrorSourceFormat(cmpiError, &rc);
  if (!outErrSourceFormat)
  {
      PROV_LOG ("++++ (%s) CMGetErrorSourceFormat (%d)", strCMPIStatus (rc),
          outErrSourceFormat);
  }

  PROV_LOG ("Calling CMGetOtherErrorSourceFormat with NULL handle");
  outOtherErrSourceFormat = CMGetOtherErrorSourceFormat(cmpiError, &rc);
  if (!outOtherErrSourceFormat)
  {
      PROV_LOG ("++++ (%s) CMGetOtherErrorSourceFormat", strCMPIStatus (rc));
  }

  PROV_LOG ("Calling CMGetCIMStatusCode with NULL handle");
  outCIMStatusCode = CMGetCIMStatusCode(cmpiError, &rc);
  if (!outCIMStatusCode)
  {
      PROV_LOG ("++++ (%s) CMGetCIMStatusCode (%d)", strCMPIStatus (rc),
          outCIMStatusCode);
  }

  PROV_LOG ("Calling CMGetCIMStatusCodeDescription with NULL handle");
  outCIMStatusCodeDesc = CMGetCIMStatusCodeDescription(cmpiError, &rc);
  if (!outCIMStatusCodeDesc)
  {
      PROV_LOG ("++++ (%s) CMGetCIMStatusCodeDescription", strCMPIStatus (rc));
  }

  PROV_LOG ("Calling CMGetMessageArguments with NULL handle");
  outMsgArgs = CMGetMessageArguments(cmpiError, &rc);
  if (!outMsgArgs)
  {
      PROV_LOG ("++++ (%s) CMGetMessageArguments", strCMPIStatus (rc));
  }

  PROV_LOG ("Calling CMSetErrorType with NULL handle");
  rc = CMSetErrorType(cmpiError, inErrType);
  PROV_LOG ("++++ (%s) CMSetErrorType", strCMPIStatus (rc));

  PROV_LOG ("Calling CMSetOtherErrorType with NULL handle");
  rc = CMSetOtherErrorType(cmpiError, inOtherErrType);
  PROV_LOG ("++++ (%s) CMSetOtherErrorType", strCMPIStatus (rc));

  PROV_LOG ("Calling CMSetProbableCauseDescription with NULL handle");
  rc = CMSetProbableCauseDescription(cmpiError, inPcd);
  PROV_LOG ("++++ (%s) CMSetProbableCauseDescription", strCMPIStatus (rc));

  PROV_LOG ("Calling CMSetRecommendedActions with NULL handle");
  rc = CMSetRecommendedActions(cmpiError, inRecActions);
  PROV_LOG ("++++ (%s) CMSetRecommendedActions", strCMPIStatus (rc));

  PROV_LOG ("Calling CMSetErrorSource with NULL handle");
  rc = CMSetErrorSource(cmpiError, inErrSource);
  PROV_LOG ("++++ (%s) CMSetErrorSource", strCMPIStatus (rc));

  PROV_LOG ("Calling CMSetErrorSourceFormat with NULL handle");
  rc = CMSetErrorSourceFormat(cmpiError, inErrSourceFormat);
  PROV_LOG ("++++ (%s) CMSetErrorSourceFormat", strCMPIStatus (rc));

  PROV_LOG ("Calling CMSetOtherErrorSourceFormat with NULL handle");
  rc = CMSetOtherErrorSourceFormat(cmpiError, inOtherErrSourceFormat);
  PROV_LOG ("++++ (%s) CMSetOtherErrorSourceFormat", strCMPIStatus (rc));

  PROV_LOG ("Calling CMSetCIMStatusCodeDescription with NULL handle");
  rc = CMSetCIMStatusCodeDescription(cmpiError, inCIMStatusCodeDesc);
  PROV_LOG ("++++ (%s) CMSetCIMStatusCodeDescription", strCMPIStatus (rc));

  PROV_LOG ("Calling CMSetMessageArguments with NULL handle");
  rc = CMSetMessageArguments(cmpiError, inMsgArgs);
  PROV_LOG ("++++ (%s) CMSetMessageArguments", strCMPIStatus (rc));
  cmpiError->hdl = handle;
  PROV_LOG_CLOSE();
  // Test case to increase coverage in CMPI_Broker.cpp
  objPath = CMNewObjectPath (_broker,
      "test/TestProvider",
      "TestCMPI_Method",
      &rc);
  retData = CBInvokeMethod(_broker,
      ctx,
      objPath,
      "testReturn",
      in,
      out,
      &rc);
  PROV_LOG_OPEN (_ClassName, _ProviderLocation);

  if(retData.value.uint32 == 2 && rc.rc == CMPI_RC_OK)
  {
      PROV_LOG ("++++ (%s) CMInvokeMethod", strCMPIStatus (rc));
  }
  PROV_LOG ("--- %s CMPI InvokeMethod() exited with CMPI_RC_ERR_FAILED",
      _ClassName);
  PROV_LOG_CLOSE();
  CMReturnWithString(inCIMStatusCode,
      CMNewString(_broker, "TestError invokeMethod() expected failure", NULL));
}

/* ---------------------------------------------------------------------------*/
/*                              Provider Factory                              */
/* ---------------------------------------------------------------------------*/


CMMethodMIStub (TestCMPIErrorProvider,
                TestCMPIErrorProvider, _broker, CMNoHook)

/* ---------------------------------------------------------------------------*/
/*             end of TestCMPIProvider                      */
/* ---------------------------------------------------------------------------*/
