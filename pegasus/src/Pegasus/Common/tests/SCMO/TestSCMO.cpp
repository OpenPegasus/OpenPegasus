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
// This code implements test cases for PEP#348 - The CMPI infrastructure using
// SCMO (Single Chunk Memory Objects). Specifically it tests integrity of the
// object model.
// The design document can be found on the OpenPegasus website openpegasus.org
// at https://collaboration.opengroup.org/pegasus/pp/documents/21210/PEP_348.pdf
//
//%/////////////////////////////////////////////////////////////////////////////

#include "TestSCMO.h"
#include <Pegasus/Common/XmlReader.h>
#include <Pegasus/Common/XmlParser.h>
#include <Pegasus/Common/Config.h>
#include <Pegasus/Common/FileSystem.h>
#include <Pegasus/Common/SCMOClassCache.h>
#include <Pegasus/Common/XmlWriter.h>
#include <Pegasus/Common/SCMOXmlWriter.h>
#include <Pegasus/Common/CIMClass.h>

// Diagnostic and display output tools
#include <Pegasus/Common/Print.h>
#include <Pegasus/Common/XmlWriter.h>

PEGASUS_USING_PEGASUS;
PEGASUS_USING_STD;

#define VCOUT if (verbose) cout
#ifdef PEGASUS_DEBUG
#  define VSCMODUMP(sd) if (verbose) (sd)
#else
#  define VSCMODUMP(sd)
#endif

static Boolean verbose;
static Boolean loadClassOnce;

const String MASTERQUALIFIER ("/src/Pegasus/Common/tests/SCMO/masterQualifier");
const String MASTERCLASS ("/src/Pegasus/Common/tests/SCMO/masterClass");
const String TESTSCMOXML("/src/Pegasus/Common/tests/SCMO/TestSCMO.xml");
const String TESTSCMO2XML("/src/Pegasus/Common/tests/SCMO/TestSCMO2.xml");
const String
   TESTCSCLASSXML("/src/Pegasus/Common/tests/SCMO/CIMComputerSystemClass.xml");
const String
   TESTCSINSTXML("/src/Pegasus/Common/tests/SCMO/CIMComputerSystemInst.xml");

// NOCHKSRC
const char * Class2XmlOut =
"<INSTANCE CLASSNAME=\"SCMO_TESTClass2\" >\n"
"<PROPERTY NAME=\"DateTimeProperty\"  TYPE=\"datetime\">\n"
"<VALUE>00000000000017.236362:000</VALUE>\n"
"</PROPERTY>\n"
"<PROPERTY.ARRAY NAME=\"DateTimePropertyArray\"  TYPE=\"datetime\" ARRAYSIZE=\"3\">\n"
"<VALUE.ARRAY>\n"
"<VALUE>00000000001628.243387:000</VALUE>\n"
"<VALUE>00000000001347.383727:000</VALUE>\n"
"<VALUE>00000000001532.933892:000</VALUE>\n"
"</VALUE.ARRAY>\n"
"</PROPERTY.ARRAY>\n"
"<PROPERTY NAME=\"BooleanProperty\"  TYPE=\"boolean\">\n"
"<VALUE>TRUE</VALUE>\n"
"</PROPERTY>\n"
"<PROPERTY.ARRAY NAME=\"BooleanPropertyArray\"  TYPE=\"boolean\" ARRAYSIZE=\"3\" PROPAGATED=\"true\">\n"
"<VALUE.ARRAY>\n"
"<VALUE>TRUE</VALUE>\n"
"<VALUE>FALSE</VALUE>\n"
"<VALUE>TRUE</VALUE>\n"
"</VALUE.ARRAY>\n"
"</PROPERTY.ARRAY>\n"
"<PROPERTY NAME=\"Char16Property\"  TYPE=\"char16\">\n"
"<VALUE>㽊</VALUE>\n"
"</PROPERTY>\n"
"<PROPERTY.ARRAY NAME=\"Char16PropertyArray\"  TYPE=\"char16\" ARRAYSIZE=\"3\" PROPAGATED=\"true\">\n"
"<VALUE.ARRAY>\n"
"<VALUE>Ѐ</VALUE>\n"
"<VALUE>ࠀ</VALUE>\n"
"<VALUE>က</VALUE>\n"
"</VALUE.ARRAY>\n"
"</PROPERTY.ARRAY>\n"
"<PROPERTY NAME=\"StringProperty\"  TYPE=\"string\">\n"
"<VALUE>This is a single String!</VALUE>\n"
"</PROPERTY>\n"
"<PROPERTY.ARRAY NAME=\"StringPropertyArray\"  TYPE=\"string\" ARRAYSIZE=\"3\" PROPAGATED=\"true\">\n"
"<VALUE.ARRAY>\n"
"<VALUE>The Array String Number one.</VALUE>\n"
"<VALUE>The Array String Number two.</VALUE>\n"
"<VALUE>The Array String Number three.</VALUE>\n"
"</VALUE.ARRAY>\n"
"</PROPERTY.ARRAY>\n"
"<PROPERTY NAME=\"Uint8Property\"  TYPE=\"uint8\">\n"
"<VALUE>119</VALUE>\n"
"</PROPERTY>\n"
"<PROPERTY.ARRAY NAME=\"Uint8PropertyArray\"  TYPE=\"uint8\" ARRAYSIZE=\"3\" PROPAGATED=\"true\">\n"
"<VALUE.ARRAY>\n"
"<VALUE>42</VALUE>\n"
"<VALUE>155</VALUE>\n"
"<VALUE>192</VALUE>\n"
"</VALUE.ARRAY>\n"
"</PROPERTY.ARRAY>\n"
"<PROPERTY NAME=\"Uint16Property\"  TYPE=\"uint16\">\n"
"<VALUE>63359</VALUE>\n"
"</PROPERTY>\n"
"<PROPERTY.ARRAY NAME=\"Uint16PropertyArray\"  TYPE=\"uint16\" ARRAYSIZE=\"3\" PROPAGATED=\"true\">\n"
"<VALUE.ARRAY>\n"
"<VALUE>218</VALUE>\n"
"<VALUE>2673</VALUE>\n"
"<VALUE>172</VALUE>\n"
"</VALUE.ARRAY>\n"
"</PROPERTY.ARRAY>\n"
"<PROPERTY NAME=\"Uint32Property\"  TYPE=\"uint32\">\n"
"<VALUE>4160223223</VALUE>\n"
"</PROPERTY>\n"
"<PROPERTY.ARRAY NAME=\"Uint32PropertyArray\"  TYPE=\"uint32\" ARRAYSIZE=\"3\" PROPAGATED=\"true\">\n"
"<VALUE.ARRAY>\n"
"<VALUE>42</VALUE>\n"
"<VALUE>289</VALUE>\n"
"<VALUE>192</VALUE>\n"
"</VALUE.ARRAY>\n"
"</PROPERTY.ARRAY>\n"
"<PROPERTY NAME=\"Uint64Property\"  TYPE=\"uint64\">\n"
"<VALUE>11574445315536769232</VALUE>\n"
"</PROPERTY>\n"
"<PROPERTY.ARRAY NAME=\"Uint64PropertyArray\"  TYPE=\"uint64\" ARRAYSIZE=\"4\" PROPAGATED=\"true\">\n"
"<VALUE.ARRAY>\n"
"<VALUE>394</VALUE>\n"
"<VALUE>483734</VALUE>\n"
"<VALUE>1311768467294899695</VALUE>\n"
"<VALUE>23903483</VALUE>\n"
"</VALUE.ARRAY>\n"
"</PROPERTY.ARRAY>\n"
"<PROPERTY NAME=\"Sint8Property\"  TYPE=\"sint8\">\n"
"<VALUE>-13</VALUE>\n"
"</PROPERTY>\n"
"<PROPERTY.ARRAY NAME=\"Sint8PropertyArray\"  TYPE=\"sint8\" ARRAYSIZE=\"3\" PROPAGATED=\"true\">\n"
"<VALUE.ARRAY>\n"
"<VALUE>-2</VALUE>\n"
"<VALUE>94</VALUE>\n"
"<VALUE>-123</VALUE>\n"
"</VALUE.ARRAY>\n"
"</PROPERTY.ARRAY>\n"
"<PROPERTY NAME=\"Sint16Property\"  TYPE=\"sint16\">\n"
"<VALUE>-3509</VALUE>\n"
"</PROPERTY>\n"
"<PROPERTY.ARRAY NAME=\"Sint16PropertyArray\"  TYPE=\"sint16\" ARRAYSIZE=\"4\" PROPAGATED=\"true\">\n"
"<VALUE.ARRAY>\n"
"<VALUE>-8</VALUE>\n"
"<VALUE>23872</VALUE>\n"
"<VALUE>334</VALUE>\n"
"<VALUE>-4081</VALUE>\n"
"</VALUE.ARRAY>\n"
"</PROPERTY.ARRAY>\n"
"<PROPERTY NAME=\"Sint32Property\"  TYPE=\"sint32\">\n"
"<VALUE>15759420</VALUE>\n"
"</PROPERTY>\n"
"<PROPERTY.ARRAY NAME=\"Sint32PropertyArray\"  TYPE=\"sint32\" ARRAYSIZE=\"3\" PROPAGATED=\"true\">\n"
"<VALUE.ARRAY>\n"
"<VALUE>42</VALUE>\n"
"<VALUE>-28937332</VALUE>\n"
"<VALUE>19248372</VALUE>\n"
"</VALUE.ARRAY>\n"
"</PROPERTY.ARRAY>\n"
"<PROPERTY NAME=\"Sint64Property\"  TYPE=\"sint64\">\n"
"<VALUE>-1</VALUE>\n"
"</PROPERTY>\n"
"<PROPERTY.ARRAY NAME=\"Sint64PropertyArray\"  TYPE=\"sint64\" ARRAYSIZE=\"4\" PROPAGATED=\"true\">\n"
"<VALUE.ARRAY>\n"
"<VALUE>394</VALUE>\n"
"<VALUE>-483734324</VALUE>\n"
"<VALUE>232349034</VALUE>\n"
"<VALUE>0</VALUE>\n"
"</VALUE.ARRAY>\n"
"</PROPERTY.ARRAY>\n"
"<PROPERTY NAME=\"Real32Property\"  TYPE=\"real32\">\n"
"<VALUE>2.4271000e-04</VALUE>\n"
"</PROPERTY>\n"
"<PROPERTY.ARRAY NAME=\"Real32PropertyArray\"  TYPE=\"real32\" ARRAYSIZE=\"3\" PROPAGATED=\"true\">\n"
"<VALUE.ARRAY>\n"
"<VALUE>3.9399999e+30</VALUE>\n"
"<VALUE>-4.8373435e-35</VALUE>\n"
"<VALUE>2.3234899e+34</VALUE>\n"
"</VALUE.ARRAY>\n"
"</PROPERTY.ARRAY>\n"
"<PROPERTY NAME=\"Real64Property\"  TYPE=\"real64\">\n"
"<VALUE>2.4270999999999999e-40</VALUE>\n"
"</PROPERTY>\n"
"<PROPERTY.ARRAY NAME=\"Real64PropertyArray\"  TYPE=\"real64\" ARRAYSIZE=\"3\" PROPAGATED=\"true\">\n"
"<VALUE.ARRAY>\n"
"<VALUE>3.9399999999999997e+38</VALUE>\n"
"<VALUE>-4.8373464399999998e-35</VALUE>\n"
"<VALUE>2.6434899999999999e+34</VALUE>\n"
"</VALUE.ARRAY>\n"
"</PROPERTY.ARRAY>\n"
"</INSTANCE>\n";

const char * InstNoClassXmlOut =
"<INSTANCE CLASSNAME=\"_ClassNameNotExist\" >\n"
"<PROPERTY.ARRAY NAME=\"StringPropertyArray\"  TYPE=\"string\" ARRAYSIZE=\"3\">\n"
"<VALUE.ARRAY>\n"
"<VALUE>The Array String Number one.</VALUE>\n"
"<VALUE>The Array String Number two.</VALUE>\n"
"<VALUE>The Array String Number three.</VALUE>\n"
"</VALUE.ARRAY>\n"
"</PROPERTY.ARRAY>\n"
"<PROPERTY NAME=\"StringProperty\"  TYPE=\"string\">\n"
"<VALUE>This is a single String!</VALUE>\n"
"</PROPERTY>\n"
"<PROPERTY.ARRAY NAME=\"DateTimePropertyArray\"  TYPE=\"datetime\" ARRAYSIZE=\"3\">\n"
"<VALUE.ARRAY>\n"
"<VALUE>00000000001628.243387:000</VALUE>\n"
"<VALUE>00000000001347.383727:000</VALUE>\n"
"<VALUE>00000000001532.933892:000</VALUE>\n"
"</VALUE.ARRAY>\n"
"</PROPERTY.ARRAY>\n"
"<PROPERTY NAME=\"DateTimeProperty\"  TYPE=\"datetime\">\n"
"<VALUE>00000000000017.236362:000</VALUE>\n"
"</PROPERTY>\n"
"<PROPERTY.ARRAY NAME=\"BooleanPropertyArray\"  TYPE=\"boolean\" ARRAYSIZE=\"3\">\n"
"<VALUE.ARRAY>\n"
"<VALUE>TRUE</VALUE>\n"
"<VALUE>FALSE</VALUE>\n"
"<VALUE>TRUE</VALUE>\n"
"</VALUE.ARRAY>\n"
"</PROPERTY.ARRAY>\n"
"<PROPERTY.ARRAY NAME=\"Real64PropertyArray\"  TYPE=\"real64\" ARRAYSIZE=\"3\">\n"
"<VALUE.ARRAY>\n"
"<VALUE>3.9399999999999997e+38</VALUE>\n"
"<VALUE>-4.8373464399999998e-35</VALUE>\n"
"<VALUE>2.6434899999999999e+34</VALUE>\n"
"</VALUE.ARRAY>\n"
"</PROPERTY.ARRAY>\n"
"<PROPERTY NAME=\"Real64Property\"  TYPE=\"real64\">\n"
"<VALUE>2.4270999999999999e-40</VALUE>\n"
"</PROPERTY>\n"
"<PROPERTY.ARRAY NAME=\"Real32PropertyArray\"  TYPE=\"real32\" ARRAYSIZE=\"3\">\n"
"<VALUE.ARRAY>\n"
"<VALUE>3.9399999e+30</VALUE>\n"
"<VALUE>-4.8373435e-35</VALUE>\n"
"<VALUE>2.3234899e+34</VALUE>\n"
"</VALUE.ARRAY>\n"
"</PROPERTY.ARRAY>\n"
"<PROPERTY NAME=\"Real32Property\"  TYPE=\"real32\">\n"
"<VALUE>2.4271000e-04</VALUE>\n"
"</PROPERTY>\n"
"<PROPERTY.ARRAY NAME=\"Sint64PropertyArray\"  TYPE=\"sint64\" ARRAYSIZE=\"4\">\n"
"<VALUE.ARRAY>\n"
"<VALUE>394</VALUE>\n"
"<VALUE>-483734324</VALUE>\n"
"<VALUE>232349034</VALUE>\n"
"<VALUE>0</VALUE>\n"
"</VALUE.ARRAY>\n"
"</PROPERTY.ARRAY>\n"
"<PROPERTY NAME=\"Sint64Property\"  TYPE=\"sint64\">\n"
"<VALUE>-1</VALUE>\n"
"</PROPERTY>\n"
"<PROPERTY.ARRAY NAME=\"Sint32PropertyArray\"  TYPE=\"sint32\" ARRAYSIZE=\"3\">\n"
"<VALUE.ARRAY>\n"
"<VALUE>42</VALUE>\n"
"<VALUE>-28937332</VALUE>\n"
"<VALUE>19248372</VALUE>\n"
"</VALUE.ARRAY>\n"
"</PROPERTY.ARRAY>\n"
"<PROPERTY NAME=\"Sint32Property\"  TYPE=\"sint32\">\n"
"<VALUE>15759420</VALUE>\n"
"</PROPERTY>\n"
"<PROPERTY.ARRAY NAME=\"Sint16PropertyArray\"  TYPE=\"sint16\" ARRAYSIZE=\"4\">\n"
"<VALUE.ARRAY>\n"
"<VALUE>-8</VALUE>\n"
"<VALUE>23872</VALUE>\n"
"<VALUE>334</VALUE>\n"
"<VALUE>-4081</VALUE>\n"
"</VALUE.ARRAY>\n"
"</PROPERTY.ARRAY>\n"
"<PROPERTY NAME=\"Sint16Property\"  TYPE=\"sint16\">\n"
"<VALUE>-3509</VALUE>\n"
"</PROPERTY>\n"
"<PROPERTY.ARRAY NAME=\"Sint8PropertyArray\"  TYPE=\"sint8\" ARRAYSIZE=\"3\">\n"
"<VALUE.ARRAY>\n"
"<VALUE>-2</VALUE>\n"
"<VALUE>94</VALUE>\n"
"<VALUE>-123</VALUE>\n"
"</VALUE.ARRAY>\n"
"</PROPERTY.ARRAY>\n"
"<PROPERTY NAME=\"Sint8Property\"  TYPE=\"sint8\">\n"
"<VALUE>-13</VALUE>\n"
"</PROPERTY>\n"
"<PROPERTY.ARRAY NAME=\"Uint64PropertyArray\"  TYPE=\"uint64\" ARRAYSIZE=\"4\">\n"
"<VALUE.ARRAY>\n"
"<VALUE>394</VALUE>\n"
"<VALUE>483734</VALUE>\n"
"<VALUE>1311768467294899695</VALUE>\n"
"<VALUE>23903483</VALUE>\n"
"</VALUE.ARRAY>\n"
"</PROPERTY.ARRAY>\n"
"<PROPERTY NAME=\"Uint64Property\"  TYPE=\"uint64\">\n"
"<VALUE>11574445315536769232</VALUE>\n"
"</PROPERTY>\n"
"<PROPERTY NAME=\"Uint32Property\"  TYPE=\"uint32\">\n"
"<VALUE>4160223223</VALUE>\n"
"</PROPERTY>\n"
"<PROPERTY.ARRAY NAME=\"Uint16PropertyArray\"  TYPE=\"uint16\" ARRAYSIZE=\"3\">\n"
"<VALUE.ARRAY>\n"
"<VALUE>218</VALUE>\n"
"<VALUE>2673</VALUE>\n"
"<VALUE>172</VALUE>\n"
"</VALUE.ARRAY>\n"
"</PROPERTY.ARRAY>\n"
"<PROPERTY NAME=\"Uint16Property\"  TYPE=\"uint16\">\n"
"<VALUE>63359</VALUE>\n"
"</PROPERTY>\n"
"<PROPERTY.ARRAY NAME=\"Uint8PropertyArray\"  TYPE=\"uint8\" ARRAYSIZE=\"3\">\n"
"<VALUE.ARRAY>\n"
"<VALUE>42</VALUE>\n"
"<VALUE>155</VALUE>\n"
"<VALUE>192</VALUE>\n"
"</VALUE.ARRAY>\n"
"</PROPERTY.ARRAY>\n"
"<PROPERTY NAME=\"Uint8Property\"  TYPE=\"uint8\">\n"
"<VALUE>119</VALUE>\n"
"</PROPERTY>\n"
"<PROPERTY.ARRAY NAME=\"Char16PropertyArray\"  TYPE=\"char16\" ARRAYSIZE=\"3\">\n"
"<VALUE.ARRAY>\n"
"<VALUE>?</VALUE>\n"
"<VALUE>?</VALUE>\n"
"<VALUE>?</VALUE>\n"
"</VALUE.ARRAY>\n"
"</PROPERTY.ARRAY>\n"
"<PROPERTY NAME=\"Char16Property\"  TYPE=\"char16\">\n"
"<VALUE>?</VALUE>\n"
"</PROPERTY>\n"
"<PROPERTY.ARRAY NAME=\"Uint32PropertyArray\"  TYPE=\"uint32\" ARRAYSIZE=\"3\">\n"
"<VALUE.ARRAY>\n"
"<VALUE>42</VALUE>\n"
"<VALUE>289</VALUE>\n"
"<VALUE>192</VALUE>\n"
"</VALUE.ARRAY>\n"
"</PROPERTY.ARRAY>\n"
"<PROPERTY NAME=\"BooleanProperty\"  TYPE=\"boolean\">\n"
"<VALUE>TRUE</VALUE>\n"
"</PROPERTY>\n"
"<PROPERTY NAME=\"BooleanPropertyValueFalse\"  TYPE=\"boolean\">\n"
"<VALUE>FALSE</VALUE>\n"
"</PROPERTY>\n"
"</INSTANCE>\n";

