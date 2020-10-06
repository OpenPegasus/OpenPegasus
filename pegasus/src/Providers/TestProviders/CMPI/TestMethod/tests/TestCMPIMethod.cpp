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

#include <Pegasus/Common/Config.h>
#include <Pegasus/Common/Constants.h>
#include <Pegasus/Common/CIMType.h>
#include <Pegasus/Common/CIMName.h>
#include <Pegasus/Common/CIMInstance.h>
#include <Pegasus/Common/CIMProperty.h>
#include <Pegasus/Common/CIMValue.h>
#include <Pegasus/Common/System.h>
#include <Pegasus/Common/InternalException.h>
#include <Pegasus/Common/AcceptLanguageList.h>
#include <Pegasus/Common/ContentLanguageList.h>
#include <Pegasus/Client/CIMClient.h>

PEGASUS_USING_PEGASUS;
PEGASUS_USING_STD;


CIMNamespaceName providerNamespace;
const CIMName CLASSNAME = CIMName ("TestCMPI_Method");

Boolean verbose;
Boolean useDefaultMsg;


void _checkStringValue
  (CIMValue & theValue, const String & value, Boolean null = false)
{
  PEGASUS_TEST_ASSERT (theValue.getType () == CIMTYPE_STRING);
  PEGASUS_TEST_ASSERT (!theValue.isArray ());
  if (null)
    {
      PEGASUS_TEST_ASSERT (theValue.isNull ());
    }
  else
    {
      PEGASUS_TEST_ASSERT (!theValue.isNull ());
      String result;
      theValue.get (result);

      if (verbose)
    {
      if (result != value)
        {
          cerr << "Property value comparison failed.  ";
          cerr << "Expected " << value << "; ";
          cerr << "Actual property value was " << result << "." << endl;
        }
    }

      PEGASUS_TEST_ASSERT (result == value);
    }
}


void
_checkUint32Value (CIMValue & theValue, Uint32 value)
{

  PEGASUS_TEST_ASSERT (theValue.getType () == CIMTYPE_UINT32);
  PEGASUS_TEST_ASSERT (!theValue.isArray ());
  PEGASUS_TEST_ASSERT (!theValue.isNull ());

  Uint32 result;
  theValue.get (result);

  if (verbose)
    {
      if (result != value)
    {
      cerr << "Property value comparison failed.  ";
      cerr << "Expected " << value << "; ";
      cerr << "Actual property value was " << result << "." << endl;
    }
    }

  PEGASUS_TEST_ASSERT (result == value);
}




void
_usage ()
{
  cerr << "Usage: TestCMPIMethod {test} {namespace}" << endl;
}

void
test01 (CIMClient & client)
{
  CIMObjectPath instanceName;

  instanceName.setNameSpace (providerNamespace);
  instanceName.setClassName (CLASSNAME);

  Array < CIMParamValue > inParams;
  Array < CIMParamValue > outParams;

  /*     String returnUint32(); */

  CIMValue retValue = client.invokeMethod (providerNamespace,
                       instanceName,
                       "returnUint32",
                       inParams,
                       outParams);
  _checkUint32Value (retValue, 42);
}

void
test02 (CIMClient & client)
{
  CIMObjectPath instanceName;

  instanceName.setNameSpace (providerNamespace);
  instanceName.setClassName (CLASSNAME);

  Array < CIMParamValue > inParams;
  Array < CIMParamValue > outParams;

  /*     String returnString(); */

  CIMValue retValue = client.invokeMethod (providerNamespace,
                       instanceName,
                       "returnString",
                       inParams,
                       outParams);

  _checkStringValue (retValue, "Returning string", false);

}

