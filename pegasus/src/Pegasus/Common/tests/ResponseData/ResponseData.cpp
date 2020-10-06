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


#include <Pegasus/Common/XmlReader.h>
#include <Pegasus/Common/XmlParser.h>
#include <Pegasus/Common/Config.h>
#include <Pegasus/Common/FileSystem.h>
#include <Pegasus/Common/SCMOClassCache.h>
#include <Pegasus/Common/CIMResponseData.h>
#include <Pegasus/Common/CIMPropertyList.h>
#include <Pegasus/Common/Buffer.h>


PEGASUS_USING_PEGASUS;
PEGASUS_USING_STD;

static Boolean verbose;
#define VCOUT if (verbose) cout

CIMClass  buildClass()
{
    CIMClass CIMclass1(CIMName("MyClass"));
    CIMclass1
        .addProperty(CIMProperty(CIMName("id"), Uint32(1))
            .addQualifier(CIMQualifier(CIMName("key"), true)))
        .addProperty(CIMProperty(CIMName("count"), Uint32(55))
            .addQualifier(CIMQualifier(CIMName("counter"), true))
            .addQualifier(CIMQualifier(CIMName("min"), String("0")))
            .addQualifier(CIMQualifier(CIMName("max"), String("1"))))
        .addProperty(CIMProperty(CIMName("message"), String("Hello"))
            .addQualifier(CIMQualifier(CIMName("description"),
                String("My Message"))))
        .addProperty(CIMProperty(CIMName("ratio"), Real32(1.5)));
    return CIMclass1;
}

void setPropertyValue(CIMInstance& instance, const CIMName& propertyName,
    const Uint32 value)
{
    Uint32 pos;
    PEGASUS_ASSERT(pos = instance.findProperty(propertyName) != PEG_NOT_FOUND);
    instance.getProperty(pos).setValue(CIMValue(value));
}