//DOCHKSRC


String _formatHexDump(
    const char * data,
    Uint32 size)
{
    String rtn;
    unsigned char* p = (unsigned char*)data;
    unsigned char buf[16];
    size_t n = 0;

    for (size_t i = 0, col = 0; i < size; i++)
    {
        unsigned char c = p[i];
        buf[n++] = c;

        if (col == 0)
        {
            rtn.appendPrintf("%06X ", (unsigned int)i);
        }

        rtn.appendPrintf("%02X", c);

        if ( ((col+1) & 3) == 0 )
        {
            rtn.append(" ");
        }
        if (col + 1 == sizeof(buf) || i + 1 == size)
        {
            for (size_t j = col + 1; j < sizeof(buf); j++)
            {
                rtn.append("   ");
            }
            for (size_t j = 0; j < n; j++)
            {
                c = buf[j];

                if (c >= ' ' && c <= '~')
                {
                    rtn.append(c);
                }
                else
                {
                    rtn.append('.');
                }

            }
            rtn.append('\n');
            n = 0;
        }
        if (col + 1 == sizeof(buf))
        {
            col = 0;
        }
        else
        {
            col++;
        }
    }
    rtn.append('\n');
    return rtn;
}

// This is the callback function from the classCache.  It always returns
// the single instance of the SCMO class CIM_TESTClass2
// Gets a SCMO class from an XML file defined by namespace and
// classname that defines the CIMClass and convert to a SCMOClass
SCMOClass _scmoClassCache_GetClass(
    const CIMNamespaceName& nameSpace,
    const CIMName& className)
{
    CIMClass CIM_TESTClass2;
    Buffer text;

    VCOUT << endl << "Loading CIM SCMO_TESTClass2" << endl;

    // check if the class was alrady loaded.
    // If this fails, the class was requested to be loaded more then once !
    PEGASUS_TEST_ASSERT(!loadClassOnce);

    String TestSCMO2XML (getenv("PEGASUS_ROOT"));
    TestSCMO2XML.append(TESTSCMO2XML);

    FileSystem::loadFileToMemory(text,(const char*)TestSCMO2XML.getCString());

    XmlParser theParser((char*)text.getData());
    XmlReader::getObject(theParser,CIM_TESTClass2);

    // The class was loaded.
    loadClassOnce = true;

    VCOUT << endl << "Done." << endl;

    return SCMOClass(
        CIM_TESTClass2,
        (const char*)nameSpace.getString().getCString());
}

void structureSizesTest()
{
    // All SCMO structures have been aligned on 64bit memory adresses in a way
    // that they will be compatible between 32bit and 64bit adressing.
    // At the same time this alignment now causes the size of SCMO structures
    // to be equal across all platforms.
    VCOUT << endl << "Testing sizes of structures..." << endl;
    VCOUT << "SCMBUnion           : " << sizeof(SCMBUnion) << endl;
    PEGASUS_TEST_ASSERT(sizeof(SCMBUnion) == 16);
    VCOUT << "SCMBDataPtr         : " << sizeof(SCMBDataPtr) << endl;
    PEGASUS_TEST_ASSERT(sizeof(SCMBDataPtr) == 16);
    VCOUT << "SCMBValue           : " << sizeof(SCMBValue) << endl;
    PEGASUS_TEST_ASSERT(sizeof(SCMBValue) == 32);
    VCOUT << "SCMBKeyBindingValue : " << sizeof(SCMBKeyBindingValue) << endl;
    PEGASUS_TEST_ASSERT(sizeof(SCMBKeyBindingValue) == 24);
    VCOUT << "SCMBUserKeyBindingElement : "
         << sizeof(SCMBUserKeyBindingElement) << endl;
    PEGASUS_TEST_ASSERT(sizeof(SCMBUserKeyBindingElement) == 64);
    VCOUT << "SCMBQualifier       : " << sizeof(SCMBQualifier) << endl;
    PEGASUS_TEST_ASSERT(sizeof(SCMBQualifier) == 64);
    VCOUT << "SCMBMgmt_Header     : " << sizeof(SCMBMgmt_Header) << endl;
    PEGASUS_TEST_ASSERT(sizeof(SCMBMgmt_Header) == 56);
    VCOUT << "SCMBClassProperty   : " << sizeof(SCMBClassProperty) << endl;
    PEGASUS_TEST_ASSERT(sizeof(SCMBClassProperty) == 112);
    VCOUT << "SCMBClassPropertySet_Header : "
         << sizeof(SCMBClassPropertySet_Header) << endl;
    PEGASUS_TEST_ASSERT(sizeof(SCMBClassPropertySet_Header) == 280);
    VCOUT << "SCMBClassPropertyNode : " << sizeof(SCMBClassPropertyNode) <<endl;
    PEGASUS_TEST_ASSERT(sizeof(SCMBClassPropertyNode) == 120);
    VCOUT << "SCMBKeyBindingNode  : " << sizeof(SCMBKeyBindingNode) << endl;
    PEGASUS_TEST_ASSERT(sizeof(SCMBKeyBindingNode) == 32);
    VCOUT << "SCMBKeyBindingSet_Header : "
         << sizeof(SCMBKeyBindingSet_Header) << endl;
    PEGASUS_TEST_ASSERT(sizeof(SCMBKeyBindingSet_Header) == 152);
    VCOUT << "SCMBClass_Main      : " << sizeof(SCMBClass_Main) << endl;
    PEGASUS_TEST_ASSERT(sizeof(SCMBClass_Main) == 600);
    VCOUT << "SCMBInstance_Main   : " << sizeof(SCMBInstance_Main) << endl;
    PEGASUS_TEST_ASSERT(sizeof(SCMBInstance_Main) == 200);

    VCOUT << "SCMBUserPropertyElement   : "
        << sizeof(SCMBUserPropertyElement) << endl;
    PEGASUS_TEST_ASSERT(sizeof(SCMBUserPropertyElement) == 80);
    VCOUT << "SCMBUserKeyBindingElement   : "
        << sizeof(SCMBUserKeyBindingElement) << endl;
    PEGASUS_TEST_ASSERT(sizeof(SCMBUserKeyBindingElement) == 64);

    VCOUT << endl << "Testing sizes of structures...Done." << endl;
}

/*
    Tests converting CIMClass to SCMOClass, creating a SCMOInstance
    given the SCMO Class and tests a number of SCMOInstance manipulation
    methods.
*/
void CIMClassToSCMOClass()
{
    CIMClass theCIMClass;
    Buffer text;
    VCOUT << endl << "CIMClass to SCMOClass..." << endl;


    VCOUT << "Loading CIM SCMO_TESTClass" << endl;

    String TestSCMOXML (getenv("PEGASUS_ROOT"));
    TestSCMOXML.append(TESTSCMOXML);

    FileSystem::loadFileToMemory(text,(const char*)TestSCMOXML.getCString());

    XmlParser theParser((char*)text.getData());
    XmlReader::getObject(theParser,theCIMClass);

    SCMOClass theSCMOClass(theCIMClass);

#ifdef PEGASUS_DEBUG
    const char TestCSMOClassLog[]="TestSCMOClass.log";
    SCMODump dump(&(TestCSMOClassLog[0]));

    // Do not dump the volatile data.
    dump.dumpSCMOClass(theSCMOClass,false);

    String masterFile (getenv("PEGASUS_ROOT"));
    masterFile.append(MASTERCLASS);

    PEGASUS_TEST_ASSERT(dump.compareFile(masterFile));

    dump.closeFile();
    dump.deleteFile();
#endif

    VCOUT << "Creating CIMClass out of SCMOClass." << endl;
    CIMClass newCimClass;

    theSCMOClass.getCIMClass(newCimClass);

    // Before the newCimClass can be compared with the orginal class,
    // the methods of the original class has to be removed,
    // because the SCMOClass currently does not support methods.
    while (0 != theCIMClass.getMethodCount())
    {
        theCIMClass.removeMethod(0);
    }

    PEGASUS_TEST_ASSERT(newCimClass.identical(theCIMClass));

    VCOUT << "Creating SCMO instance out of SCMOClass." << endl;

    SCMOInstance theSCMOInstance(theSCMOClass);

    char TestSCMOClass[]= "TestSCMO Class";
    SCMBUnion tmp;
    tmp.extString.pchar = &(TestSCMOClass[0]);
    tmp.extString.length = strlen(TestSCMOClass);

    theSCMOInstance.setPropertyWithOrigin(
        "CreationClassName",
        CIMTYPE_STRING,
        &tmp);

    VCOUT << "Test of building key bindings from properties." << endl;

    char ThisIsTheName[] = "This is the Name";
    SCMBUnion tmp2;
    tmp2.extString.pchar  = &(ThisIsTheName[0]);
    tmp2.extString.length = strlen(ThisIsTheName);

    const SCMBUnion * tmp3;
    CIMType keyType;
    CIMType cimType;
    Boolean isArray;
    Uint32 number;
    SCMO_RC rc;

    theSCMOInstance.buildKeyBindingsFromProperties();

    // Only one of two key properties are set in the instance.
    // After creating key bindings out of key properies,
    // the key binding 'Name' must not be set.

    rc = theSCMOInstance.getKeyBinding("Name",keyType,&tmp3);
    PEGASUS_TEST_ASSERT(rc == SCMO_NULL_VALUE);
    PEGASUS_TEST_ASSERT(keyType == CIMTYPE_STRING);

    theSCMOInstance.setPropertyWithOrigin(
        "Name",
        CIMTYPE_STRING,
        &tmp2);

    theSCMOInstance.buildKeyBindingsFromProperties();

    // The key binding should be generated out of the property.

    theSCMOInstance.getKeyBinding("Name",keyType,&tmp3);

    PEGASUS_TEST_ASSERT(keyType == CIMTYPE_STRING);
    PEGASUS_TEST_ASSERT(tmp3->extString.length==tmp2.extString.length);
    PEGASUS_TEST_ASSERT(strcmp(tmp3->extString.pchar,tmp2.extString.pchar)==0);

    // do not forget to clean up tmp3, because it is a string
    free((void*)tmp3);

    VCOUT << "Test of cloning SCMOInstances." << endl;

    SCMOInstance cloneInstance = theSCMOInstance.clone();

    SCMOInstance asObjectPath = theSCMOInstance.clone(true);

    // The key binding of the orignal instance has to be set on the only
    // objectpath cloning.
    asObjectPath.getKeyBinding("Name",keyType,&tmp3);

    PEGASUS_TEST_ASSERT(keyType == CIMTYPE_STRING);
    PEGASUS_TEST_ASSERT(tmp3->extString.length==tmp2.extString.length);
    PEGASUS_TEST_ASSERT(strcmp(tmp3->extString.pchar,tmp2.extString.pchar)==0);

    // do not forget to clean up tmp3, because it is a string
    free((void*)tmp3);

    // But the associated key property has to be empty.
    rc = asObjectPath.getProperty("Name",cimType,&tmp3,isArray,number);

    PEGASUS_TEST_ASSERT(tmp3 == NULL);
    PEGASUS_TEST_ASSERT(rc == SCMO_NULL_VALUE);

    // But the in the full clone the property has to be set...
    rc = cloneInstance.getProperty("Name",cimType,&tmp3,isArray,number);

    PEGASUS_TEST_ASSERT(keyType == CIMTYPE_STRING);
    PEGASUS_TEST_ASSERT(tmp3->extString.length==tmp2.extString.length);
    PEGASUS_TEST_ASSERT(strcmp(tmp3->extString.pchar,tmp2.extString.pchar)==0);

    // do not forget to clean up tmp3, because it is a string
    free((void*)tmp3);

    VCOUT << endl << "Test 1: CIMClassToSCMOClass Done." << endl;
}

void SCMOClassQualifierTest()
{
    VCOUT << endl << "Getting SCMOClass from cache ..." << endl;

    SCMOClassCache* _theCache = SCMOClassCache::getInstance();

    SCMOClass SCMO_TESTClass2 = _theCache->getSCMOClass(
            "cimv2",
            strlen("cimv2"),
            "SCMO_TESTClass2",
            strlen("SCMO_TESTClass2"));

    VCOUT << endl << "SCMOClass qualifer test ..." << endl;

#ifdef PEGASUS_DEBUG
    String masterFile (getenv("PEGASUS_ROOT"));
    masterFile.append(MASTERQUALIFIER);

    SCMODump dump("TestSCMOClassQualifier.log");

    dump.dumpSCMOClassQualifiers(SCMO_TESTClass2);

    PEGASUS_TEST_ASSERT(dump.compareFile(masterFile));

    dump.deleteFile();
#endif

    VCOUT << "Done." << endl;
}

