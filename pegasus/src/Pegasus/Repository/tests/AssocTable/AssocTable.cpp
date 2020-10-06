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

#include <Pegasus/Repository/AssocClassTable.h>
#include <Pegasus/Repository/AssocInstTable.h>

PEGASUS_USING_PEGASUS;
PEGASUS_USING_STD;

int main(int, char** argv)
{
    String assocTablePath;
    const char* tmpDir = getenv ("PEGASUS_TMP");
    if (tmpDir == NULL)
    {
        assocTablePath = ".";
    }
    else
    {
        assocTablePath = tmpDir;
    }
    assocTablePath.append("/associations.tbl");

    AssocClassTable _assocClassTable;

    //
    // create class association
    //
    _assocClassTable.append(
        assocTablePath,
        ClassAssociation(
            CIMName("Lineage"),
            CIMName("Person"),
            CIMName("parent"),
            CIMName("Person"),
            CIMName("child")));

    //
    // check that the association was really created
    //
    {
        Array<CIMName> classList;
        classList.append(CIMName("Person"));
        Array<String> referenceNames;
        _assocClassTable.getReferenceNames(
            assocTablePath,
            classList,
            Array<CIMName>(),
            String::EMPTY,
            referenceNames);
        PEGASUS_TEST_ASSERT(referenceNames.size() == 1);
    }

    //
    // delete class association
    //
    _assocClassTable.deleteAssociation(
        assocTablePath,
        CIMName("Lineage"));

    //
    // check that the association was really deleted
    //
    {
        Array<CIMName> classList;
        classList.append(CIMName("Person"));
        Array<String> referenceNames;
        _assocClassTable.getReferenceNames(
            assocTablePath,
            classList,
            Array<CIMName>(),
            String::EMPTY,
            referenceNames);
        PEGASUS_TEST_ASSERT(referenceNames.size() == 0);
    }

    //
    // create instance association
    //
    AssocInstTable::append(
        assocTablePath,
        "A.left=\"x.key=\\\"one\\\"\",right=\"y.key=\\\"two\\\"\"",
        CIMName ("A"),
        "X.key=\"one\"",
        CIMName ("X"),
        CIMName ("left"),
        "Y.key=\"two\"",
        CIMName ("Y"),
        CIMName ("right"));

    //
    // delete instance association
    //
    AssocInstTable::deleteAssociation(
        assocTablePath,
        CIMObjectPath
            ("A.left=\"x.key=\\\"one\\\"\",right=\"y.key=\\\"two\\\"\""));

    cout << argv[0] << " +++++ passed all tests" << endl;

    return 0;
}
