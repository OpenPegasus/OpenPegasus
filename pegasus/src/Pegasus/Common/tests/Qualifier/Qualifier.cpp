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
#include <Pegasus/Common/CIMQualifier.h>
#include <Pegasus/Common/CIMProperty.h>
#include <Pegasus/Common/CIMFlavor.h>
#include <Pegasus/Common/Resolver.h>
#include <Pegasus/Common/XmlWriter.h>

PEGASUS_USING_PEGASUS;
PEGASUS_USING_STD;

static const CIMFlavor CIMFLAVOR_ALL = CIMFlavor::OVERRIDABLE +
    CIMFlavor::TOSUBCLASS + CIMFlavor::TOINSTANCE + CIMFlavor::TRANSLATABLE +
    CIMFlavor::DISABLEOVERRIDE + CIMFlavor::RESTRICTED;

/* This program tests the CIMQualifier class and the CIMConstQualifier class
 including the functions in the classes:

 It creates qualifiers, tests the scope, value and flavor characteristics.
 ATTN: P3 - KS March 2002 Add more tests for scope, etc.
*/
int main(int, char** argv)
{
#if defined(PEGASUS_OS_DARWIN) || defined (PEGASUS_OS_VMS)
    static const CIMFlavor CIMFLAVOR_ALL = CIMFlavor::OVERRIDABLE +
    CIMFlavor::TOSUBCLASS + CIMFlavor::TOINSTANCE + CIMFlavor::TRANSLATABLE +
    CIMFlavor::DISABLEOVERRIDE + CIMFlavor::RESTRICTED;
#endif
    // get the output display flag.
    Boolean verbose = (getenv("PEGASUS_TEST_VERBOSE")) ? true : false;

    try
    {
    CIMQualifier q1(CIMName ("Description"), String("Hello"),
            CIMFlavor::TOINSTANCE);
    // This one sets the defaults overridable and tosubclass
    CIMQualifier q2(CIMName ("Abstract"), true);
    CIMConstQualifier q3 = q1;
    CIMConstQualifier q4;
    q4 = q3;

    if (verbose)
    {
        XmlWriter::printQualifierElement(q1);
        XmlWriter::printQualifierElement(q2);
        XmlWriter::printQualifierElement(q3);
        XmlWriter::printQualifierElement(q4);
    }

    // Note effective march 27 2002, Qualifier no longer get the default flavor
    // from the definition.  Now the defaults come from the declaraction as part
    // of the resolve.  As created, qualifiers have exactly the flavor with
    // which they are defined. They have no default flavors when there is no
    // explicit definition.
    PEGASUS_TEST_ASSERT(q4.identical(q1));
    PEGASUS_TEST_ASSERT(q1.getFlavor ().hasFlavor(CIMFlavor::TOINSTANCE));
    //PEGASUS_TEST_ASSERT(!q1.getFlavor ().hasFlavor(CIMFlavor::TOSUBCLASS));
    //PEGASUS_TEST_ASSERT(!q1.getFlavor ().hasFlavor(CIMFlavor::OVERRIDABLE));

    PEGASUS_TEST_ASSERT(q1.getFlavor ().hasFlavor(CIMFlavor::TOINSTANCE));
    PEGASUS_TEST_ASSERT(!q1.getFlavor ().hasFlavor(CIMFlavor::TOSUBCLASS));
    PEGASUS_TEST_ASSERT(!q1.getFlavor ().hasFlavor(CIMFlavor::OVERRIDABLE));

    PEGASUS_TEST_ASSERT(!q2.getFlavor ().hasFlavor(CIMFlavor::TOINSTANCE));

    // Test to be sure the defaults are set correctly
    PEGASUS_TEST_ASSERT(!q2.getFlavor ().hasFlavor(CIMFlavor::TOSUBCLASS));
    PEGASUS_TEST_ASSERT(!q2.getFlavor ().hasFlavor(CIMFlavor::OVERRIDABLE));
    PEGASUS_TEST_ASSERT(!q2.getFlavor ().hasFlavor(CIMFlavor::TRANSLATABLE));
    PEGASUS_TEST_ASSERT(!q2.getFlavor ().hasFlavor(CIMFlavor::ENABLEOVERRIDE));
    PEGASUS_TEST_ASSERT(!q2.getFlavor ().hasFlavor(CIMFlavor::DISABLEOVERRIDE));


    q2.unsetFlavor(CIMFLAVOR_ALL);
    PEGASUS_TEST_ASSERT(!q2.getFlavor ().hasFlavor(CIMFlavor::TOSUBCLASS));
    PEGASUS_TEST_ASSERT(!q2.getFlavor ().hasFlavor(CIMFlavor::TOINSTANCE));
    PEGASUS_TEST_ASSERT(!q2.getFlavor ().hasFlavor(CIMFlavor::OVERRIDABLE));
    PEGASUS_TEST_ASSERT(!q2.getFlavor ().hasFlavor(CIMFlavor::TRANSLATABLE));
    PEGASUS_TEST_ASSERT(!q2.getFlavor ().hasFlavor(CIMFlavor::ENABLEOVERRIDE));
    PEGASUS_TEST_ASSERT(!q2.getFlavor ().hasFlavor(CIMFlavor::DISABLEOVERRIDE));
    PEGASUS_TEST_ASSERT(!q2.getFlavor ().hasFlavor(CIMFlavor::RESTRICTED));

    q2.setFlavor(CIMFlavor::TOSUBCLASS);
    PEGASUS_TEST_ASSERT(q2.getFlavor ().hasFlavor(CIMFlavor::TOSUBCLASS));
    PEGASUS_TEST_ASSERT(!q2.getFlavor ().hasFlavor(CIMFlavor::OVERRIDABLE));

    q2.unsetFlavor(CIMFlavor::TOSUBCLASS);
    PEGASUS_TEST_ASSERT(!q2.getFlavor ().hasFlavor(CIMFlavor::TOSUBCLASS));
    PEGASUS_TEST_ASSERT(!q2.getFlavor ().hasFlavor(CIMFlavor::OVERRIDABLE));

    Resolver::resolveQualifierFlavor(q2, CIMFlavor (CIMFlavor::OVERRIDABLE));
    PEGASUS_TEST_ASSERT(q2.getFlavor ().hasFlavor(CIMFlavor::OVERRIDABLE));

    q2.setFlavor(CIMFLAVOR_ALL);
    PEGASUS_TEST_ASSERT(q2.getFlavor ().hasFlavor(CIMFlavor::TOSUBCLASS));
    PEGASUS_TEST_ASSERT(q2.getFlavor ().hasFlavor(CIMFlavor::TOINSTANCE));
    PEGASUS_TEST_ASSERT(q2.getFlavor ().hasFlavor(CIMFlavor::OVERRIDABLE));
    PEGASUS_TEST_ASSERT(q2.getFlavor ().hasFlavor(CIMFlavor::TRANSLATABLE));
    PEGASUS_TEST_ASSERT(q2.getFlavor ().hasFlavor(CIMFlavor::ENABLEOVERRIDE));
    PEGASUS_TEST_ASSERT(q2.getFlavor ().hasFlavor(CIMFlavor::DISABLEOVERRIDE));
    PEGASUS_TEST_ASSERT(q2.getFlavor ().hasFlavor(CIMFlavor::RESTRICTED));


    // ATTN: KS P1 24 March 2002Add test for resolveFlavor here
    q2.unsetFlavor(CIMFLAVOR_ALL);

    q2.setFlavor (CIMFlavor::TOSUBCLASS + CIMFlavor::ENABLEOVERRIDE);

    Resolver::resolveQualifierFlavor (q2, CIMFlavor
            (CIMFlavor::DISABLEOVERRIDE + CIMFlavor::RESTRICTED));
    PEGASUS_TEST_ASSERT( q2.getFlavor ().hasFlavor(CIMFlavor::DISABLEOVERRIDE));
    PEGASUS_TEST_ASSERT(!q2.getFlavor ().hasFlavor(CIMFlavor::ENABLEOVERRIDE));
    PEGASUS_TEST_ASSERT(!q2.getFlavor ().hasFlavor(CIMFlavor::TOSUBCLASS));
    PEGASUS_TEST_ASSERT(!q2.getFlavor ().hasFlavor(CIMFlavor::TOINSTANCE));

    CIMQualifier qual1(CIMName ("qual1"), String("This is a test"));

    CIMQualifier qual3(CIMName ("qual3"), String("This is a test"));
    PEGASUS_TEST_ASSERT(!qual1.identical(qual3));

    if (verbose)
    {
        XmlWriter::printQualifierElement(q4);
    }

    }
    catch (Exception& e)
    {
    cerr << "Exception: " << e.getMessage() << endl;
    }

    cout << argv[0] << " +++++ passed all tests" << endl;

    return 0;
}