void SCMOInstancePropertyTest()
{
    SCMO_RC rc;

    // definition of return values.
    const SCMBUnion* unionReturn;
    CIMType typeReturn;
    Boolean isArrayReturn;
    Uint32 sizeReturn;


    SCMBUnion boolValue;
    boolValue.simple.val.bin=true;
    boolValue.simple.hasValue=true;

    SCMOClassCache* _theCache = SCMOClassCache::getInstance();

    SCMOClass SCMO_TESTClass2 = _theCache->getSCMOClass(
            "cimv2",
            strlen("cimv2"),
            "SCMO_TESTClass2",
            strlen("SCMO_TESTClass2"));

    SCMOInstance SCMO_TESTClass2_Inst(SCMO_TESTClass2);

    /**
     * Negative test cases for setting a propertty
     */

    VCOUT << endl <<
        "SCMOInstance Negative test cases for setting a property ..."
        << endl << endl;

    VCOUT << "Invalid property name." << endl;

    rc = SCMO_TESTClass2_Inst.setPropertyWithOrigin(
        "NotAProperty",
        CIMTYPE_BOOLEAN,
        &boolValue);

    PEGASUS_TEST_ASSERT(rc==SCMO_NOT_FOUND);

    VCOUT << "Property type is different." << endl;

    rc = SCMO_TESTClass2_Inst.setPropertyWithOrigin(
        "DateTimeProperty",
        CIMTYPE_BOOLEAN,
        &boolValue);

    PEGASUS_TEST_ASSERT(rc==SCMO_WRONG_TYPE);

    VCOUT << "Property is not an array." << endl;

    rc = SCMO_TESTClass2_Inst.setPropertyWithOrigin(
        "BooleanProperty",
        CIMTYPE_BOOLEAN,
        &boolValue,
        true,10);

    PEGASUS_TEST_ASSERT(rc==SCMO_NOT_AN_ARRAY);

    VCOUT << "Value is not an array." << endl;

    rc = SCMO_TESTClass2_Inst.setPropertyWithOrigin(
        "BooleanPropertyArray",
        CIMTYPE_BOOLEAN,
        &boolValue);

    PEGASUS_TEST_ASSERT(rc==SCMO_IS_AN_ARRAY);

    VCOUT << "Empty Array." << endl;

    // this just to get an valid pointer but I put no elements in it
    SCMBUnion *uint32ArrayValue = (SCMBUnion*)malloc(1*sizeof(Uint32));

    rc = SCMO_TESTClass2_Inst.setPropertyWithOrigin(
        "Uint32PropertyArray",
        CIMTYPE_UINT32,
        uint32ArrayValue,
        true,0);

    PEGASUS_TEST_ASSERT(rc==SCMO_OK);

    rc = SCMO_TESTClass2_Inst.getProperty(
        "Uint32PropertyArray",
        typeReturn,
        &unionReturn,
        isArrayReturn,
        sizeReturn);

    PEGASUS_TEST_ASSERT(rc==SCMO_OK);
    PEGASUS_TEST_ASSERT(sizeReturn==0);
    PEGASUS_TEST_ASSERT(isArrayReturn);
    PEGASUS_TEST_ASSERT(unionReturn==NULL);

    free(uint32ArrayValue);

    VCOUT << endl << "Done." << endl << endl;

     /**
     * Negative test cases for getting a propertty
     */

    VCOUT << endl <<
        "SCMOInstance Negative test cases for setting a property ..."
        << endl << endl;

    VCOUT << "Get Invalid property name." << endl;

    rc = SCMO_TESTClass2_Inst.getProperty(
    "NotAProperty",
    typeReturn,
    &unionReturn,
    isArrayReturn,
    sizeReturn);

    PEGASUS_TEST_ASSERT(rc==SCMO_NOT_FOUND);

    VCOUT << endl << "Done." << endl << endl;

    /**
    * Test positive cases setting and getting of all of the property
    * types
    */
    VCOUT << "SCMOInstance setting and reading properties ..." << endl;

    /**
     * Test Char16
     */

    VCOUT << endl << "Test Char16" << endl;

    SCMBUnion char16value;
    char16value.simple.val.c16=0x3F4A;
    char16value.simple.hasValue=true;

    rc = SCMO_TESTClass2_Inst.setPropertyWithOrigin(
        "Char16Property",
        CIMTYPE_CHAR16,
        &char16value);

    PEGASUS_TEST_ASSERT(rc==SCMO_OK);

    rc = SCMO_TESTClass2_Inst.getProperty(
        "Char16Property",
        typeReturn,
        &unionReturn,
        isArrayReturn,
        sizeReturn);

    PEGASUS_TEST_ASSERT(rc==SCMO_OK);
    PEGASUS_TEST_ASSERT(sizeReturn==0);
    PEGASUS_TEST_ASSERT(!isArrayReturn);
    PEGASUS_TEST_ASSERT(
        char16value.simple.val.c16 == unionReturn->simple.val.c16);
    PEGASUS_TEST_ASSERT(unionReturn->simple.hasValue);

    SCMBUnion char16ArrayValue[3];
    char16ArrayValue[0].simple.val.c16 = 1024;
    char16ArrayValue[0].simple.hasValue=true;
    char16ArrayValue[1].simple.val.c16 = 2048;
    char16ArrayValue[1].simple.hasValue=true;
    char16ArrayValue[2].simple.val.c16 = 4096;
    char16ArrayValue[2].simple.hasValue=true;

    rc = SCMO_TESTClass2_Inst.setPropertyWithOrigin(
        "Char16PropertyArray",
        CIMTYPE_CHAR16,
        char16ArrayValue,
        true,3);

    PEGASUS_TEST_ASSERT(rc==SCMO_OK);

    rc = SCMO_TESTClass2_Inst.getProperty(
        "Char16PropertyArray",
        typeReturn,
        &unionReturn,
        isArrayReturn,
        sizeReturn);

    PEGASUS_TEST_ASSERT(sizeReturn==3);
    PEGASUS_TEST_ASSERT(isArrayReturn);
    PEGASUS_TEST_ASSERT(rc==SCMO_OK);

    PEGASUS_TEST_ASSERT(
        char16ArrayValue[0].simple.val.c16 == unionReturn[0].simple.val.c16);
    PEGASUS_TEST_ASSERT(unionReturn[0].simple.hasValue);
    PEGASUS_TEST_ASSERT(
        char16ArrayValue[1].simple.val.c16 == unionReturn[1].simple.val.c16);
    PEGASUS_TEST_ASSERT(unionReturn[1].simple.hasValue);
    PEGASUS_TEST_ASSERT(
        char16ArrayValue[2].simple.val.c16 == unionReturn[2].simple.val.c16);
    PEGASUS_TEST_ASSERT(unionReturn[2].simple.hasValue);

    /**
     * Test Uint8
     */
    VCOUT << "Test Uint8" << endl;

    SCMBUnion uint8value;
    uint8value.simple.val.u8=0x77;
    uint8value.simple.hasValue=true;

    rc = SCMO_TESTClass2_Inst.setPropertyWithOrigin(
        "Uint8Property",
        CIMTYPE_UINT8,
        &uint8value);

    PEGASUS_TEST_ASSERT(rc==SCMO_OK);

    rc = SCMO_TESTClass2_Inst.getProperty(
        "Uint8Property",
        typeReturn,
        &unionReturn,
        isArrayReturn,
        sizeReturn);

    PEGASUS_TEST_ASSERT(rc==SCMO_OK);
    PEGASUS_TEST_ASSERT(sizeReturn==0);
    PEGASUS_TEST_ASSERT(!isArrayReturn);

    PEGASUS_TEST_ASSERT(
        uint8value.simple.val.u8 == unionReturn->simple.val.u8);
    PEGASUS_TEST_ASSERT(unionReturn->simple.hasValue);

    SCMBUnion uint8ArrayValue[3];
    uint8ArrayValue[0].simple.val.u8 = 42;
    uint8ArrayValue[0].simple.hasValue=true;
    uint8ArrayValue[1].simple.val.u8 = 155;
    uint8ArrayValue[1].simple.hasValue=true;
    uint8ArrayValue[2].simple.val.u8 = 192;
    uint8ArrayValue[2].simple.hasValue=true;

    rc = SCMO_TESTClass2_Inst.setPropertyWithOrigin(
        "Uint8PropertyArray",
        CIMTYPE_UINT8,
        uint8ArrayValue,
        true,3);

    PEGASUS_TEST_ASSERT(rc==SCMO_OK);

    rc = SCMO_TESTClass2_Inst.getProperty(
        "Uint8PropertyArray",
        typeReturn,
        &unionReturn,
        isArrayReturn,
        sizeReturn);

    PEGASUS_TEST_ASSERT(sizeReturn==3);
    PEGASUS_TEST_ASSERT(isArrayReturn);
    PEGASUS_TEST_ASSERT(rc==SCMO_OK);

    PEGASUS_TEST_ASSERT(
        uint8ArrayValue[0].simple.val.u8 == unionReturn[0].simple.val.u8);
    PEGASUS_TEST_ASSERT(unionReturn[0].simple.hasValue);
    PEGASUS_TEST_ASSERT(
        uint8ArrayValue[1].simple.val.u8 == unionReturn[1].simple.val.u8);
    PEGASUS_TEST_ASSERT(unionReturn[1].simple.hasValue);
    PEGASUS_TEST_ASSERT(
        uint8ArrayValue[2].simple.val.u8 == unionReturn[2].simple.val.u8);
    PEGASUS_TEST_ASSERT(unionReturn[2].simple.hasValue);

    /**
     * Test Uint16
     */

    VCOUT << "Test Uint16" << endl;

    SCMBUnion uint16value;
    uint16value.simple.val.u16=0xF77F;
    uint16value.simple.hasValue=true;

    rc = SCMO_TESTClass2_Inst.setPropertyWithOrigin(
        "Uint16Property",
        CIMTYPE_UINT16,
        &uint16value);

    PEGASUS_TEST_ASSERT(rc==SCMO_OK);

    rc = SCMO_TESTClass2_Inst.getProperty(
        "Uint16Property",
        typeReturn,
        &unionReturn,
        isArrayReturn,
        sizeReturn);

    PEGASUS_TEST_ASSERT(rc==SCMO_OK);
    PEGASUS_TEST_ASSERT(sizeReturn==0);
    PEGASUS_TEST_ASSERT(!isArrayReturn);

    PEGASUS_TEST_ASSERT(
        uint16value.simple.val.u16 == unionReturn->simple.val.u16);
    PEGASUS_TEST_ASSERT(unionReturn->simple.hasValue);

    SCMBUnion uint16ArrayValue[3];
    uint16ArrayValue[0].simple.val.u16 = 218;
    uint16ArrayValue[0].simple.hasValue=true;
    uint16ArrayValue[1].simple.val.u16 = 2673;
    uint16ArrayValue[1].simple.hasValue=true;
    uint16ArrayValue[2].simple.val.u16 = 172;
    uint16ArrayValue[2].simple.hasValue=true;

    rc = SCMO_TESTClass2_Inst.setPropertyWithOrigin(
        "Uint16PropertyArray",
        CIMTYPE_UINT16,
        uint16ArrayValue,
        true,3);

    PEGASUS_TEST_ASSERT(rc==SCMO_OK);

    rc = SCMO_TESTClass2_Inst.getProperty(
        "Uint16PropertyArray",
        typeReturn,
        &unionReturn,
        isArrayReturn,
        sizeReturn);

    PEGASUS_TEST_ASSERT(rc==SCMO_OK);
    PEGASUS_TEST_ASSERT(sizeReturn==3);
    PEGASUS_TEST_ASSERT(isArrayReturn);

    PEGASUS_TEST_ASSERT(
        uint16ArrayValue[0].simple.val.u16 == unionReturn[0].simple.val.u16);
    PEGASUS_TEST_ASSERT(unionReturn[0].simple.hasValue);
    PEGASUS_TEST_ASSERT(
        uint16ArrayValue[1].simple.val.u16 == unionReturn[1].simple.val.u16);
    PEGASUS_TEST_ASSERT(unionReturn[1].simple.hasValue);
    PEGASUS_TEST_ASSERT(
        uint16ArrayValue[2].simple.val.u16 == unionReturn[2].simple.val.u16);
    PEGASUS_TEST_ASSERT(unionReturn[2].simple.hasValue);



    /**
     * Test Uint32
     */

    VCOUT << "Test Uint32" << endl;

    SCMBUnion uint32value;
    uint32value.simple.val.u32=0xF7F7F7F7;
    uint32value.simple.hasValue=true;

    rc = SCMO_TESTClass2_Inst.setPropertyWithOrigin(
        "Uint32Property",
        CIMTYPE_UINT32,
        &uint32value);

    PEGASUS_TEST_ASSERT(rc==SCMO_OK);

    rc = SCMO_TESTClass2_Inst.getProperty(
        "Uint32Property",
        typeReturn,
        &unionReturn,
        isArrayReturn,
        sizeReturn);

    PEGASUS_TEST_ASSERT(rc==SCMO_OK);
    PEGASUS_TEST_ASSERT(sizeReturn==0);
    PEGASUS_TEST_ASSERT(!isArrayReturn);

    PEGASUS_TEST_ASSERT(
        uint32value.simple.val.u32 == unionReturn->simple.val.u32);
    PEGASUS_TEST_ASSERT(unionReturn->simple.hasValue);

    SCMBUnion uint32ArrayValue2[3];
    uint32ArrayValue2[0].simple.val.u32 = 42;
    uint32ArrayValue2[0].simple.hasValue=true;
    uint32ArrayValue2[1].simple.val.u32 = 289;
    uint32ArrayValue2[1].simple.hasValue=true;
    uint32ArrayValue2[2].simple.val.u32 = 192;
    uint32ArrayValue2[2].simple.hasValue=true;

    rc = SCMO_TESTClass2_Inst.setPropertyWithOrigin(
        "Uint32PropertyArray",
        CIMTYPE_UINT32,
        uint32ArrayValue2,
        true,3);

    PEGASUS_TEST_ASSERT(rc==SCMO_OK);

    rc = SCMO_TESTClass2_Inst.getProperty(
        "Uint32PropertyArray",
        typeReturn,
        &unionReturn,
        isArrayReturn,
        sizeReturn);

    PEGASUS_TEST_ASSERT(sizeReturn==3);
    PEGASUS_TEST_ASSERT(isArrayReturn);
    PEGASUS_TEST_ASSERT(rc==SCMO_OK);

    PEGASUS_TEST_ASSERT(
        uint32ArrayValue2[0].simple.val.u32 == unionReturn[0].simple.val.u32);
    PEGASUS_TEST_ASSERT(unionReturn[0].simple.hasValue);
    PEGASUS_TEST_ASSERT(
        uint32ArrayValue2[1].simple.val.u32 == unionReturn[1].simple.val.u32);
    PEGASUS_TEST_ASSERT(unionReturn[1].simple.hasValue);
    PEGASUS_TEST_ASSERT(
        uint32ArrayValue2[2].simple.val.u32 == unionReturn[2].simple.val.u32);
    PEGASUS_TEST_ASSERT(unionReturn[2].simple.hasValue);


    /**
     * Test Uint64
     */

    VCOUT << "Test Uint64" << endl;

    SCMBUnion uint64value;
    uint64value.simple.val.u64=PEGASUS_UINT64_LITERAL(0xA0A0B0B0C0C0D0D0);
    uint64value.simple.hasValue=true;

    rc = SCMO_TESTClass2_Inst.setPropertyWithOrigin(
        "Uint64Property",
        CIMTYPE_UINT64,
        &uint64value);

    PEGASUS_TEST_ASSERT(rc==SCMO_OK);

    rc = SCMO_TESTClass2_Inst.getProperty(
        "Uint64Property",
        typeReturn,
        &unionReturn,
        isArrayReturn,
        sizeReturn);

    PEGASUS_TEST_ASSERT(rc==SCMO_OK);
    PEGASUS_TEST_ASSERT(sizeReturn==0);
    PEGASUS_TEST_ASSERT(!isArrayReturn);

    PEGASUS_TEST_ASSERT(
        uint64value.simple.val.u64 == unionReturn->simple.val.u64);
    PEGASUS_TEST_ASSERT(unionReturn->simple.hasValue);

    SCMBUnion uint64ArrayValue[4];
    uint64ArrayValue[0].simple.val.u64 = 394;
    uint64ArrayValue[0].simple.hasValue=true;
    uint64ArrayValue[1].simple.val.u64 = 483734;
    uint64ArrayValue[1].simple.hasValue=true;
    uint64ArrayValue[2].simple.val.u64 =
        PEGASUS_UINT64_LITERAL(0x1234567890ABCDEF);
    uint64ArrayValue[2].simple.hasValue=true;
    uint64ArrayValue[3].simple.val.u64 = 23903483;
    uint64ArrayValue[3].simple.hasValue=true;

    rc = SCMO_TESTClass2_Inst.setPropertyWithOrigin(
        "Uint64PropertyArray",
        CIMTYPE_UINT64,
        uint64ArrayValue,
        true,4);

    PEGASUS_TEST_ASSERT(rc==SCMO_OK);

    rc = SCMO_TESTClass2_Inst.getProperty(
        "Uint64PropertyArray",
        typeReturn,
        &unionReturn,
        isArrayReturn,
        sizeReturn);

    PEGASUS_TEST_ASSERT(sizeReturn==4);
    PEGASUS_TEST_ASSERT(isArrayReturn);
    PEGASUS_TEST_ASSERT(rc==SCMO_OK);

    PEGASUS_TEST_ASSERT(
        uint64ArrayValue[0].simple.val.u64 == unionReturn[0].simple.val.u64);
    PEGASUS_TEST_ASSERT(unionReturn[0].simple.hasValue);
    PEGASUS_TEST_ASSERT(
        uint64ArrayValue[1].simple.val.u64 == unionReturn[1].simple.val.u64);
    PEGASUS_TEST_ASSERT(unionReturn[1].simple.hasValue);
    PEGASUS_TEST_ASSERT(
        uint64ArrayValue[2].simple.val.u64 == unionReturn[2].simple.val.u64);
    PEGASUS_TEST_ASSERT(unionReturn[2].simple.hasValue);
    PEGASUS_TEST_ASSERT(
        uint64ArrayValue[3].simple.val.u64 == unionReturn[3].simple.val.u64);
    PEGASUS_TEST_ASSERT(unionReturn[3].simple.hasValue);

    /**
     * Test Sint8
     */
    VCOUT << "Test Sint8" << endl;

    SCMBUnion sint8value;
    sint8value.simple.val.s8=Sint8(0xF3);
    sint8value.simple.hasValue=true;

    rc = SCMO_TESTClass2_Inst.setPropertyWithOrigin(
        "Sint8Property",
        CIMTYPE_SINT8,
        &sint8value);

    PEGASUS_TEST_ASSERT(rc==SCMO_OK);

    rc = SCMO_TESTClass2_Inst.getProperty(
        "Sint8Property",
        typeReturn,
        &unionReturn,
        isArrayReturn,
        sizeReturn);

    PEGASUS_TEST_ASSERT(rc==SCMO_OK);
    PEGASUS_TEST_ASSERT(sizeReturn==0);
    PEGASUS_TEST_ASSERT(!isArrayReturn);

    PEGASUS_TEST_ASSERT(
        sint8value.simple.val.s8 == unionReturn->simple.val.s8);
    PEGASUS_TEST_ASSERT(unionReturn->simple.hasValue);

    SCMBUnion sint8ArrayValue[3];
    sint8ArrayValue[0].simple.val.s8 = -2;
    sint8ArrayValue[0].simple.hasValue=true;
    sint8ArrayValue[1].simple.val.s8 = 94;
    sint8ArrayValue[1].simple.hasValue=true;
    sint8ArrayValue[2].simple.val.s8 = -123;
    sint8ArrayValue[2].simple.hasValue=true;

    rc = SCMO_TESTClass2_Inst.setPropertyWithOrigin(
        "Sint8PropertyArray",
        CIMTYPE_SINT8,
        sint8ArrayValue,
        true,3);

    PEGASUS_TEST_ASSERT(rc==SCMO_OK);

    rc = SCMO_TESTClass2_Inst.getProperty(
        "Sint8PropertyArray",
        typeReturn,
        &unionReturn,
        isArrayReturn,
        sizeReturn);

    PEGASUS_TEST_ASSERT(sizeReturn==3);
    PEGASUS_TEST_ASSERT(isArrayReturn);
    PEGASUS_TEST_ASSERT(rc==SCMO_OK);

    PEGASUS_TEST_ASSERT(
        sint8ArrayValue[0].simple.val.s8 == unionReturn[0].simple.val.s8);
    PEGASUS_TEST_ASSERT(unionReturn[0].simple.hasValue);
    PEGASUS_TEST_ASSERT(
        sint8ArrayValue[1].simple.val.s8 == unionReturn[1].simple.val.s8);
    PEGASUS_TEST_ASSERT(unionReturn[1].simple.hasValue);
    PEGASUS_TEST_ASSERT(
        sint8ArrayValue[2].simple.val.s8 == unionReturn[2].simple.val.s8);
    PEGASUS_TEST_ASSERT(unionReturn[2].simple.hasValue);

    /**
     * Test Sint16
     */

    VCOUT << "Test Sint16" << endl;

    SCMBUnion sint16value;
    sint16value.simple.val.s16=Sint16(0xF24B);
    sint16value.simple.hasValue=true;

    rc = SCMO_TESTClass2_Inst.setPropertyWithOrigin(
        "Sint16Property",
        CIMTYPE_SINT16,
        &sint16value);

    PEGASUS_TEST_ASSERT(rc==SCMO_OK);

    rc = SCMO_TESTClass2_Inst.getProperty(
        "Sint16Property",
        typeReturn,
        &unionReturn,
        isArrayReturn,
        sizeReturn);

    PEGASUS_TEST_ASSERT(rc==SCMO_OK);
    PEGASUS_TEST_ASSERT(sizeReturn==0);
    PEGASUS_TEST_ASSERT(!isArrayReturn);

    PEGASUS_TEST_ASSERT(
        sint16value.simple.val.s16 == unionReturn->simple.val.s16);
    PEGASUS_TEST_ASSERT(unionReturn->simple.hasValue);

    SCMBUnion sint16ArrayValue[4];
    sint16ArrayValue[0].simple.val.s16 = Sint16(-8);
    sint16ArrayValue[0].simple.hasValue=true;
    sint16ArrayValue[1].simple.val.s16 = Sint16(23872);
    sint16ArrayValue[1].simple.hasValue=true;
    sint16ArrayValue[2].simple.val.s16 = Sint16(334);
    sint16ArrayValue[2].simple.hasValue=true;
    sint16ArrayValue[3].simple.val.s16 = Sint16(0xF00F);
    sint16ArrayValue[3].simple.hasValue=true;

    rc = SCMO_TESTClass2_Inst.setPropertyWithOrigin(
        "Sint16PropertyArray",
        CIMTYPE_SINT16,
        sint16ArrayValue,
        true,4);

    PEGASUS_TEST_ASSERT(rc==SCMO_OK);

    rc = SCMO_TESTClass2_Inst.getProperty(
        "Sint16PropertyArray",
        typeReturn,
        &unionReturn,
        isArrayReturn,
        sizeReturn);

    PEGASUS_TEST_ASSERT(rc==SCMO_OK);
    PEGASUS_TEST_ASSERT(sizeReturn==4);
    PEGASUS_TEST_ASSERT(isArrayReturn);

    PEGASUS_TEST_ASSERT(
        sint16ArrayValue[0].simple.val.s16 == unionReturn[0].simple.val.s16);
    PEGASUS_TEST_ASSERT(unionReturn[0].simple.hasValue);
    PEGASUS_TEST_ASSERT(
        sint16ArrayValue[1].simple.val.s16 == unionReturn[1].simple.val.s16);
    PEGASUS_TEST_ASSERT(unionReturn[1].simple.hasValue);
    PEGASUS_TEST_ASSERT(
        sint16ArrayValue[2].simple.val.s16 == unionReturn[2].simple.val.s16);
    PEGASUS_TEST_ASSERT(unionReturn[2].simple.hasValue);
    PEGASUS_TEST_ASSERT(
        sint16ArrayValue[3].simple.val.s16 == unionReturn[3].simple.val.s16);
    PEGASUS_TEST_ASSERT(unionReturn[3].simple.hasValue);

    /**
     * Test Sint32
     */

    VCOUT << "Test Sint32" << endl;

    SCMBUnion sint32value;
    sint32value.simple.val.s32=0xF0783C;
    sint32value.simple.hasValue=true;

    rc = SCMO_TESTClass2_Inst.setPropertyWithOrigin(
        "Sint32Property",
        CIMTYPE_SINT32,
        &sint32value);

    PEGASUS_TEST_ASSERT(rc==SCMO_OK);

    rc = SCMO_TESTClass2_Inst.getProperty(
        "Sint32Property",
        typeReturn,
        &unionReturn,
        isArrayReturn,
        sizeReturn);

    PEGASUS_TEST_ASSERT(rc==SCMO_OK);
    PEGASUS_TEST_ASSERT(sizeReturn==0);
    PEGASUS_TEST_ASSERT(!isArrayReturn);

    PEGASUS_TEST_ASSERT(
        sint32value.simple.val.s32 == unionReturn->simple.val.s32);
    PEGASUS_TEST_ASSERT(unionReturn->simple.hasValue);

    SCMBUnion sint32ArrayValue2[3];
    sint32ArrayValue2[0].simple.val.s32 = 42;
    sint32ArrayValue2[0].simple.hasValue=true;
    sint32ArrayValue2[1].simple.val.s32 = -28937332;
    sint32ArrayValue2[1].simple.hasValue=true;
    sint32ArrayValue2[2].simple.val.s32 = 19248372;
    sint32ArrayValue2[2].simple.hasValue=true;

    rc = SCMO_TESTClass2_Inst.setPropertyWithOrigin(
        "Sint32PropertyArray",
        CIMTYPE_SINT32,
        sint32ArrayValue2,
        true,3);

    PEGASUS_TEST_ASSERT(rc==SCMO_OK);

    rc = SCMO_TESTClass2_Inst.getProperty(
        "Sint32PropertyArray",
        typeReturn,
        &unionReturn,
        isArrayReturn,
        sizeReturn);

    PEGASUS_TEST_ASSERT(sizeReturn==3);
    PEGASUS_TEST_ASSERT(isArrayReturn);
    PEGASUS_TEST_ASSERT(rc==SCMO_OK);

    PEGASUS_TEST_ASSERT(
        sint32ArrayValue2[0].simple.val.s32 == unionReturn[0].simple.val.s32);
    PEGASUS_TEST_ASSERT(unionReturn[0].simple.hasValue);
    PEGASUS_TEST_ASSERT(
        sint32ArrayValue2[1].simple.val.s32 == unionReturn[1].simple.val.s32);
    PEGASUS_TEST_ASSERT(unionReturn[1].simple.hasValue);
    PEGASUS_TEST_ASSERT(
        sint32ArrayValue2[2].simple.val.s32 == unionReturn[2].simple.val.s32);
    PEGASUS_TEST_ASSERT(unionReturn[2].simple.hasValue);

    /**
     * Test Uint64
     */

    VCOUT << "Test Uint64" << endl;

    SCMBUnion sint64value;
    sint64value.simple.val.s64=(Sint64)-1;
    sint64value.simple.hasValue=true;

    rc = SCMO_TESTClass2_Inst.setPropertyWithOrigin(
        "Sint64Property",
        CIMTYPE_SINT64,
        &sint64value);

    PEGASUS_TEST_ASSERT(rc==SCMO_OK);

    rc = SCMO_TESTClass2_Inst.getProperty(
        "Sint64Property",
        typeReturn,
        &unionReturn,
        isArrayReturn,
        sizeReturn);

    PEGASUS_TEST_ASSERT(rc==SCMO_OK);
    PEGASUS_TEST_ASSERT(sizeReturn==0);
    PEGASUS_TEST_ASSERT(!isArrayReturn);

    PEGASUS_TEST_ASSERT(
        sint64value.simple.val.s64 == unionReturn->simple.val.s64);
    PEGASUS_TEST_ASSERT(unionReturn->simple.hasValue);

    SCMBUnion sint64ArrayValue[4];
    sint64ArrayValue[0].simple.val.s64 = 394;
    sint64ArrayValue[0].simple.hasValue=true;
    sint64ArrayValue[1].simple.val.s64 = -483734324;
    sint64ArrayValue[1].simple.hasValue=true;
    sint64ArrayValue[2].simple.val.s64 = 232349034;
    sint64ArrayValue[2].simple.hasValue=true;
    sint64ArrayValue[3].simple.val.s64 = 0;
    sint64ArrayValue[3].simple.hasValue=true;

    rc = SCMO_TESTClass2_Inst.setPropertyWithOrigin(
        "Sint64PropertyArray",
        CIMTYPE_SINT64,
        sint64ArrayValue,
        true,4);

    PEGASUS_TEST_ASSERT(rc==SCMO_OK);

    rc = SCMO_TESTClass2_Inst.getProperty(
        "Sint64PropertyArray",
        typeReturn,
        &unionReturn,
        isArrayReturn,
        sizeReturn);

    PEGASUS_TEST_ASSERT(sizeReturn==4);
    PEGASUS_TEST_ASSERT(isArrayReturn);
    PEGASUS_TEST_ASSERT(rc==SCMO_OK);

    PEGASUS_TEST_ASSERT(
        sint64ArrayValue[0].simple.val.s64 == unionReturn[0].simple.val.s64);
    PEGASUS_TEST_ASSERT(unionReturn[0].simple.hasValue);
    PEGASUS_TEST_ASSERT(
        sint64ArrayValue[1].simple.val.s64 == unionReturn[1].simple.val.s64);
    PEGASUS_TEST_ASSERT(unionReturn[1].simple.hasValue);
    PEGASUS_TEST_ASSERT(
        sint64ArrayValue[2].simple.val.s64 == unionReturn[2].simple.val.s64);
    PEGASUS_TEST_ASSERT(unionReturn[2].simple.hasValue);
    PEGASUS_TEST_ASSERT(
        sint64ArrayValue[3].simple.val.s64 == unionReturn[3].simple.val.s64);
    PEGASUS_TEST_ASSERT(unionReturn[3].simple.hasValue);

    /**
     * Test Real32
     */

    VCOUT << "Test Real32" << endl;

    SCMBUnion real32value;
    real32value.simple.val.r32=Real32(2.4271e-4);
    real32value.simple.hasValue=true;

    rc = SCMO_TESTClass2_Inst.setPropertyWithOrigin(
        "Real32Property",
        CIMTYPE_REAL32,
        &real32value);

    PEGASUS_TEST_ASSERT(rc==SCMO_OK);

    rc = SCMO_TESTClass2_Inst.getProperty(
        "Real32Property",
        typeReturn,
        &unionReturn,
        isArrayReturn,
        sizeReturn);

    PEGASUS_TEST_ASSERT(rc==SCMO_OK);
    PEGASUS_TEST_ASSERT(sizeReturn==0);
    PEGASUS_TEST_ASSERT(!isArrayReturn);

    PEGASUS_TEST_ASSERT(
        real32value.simple.val.r32 == unionReturn->simple.val.r32);
    PEGASUS_TEST_ASSERT(unionReturn->simple.hasValue);

    SCMBUnion real32ArrayValue[3];
    real32ArrayValue[0].simple.val.r32 = Real32(3.94e30);
    real32ArrayValue[0].simple.hasValue=true;
    real32ArrayValue[1].simple.val.r32 = Real32(-4.83734324e-35);
    real32ArrayValue[1].simple.hasValue=true;
    real32ArrayValue[2].simple.val.r32 = Real32(2.323490e34);
    real32ArrayValue[2].simple.hasValue=true;

    rc = SCMO_TESTClass2_Inst.setPropertyWithOrigin(
        "Real32PropertyArray",
        CIMTYPE_REAL32,
        real32ArrayValue,
        true,3);

    PEGASUS_TEST_ASSERT(rc==SCMO_OK);

    rc = SCMO_TESTClass2_Inst.getProperty(
        "Real32PropertyArray",
        typeReturn,
        &unionReturn,
        isArrayReturn,
        sizeReturn);

    PEGASUS_TEST_ASSERT(rc==SCMO_OK);
    PEGASUS_TEST_ASSERT(sizeReturn==3);
    PEGASUS_TEST_ASSERT(isArrayReturn);

    PEGASUS_TEST_ASSERT(
        real32ArrayValue[0].simple.val.r32 == unionReturn[0].simple.val.r32);
    PEGASUS_TEST_ASSERT(unionReturn[0].simple.hasValue);
    PEGASUS_TEST_ASSERT(
        real32ArrayValue[1].simple.val.r32 == unionReturn[1].simple.val.r32);
    PEGASUS_TEST_ASSERT(unionReturn[1].simple.hasValue);
    PEGASUS_TEST_ASSERT(
        real32ArrayValue[2].simple.val.r32 == unionReturn[2].simple.val.r32);
    PEGASUS_TEST_ASSERT(unionReturn[2].simple.hasValue);

    /**
     * Test Real64
     */

    VCOUT << "Test Real64" << endl;

    SCMBUnion real64value;
    real64value.simple.val.r64=Real64(2.4271e-40);
    real64value.simple.hasValue=true;

    rc = SCMO_TESTClass2_Inst.setPropertyWithOrigin(
        "Real64Property",
        CIMTYPE_REAL64,
        &real64value);

    PEGASUS_TEST_ASSERT(rc==SCMO_OK);

    rc = SCMO_TESTClass2_Inst.getProperty(
        "Real64Property",
        typeReturn,
        &unionReturn,
        isArrayReturn,
        sizeReturn);

    PEGASUS_TEST_ASSERT(rc==SCMO_OK);
    PEGASUS_TEST_ASSERT(sizeReturn==0);
    PEGASUS_TEST_ASSERT(!isArrayReturn);

    PEGASUS_TEST_ASSERT(
        real64value.simple.val.r64 == unionReturn->simple.val.r64);
    PEGASUS_TEST_ASSERT(unionReturn->simple.hasValue);

    SCMBUnion real64ArrayValue[3];
    real64ArrayValue[0].simple.val.r64 = Real64(3.94e38);
    real64ArrayValue[0].simple.hasValue=true;
    real64ArrayValue[1].simple.val.r64 = Real64(-4.83734644e-35);
    real64ArrayValue[1].simple.hasValue=true;
    real64ArrayValue[2].simple.val.r64 = Real64(2.643490e34);
    real64ArrayValue[2].simple.hasValue=true;

    rc = SCMO_TESTClass2_Inst.setPropertyWithOrigin(
        "Real64PropertyArray",
        CIMTYPE_REAL64,
        real64ArrayValue,
        true,3);

    PEGASUS_TEST_ASSERT(rc==SCMO_OK);

    rc = SCMO_TESTClass2_Inst.getProperty(
        "Real64PropertyArray",
        typeReturn,
        &unionReturn,
        isArrayReturn,
        sizeReturn);

    PEGASUS_TEST_ASSERT(rc==SCMO_OK);
    PEGASUS_TEST_ASSERT(sizeReturn==3);
    PEGASUS_TEST_ASSERT(isArrayReturn);

    PEGASUS_TEST_ASSERT(
        real64ArrayValue[0].simple.val.r64 == unionReturn[0].simple.val.r64);
    PEGASUS_TEST_ASSERT(unionReturn[0].simple.hasValue);
    PEGASUS_TEST_ASSERT(
        real64ArrayValue[1].simple.val.r64 == unionReturn[1].simple.val.r64);
    PEGASUS_TEST_ASSERT(unionReturn[1].simple.hasValue);
    PEGASUS_TEST_ASSERT(
        real64ArrayValue[2].simple.val.r64 == unionReturn[2].simple.val.r64);
    PEGASUS_TEST_ASSERT(unionReturn[2].simple.hasValue);

    /**
     * Test Boolean
     */

    VCOUT << "Test Boolean" << endl;

    boolValue.simple.val.bin=true;

    rc = SCMO_TESTClass2_Inst.setPropertyWithOrigin(
        "BooleanProperty",
        CIMTYPE_BOOLEAN,
        &boolValue);

    PEGASUS_TEST_ASSERT(rc==SCMO_OK);

    rc = SCMO_TESTClass2_Inst.getProperty(
        "BooleanProperty",
        typeReturn,
        &unionReturn,
        isArrayReturn,
        sizeReturn);

    PEGASUS_TEST_ASSERT(rc==SCMO_OK);
    PEGASUS_TEST_ASSERT(sizeReturn==0);
    PEGASUS_TEST_ASSERT(!isArrayReturn);

    PEGASUS_TEST_ASSERT(unionReturn->simple.val.bin);

    SCMBUnion boolArrayValue[3];
    boolArrayValue[0].simple.val.bin = true;
    boolArrayValue[0].simple.hasValue= true;
    boolArrayValue[1].simple.val.bin = false;
    boolArrayValue[1].simple.hasValue= true;
    boolArrayValue[2].simple.val.bin = true;
    boolArrayValue[2].simple.hasValue= true;


    rc = SCMO_TESTClass2_Inst.setPropertyWithOrigin(
        "BooleanPropertyArray",
        CIMTYPE_BOOLEAN,
        boolArrayValue,
        true,3);

    PEGASUS_TEST_ASSERT(rc==SCMO_OK);

    rc = SCMO_TESTClass2_Inst.getProperty(
        "BooleanPropertyArray",
        typeReturn,
        &unionReturn,
        isArrayReturn,
        sizeReturn);

    PEGASUS_TEST_ASSERT(rc==SCMO_OK);
    PEGASUS_TEST_ASSERT(sizeReturn==3);
    PEGASUS_TEST_ASSERT(isArrayReturn);

    PEGASUS_TEST_ASSERT(unionReturn[0].simple.val.bin);
    PEGASUS_TEST_ASSERT(unionReturn[0].simple.hasValue);
    PEGASUS_TEST_ASSERT(!unionReturn[1].simple.val.bin);
    PEGASUS_TEST_ASSERT(unionReturn[1].simple.hasValue);
    PEGASUS_TEST_ASSERT(unionReturn[2].simple.val.bin);
    PEGASUS_TEST_ASSERT(unionReturn[2].simple.hasValue);

    /**
     * Test DateTime
     */

    VCOUT << "Test DateTime" << endl;

    SCMBUnion myDateTimeValue;
    myDateTimeValue.dateTimeValue.usec = PEGASUS_UINT64_LITERAL(17236362);
    myDateTimeValue.dateTimeValue.utcOffset = 0;
    myDateTimeValue.dateTimeValue.sign = ':';
    myDateTimeValue.dateTimeValue.numWildcards = 0;

    rc = SCMO_TESTClass2_Inst.setPropertyWithOrigin(
        "DateTimeProperty",
        CIMTYPE_DATETIME,
        &myDateTimeValue);

    PEGASUS_TEST_ASSERT(rc==SCMO_OK);

    rc = SCMO_TESTClass2_Inst.getProperty(
        "DateTimeProperty",
        typeReturn,
        &unionReturn,
        isArrayReturn,
        sizeReturn);

    PEGASUS_TEST_ASSERT(rc==SCMO_OK);
    PEGASUS_TEST_ASSERT(sizeReturn==0);
    PEGASUS_TEST_ASSERT(!isArrayReturn);

    PEGASUS_TEST_ASSERT(
        memcmp(
            &(myDateTimeValue.dateTimeValue),
            &(unionReturn->dateTimeValue),
            sizeof(CIMDateTimeRep))== 0);

    SCMBUnion dateTimeArrayValue[3];

    dateTimeArrayValue[0].dateTimeValue.usec = Uint64(988243387);
    dateTimeArrayValue[0].dateTimeValue.utcOffset = 0;
    dateTimeArrayValue[0].dateTimeValue.sign = ':';
    dateTimeArrayValue[0].dateTimeValue.numWildcards = 0;

    dateTimeArrayValue[1].dateTimeValue.usec = Uint64(827383727);
    dateTimeArrayValue[1].dateTimeValue.utcOffset = 0;
    dateTimeArrayValue[1].dateTimeValue.sign = ':';
    dateTimeArrayValue[1].dateTimeValue.numWildcards = 0;

    dateTimeArrayValue[2].dateTimeValue.usec = Uint64(932933892);
    dateTimeArrayValue[2].dateTimeValue.utcOffset = 0;
    dateTimeArrayValue[2].dateTimeValue.sign = ':';
    dateTimeArrayValue[2].dateTimeValue.numWildcards = 0;

    rc = SCMO_TESTClass2_Inst.setPropertyWithOrigin(
        "DateTimePropertyArray",
        CIMTYPE_DATETIME,
        dateTimeArrayValue,
        true,3);

    PEGASUS_TEST_ASSERT(rc==SCMO_OK);

    rc = SCMO_TESTClass2_Inst.getProperty(
        "DateTimePropertyArray",
        typeReturn,
        &unionReturn,
        isArrayReturn,
        sizeReturn);

    PEGASUS_TEST_ASSERT(rc==SCMO_OK);
    PEGASUS_TEST_ASSERT(sizeReturn==3);
    PEGASUS_TEST_ASSERT(isArrayReturn);


    PEGASUS_TEST_ASSERT(
        memcmp(&(dateTimeArrayValue[0].dateTimeValue),
               &(unionReturn[0].dateTimeValue),
               sizeof(CIMDateTimeRep)
               )== 0);
    PEGASUS_TEST_ASSERT(
        memcmp(
            &(dateTimeArrayValue[1].dateTimeValue),
            &(unionReturn[1].dateTimeValue),
            sizeof(CIMDateTimeRep))== 0);
    PEGASUS_TEST_ASSERT(
        memcmp(
            &(dateTimeArrayValue[2].dateTimeValue),
            &(unionReturn[2].dateTimeValue),
            sizeof(CIMDateTimeRep))== 0);

    /**
     * Test string
     */

    VCOUT << "Test String" << endl;

    char ThisIsASingleString[] = "This is a single String!";
    SCMBUnion stringValue;
    stringValue.extString.pchar = &(ThisIsASingleString[0]);
    stringValue.extString.length = strlen(ThisIsASingleString);

    rc = SCMO_TESTClass2_Inst.setPropertyWithOrigin(
        "StringProperty",
        CIMTYPE_STRING,
        &stringValue);

    PEGASUS_TEST_ASSERT(rc==SCMO_OK);

    rc = SCMO_TESTClass2_Inst.getProperty(
        "StringProperty",
        typeReturn,
        &unionReturn,
        isArrayReturn,
        sizeReturn);

    PEGASUS_TEST_ASSERT(rc==SCMO_OK);
    PEGASUS_TEST_ASSERT(sizeReturn==0);
    PEGASUS_TEST_ASSERT(!isArrayReturn);

    PEGASUS_TEST_ASSERT(
        stringValue.extString.length==unionReturn->extString.length);
    PEGASUS_TEST_ASSERT(
        strcmp(
            stringValue.extString.pchar,
            unionReturn->extString.pchar) == 0);

    free((void*)unionReturn);

    char arraySting0[] = "The Array String Number one.";
    char arraySting1[] = "The Array String Number two.";
    char arraySting2[] = "The Array String Number three.";
    SCMBUnion stringArrayValue[3];
    stringArrayValue[0].extString.pchar= &(arraySting0[0]);
    stringArrayValue[0].extString.length= strlen(arraySting0);

    stringArrayValue[1].extString.pchar=&(arraySting1[0]);
    stringArrayValue[1].extString.length=strlen(arraySting1);

    stringArrayValue[2].extString.pchar=&(arraySting2[0]);
    stringArrayValue[2].extString.length=strlen(arraySting2);

    rc = SCMO_TESTClass2_Inst.setPropertyWithOrigin(
        "StringPropertyArray",
        CIMTYPE_STRING,
        stringArrayValue,
        true,3);

    PEGASUS_TEST_ASSERT(rc==SCMO_OK);

    rc = SCMO_TESTClass2_Inst.getProperty(
        "StringPropertyArray",
        typeReturn,
        &unionReturn,
        isArrayReturn,
        sizeReturn);

    PEGASUS_TEST_ASSERT(rc==SCMO_OK);
    PEGASUS_TEST_ASSERT(sizeReturn==3);
    PEGASUS_TEST_ASSERT(isArrayReturn);

    PEGASUS_TEST_ASSERT(
        stringArrayValue[0].extString.length==unionReturn[0].extString.length);
    PEGASUS_TEST_ASSERT(
        strcmp(
            stringArrayValue[0].extString.pchar,
            unionReturn[0].extString.pchar) == 0);
    PEGASUS_TEST_ASSERT(
        stringArrayValue[1].extString.length==unionReturn[1].extString.length);
    PEGASUS_TEST_ASSERT(
        strcmp(
            stringArrayValue[1].extString.pchar,
            unionReturn[1].extString.pchar) == 0);
    PEGASUS_TEST_ASSERT(
        stringArrayValue[2].extString.length==unionReturn[2].extString.length);
    PEGASUS_TEST_ASSERT(
        strcmp(
            stringArrayValue[2].extString.pchar,
            unionReturn[2].extString.pchar) == 0);

    // do not forget !!!
    free((void*)unionReturn);

    // Test the SCMOWriter against the created instance

    Buffer buf;
    Array<Uint32> emptyNodes;
    SCMOXmlWriter::appendInstanceElement(
        buf,
        SCMO_TESTClass2_Inst,
        false,
        emptyNodes);

    if (verbose)
    {
        cout << "Display of SCMOXmlWriter::appendInstanceElement" << endl;
        cout << endl << buf.getData() << endl;
    }
    const char * bufData = buf.getData();
    for (Uint32 i = 0; i < strlen(bufData); i ++)
    {
        if (bufData[i] != Class2XmlOut[i])
        {
            cout << " Error at char " << i << endl;
            break;
        }
    }

    if (strcmp(buf.getData(), Class2XmlOut) != 0)
    {
        cout << "Dumping Error in XmlWriter Output" << endl;
        cout << strlen(buf.getData()) << " " << strlen(Class2XmlOut) << endl;
        cout << endl << "=========" << endl << buf.getData()
             << endl << "==========" << endl << Class2XmlOut << endl;
    }

    PEGASUS_TEST_ASSERT(strcmp(buf.getData(), Class2XmlOut) == 0);

    VCOUT << endl << "SCMOInstancePropertyTest Done." << endl << endl;
}

