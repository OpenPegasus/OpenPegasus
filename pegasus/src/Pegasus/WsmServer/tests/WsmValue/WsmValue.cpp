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
#include <stdio.h>

#include <Pegasus/Common/String.h>
#include <Pegasus/Common/PegasusAssert.h>
#include <Pegasus/Common/MessageLoader.h>

#include <Pegasus/WsmServer/WsmValue.h>
#include <Pegasus/WsmServer/WsmEndpointReference.h>
#include <Pegasus/WsmServer/WsmInstance.h>
#include <Pegasus/WsmServer/WsmSelectorSet.h>

PEGASUS_USING_PEGASUS;
PEGASUS_USING_STD;


static void _testStrings(void)
{
        {
            // Test defaul constructor
            WsmValue val;
            if (val.getType() != WSMTYPE_OTHER ||
                val.isArray() || !val.isNull())
                throw Exception("Invalid default value.");
        }

        {
            // Test string value constructor
            WsmValue val("Test string 1");
            if (val.getType() != WSMTYPE_OTHER ||
                val.isArray() || val.isNull())
                throw Exception("Invalid string value.");

            // Test string get()
            String str;
            val.get(str);
            if (str != "Test string 1")
                throw Exception("Invalid string from value get().");

            // Convert string value to string array value
            val.toArray();
            if (val.getType() != WSMTYPE_OTHER ||
                !val.isArray() || val.isNull() || val.getArraySize() != 1)
                throw Exception("Invalid string to array conversion.");

            // Make sure we still have the same string
            Array<String> stra;
            val.get(stra);
            if (stra[0] != "Test string 1")
                throw Exception("Invalid string array from value get().");

            // Add more strings to the array
            for (int i = 2; i < 5; i++)
            {
                char buf[20];
                sprintf(buf, "Test string %d", i);
                WsmValue tmp(buf);
                val.add(tmp);
            }

            // Make sure we can get the array + verify it's size
            val.get(stra);
            if (stra.size() != 4)
                throw Exception("Invalid string array size.");

            // Test that we have the right entries in the array
            for (int i = 1; i < 5; i++)
            {
                char buf[20];
                sprintf(buf, "Test string %d", i);

                if (stra[i-1] != buf)
                    throw Exception("Invalid string array entry.");
            }

            // Re-set the value from array to string
            val.set("New test string 1");
            if (val.getType() != WSMTYPE_OTHER ||
                val.isArray() || val.isNull())
                throw Exception("Invalid string value.");

            // Make sure set() worked correctly
            val.get(str);
            if (str != "New test string 1")
                throw Exception("Can't set string value.");

            // Add string values. The value must convert itself into
            // string array.
            for (int i = 2; i < 6; i++)
            {
                char buf[20];
                sprintf(buf, "New test string %d", i);
                WsmValue tmp(buf);
                val.add(tmp);
            }

             // Make sure we can get the array + verify it's size
            val.get(stra);
            if (stra.size() != 5)
                throw Exception("Invalid string array size.");

            // Test that we have the right entries in the array
            for (int i = 1; i < 6; i++)
            {
                char buf[20];
                sprintf(buf, "New test string %d", i);

                if (stra[i-1] != buf)
                    throw Exception("Invalid string array entry.");
            }

            // Test string array constructor array value set
            Array<String> tmp;
            for (int i = 1; i < 4; i++)
            {
                char buf[20];
                sprintf(buf, "Test string - %d", i);
                tmp.append(buf);
            }
            val.set(tmp);
            val.get(stra);

            // Test that we have the right entries in the array
            for (int i = 1; i < 4; i++)
            {
                char buf[20];
                sprintf(buf, "Test string - %d", i);

                if (stra[i-1] != buf)
                    throw Exception("Invalid string array entry.");
            }

            WsmValue val1(tmp);
            Array<String> stra1;
            val1.get(stra1);

            // Test that we have the right entries in the array
            for (int i = 1; i < 4; i++)
            {
                char buf[20];
                sprintf(buf, "Test string - %d", i);

                if (stra1[i-1] != buf)
                    throw Exception("Invalid string array entry.");
            }
       }
}

