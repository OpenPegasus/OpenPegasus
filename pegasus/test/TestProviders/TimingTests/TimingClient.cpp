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
#include <unistd.h>
#include <sys/types.h>
#include <Pegasus/Common/PegasusAssert.h>
#include <Pegasus/Common/Config.h>
#include <Pegasus/Client/CIMClient.h>
#include <Pegasus/Common/Constants.h>
#include <Pegasus/Common/CIMName.h>
#include <Pegasus/Common/XmlWriter.h>
#include <Pegasus/General/MofWriter.h>
#include <Pegasus/General/OptionManager.h>

PEGASUS_USING_PEGASUS;
PEGASUS_USING_STD;

const CIMNamespaceName NAMESPACE = CIMNamespaceName ("root/SampleProvider");

const String CLASSONE = "TimeOne";
const String CLASSTWO = "TimeTwo";

static void EnumerateClassesTiming(CIMClient client, String BaseClassName)
{
    try
    {
        Boolean deepInheritance = true;
        CIMName className;
        Array<CIMName> classNames = client.enumerateClassNames(
                     NAMESPACE, CIMName(BaseClassName), false);
        cout << classNames.size() << " ClassNames" << endl;
        for (Uint32 i = 0, n = classNames.size(); i < n; i++)
            cout << classNames[i] << endl;
    }
    catch(Exception& e)
    {
        cout << "Error Classes Enumeration:" << endl;
        cout << e.getMessage() << endl;
    }
}

static void EnumerateInstancesTiming(CIMClient client, String ClassName)
{
    Boolean localOnly = false;
    Boolean deepInheritance = false;
    Boolean includeQualifiers = false;
    Boolean includeClassOrigin = false;

    CIMClass c1=client.getClass(NAMESPACE,CIMName(ClassName), false);
    cout << "Class = " << c1.getClassName() << endl;

    cout << "Fetching Instances for" << c1.getClassName() << endl;

    Array<CIMInstance> instanceNames;
    instanceNames = client.enumerateInstances
                        (NAMESPACE,
                         ClassName,
                         deepInheritance,
                         localOnly,
                         includeQualifiers,
                         includeClassOrigin);

    cout << "instanceNames.size() = " << instanceNames.size() << endl;
    for (Uint32 i = 0; i < instanceNames.size(); i++)
    {
        CIMInstance inst1 = instanceNames[i];
        XmlWriter::printInstanceElement(inst1, cout);
    }
}

int main(int argc, char** argv)
{
    pid_t pid;
    try
    {
        CIMClient client;

        client.connect("localhost", 5988, "", "");

        if ((pid = fork()) < 0)
        {
                cout << "Fork Error\n" << endl;
                exit(0);
        }
        else if (pid == 0)
        {   // child
            sleep(10);
            EnumerateInstancesTiming(client, CLASSONE);
            _exit(0);
        }
        // parent
        sleep(10);
        EnumerateInstancesTiming(client, CLASSTWO);
        sleep(5);
        EnumerateClassesTiming(client, "TimingSampleClass");
    }
    catch(Exception& e)
    {
        PEGASUS_STD(cerr) << "Error: " << e.getMessage() <<
            PEGASUS_STD(endl);
        exit(1);
    }
    return 0;
}
