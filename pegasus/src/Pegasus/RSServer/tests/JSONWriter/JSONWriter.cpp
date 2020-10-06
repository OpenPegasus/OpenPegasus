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

#include <cstdio>
#include <Pegasus/Common/PegasusAssert.h>
#include <Pegasus/RSServer/JSONWriter.h>
#include <Pegasus/Common/FileSystem.h>

PEGASUS_USING_PEGASUS;
PEGASUS_USING_STD;

static Boolean verbose = false;

void testClass()
{
    Buffer expected;
    FileSystem::loadFileToMemory(expected, "./class.json");
    if (verbose) cout << "Expected: " << expected.getData() << endl;

    Buffer outputBuffer;
    JSONWriter writer(outputBuffer);

    CIMClass c(CIMName("className"), CIMName("superClassName"));
    c.addProperty(CIMProperty(CIMName("boolProp"), CIMValue(true)));
    c.addProperty(CIMProperty(CIMName("intProp"), CIMValue(1)));
    c.addProperty(CIMProperty(
            CIMName("stringProp"),
            CIMValue(String("hello world"))));

    writer._append(c);
    if (verbose) cout << "Got: " << outputBuffer.getData() << endl;

    PEGASUS_TEST_ASSERT(
            System::strcasecmp(
                expected.getData(),
                outputBuffer.getData()) == 0);
}


void testInstance()
{
    Buffer expected;
    FileSystem::loadFileToMemory(expected, "./instance.json");
    if (verbose) cout << "Expected: " << expected.getData() << endl;

    Buffer outputBuffer;
    JSONWriter writer(outputBuffer);

    CIMInstance x(CIMName("className"));
    x.addProperty(CIMProperty(CIMName("boolProp"), CIMValue(true)));
    x.addProperty(CIMProperty(CIMName("intProp"), CIMValue(1)));
    x.addProperty(CIMProperty(
            CIMName("stringProp"),
            CIMValue(String("hello world"))));

    writer._append(x, false, true);
    if (verbose) cout << "Got: " << outputBuffer.getData() << endl;

    PEGASUS_TEST_ASSERT(
            System::strcasecmp(
                expected.getData(),
                outputBuffer.getData()) == 0);
}

void testInstanceCollection()
{
    Buffer expected;
    FileSystem::loadFileToMemory(expected, "./instanceCollection.json");
    if (verbose) cout << "Expected: " << expected.getData() << endl;

    Buffer outputBuffer;
    JSONWriter writer(outputBuffer);
    CIMName className = "className";

    Array<CIMObject> instances;

    for (Uint32 i = 0; i < 10; i++)
    {
        CIMInstance x(className);
        x.addProperty(CIMProperty(CIMName("boolProp"), CIMValue(true)));
        x.addProperty(CIMProperty(CIMName("intProp"), CIMValue(i)));
        x.addProperty(CIMProperty(
                CIMName("stringProp"),
                CIMValue(String("hello world"))));
        Buffer objPath;
        objPath << className.getString() << ".intProp=" << i;
        x.setPath(CIMObjectPath(objPath.getData()));
        instances.append(x);
    }

    writer._append(instances);
    if (verbose) cout << "Got: " << outputBuffer.getData() << endl;

    PEGASUS_TEST_ASSERT(
            System::strcasecmp(
                expected.getData(),
                outputBuffer.getData()) == 0);
}



int main(int argc, char** argv)
{
    verbose = getenv("PEGASUS_TEST_VERBOSE") ? true : false;

    testClass();
    testInstance();
    testInstanceCollection();

    cout << argv[0] << " +++++ passed all tests" << endl;
    return 0;
}
