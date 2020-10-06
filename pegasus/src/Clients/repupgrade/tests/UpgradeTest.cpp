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

#include <Pegasus/Repository/CIMRepository.h>

PEGASUS_USING_STD;
PEGASUS_USING_PEGASUS;

const CIMNamespaceName TESTNS = CIMNamespaceName("test/repupgrade");

void testQualifierTransfer(
    CIMRepository& oldRepository,
    CIMRepository& newRepository,
    const CIMNamespaceName& ns,
    const CIMName& qualifierName)
{
    CIMQualifierDecl q1 = oldRepository.getQualifier(ns, qualifierName);
    CIMQualifierDecl q2 = newRepository.getQualifier(ns, qualifierName);

    PEGASUS_TEST_ASSERT(q1.identical(q2));
}

void testClassTransfer(
    CIMRepository& oldRepository,
    CIMRepository& newRepository,
    const CIMNamespaceName& ns,
    const CIMName& className)
{
    CIMClass c1 = oldRepository.getClass(ns, className);
    CIMClass c2 = newRepository.getClass(ns, className);

    PEGASUS_TEST_ASSERT(c1.identical(c2));
}

void testInstancesTransfer(
    CIMRepository& oldRepository,
    CIMRepository& newRepository,
    const CIMNamespaceName& ns,
    const CIMName& className)
{
    Array<CIMInstance> i1 =
        oldRepository.enumerateInstancesForClass(ns, className);
    Array<CIMInstance> i2 =
        newRepository.enumerateInstancesForClass(ns, className);

    PEGASUS_TEST_ASSERT(i1.size() == i2.size());

    for (Uint32 i = 0; i < i1.size(); i++)
    {
        Boolean found = false;

        for (Uint32 j = 0; j < i2.size(); j++)
        {
            if (i1[i].identical(i2[j]))
            {
                found = true;
                break;
            }
        }

        PEGASUS_TEST_ASSERT(found);
    }
}

int main(int, char** argv)
{
    String  oldRepositoryPath = argv[1];
    String  newRepositoryPath = argv[2];

    CIMRepository oldRepository(oldRepositoryPath);
    CIMRepository newRepository(newRepositoryPath);
#ifdef NS_INTEROP
    String  masterRepositoryPath = argv[3];
    CIMRepository masterRepository(masterRepositoryPath);
#endif

    testQualifierTransfer(oldRepository, newRepository, TESTNS, "ASSOCIATION");
    testQualifierTransfer(oldRepository, newRepository, TESTNS, "Description");
    testQualifierTransfer(oldRepository, newRepository, TESTNS, "Key");
    testQualifierTransfer(oldRepository, newRepository, TESTNS, "Version");

    testClassTransfer(
        oldRepository, newRepository, TESTNS, "TST_LabeledLineage");
    testClassTransfer(
        oldRepository, newRepository, TESTNS, "TST_LabeledLineageDynamic");
    testClassTransfer(
        oldRepository, newRepository, TESTNS, "TST_Lineage");
    testClassTransfer(
        oldRepository, newRepository, TESTNS, "TST_LineageDynamic");
    testClassTransfer(
        oldRepository, newRepository, TESTNS, "TST_LineageDynamicSubClass");
    testClassTransfer(
        oldRepository, newRepository, TESTNS, "TST_Person");
    testClassTransfer(
        oldRepository, newRepository, TESTNS, "TST_PersonDynamic");
    testClassTransfer(
        oldRepository, newRepository, TESTNS, "TST_PersonDynamicSubClass");
    testClassTransfer(
        oldRepository, newRepository, TESTNS, "TST_PersonS");

    testInstancesTransfer(
        oldRepository, newRepository, TESTNS, "TST_Person");
    testInstancesTransfer(
        oldRepository, newRepository, TESTNS, "TST_PersonS");

#ifdef NS_INTEROP
    // interop support testing
    testInstancesTransfer( newRepository, masterRepository, 
        PEGASUS_NAMESPACENAME_INTEROP, "TST_Lineage");
#endif
    
    cout << argv[0] << " +++++ passed all tests" << endl;
    return 0;
}