void SCMOInstanceKeyBindingsTest()
{
    SCMO_RC rc;

    CIMType returnKeyBindType;
    const SCMBUnion * returnKeyBindValue;
    Uint32 noKeyBind;
    const char * returnName;

    SCMOClassCache* _theCache = SCMOClassCache::getInstance();

    SCMOClass SCMO_TESTClass2 = _theCache->getSCMOClass(
            "cimv2",
            strlen("cimv2"),
            "SCMO_TESTClass2",
            strlen("SCMO_TESTClass2"));

    SCMOInstance SCMO_TESTClass2_Inst(SCMO_TESTClass2);

    SCMBUnion uint64KeyVal;
    uint64KeyVal.simple.val.u64 = PEGASUS_UINT64_LITERAL(4834987289728);
    uint64KeyVal.simple.hasValue= true;

    SCMBUnion boolKeyVal;
    boolKeyVal.simple.val.bin  = true;
    boolKeyVal.simple.hasValue = true;

    SCMBUnion real32KeyVal;
    real32KeyVal.simple.val.r32 = 3.9399998628365712e+30;
    real32KeyVal.simple.hasValue = true;

    char stringKeyBinding[] = "This is the String key binding.";
    SCMBUnion stringKeyVal;
    stringKeyVal.extString.pchar = &(stringKeyBinding[0]);
    stringKeyVal.extString.length = strlen(stringKeyBinding);

    /**
     * Test Key bindings
     */

    VCOUT << "Key Bindings Tests." << endl << endl;

    VCOUT << "Wrong key binding type." << endl;
    // Real32Property is a key property
    rc = SCMO_TESTClass2_Inst.setKeyBinding(
        "Real32Property",
        CIMTYPE_BOOLEAN,
        &real32KeyVal);

    PEGASUS_TEST_ASSERT(rc==SCMO_TYPE_MISSMATCH);

    VCOUT << "Key binding not set." << endl;

    rc = SCMO_TESTClass2_Inst.getKeyBinding(
        "Real32Property",
        returnKeyBindType,
        &returnKeyBindValue);

    PEGASUS_TEST_ASSERT(rc==SCMO_NULL_VALUE);
    PEGASUS_TEST_ASSERT(returnKeyBindType==CIMTYPE_REAL32);
    PEGASUS_TEST_ASSERT(returnKeyBindValue==NULL);

    // set key bindings

    rc = SCMO_TESTClass2_Inst.setKeyBinding(
        "StringProperty",
        CIMTYPE_STRING,
        &stringKeyVal);

    PEGASUS_TEST_ASSERT(rc==SCMO_OK);

    rc = SCMO_TESTClass2_Inst.setKeyBinding(
        "Real32Property",
        CIMTYPE_REAL32,
        &real32KeyVal);

    PEGASUS_TEST_ASSERT(rc==SCMO_OK);

    rc = SCMO_TESTClass2_Inst.setKeyBinding(
        "Uint64Property",
        CIMTYPE_UINT64,
        &uint64KeyVal);

    PEGASUS_TEST_ASSERT(rc==SCMO_OK);

    rc = SCMO_TESTClass2_Inst.getKeyBinding(
        "Real32Property",
        returnKeyBindType,
        &returnKeyBindValue);

    PEGASUS_TEST_ASSERT(rc==SCMO_OK);
    PEGASUS_TEST_ASSERT(returnKeyBindType==CIMTYPE_REAL32);
    PEGASUS_TEST_ASSERT(returnKeyBindValue->simple.val.r32 ==
                        real32KeyVal.simple.val.r32);

    rc = SCMO_TESTClass2_Inst.getKeyBinding(
        "Uint64Property",
        returnKeyBindType,
        &returnKeyBindValue);

    PEGASUS_TEST_ASSERT(rc==SCMO_OK);
    PEGASUS_TEST_ASSERT(returnKeyBindType==CIMTYPE_UINT64);
    PEGASUS_TEST_ASSERT(returnKeyBindValue->simple.val.u64 ==
                        uint64KeyVal.simple.val.u64);

    rc = SCMO_TESTClass2_Inst.getKeyBinding(
        "StringProperty",
        returnKeyBindType,
        &returnKeyBindValue);

    PEGASUS_TEST_ASSERT(rc==SCMO_OK);
    PEGASUS_TEST_ASSERT(returnKeyBindType==CIMTYPE_STRING);

    PEGASUS_TEST_ASSERT(
        stringKeyVal.extString.length==returnKeyBindValue->extString.length);
    PEGASUS_TEST_ASSERT(
        strcmp(
            stringKeyVal.extString.pchar,
            returnKeyBindValue->extString.pchar) == 0);

    // do not forget !
    free((void*)returnKeyBindValue);

    VCOUT << "Get Key binding by index." << endl;

    noKeyBind = SCMO_TESTClass2_Inst.getKeyBindingCount();

    PEGASUS_TEST_ASSERT(noKeyBind==3);

    VCOUT << "Test index boundaries." << endl;

    rc = SCMO_TESTClass2_Inst.getKeyBindingAt(
        noKeyBind,
        &returnName,
        returnKeyBindType,
        &returnKeyBindValue);

    PEGASUS_TEST_ASSERT(rc==SCMO_INDEX_OUT_OF_BOUND);
    PEGASUS_TEST_ASSERT(returnKeyBindValue==NULL);

    VCOUT << "Iterate for index 0 to " << noKeyBind-1 << "." << endl;
    for (Uint32 i = 0; i < noKeyBind; i++)
    {
        rc = SCMO_TESTClass2_Inst.getKeyBindingAt(
            i,
            &returnName,
            returnKeyBindType,
            &returnKeyBindValue);

        if (returnKeyBindType == CIMTYPE_STRING)
        {
            free((void*)returnKeyBindValue);
        }
        PEGASUS_TEST_ASSERT(rc==SCMO_OK);
    }

    VCOUT << "Test User defined Key Bindings." << endl;

    char stringKeyBinding2[]="This is the as User defined String key binding.";
    SCMBUnion stringUserKeyVal;
    stringUserKeyVal.extString.pchar = &(stringKeyBinding2[0]);
    stringUserKeyVal.extString.length = strlen(stringKeyBinding2);

    rc = SCMO_TESTClass2_Inst.setKeyBinding(
        "UserStringProperty",
        CIMTYPE_STRING,
        &stringUserKeyVal);


    rc = SCMO_TESTClass2_Inst.getKeyBinding(
        "UserStringProperty",
        returnKeyBindType,
        &returnKeyBindValue);

    PEGASUS_TEST_ASSERT(rc==SCMO_OK);
    PEGASUS_TEST_ASSERT(returnKeyBindType==CIMTYPE_STRING);

    PEGASUS_TEST_ASSERT(
        stringUserKeyVal.extString.length==
        returnKeyBindValue->extString.length);
    PEGASUS_TEST_ASSERT(
        strcmp(
            stringUserKeyVal.extString.pchar,
            returnKeyBindValue->extString.pchar) == 0);

    // do not forget !
    free((void*)returnKeyBindValue);

    VCOUT << "Test User defined Key Bindings with index." << endl;

    noKeyBind = SCMO_TESTClass2_Inst.getKeyBindingCount();
    PEGASUS_TEST_ASSERT(rc==SCMO_OK);
    PEGASUS_TEST_ASSERT(noKeyBind == 4);

    rc = SCMO_TESTClass2_Inst.getKeyBindingAt(
        noKeyBind,
        &returnName,
        returnKeyBindType,
        &returnKeyBindValue);

    PEGASUS_TEST_ASSERT(rc==SCMO_INDEX_OUT_OF_BOUND);
    PEGASUS_TEST_ASSERT(returnKeyBindValue==NULL);

    VCOUT << "Iterate for index 0 to " << noKeyBind-1 << "." << endl;
    for (Uint32 i = 0; i < noKeyBind; i++)
    {
        rc = SCMO_TESTClass2_Inst.getKeyBindingAt(
            i,
            &returnName,
            returnKeyBindType,
            &returnKeyBindValue);

        if (returnKeyBindType == CIMTYPE_STRING)
        {
            free((void*)returnKeyBindValue);
        }
        PEGASUS_TEST_ASSERT(rc==SCMO_OK);
    }

    VCOUT << "Test CIMObjectpath." << endl;

    CIMObjectPath theCIMPath;

    SCMO_TESTClass2_Inst.getCIMObjectPath(theCIMPath);

     Array<CIMKeyBinding> theCIMKeyBindings =
         theCIMPath.getKeyBindings();

    PEGASUS_TEST_ASSERT(theCIMKeyBindings.size() == 4);

    SCMOClass theDummySCMOInstance(SCMO_TESTClass2_Inst.getClassName(),
                  SCMO_TESTClass2_Inst.getNameSpace());

    SCMOInstance theDummyInstance(theDummySCMOInstance,theCIMPath);

    theDummyInstance.markAsCompromised();

    for (Uint32 i = 0 ; i < theDummyInstance.getKeyBindingCount();i++)
    {
        rc = theDummyInstance.getKeyBindingAt(
            i,
            &returnName,
            returnKeyBindType,
            &returnKeyBindValue);

        PEGASUS_TEST_ASSERT(rc==SCMO_OK);

        rc = SCMO_TESTClass2_Inst.setKeyBinding(
            returnName,
            returnKeyBindType,
            returnKeyBindValue);

        PEGASUS_TEST_ASSERT(rc==SCMO_OK);

        if (returnKeyBindType == CIMTYPE_STRING)
        {
            free((void*)returnKeyBindValue);
        }

    }
    VCOUT << endl << "SCMOInstanceKeyBindingsTest Done." << endl;
}


