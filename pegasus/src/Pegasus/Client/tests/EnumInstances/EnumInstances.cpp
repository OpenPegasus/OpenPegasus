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
#include <Pegasus/Client/CIMClient.h>

PEGASUS_USING_PEGASUS;
PEGASUS_USING_STD;

const String NAMESPACE = "root/SampleProvider";
const CIMObjectPath INSTANCE1("Sample_InstanceProviderClass.Identifier=1");
const CIMObjectPath INSTANCE2("Sample_InstanceProviderClass.Identifier=2");
const CIMObjectPath INSTANCE3("Sample_InstanceProviderClass.Identifier=3");
const String CLASSNAME = "Sample_InstanceProviderClass";

int main()
{
    try
    {
        CIMClient client;
        client.connectLocal();

        Boolean deepInheritance = true;
        Boolean localOnly = true;
        Boolean includeQualifiers = false;
        Boolean includeClassOrigin = false;
        Array<CIMInstance> cimNInstances = client.enumerateInstances(
            NAMESPACE,
            CLASSNAME,
            deepInheritance,
            localOnly,
            includeQualifiers,
            includeClassOrigin);

        PEGASUS_TEST_ASSERT(cimNInstances.size() == 3);
        for (Uint32 i = 0; i < cimNInstances.size(); i++)
        {
            if (!((cimNInstances[i].getPath() == INSTANCE1) ||
                  (cimNInstances[i].getPath() == INSTANCE2) ||
                  (cimNInstances[i].getPath() == INSTANCE3)))
            {
                cerr << "Error: EnumInstances failed" << endl;
                exit(1);
            }
        }
    }
    catch (Exception& e)
    {
        cerr << "Error: " << e.getMessage() << endl;
        exit(1);
    }

    cout << "EnumInstances +++++ passed all tests" << endl;
    return 0;
}