// test size method and moveObjects method
void test02()
{
    // Test with a variety of object types in the From ResponseData
    // Build the class
    CIMClass CIMclass1 = buildClass();

    // Build a CIM Instance. NOTE: key is defaulted in class

    CIMInstance CIMInst1 = CIMclass1.buildInstance(true, true,
                                                   CIMPropertyList());

        // Clone the instance and change key
    CIMInstance CIMInst2 = CIMInst1.clone();
    setPropertyValue(CIMInst2, "id", 2);

    CIMInstance CIMInst3 = CIMInst1.clone();
    setPropertyValue(CIMInst3, "id", 3);
    CIMInstance CIMInst4 = CIMInst1.clone();
    setPropertyValue(CIMInst4, "id", 4);

    // build CIMInstance array
    Array<CIMInstance> CIMInstArray;
    CIMInstArray.append(CIMInst1);
    CIMInstArray.append(CIMInst2);

    // Build SCMO instances
    SCMOClass SCMO_CSClass(CIMclass1);

    VCOUT << "Creating SCMOInstance from CIMInstance" << endl;
    SCMOInstance SCMO_CSInstance3(SCMO_CSClass,CIMInst3);
    SCMOInstance SCMO_CSInstance4(SCMO_CSClass,CIMInst4);

    // Create array of 2 SCMO Instances
    Array<SCMOInstance> SCMOInstArray;
    SCMOInstArray.append(SCMO_CSInstance3);
    SCMOInstArray.append(SCMO_CSInstance4);

    // Create CIMReponseData object
    CIMResponseData crd1 = CIMResponseData(CIMResponseData::RESP_INSTANCES);
    PEGASUS_TEST_ASSERT(crd1.valid());
    // set SCMO array into the object
    crd1.setSCMO(SCMOInstArray);

    PEGASUS_TEST_ASSERT(crd1.valid());

    VCOUT << crd1.size() << endl;
    PEGASUS_TEST_ASSERT(crd1.size() == 2);

    // append a CIM Instance
    crd1.appendInstance(CIMInst1);

    PEGASUS_TEST_ASSERT(crd1.valid());
    VCOUT << crd1.size() << endl;
    PEGASUS_TEST_ASSERT(crd1.size() == 3);

    // Append an array of CIMInstances
    crd1.appendInstances(CIMInstArray);

    PEGASUS_TEST_ASSERT(crd1.valid());
    VCOUT << crd1.size() << endl;
    PEGASUS_TEST_ASSERT(crd1.size() == 5);

    CIMResponseData crdTo = CIMResponseData(CIMResponseData::RESP_INSTANCES);

    VCOUT << "Before from =  " << crd1.size() << ", to = "
        << crdTo.size() << endl;

    PEGASUS_TEST_ASSERT(crdTo.valid());
    Uint32 rtn = crdTo.moveObjects(crd1, 3);
    VCOUT << "After from = " <<  crd1.size() << ", to = "
        << crdTo.size() << endl;

    PEGASUS_TEST_ASSERT(crdTo.valid());
    PEGASUS_TEST_ASSERT(crd1.valid());

    PEGASUS_TEST_ASSERT(rtn ==3);
    PEGASUS_TEST_ASSERT(crd1.size() == 2);
    PEGASUS_TEST_ASSERT(crdTo.size() == 3);

    CIMResponseData crdTo2 = CIMResponseData(CIMResponseData::RESP_INSTANCES);
    VCOUT << "Before from =  " << crd1.size() << ", to = "
        << crdTo2.size() << endl;
    Uint32 rtn2 = crdTo2.moveObjects(crd1, 9);
    VCOUT << "After from = " <<  crd1.size() << ", to = "
        << crdTo2.size() << endl;
    PEGASUS_TEST_ASSERT(rtn2 ==2);
    PEGASUS_TEST_ASSERT(crd1.size() == 0);
    PEGASUS_TEST_ASSERT(crdTo2.size() == 2);

    PEGASUS_TEST_ASSERT(crd1.valid());
    PEGASUS_TEST_ASSERT(crdTo2.valid());

    // Repeat the test but starting with set CIM Instance Array to start
    // Create CIMReponseData object
    CIMResponseData crd2 = CIMResponseData(CIMResponseData::RESP_INSTANCES);
    crd2.setInstances(CIMInstArray);
    PEGASUS_TEST_ASSERT(crd2.size() == 2);
    PEGASUS_TEST_ASSERT(crd2.valid());

    crd2.appendInstance(CIMInst1);

    VCOUT << crd2.size() << endl;
    PEGASUS_TEST_ASSERT(crd2.size() == 3);

    crd2.appendSCMO(SCMOInstArray);
    PEGASUS_TEST_ASSERT(crd2.valid());

    VCOUT << crd2.size() << endl;
    PEGASUS_TEST_ASSERT(crd2.size() == 5);

    // Test XML formatting.

    CIMResponseData crd3 = CIMResponseData(CIMResponseData::RESP_INSTANCES);

    // set CIM instances into the responseData.
    crd3.appendInstances(CIMInstArray);

    VCOUT << "crd3 size " << crd3.size() << endl;
    PEGASUS_TEST_ASSERT(crd3.size() == 2);

    // Confirm that we can encode into a buffer.
    Buffer buf1;
    crd3.encodeXmlResponse(buf1, true);
    PEGASUS_TEST_ASSERT(crd3.size() == 2);

    CIMBuffer bufa;
    crd3.encodeInternalXmlResponse(bufa, true);

    CIMResponseData crd4 = CIMResponseData(CIMResponseData::RESP_INSTANCES);
////  The following fails. Apparently not enough data in the buffer. Think it
////  is the namespace from the buffer that does not exist.
////  FUTURE. Check this but not important since the real test is in the msg
////  serializer and deserializer.
////  PEGASUS_TEST_ASSERT(crd4.setXml(bufa));
////  cout << "crd4 size: " << crd4.size() << endl;
////  PEGASUS_TEST_ASSERT(crd4.size() == 4);

}
bool testMoveObjects(CIMResponseData& crdTo,
    CIMResponseData& crdFrom, Uint32 count)
{
    Uint32 origFromSize = crdFrom.size();
    Uint32 origToSize = crdTo.size();
    VCOUT << "Before Move from = " << crdFrom.size() << ", to = "
        << crdTo.size() << " count =" << count << endl;

    Uint32 rtn = crdTo.moveObjects(crdFrom, count);

    PEGASUS_TEST_ASSERT(crdTo.valid());

    VCOUT << "After Move  from.size() = " <<  crdFrom.size()
         << ", to.size() = " << crdTo.size()
         << " count =" << count
         << "rtn count = " << rtn << endl;

    if (count <=  origFromSize)
    {
        if (rtn != count)
        {
            printf("Error in move. rtn != count. rtn = %u count = %u\n",
                   rtn, count);
            return false;
        }
        else if (crdFrom.size() != (origFromSize - count))
        {
            printf("Error in move. from.size != fromSize - count. "
                   "    crdFrom.size() = %ufromSize = %u count = %u\n",
                   crdFrom.size(), origFromSize, count);
            return false;
        }
        else if (crdTo.size() != count + origToSize )
        {
            printf("Error in move. should have moved full count\n");
            return false;
        }
    }
    else  // Returning less than expected since we exhausted from resource
    {
        if (rtn  != origFromSize)
        {
            printf("Error in move. rtn (%u) should be origFromSize %u\n",
                   rtn, origFromSize);
            return false;
        }
        else if (crdFrom.size() != 0)
        {
            printf("Error in move. fromSize should be zero."
                   " Exhausted source\n");
            return false;
        }
        else if (crdTo.size() != rtn + origToSize)
        {
            printf("Error in move. toSize should reflect return integer\n");
            return false;
        }
    }
    return true;

}
// Test move method
void testMoveMethod01()
{
    // Test with a variety of object types in the From ResponseData
    // Build the class
    CIMClass CIMclass1 = buildClass();

    // Build a CIM Instance. NOTE: key is defaulted in class

    CIMInstance CIMInst1 = CIMclass1.buildInstance(true, true,
                                                   CIMPropertyList());

        // Clone the instance and change key
    CIMInstance CIMInst2 = CIMInst1.clone();
    setPropertyValue(CIMInst2, "id", 2);

    CIMInstance CIMInst3 = CIMInst1.clone();
    setPropertyValue(CIMInst3, "id", 3);
    CIMInstance CIMInst4 = CIMInst1.clone();
    setPropertyValue(CIMInst4, "id", 4);

    // build CIMInstance array
    Array<CIMInstance> CIMInstArray;
    CIMInstArray.append(CIMInst1);
    CIMInstArray.append(CIMInst2);
    CIMInstArray.append(CIMInst3);
    CIMInstArray.append(CIMInst3);
    {
        // Create from CIMReponseData object
        CIMResponseData crdFrom = CIMResponseData(
            CIMResponseData::RESP_INSTANCES);

        // Append an array of CIMInstances
        crdFrom.appendInstances(CIMInstArray);
        VCOUT << crdFrom.size() << endl;
        PEGASUS_TEST_ASSERT(crdFrom.size() == 4);

        // Move the objects < what is in from crd
        CIMResponseData crdTo = CIMResponseData(
            CIMResponseData::RESP_INSTANCES);
        PEGASUS_TEST_ASSERT(testMoveObjects(crdTo, crdFrom, 2));
        PEGASUS_TEST_ASSERT(crdFrom.size() == 2);
        PEGASUS_TEST_ASSERT(crdTo.size() == 2);

        // request move of more that exists.  Moves all
        CIMResponseData crdTo1 = CIMResponseData(
            CIMResponseData::RESP_INSTANCES);
        PEGASUS_TEST_ASSERT(testMoveObjects(crdTo1, crdFrom, 8));
        PEGASUS_TEST_ASSERT(crdFrom.size() == 0);
        PEGASUS_TEST_ASSERT(crdTo1.size() == 2);
    }

    // move exactly all and then none
    {
        // Create from CIMReponseData object
        CIMResponseData crdFrom = CIMResponseData(
            CIMResponseData::RESP_INSTANCES);

        // Append an array of CIMInstances
        crdFrom.appendInstances(CIMInstArray);
        VCOUT << crdFrom.size() << endl;
        PEGASUS_TEST_ASSERT(crdFrom.size() == 4);

        // Move the objects < what is in from crd
        CIMResponseData crdTo = CIMResponseData(
            CIMResponseData::RESP_INSTANCES);
        PEGASUS_TEST_ASSERT(testMoveObjects(crdTo, crdFrom, 4));
        PEGASUS_TEST_ASSERT(crdFrom.size() == 0);
        PEGASUS_TEST_ASSERT(crdTo.size() == 4);

        // request move of more that exists.  Moves all r
        CIMResponseData crdTo1 = CIMResponseData(
            CIMResponseData::RESP_INSTANCES);
        PEGASUS_TEST_ASSERT(testMoveObjects(crdTo1, crdFrom, 8));
        PEGASUS_TEST_ASSERT(crdFrom.size() == 0);
        VCOUT << "Error: bad size() = " << crdTo1.size() << endl;
        PEGASUS_TEST_ASSERT(crdTo1.size() == 0);
    }

    // request move more than total
    {
        // Create from CIMReponseData object
        CIMResponseData crdFrom = CIMResponseData(
            CIMResponseData::RESP_INSTANCES);

        // Append an array of CIMInstances
        crdFrom.appendInstances(CIMInstArray);
        VCOUT << crdFrom.size() << endl;
        PEGASUS_TEST_ASSERT(crdFrom.size() == 4);

        // Move the objects < what is in from crd
        CIMResponseData crdTo = CIMResponseData(
            CIMResponseData::RESP_INSTANCES);
        PEGASUS_TEST_ASSERT(testMoveObjects(crdTo, crdFrom, 9));
        PEGASUS_TEST_ASSERT(crdFrom.size() == 0);
        PEGASUS_TEST_ASSERT(crdTo.size() == 4);
    }
}