void SCMOInstanceConverterTest()
{

    CIMClass CIM_CSClass;
    CIMInstance CIM_CSInstance;
    Buffer text;

    VCOUT << endl << "Conversion Test CIM<->SCMO.." << endl;
    VCOUT << endl << "Loading CIMComputerSystemClass.xml" << endl;

    String TestCSClassXML (getenv("PEGASUS_ROOT"));
    TestCSClassXML.append(TESTCSCLASSXML);

    FileSystem::loadFileToMemory(text,(const char*)TestCSClassXML.getCString());

    XmlParser theParser((char*)text.getData());
    XmlReader::getObject(theParser,CIM_CSClass);

    text.clear();

    VCOUT << "Loading CIMComputerSystemInstance.xml" << endl;

    String TestCSInstXML (getenv("PEGASUS_ROOT"));
    TestCSInstXML.append(TESTCSINSTXML);

    FileSystem::loadFileToMemory(text,(const char*)TestCSInstXML.getCString());

    XmlParser theParser2((char*)text.getData());
    XmlReader::getObject(theParser2,CIM_CSInstance);

    VCOUT << "Creating SCMOClass from CIMClass" << endl;

    SCMOClass SCMO_CSClass(CIM_CSClass);

    VCOUT << "Creating SCMOInstance from CIMInstance" << endl;
    SCMOInstance SCMO_CSInstance(SCMO_CSClass, CIM_CSInstance);

    CIMInstance newInstance;

    VCOUT << "Converting CIMInstance from SCMOInstance" << endl;
    SCMO_CSInstance.getCIMInstance(newInstance);

    PEGASUS_TEST_ASSERT(newInstance.identical(CIM_CSInstance));

    VCOUT << endl << "SCMOInstanceConverterTest Done." << endl << endl;
}