void
test03 (CIMClient & client)
{
  CIMObjectPath instanceName;

  instanceName.setNameSpace (providerNamespace);
  instanceName.setClassName (CLASSNAME);

  Array < CIMParamValue > inParams;
  Array < CIMParamValue > outParams;

  /*     uint32 TestCMPIBroker (
   *          [IN, Description (
   *          "The requested are to test different
   *          CMPI data structure operations."),
   *          ValueMap { "1", "2", "3", "4", "5", "6"},
   *          Values {"CDGetType", "CDToString", "CDIsOfType",
   *                  "CMGetMessage",  "CMLogMessage","CDTraceMessage"}]
   *          uint32 Operation,
   *          [OUT, Description (
   *          " The result of what the operation carried out.")]
   *          string Result);
   *
   */
  {
    /* CDGetType */
    inParams.append (CIMParamValue ("Operation", CIMValue (Uint32 (1))));
    CIMValue retValue = client.invokeMethod (providerNamespace,
                         instanceName,
                         "TestCMPIBroker",
                         inParams,
                         outParams);
    // Check the return value. Make sure it is 0.
    _checkUint32Value (retValue, 0);

    PEGASUS_TEST_ASSERT (outParams.size () == 1);
    CIMValue paramValue = outParams[0].getValue ();
    _checkStringValue (paramValue, "CMPIArgs");
  }
  inParams.clear ();
  outParams.clear ();
  {
    /* CDToString */
    inParams.append (CIMParamValue ("Operation", CIMValue (Uint32 (2))));
    CIMValue retValue = client.invokeMethod (providerNamespace,
                         instanceName,
                         "TestCMPIBroker",
                         inParams,
                         outParams);
    // Check the return value. Make sure it is 0.
    _checkUint32Value (retValue, 0);

    PEGASUS_TEST_ASSERT (outParams.size () == 1);
    CIMValue paramValue = outParams[0].getValue ();
    _checkStringValue (paramValue, " Operation:2\n");
  }
  inParams.clear ();
  outParams.clear ();
  {
    /* CDIsOfType */
    inParams.append (CIMParamValue ("Operation", CIMValue (Uint32 (3))));
    CIMValue retValue = client.invokeMethod (providerNamespace,
                         instanceName,
                         "TestCMPIBroker",
                         inParams,
                         outParams);
    // Check the return value. Make sure it is 0.
    _checkUint32Value (retValue, 0);

    PEGASUS_TEST_ASSERT (outParams.size () == 1);
    CIMValue paramValue = outParams[0].getValue ();
    _checkStringValue (paramValue, "++++ CMPIArgs = Yes");
  }
  inParams.clear ();
  outParams.clear ();
  {
    /* CMGetMessage */
    inParams.append (CIMParamValue ("Operation", CIMValue (Uint32 (4))));
    CIMValue retValue = client.invokeMethod (providerNamespace,
                         instanceName,
                         "TestCMPIBroker",
                         inParams,
                         outParams);
    // Check the return value. Make sure it is 0.
    _checkUint32Value (retValue, 0);

    PEGASUS_TEST_ASSERT (outParams.size () == 1);
    CIMValue paramValue = outParams[0].getValue ();
    // If PEGASUS_USE_DEFAULT_MESSAGES is not set, we can't make an
    // assumption about what the value of the msg will be.
    if (useDefaultMsg)
    {
        _checkStringValue (paramValue, "CIM_ERR_SUCCESS: Successful.");
    }
  }
  inParams.clear ();
  outParams.clear ();
  {
    /* CMLogMessage */
    inParams.append (CIMParamValue ("Operation", CIMValue (Uint32 (5))));
    CIMValue retValue = client.invokeMethod (providerNamespace,
                         instanceName,
                         "TestCMPIBroker",
                         inParams,
                         outParams);
    // Check the return value. Make sure it is 0.
    _checkUint32Value (retValue, 0);
    // Nothing is returned
    PEGASUS_TEST_ASSERT (outParams.size () == 1);

    CIMValue paramValue = outParams[0].getValue ();
    PEGASUS_TEST_ASSERT (paramValue.isNull ());

  }
  inParams.clear ();
  outParams.clear ();
  {
    /* CMTraceMessage */
    inParams.append (CIMParamValue ("Operation", CIMValue (Uint32 (6))));
    CIMValue retValue = client.invokeMethod (providerNamespace,
                         instanceName,
                         "TestCMPIBroker",
                         inParams,
                         outParams);
    // Check the return value. Make sure it is 0.
    _checkUint32Value (retValue, 0);
    // Nothing is returned
    PEGASUS_TEST_ASSERT (outParams.size () == 1);

    CIMValue paramValue = outParams[0].getValue ();
    PEGASUS_TEST_ASSERT (paramValue.isNull ());

  }
  inParams.clear ();
  outParams.clear ();
  {
//       CMGetMessage2
      inParams.append(CIMParamValue(
                          "Operation",
                          CIMValue(Uint32(7))));
      inParams.append(CIMParamValue(
                          "msgFile",
                          CIMValue(String("test/pegasusTest"))));
      inParams.append(CIMParamValue(
                          "msgId",
                          CIMValue(String("CIMStatusCode.CIM_ERR_SUCCESS"))));
      inParams.append(CIMParamValue(
                          "insert1",
                          CIMValue(String("rab oof is foo bar backwards"))));
      inParams.append(CIMParamValue(
                          "insert2",
                          CIMValue(Uint32(64001))));

      AcceptLanguageList accLangs;
      accLangs.insert(LanguageTag("en-US"),Real32(1.0));
      accLangs.insert(LanguageTag("fr"),Real32(0.8));
      client.setRequestAcceptLanguages(accLangs);

      CIMValue retValue = client.invokeMethod (providerNamespace,
          instanceName,
          "TestCMPIBroker",
          inParams,
          outParams);
      // Check the return value. Make sure it is 0.
      _checkUint32Value (retValue, 0);

      PEGASUS_TEST_ASSERT (outParams.size () == 1);
      CIMValue paramValue = outParams[0].getValue ();
      // If PEGASUS_USE_DEFAULT_MESSAGES is not set, we can't make an
      // assumption about what the value of the msg will be, or the
      // ContentLanguage.
#ifdef PEGASUS_HAS_MESSAGES
      if (useDefaultMsg)
      {
#endif
          _checkStringValue (paramValue, "CIM_ERR_SUCCESS: Successful.");
#ifdef PEGASUS_HAS_MESSAGES
      }
      else
      {
          ContentLanguageList contLangs;
          contLangs = client.getResponseContentLanguages();
          cout << "ContentLanguage size == " << contLangs.size() << endl;
          PEGASUS_TEST_ASSERT(contLangs.size() == 1);
          cout << "ContentLanguage == "
               << contLangs.getLanguageTag(0).toString();
          PEGASUS_TEST_ASSERT(
              contLangs.getLanguageTag(0).toString() == "en-US");
      }
#endif
      // Reset client
      accLangs.clear();
      client.setRequestAcceptLanguages(accLangs);
  }


    inParams.clear();
    outParams.clear();
    {
        // _testCMPIEnumeration
        inParams.append (CIMParamValue ("Operation", CIMValue (Uint32 (8))));
        CIMValue retValue = client.invokeMethod (providerNamespace,
                                                 instanceName,
                                                 "TestCMPIBroker",
                                                 inParams,
                                                 outParams);
        // Check the return value. Make sure it is 0.
        _checkUint32Value (retValue, 0);

        PEGASUS_TEST_ASSERT (outParams.size () == 1);
        CIMValue paramValue = outParams[0].getValue ();
        PEGASUS_TEST_ASSERT (paramValue.isNull ());
    }


    inParams.clear();
    outParams.clear();
    {
        // _testCMPIArray
        inParams.append (CIMParamValue ("Operation", CIMValue (Uint32 (9))));
        CIMValue retValue = client.invokeMethod (providerNamespace,
                                                 instanceName,
                                                 "TestCMPIBroker",
                                                 inParams,
                                                 outParams);
        // Check the return value. Make sure it is 0.
        _checkUint32Value (retValue, 0);

        PEGASUS_TEST_ASSERT (outParams.size () == 1);
        CIMValue paramValue = outParams[0].getValue ();
        PEGASUS_TEST_ASSERT (paramValue.isNull ());
    }

    inParams.clear();
    outParams.clear();
    {
        // _testCMPIcontext

        inParams.append (CIMParamValue ("Operation", CIMValue (Uint32 (10))));
        CIMValue retValue = client.invokeMethod (providerNamespace,
                             instanceName,
                             "TestCMPIBroker",
                             inParams,
                             outParams);
        // Check the return value. Make sure it is 0.
        _checkUint32Value (retValue, 0);

        PEGASUS_TEST_ASSERT (outParams.size () == 1);
        CIMValue paramValue = outParams[0].getValue ();
        PEGASUS_TEST_ASSERT (paramValue.isNull ());
    }

    inParams.clear();
    outParams.clear();
    {
        // _testCMPIDateTime

        inParams.append (CIMParamValue ("Operation", CIMValue (Uint32 (11))));
        CIMValue retValue = client.invokeMethod (providerNamespace,
                                                 instanceName,
                                                 "TestCMPIBroker",
                                                 inParams,
                                                 outParams);
        // Check the return value. Make sure it is 0.
        _checkUint32Value (retValue, 0);
        // Nothing is returned
        PEGASUS_TEST_ASSERT (outParams.size () == 1);

        CIMValue paramValue = outParams[0].getValue ();
        PEGASUS_TEST_ASSERT (paramValue.isNull ());
    }

    inParams.clear();
    outParams.clear();
    {
      //   _testCMPIInstance

        inParams.append (CIMParamValue ("Operation", CIMValue (Uint32 (12))));

        CIMValue retValue = client.invokeMethod (providerNamespace,
                                                 instanceName,
                                                 "TestCMPIBroker",
                                                 inParams,
                                                 outParams);
        // Check the return value. Make sure it is 0.
        _checkUint32Value (retValue, 0);
        // Nothing is returned
        PEGASUS_TEST_ASSERT (outParams.size () == 1);
        CIMValue paramValue = outParams[0].getValue ();
        PEGASUS_TEST_ASSERT (paramValue.isNull ());
    }

    inParams.clear();
    outParams.clear();
    {
    //     _testCMPIObjectPath

        inParams.append (CIMParamValue ("Operation", CIMValue (Uint32 (13))));
        CIMValue retValue = client.invokeMethod (providerNamespace,
                                                 instanceName,
                                                 "TestCMPIBroker",
                                                 inParams,
                                                 outParams);
        // Check the return value. Make sure it is 0.
        _checkUint32Value (retValue, 0);
        // Nothing is returned
        PEGASUS_TEST_ASSERT (outParams.size () == 1);

        CIMValue paramValue = outParams[0].getValue ();
        PEGASUS_TEST_ASSERT (paramValue.isNull ());
    }

    inParams.clear();
    outParams.clear();
    {
        //  _testCMPIResult

        inParams.append (CIMParamValue ("Operation", CIMValue (Uint32 (14))));
        CIMValue retValue = client.invokeMethod (providerNamespace,
                                                 instanceName,
                                                 "TestCMPIBroker",
                                                 inParams,
                                                 outParams);
        // Check the return value. Make sure it is 0.
        _checkUint32Value (retValue, 0);
        // Nothing is returned
        PEGASUS_TEST_ASSERT (outParams.size () == 1);

        CIMValue paramValue = outParams[0].getValue ();
        PEGASUS_TEST_ASSERT (paramValue.isNull ());
    }

    inParams.clear();
    outParams.clear();
    {
        //  _testCMPIString

        inParams.append (CIMParamValue ("Operation", CIMValue (Uint32 (15))));
        CIMValue retValue = client.invokeMethod (providerNamespace,
                                                 instanceName,
                                                 "TestCMPIBroker",
                                                 inParams,
                                                 outParams);
        // Check the return value. Make sure it is 0.
        _checkUint32Value (retValue, 0);
        // Nothing is returned
        PEGASUS_TEST_ASSERT (outParams.size () == 1);

        CIMValue paramValue = outParams[0].getValue ();
        PEGASUS_TEST_ASSERT (paramValue.isNull ());
    }

    inParams.clear();
    outParams.clear();
    {
        //_testCMPIArgs

        inParams.append (CIMParamValue ("Operation", CIMValue (Uint32 (16))));
        CIMValue retValue = client.invokeMethod (providerNamespace,
                                                 instanceName,
                                                 "TestCMPIBroker",
                                                 inParams,
                                                 outParams);
        //Check the return value. Make sure it is 0.
        _checkUint32Value (retValue, 0);
        // Nothing is returned
        PEGASUS_TEST_ASSERT (outParams.size () == 1);

        CIMValue paramValue = outParams[0].getValue ();
        PEGASUS_TEST_ASSERT (paramValue.isNull ());
    }
    inParams.clear();
    outParams.clear();
    {
         //_testCMPIBroker

        inParams.append (CIMParamValue ("Operation", CIMValue (Uint32 (17))));
        CIMValue retValue = client.invokeMethod (providerNamespace,
                                                 instanceName,
                                                 "TestCMPIBroker",
                                                 inParams,
                                                 outParams);
        //Check the return value. Make sure it is 0.
        _checkUint32Value (retValue, 0);
        // Nothing is returned
        PEGASUS_TEST_ASSERT (outParams.size () == 1);

        CIMValue paramValue = outParams[0].getValue ();

        PEGASUS_TEST_ASSERT (paramValue.isNull ());
    }

}
void
test04 (CIMClient & client)
{
  CIMObjectPath instanceName;

  instanceName.setNameSpace (providerNamespace);
  instanceName.setClassName (CLASSNAME);

  Array < CIMParamValue > inParams;
  Array < CIMParamValue > outParams;

  /*     [EmbeddedObject] String returnInstance(); */

  CIMValue retValue = client.invokeMethod (providerNamespace,
                       instanceName,
                       "returnInstance",
                       inParams,
                       outParams);

  PEGASUS_TEST_ASSERT (retValue.getType () == CIMTYPE_OBJECT);
  PEGASUS_TEST_ASSERT (!retValue.isArray ());
  PEGASUS_TEST_ASSERT (!retValue.isNull ());

  CIMObject result;
  retValue.get (result);

  CIMObjectPath objPath  = result.getPath();
  PEGASUS_TEST_ASSERT (objPath.toString() == "TestCMPI_Instance");

}
void
test05 (CIMClient & client)
{
  CIMObjectPath instanceName;

  instanceName.setNameSpace (providerNamespace);
  instanceName.setClassName (CLASSNAME);

  Array < CIMParamValue > inParams;
  Array < CIMParamValue > outParams;

  /*     String returnDateTime(); */

  CIMValue retValue = client.invokeMethod (providerNamespace,
                       instanceName,
                       "returnDateTime",
                       inParams,
                       outParams);
  PEGASUS_TEST_ASSERT (retValue.getType () == CIMTYPE_DATETIME);
  PEGASUS_TEST_ASSERT (!retValue.isArray ());
  PEGASUS_TEST_ASSERT (!retValue.isNull ());

}
void
test06 (CIMClient & client)
{
  CIMObjectPath instanceName;
  Uint32 exception  =0;
  instanceName.setNameSpace (providerNamespace);
  instanceName.setClassName (CLASSNAME);

  Array < CIMParamValue > inParams;
  Array < CIMParamValue > outParams;

  /*     String returnDateTime(); */
  try {
  CIMValue retValue = client.invokeMethod (providerNamespace,
                       instanceName,
                       "noSuchFunction",
                       inParams,
                       outParams);
  } catch (const CIMException &e)
  {
      exception ++;
    PEGASUS_TEST_ASSERT (e.getCode() == CIM_ERR_NOT_FOUND);
  }
  PEGASUS_TEST_ASSERT (exception == 1);
}
void
test07 (CIMClient & client)
{
  CIMObjectPath instanceName;
  instanceName.setNameSpace (providerNamespace);
  instanceName.setClassName (CLASSNAME);

  Array < CIMParamValue > inParams;
  Array < CIMParamValue > outParams;

    inParams.append (CIMParamValue ("Operation", CIMValue (String ("Boom"))));
  CIMValue retValue = client.invokeMethod (providerNamespace,
                       instanceName,
                       "TestCMPIBroker",
                       inParams,
                       outParams);
    _checkUint32Value (retValue, 1);
}
void
test08 (CIMClient & client)
{
  CIMObjectPath instanceName;
  instanceName.setNameSpace (providerNamespace);
  instanceName.setClassName (CLASSNAME);

  Array < CIMParamValue > inParams;
  Array < CIMParamValue > outParams;

  inParams.append (CIMParamValue ("Operation", CIMValue (Uint64 (1))));
  CIMValue retValue = client.invokeMethod (providerNamespace,
                       instanceName,
                       "TestCMPIBroker",
                       inParams,
                       outParams);
    _checkUint32Value (retValue, 1);
}