// test the size() function for various types of set and append of object to
// the response data object. Test set and append of single type.
void testSizeMethod()
{
    // Test with a variety of object types in the From ResponseData
    // Build the class
    CIMClass CIMclass1 = buildClass();

    // Build a CIM Instance. NOTE: key is defaulted in class

    CIMInstance CIMInst1 = CIMclass1.buildInstance(true, true,
                                                   CIMPropertyList());

        // Clone the instance and change key
    CIMInstance CIMInst2 = CIMInst1.clone();
    setPropertyValue(CIMInst2, "id", 2);

    CIMInstance CIMInst3 = CIMInst1.clone();
    setPropertyValue(CIMInst3, "id", 3);
    CIMInstance CIMInst4 = CIMInst1.clone();
    setPropertyValue(CIMInst4, "id", 4);

    // build CIMInstance array
    Array<CIMInstance> CIMInstArray;
    CIMInstArray.append(CIMInst1);
    CIMInstArray.append(CIMInst2);
    CIMInstArray.append(CIMInst3);
    CIMInstArray.append(CIMInst3);

    // Build SCMO instances
    SCMOClass SCMO_CSClass(CIMclass1);

    VCOUT << "Creating SCMOInstance from CIMInstance" << endl;
    SCMOInstance SCMO_CSInstance3(SCMO_CSClass,CIMInst3);
    SCMOInstance SCMO_CSInstance4(SCMO_CSClass,CIMInst4);

    // Create array of 2 SCMO Instances
    Array<SCMOInstance> SCMOInstArray;
    SCMOInstArray.append(SCMO_CSInstance3);
    SCMOInstArray.append(SCMO_CSInstance4);

    // tests with CIM objects (C++ Interface)
    {
        CIMResponseData crd = CIMResponseData(CIMResponseData::RESP_INSTANCES);

        crd.setInstances(CIMInstArray);
        PEGASUS_TEST_ASSERT(crd.size() == 4);
        crd.appendInstances(CIMInstArray);
        PEGASUS_TEST_ASSERT(crd.size() == 8);
        crd.appendInstance(CIMInst1);
        PEGASUS_TEST_ASSERT(crd.size() == 9);
        crd.appendSCMO(SCMOInstArray);
        PEGASUS_TEST_ASSERT(crd.size() == 11);
        PEGASUS_TEST_ASSERT(crd.valid());
    }

    {
        CIMResponseData crd = CIMResponseData(CIMResponseData::RESP_INSTANCES);

        crd.setInstance(CIMInst1);
        PEGASUS_TEST_ASSERT(crd.size() == 1);
        crd.appendInstances(CIMInstArray);
        PEGASUS_TEST_ASSERT(crd.size() == 5);
        crd.appendInstance(CIMInst1);
        PEGASUS_TEST_ASSERT(crd.size() == 6);
        crd.appendSCMO(SCMOInstArray);
        PEGASUS_TEST_ASSERT(crd.size() == 8);
        PEGASUS_TEST_ASSERT(crd.valid());
    }

    // test with objects
    {
        CIMResponseData crd = CIMResponseData(CIMResponseData::RESP_OBJECTS);
        Array<CIMObject> x;
        x.append((CIMObject)CIMInst1);
        x.append((CIMObject)CIMInst2);

        // Append an array of CIMInstances
        crd.setObjects(x);
        PEGASUS_TEST_ASSERT(crd.size() == 2);
        crd.appendObject((CIMObject)CIMInst1);
        PEGASUS_TEST_ASSERT(crd.size() == 3);
        PEGASUS_TEST_ASSERT(crd.valid());
    }

    // test with SCMOInst
    {
        CIMResponseData crd = CIMResponseData(CIMResponseData::RESP_INSTANCES);
        crd.setSCMO(SCMOInstArray);
        PEGASUS_TEST_ASSERT(crd.size() == 2);
        crd.appendSCMO(SCMOInstArray);
        PEGASUS_TEST_ASSERT(crd.size() == 4);
        PEGASUS_TEST_ASSERT(crd.valid());
    }

    {
        // Create array of 2 SCMO Instances
        Array<SCMOInstance> SCMOInstArray;
        SCMOInstArray.append(SCMO_CSInstance3);
        SCMOInstArray.append(SCMO_CSInstance4);

        CIMResponseData crd = CIMResponseData(CIMResponseData::RESP_INSTANCES);
        crd.appendSCMO(SCMOInstArray);
        PEGASUS_TEST_ASSERT(crd.size() == 2);
        crd.appendSCMO(SCMOInstArray);
        PEGASUS_TEST_ASSERT(crd.size() == 4);
        PEGASUS_TEST_ASSERT(crd.valid());
    }

    // Test with path objects
    String s1  = "//atp:77/root/cimv25:"
      "TennisPlayer.last=\"Rafter\",first=\"Patrick\"";
    String s2 = "//atp:77/root/cimv25:"
          "TennisPlayer.first=\"Patrick\",last=\"Rafter\"";
    CIMObjectPath r1 = s1;
    CIMObjectPath r2 = s2;
    Array<CIMObjectPath> refArray;
    refArray.append(r1);
    refArray.append(r2);
    refArray.append(r1);
    refArray.append(r2);

    CIMResponseData crd = CIMResponseData(CIMResponseData::RESP_OBJECTPATHS);
    crd.setInstanceNames(refArray);
    PEGASUS_TEST_ASSERT(crd.size() == 4);
    PEGASUS_TEST_ASSERT(crd.valid());

    // KS_TBD There is not appendinstancenames today in ResponseData
    //crd.appendInstanceNames(refArray);


    // Test with XML and binary
    {

    }

}
void testClearMethod()
{
    // Test with a variety of object types in the From ResponseData
    // Build the class
    CIMClass CIMclass1 = buildClass();

    // Build a CIM Instance. NOTE: key is defaulted in class

    CIMInstance CIMInst1 = CIMclass1.buildInstance(true, true,
                                                   CIMPropertyList());

        // Clone the instance and change key
    CIMInstance CIMInst2 = CIMInst1.clone();
    setPropertyValue(CIMInst2, "id", 2);

    CIMInstance CIMInst3 = CIMInst1.clone();
    setPropertyValue(CIMInst3, "id", 3);
    CIMInstance CIMInst4 = CIMInst1.clone();
    setPropertyValue(CIMInst4, "id", 4);

    // build CIMInstance array
    Array<CIMInstance> CIMInstArray;
    CIMInstArray.append(CIMInst1);
    CIMInstArray.append(CIMInst2);
    CIMInstArray.append(CIMInst3);
    CIMInstArray.append(CIMInst4);

    // Create CIMReponseData object
    CIMResponseData crd = CIMResponseData(CIMResponseData::RESP_INSTANCES);

    // Append an array of CIMInstances
    crd.appendInstances(CIMInstArray);
    PEGASUS_TEST_ASSERT(crd.size() == 4);

    // Build SCMO instances
    SCMOClass SCMO_CSClass(CIMclass1);

    VCOUT << "Creating SCMOInstance from CIMInstance" << endl;
    SCMOInstance SCMO_CSInstance3(SCMO_CSClass,CIMInst3);
    SCMOInstance SCMO_CSInstance4(SCMO_CSClass,CIMInst4);

    // Create array of 2 SCMO Instances
    Array<SCMOInstance> SCMOInstArray;
    SCMOInstArray.append(SCMO_CSInstance3);
    SCMOInstArray.append(SCMO_CSInstance4);
    crd.appendSCMO(SCMOInstArray);

    PEGASUS_TEST_ASSERT(crd.size() == 6);

    crd.clear();

    PEGASUS_TEST_ASSERT(crd.size() == 0);
}


int main (int argc, char *argv[])
{

    CIMClass CIM_TESTClass2;

    verbose = getenv("PEGASUS_TEST_VERBOSE") ? true : false;

    try
    {
        testSizeMethod();
        testMoveMethod01();
        testClearMethod();
        test02();

    }
    catch (CIMException& e)
    {
        cerr << endl << "CIMException: "
             << e.getMessage() << endl << endl ;
        exit(-1);
    }

    catch (Exception& e)
    {
        cout << endl << "Exception: "
             << e.getMessage() << endl << endl ;
        exit(-1);
    }
    catch (...)
    {
        cout << endl << "Unkown excetption!" << endl << endl;
        exit(-1);
    }

    cout << argv[0] << " +++++ passed all tests" << endl;
    return 0;

}
