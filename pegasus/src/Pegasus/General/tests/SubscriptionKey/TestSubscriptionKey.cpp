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
#include <Pegasus/General/SubscriptionKey.h>

PEGASUS_USING_PEGASUS;
PEGASUS_USING_STD;

const String nameProp="Name1";
const String ns1="root/test/inter";
const String ns2="root";
const String className="MyClassName";
const String keyBinds=
    "CreationClassName=\"CIM_IndicationFilter\",Name=\""+nameProp+"\","
        "SystemCreationClassName=\"CIM_ComputerSystem\",SystemName=\"TestSys\"";

class TestSubscriptionKey:SubscriptionKey
{
public:

    // Check that parsing can handle object path that does have hostname
    void testFullObjPath()
    {
        String fullPath="//TestSys/"+ns1+":"+className+"."+keyBinds;
        String name;
        String ns;
        String cn;

        SubscriptionKey::_parseObjectName(fullPath,name,ns,cn);
        PEGASUS_TEST_ASSERT(name==nameProp);
        PEGASUS_TEST_ASSERT(ns==ns1);
        PEGASUS_TEST_ASSERT(cn==className);
    }

    // Check that parsing can handle object path without hostname
    void testNoHostnameObjPath()
    {
        String fullPath=ns1+":"+className+"."+keyBinds;
        String name;
        String ns;
        String cn;

        SubscriptionKey::_parseObjectName(fullPath,name,ns,cn);
        PEGASUS_TEST_ASSERT(name==nameProp);
        PEGASUS_TEST_ASSERT(ns==ns1);
        PEGASUS_TEST_ASSERT(cn==className);
    }
    // Check that parsing can handle object path without hostname that starts
    // with a slash (need ignore slash)
    void testNoHostnameButSlashObjPath()
    {
        String fullPath="/"+ns1+":"+className+"."+keyBinds;
        String name;
        String ns;
        String cn;

        SubscriptionKey::_parseObjectName(fullPath,name,ns,cn);
        PEGASUS_TEST_ASSERT(name==nameProp);
        PEGASUS_TEST_ASSERT(ns==ns1);
        PEGASUS_TEST_ASSERT(cn==className);
    }
    // Check that parsing can handle object path without name space that
    // starts with a colon
    void testNoNameSpaceButColonObjPath()
    {
        String fullPath=":"+className+"."+keyBinds;
        String name;
        String ns;
        String cn;

        SubscriptionKey::_parseObjectName(fullPath,name,ns,cn);
        PEGASUS_TEST_ASSERT(name==nameProp);
        PEGASUS_TEST_ASSERT(ns.size() == 0);
        PEGASUS_TEST_ASSERT(cn==className);
    }
    // Check that parsing can handle object path without name space
    void testNoNameSpaceObjPath()
    {
        String fullPath=className+"."+keyBinds;
        String name;
        String ns;
        String cn;

        SubscriptionKey::_parseObjectName(fullPath,name,ns,cn);
        PEGASUS_TEST_ASSERT(name==nameProp);
        PEGASUS_TEST_ASSERT(ns.size() == 0);
        PEGASUS_TEST_ASSERT(cn==className);
    }
};

int main()
{
    TestSubscriptionKey x;
    x.testFullObjPath();
    x.testNoHostnameObjPath();
    x.testNoHostnameButSlashObjPath();
    x.testNoNameSpaceButColonObjPath();
    x.testNoNameSpaceObjPath();

    return 0;
}