/**
 * This tests the embedded instance functionality through the CMPI Test
 * Method Provider. It first invokes the returnInstance() method to retrieve
 * an instance that can be used
 */
void test09 (CIMClient & client)
{
  CIMObjectPath instanceName;

  instanceName.setNameSpace (providerNamespace);
  instanceName.setClassName (CLASSNAME);

  Array < CIMParamValue > inParams;
  Array < CIMParamValue > outParams;

  /*     [EmbeddedObject] String returnInstance(); */

  CIMValue retValue = client.invokeMethod (providerNamespace,
                       instanceName,
                       "returnInstance",
                       inParams,
                       outParams);

  PEGASUS_TEST_ASSERT (retValue.getType () == CIMTYPE_OBJECT);
  PEGASUS_TEST_ASSERT (!retValue.isArray ());
  PEGASUS_TEST_ASSERT (!retValue.isNull ());

  CIMObject result;
  retValue.get (result);

  CIMObjectPath objPath  = result.getPath();

  CIMInstance inputInstance(result);
  CIMInstance outputInstance;

  inParams.append(
      CIMParamValue(String("inputInstance"), CIMValue(inputInstance)));

  retValue = client.invokeMethod (providerNamespace,
      instanceName,
      "processEmbeddedInstance",
      inParams,
      outParams);

  // First test the return value
  PEGASUS_TEST_ASSERT(retValue.getType() == CIMTYPE_INSTANCE);
  PEGASUS_TEST_ASSERT(!retValue.isArray());
  PEGASUS_TEST_ASSERT(!retValue.isNull());
  retValue.get(outputInstance);
  PEGASUS_TEST_ASSERT(objPath.toString() ==
    outputInstance.getPath().toString());
  PEGASUS_TEST_ASSERT(outputInstance.getPropertyCount() ==
    inputInstance.getPropertyCount());
  for(unsigned int i = 0, n = outputInstance.getPropertyCount(); i < n; ++i)
  {
    CIMProperty outputProp(outputInstance.getProperty(i));
    CIMProperty inputProp(inputInstance.getProperty(i));

    PEGASUS_TEST_ASSERT(outputProp.getName() == inputProp.getName());
    PEGASUS_TEST_ASSERT(outputProp.getValue() == inputProp.getValue());
  }

  // Now test the output parameter
  PEGASUS_TEST_ASSERT(outParams.size() == 1);
  retValue = outParams[0].getValue();

  PEGASUS_TEST_ASSERT(retValue.getType() == CIMTYPE_INSTANCE);
  PEGASUS_TEST_ASSERT(!retValue.isArray());
  PEGASUS_TEST_ASSERT(!retValue.isNull());
  retValue.get(outputInstance);
  PEGASUS_TEST_ASSERT(objPath.toString() ==
    outputInstance.getPath().toString());
  PEGASUS_TEST_ASSERT(outputInstance.getPropertyCount() ==
    inputInstance.getPropertyCount());
  for(unsigned int i = 0, n = outputInstance.getPropertyCount(); i < n; ++i)
  {
    CIMProperty outputProp(outputInstance.getProperty(i));
    CIMProperty inputProp(inputInstance.getProperty(i));

    cout << "output " << outputProp.getName().getString()
         << "\ninput  " << inputProp.getName().getString() << endl;
    PEGASUS_TEST_ASSERT(outputProp.getName() == inputProp.getName());
    PEGASUS_TEST_ASSERT(outputProp.getValue() == inputProp.getValue());
  }
}

