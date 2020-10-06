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
#include <Pegasus/Common/CIMMethod.h>
#include <Pegasus/Common/DeclContext.h>
#include <Pegasus/Common/XmlWriter.h>
#include <Pegasus/General/MofWriter.h>

PEGASUS_USING_PEGASUS;
PEGASUS_USING_STD;

static Boolean verbose;

int main(int, char** argv)
{
    verbose = getenv("PEGASUS_TEST_VERBOSE") ? true : false;

    try
    {
        CIMMethod m1(CIMName ("getHostName"), CIMTYPE_STRING);
        m1.addQualifier(CIMQualifier(CIMName ("stuff"), true));
        m1.addQualifier(CIMQualifier(CIMName ("stuff2"), true));
        m1.addParameter(CIMParameter(CIMName ("ipaddress"), CIMTYPE_STRING));


        // Tests for Qualifiers
        PEGASUS_TEST_ASSERT(
            m1.findQualifier(CIMName ("stuff")) != PEG_NOT_FOUND);
        PEGASUS_TEST_ASSERT(
            m1.findQualifier(CIMName ("stuff2")) != PEG_NOT_FOUND);
        PEGASUS_TEST_ASSERT(
            m1.findQualifier(CIMName ("stuff21")) == PEG_NOT_FOUND);
        PEGASUS_TEST_ASSERT(
            m1.findQualifier(CIMName ("stuf")) == PEG_NOT_FOUND);
        PEGASUS_TEST_ASSERT(m1.getQualifierCount() == 2);

        PEGASUS_TEST_ASSERT(
            m1.findQualifier(CIMName ("stuff")) != PEG_NOT_FOUND);
        PEGASUS_TEST_ASSERT(
            m1.findQualifier(CIMName ("stuff2")) != PEG_NOT_FOUND);

        PEGASUS_TEST_ASSERT(
            m1.findQualifier(CIMName ("stuff21")) == PEG_NOT_FOUND);
        PEGASUS_TEST_ASSERT(
            m1.findQualifier(CIMName ("stuf")) == PEG_NOT_FOUND);

        Uint32 posQualifier;
        posQualifier = m1.findQualifier(CIMName ("stuff"));
        PEGASUS_TEST_ASSERT(posQualifier != PEG_NOT_FOUND);
        PEGASUS_TEST_ASSERT(posQualifier < m1.getQualifierCount());

        m1.removeQualifier(posQualifier);
        PEGASUS_TEST_ASSERT(m1.getQualifierCount() == 1);
        PEGASUS_TEST_ASSERT(
            m1.findQualifier(CIMName ("stuff")) == PEG_NOT_FOUND);
        PEGASUS_TEST_ASSERT(
            m1.findQualifier(CIMName ("stuff2")) != PEG_NOT_FOUND);

        // Tests for Parameters
        PEGASUS_TEST_ASSERT(m1.findParameter(
            CIMName ("ipaddress")) != PEG_NOT_FOUND);
        PEGASUS_TEST_ASSERT(m1.findParameter(
            CIMName ("noparam"))  == PEG_NOT_FOUND);
        PEGASUS_TEST_ASSERT(m1.getParameterCount()  == 1);
        CIMParameter cp = m1.getParameter(
            m1.findParameter(CIMName ("ipaddress")));
        PEGASUS_TEST_ASSERT(cp.getName() == CIMName ("ipaddress"));

        m1.removeParameter (m1.findParameter (
            CIMName (CIMName ("ipaddress"))));
        PEGASUS_TEST_ASSERT (m1.getParameterCount ()  == 0);
        m1.addParameter (CIMParameter (CIMName ("ipaddress"),
                                       CIMTYPE_STRING));
        PEGASUS_TEST_ASSERT (m1.getParameterCount ()  == 1);

        // throws OutOfBounds
        try
        {
            m1.removeParameter (1);
        }
        catch (IndexOutOfBoundsException & oob)
        {
            if (verbose)
            {
                PEGASUS_STD (cout) << "Caught expected exception: "
                                   << oob.getMessage () << PEGASUS_STD (endl);
            }
        }

        CIMMethod m2(CIMName ("test"), CIMTYPE_STRING);
        m2.setName(CIMName ("getVersion"));
        PEGASUS_TEST_ASSERT(m2.getName() == CIMName ("getVersion"));

        m2.setType(CIMTYPE_STRING);
        PEGASUS_TEST_ASSERT(m2.getType() == CIMTYPE_STRING);

        m2.setClassOrigin(CIMName ("test"));
        PEGASUS_TEST_ASSERT(m2.getClassOrigin() == CIMName ("test"));

        m2.setPropagated(true);
        PEGASUS_TEST_ASSERT(m2.getPropagated() == true);

        const CIMMethod cm1(m1);
        PEGASUS_TEST_ASSERT(cm1.findQualifier(
            CIMName ("stuff21")) == PEG_NOT_FOUND);
        PEGASUS_TEST_ASSERT(cm1.findQualifier(
            CIMName ("stuf")) == PEG_NOT_FOUND);
        PEGASUS_TEST_ASSERT((cm1.getParameterCount() != 3));
        PEGASUS_TEST_ASSERT(cm1.findParameter(
            CIMName ("ipaddress")) != PEG_NOT_FOUND);
        PEGASUS_TEST_ASSERT(cm1.findQualifier(
            CIMName ("stuff")) == PEG_NOT_FOUND);

        CIMQualifier q = m1.getQualifier(posQualifier);
        CIMConstParameter ccp = cm1.getParameter(
                    cm1.findParameter(CIMName ("ipaddress")));
        PEGASUS_TEST_ASSERT(cm1.getName() == CIMName ("getHostName"));
        PEGASUS_TEST_ASSERT(cm1.getType() == CIMTYPE_STRING);
        PEGASUS_TEST_ASSERT(!(cm1.getClassOrigin() == CIMName ("test")));
        PEGASUS_TEST_ASSERT(!cm1.getPropagated() == true);
        PEGASUS_TEST_ASSERT(!m1.identical(m2));

        // throws OutOfBounds
        try
        {
            CIMConstParameter p = cm1.getParameter(cm1.findParameter(
                                        CIMName ("ipaddress")));
        }
        catch(IndexOutOfBoundsException&)
        {
        }

        // throws OutOfBounds
        try
        {
            CIMConstQualifier q1 = cm1.getQualifier(cm1.findQualifier(
                                        CIMName ("abstract")));
        }
        catch(IndexOutOfBoundsException&)
        {
        }

        if (verbose)
        {
            XmlWriter::printMethodElement(m1);
            XmlWriter::printMethodElement(cm1);
        }
        Buffer out;
        XmlWriter::appendMethodElement(out, cm1);
        MofWriter::appendMethodElement(out, cm1);

        Boolean nullMethod = cm1.isUninitialized();
        PEGASUS_TEST_ASSERT(!nullMethod);

        CIMMethod m3 = m2.clone();
        m3 = cm1.clone();

        CIMMethod m4;
        CIMMethod m5(m4);

        CIMConstMethod ccm1(CIMName ("getHostName"),CIMTYPE_STRING);
        PEGASUS_TEST_ASSERT(!(ccm1.getParameterCount() == 3));

        PEGASUS_TEST_ASSERT(ccm1.getName() == CIMName ("getHostName"));
        PEGASUS_TEST_ASSERT(ccm1.getType() == CIMTYPE_STRING);
        PEGASUS_TEST_ASSERT(!(ccm1.getClassOrigin() == CIMName ("test")));
        PEGASUS_TEST_ASSERT(!ccm1.getPropagated() == true);
        PEGASUS_TEST_ASSERT(!(ccm1.getParameterCount() == 3));
        PEGASUS_TEST_ASSERT(ccm1.getQualifierCount() == 0);
        PEGASUS_TEST_ASSERT(ccm1.findQualifier(
            CIMName ("Stuff")) == PEG_NOT_FOUND);
        PEGASUS_TEST_ASSERT(ccm1.findParameter(
            CIMName ("ipaddress")) == PEG_NOT_FOUND);

        if (verbose)
        {
            XmlWriter::printMethodElement(m1);
            XmlWriter::printMethodElement(ccm1);
        }

        XmlWriter::appendMethodElement(out, ccm1);

        CIMConstMethod ccm2(ccm1);
        CIMConstMethod ccm3;

        ccm3 = ccm1.clone();
        ccm1 = ccm3;
        PEGASUS_TEST_ASSERT(ccm1.identical(ccm3));
        PEGASUS_TEST_ASSERT(ccm1.findQualifier(
            CIMName ("stuff")) == PEG_NOT_FOUND);
        PEGASUS_TEST_ASSERT(ccm1.findParameter(
            CIMName ("ipaddress")) == PEG_NOT_FOUND);

        nullMethod = ccm1.isUninitialized();
        PEGASUS_TEST_ASSERT(!nullMethod);

        // throws OutOfBounds
        try
        {
            //CIMParameter p = m1.getParameter(
            //     m1.findParameter(CIMName ("ipaddress")));
            CIMConstParameter p = ccm1.getParameter(0);
        }
        catch(IndexOutOfBoundsException&)
        {
        }

        // throws OutOfBounds
        try
        {
            CIMConstQualifier q1 = ccm1.getQualifier(0);
        }
        catch(IndexOutOfBoundsException&)
        {
        }
    }
    catch(Exception& e)
    {
        cerr << "Exception: " << e.getMessage() << endl;
    }

    // Test for add second qualifier with same name.
    // Should do exception

    cout << argv[0] << " +++++ passed all tests" << endl;

    return 0;
}
