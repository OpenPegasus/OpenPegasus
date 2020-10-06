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
#include <Pegasus/Repository/XmlStreamer.h>
#include <Pegasus/Common/PegasusAssert.h>
#include <Pegasus/Repository/CIMRepository.h>
#include <Pegasus/Common/XmlWriter.h>

PEGASUS_USING_PEGASUS;
PEGASUS_USING_STD;
Boolean verbose;

String repositoryRoot;
bool trace = false;
size_t failures = 0;

void PutClass(const char* filename, const CIMClass& cimClass)
{
  //    Array<Sint8> out;
    Buffer out;
    XmlStreamer stream;
    stream.encode(out, cimClass);

    FILE* fp = fopen(filename, "wb");
    PEGASUS_TEST_ASSERT(fp != NULL);
    fputs(out.getData(), fp);
    fclose(fp);
}

void CompareClasses(
    CIMRepository& r1,
    CIMRepository& r2,
    const CIMNamespaceName& namespaceName)
{
    Array<CIMName> classNames1 = r1.enumerateClassNames(namespaceName);
    Array<CIMName> classNames2 = r2.enumerateClassNames(namespaceName);
    BubbleSort(classNames1);
    BubbleSort(classNames2);


    PEGASUS_TEST_ASSERT(classNames1 == classNames2);

    for (Uint32 i = 0; i < classNames1.size(); i++)
    {
    CIMClass class1 = r1.getClass(namespaceName, classNames1[i]);
    CIMClass class2 = r2.getClass(namespaceName, classNames2[i]);

    if (verbose)
    {
        cout << "testing class " << namespaceName.getString() << "/";
        cout << classNames1[i].getString() << "..." << endl;
    }

    if (!class1.identical(class2))
    {
        PutClass("file1", class1);
        PutClass("file2", class2);

        cout << "=========================================================";
        cout << "=========================================================";
        cout << endl;
        cout << "ERROR: not identical! - ";


        cout << "ERROR FOUND testing class: " << namespaceName.getString();
        cout << "/";
        cout << classNames1[i].getString();

        cout << " .... differences follow:" << endl << endl;

        if (system("diff file1 file2") == -1)
        {
            cout << "Error:  system(\"diff file1 file2\") failed." << endl;
        }

        if (verbose)
        {
            XmlWriter::printClassElement(class1, cout);
            XmlWriter::printClassElement(class2, cout);
        }
        failures++;
    }
    }
}

void CompareInstances(
    CIMRepository& r1,
    CIMRepository& r2,
    const CIMNamespaceName& namespaceName)
{
    Array<CIMName> classNames1 = r1.enumerateClassNames(namespaceName);
    Array<CIMName> classNames2 = r2.enumerateClassNames(namespaceName);
    BubbleSort(classNames1);
    BubbleSort(classNames2);
    PEGASUS_TEST_ASSERT(classNames1 == classNames2);

    for (Uint32 i = 0; i < classNames1.size(); i++)
    {
    Array<CIMObjectPath> objectPaths1 = r1.enumerateInstanceNamesForClass(
        namespaceName, classNames1[i]);
    Array<CIMObjectPath> objectPaths2 = r2.enumerateInstanceNamesForClass(
        namespaceName, classNames2[i]);
    // BubbleSort(objectPaths1);
    // BubbleSort(objectPaths2);
    PEGASUS_TEST_ASSERT(objectPaths1 == objectPaths2);

    for (Uint32 j = 0; j < objectPaths2.size(); j++)
    {
        CIMInstance inst1 = r1.getInstance(namespaceName, objectPaths1[j]);
        CIMInstance inst2 = r2.getInstance(namespaceName, objectPaths2[j]);

        if (verbose)
        {
        cout << "testing instance " << namespaceName.getString() << "/";
        cout << objectPaths1[j].toString() << "..." << endl;
        }

        PEGASUS_TEST_ASSERT(inst1.identical(inst2));
    }
    }
}

PEGASUS_NAMESPACE_BEGIN

// This operator is needed to allow BubbleSort to operate on CIMQualifierDecls.
Boolean operator>(const CIMQualifierDecl& q1, const CIMQualifierDecl& q2)
{
    return q1.getName().getString() > q2.getName().getString();
}

PEGASUS_NAMESPACE_END

void CompareQualifiers(
    CIMRepository& r1,
    CIMRepository& r2,
    const CIMNamespaceName& namespaceName)
{
    Array<CIMQualifierDecl> quals1 = r1.enumerateQualifiers(namespaceName);
    Array<CIMQualifierDecl> quals2 = r2.enumerateQualifiers(namespaceName);
    PEGASUS_TEST_ASSERT(quals1.size() == quals2.size());

    BubbleSort(quals1);
    BubbleSort(quals2);

    for (Uint32 i = 0; i < quals2.size(); i++)
    {
        if (verbose)
        {
            cout << "testing qualifier " << namespaceName.getString() << "/";
            cout << quals1[i].getName().getString() << "/ against /";
            cout << quals2[i].getName().getString() << "/" << endl;
        }

        PEGASUS_TEST_ASSERT(quals1[i].identical(quals2[i]));
    }
}

void Compare(
    const String& repositoryRoot1,
    const String& repositoryRoot2)
{
    //
    // Create repositories:
    //

    CIMRepository r1(repositoryRoot1);
    CIMRepository r2(repositoryRoot2);

    //
    // Compare the namespaces.
    //

    Array<CIMNamespaceName> nameSpaces1 = r1.enumerateNameSpaces();
    Array<CIMNamespaceName> nameSpaces2 = r2.enumerateNameSpaces();
    BubbleSort(nameSpaces1);
    BubbleSort(nameSpaces2);
    PEGASUS_TEST_ASSERT(nameSpaces1 == nameSpaces2);

    //
    // Compare classes in each namespace:
    //

    for (Uint32 i = 0; i < nameSpaces1.size(); i++)
    {
    CompareQualifiers(r1, r2, nameSpaces1[i]);
    CompareClasses(r1, r2, nameSpaces1[i]);
    CompareInstances(r1, r2, nameSpaces1[i]);
    }
}

int main(int argc, char** argv)
{

    verbose = (getenv ("PEGASUS_TEST_VERBOSE")) ? true : false;
    if (verbose) cout << argv[0] << ":" << endl;

    //
    // Usage:
    //


    if (argc != 3)
    {
    fprintf(stderr,
        "Usage: %s repository-root-1 repository-root-2\n", argv[0]);
    exit(1);
    }

    //
    // Extract repository roots:
    //

    try
    {
    Compare(argv[1], argv[2]);
    }
    catch (Exception& e)
    {
        cout << argv[0] << " " << e.getMessage() << endl;
    exit(1);
    }

    if (!failures)
    {
        cout << argv[0] << ": +++++ passed all tests" << endl;
        return 0;
    }
    else
    {
        cerr << argv[0] << ": +++++ There were " << failures << " failures"
            << endl;
        return 1;
    }
}
