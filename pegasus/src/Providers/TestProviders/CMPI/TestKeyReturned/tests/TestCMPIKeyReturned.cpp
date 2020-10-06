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
#include <Pegasus/Common/PegasusAssert.h>
#include <Pegasus/Common/CIMName.h>
#include <Pegasus/Common/CIMNameCast.h>
#include <Pegasus/Common/CIMInstance.h>
#include <Pegasus/Client/CIMClient.h>

PEGASUS_USING_PEGASUS;
PEGASUS_USING_STD;


const CIMNamespaceName PROVIDERNAMESPACE=CIMNamespaceName ("test/TestProvider");
const CIMName CLASSNAME=CIMNameCast("TestCMPI_KeyReturned");

Boolean verbose;

void _checkResult(const Array<CIMInstance> & instArr)
{
    // provider returns exactly six instances
    PEGASUS_TEST_ASSERT(6==instArr.size());

    for (int i=0; i<6;i++)
    {
        const CIMObjectPath& objPath = instArr[i].getPath();
        const Array<CIMKeyBinding>& keyBinds = objPath.getKeyBindings();

        // instance should have three key bindings
        PEGASUS_TEST_ASSERT(3==keyBinds.size());
    }
}

void _test(CIMClient & client)
{
  try
  {
      // empty property list
      Array<CIMInstance> x=
          client.enumerateInstances(PROVIDERNAMESPACE,CLASSNAME);

      _checkResult(x);

      // property list leaving "Name" key out...
      Array<CIMName> n;
      n.append("Number");
      n.append("Flag");
      CIMPropertyList pl(n);

      Array<CIMInstance> y=
          client.enumerateInstances(
              PROVIDERNAMESPACE,
              CLASSNAME,
              true,          // deepInheritance
              true,          // localOnly
              false,         // includeQualifiers
              false,         // includeClassOrigin
              pl);           // property list

      _checkResult(y);
  }
  catch (Exception & e)
  {
    cerr << "test failed: " << e.getMessage () << endl;
    exit(-1);
  }
  cout << "+++++ test completed successfully" << endl;
}


int main()
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
  _test(client);
  return 0;
}