static void _createEPR(int idx, WsmEndpointReference& epr)
{
    char buf[20];
    sprintf(buf, "Address %d", idx);
    epr.address = buf;
    sprintf(buf, "resourceUri %d", idx);
    epr.resourceUri = buf;
}

static Boolean _compareEPRs(WsmEndpointReference& epr1,
    WsmEndpointReference& epr2)
{
    if (epr1.address != epr2.address ||
        epr1.resourceUri != epr2.resourceUri ||
        epr1.selectorSet->selectors.size() !=
        epr2.selectorSet->selectors.size())
        return false;

    return true;
}

static Boolean _verifyEPR(int idx, WsmEndpointReference& epr)
{
    WsmEndpointReference tmp;
    _createEPR(idx, tmp);
    return _compareEPRs(epr, tmp);
}

static void _testEPRs(void)
{
    WsmEndpointReference epr;

    _createEPR(0, epr);
    WsmValue val(epr);

    // Test EPR value constructor
    if (val.getType() != WSMTYPE_REFERENCE ||
        val.isArray() || val.isNull())
        throw Exception("Invalid EPR value.");

    // Test EPR get()
    WsmEndpointReference epr1;
    val.get(epr1);
    if (!_verifyEPR(0, epr1))
        throw Exception("Invalid EPR from value get().");

    // Convert EPR value to EPR array value
    val.toArray();
    if (val.getType() != WSMTYPE_REFERENCE ||
        !val.isArray() || val.isNull() || val.getArraySize() != 1)
        throw Exception("Invalid EPR to array conversion.");

    // Make sure we still have the same EPR
    Array<WsmEndpointReference> epra;
    val.get(epra);
    if (!_verifyEPR(0, epra[0]))
        throw Exception("Invalid EPR array from value get().");

    // Add more EPRs to the array
    for (int i = 1; i < 4; i++)
    {
        WsmEndpointReference tmp_epr;
        _createEPR(i, tmp_epr);
        WsmValue tmp_val(tmp_epr);
        val.add(tmp_val);
    }

    // Make sure we can get the array + verify it's size
    val.get(epra);
    if (epra.size() != 4)
        throw Exception("Invalid EPR array size.");

    // Test that we have the right entries in the array
    for (int i = 0; i < 4; i++)
    {
        if (!_verifyEPR(i, epra[i]))
            throw Exception("Invalid EPR array entry.");
    }

    // Re-set the value from array to EPR
    _createEPR(10, epr);
    val.set(epr);
    if (val.getType() != WSMTYPE_REFERENCE ||
        val.isArray() || val.isNull())
        throw Exception("Invalid EPR value.");

    // Make sure set() worked correctly
    val.get(epr1);
    if (!_verifyEPR(10, epr1))
        throw Exception("Can't set EPR value.");

    // Add string values. The value must convert itself into EPR array.
    for (int i = 11; i < 15; i++)
    {
        WsmEndpointReference tmp_epr;
        _createEPR(i, tmp_epr);
        WsmValue tmp_val(tmp_epr);
        val.add(tmp_val);
    }

    // Make sure we can get the array + verify it's size
    val.get(epra);
    if (epra.size() != 5)
        throw Exception("Invalid EPR array size.");

    // Test that we have the right entries in the array
    for (int i = 10; i < 15; i++)
    {
        if (!_verifyEPR(i, epra[i-10]))
            throw Exception("Invalid EPR array entry.");
    }

    // Test EPR array constructor and array value set
    Array<WsmEndpointReference> tmp;
    for (int i = 20; i < 26; i++)
    {
        WsmEndpointReference tmp_epr;
        _createEPR(i, tmp_epr);
        tmp.append(tmp_epr);
    }
    val.set(tmp);
    val.get(epra);

    // Test that we have the right entries in the array
    for (int i = 20; i < 26; i++)
    {
        if (!_verifyEPR(i, epra[i-20]))
            throw Exception("Invalid EPR array entry.");
    }

    WsmValue val1(tmp);
    Array<WsmEndpointReference> epra1;
    val1.get(epra1);

    // Test that we have the right entries in the array
    for (int i = 20; i < 26; i++)
    {
        if (!_verifyEPR(i, epra1[i-20]))
            throw Exception("Invalid EPR array entry.");
    }
}