void test10 (CIMClient &client)
{
    CIMObjectPath instanceName;
    CIMValue output;
    instanceName.setNameSpace (providerNamespace);
    instanceName.setClassName (CLASSNAME);

    Array < CIMParamValue > inParams;
    Array < CIMParamValue > outParams;

    CIMValue retValue = client.invokeMethod (providerNamespace,
        instanceName,
        "testArrayTypes",
        inParams,
        outParams);

    _checkUint32Value (retValue, 1);
}

void test11 (CIMClient &client)
  {
    CIMObjectPath instanceName;
    CIMValue output;
    instanceName.setNameSpace (providerNamespace);
    instanceName.setClassName (CLASSNAME);

    Array < CIMParamValue > inParams;
    Array < CIMParamValue > outParams;

    CIMValue retValue = client.invokeMethod (providerNamespace,
        instanceName,
        "testErrorPaths",
        inParams,
        outParams);

    _checkUint32Value (retValue, 1);
}

void test12 (CIMClient &client)
{
    CIMObjectPath instanceName;
    CIMValue output;
    instanceName.setNameSpace (providerNamespace);
    instanceName.setClassName (CLASSNAME);

    Array < CIMParamValue > inParams;
    Array < CIMParamValue > outParams;

    CIMValue retValue = client.invokeMethod (providerNamespace,
        instanceName,
        "testSimpleTypes",
        inParams,
        outParams);

    _checkUint32Value (retValue, 1);
}

