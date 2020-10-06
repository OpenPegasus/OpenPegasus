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
#include <cstdlib>
#include <iostream>
#include <Pegasus/Common/CIMObjectPath.h>
#include <Pegasus/Common/CIMName.h>
#include <Pegasus/Common/XmlWriter.h>
#include <Pegasus/Common/InternalException.h>

#include <Pegasus/General/MofWriter.h>

PEGASUS_USING_PEGASUS;
PEGASUS_USING_STD;

static Boolean verbose;

void test01()
{
    {
        String on1;
        on1 = "//atp:77/root/cimv25:"
              "TennisPlayer.last=\"Rafter\",first=\"Patrick\"";

        String on2;
        on2 = "//atp:77/root/cimv25:"
              "TennisPlayer.first=\"Patrick\",last=\"Rafter\"";

        CIMObjectPath r = on1;
        PEGASUS_TEST_ASSERT(r.toString() != on1);
        PEGASUS_TEST_ASSERT(r.toString() == on2);

        CIMObjectPath r2 = r;
        CIMObjectPath r3 = CIMObjectPath
            ("//atp:77/root/cimv25:"
            "TennisPlayer.first=\"Chris\",last=\"Evert\"");

        if (verbose)
        {
            XmlWriter::printValueReferenceElement(r, false);
            cout << r.toString() << endl;
        }

        Buffer mofOut;
        MofWriter::appendValueReferenceElement(mofOut, r);
        r.clear();
    }

    {
        CIMObjectPath r1 = CIMObjectPath
            ("MyClass.z=true,y=1234,x=\"Hello World\"");
        CIMObjectPath r2 = CIMObjectPath
            ("myclass.X=\"Hello World\",Z=true,Y=1234");
        CIMObjectPath r3 = CIMObjectPath ("myclass.X=\"Hello\",Z=true,Y=1234");
        // cout << r1.toString() << endl;
        // cout << r2.toString() << endl;
        PEGASUS_TEST_ASSERT(r1 == r2);
        PEGASUS_TEST_ASSERT(r1 != r3);
    }

    // Test case independence and order independence of parameters.
    {
        CIMObjectPath r1 = CIMObjectPath ("X.a=123,b=true");
        CIMObjectPath r2 = CIMObjectPath ("x.B=TRUE,A=123");
        PEGASUS_TEST_ASSERT(r1 == r2);
        PEGASUS_TEST_ASSERT(r1.makeHashCode() == r2.makeHashCode());

        CIMObjectPath r3 = CIMObjectPath ("x.B=TRUE,A=123,c=FALSE");
        PEGASUS_TEST_ASSERT(r1 != r3);
        String keyValue;

        Array<CIMKeyBinding> kbArray;
        {
            Boolean found = false;
            kbArray = r3.getKeyBindings();
            for (Uint32 i = 0; i < kbArray.size(); i++)
            {
                if (verbose)
                {
                    cout << "keyName= " <<  kbArray[i].getName().getString()
                         << " Value= " << kbArray[i].getValue() << endl;
                }
                if ( kbArray[i].getName() == CIMName ("B") )
                {
                    keyValue = kbArray[i].getValue();
                    if(keyValue == "TRUE")
                    found = true;
                }
            }
            if(!found)
            {
                cerr << "Key Binding Test error " << endl;
                exit(1);
            }
            //ATTN: KS 12 May 2002 P3 DEFER - keybinding manipulation. too
            // simplistic.
            // This code demonstrates that it is not easy to manipulate and
            // test keybindings.  Needs better tool both in CIMObjectPath and
            // separate.
        }
    }


    // Test building from component parts of CIM Reference.
    {
        CIMObjectPath r1 ("atp:77", CIMNamespaceName ("root/cimv25"),
            CIMName ("TennisPlayer"));
        CIMObjectPath r2 ("//atp:77/root/cimv25:TennisPlayer.");
        //cout << "r1 " << r1.toString() << endl;
        //cout << "r2 " << r2.toString() << endl;

        PEGASUS_TEST_ASSERT(r1 == r2);
        PEGASUS_TEST_ASSERT(r1.toString() == r2.toString());

    }


    {
        String hostName = "atp:77";
        String nameSpace = "root/cimv2";
        String className = "tennisplayer";

        CIMObjectPath r1;
        r1.setHost(hostName);
        r1.setNameSpace(nameSpace);
        r1.setClassName(className);
        PEGASUS_TEST_ASSERT(r1.getClassName().equal(CIMName ("TENNISPLAYER")));
        PEGASUS_TEST_ASSERT(!r1.getClassName().equal(CIMName ("blob")));


        String newHostName = r1.getHost();
        //cout << "HostName = " << newHostName << endl;

        CIMObjectPath r2 (hostName, nameSpace, className);
        PEGASUS_TEST_ASSERT(r1 == r2);
    }

    // Test cases for the Hostname.  CIMObjectPaths allows the
    // host to include the domain. Eg. xyz.company.com

    // First, try a good hostname
    CIMObjectPath h0("//usoPen-9.ustA-1-a.org:77/root/cimv25:"
        "TennisPlayer.first=\"Chris\",last=\"Evert\"");
    CIMObjectPath h1("//usoPen-9:77/root/cimv25:"
        "TennisPlayer.first=\"Chris\",last=\"Evert\"");
    CIMObjectPath h2("//usoPen-9/root/cimv25:"
        "TennisPlayer.first=\"Chris\",last=\"Evert\"");
    CIMObjectPath h3("//usoPen-9.ustA-1-a.org:0/root/cimv25:"
        "TennisPlayer.first=\"Chris\",last=\"Evert\"");
    CIMObjectPath h4("//usoPen-9.ustA-1-a.org:9876/root/cimv25:"
        "TennisPlayer.first=\"Chris\",last=\"Evert\"");
    CIMObjectPath h5("//usoPen-9.ustA-1-a.org:65535/root/cimv25:"
        "TennisPlayer.first=\"Chris\",last=\"Evert\"");
    CIMObjectPath h6("//usopen-9.usta-1-a.1org:77/root/cimv25:"
        "TennisPlayer.first=\"Chris\",last=\"Evert\"");
    CIMObjectPath h7("//192.168.1.com:77/root/cimv25:"
        "TennisPlayer.first=\"Chris\",last=\"Evert\"");
    CIMObjectPath h8("//192.168.0.org/root/cimv25:"
        "TennisPlayer.first=\"Chris\",last=\"Evert\"");
    CIMObjectPath h9("//192.168.1.80.com:77/root/cimv25:"
        "TennisPlayer.first=\"Chris\",last=\"Evert\"");
    CIMObjectPath h10("//192.168.0.80.org/root/cimv25:"
        "TennisPlayer.first=\"Chris\",last=\"Evert\"");
    CIMObjectPath h11("//192.168.1.80.255.com:77/root/cimv25:"
        "TennisPlayer.first=\"Chris\",last=\"Evert\"");
    CIMObjectPath h12("//192.168.0.80.254.org/root/cimv25:"
        "TennisPlayer.first=\"Chris\",last=\"Evert\"");
    CIMObjectPath h13("//192.168.257.80.com:77/root/cimv25:"
        "TennisPlayer.first=\"Chris\",last=\"Evert\"");
    CIMObjectPath h14("//192.256.0.80.org/root/cimv25:"
        "TennisPlayer.first=\"Chris\",last=\"Evert\"");

    CIMObjectPath h15("//localhost/root/cimv25:"
        "TennisPlayer.first=\"Chris\",last=\"Evert\"");
    CIMObjectPath h16("//ou812/root/cimv25:"
        "TennisPlayer.first=\"Chris\",last=\"Evert\"");
    CIMObjectPath h17("//u812/root/cimv25:"
        "TennisPlayer.first=\"Chris\",last=\"Evert\"");

    // Hostname with '_' character support checks, see bug#2556.
    CIMObjectPath h18("//_atp:9999/_root/_cimv25:_TennisPlayer");
    CIMObjectPath h19("//a_tp/_root/_cimv25:_TennisPlayer");
    CIMObjectPath h20("//atp_:9999/_root/_cimv25:_TennisPlayer");
    CIMObjectPath h21("//atp_-9:9999/_root/_cimv25:_TennisPlayer");
    CIMObjectPath h22(
        "//_a_t_p_-9.ustA-1-a.org:9999/_root/_cimv25:_TennisPlayer");
    CIMObjectPath h23("//_/root/cimv25:_TennisPlayer");
    CIMObjectPath h24("//_______/root/cimv25:_TennisPlayer");

    // try IPAddress as hostname which should be good
    CIMObjectPath h_ip0("//192.168.1.80:77/root/cimv25:"
        "TennisPlayer.first=\"Chris\",last=\"Evert\"");
    CIMObjectPath h_ip1("//192.168.0.255/root/cimv25:"
        "TennisPlayer.first=\"Chris\",last=\"Evert\"");

    // Try IPv6 Addresses.
    CIMObjectPath ip6_1("//[::1]:77/root/cimv25:"
        "TennisPlayer.first=\"Chris\",last=\"Evert\"");

    CIMObjectPath ip6_2("//[::ffff:192.1.2.3]:77/root/cimv25:"
        "TennisPlayer.first=\"Chris\",last=\"Evert\"");

    CIMObjectPath ip6_3("//[fffe:233:321:234d:e45:fad4:78:12]:77/root/cimv25:"
        "TennisPlayer.first=\"Chris\",last=\"Evert\"");

    CIMObjectPath ip6_4("//[fffe::]:77/root/cimv25:"
        "TennisPlayer.first=\"Chris\",last=\"Evert\"");


    Boolean errorDetected = false;

    // Invalid IPV6 Addresses
    try
    { // IPv6 addresses must be enclosed in brackets
        CIMObjectPath ip6_mb("//fffe::12ef:127/root/cimv25:"
            "TennisPlayer.first=\"Chris\",last=\"Evert\"");
    }
    catch (const Exception&)
    {
       errorDetected = true;
    }
    PEGASUS_TEST_ASSERT(errorDetected);

    errorDetected = false;
    try
    { // IPv6 address invalid
        CIMObjectPath ip6_invalid("//[fffe::sd:77]/root/cimv25:"
            "TennisPlayer.first=\"Chris\",last=\"Evert\"");
    }
    catch (const Exception&)
    {
       errorDetected = true;
    }
    PEGASUS_TEST_ASSERT(errorDetected);

    errorDetected = false;
    try
    {
       //Port number out of range.
       CIMObjectPath h_Port("//usoPen-9.ustA-1-a.org:9876543210/root/cimv25:"
           "TennisPlayer.first=\"Chris\",last=\"Evert\"");
    }
    catch (const Exception&)
    {
       errorDetected = true;
    }
    PEGASUS_TEST_ASSERT(errorDetected);

    errorDetected = false;
    try
    {
       //Port number out of range.
       CIMObjectPath h_Port("//usoPen-9.ustA-1-a.org:65536/root/cimv25:"
           "TennisPlayer.first=\"Chris\",last=\"Evert\"");
    }
    catch (const Exception&)
    {
       errorDetected = true;
    }
    PEGASUS_TEST_ASSERT(errorDetected);

    errorDetected = false;
    try
    {
       //Port number out of range.
       CIMObjectPath h_Port("//usoPen-9.ustA-1-a.org:100000/root/cimv25:"
           "TennisPlayer.first=\"Chris\",last=\"Evert\"");
    }
    catch (const Exception&)
    {
       errorDetected = true;
    }
    PEGASUS_TEST_ASSERT(errorDetected);

    errorDetected = false;
    try
    {
        //more than three digits in an octect
        CIMObjectPath h_ErrIp0("//192.1600008.1.80:77/root/cimv25:"
            "TennisPlayer.first=\"Chris\",last=\"Evert\"");
    }
    catch (const Exception&)
    {
        errorDetected = true;
    }
    PEGASUS_TEST_ASSERT(errorDetected);

    errorDetected = false;
    try
    {
        // Octet out of range
        CIMObjectPath op("//192.168.256.80:77/root/cimv25:"
            "TennisPlayer.first=\"Chris\",last=\"Evert\"");
    }
    catch (const Exception&)
    {
        errorDetected = true;
    }
    PEGASUS_TEST_ASSERT(errorDetected);

    errorDetected = false;
    try
    {
        // Missing port is okay, needs be ignored
        CIMObjectPath op("//192.168.1.80:/root/cimv25:"
            "TennisPlayer.first=\"Chris\",last=\"Evert\"");
    }
    catch (const Exception&)
    {
        errorDetected = true;
    }
    PEGASUS_TEST_ASSERT(!errorDetected);

    errorDetected = false;
    try
    {
        // Too many octets
        CIMObjectPath op("//192.168.1.80.12/root/cimv25:"
            "TennisPlayer.first=\"Chris\",last=\"Evert\"");
    }
    catch (const Exception&)
    {
        errorDetected = true;
    }
    PEGASUS_TEST_ASSERT(errorDetected);

    errorDetected = false;
    try
    {
        // Too few octets
        CIMObjectPath op("//192.168.80:77/root/cimv25:"
            "TennisPlayer.first=\"Chris\",last=\"Evert\"");
    }
    catch (const Exception&)
    {
        errorDetected = true;
    }
    PEGASUS_TEST_ASSERT(errorDetected);

    errorDetected = false;
    try
    {
        // Missing port is okay, needs be ignored
        CIMObjectPath op("//usopen-9.usta-1-a.org:/root/cimv25:"
            "TennisPlayer.first=\"Chris\",last=\"Evert\"");
    }
    catch (Exception&)
    {
        errorDetected = true;
    }
    PEGASUS_TEST_ASSERT(!errorDetected);

    errorDetected = false;
    try
    {
        // Hostname (IP) without trailing '/' (with port)
        CIMObjectPath op("//192.168.256.80:77");
    }
    catch (const Exception&)
    {
        errorDetected = true;
    }
    PEGASUS_TEST_ASSERT(errorDetected);

    errorDetected = false;
    try
    {
        // Hostname (IP) without trailing '/' (without port)
        CIMObjectPath op("//192.168.256.80");
    }
    catch (const Exception&)
    {
        errorDetected = true;
    }
    PEGASUS_TEST_ASSERT(errorDetected);

    errorDetected = false;
    try
    {
        // Hostname without trailing '/' (with port)
        CIMObjectPath op("//usopen-9.usta-1-a.org:77");
    }
    catch (const Exception&)
    {
        errorDetected = true;
    }
    PEGASUS_TEST_ASSERT(errorDetected);

    errorDetected = false;
    try
    {
        // Hostname without trailing '/' (without port)
        CIMObjectPath op("//usopen-9.usta-1-a.org");
    }
    catch (const Exception&)
    {
        errorDetected = true;
    }
    PEGASUS_TEST_ASSERT(errorDetected);

    errorDetected = false;
    try
    {
        // Invalid first character
        CIMObjectPath op("//+usopen-9.usta-1-a.1org:77/root/cimv25:"
            "TennisPlayer.first=\"Chris\",last=\"Evert\"");
    }
    catch (Exception&)
    {
        errorDetected = true;
    }
    PEGASUS_TEST_ASSERT(errorDetected);

    errorDetected = false;
    try
    {
        // Non-alphanum char (?)
        CIMObjectPath op("//usopen-9.usta?-1-a.org:77/root/cimv25:"
            "TennisPlayer.first=\"Chris\",last=\"Evert\"");
    }
    catch (Exception&)
    {
        errorDetected = true;
    }
    PEGASUS_TEST_ASSERT(errorDetected);

    errorDetected = false;
    try
    {
        // Leading dot
        CIMObjectPath op("//.usopen-9.usta-1-a.org:77/root/cimv25:"
            "TennisPlayer.first=\"Chris\",last=\"Evert\"");
    }
    catch (Exception&)
    {
        errorDetected = true;
    }
    PEGASUS_TEST_ASSERT(errorDetected);

    errorDetected = false;
    try
    {
        // Dot in the wrong spot (before a -)
        CIMObjectPath op("//usopen.-9.usta-1-a.org:77/root/cimv25:"
            "TennisPlayer.first=\"Chris\",last=\"Evert\"");
    }
    catch (Exception&)
    {
        errorDetected = true;
    }
    PEGASUS_TEST_ASSERT(errorDetected);

    errorDetected = false;
    try
    {
        // Two dots in a row
        CIMObjectPath op("//usopen-9.usta-1-a..org:77/root/cimv25:"
            "TennisPlayer.first=\"Chris\",last=\"Evert\"");
    }
    catch (Exception&)
    {
        errorDetected = true;
    }
    PEGASUS_TEST_ASSERT(errorDetected);

    errorDetected = false;
    try
    {
        // Trailing dot
        CIMObjectPath op("//usopen-9.usta-1-a.org.:77/root/cimv25:"
            "TennisPlayer.first=\"Chris\",last=\"Evert\"");
    }
    catch (Exception&)
    {
        errorDetected = true;
    }
    PEGASUS_TEST_ASSERT(errorDetected);
}