static void _createInstance(int idx, WsmInstance& inst)
{
    char buf[20];
    sprintf(buf, "Class name %d", idx);
    inst.setClassName(buf);
}

static Boolean _compareInstances(WsmInstance& inst1, WsmInstance& inst2)
{
    if (inst1.getClassName() != inst2.getClassName())
        return false;

    return true;
}

static Boolean _verifyInstance(int idx, WsmInstance& inst)
{
    WsmInstance tmp;
    _createInstance(idx, tmp);
    return _compareInstances(inst, tmp);
}

static void _testInstances(void)
{
    WsmInstance inst;

    _createInstance(0, inst);
    WsmValue val(inst);

    // Test Instance value constructor
    if (val.getType() != WSMTYPE_INSTANCE ||
        val.isArray() || val.isNull())
        throw Exception("Invalid Instance value.");

    // Test Instance get()
    WsmInstance inst1;
    val.get(inst1);
    if (!_verifyInstance(0, inst1))
        throw Exception("Invalid Instance from value get().");

    // Convert Instance value to Instance array value
    val.toArray();
    if (val.getType() != WSMTYPE_INSTANCE ||
        !val.isArray() || val.isNull() || val.getArraySize() != 1)
        throw Exception("Invalid Instance to array conversion.");

    // Make sure we still have the same Instance
    Array<WsmInstance> insta;
    val.get(insta);
    if (!_verifyInstance(0, insta[0]))
        throw Exception("Invalid Instance array from value get().");

    // Add more Instances to the array
    for (int i = 1; i < 4; i++)
    {
        WsmInstance tmp_inst;
        _createInstance(i, tmp_inst);
        WsmValue tmp_val(tmp_inst);
        val.add(tmp_val);
    }

    // Make sure we can get the array + verify it's size
    val.get(insta);
    if (insta.size() != 4)
        throw Exception("Invalid Instance array size.");

    // Test that we have the right entries in the array
    for (int i = 0; i < 4; i++)
    {
        if (!_verifyInstance(i, insta[i]))
            throw Exception("Invalid Instance array entry.");
    }

    // Re-set the value from array to Instance
    _createInstance(10, inst);
    val.set(inst);
    if (val.getType() != WSMTYPE_INSTANCE ||
        val.isArray() || val.isNull())
        throw Exception("Invalid Instance value.");

    // Make sure set() worked correctly
    val.get(inst1);
    if (!_verifyInstance(10, inst1))
        throw Exception("Can't set Instance value.");

    // Add Instance values. The value must convert itself into Instance array.
    for (int i = 11; i < 15; i++)
    {
        WsmInstance tmp_inst;
        _createInstance(i, tmp_inst);
        WsmValue tmp_val(tmp_inst);
        val.add(tmp_val);
    }

    // Make sure we can get the array + verify it's size
    val.get(insta);
    if (insta.size() != 5)
        throw Exception("Invalid Instance array size.");

    // Test that we have the right entries in the array
    for (int i = 10; i < 15; i++)
    {
        if (!_verifyInstance(i, insta[i-10]))
            throw Exception("Invalid Instance array entry.");
    }

    // Test Instance array constructor and array value set
    Array<WsmInstance> tmp;
    for (int i = 20; i < 26; i++)
    {
        WsmInstance tmp_inst;
        _createInstance(i, tmp_inst);
        tmp.append(tmp_inst);
    }
    val.set(tmp);
    val.get(insta);

    // Test that we have the right entries in the array
    for (int i = 20; i < 26; i++)
    {
        if (!_verifyInstance(i, insta[i-20]))
            throw Exception("Invalid Instance array entry.");
    }

    WsmValue val1(tmp);
    Array<WsmInstance> insta1;
    val1.get(insta1);

    // Test that we have the right entries in the array
    for (int i = 20; i < 26; i++)
    {
        if (!_verifyInstance(i, insta1[i-20]))
            throw Exception("Invalid Instance array entry.");
    }
}

