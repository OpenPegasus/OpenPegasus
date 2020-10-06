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

#include <Pegasus/Common/PegasusAssert.h>
#include <Pegasus/Repository/AssocClassCache.h>

PEGASUS_USING_PEGASUS;
PEGASUS_USING_STD;

int main(int, char** argv)
{
    AssocClassCacheManager cacheManager;
    ClassAssociation entry;
    Array<ClassAssociation> entryList;

    AssocClassCache* a1 = cacheManager.getAssocClassCache("a");
    PEGASUS_TEST_ASSERT(a1->getNameSpace() == "a");
    PEGASUS_TEST_ASSERT(!a1->isActive());
    a1->setActive(true);
    PEGASUS_TEST_ASSERT(a1->isActive());
    PEGASUS_TEST_ASSERT(
        !a1->removeRecord(CIMName("X"), CIMName("Assoc1")));
    PEGASUS_TEST_ASSERT(!a1->getAssocClassEntry(CIMName("X"), entryList));
    entry = ClassAssociation(
        CIMName("Assoc1"),
        CIMName("X"),
        CIMName("prop1"),
        CIMName("Y"),
        CIMName("prop2"));
    PEGASUS_TEST_ASSERT(a1->addRecord(entry.fromClassName, entry));

    {
        // Ensure AssocClassCacheManagers are independent by introducing
        // a competing object.
        AssocClassCacheManager tmpCacheManager;
        AssocClassCache* a2 = tmpCacheManager.getAssocClassCache("a");
        PEGASUS_TEST_ASSERT(!a2->getAssocClassEntry(CIMName("X"), entryList));
        entry = ClassAssociation(
            CIMName("AssocClass"),
            CIMName("FromClass"),
            CIMName("from"),
            CIMName("ToClass"),
            CIMName("to"));
        PEGASUS_TEST_ASSERT(a2->addRecord(entry.fromClassName, entry));
    }

    entry = ClassAssociation(
        CIMName("Assoc1"),
        CIMName("Y"),
        CIMName("prop2"),
        CIMName("X"),
        CIMName("prop1"));
    PEGASUS_TEST_ASSERT(a1->addRecord(entry.fromClassName, entry));
    entry = ClassAssociation(
        CIMName("Assoc2"),
        CIMName("Y"),
        CIMName("prop2"),
        CIMName("Z"),
        CIMName("prop3"));
    PEGASUS_TEST_ASSERT(a1->addRecord(entry.fromClassName, entry));

    PEGASUS_TEST_ASSERT(a1->getAssocClassEntry(CIMName("X"), entryList));
    PEGASUS_TEST_ASSERT(entryList.size() == 1);
    PEGASUS_TEST_ASSERT(a1->getAssocClassEntry(CIMName("Y"), entryList));
    PEGASUS_TEST_ASSERT(entryList.size() == 2);
    PEGASUS_TEST_ASSERT(a1->removeAssocClassRecords(CIMName("Assoc1")));
    PEGASUS_TEST_ASSERT(!a1->getAssocClassEntry(CIMName("X"), entryList));
    PEGASUS_TEST_ASSERT(a1->getAssocClassEntry(CIMName("Y"), entryList));
    PEGASUS_TEST_ASSERT(entryList.size() == 1);
    entry = ClassAssociation(
        CIMName("Assoc1"),
        CIMName("Y"),
        CIMName("prop2"),
        CIMName("X"),
        CIMName("prop1"));
    PEGASUS_TEST_ASSERT(a1->addRecord(entry.fromClassName, entry));
    PEGASUS_TEST_ASSERT(a1->addRecord(entry.toClassName, entry));

    PEGASUS_TEST_ASSERT(a1->getAssocClassEntry(CIMName("X"), entryList));
    PEGASUS_TEST_ASSERT(entryList.size() == 1);
    PEGASUS_TEST_ASSERT(a1->getAssocClassEntry(CIMName("Y"), entryList));
    PEGASUS_TEST_ASSERT(entryList.size() == 2);
    PEGASUS_TEST_ASSERT(a1->removeRecord(CIMName("Y"), CIMName("Assoc1")));
    PEGASUS_TEST_ASSERT(a1->getAssocClassEntry(CIMName("Y"), entryList));
    PEGASUS_TEST_ASSERT(entryList.size() == 1);
    PEGASUS_TEST_ASSERT(a1->removeRecord(CIMName("Y"), CIMName("Assoc2")));
    PEGASUS_TEST_ASSERT(!a1->getAssocClassEntry(CIMName("Y"), entryList));
    PEGASUS_TEST_ASSERT(!a1->removeRecord(CIMName("X"), CIMName("Assoc2")));
    PEGASUS_TEST_ASSERT(a1->getAssocClassEntry(CIMName("X"), entryList));
    PEGASUS_TEST_ASSERT(a1->removeRecord(CIMName("X"), CIMName("Assoc1")));
    PEGASUS_TEST_ASSERT(!a1->getAssocClassEntry(CIMName("X"), entryList));

    entry = ClassAssociation(
        CIMName("Assoc1"),
        CIMName("X"),
        CIMName("prop1"),
        CIMName("Y"),
        CIMName("prop2"));
    PEGASUS_TEST_ASSERT(a1->addRecord(entry.fromClassName, entry));
    PEGASUS_TEST_ASSERT(a1->getAssocClassEntry(CIMName("X"), entryList));
    PEGASUS_TEST_ASSERT(entryList.size() == 1);
    PEGASUS_TEST_ASSERT(a1->removeRecord(CIMName("X"), CIMName("Assoc1")));
    PEGASUS_TEST_ASSERT(!a1->getAssocClassEntry(CIMName("X"), entryList));

    cout << argv[0] << " +++++ passed all tests" << endl;

    return 0;
}