void test02()
{
    // test cases with commas in the key value string

    CIMObjectPath testr1 = CIMObjectPath
        ("MyClass.z=true,y=1234,x=\"Hello,World\"");

    CIMObjectPath testr2 = CIMObjectPath
        ("MyClass.z=true,y=1234,x=\"Hello World,\"");

    CIMObjectPath testr3 = CIMObjectPath
        ("MyClass.z=true,y=1234,x=\"Hello,,World\"");

    CIMObjectPath testr4 = CIMObjectPath
        ("//atp:77/root/cimv25:test.last=\"Rafter,Smith.Jones"
         " long_name:any*char=any123%#@!<>?+^\",first=\"Patrick\"");

    // test cases with colon inside keybinding string value

    CIMObjectPath testc1 = CIMObjectPath
        ("MyClass.z=true,y=1234,x=\"Hello:World\"");

    Boolean colonException = false;
    try
    {
        CIMObjectPath testc2 = CIMObjectPath
            ("MyNamespace.ns:MyClass.z=true,y=1234,x=\"Hello:World\"");
    }
    catch (Exception&)
    {
        colonException = true;
    }
    PEGASUS_TEST_ASSERT(colonException);

    // test error cases

    Boolean errorDetected = false;
    try
    {
        CIMObjectPath testerr1 = CIMObjectPath
           ("myclass.X=\"Hello World\"Z=trueY=1234");
    }
    catch (Exception&)
    {
        errorDetected = true;
    }
    PEGASUS_TEST_ASSERT(errorDetected);

    errorDetected = false;
    try
    {
        CIMObjectPath testerr2 = CIMObjectPath ("myclass.XYZ");
    }
    catch (Exception&)
    {
        errorDetected = true;
    }
    PEGASUS_TEST_ASSERT(errorDetected);

    errorDetected = false;
    try
    {
        CIMObjectPath testerr3 = CIMObjectPath
            ("MyClass.z=true,y=1234abc,x=\"Hello World\"");
    }
    catch (Exception&)
    {
        errorDetected = true;
    }
    PEGASUS_TEST_ASSERT(errorDetected);

    errorDetected = false;
    try
    {
        CIMObjectPath testerr4 = CIMObjectPath
            ("MyClass.z=nottrue,y=1234,x=\"Hello World\"");
    }
    catch (Exception&)
    {
        errorDetected = true;
    }
    PEGASUS_TEST_ASSERT(errorDetected);
}

