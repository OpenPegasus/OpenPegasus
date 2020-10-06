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

/**
    This file has testcases that are added to cover the functions in
    Pegasus/ProviderManager2/ProviderName.cpp.
*/

#include <Pegasus/ProviderManager2/ProviderName.h>
#include <Pegasus/Common/PegasusAssert.h>

PEGASUS_USING_PEGASUS;
PEGASUS_USING_STD;

void test01()
{
    /**
        Added to cover the Function
        ProviderName::ProviderName(
        const String& moduleName,
        const String & logicalName,
        const String & physicalName)
    */
    ProviderName pnobj(
        "dummymodulename",
        "dummylogicalname",
        "dummyphysicalname");

    /**
        Added to cover the Function
        String ProviderName::getModuleName(void) const
    */
    String get_module_name = pnobj.getModuleName();
    PEGASUS_TEST_ASSERT(get_module_name == "dummymodulename");

    /**
        Added to cover the Function
        String ProviderName::getLogicalName(void) const
    */
    String get_logical_name = pnobj.getLogicalName();
    PEGASUS_TEST_ASSERT(get_logical_name == "dummylogicalname");

    /**
        Added to cover the Functions
        void ProviderName::setLocation(const String &location)
                        AND
        String ProviderName::getLocation(void) const
    */
    pnobj.setLocation("//localhost/root/dummy");
    String get_loc = pnobj.getLocation();
    PEGASUS_TEST_ASSERT(get_loc == "//localhost/root/dummy");
}

int main(int, char** argv)
{
    try
    {
        test01();
    }
    catch (Exception& e)
    {
        PEGASUS_STD(cout) << "Exception: " << e.getMessage() <<
            PEGASUS_STD(endl);
        exit(1);
    }

    PEGASUS_STD(cout) << argv[0] << " +++++ passed all tests" <<
        PEGASUS_STD(endl);
    return 0;
}