void test13 (CIMClient &client)
{
    CIMObjectPath instanceName;
    CIMValue output;
    instanceName.setNameSpace (providerNamespace);
    instanceName.setClassName (CLASSNAME);

    Array < CIMParamValue > inParams;
    Array < CIMParamValue > outParams;

    CIMValue retValue = client.invokeMethod (providerNamespace,
        instanceName,
        "testArrayClone",
        inParams,
        outParams);

    _checkUint32Value (retValue, 1);
}

void test14 (CIMClient & client)
{
    CIMObjectPath instanceName;

    instanceName.setNameSpace (providerNamespace);
    instanceName.setClassName (CLASSNAME);

    Array < CIMParamValue > inParams;
    Array < CIMInstance> eObjs;
    Array < CIMParamValue > outParams;

    CIMValue retValue = client.invokeMethod (providerNamespace,
        instanceName,
        "returnInstance",
        inParams,
        outParams);

    PEGASUS_TEST_ASSERT (retValue.getType () == CIMTYPE_OBJECT);
    PEGASUS_TEST_ASSERT (!retValue.isArray ());
    PEGASUS_TEST_ASSERT (!retValue.isNull ());

    CIMObject result;
    retValue.get (result);
    CIMObjectPath objPath  = result.getPath();
    CIMInstance inputInstance(result);
    CIMInstance outputInstance;
    eObjs.append(inputInstance);
    eObjs.append(inputInstance);
    eObjs.append(inputInstance);

    inParams.append (
        CIMParamValue(String("inputInstances"), CIMValue(eObjs)));

    retValue = client.invokeMethod (providerNamespace,
        instanceName,
        "processArrayEmbeddedInstance",
        inParams,
        outParams);

    // First test the return value
    PEGASUS_TEST_ASSERT(retValue.getType() == CIMTYPE_INSTANCE);
    PEGASUS_TEST_ASSERT(!retValue.isArray());
    PEGASUS_TEST_ASSERT(!retValue.isNull());
    retValue.get(outputInstance);
    PEGASUS_TEST_ASSERT(objPath.toString() ==
        outputInstance.getPath().toString());
    PEGASUS_TEST_ASSERT(outputInstance.getPropertyCount() ==
        inputInstance.getPropertyCount());

    for(unsigned int i = 0, n = outputInstance.getPropertyCount(); i < n; ++i)
    {
        CIMProperty outputProp(outputInstance.getProperty(i));
        CIMProperty inputProp(inputInstance.getProperty(i));

        PEGASUS_TEST_ASSERT(outputProp.getName() == inputProp.getName());
        PEGASUS_TEST_ASSERT(outputProp.getValue() == inputProp.getValue());
    }

    // Now test the output parameters
    PEGASUS_TEST_ASSERT(outParams.size() == 3);
    CIMValue outParamValue = outParams[0].getValue();
    PEGASUS_TEST_ASSERT(outParamValue.isArray());
    PEGASUS_TEST_ASSERT(!outParamValue.isNull());

    Array<CIMInstance> instances;
    outParamValue.get(instances);

    for (unsigned int j = 0; j < instances.size () ; ++j)
    {
        outputInstance = instances[j];
        PEGASUS_TEST_ASSERT(objPath.toString() ==
            outputInstance.getPath().toString());
        PEGASUS_TEST_ASSERT(outputInstance.getPropertyCount() ==
            eObjs[j].getPropertyCount());
        for(unsigned int i = 0, n = outputInstance.getPropertyCount();
            i < n; ++i)
        {
            CIMProperty outputProp(outputInstance.getProperty(i));
            CIMProperty inputProp(eObjs[j].getProperty(i));
            PEGASUS_TEST_ASSERT(outputProp.getName() == inputProp.getName());
            PEGASUS_TEST_ASSERT(outputProp.getValue() == inputProp.getValue());
        }
    }

    outParamValue = outParams[1].getValue();
    PEGASUS_TEST_ASSERT(outParamValue.isArray());
    PEGASUS_TEST_ASSERT(!outParamValue.isNull());

    Array<CIMObject> objs;
    outParamValue.get(objs);

    for (unsigned int j = 0; j < objs.size () ; ++j)
    {
        outputInstance = CIMInstance(objs[j]);
        PEGASUS_TEST_ASSERT(objPath.toString() ==
            outputInstance.getPath().toString());
        PEGASUS_TEST_ASSERT(outputInstance.getPropertyCount() ==
            eObjs[j].getPropertyCount());
        for(unsigned int i = 0, n = outputInstance.getPropertyCount();
            i < n; ++i)
        {
            CIMProperty outputProp(outputInstance.getProperty(i));
            CIMProperty inputProp(eObjs[j].getProperty(i));
            PEGASUS_TEST_ASSERT(outputProp.getName() == inputProp.getName());
            PEGASUS_TEST_ASSERT(outputProp.getValue() == inputProp.getValue());
        }
    }

    outParamValue = outParams[2].getValue();
    PEGASUS_TEST_ASSERT(outParamValue.isArray());
    PEGASUS_TEST_ASSERT(!outParamValue.isNull());

    outParamValue.get(objs);

    for (Uint32 j = 0, m = objs.size(); j < m ; ++j)
    {
        outputInstance = CIMInstance(objs[j]);
        Uint32 id;
        CIMInstance emInstance;
        CIMObject emObject;
        outputInstance.getProperty(
            outputInstance.findProperty("id")).getValue().get(id);
        outputInstance.getProperty(
            outputInstance.findProperty("emInstance")).
                getValue().get(emInstance);
        outputInstance.getProperty(
            outputInstance.findProperty("emObject")).getValue().get(emObject);
        PEGASUS_TEST_ASSERT(eObjs[j].identical(emInstance));
        PEGASUS_TEST_ASSERT(eObjs[j].identical(CIMInstance(emObject)));
        PEGASUS_TEST_ASSERT(id == j+1);
    }

}