// Test CIMKeyBinding constructor (CIMValue variety) and equal(CIMValue) method
void test03()
{
    CIMKeyBinding kb0("test0", Real32(3.14159));
    PEGASUS_TEST_ASSERT(kb0.equal(Real32(3.14159)));
    PEGASUS_TEST_ASSERT(!kb0.equal(Real32(3.141593)));

    CIMKeyBinding kb1("test1", String("3.14159"), CIMKeyBinding::NUMERIC);
    PEGASUS_TEST_ASSERT(kb1.equal(Real32(3.14159)));
    PEGASUS_TEST_ASSERT(!kb1.equal(String("3.14159")));

    CIMKeyBinding kb2("test2", Uint32(1000));
    PEGASUS_TEST_ASSERT(kb2.equal(Uint32(1000)));
    PEGASUS_TEST_ASSERT(!kb2.equal(Uint32(1001)));
    PEGASUS_TEST_ASSERT(kb2.getValue() == "1000");

    CIMKeyBinding kb3("test3", Char16('X'));
    PEGASUS_TEST_ASSERT(kb3.equal(Char16('X')));
    PEGASUS_TEST_ASSERT(!kb3.equal(Char16('Y')));
    PEGASUS_TEST_ASSERT(kb3.getValue() == "X");

    CIMKeyBinding kb4("test4", CIMDateTime("19991224120000.000000+360"));
    PEGASUS_TEST_ASSERT(kb4.equal(CIMDateTime("19991224120000.000000+360")));
    PEGASUS_TEST_ASSERT(!kb4.equal(CIMDateTime("19991225120000.000000+360")));
    PEGASUS_TEST_ASSERT(kb4.getValue() == "19991224120000.000000+360");
    kb4.setValue("0");
    PEGASUS_TEST_ASSERT(!kb4.equal(CIMDateTime("19991224120000.000000+360")));

    CIMKeyBinding kb5("test5", String("StringTest"));
    PEGASUS_TEST_ASSERT(kb5.equal(String("StringTest")));
    PEGASUS_TEST_ASSERT(!kb5.equal(String("StringTest1")));
    PEGASUS_TEST_ASSERT(kb5.getValue() == "StringTest");

    CIMKeyBinding kb6("test6", Boolean(true));
    PEGASUS_TEST_ASSERT(kb6.equal(Boolean(true)));
    PEGASUS_TEST_ASSERT(!kb6.equal(Boolean(false)));
    PEGASUS_TEST_ASSERT(kb6.getValue() == "TRUE");
    kb6.setValue("true1");
    PEGASUS_TEST_ASSERT(!kb6.equal(Boolean(true)));

    CIMKeyBinding kb7("test7",
        CIMObjectPath("//atp:77/root/cimv25:TennisPlayer."
            "last=\"Rafter\",first=\"Patrick\""));

    String path = "//atp:77/root/cimv25:TennisPlayer."
                  "last=\"Rafter\",first=\"Patrick\"";
    PEGASUS_TEST_ASSERT(kb7.equal(CIMObjectPath(path)));

    path = "//atp:77/root/cimv25:TennisPlayer."
           "FIRST=\"Patrick\",LAST=\"Rafter\"";
    PEGASUS_TEST_ASSERT(kb7.equal(CIMObjectPath(path)));

    path = "//atp:77/root/cimv25:TennisPlayer.last=\"Rafter\"";
    PEGASUS_TEST_ASSERT(!kb7.equal(CIMObjectPath(path)));

    Boolean exceptionFlag = false;
    try
    {
        CIMKeyBinding kb8("test8", Array<Uint32>());
    }
    catch (TypeMismatchException&)
    {
        exceptionFlag = true;
    }
    PEGASUS_TEST_ASSERT(exceptionFlag);

    CIMKeyBinding kb9("test9", String("1000"), CIMKeyBinding::STRING);
    PEGASUS_TEST_ASSERT(!kb9.equal(Uint32(1000)));

    CIMKeyBinding kb10("test10", String("100"), CIMKeyBinding::NUMERIC);
    PEGASUS_TEST_ASSERT(kb10.equal(Uint64(100)));
    PEGASUS_TEST_ASSERT(kb10.equal(Uint32(100)));
    PEGASUS_TEST_ASSERT(kb10.equal(Uint16(100)));
    PEGASUS_TEST_ASSERT(kb10.equal(Uint8(100)));
    PEGASUS_TEST_ASSERT(kb10.equal(Sint64(100)));
    PEGASUS_TEST_ASSERT(kb10.equal(Sint32(100)));
    PEGASUS_TEST_ASSERT(kb10.equal(Sint16(100)));
    PEGASUS_TEST_ASSERT(kb10.equal(Sint8(100)));
    PEGASUS_TEST_ASSERT(!kb10.equal(String("100")));

    CIMKeyBinding kb11("test11", String("+100"), CIMKeyBinding::NUMERIC);
    // Unsigned ints may not start with "+"
    PEGASUS_TEST_ASSERT(!kb11.equal(Uint64(100)));
    PEGASUS_TEST_ASSERT(!kb11.equal(Uint32(100)));
    PEGASUS_TEST_ASSERT(!kb11.equal(Uint16(100)));
    PEGASUS_TEST_ASSERT(!kb11.equal(Uint8(100)));
    PEGASUS_TEST_ASSERT(kb11.equal(Sint64(100)));
    PEGASUS_TEST_ASSERT(kb11.equal(Sint32(100)));
    PEGASUS_TEST_ASSERT(kb11.equal(Sint16(100)));
    PEGASUS_TEST_ASSERT(kb11.equal(Sint8(100)));
    PEGASUS_TEST_ASSERT(!kb11.equal(String("100")));
}

