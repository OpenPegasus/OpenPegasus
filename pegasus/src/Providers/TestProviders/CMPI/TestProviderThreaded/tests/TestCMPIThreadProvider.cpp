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
#include <Pegasus/Client/CIMClient.h>

PEGASUS_USING_PEGASUS;
PEGASUS_USING_STD;

CIMNamespaceName providerNamespace;

Boolean verbose;

void
_usage ()
{
  cerr << "Usage: TestCMPIThreadProvider {test} {namespace}" << endl;
}

void
test01( CIMClient & client)
{
  try {
   Array<CIMInstance> instances = client.enumerateInstances(
           providerNamespace,
        CIMName("TestCMPI_Thread"));
  } catch (const CIMException &e)
  {
    // The provided is ONLY suppose to return not supported.
    if (e.getCode() != CIM_ERR_NOT_SUPPORTED)
        throw e;
  }
}

static void
test02 (CIMClient & client)
{
  Uint32 exceptions = 0;
  CIMObjectPath instanceName;
  Array < CIMKeyBinding > keyBindings;

  keyBindings.append (CIMKeyBinding ("ElementNameName",
                                     "TestCMPI_Thread",
                                     CIMKeyBinding::STRING));

  instanceName.setNameSpace (providerNamespace);
  instanceName.setClassName ("TestCMPI_Thread");
  instanceName.setKeyBindings (keyBindings);

  /* Call the unsupported functions of the provider. */
  try
  {
    CIMInstance instance (client.getInstance (providerNamespace,
                                              instanceName));
  } catch (const CIMException &)
  {
     exceptions ++;
  }


  try
  {
    client.deleteInstance (providerNamespace, instanceName);

  } catch (const CIMException & )
  {
     exceptions ++;
  }
  CIMInstance newInstance ("TestCMPI_Thread");
  newInstance.setPath (instanceName);
  try
  {

    CIMObjectPath objectPath (client.createInstance (providerNamespace,
                                                     newInstance));


  } catch (const CIMException &)
  {
     exceptions ++;
  }

  try
  {
    client.modifyInstance (providerNamespace, newInstance);

  } catch (const CIMException &)
  {
     exceptions ++;
  }
  try
  {

    Array < CIMInstance > instances =
      client.enumerateInstances (providerNamespace,
                                 CIMName ("TestCMPI_Thread"));
  } catch (const CIMException &)
  {
     exceptions ++;
  }

  try
  {
    Array < CIMObjectPath > objectPaths =
      client.enumerateInstanceNames (providerNamespace,
                                     CIMName ("TestCMPI_Thread"));
  } catch (const CIMException &)
  {
     exceptions ++;
  }

  PEGASUS_TEST_ASSERT(exceptions ==  6);

}

void
executeMethod (CIMClient & client, String operation)
{

  CIMObjectPath instanceName;
  Array<CIMKeyBinding> keyBindings;
  Array<CIMParamValue> inParm;
  Array < CIMParamValue > outParm;

  keyBindings.append (CIMKeyBinding ("Name",
    "TestCMPIThreadProviderModule", CIMKeyBinding::STRING));

  instanceName.setNameSpace (PEGASUS_NAMESPACENAME_INTEROP);
  instanceName.setClassName ("PG_ProviderModule");
  instanceName.setKeyBindings(keyBindings);

  CIMValue retVal  = client.invokeMethod(
                        PEGASUS_NAMESPACENAME_INTEROP.getString(),
                        instanceName,
                        operation,
                        inParm,
                        outParm);
}
void
_test (CIMClient & client)
{
  try
  {
    test01(client);
    test02(client);
    executeMethod(client, "stop");
    executeMethod(client, "start");
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

  cout << argv[0] << " +++++ passed all tests" << endl;

  return 0;
}
