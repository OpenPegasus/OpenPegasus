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
// Author: Mike Brasher (mbrasher@bmc.com)
//
// Modified By: Jenny Yu, Hewlett-Packard Company (jenny_yu@hp.com)
//              Carol Ann Krug Graves, Hewlett-Packard Company
//                  (carolann_graves@hp.com)
//              David Dillard, VERITAS Software Corp.
//                  (david.dillard@veritas.com)
//
//%/////////////////////////////////////////////////////////////////////////////

#include <Pegasus/Common/PegasusAssert.h>
#include <Pegasus/Common/CIMParameter.h>
#include <Pegasus/Common/XmlWriter.h>
#include <Pegasus/General/MofWriter.h>

PEGASUS_USING_PEGASUS;
PEGASUS_USING_STD;
static Boolean verbose;   // controls IO from test

void test01()
{
    CIMParameter p;
    PEGASUS_TEST_ASSERT(p.isUninitialized());

    CIMParameter p1(CIMName ("message"), CIMTYPE_STRING);
    p1.addQualifier(CIMQualifier(CIMName ("in"), true));
    if(verbose)
        XmlWriter::printParameterElement(p1, cout);

    CIMParameter p2(CIMName ("message2"), CIMTYPE_STRING);
    p2.addQualifier(CIMQualifier(CIMName ("in"), true));
    if(verbose)
        XmlWriter::printParameterElement(p2, cout);

    CIMParameter p3(CIMName ("message3"), CIMTYPE_STRING);
    p3.setName(CIMName ("message3a"));
    PEGASUS_TEST_ASSERT(p3.getName() == CIMName ("message3a"));
    if(verbose)
        XmlWriter::printParameterElement(p3, cout);
    PEGASUS_TEST_ASSERT(p3.getType() == CIMTYPE_STRING);;

    //
    // clone
    //
    CIMParameter p1clone = p1.clone();
    if(verbose)
        XmlWriter::printParameterElement(p1clone, cout);

    //
    // toMof
    //
    Buffer mofOut;
    if(verbose)
        MofWriter::appendParameterElement(mofOut, p1);

    //
    // toXml
    //
    Buffer xmlOut;
    if(verbose)
        XmlWriter::appendParameterElement(xmlOut, p1);

    //
    // identical
    //
    Boolean same;
    same  = p1clone.identical(p1);
    PEGASUS_TEST_ASSERT(same);

    //
    // not identical
    //
    same = p1clone.identical(p2);
    PEGASUS_TEST_ASSERT(!same);

    //
    // get qualifier count
    //
    PEGASUS_TEST_ASSERT(p1.getQualifierCount() == 1);
    PEGASUS_TEST_ASSERT(p2.getQualifierCount() == 1);
    PEGASUS_TEST_ASSERT(p3.getQualifierCount() == 0);

    //
    // find qualifier
    //
    PEGASUS_TEST_ASSERT(p1.findQualifier(CIMName ("in")) != PEG_NOT_FOUND);
    PEGASUS_TEST_ASSERT(p2.findQualifier(CIMName ("in")) != PEG_NOT_FOUND);
    PEGASUS_TEST_ASSERT(p2.findQualifier(CIMName ("none")) == PEG_NOT_FOUND);
    PEGASUS_TEST_ASSERT(p3.findQualifier(CIMName ("none")) == PEG_NOT_FOUND);

    //
    // get qualifiers
    //
    CIMQualifier q1 = p1.getQualifier(0);
    PEGASUS_TEST_ASSERT(!q1.isUninitialized());
    CIMConstQualifier q1const = p1.getQualifier(0);
    PEGASUS_TEST_ASSERT(!q1const.isUninitialized());

    //
    // remove qualifiers
    //
    p1.removeQualifier (p1.findQualifier (CIMName ("in")));
    PEGASUS_TEST_ASSERT (p1.findQualifier (CIMName ("in")) == PEG_NOT_FOUND);
    PEGASUS_TEST_ASSERT (p1.getQualifierCount () == 0);
    p1.addQualifier (CIMQualifier (CIMName ("in"), true));

    try
    {
        //
        // should throw OutOfBounds
        //
        p1.removeQualifier (1);
    }
    catch (IndexOutOfBoundsException & oob)
    {
        if (verbose)
        {
            PEGASUS_STD (cout) << "Caught expected exception: "
                               << oob.getMessage () << PEGASUS_STD (endl);
            PEGASUS_STD (cout) << PEGASUS_STD (endl);
        }
    }

    //
    // isArray
    //
    PEGASUS_TEST_ASSERT(p1.isArray() == false);

    //
    // getArraySize
    //
    PEGASUS_TEST_ASSERT(p1.getArraySize() == 0);

    //
    // test CIMConstParameter methods
    //
    CIMConstParameter cp1 = p1;
    CIMConstParameter cp2 = p2;
    CIMConstParameter cp3 = p3;
    CIMConstParameter cp4(CIMName ("message4"), CIMTYPE_STRING);

    if(verbose)
        XmlWriter::printParameterElement(cp1, cout);

    PEGASUS_TEST_ASSERT(cp3.getName() == CIMName ("message3a"));
    PEGASUS_TEST_ASSERT(cp3.getType() == CIMTYPE_STRING);;

    const CIMParameter cp1clone = cp1.clone();
    if(verbose)
        XmlWriter::printParameterElement(cp1clone, cout);

    XmlWriter::appendParameterElement(xmlOut, cp1);

    PEGASUS_TEST_ASSERT(cp1.identical(cp1) == true);
    PEGASUS_TEST_ASSERT(cp1.identical(cp2) == false);
    PEGASUS_TEST_ASSERT(cp1.isArray() == false);
    PEGASUS_TEST_ASSERT(cp1.getArraySize() == 0);
    PEGASUS_TEST_ASSERT(cp1.getQualifierCount() == 1);
    PEGASUS_TEST_ASSERT(cp1.findQualifier(CIMName ("in")) != PEG_NOT_FOUND);
    CIMConstQualifier cq1 = cp1.getQualifier(0);
    PEGASUS_TEST_ASSERT(!cq1.isUninitialized());
}

int main(int, char** argv)
{
    verbose = getenv("PEGASUS_TEST_VERBOSE") ? true : false;

    try
    {
        test01();
    }
    catch (Exception& e)
    {
        cout << "Exception: " << e.getMessage() << endl;
    }

    cout << argv[0] << " +++++ passed all tests" << endl;

    return 0;
}