//
//  Test identical() function with keys that are references
//
void test04()
{
    //
    // Create classes A and B referenced classes, C - Association
    //
    CIMClass classA (CIMName ("A"), CIMName ());
    CIMProperty propertyX ("x", String ());
    propertyX.addQualifier (CIMQualifier (CIMName ("Key"), true));
    CIMProperty propertyY ("y", String ());
    propertyY.addQualifier (CIMQualifier (CIMName ("Key"), true));
    CIMProperty propertyZ ("z", String ());
    propertyZ.addQualifier (CIMQualifier (CIMName ("Key"), true));
    classA.addProperty (propertyX);
    classA.addProperty (propertyY);
    classA.addProperty (propertyZ);

    CIMClass classB ("B");
    CIMProperty propertyQ ("q", String ());
    propertyQ.addQualifier (CIMQualifier (CIMName ("Key"), true));
    CIMProperty propertyR ("r", String ());
    propertyR.addQualifier (CIMQualifier (CIMName ("Key"), true));
    CIMProperty propertyS ("s", String ());
    propertyS.addQualifier (CIMQualifier (CIMName ("Key"), true));
    classB.addProperty (propertyQ);
    classB.addProperty (propertyR);
    classB.addProperty (propertyS);

    CIMClass classC ("C");
    CIMProperty propertyA ("a", CIMValue ());
    propertyA.addQualifier (CIMQualifier (CIMName ("Key"), true));
    CIMProperty propertyB ("b", CIMValue ());
    propertyB.addQualifier (CIMQualifier (CIMName ("Key"), true));
    classC.addProperty (propertyA);
    classC.addProperty (propertyB);

    //
    //  Create instances of each classa
    //
    CIMInstance instanceA (CIMName ("A"));
    instanceA.addProperty (CIMProperty (CIMName ("x"), String ("rose")));
    instanceA.addProperty (CIMProperty (CIMName ("y"), String ("lavender")));
    instanceA.addProperty (CIMProperty (CIMName ("z"), String ("rosemary")));
    CIMObjectPath aPath = instanceA.buildPath (classA);
    CIMObjectPath aPath2 ("A.y=\"lavender\",x=\"rose\",z=\"rosemary\"");
    PEGASUS_TEST_ASSERT (aPath.identical (aPath2));

    CIMInstance instanceB (CIMName ("B"));
    instanceB.addProperty (CIMProperty (CIMName ("q"),
        String ("pelargonium")));
    instanceB.addProperty (CIMProperty (CIMName ("r"), String ("thyme")));
    instanceB.addProperty (CIMProperty (CIMName ("s"), String ("sage")));

    // Test to assure that the buildpath function works.
    CIMObjectPath bPath = instanceB.buildPath (classB);
    CIMObjectPath bPath2 ("B.s=\"sage\",q=\"pelargonium\",r=\"thyme\"");
    PEGASUS_TEST_ASSERT (bPath.identical (bPath2));

    // Build instance of C and build path from buildPath function.
    CIMInstance instanceC (CIMName ("C"));
    instanceC.addProperty (CIMProperty (CIMName ("a"), aPath, 0,
        CIMName ("A")));
    instanceC.addProperty (CIMProperty (CIMName ("b"), bPath, 0,
        CIMName ("B")));
    CIMObjectPath cPath = instanceC.buildPath (classC);

    // Build CIMObjectPath from keybindings.
    Array <CIMKeyBinding> keyBindings;
    CIMKeyBinding aBinding ("a", "A.y=\"lavender\",x=\"rose\",z=\"rosemary\"",
        CIMKeyBinding::REFERENCE);
    CIMKeyBinding bBinding ("b", "B.s=\"sage\",q=\"pelargonium\",r=\"thyme\"",
        CIMKeyBinding::REFERENCE);
    keyBindings.append (aBinding);
    keyBindings.append (bBinding);

    CIMObjectPath cPath2 ("", CIMNamespaceName (),
        cPath.getClassName (), keyBindings);

    // Assert that the CIMObjectPaths for C from build path and direct
    // from keybindings are equal.
    PEGASUS_TEST_ASSERT (cPath.identical (cPath2));

    // ATTN: KS 25 Feb 2003 P3 - Think we can extend these tests
    // since this is creation of classes and
    // instnaces for associations and referenced classes.
}