/*Test Case to test CMPIProviderManager when method is not declared in mof*/
void test15 (CIMClient & client)
{
    CIMObjectPath instanceName;

    instanceName.setNameSpace (providerNamespace);
    instanceName.setClassName (CLASSNAME);

    Array < CIMParamValue > inParams;
    Array < CIMParamValue > outParams;

    CIMValue retValue = client.invokeMethod (providerNamespace,
        instanceName,
        "methodNotInMof",
        inParams,
        outParams);
    _checkUint32Value (retValue, 42);
}

void _test (CIMClient & client)
{
  try
  {
    test01 (client);
    test02 (client);
    test03 (client);
    test04 (client);
    test05 (client);
    test06 (client);
    test07 (client);
    test08 (client);
    test10 (client);
    test11 (client);
    test12 (client);
    test13 (client);
    test15 (client);
    // Don't run against the remote-namespace, not yet suppoted.
    if (providerNamespace == "test/TestProvider")
    {
        test09 (client); // Embedded Instance Test
        test14(client); // Embedded Instance Array Test
    }
  }
  catch (Exception & e)
  {
    cerr << "test failed: " << e.getMessage () << endl;
    exit (-1);
  }
}


int
main (int argc, char **argv)
{
  verbose = (getenv ("PEGASUS_TEST_VERBOSE")) ? true : false;
  useDefaultMsg = (getenv("PEGASUS_USE_DEFAULT_MESSAGES")) ? true : false;
  CIMClient client;
  try
  {
    client.connectLocal ();
  }
  catch (Exception & e)
  {
    cerr << e.getMessage () << endl;
    return -1;
  }

  if (argc != 3)
    {
      _usage ();
      return 1;
    }

  else
    {
      const char *opt = argv[1];

      if (String::equalNoCase (opt, "test"))
    {
          providerNamespace = CIMNamespaceName (argv[2]);
      _test (client);
    }
      else
    {
      cerr << "Invalid option: " << opt << endl;
      _usage ();
      return -1;
    }
    }

  cout << argv[0] << " +++++ completed" << endl;

  return 0;
}