/*
    Test implementation of UserDefined Instances. This tests the methodsL
        NewInstance
        setPropertyWithOrigin
        getProperty

    For the case where instances are created for classes that do not
    exist in the repository and properties inserted into those classes.

*/
void SCMOUserDefinedInstancePropertyTest()
{
    VCOUT << "Start SCMOUserDefinedInstancePropertyTest" << endl;

    SCMO_RC rc;
    // definition of return values from getProperty.
    const SCMBUnion* unionReturn;
    CIMType typeReturn;
    Boolean isArrayReturn;
    Uint32 sizeReturn;

    // Create class in cache but do not use the created class
    // since creating instances for non-existent class.
    SCMOClassCache* _theCache = SCMOClassCache::getInstance();

    SCMOClass SCMO_TESTClass2 = _theCache->getSCMOClass(
            "cimv2",
            strlen("cimv2"),
            "SCMO_TESTClass2",
            strlen("SCMO_TESTClass2"));

    // Create the class and instance for the class that does not
    // exist in the cache.
    SCMOClass SCMO_TestClassNotExist("_ClassNameNotExist", "Namespace");

    SCMOInstance SCMO_TESTClassNotExist_Inst(SCMO_TestClassNotExist);

    // Test the noClass flag.  This instance should be marked as
    // not having a class.
    SCMO_TESTClassNotExist_Inst.markNoClassForInstance(true);

    PEGASUS_TEST_ASSERT(SCMO_TESTClassNotExist_Inst.noClassForInstance());

    /*
        Test Creating a simple property and test manipulating
        that property
    */

    VCOUT << "Test Boolean Property value false" << endl;

    // Create a first property.
    SCMBUnion boolValue0;
    boolValue0.simple.val.bin=false;
    boolValue0.simple.hasValue=true;
    rc = SCMO_TESTClassNotExist_Inst.setPropertyWithOrigin(
        "BooleanPropertyValueFalse",
        CIMTYPE_BOOLEAN,
        &boolValue0);

    PEGASUS_TEST_ASSERT(rc==SCMO_OK);

    // set dump for diagnostics
#ifdef PEGASUS_DEBUG
    SCMODump sd;
#endif
    VSCMODUMP(sd.dumpSCMOInstance(SCMO_TESTClassNotExist_Inst, true, true));

    SCMBUnion boolValue;
    boolValue.simple.val.bin=true;
    boolValue.simple.hasValue=true;

    PEGASUS_TEST_ASSERT(SCMO_TESTClassNotExist_Inst.getPropertyCount() == 1);

    rc = SCMO_TESTClassNotExist_Inst.getProperty(
        "BooleanPropertyValueFalse",
        typeReturn,
        &unionReturn,
        isArrayReturn,
        sizeReturn);

    PEGASUS_TEST_ASSERT(rc==SCMO_OK);
    PEGASUS_TEST_ASSERT(sizeReturn==0);
    PEGASUS_TEST_ASSERT(!isArrayReturn);
    PEGASUS_TEST_ASSERT(typeReturn == CIMTYPE_BOOLEAN);
    PEGASUS_TEST_ASSERT(unionReturn->simple.hasValue);
    PEGASUS_TEST_ASSERT(unionReturn->simple.val.bin == false);

    // Test errors now that we have a property
    PEGASUS_TEST_ASSERT(SCMO_TESTClassNotExist_Inst.getPropertyCount() == 1);

    // Test for error setting new type into  existingproperty.
    {
        SCMBUnion uint32value;
        uint32value.simple.val.u32=0xF7F7F7F7;
        uint32value.simple.hasValue=true;

        rc = SCMO_TESTClassNotExist_Inst.setPropertyWithOrigin(
            "BooleanPropertyValueFalse",
            CIMTYPE_UINT32,
            &uint32value);

        PEGASUS_TEST_ASSERT(rc==SCMO_TYPE_MISSMATCH);
    }
    PEGASUS_TEST_ASSERT(SCMO_TESTClassNotExist_Inst.getPropertyCount() == 1);

    // Test for error setting to Boolean array.  Should be refused.
    SCMBUnion *BooleanArrayValue = (SCMBUnion*)malloc(1*sizeof(Boolean));

    rc = SCMO_TESTClassNotExist_Inst.setPropertyWithOrigin(
        "BooleanPropertyValueFalse",
        CIMTYPE_BOOLEAN,
        BooleanArrayValue,
        true,0);

    PEGASUS_TEST_ASSERT(rc!= SCMO_OK);
    PEGASUS_TEST_ASSERT(SCMO_TESTClassNotExist_Inst.getPropertyCount() == 1);

    // Test Setting the value to a different value
    boolValue0.simple.val.bin=true;
    boolValue0.simple.hasValue=true;
    rc = SCMO_TESTClassNotExist_Inst.setPropertyWithOrigin(
        "BooleanPropertyValueFalse",
        CIMTYPE_BOOLEAN,
        &boolValue0);

    PEGASUS_TEST_ASSERT(rc==SCMO_OK);

    VSCMODUMP(sd.dumpSCMOInstance(SCMO_TESTClassNotExist_Inst, true, true));

    boolValue.simple.val.bin=true;
    boolValue.simple.hasValue=true;

    PEGASUS_TEST_ASSERT(SCMO_TESTClassNotExist_Inst.getPropertyCount() == 1);

    rc = SCMO_TESTClassNotExist_Inst.getProperty(
        "BooleanPropertyValueFalse",
        typeReturn,
        &unionReturn,
        isArrayReturn,
        sizeReturn);

    PEGASUS_TEST_ASSERT(rc==SCMO_OK);
    PEGASUS_TEST_ASSERT(sizeReturn==0);
    PEGASUS_TEST_ASSERT(!isArrayReturn);
    PEGASUS_TEST_ASSERT(typeReturn == CIMTYPE_BOOLEAN);
    PEGASUS_TEST_ASSERT(unionReturn->simple.hasValue);
    PEGASUS_TEST_ASSERT(unionReturn->simple.val.bin == true);

    // Test errors now that we have a property
    PEGASUS_TEST_ASSERT(SCMO_TESTClassNotExist_Inst.getPropertyCount() == 1);

    /*
        Test all of the various property types
    */
    VCOUT << "Set a Boolean scalar property" << endl;

    rc = SCMO_TESTClassNotExist_Inst.setPropertyWithOrigin(
        "BooleanProperty",
        CIMTYPE_BOOLEAN,
        &boolValue);

    PEGASUS_TEST_ASSERT(rc==SCMO_OK);

    PEGASUS_TEST_ASSERT(SCMO_TESTClassNotExist_Inst.getPropertyCount() == 2);

    VSCMODUMP(sd.dumpSCMOInstance(SCMO_TESTClassNotExist_Inst, true, true));


    rc = SCMO_TESTClassNotExist_Inst.getProperty(
        "BooleanProperty",
        typeReturn,
        &unionReturn,
        isArrayReturn,
        sizeReturn);

    PEGASUS_TEST_ASSERT(rc==SCMO_OK);
    PEGASUS_TEST_ASSERT(sizeReturn==0);
    PEGASUS_TEST_ASSERT(!isArrayReturn);
    PEGASUS_TEST_ASSERT(typeReturn == CIMTYPE_BOOLEAN);
    PEGASUS_TEST_ASSERT(unionReturn->simple.hasValue);
    PEGASUS_TEST_ASSERT(unionReturn->simple.val.bin);

    VCOUT << "Test Empty Array." << endl;

    // this just to get an valid pointer but I put no elements in it
    SCMBUnion *uint32ArrayValue = (SCMBUnion*)malloc(1*sizeof(Uint32));

    rc = SCMO_TESTClassNotExist_Inst.setPropertyWithOrigin(
        "Uint32PropertyArray",
        CIMTYPE_UINT32,
        uint32ArrayValue,
        true,0);

    PEGASUS_TEST_ASSERT(rc==SCMO_OK);

    VSCMODUMP(sd.dumpSCMOInstance(SCMO_TESTClassNotExist_Inst, true, true));

    PEGASUS_TEST_ASSERT(SCMO_TESTClassNotExist_Inst.getPropertyCount() == 3);

    rc = SCMO_TESTClassNotExist_Inst.getProperty(
        "Uint32PropertyArray",
        typeReturn,
        &unionReturn,
        isArrayReturn,
        sizeReturn);

    PEGASUS_TEST_ASSERT(rc==SCMO_OK);
    PEGASUS_TEST_ASSERT(sizeReturn==0);
    PEGASUS_TEST_ASSERT(isArrayReturn);
    PEGASUS_TEST_ASSERT(unionReturn==NULL);

    free(uint32ArrayValue);

    // test getPropertyAt and confirm at least one of the properties was found
    bool instanceFound = false;
    for (Uint32 i = 0 ; i < SCMO_TESTClassNotExist_Inst.getPropertyCount(); i++)
    {
        const SCMBUnion* value = 0;
        Boolean isArray = 0;
        Uint32 size = 0;
        CIMType type = (CIMType)0;
        const char* pName=0;
        SCMO_TESTClassNotExist_Inst.getPropertyAt(i,&pName,
                                          type,
                                          &value,
                                          isArray,
                                          size);
        if (strcmp(pName, "Uint32PropertyArray") == 0 )
        {
            PEGASUS_TEST_ASSERT(type == CIMTYPE_UINT32);
        }
        instanceFound = true;
    }
    PEGASUS_TEST_ASSERT(instanceFound);

    /**
     * Test Char16
     */

    VCOUT << endl << "Test Char16" << endl;

    SCMBUnion char16value;
    char16value.simple.val.c16=0x3F4A;
    char16value.simple.hasValue=true;

    rc = SCMO_TESTClassNotExist_Inst.setPropertyWithOrigin(
        "Char16Property",
        CIMTYPE_CHAR16,
        &char16value);

    PEGASUS_TEST_ASSERT(rc==SCMO_OK);

    rc = SCMO_TESTClassNotExist_Inst.getProperty(
        "Char16Property",
        typeReturn,
        &unionReturn,
        isArrayReturn,
        sizeReturn);

    PEGASUS_TEST_ASSERT(rc==SCMO_OK);
    PEGASUS_TEST_ASSERT(sizeReturn==0);
    PEGASUS_TEST_ASSERT(!isArrayReturn);
    PEGASUS_TEST_ASSERT(
        char16value.simple.val.c16 == unionReturn->simple.val.c16);
    PEGASUS_TEST_ASSERT(unionReturn->simple.hasValue);

    SCMBUnion char16ArrayValue[3];
    char16ArrayValue[0].simple.val.c16 = 1024;
    char16ArrayValue[0].simple.hasValue=true;
    char16ArrayValue[1].simple.val.c16 = 2048;
    char16ArrayValue[1].simple.hasValue=true;
    char16ArrayValue[2].simple.val.c16 = 4096;
    char16ArrayValue[2].simple.hasValue=true;

    rc = SCMO_TESTClassNotExist_Inst.setPropertyWithOrigin(
        "Char16PropertyArray",
        CIMTYPE_CHAR16,
        char16ArrayValue,
        true,3);

    PEGASUS_TEST_ASSERT(rc==SCMO_OK);

    rc = SCMO_TESTClassNotExist_Inst.getProperty(
        "Char16PropertyArray",
        typeReturn,
        &unionReturn,
        isArrayReturn,
        sizeReturn);

    PEGASUS_TEST_ASSERT(sizeReturn==3);
    PEGASUS_TEST_ASSERT(isArrayReturn);
    PEGASUS_TEST_ASSERT(rc==SCMO_OK);

    PEGASUS_TEST_ASSERT(
        char16ArrayValue[0].simple.val.c16 == unionReturn[0].simple.val.c16);
    PEGASUS_TEST_ASSERT(unionReturn[0].simple.hasValue);
    PEGASUS_TEST_ASSERT(
        char16ArrayValue[1].simple.val.c16 == unionReturn[1].simple.val.c16);
    PEGASUS_TEST_ASSERT(unionReturn[1].simple.hasValue);
    PEGASUS_TEST_ASSERT(
        char16ArrayValue[2].simple.val.c16 == unionReturn[2].simple.val.c16);
    PEGASUS_TEST_ASSERT(unionReturn[2].simple.hasValue);


    /**
     * Test Uint8
     */
    VCOUT << "Test Uint8" << endl;

    SCMBUnion uint8value;
    uint8value.simple.val.u8=0x77;
    uint8value.simple.hasValue=true;

    rc = SCMO_TESTClassNotExist_Inst.setPropertyWithOrigin(
        "Uint8Property",
        CIMTYPE_UINT8,
        &uint8value);

    PEGASUS_TEST_ASSERT(rc==SCMO_OK);

    rc = SCMO_TESTClassNotExist_Inst.getProperty(
        "Uint8Property",
        typeReturn,
        &unionReturn,
        isArrayReturn,
        sizeReturn);

    PEGASUS_TEST_ASSERT(rc==SCMO_OK);
    PEGASUS_TEST_ASSERT(sizeReturn==0);
    PEGASUS_TEST_ASSERT(!isArrayReturn);

    PEGASUS_TEST_ASSERT(
        uint8value.simple.val.u8 == unionReturn->simple.val.u8);
    PEGASUS_TEST_ASSERT(unionReturn->simple.hasValue);

    SCMBUnion uint8ArrayValue[3];
    uint8ArrayValue[0].simple.val.u8 = 42;
    uint8ArrayValue[0].simple.hasValue=true;
    uint8ArrayValue[1].simple.val.u8 = 155;
    uint8ArrayValue[1].simple.hasValue=true;
    uint8ArrayValue[2].simple.val.u8 = 192;
    uint8ArrayValue[2].simple.hasValue=true;

    rc = SCMO_TESTClassNotExist_Inst.setPropertyWithOrigin(
        "Uint8PropertyArray",
        CIMTYPE_UINT8,
        uint8ArrayValue,
        true,3);

    PEGASUS_TEST_ASSERT(rc==SCMO_OK);

    rc = SCMO_TESTClassNotExist_Inst.getProperty(
        "Uint8PropertyArray",
        typeReturn,
        &unionReturn,
        isArrayReturn,
        sizeReturn);

    PEGASUS_TEST_ASSERT(sizeReturn==3);
    PEGASUS_TEST_ASSERT(isArrayReturn);
    PEGASUS_TEST_ASSERT(rc==SCMO_OK);

    PEGASUS_TEST_ASSERT(
        uint8ArrayValue[0].simple.val.u8 == unionReturn[0].simple.val.u8);
    PEGASUS_TEST_ASSERT(unionReturn[0].simple.hasValue);
    PEGASUS_TEST_ASSERT(
        uint8ArrayValue[1].simple.val.u8 == unionReturn[1].simple.val.u8);
    PEGASUS_TEST_ASSERT(unionReturn[1].simple.hasValue);
    PEGASUS_TEST_ASSERT(
        uint8ArrayValue[2].simple.val.u8 == unionReturn[2].simple.val.u8);
    PEGASUS_TEST_ASSERT(unionReturn[2].simple.hasValue);

    /**
     * Test Uint16
     */

    VCOUT << "Test Uint16" << endl;

    SCMBUnion uint16value;
    uint16value.simple.val.u16=0xF77F;
    uint16value.simple.hasValue=true;

    rc = SCMO_TESTClassNotExist_Inst.setPropertyWithOrigin(
        "Uint16Property",
        CIMTYPE_UINT16,
        &uint16value);

    PEGASUS_TEST_ASSERT(rc==SCMO_OK);

    rc = SCMO_TESTClassNotExist_Inst.getProperty(
        "Uint16Property",
        typeReturn,
        &unionReturn,
        isArrayReturn,
        sizeReturn);

    PEGASUS_TEST_ASSERT(rc==SCMO_OK);
    PEGASUS_TEST_ASSERT(sizeReturn==0);
    PEGASUS_TEST_ASSERT(!isArrayReturn);

    PEGASUS_TEST_ASSERT(
        uint16value.simple.val.u16 == unionReturn->simple.val.u16);
    PEGASUS_TEST_ASSERT(unionReturn->simple.hasValue);

    SCMBUnion uint16ArrayValue[3];
    uint16ArrayValue[0].simple.val.u16 = 218;
    uint16ArrayValue[0].simple.hasValue=true;
    uint16ArrayValue[1].simple.val.u16 = 2673;
    uint16ArrayValue[1].simple.hasValue=true;
    uint16ArrayValue[2].simple.val.u16 = 172;
    uint16ArrayValue[2].simple.hasValue=true;

    rc = SCMO_TESTClassNotExist_Inst.setPropertyWithOrigin(
        "Uint16PropertyArray",
        CIMTYPE_UINT16,
        uint16ArrayValue,
        true,3);

    PEGASUS_TEST_ASSERT(rc==SCMO_OK);

    rc = SCMO_TESTClassNotExist_Inst.getProperty(
        "Uint16PropertyArray",
        typeReturn,
        &unionReturn,
        isArrayReturn,
        sizeReturn);

    PEGASUS_TEST_ASSERT(rc==SCMO_OK);
    PEGASUS_TEST_ASSERT(sizeReturn==3);
    PEGASUS_TEST_ASSERT(isArrayReturn);

    PEGASUS_TEST_ASSERT(
        uint16ArrayValue[0].simple.val.u16 == unionReturn[0].simple.val.u16);
    PEGASUS_TEST_ASSERT(unionReturn[0].simple.hasValue);
    PEGASUS_TEST_ASSERT(
        uint16ArrayValue[1].simple.val.u16 == unionReturn[1].simple.val.u16);
    PEGASUS_TEST_ASSERT(unionReturn[1].simple.hasValue);
    PEGASUS_TEST_ASSERT(
        uint16ArrayValue[2].simple.val.u16 == unionReturn[2].simple.val.u16);
    PEGASUS_TEST_ASSERT(unionReturn[2].simple.hasValue);



    /**
     * Test Uint32
     */

    VCOUT << "Test Uint32" << endl;

    SCMBUnion uint32value;
    uint32value.simple.val.u32=0xF7F7F7F7;
    uint32value.simple.hasValue=true;

    rc = SCMO_TESTClassNotExist_Inst.setPropertyWithOrigin(
        "Uint32Property",
        CIMTYPE_UINT32,
        &uint32value);

    PEGASUS_TEST_ASSERT(rc==SCMO_OK);

    rc = SCMO_TESTClassNotExist_Inst.getProperty(
        "Uint32Property",
        typeReturn,
        &unionReturn,
        isArrayReturn,
        sizeReturn);

    PEGASUS_TEST_ASSERT(rc==SCMO_OK);
    PEGASUS_TEST_ASSERT(sizeReturn==0);
    PEGASUS_TEST_ASSERT(!isArrayReturn);

    PEGASUS_TEST_ASSERT(
        uint32value.simple.val.u32 == unionReturn->simple.val.u32);
    PEGASUS_TEST_ASSERT(unionReturn->simple.hasValue);

    SCMBUnion uint32ArrayValue2[3];
    uint32ArrayValue2[0].simple.val.u32 = 42;
    uint32ArrayValue2[0].simple.hasValue=true;
    uint32ArrayValue2[1].simple.val.u32 = 289;
    uint32ArrayValue2[1].simple.hasValue=true;
    uint32ArrayValue2[2].simple.val.u32 = 192;
    uint32ArrayValue2[2].simple.hasValue=true;

    rc = SCMO_TESTClassNotExist_Inst.setPropertyWithOrigin(
        "Uint32PropertyArray",
        CIMTYPE_UINT32,
        uint32ArrayValue2,
        true,3);

    PEGASUS_TEST_ASSERT(rc==SCMO_OK);

    rc = SCMO_TESTClassNotExist_Inst.getProperty(
        "Uint32PropertyArray",
        typeReturn,
        &unionReturn,
        isArrayReturn,
        sizeReturn);

    PEGASUS_TEST_ASSERT(sizeReturn==3);
    PEGASUS_TEST_ASSERT(isArrayReturn);
    PEGASUS_TEST_ASSERT(rc==SCMO_OK);

    PEGASUS_TEST_ASSERT(
        uint32ArrayValue2[0].simple.val.u32 == unionReturn[0].simple.val.u32);
    PEGASUS_TEST_ASSERT(unionReturn[0].simple.hasValue);
    PEGASUS_TEST_ASSERT(
        uint32ArrayValue2[1].simple.val.u32 == unionReturn[1].simple.val.u32);
    PEGASUS_TEST_ASSERT(unionReturn[1].simple.hasValue);
    PEGASUS_TEST_ASSERT(
        uint32ArrayValue2[2].simple.val.u32 == unionReturn[2].simple.val.u32);
    PEGASUS_TEST_ASSERT(unionReturn[2].simple.hasValue);


    /**
     * Test Uint64
     */

    VCOUT << "Test Uint64" << endl;

    SCMBUnion uint64value;
    uint64value.simple.val.u64=PEGASUS_UINT64_LITERAL(0xA0A0B0B0C0C0D0D0);
    uint64value.simple.hasValue=true;

    rc = SCMO_TESTClassNotExist_Inst.setPropertyWithOrigin(
        "Uint64Property",
        CIMTYPE_UINT64,
        &uint64value);

    PEGASUS_TEST_ASSERT(rc==SCMO_OK);

    rc = SCMO_TESTClassNotExist_Inst.getProperty(
        "Uint64Property",
        typeReturn,
        &unionReturn,
        isArrayReturn,
        sizeReturn);

    PEGASUS_TEST_ASSERT(rc==SCMO_OK);
    PEGASUS_TEST_ASSERT(sizeReturn==0);
    PEGASUS_TEST_ASSERT(!isArrayReturn);

    PEGASUS_TEST_ASSERT(
        uint64value.simple.val.u64 == unionReturn->simple.val.u64);
    PEGASUS_TEST_ASSERT(unionReturn->simple.hasValue);

    SCMBUnion uint64ArrayValue[4];
    uint64ArrayValue[0].simple.val.u64 = 394;
    uint64ArrayValue[0].simple.hasValue=true;
    uint64ArrayValue[1].simple.val.u64 = 483734;
    uint64ArrayValue[1].simple.hasValue=true;
    uint64ArrayValue[2].simple.val.u64 =
        PEGASUS_UINT64_LITERAL(0x1234567890ABCDEF);
    uint64ArrayValue[2].simple.hasValue=true;
    uint64ArrayValue[3].simple.val.u64 = 23903483;
    uint64ArrayValue[3].simple.hasValue=true;

    rc = SCMO_TESTClassNotExist_Inst.setPropertyWithOrigin(
        "Uint64PropertyArray",
        CIMTYPE_UINT64,
        uint64ArrayValue,
        true,4);

    PEGASUS_TEST_ASSERT(rc==SCMO_OK);

    rc = SCMO_TESTClassNotExist_Inst.getProperty(
        "Uint64PropertyArray",
        typeReturn,
        &unionReturn,
        isArrayReturn,
        sizeReturn);

    PEGASUS_TEST_ASSERT(sizeReturn==4);
    PEGASUS_TEST_ASSERT(isArrayReturn);
    PEGASUS_TEST_ASSERT(rc==SCMO_OK);

    PEGASUS_TEST_ASSERT(
        uint64ArrayValue[0].simple.val.u64 == unionReturn[0].simple.val.u64);
    PEGASUS_TEST_ASSERT(unionReturn[0].simple.hasValue);
    PEGASUS_TEST_ASSERT(
        uint64ArrayValue[1].simple.val.u64 == unionReturn[1].simple.val.u64);
    PEGASUS_TEST_ASSERT(unionReturn[1].simple.hasValue);
    PEGASUS_TEST_ASSERT(
        uint64ArrayValue[2].simple.val.u64 == unionReturn[2].simple.val.u64);
    PEGASUS_TEST_ASSERT(unionReturn[2].simple.hasValue);
    PEGASUS_TEST_ASSERT(
        uint64ArrayValue[3].simple.val.u64 == unionReturn[3].simple.val.u64);
    PEGASUS_TEST_ASSERT(unionReturn[3].simple.hasValue);

    /**
     * Test Sint8
     */
    VCOUT << "Test Sint8" << endl;

    SCMBUnion sint8value;
    sint8value.simple.val.s8=Sint8(0xF3);
    sint8value.simple.hasValue=true;

    rc = SCMO_TESTClassNotExist_Inst.setPropertyWithOrigin(
        "Sint8Property",
        CIMTYPE_SINT8,
        &sint8value);

    PEGASUS_TEST_ASSERT(rc==SCMO_OK);

    rc = SCMO_TESTClassNotExist_Inst.getProperty(
        "Sint8Property",
        typeReturn,
        &unionReturn,
        isArrayReturn,
        sizeReturn);

    PEGASUS_TEST_ASSERT(rc==SCMO_OK);
    PEGASUS_TEST_ASSERT(sizeReturn==0);
    PEGASUS_TEST_ASSERT(!isArrayReturn);

    PEGASUS_TEST_ASSERT(
        sint8value.simple.val.s8 == unionReturn->simple.val.s8);
    PEGASUS_TEST_ASSERT(unionReturn->simple.hasValue);

    SCMBUnion sint8ArrayValue[3];
    sint8ArrayValue[0].simple.val.s8 = -2;
    sint8ArrayValue[0].simple.hasValue=true;
    sint8ArrayValue[1].simple.val.s8 = 94;
    sint8ArrayValue[1].simple.hasValue=true;
    sint8ArrayValue[2].simple.val.s8 = -123;
    sint8ArrayValue[2].simple.hasValue=true;

    rc = SCMO_TESTClassNotExist_Inst.setPropertyWithOrigin(
        "Sint8PropertyArray",
        CIMTYPE_SINT8,
        sint8ArrayValue,
        true,3);

    PEGASUS_TEST_ASSERT(rc==SCMO_OK);

    rc = SCMO_TESTClassNotExist_Inst.getProperty(
        "Sint8PropertyArray",
        typeReturn,
        &unionReturn,
        isArrayReturn,
        sizeReturn);

    PEGASUS_TEST_ASSERT(sizeReturn==3);
    PEGASUS_TEST_ASSERT(isArrayReturn);
    PEGASUS_TEST_ASSERT(rc==SCMO_OK);

    PEGASUS_TEST_ASSERT(
        sint8ArrayValue[0].simple.val.s8 == unionReturn[0].simple.val.s8);
    PEGASUS_TEST_ASSERT(unionReturn[0].simple.hasValue);
    PEGASUS_TEST_ASSERT(
        sint8ArrayValue[1].simple.val.s8 == unionReturn[1].simple.val.s8);
    PEGASUS_TEST_ASSERT(unionReturn[1].simple.hasValue);
    PEGASUS_TEST_ASSERT(
        sint8ArrayValue[2].simple.val.s8 == unionReturn[2].simple.val.s8);
    PEGASUS_TEST_ASSERT(unionReturn[2].simple.hasValue);

    /**
     * Test Sint16
     */

    VCOUT << "Test Sint16" << endl;

    SCMBUnion sint16value;
    sint16value.simple.val.s16=Sint16(0xF24B);
    sint16value.simple.hasValue=true;

    rc = SCMO_TESTClassNotExist_Inst.setPropertyWithOrigin(
        "Sint16Property",
        CIMTYPE_SINT16,
        &sint16value);

    PEGASUS_TEST_ASSERT(rc==SCMO_OK);

    rc = SCMO_TESTClassNotExist_Inst.getProperty(
        "Sint16Property",
        typeReturn,
        &unionReturn,
        isArrayReturn,
        sizeReturn);

    PEGASUS_TEST_ASSERT(rc==SCMO_OK);
    PEGASUS_TEST_ASSERT(sizeReturn==0);
    PEGASUS_TEST_ASSERT(!isArrayReturn);

    PEGASUS_TEST_ASSERT(
        sint16value.simple.val.s16 == unionReturn->simple.val.s16);
    PEGASUS_TEST_ASSERT(unionReturn->simple.hasValue);

    SCMBUnion sint16ArrayValue[4];
    sint16ArrayValue[0].simple.val.s16 = Sint16(-8);
    sint16ArrayValue[0].simple.hasValue=true;
    sint16ArrayValue[1].simple.val.s16 = Sint16(23872);
    sint16ArrayValue[1].simple.hasValue=true;
    sint16ArrayValue[2].simple.val.s16 = Sint16(334);
    sint16ArrayValue[2].simple.hasValue=true;
    sint16ArrayValue[3].simple.val.s16 = Sint16(0xF00F);
    sint16ArrayValue[3].simple.hasValue=true;

    rc = SCMO_TESTClassNotExist_Inst.setPropertyWithOrigin(
        "Sint16PropertyArray",
        CIMTYPE_SINT16,
        sint16ArrayValue,
        true,4);

    PEGASUS_TEST_ASSERT(rc==SCMO_OK);

    rc = SCMO_TESTClassNotExist_Inst.getProperty(
        "Sint16PropertyArray",
        typeReturn,
        &unionReturn,
        isArrayReturn,
        sizeReturn);

    PEGASUS_TEST_ASSERT(rc==SCMO_OK);
    PEGASUS_TEST_ASSERT(sizeReturn==4);
    PEGASUS_TEST_ASSERT(isArrayReturn);

    PEGASUS_TEST_ASSERT(
        sint16ArrayValue[0].simple.val.s16 == unionReturn[0].simple.val.s16);
    PEGASUS_TEST_ASSERT(unionReturn[0].simple.hasValue);
    PEGASUS_TEST_ASSERT(
        sint16ArrayValue[1].simple.val.s16 == unionReturn[1].simple.val.s16);
    PEGASUS_TEST_ASSERT(unionReturn[1].simple.hasValue);
    PEGASUS_TEST_ASSERT(
        sint16ArrayValue[2].simple.val.s16 == unionReturn[2].simple.val.s16);
    PEGASUS_TEST_ASSERT(unionReturn[2].simple.hasValue);
    PEGASUS_TEST_ASSERT(
        sint16ArrayValue[3].simple.val.s16 == unionReturn[3].simple.val.s16);
    PEGASUS_TEST_ASSERT(unionReturn[3].simple.hasValue);

    /**
     * Test Sint32
     */

    VCOUT << "Test Sint32" << endl;

    SCMBUnion sint32value;
    sint32value.simple.val.s32=0xF0783C;
    sint32value.simple.hasValue=true;

    rc = SCMO_TESTClassNotExist_Inst.setPropertyWithOrigin(
        "Sint32Property",
        CIMTYPE_SINT32,
        &sint32value);

    PEGASUS_TEST_ASSERT(rc==SCMO_OK);

    rc = SCMO_TESTClassNotExist_Inst.getProperty(
        "Sint32Property",
        typeReturn,
        &unionReturn,
        isArrayReturn,
        sizeReturn);

    PEGASUS_TEST_ASSERT(rc==SCMO_OK);
    PEGASUS_TEST_ASSERT(sizeReturn==0);
    PEGASUS_TEST_ASSERT(!isArrayReturn);

    PEGASUS_TEST_ASSERT(
        sint32value.simple.val.s32 == unionReturn->simple.val.s32);
    PEGASUS_TEST_ASSERT(unionReturn->simple.hasValue);

    SCMBUnion sint32ArrayValue2[3];
    sint32ArrayValue2[0].simple.val.s32 = 42;
    sint32ArrayValue2[0].simple.hasValue=true;
    sint32ArrayValue2[1].simple.val.s32 = -28937332;
    sint32ArrayValue2[1].simple.hasValue=true;
    sint32ArrayValue2[2].simple.val.s32 = 19248372;
    sint32ArrayValue2[2].simple.hasValue=true;

    rc = SCMO_TESTClassNotExist_Inst.setPropertyWithOrigin(
        "Sint32PropertyArray",
        CIMTYPE_SINT32,
        sint32ArrayValue2,
        true,3);

    PEGASUS_TEST_ASSERT(rc==SCMO_OK);

    rc = SCMO_TESTClassNotExist_Inst.getProperty(
        "Sint32PropertyArray",
        typeReturn,
        &unionReturn,
        isArrayReturn,
        sizeReturn);

    PEGASUS_TEST_ASSERT(sizeReturn==3);
    PEGASUS_TEST_ASSERT(isArrayReturn);
    PEGASUS_TEST_ASSERT(rc==SCMO_OK);

    PEGASUS_TEST_ASSERT(
        sint32ArrayValue2[0].simple.val.s32 == unionReturn[0].simple.val.s32);
    PEGASUS_TEST_ASSERT(unionReturn[0].simple.hasValue);
    PEGASUS_TEST_ASSERT(
        sint32ArrayValue2[1].simple.val.s32 == unionReturn[1].simple.val.s32);
    PEGASUS_TEST_ASSERT(unionReturn[1].simple.hasValue);
    PEGASUS_TEST_ASSERT(
        sint32ArrayValue2[2].simple.val.s32 == unionReturn[2].simple.val.s32);
    PEGASUS_TEST_ASSERT(unionReturn[2].simple.hasValue);

    /**
     * Test Uint64
     */

    VCOUT << "Test Uint64" << endl;

    SCMBUnion sint64value;
    sint64value.simple.val.s64=(Sint64)-1;
    sint64value.simple.hasValue=true;

    rc = SCMO_TESTClassNotExist_Inst.setPropertyWithOrigin(
        "Sint64Property",
        CIMTYPE_SINT64,
        &sint64value);

    PEGASUS_TEST_ASSERT(rc==SCMO_OK);

    rc = SCMO_TESTClassNotExist_Inst.getProperty(
        "Sint64Property",
        typeReturn,
        &unionReturn,
        isArrayReturn,
        sizeReturn);

    PEGASUS_TEST_ASSERT(rc==SCMO_OK);
    PEGASUS_TEST_ASSERT(sizeReturn==0);
    PEGASUS_TEST_ASSERT(!isArrayReturn);

    PEGASUS_TEST_ASSERT(
        sint64value.simple.val.s64 == unionReturn->simple.val.s64);
    PEGASUS_TEST_ASSERT(unionReturn->simple.hasValue);

    SCMBUnion sint64ArrayValue[4];
    sint64ArrayValue[0].simple.val.s64 = 394;
    sint64ArrayValue[0].simple.hasValue=true;
    sint64ArrayValue[1].simple.val.s64 = -483734324;
    sint64ArrayValue[1].simple.hasValue=true;
    sint64ArrayValue[2].simple.val.s64 = 232349034;
    sint64ArrayValue[2].simple.hasValue=true;
    sint64ArrayValue[3].simple.val.s64 = 0;
    sint64ArrayValue[3].simple.hasValue=true;

    rc = SCMO_TESTClassNotExist_Inst.setPropertyWithOrigin(
        "Sint64PropertyArray",
        CIMTYPE_SINT64,
        sint64ArrayValue,
        true,4);

    PEGASUS_TEST_ASSERT(rc==SCMO_OK);

    rc = SCMO_TESTClassNotExist_Inst.getProperty(
        "Sint64PropertyArray",
        typeReturn,
        &unionReturn,
        isArrayReturn,
        sizeReturn);

    PEGASUS_TEST_ASSERT(sizeReturn==4);
    PEGASUS_TEST_ASSERT(isArrayReturn);
    PEGASUS_TEST_ASSERT(rc==SCMO_OK);

    PEGASUS_TEST_ASSERT(
        sint64ArrayValue[0].simple.val.s64 == unionReturn[0].simple.val.s64);
    PEGASUS_TEST_ASSERT(unionReturn[0].simple.hasValue);
    PEGASUS_TEST_ASSERT(
        sint64ArrayValue[1].simple.val.s64 == unionReturn[1].simple.val.s64);
    PEGASUS_TEST_ASSERT(unionReturn[1].simple.hasValue);
    PEGASUS_TEST_ASSERT(
        sint64ArrayValue[2].simple.val.s64 == unionReturn[2].simple.val.s64);
    PEGASUS_TEST_ASSERT(unionReturn[2].simple.hasValue);
    PEGASUS_TEST_ASSERT(
        sint64ArrayValue[3].simple.val.s64 == unionReturn[3].simple.val.s64);
    PEGASUS_TEST_ASSERT(unionReturn[3].simple.hasValue);

    /**
     * Test Real32
     */

    VCOUT << "Test Real32" << endl;

    SCMBUnion real32value;
    real32value.simple.val.r32=Real32(2.4271e-4);
    real32value.simple.hasValue=true;

    rc = SCMO_TESTClassNotExist_Inst.setPropertyWithOrigin(
        "Real32Property",
        CIMTYPE_REAL32,
        &real32value);

    PEGASUS_TEST_ASSERT(rc==SCMO_OK);

    rc = SCMO_TESTClassNotExist_Inst.getProperty(
        "Real32Property",
        typeReturn,
        &unionReturn,
        isArrayReturn,
        sizeReturn);

    PEGASUS_TEST_ASSERT(rc==SCMO_OK);
    PEGASUS_TEST_ASSERT(sizeReturn==0);
    PEGASUS_TEST_ASSERT(!isArrayReturn);

    PEGASUS_TEST_ASSERT(
        real32value.simple.val.r32 == unionReturn->simple.val.r32);
    PEGASUS_TEST_ASSERT(unionReturn->simple.hasValue);

    SCMBUnion real32ArrayValue[3];
    real32ArrayValue[0].simple.val.r32 = Real32(3.94e30);
    real32ArrayValue[0].simple.hasValue=true;
    real32ArrayValue[1].simple.val.r32 = Real32(-4.83734324e-35);
    real32ArrayValue[1].simple.hasValue=true;
    real32ArrayValue[2].simple.val.r32 = Real32(2.323490e34);
    real32ArrayValue[2].simple.hasValue=true;

    rc = SCMO_TESTClassNotExist_Inst.setPropertyWithOrigin(
        "Real32PropertyArray",
        CIMTYPE_REAL32,
        real32ArrayValue,
        true,3);

    PEGASUS_TEST_ASSERT(rc==SCMO_OK);

    rc = SCMO_TESTClassNotExist_Inst.getProperty(
        "Real32PropertyArray",
        typeReturn,
        &unionReturn,
        isArrayReturn,
        sizeReturn);

    PEGASUS_TEST_ASSERT(rc==SCMO_OK);
    PEGASUS_TEST_ASSERT(sizeReturn==3);
    PEGASUS_TEST_ASSERT(isArrayReturn);

    PEGASUS_TEST_ASSERT(
        real32ArrayValue[0].simple.val.r32 == unionReturn[0].simple.val.r32);
    PEGASUS_TEST_ASSERT(unionReturn[0].simple.hasValue);
    PEGASUS_TEST_ASSERT(
        real32ArrayValue[1].simple.val.r32 == unionReturn[1].simple.val.r32);
    PEGASUS_TEST_ASSERT(unionReturn[1].simple.hasValue);
    PEGASUS_TEST_ASSERT(
        real32ArrayValue[2].simple.val.r32 == unionReturn[2].simple.val.r32);
    PEGASUS_TEST_ASSERT(unionReturn[2].simple.hasValue);

    /**
     * Test Real64
     */

    VCOUT << "Test Real64" << endl;

    SCMBUnion real64value;
    real64value.simple.val.r64=Real64(2.4271e-40);
    real64value.simple.hasValue=true;

    rc = SCMO_TESTClassNotExist_Inst.setPropertyWithOrigin(
        "Real64Property",
        CIMTYPE_REAL64,
        &real64value);

    PEGASUS_TEST_ASSERT(rc==SCMO_OK);

    rc = SCMO_TESTClassNotExist_Inst.getProperty(
        "Real64Property",
        typeReturn,
        &unionReturn,
        isArrayReturn,
        sizeReturn);

    PEGASUS_TEST_ASSERT(rc==SCMO_OK);
    PEGASUS_TEST_ASSERT(sizeReturn==0);
    PEGASUS_TEST_ASSERT(!isArrayReturn);

    PEGASUS_TEST_ASSERT(
        real64value.simple.val.r64 == unionReturn->simple.val.r64);
    PEGASUS_TEST_ASSERT(unionReturn->simple.hasValue);

    SCMBUnion real64ArrayValue[3];
    real64ArrayValue[0].simple.val.r64 = Real64(3.94e38);
    real64ArrayValue[0].simple.hasValue=true;
    real64ArrayValue[1].simple.val.r64 = Real64(-4.83734644e-35);
    real64ArrayValue[1].simple.hasValue=true;
    real64ArrayValue[2].simple.val.r64 = Real64(2.643490e34);
    real64ArrayValue[2].simple.hasValue=true;

    rc = SCMO_TESTClassNotExist_Inst.setPropertyWithOrigin(
        "Real64PropertyArray",
        CIMTYPE_REAL64,
        real64ArrayValue,
        true,3);

    PEGASUS_TEST_ASSERT(rc==SCMO_OK);

    rc = SCMO_TESTClassNotExist_Inst.getProperty(
        "Real64PropertyArray",
        typeReturn,
        &unionReturn,
        isArrayReturn,
        sizeReturn);

    PEGASUS_TEST_ASSERT(rc==SCMO_OK);
    PEGASUS_TEST_ASSERT(sizeReturn==3);
    PEGASUS_TEST_ASSERT(isArrayReturn);

    PEGASUS_TEST_ASSERT(
        real64ArrayValue[0].simple.val.r64 == unionReturn[0].simple.val.r64);
    PEGASUS_TEST_ASSERT(unionReturn[0].simple.hasValue);
    PEGASUS_TEST_ASSERT(
        real64ArrayValue[1].simple.val.r64 == unionReturn[1].simple.val.r64);
    PEGASUS_TEST_ASSERT(unionReturn[1].simple.hasValue);
    PEGASUS_TEST_ASSERT(
        real64ArrayValue[2].simple.val.r64 == unionReturn[2].simple.val.r64);
    PEGASUS_TEST_ASSERT(unionReturn[2].simple.hasValue);

    /**
     * Test Boolean
     */

    VCOUT << "Test Boolean Property" << endl;

    boolValue.simple.val.bin=true;

    rc = SCMO_TESTClassNotExist_Inst.setPropertyWithOrigin(
        "BooleanProperty",
        CIMTYPE_BOOLEAN,
        &boolValue);

    PEGASUS_TEST_ASSERT(rc==SCMO_OK);

    rc = SCMO_TESTClassNotExist_Inst.getProperty(
        "BooleanProperty",
        typeReturn,
        &unionReturn,
        isArrayReturn,
        sizeReturn);

    PEGASUS_TEST_ASSERT(rc==SCMO_OK);
    PEGASUS_TEST_ASSERT(sizeReturn==0);
    PEGASUS_TEST_ASSERT(!isArrayReturn);

    PEGASUS_TEST_ASSERT(unionReturn->simple.val.bin);

    SCMBUnion boolArrayValue[3];
    boolArrayValue[0].simple.val.bin = true;
    boolArrayValue[0].simple.hasValue= true;
    boolArrayValue[1].simple.val.bin = false;
    boolArrayValue[1].simple.hasValue= true;
    boolArrayValue[2].simple.val.bin = true;
    boolArrayValue[2].simple.hasValue= true;

    VCOUT << "Test Boolean Property Array" << endl;

    rc = SCMO_TESTClassNotExist_Inst.setPropertyWithOrigin(
        "BooleanPropertyArray",
        CIMTYPE_BOOLEAN,
        boolArrayValue,
        true,3);

    PEGASUS_TEST_ASSERT(rc==SCMO_OK);

    rc = SCMO_TESTClassNotExist_Inst.getProperty(
        "BooleanPropertyArray",
        typeReturn,
        &unionReturn,
        isArrayReturn,
        sizeReturn);

    PEGASUS_TEST_ASSERT(rc==SCMO_OK);
    PEGASUS_TEST_ASSERT(sizeReturn==3);
    PEGASUS_TEST_ASSERT(isArrayReturn);

    PEGASUS_TEST_ASSERT(unionReturn[0].simple.val.bin);
    PEGASUS_TEST_ASSERT(unionReturn[0].simple.hasValue);
    PEGASUS_TEST_ASSERT(!unionReturn[1].simple.val.bin);
    PEGASUS_TEST_ASSERT(unionReturn[1].simple.hasValue);
    PEGASUS_TEST_ASSERT(unionReturn[2].simple.val.bin);
    PEGASUS_TEST_ASSERT(unionReturn[2].simple.hasValue);

    /**
     * Test DateTime
     */

    VCOUT << "Test DateTime" << endl;

    SCMBUnion myDateTimeValue;
    myDateTimeValue.dateTimeValue.usec = PEGASUS_UINT64_LITERAL(17236362);
    myDateTimeValue.dateTimeValue.utcOffset = 0;
    myDateTimeValue.dateTimeValue.sign = ':';
    myDateTimeValue.dateTimeValue.numWildcards = 0;

    rc = SCMO_TESTClassNotExist_Inst.setPropertyWithOrigin(
        "DateTimeProperty",
        CIMTYPE_DATETIME,
        &myDateTimeValue);

    PEGASUS_TEST_ASSERT(rc==SCMO_OK);

    rc = SCMO_TESTClassNotExist_Inst.getProperty(
        "DateTimeProperty",
        typeReturn,
        &unionReturn,
        isArrayReturn,
        sizeReturn);

    PEGASUS_TEST_ASSERT(rc==SCMO_OK);
    PEGASUS_TEST_ASSERT(sizeReturn==0);
    PEGASUS_TEST_ASSERT(!isArrayReturn);

    PEGASUS_TEST_ASSERT(
        memcmp(
            &(myDateTimeValue.dateTimeValue),
            &(unionReturn->dateTimeValue),
            sizeof(CIMDateTimeRep))== 0);

    SCMBUnion dateTimeArrayValue[3];

    dateTimeArrayValue[0].dateTimeValue.usec = Uint64(988243387);
    dateTimeArrayValue[0].dateTimeValue.utcOffset = 0;
    dateTimeArrayValue[0].dateTimeValue.sign = ':';
    dateTimeArrayValue[0].dateTimeValue.numWildcards = 0;

    dateTimeArrayValue[1].dateTimeValue.usec = Uint64(827383727);
    dateTimeArrayValue[1].dateTimeValue.utcOffset = 0;
    dateTimeArrayValue[1].dateTimeValue.sign = ':';
    dateTimeArrayValue[1].dateTimeValue.numWildcards = 0;

    dateTimeArrayValue[2].dateTimeValue.usec = Uint64(932933892);
    dateTimeArrayValue[2].dateTimeValue.utcOffset = 0;
    dateTimeArrayValue[2].dateTimeValue.sign = ':';
    dateTimeArrayValue[2].dateTimeValue.numWildcards = 0;

    rc = SCMO_TESTClassNotExist_Inst.setPropertyWithOrigin(
        "DateTimePropertyArray",
        CIMTYPE_DATETIME,
        dateTimeArrayValue,
        true,3);

    PEGASUS_TEST_ASSERT(rc==SCMO_OK);

    rc = SCMO_TESTClassNotExist_Inst.getProperty(
        "DateTimePropertyArray",
        typeReturn,
        &unionReturn,
        isArrayReturn,
        sizeReturn);

    PEGASUS_TEST_ASSERT(rc==SCMO_OK);
    PEGASUS_TEST_ASSERT(sizeReturn==3);
    PEGASUS_TEST_ASSERT(isArrayReturn);


    PEGASUS_TEST_ASSERT(
        memcmp(&(dateTimeArrayValue[0].dateTimeValue),
               &(unionReturn[0].dateTimeValue),
               sizeof(CIMDateTimeRep)
               )== 0);
    PEGASUS_TEST_ASSERT(
        memcmp(
            &(dateTimeArrayValue[1].dateTimeValue),
            &(unionReturn[1].dateTimeValue),
            sizeof(CIMDateTimeRep))== 0);
    PEGASUS_TEST_ASSERT(
        memcmp(
            &(dateTimeArrayValue[2].dateTimeValue),
            &(unionReturn[2].dateTimeValue),
            sizeof(CIMDateTimeRep))== 0);

    /**
     * Test string
     */

    VCOUT << "Test String" << endl;

    char ThisIsASingleString[] = "This is a single String!";
    SCMBUnion stringValue;
    stringValue.extString.pchar = &(ThisIsASingleString[0]);
    stringValue.extString.length = strlen(ThisIsASingleString);

    rc = SCMO_TESTClassNotExist_Inst.setPropertyWithOrigin(
        "StringProperty",
        CIMTYPE_STRING,
        &stringValue);

    PEGASUS_TEST_ASSERT(rc==SCMO_OK);

    rc = SCMO_TESTClassNotExist_Inst.getProperty(
        "StringProperty",
        typeReturn,
        &unionReturn,
        isArrayReturn,
        sizeReturn);

    PEGASUS_TEST_ASSERT(rc==SCMO_OK);
    PEGASUS_TEST_ASSERT(sizeReturn==0);
    PEGASUS_TEST_ASSERT(!isArrayReturn);

    PEGASUS_TEST_ASSERT(
        stringValue.extString.length==unionReturn->extString.length);
    PEGASUS_TEST_ASSERT(
        strcmp(
            stringValue.extString.pchar,
            unionReturn->extString.pchar) == 0);

    free((void*)unionReturn);

    char arraySting0[] = "The Array String Number one.";
    char arraySting1[] = "The Array String Number two.";
    char arraySting2[] = "The Array String Number three.";
    SCMBUnion stringArrayValue[3];
    stringArrayValue[0].extString.pchar= &(arraySting0[0]);
    stringArrayValue[0].extString.length= strlen(arraySting0);

    stringArrayValue[1].extString.pchar=&(arraySting1[0]);
    stringArrayValue[1].extString.length=strlen(arraySting1);

    stringArrayValue[2].extString.pchar=&(arraySting2[0]);
    stringArrayValue[2].extString.length=strlen(arraySting2);

    rc = SCMO_TESTClassNotExist_Inst.setPropertyWithOrigin(
        "StringPropertyArray",
        CIMTYPE_STRING,
        stringArrayValue,
        true,3);

    PEGASUS_TEST_ASSERT(rc==SCMO_OK);

    rc = SCMO_TESTClassNotExist_Inst.getProperty(
        "StringPropertyArray",
        typeReturn,
        &unionReturn,
        isArrayReturn,
        sizeReturn);

    PEGASUS_TEST_ASSERT(rc==SCMO_OK);
    PEGASUS_TEST_ASSERT(sizeReturn==3);
    PEGASUS_TEST_ASSERT(isArrayReturn);

    PEGASUS_TEST_ASSERT(
        stringArrayValue[0].extString.length==unionReturn[0].extString.length);
    PEGASUS_TEST_ASSERT(
        strcmp(
            stringArrayValue[0].extString.pchar,
            unionReturn[0].extString.pchar) == 0);
    PEGASUS_TEST_ASSERT(
        stringArrayValue[1].extString.length==unionReturn[1].extString.length);
    PEGASUS_TEST_ASSERT(
        strcmp(
            stringArrayValue[1].extString.pchar,
            unionReturn[1].extString.pchar) == 0);
    PEGASUS_TEST_ASSERT(
        stringArrayValue[2].extString.length==unionReturn[2].extString.length);
    PEGASUS_TEST_ASSERT(
        strcmp(
            stringArrayValue[2].extString.pchar,
            unionReturn[2].extString.pchar) == 0);

    // test getPropertyAt and confirm at least one of the properties was found
    instanceFound = false;
    for (Uint32 i = 0 ; i < SCMO_TESTClassNotExist_Inst.getPropertyCount(); i++)
    {
        const SCMBUnion* value = 0;
        Boolean isArray = 0;
        Uint32 size = 0;
        CIMType type = (CIMType)0;
        const char* pName=0;
        SCMO_TESTClassNotExist_Inst.getPropertyAt(i,&pName,
                                          type,
                                          &value,
                                          isArray,
                                          size);
        if (strcmp(pName, "Uint32PropertyArray") == 0 )
        {
            PEGASUS_TEST_ASSERT(type == CIMTYPE_UINT32);
        }
        instanceFound = true;
    }
    PEGASUS_TEST_ASSERT(instanceFound);

    VSCMODUMP(sd.dumpSCMOInstance(SCMO_TESTClassNotExist_Inst, true, true));
    // do not forget !!!
    free((void*)unionReturn);

    // KS_TODO this should really be in the SCMOStreamer test
    Buffer buf;
    Array<Uint32> emptyNodes;
    SCMOXmlWriter::appendInstanceElement(
        buf,
        SCMO_TESTClassNotExist_Inst,
        false,
        emptyNodes);
    if (verbose)
    {
        cout << "Display of SCMOXmlWriter::appendInstanceElement" << endl;
        cout << endl << buf.getData() << endl;
    }


    VCOUT << "SCMOUserDefinedInstancePropertyTest Done" << endl;
}

