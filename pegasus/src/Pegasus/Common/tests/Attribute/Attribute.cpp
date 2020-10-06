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
//
//%////////////////////////////////////////////////////////////////////////////

/**
    This file has testcases that are added to cover the functions in
    Pegasus/Common/Attribute.cpp.
*/

#include <cstdlib>
#include <iostream>
#include <Pegasus/Common/PegasusAssert.h>
#include <Pegasus/Common/Attribute.h>
#include <Pegasus/Common/String.h>
#include <Pegasus/Common/FileSystem.h>

PEGASUS_USING_PEGASUS;
PEGASUS_USING_STD;

void test01()
{
    /**
        Function to cover
        const String& Attribute::getTag() const
    */
    Attribute attribute("dummy");
    const String  ret_tag=attribute.getTag();
    PEGASUS_TEST_ASSERT( ret_tag == "dummy" );

    /**
        Function to cover
        void Attribute::addValue(const String& value)
            and
        const Array<String>& Attribute::getValues() const
    */
    attribute.addValue("value1");
    attribute.addValue("value2");
    attribute.addValue("value3");
    Array<String> str = attribute.getValues();
    PEGASUS_TEST_ASSERT( str.size() == 3 );

    /**
        Function to cover
        PEGASUS_STD(ostream)& operator<<(
            PEGASUS_STD(ostream)& os,const Attribute& attr)
    */
    ofstream outputFile;
    outputFile.open ("test.txt", ofstream::out);
    outputFile << attribute;
    outputFile.close();

    PEGASUS_TEST_ASSERT(FileSystem::compareFiles("test.txt", "master.txt"));
    PEGASUS_TEST_ASSERT(FileSystem::removeFile("test.txt"));
}

int main(int, char *argv[] )
{
    try
    {
        test01();
    }
    catch (Exception& e)
    {
        PEGASUS_STD (cout) << "Exception: " << e.getMessage()
            << PEGASUS_STD(endl);
        exit(1);
    }

    PEGASUS_STD (cout) << argv[0] << " +++++ passed all tests"
        << PEGASUS_STD(endl);
    return 0;
}