static void _testValueErrors()
{
    // Get 'get' type mismatches of EPR values
    try
    {
        String str;
        WsmEndpointReference epr;
        WsmValue val(epr);
        val.get(str);
        throw Exception("Expected type mismatch");
    }
    catch (TypeMismatchException&)
    {
    }

    try
    {
        Array<WsmEndpointReference> epra;
        WsmEndpointReference epr;
        WsmValue val(epr);
        val.get(epra);
        throw Exception("Expected type mismatch");
    }
    catch (TypeMismatchException&)
    {
    }

    try
    {
        String str;
        Array<WsmEndpointReference> epr;
        WsmValue val(epr);
        val.get(str);
        throw Exception("Expected type mismatch");
    }
    catch (TypeMismatchException&)
    {
    }

    try
    {
        Array<WsmEndpointReference> epra;
        WsmEndpointReference epr;
        WsmValue val(epra);
        val.get(epr);
        throw Exception("Expected type mismatch");
    }
    catch (TypeMismatchException&)
    {
    }

    // Test 'get' type mismatches of instance values
    try
    {
        String str;
        WsmInstance inst;
        WsmValue val(inst);
        val.get(str);
        throw Exception("Expected type mismatch");
    }
    catch (TypeMismatchException&)
    {
    }

    try
    {
        Array<WsmInstance> insta;
        WsmInstance inst;
        WsmValue val(inst);
        val.get(insta);
        throw Exception("Expected type mismatch");
    }
    catch (TypeMismatchException&)
    {
    }

    try
    {
        String str;
        Array<WsmInstance> inst;
        WsmValue val(inst);
        val.get(str);
        throw Exception("Expected type mismatch");
    }
    catch (TypeMismatchException&)
    {
    }

    try
    {
        Array<WsmInstance> insta;
        WsmInstance inst;
        WsmValue val(insta);
        val.get(inst);
        throw Exception("Expected type mismatch");
    }
    catch (TypeMismatchException&)
    {
    }

    //  Test 'get' type mismatches of string values
    try
    {
        WsmInstance inst;
        String str;
        WsmValue val(str);
        val.get(inst);
        throw Exception("Expected type mismatch");
    }
    catch (TypeMismatchException&)
    {
    }

    try
    {
        Array<String> stra;
        String str;
        WsmValue val(str);
        val.get(stra);
        throw Exception("Expected type mismatch");
    }
    catch (TypeMismatchException&)
    {
    }

    try
    {
        WsmInstance inst;
        Array<String> str;
        WsmValue val(str);
        val.get(inst);
        throw Exception("Expected type mismatch");
    }
    catch (TypeMismatchException&)
    {
    }

    try
    {
        Array<String> stra;
        String str;
        WsmValue val(stra);
        val.get(str);
        throw Exception("Expected type mismatch");
    }
    catch (TypeMismatchException&)
    {
    }

    // Test adding a type mismatched value to an array value
    try
    {
        String str;
        WsmInstance inst;
        WsmValue ival(inst);
        WsmValue sval(str);
        ival.add(sval);
    }
    catch (TypeMismatchException&)
    {
    }
}

static void _testMisc(void)
{
    // Test that array conversion works on an array value
    {
        Array<String> stra;
        WsmValue val(stra);
        val.toArray();
        if (val.isNull() || !val.isArray() ||
            val.getType() != WSMTYPE_OTHER)
            throw Exception("Invalid array value");
    }

    // Test setNull()
    WsmInstance inst;
    {
        WsmValue val(inst);
        val.setNull();
        if (!val.isNull() || val.isArray() ||
            val.getType() != WSMTYPE_OTHER)
            throw Exception("Invalid NULL value");
    }
}

int main(int argc, char** argv)
{
    try
    {
        _testStrings();
        _testEPRs();
        _testInstances();
        _testMisc();
        _testValueErrors();
    }
    catch (Exception& e)
    {
        cerr << "Error: " << e.getMessage() << endl;
        exit(1);
    }
    cout << argv[0] << " +++++ passed all tests" << endl;
    return 0;
}