//
//  The following are part of the test for CIM <-->SCMO conversion testing
//  with embedded instances that have no class.
//  Easier to create the instance and class in memory than use the XML and
//  Loader functions. Also more flexible for the future since the XML is a
//  mess to change especially since the original authors did not leave the
//  mof behind to help conversion.
//
// Create a single class that includes a property that is an embedded
// object and return the CIMClass created.
CIMClass createClassWithEmbedObj()
{
    // Create a class that will be embedded. This needed to be able
    // to define the embeddedObjectProperty with the C++ class
    // declaration method.
    CIMClass embClass(CIMName ("EmbeddedObjClass"), CIMName ());

    //Create the class that will embed
    CIMClass classwithEmbObj(CIMName ("ClassWithEmbObj"),
                             CIMName ("SuperClass"));

    classwithEmbObj
    .addQualifier(CIMQualifier(CIMName ("q1"), Uint32(55)))
    .addQualifier(CIMQualifier(CIMName ("q2"), String("Hello")))
    .addProperty(CIMProperty(CIMName ("message"), String("Hello")))
    .addProperty(CIMProperty(CIMName ("count"), Uint32(77), 0, CIMName(),
            CIMName("YourClass"), true))
    .addProperty(CIMProperty(CIMName ("embeddedObj"), CIMObject(embClass)))

    .addMethod(CIMMethod(CIMName ("isActive"), CIMTYPE_BOOLEAN)
        .addParameter(CIMParameter(CIMName ("hostname"), CIMTYPE_STRING))
        .addParameter(CIMParameter(CIMName ("port"), CIMTYPE_UINT32)));

    return classwithEmbObj;
}