// Test handling of escape characters
void test05()
{
    {
        // Test '\' and '"' characters in a key value
        // This represents MyClass.key1="\\\"\"\\",key2="\"\"\"\"\\\\\\\\"
        String s1 = "MyClass.key1=\"\\\\\\\"\\\"\\\\\","
                "key2=\"\\\"\\\"\\\"\\\"\\\\\\\\\\\\\\\\\"";
        CIMObjectPath r1 = s1;
        PEGASUS_TEST_ASSERT(r1.toString() == s1);
    }

    // Catch invalid escape sequences in a key value
    Boolean errorDetected;

    // Invalid trailing backslash
    errorDetected = false;
    try
    {
        CIMObjectPath r1("MyClass.key1=\"\\\"");
    }
    catch (const MalformedObjectNameException&)
    {
       errorDetected = true;
    }
    PEGASUS_TEST_ASSERT(errorDetected);

    // Invalid "\n" sequence
    errorDetected = false;
    try
    {
        CIMObjectPath r1("MyClass.key1=\"\\n\"");
    }
    catch (const MalformedObjectNameException&)
    {
       errorDetected = true;
    }
    PEGASUS_TEST_ASSERT(errorDetected);

    // Invalid hex sequence
    errorDetected = false;
    try
    {
        CIMObjectPath r1("MyClass.key1=\"\\x000A\"");
    }
    catch (const MalformedObjectNameException&)
    {
       errorDetected = true;
    }
    PEGASUS_TEST_ASSERT(errorDetected);
}

