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
#include <Pegasus/Common/String.h>
//#include <Pegasus/Compiler/cimmofParser.h>
#include <Pegasus/Repository/CIMRepository.h>

PEGASUS_USING_PEGASUS;
PEGASUS_USING_STD;

// #define DO_NOT_DELETE_INSTANCE
static Boolean verbose;

void TestInstanceAliases(CIMRepository& r)
{

    if (verbose)
        cout << "TestInstanceAliases Starting" << endl;

    CIMObjectPath instanceName_JohnSmith =
        CIMObjectPath ("X.key1=\"John Smith\",key2=\"father\"");

    CIMObjectPath instanceName_JohnJones =
        CIMObjectPath("/root:Y.keya=\"John Jones\",keyb=\"Grandfather\"");

    CIMObjectPath instanceName_Assoc = CIMObjectPath ("/root:A."
            "left=\"X.key1=\\\"John Smith\\\",key2=\\\"father\\\"\","
            "right=\"Y.keya=\\\"John Jones\\\",keyb=\\\"Grandfather\\\"\"" );

    String nameSpace = "root";
    {
        Array<CIMObjectPath> names = r.associatorNames(
                nameSpace,
                instanceName_JohnSmith,
                CIMName ("A"),
                CIMName ("Y"),
                "left",
                "right");

        if (verbose)
            cout << "names.size() = " << names.size() << endl;

        PEGASUS_TEST_ASSERT(names.size() == 1);

        if (verbose)
            cout << "names[0] = " << names[0].toString() << endl << endl;

        names[0].setHost(String::EMPTY);

        PEGASUS_TEST_ASSERT(names[0] == instanceName_JohnJones);
    }

    {
        Array<CIMObject> result = r.associators(
                nameSpace,
                instanceName_JohnSmith,
                CIMName ("a"),
                CIMName ("y"),
                "LEFT",
                "RIGHT");

        if (verbose)
            cout << "result.size() = " << result.size() << endl;

        PEGASUS_TEST_ASSERT(result.size() == 1);

        CIMObjectPath cimReference = result[0].getPath ();
        CIMInstance cimInstance = CIMInstance(result[0]);

        CIMClass tmpClass = r.getClass(nameSpace, cimInstance.getClassName());
        CIMObjectPath tmpInstanceName = cimInstance.buildPath(tmpClass);

        tmpInstanceName.setNameSpace(nameSpace);

        if (verbose)
        {
            cout << "tmpInstanceName = " << tmpInstanceName.toString() << endl;
            cout << "instanceName_JohnJones = "
                 << instanceName_JohnJones.toString() << endl;
            cout << endl;
        }

        PEGASUS_TEST_ASSERT(tmpInstanceName == instanceName_JohnJones);

    }

    {

        Array<CIMObjectPath> result = r.referenceNames(
            nameSpace,
            instanceName_JohnSmith,
            CIMName ("A"),
            "left");

        if (verbose)
            cout << "result.size() = " << result.size() << endl;

        PEGASUS_TEST_ASSERT(result.size() == 1);

        result[0].setHost(String::EMPTY);

        if (verbose)
        {
            cout << "result = " << result[0].toString() << endl;
            cout << "instanceName_Assoc = " << instanceName_Assoc.toString()
                 << endl;
            cout << endl;
        }

        PEGASUS_TEST_ASSERT(result[0] == instanceName_Assoc);
    }

    {
        Array<CIMObject> result = r.references(
                nameSpace,
                instanceName_JohnSmith,
                CIMName ("A"),
                "left");

        if (verbose)
            cout << "result.size() = " << result.size() << endl;

        PEGASUS_TEST_ASSERT(result.size() == 1);

        // Too much output
        // if (verbose)
        //   cout << "result[0] = " << result[0].toString() << "\n\n";

        CIMClass tmpClass = r.getClass(
                nameSpace, CIMInstance(result[0]).getClassName());

        CIMObjectPath tmpInstanceName =
            CIMInstance(result[0]).buildPath(tmpClass);

        tmpInstanceName.setNameSpace(nameSpace);
        if (verbose)
        {
            cout << "tmpInstanceName = " << tmpInstanceName.toString() << endl;
            cout << "instanceName_Assoc = " << instanceName_Assoc.toString()
                 << endl;
            cout << endl;
        }

        PEGASUS_TEST_ASSERT(tmpInstanceName == instanceName_Assoc);
    }

    // test X2 and X2_Association. get the association and compare
    // left and rigt they shold be the same path
    {
        CIMObjectPath X2_Instance = CIMObjectPath ("X2."
            "key1=\"Jimmy Smith\",key2=\"son\",key3=true,key4=999");
        if (verbose)
        {
            cout << "X2 Instance " << X2_Instance.toString() << endl;
        }

        Array<CIMObject> result = r.references(
                nameSpace,
                X2_Instance);

        if (verbose)
            cout << "result.size() = " << result.size() << endl;

        PEGASUS_TEST_ASSERT(result.size() == 1);

        CIMObject refInst = result[0];

        Uint32 pos1 = refInst.findProperty("left");
        Uint32 pos2 = refInst.findProperty("right");
        PEGASUS_TEST_ASSERT(pos1 != PEG_NOT_FOUND);
        PEGASUS_TEST_ASSERT(pos2 != PEG_NOT_FOUND);

        CIMProperty r1 = refInst.getProperty(pos1);
        CIMProperty r2 = refInst.getProperty(pos2);

        // test to confirm paths are the same.

        CIMValue v1 = r1.getValue();
        CIMValue v2 = r2.getValue();
        CIMObjectPath path1;
        CIMObjectPath path2;
        v1.get(path1);
        v2.get(path2);
        if (verbose)
        {
            cout << "X2_Association Paths\n"
                 << path1.toString() << endl
                 << path2.toString() << endl;
        }
        PEGASUS_TEST_ASSERT(path1 == path2);
        CIMInstance inst = r.getInstance(nameSpace, path1);
    }
}

int main(int, char** argv)
{
    String repositoryRoot;

    verbose = (getenv("PEGASUS_TEST_VERBOSE")) ? true : false;
    const char* tmpDir = getenv ("PEGASUS_TMP");
    if (tmpDir == NULL)
    {
        repositoryRoot = ".";
    }
    else
    {
        repositoryRoot = tmpDir;
    }
    repositoryRoot.append("/repository");

    try
    {
        CIMRepository r (repositoryRoot);

        TestInstanceAliases(r);
    }
    catch (Exception& e)
    {
        cerr << e.getMessage() << endl;
        exit(1);
    }

    cout << argv[0] << " +++++ passed all tests" << endl;

    return 0;
}