/*
    This is a SCMO Cache callback function that returns a single
    SCMO class. This cache returns a valid class for the class
    ClassWithEmbObj but empty for any other class.
*/
SCMOClass _scmoClassCache_GetClassWithEmbedObj(
    const CIMNamespaceName& nameSpace,
    const CIMName& className)
{
    // Get the test class

    CIMClass embCimClass = createClassWithEmbedObj();

    // Convert to SCMO and return the created class
    if (className.getString() == "ClassWithEmbObj")
    {
        return SCMOClass(
            embCimClass,
            (const char*)nameSpace.getString().getCString());
    }
    // Otherwise return an empty class.
    else
    {
        return SCMOClass();
    }
}

/*
    Test the conversion from a SCMO instance to a CIMInstance and back
    to a SCMO instance for a SCMO instance with an embeddeed object
    that does not have a corresponding class in the cache.
*/
void SCMOUserDefinedInstanceConverterTest()
{
    VCOUT << "Start " << "SCMOUserDefinedInstanceConverterTest()" << endl;

    // This gets the class from the cache
    SCMOClassCache* _theCache = SCMOClassCache::getInstance();

    // To be certain that the namespace logic in SCMO works for
    // these user-defined properties, this test is run in an alternate
    // namespace since in some conditions SCMO substitutes cimv2 as
    // the alternate namespace.

    SCMOClass SCMO_TestClassWithEmbObj = _theCache->getSCMOClass(
            "NameSpaceNotExist", strlen("NameSpaceNotExist"),
            "ClassWithEmbObj", strlen("ClassWithEmbObj"));

    PEGASUS_TEST_ASSERT(!SCMO_TestClassWithEmbObj.isEmpty());

#ifdef PEGASUS_DEBUG
    SCMODump sd;
#endif
    VSCMODUMP(sd.dumpSCMOClass(SCMO_TestClassWithEmbObj));

    // Try to get the EmbeddedObjecClass from our cache.
    SCMOClass SCMO_TestClassEmbObjClass = _theCache->getSCMOClass(
        "NameSpaceNotExist", strlen("NameSpaceNotExist"),
        "EmbeddedObjClass", strlen("EmbeddedObjClass"));

    PEGASUS_TEST_ASSERT(SCMO_TestClassEmbObjClass.isEmpty());

    // Create an instance of the embedded class. This one has no
    // Class in the cache. Add a path to be sure we are passing the
    // path info through
    CIMInstance instanceEmbed(CIMName("EmbeddedObjClass"));
    instanceEmbed.addProperty(CIMProperty(CIMName("EmbedUint64Prop"),
        Uint64(9876)));
    instanceEmbed.addProperty(CIMProperty(CIMName("EmbeddedUint32Prop"),
        Uint32(6586)));
//  instanceEmbed.addProperty(CIMProperty(CIMName("EmbedStrProp"),
//      String("I am a property in an embedded instance with no class")));

    CIMObjectPath embedPath =
        CIMObjectPath("/NameSpaceNotExist:EmbeddedObjClass");
    instanceEmbed.setPath(embedPath);


    // Create the instance of class to be converted which
    // includes the embedded instance. Note that both have a valid path.
    CIMInstance originalCimInstance(CIMName("ClassWithEmbObj"));
    originalCimInstance.addProperty(CIMProperty(CIMName("message"),
                                                String("Goodbye")));
    originalCimInstance.addProperty(CIMProperty(
        CIMName ("count"), Uint32(77), 0, CIMName(),
        CIMName("YourClass"), true));
    originalCimInstance.addProperty(CIMProperty(CIMName ("embeddedObj"),
         CIMObject(instanceEmbed)));

    CIMObjectPath path =
        CIMObjectPath("/NameSpaceNotExist:ClassWithEmbObj");
    originalCimInstance.setPath(path);

    if (verbose)
    {
        PrintInstance(cout, originalCimInstance);
        cout << "Original CIM Instance " << endl;
        XmlWriter::printInstanceElement(originalCimInstance,cout);
    }

    SCMOInstance SCMO_TESTClassWithEmbObj(SCMO_TestClassWithEmbObj);

    VCOUT << "Create SCMOInstance from CIMInstance" << endl;

    SCMOInstance SCMO_CSInstance(SCMO_TestClassWithEmbObj, originalCimInstance);

    VCOUT << "Created SCMOInstance from CIMInstance" << endl;

    CIMInstance newCimInstance;

    VCOUT << "Recreate CIMInstance from SCMOInstance" << endl;

    SCMO_CSInstance.getCIMInstance(newCimInstance);

    // If not identical, dump everything since this is an error
    // Note that the identical test depends not only on having the
    // same properties but the same order for properties in the
    // two instances.
    if (!(newCimInstance.identical(originalCimInstance)))
    {
        cout << "Dumping instances that are NOT identical" << endl;
        cout << "Instance created " << endl;
        XmlWriter::printInstanceElement(newCimInstance, cout);
        cout << "Original CIM Instance " << endl;
        XmlWriter::printInstanceElement(originalCimInstance,cout);
        cout << "SCMO Instance created from original instance" << endl;
#ifdef PEGASUS_DEBUG
        sd.dumpSCMOInstance(SCMO_CSInstance, true, true);
#endif
    }

    PEGASUS_TEST_ASSERT(newCimInstance.identical(originalCimInstance));

    VCOUT << "SCMOUserDefinedInstanceConverterTest Done" << endl;
}

/****************************************************
     Main
*****************************************************/
int main (int, char *argv[])
{
    CIMClass CIM_TESTClass2;

    verbose = getenv("PEGASUS_TEST_VERBOSE") ? true : false;

    // check if cache is loading the class only once
    loadClassOnce = false;

    try
    {
        structureSizesTest();

        CIMClassToSCMOClass();

        // init the cache to point back _scmoClassCache_GetClass
        // This puts one class in the cache
        SCMOClassCache* _thecache = SCMOClassCache::getInstance();
        _thecache->setCallBack(_scmoClassCache_GetClass);

        SCMOClassQualifierTest();

        SCMOInstancePropertyTest();

        SCMOInstanceKeyBindingsTest();

        SCMOInstanceConverterTest();

        SCMOUserDefinedInstancePropertyTest();

         //destroy the cache and create a new one that contains a
         // class with an embedded object to execute the
        // next test.
        _thecache->destroy();

        _thecache = SCMOClassCache::getInstance();

        _thecache->setCallBack(_scmoClassCache_GetClassWithEmbedObj);

        SCMOUserDefinedInstanceConverterTest();

         //destroy the cache.
        _thecache->destroy();
    }
    catch (CIMException& e)
    {
        cout << endl << "CIMException: " ;
        cout << e.getMessage() << endl << endl ;
        exit(-1);
    }

    catch (Exception& e)
    {
        cout << endl << "Exception: " ;
        cout << e.getMessage() << endl << endl ;
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