// Test CIMKeyBinding operator==
void test06()
{
    CIMKeyBinding kb1;
    CIMKeyBinding kb2;

    // Key bindings of different types are not equal
    kb1 = CIMKeyBinding("a", "true", CIMKeyBinding::BOOLEAN);
    kb2 = CIMKeyBinding("a", "true", CIMKeyBinding::STRING);
    PEGASUS_TEST_ASSERT(!(kb1 == kb2));

    // Key bindings with different names are not equal
    kb1 = CIMKeyBinding("a", "true", CIMKeyBinding::BOOLEAN);
    kb2 = CIMKeyBinding("b", "true", CIMKeyBinding::BOOLEAN);
    PEGASUS_TEST_ASSERT(!(kb1 == kb2));

    // Key bindings names are not case sensitive
    kb1 = CIMKeyBinding("a", "true", CIMKeyBinding::BOOLEAN);
    kb2 = CIMKeyBinding("A", "true", CIMKeyBinding::BOOLEAN);
    PEGASUS_TEST_ASSERT(kb1 == kb2);

    // Boolean key bindings are not case sensitive
    kb1 = CIMKeyBinding("a", "true", CIMKeyBinding::BOOLEAN);
    kb2 = CIMKeyBinding("a", "TrUe", CIMKeyBinding::BOOLEAN);
    PEGASUS_TEST_ASSERT(kb1 == kb2);

    // Boolean key bindings are not equal if they differ other than in case
    kb1 = CIMKeyBinding("a", "true", CIMKeyBinding::BOOLEAN);
    kb2 = CIMKeyBinding("a", "truee", CIMKeyBinding::BOOLEAN);
    PEGASUS_TEST_ASSERT(!(kb1 == kb2));

    // Numeric key binding comparisons are done based on integer values
    kb1 = CIMKeyBinding("a", "14", CIMKeyBinding::NUMERIC);
    kb2 = CIMKeyBinding("a", "14", CIMKeyBinding::NUMERIC);
    PEGASUS_TEST_ASSERT(kb1 == kb2);

    kb1 = CIMKeyBinding("a", "14", CIMKeyBinding::NUMERIC);
    kb2 = CIMKeyBinding("a", "140", CIMKeyBinding::NUMERIC);
    PEGASUS_TEST_ASSERT(!(kb1 == kb2));

    kb1 = CIMKeyBinding("a", "+14", CIMKeyBinding::NUMERIC);
    kb2 = CIMKeyBinding("a", "14", CIMKeyBinding::NUMERIC);
    PEGASUS_TEST_ASSERT(kb1 == kb2);

    kb1 = CIMKeyBinding("a", "0", CIMKeyBinding::NUMERIC);
    kb2 = CIMKeyBinding("a", "-0", CIMKeyBinding::NUMERIC);
    PEGASUS_TEST_ASSERT(kb1 == kb2);

    // Hexadecimal format is also understood
    kb1 = CIMKeyBinding("a", "14", CIMKeyBinding::NUMERIC);
    kb2 = CIMKeyBinding("a", "0x0E", CIMKeyBinding::NUMERIC);
    PEGASUS_TEST_ASSERT(kb1 == kb2);

    kb1 = CIMKeyBinding("a", "0x0E", CIMKeyBinding::NUMERIC);
    kb2 = CIMKeyBinding("a", "0x0e", CIMKeyBinding::NUMERIC);
    PEGASUS_TEST_ASSERT(kb1 == kb2);

    // Numeric key bindings are not equal if they differ other than in case
    kb1 = CIMKeyBinding("a", "true", CIMKeyBinding::NUMERIC);
    kb2 = CIMKeyBinding("a", "truee", CIMKeyBinding::NUMERIC);
    PEGASUS_TEST_ASSERT(!(kb1 == kb2));

    // String comparison is used if numeric key binding is not a valid integer
    kb1 = CIMKeyBinding("a", "14a", CIMKeyBinding::NUMERIC);
    kb2 = CIMKeyBinding("a", "14a", CIMKeyBinding::NUMERIC);
    PEGASUS_TEST_ASSERT(kb1 == kb2);

    kb1 = CIMKeyBinding("a", "14", CIMKeyBinding::NUMERIC);
    kb2 = CIMKeyBinding("a", "14a", CIMKeyBinding::NUMERIC);
    PEGASUS_TEST_ASSERT(!(kb1 == kb2));

    // String key bindings are compared using a simple string comparison
    kb1 = CIMKeyBinding("a", "My String", CIMKeyBinding::STRING);
    kb2 = CIMKeyBinding("a", "My String", CIMKeyBinding::STRING);
    PEGASUS_TEST_ASSERT(kb1 == kb2);

    kb1 = CIMKeyBinding("a", "My String", CIMKeyBinding::STRING);
    kb2 = CIMKeyBinding("a", "my string", CIMKeyBinding::STRING);
    PEGASUS_TEST_ASSERT(!(kb1 == kb2));

    // Reference key bindings are compared as CIMObjectPath objects
    kb1 = CIMKeyBinding("a", "aClass.key1=true,key2=256",
                        CIMKeyBinding::REFERENCE);
    kb2 = CIMKeyBinding("a", "AClass.Key2=0x100,Key1=TRUE",
                        CIMKeyBinding::REFERENCE);
    PEGASUS_TEST_ASSERT(kb1 == kb2);

    kb1 = CIMKeyBinding("a", "aClass.key1=\"true\"",
                        CIMKeyBinding::REFERENCE);
    kb2 = CIMKeyBinding("a", "AClass.Key1=\"TRUE\"",
                        CIMKeyBinding::REFERENCE);
    PEGASUS_TEST_ASSERT(!(kb1 == kb2));

    kb1 = CIMKeyBinding("a", "a.ref=\"aClass.key1=true,key2=256\"",
                        CIMKeyBinding::REFERENCE);
    kb2 = CIMKeyBinding("a", "a.ref=\"AClass.Key2=0x100,Key1=TRUE\"",
                        CIMKeyBinding::REFERENCE);
    PEGASUS_TEST_ASSERT(kb1 == kb2);

    kb1 = CIMKeyBinding("a", "//myHost/ns1/ns2:aClass.key1=1",
                        CIMKeyBinding::REFERENCE);
    kb2 = CIMKeyBinding("a", "//MyHost/NS1/NS2:AClass.key1=0x1",
                        CIMKeyBinding::REFERENCE);
    PEGASUS_TEST_ASSERT(kb1 == kb2);

    kb1 = CIMKeyBinding("a", "//myHost/ns1/ns2:aClass.key1=1",
                        CIMKeyBinding::REFERENCE);
    kb2 = CIMKeyBinding("a", "ns1/ns2:aClass.key1=1",
                        CIMKeyBinding::REFERENCE);
    PEGASUS_TEST_ASSERT(!(kb1 == kb2));

    kb1 = CIMKeyBinding("a", "ns1/ns2:aClass.key1=1",
                        CIMKeyBinding::REFERENCE);
    kb2 = CIMKeyBinding("a", "aClass.key1=1",
                        CIMKeyBinding::REFERENCE);
    PEGASUS_TEST_ASSERT(!(kb1 == kb2));

    kb1 = CIMKeyBinding("a", "aClass.key1=1,key2=2",
                        CIMKeyBinding::REFERENCE);
    kb2 = CIMKeyBinding("a", "aClass.key1=1",
                        CIMKeyBinding::REFERENCE);
    PEGASUS_TEST_ASSERT(!(kb1 == kb2));

    // String comparison is used if numeric key binding is not a valid integer
    kb1 = CIMKeyBinding("a", "Bad Reference", CIMKeyBinding::REFERENCE);
    kb2 = CIMKeyBinding("a", "Bad Reference", CIMKeyBinding::REFERENCE);
    PEGASUS_TEST_ASSERT(kb1 == kb2);

    kb1 = CIMKeyBinding("a", "Bad Reference", CIMKeyBinding::REFERENCE);
    kb2 = CIMKeyBinding("a", "bad reference", CIMKeyBinding::REFERENCE);
    PEGASUS_TEST_ASSERT(!(kb1 == kb2));
}

// Test hash code generation
void test07()
{
    CIMObjectPath op1;
    CIMObjectPath op2;

    // Hostname case does not affect the hash code
    op1 = CIMObjectPath("//myhost/ns1/ns2:aClass.key1=1");
    op2 = CIMObjectPath("//MyHost/ns1/ns2:aClass.key1=1");
    PEGASUS_TEST_ASSERT(op1.makeHashCode() == op2.makeHashCode());

    // Namespace case does not affect the hash code
    op1 = CIMObjectPath("//myhost/ns1/ns2:aClass.key1=1");
    op2 = CIMObjectPath("//myhost/NS1/Ns2:aClass.key1=1");
    PEGASUS_TEST_ASSERT(op1.makeHashCode() == op2.makeHashCode());

    // Class name case does not affect the hash code
    op1 = CIMObjectPath("//myhost/ns1/ns2:aClass.key1=1");
    op2 = CIMObjectPath("//myhost/ns1/ns2:ACLASS.key1=1");
    PEGASUS_TEST_ASSERT(op1.makeHashCode() == op2.makeHashCode());

    //
    // Equivalent key bindings generate the same hash code
    //
    Array<CIMKeyBinding> kba1;
    Array<CIMKeyBinding> kba2;

    // Key name case does not affect the hash code
    kba1.clear();
    kba2.clear();
    kba1.append(CIMKeyBinding("key1", "1", CIMKeyBinding::NUMERIC));
    kba2.append(CIMKeyBinding("KeY1", "1", CIMKeyBinding::NUMERIC));
    op1.set("myhost", CIMNamespaceName("ns1/ns2"), CIMName("aClass"), kba1);
    op2.set("myhost", CIMNamespaceName("ns1/ns2"), CIMName("aClass"), kba2);
    PEGASUS_TEST_ASSERT(op1.makeHashCode() == op2.makeHashCode());

    // Key binding order does not affect the hash code
    kba1.clear();
    kba2.clear();
    kba1.append(CIMKeyBinding("key1", "1", CIMKeyBinding::NUMERIC));
    kba1.append(CIMKeyBinding("key2", "2", CIMKeyBinding::NUMERIC));
    kba2.append(CIMKeyBinding("key2", "2", CIMKeyBinding::NUMERIC));
    kba2.append(CIMKeyBinding("key1", "1", CIMKeyBinding::NUMERIC));
    op1.set("myhost", CIMNamespaceName("ns1/ns2"), CIMName("aClass"), kba1);
    op2.set("myhost", CIMNamespaceName("ns1/ns2"), CIMName("aClass"), kba2);
    PEGASUS_TEST_ASSERT(op1.makeHashCode() == op2.makeHashCode());

    // Boolean key value case does not affect the hash code
    kba1.clear();
    kba2.clear();
    kba1.append(CIMKeyBinding("key1", "true", CIMKeyBinding::BOOLEAN));
    kba2.append(CIMKeyBinding("KeY1", "True", CIMKeyBinding::BOOLEAN));
    op1.set("myhost", CIMNamespaceName("ns1/ns2"), CIMName("aClass"), kba1);
    op2.set("myhost", CIMNamespaceName("ns1/ns2"), CIMName("aClass"), kba2);
    PEGASUS_TEST_ASSERT(op1.makeHashCode() == op2.makeHashCode());

    // Equivalent numeric key value forms do not affect the hash code
    kba1.clear();
    kba2.clear();
    kba1.append(CIMKeyBinding("key1", "1023", CIMKeyBinding::NUMERIC));
    kba2.append(CIMKeyBinding("KeY1", "0x3FF", CIMKeyBinding::NUMERIC));
    op1.set("myhost", CIMNamespaceName("ns1/ns2"), CIMName("aClass"), kba1);
    op2.set("myhost", CIMNamespaceName("ns1/ns2"), CIMName("aClass"), kba2);
    PEGASUS_TEST_ASSERT(op1.makeHashCode() == op2.makeHashCode());

    kba1.clear();
    kba2.clear();
    kba1.append(CIMKeyBinding("key1", "0", CIMKeyBinding::NUMERIC));
    kba2.append(CIMKeyBinding("KeY1", "-0", CIMKeyBinding::NUMERIC));
    op1.set("myhost", CIMNamespaceName("ns1/ns2"), CIMName("aClass"), kba1);
    op2.set("myhost", CIMNamespaceName("ns1/ns2"), CIMName("aClass"), kba2);
    PEGASUS_TEST_ASSERT(op1.makeHashCode() == op2.makeHashCode());

    // Invalid numeric key value forms generate a consistent hash code
    kba1.clear();
    kba2.clear();
    kba1.append(CIMKeyBinding("key1", "abc", CIMKeyBinding::NUMERIC));
    kba2.append(CIMKeyBinding("KeY1", "abc", CIMKeyBinding::NUMERIC));
    op1.set("myhost", CIMNamespaceName("ns1/ns2"), CIMName("aClass"), kba1);
    op2.set("myhost", CIMNamespaceName("ns1/ns2"), CIMName("aClass"), kba2);
    PEGASUS_TEST_ASSERT(op1.makeHashCode() == op2.makeHashCode());

    // Equivalent reference key value forms do not affect the hash code
    kba1.clear();
    kba2.clear();
    kba1.append(CIMKeyBinding("key1", "aClass.key1=true,key2=256",
                CIMKeyBinding::REFERENCE));
    kba2.append(CIMKeyBinding("KeY1", "AClass.Key2=0x100,Key1=TRUE",
                CIMKeyBinding::REFERENCE));
    op1.set("myhost", CIMNamespaceName("ns1/ns2"), CIMName("aClass"), kba1);
    op2.set("myhost", CIMNamespaceName("ns1/ns2"), CIMName("aClass"), kba2);
    PEGASUS_TEST_ASSERT(op1.makeHashCode() == op2.makeHashCode());

    kba1.clear();
    kba2.clear();
    kba1.append(CIMKeyBinding("key2", "a.ref=\"aClass.key1=true,key2=256\"",
                CIMKeyBinding::REFERENCE));
    kba2.append(CIMKeyBinding("Key2", "A.ref=\"AClass.Key2=0x100,Key1=TRUE\"",
                CIMKeyBinding::REFERENCE));
    op1.set("myhost", CIMNamespaceName("ns1/ns2"), CIMName("aClass"), kba1);
    op2.set("myhost", CIMNamespaceName("ns1/ns2"), CIMName("aClass"), kba2);
    PEGASUS_TEST_ASSERT(op1.makeHashCode() == op2.makeHashCode());

    kba1.clear();
    kba2.clear();
    kba1.append(CIMKeyBinding("key", "//myHost/ns1/ns2:aClass.key1=1",
                CIMKeyBinding::REFERENCE));
    kba2.append(CIMKeyBinding("Key", "//MyHost/NS1/NS2:AClass.key1=0x1",
                CIMKeyBinding::REFERENCE));
    op1.set("myhost", CIMNamespaceName("ns1/ns2"), CIMName("aClass"), kba1);
    op2.set("myhost", CIMNamespaceName("ns1/ns2"), CIMName("aClass"), kba2);
    PEGASUS_TEST_ASSERT(op1.makeHashCode() == op2.makeHashCode());

    // Kitchen sink
    kba1.clear();
    kba2.clear();
    kba1.append(CIMKeyBinding("bck", "true", CIMKeyBinding::BOOLEAN));
    kba1.append(CIMKeyBinding("rk2", "a.ref=\"aClass.key1=true,key2=256\"",
                CIMKeyBinding::REFERENCE));
    kba1.append(CIMKeyBinding("neK1", "1023", CIMKeyBinding::NUMERIC));
    kba1.append(CIMKeyBinding("SEk", "abc", CIMKeyBinding::STRING));
    kba1.append(CIMKeyBinding("rk3", "//myHost/ns1/ns2:aClass.key1=1",
                CIMKeyBinding::REFERENCE));
    kba1.append(CIMKeyBinding("NEk2", "0", CIMKeyBinding::NUMERIC));
    kba1.append(CIMKeyBinding("NiK", "abc", CIMKeyBinding::NUMERIC));
    kba1.append(CIMKeyBinding("rK1", "aClass.key1=true,key2=256",
                CIMKeyBinding::REFERENCE));
    kba2.append(CIMKeyBinding("rk1", "AClass.Key2=0x100,Key1=TRUE",
                CIMKeyBinding::REFERENCE));
    kba2.append(CIMKeyBinding("nek2", "-0", CIMKeyBinding::NUMERIC));
    kba2.append(CIMKeyBinding("BcK", "True", CIMKeyBinding::BOOLEAN));
    kba2.append(CIMKeyBinding("rk2", "A.ref=\"AClass.Key2=0x100,Key1=TRUE\"",
                CIMKeyBinding::REFERENCE));
    kba2.append(CIMKeyBinding("Nek1", "0x3FF", CIMKeyBinding::NUMERIC));
    kba2.append(CIMKeyBinding("Nik", "abc", CIMKeyBinding::NUMERIC));
    kba2.append(CIMKeyBinding("Sek", "abc", CIMKeyBinding::STRING));
    kba2.append(CIMKeyBinding("rk3", "//MyHost/NS1/NS2:AClass.key1=0x1",
                CIMKeyBinding::REFERENCE));
    op1.set("myhost", CIMNamespaceName("ns1/ns2"), CIMName("aClass"), kba1);
    op2.set("MyHOST", CIMNamespaceName("Ns1/NS2"), CIMName("Aclass"), kba2);
    PEGASUS_TEST_ASSERT(op1.makeHashCode() == op2.makeHashCode());
}

// Test non-ASCII character handling
void test08()
{
    CIMObjectPath op = CIMObjectPath("//myHost/ns1/ns2:aClass.key1=1");
    Boolean errorDetected;

    // Test non-ASCII first character of hostname
    errorDetected = false;
    try
    {
        String hostname = "123.123.123.123";
        hostname[0] = 0x131;
        op.setHost(hostname);
    }
    catch (const MalformedObjectNameException&)
    {
        errorDetected = true;
    }
    PEGASUS_TEST_ASSERT(errorDetected);

    // Test non-ASCII non-first character of IP address octet
    errorDetected = false;
    try
    {
        String hostname = "123.123.123.123";
        hostname[1] = 0x132;
        op.setHost(hostname);
    }
    catch (const MalformedObjectNameException&)
    {
        errorDetected = true;
    }
    PEGASUS_TEST_ASSERT(errorDetected);

    // Test non-ASCII first character of IP address octet
    errorDetected = false;
    try
    {
        String hostname = "123.123.123.123";
        hostname[4] = 0x131;
        op.setHost(hostname);
    }
    catch (const MalformedObjectNameException&)
    {
        errorDetected = true;
    }
    PEGASUS_TEST_ASSERT(errorDetected);

    // Test non-ASCII first character of hostname segment
    errorDetected = false;
    try
    {
        String hostname = "myhost.123.com";
        hostname[7] = 0x131;
        op.setHost(hostname);
    }
    catch (const MalformedObjectNameException&)
    {
        errorDetected = true;
    }
    PEGASUS_TEST_ASSERT(errorDetected);

    // Test non-ASCII non-first character of hostname segment
    errorDetected = false;
    try
    {
        String hostname = "myhost.123.com";
        hostname[8] = 0x132;
        op.setHost(hostname);
    }
    catch (const MalformedObjectNameException&)
    {
        errorDetected = true;
    }
    PEGASUS_TEST_ASSERT(errorDetected);

    // Test non-ASCII first character of port number
    errorDetected = false;
    try
    {
        String hostname = "myhost.123.com:1234";
        hostname[15] = 0x131;
        op.setHost(hostname);
    }
    catch (const MalformedObjectNameException&)
    {
        errorDetected = true;
    }
    PEGASUS_TEST_ASSERT(errorDetected);

    // Test non-ASCII non-first character of port number
    errorDetected = false;
    try
    {
        String hostname = "myhost.123.com:1234";
        hostname[18] = 0x134;
        op.setHost(hostname);
    }
    catch (const MalformedObjectNameException&)
    {
        errorDetected = true;
    }
    PEGASUS_TEST_ASSERT(errorDetected);
}

int main(int, char** argv)
{
    verbose = getenv("PEGASUS_TEST_VERBOSE") ? true : false;

    try
    {
        test01();
        test02();
        test03();
        test04();
        test05();
        test06();
        test07();
        test08();

        cout << argv[0] << " +++++ passed all tests" << endl;
    }
    catch (Exception& e)
    {
        cerr << argv[0] << " Exception " << e.getMessage() << endl;
        exit(1);
    }
    return 0;
}
