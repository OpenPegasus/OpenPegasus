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
//
//%/////////////////////////////////////////////////////////////////////////////

#include <Pegasus/Common/Config.h>
#include <Pegasus/Common/Constants.h>
#include <Pegasus/Common/PegasusVersion.h>
#include <Pegasus/Common/PegasusAssert.h>
#include <Pegasus/Common/TLS.h>
#include <Pegasus/Common/CIMName.h>
#include <Pegasus/Common/FileSystem.h>
#include <Pegasus/Common/System.h>
#include <Pegasus/Common/Exception.h>
#include <Pegasus/Common/AcceptLanguageList.h>
#include <Pegasus/Common/ContentLanguageList.h>
#include <Pegasus/Common/LanguageParser.h>
#include <Pegasus/Common/HostLocator.h>

#include <Pegasus/Consumer/CIMIndicationConsumer.h>

#include <Pegasus/Listener/CIMListener.h>

#include <Pegasus/Client/CIMClient.h>

#include <Pegasus/General/OptionManager.h>
#include <Pegasus/General/Stopwatch.h>

PEGASUS_USING_PEGASUS;
PEGASUS_USING_STD;

#define TESTMACRO(EXCTYPE) throw new EXCTYPE;

//#define MYASSERT ASSERTTEMP
#define MYASSERT PEGASUS_TEST_ASSERT

static const char* programVersion =  "1.0";
Boolean skipICU = false;

// Array of UTF-16 chars to be sent and received to the server.
// Note: the dbc0/dc01 pair are surrogates
Char16 utf16Chars[] =
        {
        0x6A19, 0x6E96, 0x842C, 0x570B, 0x78BC,
        0x042E, 0x043D, 0x0438, 0x043A, 0x043E, 0x0434,
        0x110B, 0x1172, 0x1102, 0x1165, 0x110F, 0x1169, 0x11AE,
        0x10E3, 0x10DC, 0x10D8, 0x10D9, 0x10DD, 0x10D3, 0x10D8,
    0xdbc0, 0xdc01,
        0x05D9, 0x05D5, 0x05E0, 0x05D9, 0x05E7, 0x05D0, 0x05B8, 0x05D3,
        0x064A, 0x0648, 0x0646, 0x0650, 0x0643, 0x0648, 0x062F,
        0x092F, 0x0942, 0x0928, 0x093F, 0x0915, 0x094B, 0x0921,
        0x016A, 0x006E, 0x012D, 0x0063, 0x014D, 0x0064, 0x0065, 0x033D,
        0x00E0, 0x248B, 0x0061, 0x2173, 0x0062, 0x1EA6, 0xFF21, 0x00AA, 0x0325,
        0x2173, 0x249C, 0x0063,
        0x02C8, 0x006A, 0x0075, 0x006E, 0x026A, 0x02CC, 0x006B, 0x006F, 0x02D0,
        0x0064,
        0x30E6, 0x30CB, 0x30B3, 0x30FC, 0x30C9,
        0xFF95, 0xFF86, 0xFF7A, 0xFF70, 0xFF84, 0xFF9E,
        0xC720, 0xB2C8, 0xCF5B, 0x7D71, 0x4E00, 0x78BC,
    0xdbc0, 0xdc01,
        0x00};

// A shorter array of UTF-16 chars to be used in the repository tests
// for the names of the repository files.
// Longer strings are more likely to hit the filesystem maximum name length
// if the repository is set up to escape these characters in its file names.
// Mix in a utf-16 surrogate pair at the start (dbc0/dc01).
Char16 utfRepChars[] =
  {
    0xdbc0, 0xdc01,
    'a',
    0xFF95,
    'z',
    0xFF86,
    0x00
  };

////////////////////////////////////////////////////////////////////////////////
//
// Indication Related Stuff
//
////////////////////////////////////////////////////////////////////////////////

Semaphore indicationReceived(0);
OperationContext indicationContext;
CIMInstance indication;

class MyIndicationConsumer : public CIMIndicationConsumer
{
public:
    MyIndicationConsumer(const String& name_);
    ~MyIndicationConsumer();

    void consumeIndication(const OperationContext& context,
        const String & url,
        const CIMInstance& indicationInstance);

private:
    String name;

};

MyIndicationConsumer::MyIndicationConsumer(const String& name_)
{
    this->name = name_;
}

MyIndicationConsumer::~MyIndicationConsumer()
{
}

void MyIndicationConsumer::consumeIndication(
                         const OperationContext & context,
                         const String & url,
                         const CIMInstance& indicationInstance)
{
  // Save the objects for the main code to verify
  indication = indicationInstance;
  indicationContext = context;

  // Signal the main code that the indication was received
  indicationReceived.signal();
}

///////////////////////////////////////////////////////////////////////////


/** ErrorExit - Print out the error message as an
    and get out.
    @param - Text for error message
    @return - None, Terminates the program
    @execption - This function terminates the program
    ATTN: Should write to stderr
*/
void ErrorExit(const String& message)
{

    cout << message << endl;
    exit(1);
}


/* Status display of the various steps.  Shows message of function and
time to execute.  Grow this to a class so we have start and stop and time
display with success/failure for each function.
*/
static void testStart(const String& message)
{
    cout << "++++ " << message << " ++++" << endl;

}

static void testEnd(const double elapsedTime)
{
    cout << "In " << elapsedTime << " Seconds\n\n";
}


/*
   Tests the UTF support in the repository
*/
static void TestUTFRepository( CIMClient& client,
                               Boolean utfRepNames,
                               Boolean activeTest,
                               Boolean verboseTest )
{
    if (!activeTest)
    {
        cout << "Active tests are disabled."
                    " Nothing to do for this set of tests."
             << endl;
        return;
    }

    const CIMNamespaceName NAMESPACE = CIMNamespaceName ("test/TestProvider");
    const CIMNamespaceName ROOT_NAMESPACE = CIMNamespaceName ("root");

    try
    {
        // The repository does not support these headers and will throw errors
        // if they are used.
        client.setRequestAcceptLanguages(AcceptLanguageList());
        client.setRequestContentLanguages(ContentLanguageList());

        String utf16String(utf16Chars);
        String utf16FileName(utfRepChars);

        //
        //  TEST 1 - Create qualifier
        //  Create a qualifier with UTF-16 in the qualifier name,
            //  and value
        //  This will test UTF support in the repository.
        //

        cout << endl
             << "REPOSITORY TEST 1: Create Qualifier containing UTF-16 chars"
             << endl;

        // Decide whether to use UTF-16 in the name of the qualifier
        CIMName qualDeclName("UTFTestQualifier");
        if (!utfRepNames)
        {
             cout<< "Note: utfrep option was not set."
                " Not using UTF-16 in the qualifier name" << endl;
        }
        else
        {
            if (verboseTest)
                cout << "Using UTF-16 in the qualifier name" << endl;

            // This will create a repository file with UTF-8 chars in the name
            qualDeclName = utf16FileName;
        }

        //  First, delete the qualifier if it was there from before
        if (verboseTest)
            cout << "Deleting the qualifier in case it was there from before"
                << endl;

        try
        {
            client.deleteQualifier(NAMESPACE, qualDeclName);
        }
        catch (CIMException & e)
        {
            if (e.getCode() != CIM_ERR_NOT_FOUND)
                throw;
        }

        if (verboseTest)
            cout << "Creating the qualifier" << endl;

        CIMQualifierDecl qualDecl(qualDeclName,
                     utf16String,
                     CIMScope::PROPERTY + CIMScope::CLASS,
                     CIMFlavor::OVERRIDABLE);

        client.setQualifier(NAMESPACE, qualDecl);

        if (verboseTest)
            cout << "Getting the qualifier that was just created" << endl;

        CIMQualifierDecl rtnQualDecl = client.getQualifier(
            NAMESPACE,
            qualDeclName);

        if (verboseTest)
            cout << "Checking that UTF-16 was preserved" << endl;

        MYASSERT(qualDecl.identical(rtnQualDecl));

        if (verboseTest)
            cout << "Enumerating qualifiers, and looking for the UTF-16 one"
                << endl;

        Array<CIMQualifierDecl> rtnQualDeclArray = client.enumerateQualifiers(
                  NAMESPACE );

        Boolean foundQualDecl = false;
        for (Uint32 i = 0; i < rtnQualDeclArray.size(); i++)
        {
            if (qualDecl.identical(rtnQualDeclArray[i]))
                foundQualDecl = true;
        }

        MYASSERT(foundQualDecl == true);

        //
        //  TEST 2 - Create class
        //  Create a class with UTF-16 in the class name,
        //  property names, and qualifiers.
        //
        //  We will use the UTF16 qualifier created above
        //  in this class.
        //
        //  This will test UTF support in the repository.
        //

        cout << endl
             << "REPOSITORY TEST 2: Create Class containing UTF-16 chars"
             << endl;

        // Decide whether to use UTF-16 in the name of the class
        CIMName className("UTFTestClass");
        if (!utfRepNames)
        {
        cout << "Note: utfrep option was not set."
                " Not using UTF-16 in the class name"
             << endl;
        }
        else
        {
            if (verboseTest)
                cout << "Using UTF-16 in the class name" << endl;

            // This will create a repository file with UTF-8 chars in the name
            className = utf16FileName;
        }

        //  First, delete the class if it was there from before
        if (verboseTest)
            cout << "Deleting the class in case it was there from before"
                << endl;

        try
        {
            client.deleteClass(NAMESPACE, className);
        }
        catch (CIMException & e)
        {
            if (e.getCode() != CIM_ERR_NOT_FOUND)
                throw;
        }

        if (verboseTest)
            cout << "Creating the class" << endl;

        // How did this get so ugly?
        CIMClass cimClass(className, CIMName ("CIM_ManagedElement"));
        CIMName name1("fred");
        CIMName name2(utf16String);
        CIMValue fred("fred");
        CIMName key("key");

        CIMProperty prop1(name1, fred);
        CIMQualifier qual1(key, true);
        CIMProperty prop2 = prop1.addQualifier(qual1);
        cimClass.addProperty(prop2);

        CIMProperty prop3(name2, fred);             // UTF16 prop name
        CIMQualifier qual2(qualDecl.getName(),      // UTF16 qualifier
                           qualDecl.getValue(),
                           qualDecl.getFlavor());
        CIMProperty prop4 = prop3.addQualifier(qual2);
        cimClass.addProperty(prop4);

        client.createClass(NAMESPACE, cimClass);

        if (verboseTest)
            cout << "Getting the class that was just created" << endl;

        CIMClass rtnClass = client.getClass(
            NAMESPACE,
            className,
            true,
            true);

        if (verboseTest)
            cout << "Checking that the UTF-16 was preserved in the property"
                        " name"
                << endl;

        Uint32 idx = rtnClass.findProperty(name2);  // name2 is UTF16
        MYASSERT(idx != PEG_NOT_FOUND);

        if (verboseTest)
            cout << "Checking that the UTF-16 was preserved in the qualifier"
                << endl;

        CIMProperty rtnProp = rtnClass.getProperty(idx);
        idx = rtnProp.findQualifier(qualDecl.getName());
        MYASSERT(idx != PEG_NOT_FOUND);

        CIMQualifier rtnQual = rtnProp.getQualifier(idx);
        MYASSERT(rtnQual.getValue() == utf16String);

        if (verboseTest)
            cout << "Modifying the class" << endl;

        CIMName name3("anotherprop");
        CIMProperty prop5(name3, fred);
        CIMQualifier qual3(qualDecl.getName(),      // UTF16 qualifier
                           qualDecl.getValue(),
                           qualDecl.getFlavor());
        CIMProperty prop6 = prop5.addQualifier(qual3);
        cimClass.addProperty(prop6);

        client.modifyClass(NAMESPACE,cimClass);

        if (verboseTest)
            cout << "Getting the class that was just modified" << endl;

        rtnClass = client.getClass(
                   NAMESPACE,
                   className,
                   true,
                   true);

        if (verboseTest)
            cout << "Checking that the UTF-16 was preserved in the property"
                    " name"
                << endl;

        idx = rtnClass.findProperty(name2);  // name2 is UTF16
            MYASSERT(idx != PEG_NOT_FOUND);
        idx = rtnClass.findProperty(name3);
            MYASSERT(idx != PEG_NOT_FOUND);

        if (verboseTest)
            cout << "Checking that the UTF-16 was preserved in the qualifier"
                << endl;

        rtnProp = rtnClass.getProperty(idx);
        idx = rtnProp.findQualifier(qualDecl.getName());
        MYASSERT(idx != PEG_NOT_FOUND);

        rtnQual = rtnProp.getQualifier(idx);
        MYASSERT(rtnQual.getValue() == utf16String);

        //
        // Clean up the repository
        //
        if (verboseTest)
        cout << endl << "Cleaning up the repository" << endl;

        if (verboseTest)
        cout << "Deleting the class" << endl;

        client.deleteClass(NAMESPACE, className);

        if (verboseTest)
        cout << "Delete the qualifier" << endl;

        client.deleteQualifier(NAMESPACE, qualDeclName);

        //
        //  TEST 3 - Create namespace
            //
        //  Create a namespace with UTF-16 in the namespace name, and
        //  add class to the namespace.
        //

        cout << endl
            << "REPOSITORY TEST 3: Create Namespace with UTF-16 chars in the"
                    " name"
            << endl;

        if (!utfRepNames)
        {
            // Since namespaces are only directory names, no point in doing this
            // if the utfrep option wasn't set.
            cout << "Note: Skipping this test because the utfrep option was"
                        " not used"
                 << endl;
        }
        else
        {
            if (verboseTest)
                cout << endl
                    << "Deleting the old namespace in case it was there from"
                            " before."
                    << endl;

            CIMName __nameSpace = "__NameSpace";

            Array<CIMObjectPath> enumNamespaces = client.enumerateInstanceNames(
                          CIMNamespaceName("root"),
                          __nameSpace);

            for (Uint32 i = 0; i < enumNamespaces.size(); i++)
            {
                Array<CIMKeyBinding> kb = enumNamespaces[i].getKeyBindings();
                if (kb[0].getValue() == utf16FileName)
                {
                    client.deleteInstance(ROOT_NAMESPACE, enumNamespaces[i]);
                }
            }

            if (verboseTest)
                cout << endl << "Creating the namespace" << endl;

            CIMInstance nsInstance(__nameSpace);
            CIMName nsPropName("Name");
            CIMProperty nsProp(nsPropName,
                     CIMValue(utf16FileName));
            nsInstance.addProperty(nsProp);

            CIMObjectPath nsInstanceName = client.createInstance(ROOT_NAMESPACE,
                                       nsInstance);

            if (verboseTest)
                cout << endl << "Getting the namespace just created" << endl;

            CIMInstance rtnNSInst =
              client.getInstance(ROOT_NAMESPACE, nsInstanceName);

            idx = rtnNSInst.findProperty(nsPropName);
            MYASSERT(idx != PEG_NOT_FOUND);

            CIMProperty rtnNSProp = rtnNSInst.getProperty(idx);
            String rtnNSName;
            rtnNSProp.getValue().get(rtnNSName);
            MYASSERT(rtnNSName == utf16FileName);

            if (verboseTest)
                cout << endl << "Creating classes in that namespace" << endl;

            String root("root/");
            root.append(utf16FileName);
            CIMNamespaceName ns(root);

            // Do a little inheriting with UTF-16 class names
            CIMName baseClassName(utf16FileName);
            CIMClass baseClass(baseClassName);
            CIMName basePropName("baseProp1");
            CIMValue basePropValue("fred");
            CIMProperty baseProp(basePropName, basePropValue);
            baseClass.addProperty(baseProp);

            CIMName inhClassName(&utf16Chars[1]);
            CIMClass inhClass(inhClassName, baseClassName);
            CIMName inhPropName("inhProp1");
            CIMValue inhPropValue("fred");
            CIMProperty inhProp(inhPropName, inhPropValue);
            inhClass.addProperty(inhProp);

            client.createClass(ns, baseClass);
            client.createClass(ns, inhClass);

            if (verboseTest)
                cout << endl << "Deleting classes in that namespace" << endl;

            client.deleteClass(ns, inhClassName);
            client.deleteClass(ns, baseClassName);

            if (verboseTest)
                cout << endl << "Deleting the namespace" << endl;

            client.deleteInstance(ROOT_NAMESPACE, nsInstanceName);
        }  // endif utfrep
    }
    catch(Exception& e)
    {
        PEGASUS_STD(cerr) << "Error in TestLocalizedRepository: "
                          << e.getMessage() << PEGASUS_STD(endl);
        throw;
    }
}

/*
   Tests the globalization support of the LocalizedProvider
   for the method operations
*/
static void TestLocalizedMethods( CIMClient& client,
                                  Boolean activeTest,
                                  Boolean verboseTest )
{
    const CIMNamespaceName NAMESPACE = CIMNamespaceName ("test/TestProvider");
    const CIMName CLASSNAME = CIMName ("Test_LocalizedProviderClass");
    const String INPARAM1 = "inStr";
    const String INPARAM2 = "inChar16";
    const CIMName METHOD = CIMName("UTFMethod");
    const CIMObjectPath REFERENCE =
      CIMObjectPath("Test_LocalizedProviderClass.Identifier=0");

    // Array of UTF-16 chars to be sent and received to the server.
    // Note: the first 3 chars are taken from section 3.3.2 of the
    // CIM-over-HTTP spec.
    // The next 2 chars are a UTF-16 surrogate pair
    Char16 hangugo[] = {0xD55C, 0xAD6D, 0xC5B4,
            0xdbc0,
            0xdc01,
                    'g','l','o','b','a','l',
            0x00};

    try
    {
         // Before we begin, set the language objects to be sent to the provider
         AcceptLanguageList AL_DE;
         AL_DE.insert(LanguageTag("de"), Real32(0.8));
         ContentLanguageList CL_DE;
         CL_DE.append(LanguageTag("de"));
         client.setRequestAcceptLanguages(AL_DE);
         client.setRequestContentLanguages(CL_DE);

         //
         //  TEST 1 - Invoke Method with UTF-16 in input parameters,
         //  output parameters and return value.
         //
         //  This will also check that Accept-Language and Content-Language are
         //  are sent to and received from a Method Provider.

         cout << endl << "METHOD TEST 1: Invoke Method with UTF-16 parameters"
             << endl;

         // Strings sent to the provider
         String inString(utf16Chars);
         Char16 inChar16 = utf16Chars[1];

         // Expected strings from the provider
         String outString(utf16Chars);
         Char16 outChar16 = utf16Chars[2];
         String expectedRtnString(utf16Chars);

         // Strings returned by the provider
         String rtnString;
         String outParam1;
         Char16 outParam2;

         Array<CIMParamValue> inParams;
         Array<CIMParamValue> outParams;

         inParams.append( CIMParamValue(  INPARAM1, CIMValue( inString ) ) );
         inParams.append( CIMParamValue(  INPARAM2, CIMValue( inChar16 ) ) );

         if (verboseTest)
             cout << "Invoking the method" << endl;

         CIMValue retValue = client.invokeMethod(
            NAMESPACE,
            REFERENCE,
            METHOD,
            inParams,
            outParams);

         if (verboseTest)
             cout << "Checking for UTF-16 chars returned" << endl;

         retValue.get(rtnString);
         MYASSERT (expectedRtnString == rtnString);

         CIMValue paramVal = outParams[0].getValue();
         paramVal.get( outParam1 );
         MYASSERT (outString == outParam1);

         paramVal = outParams[1].getValue();
         paramVal.get( outParam2 );
         MYASSERT (outChar16 == outParam2);

         if (verboseTest)
            cout << "Checking returned Content-Language = " <<
                LanguageParser::buildContentLanguageHeader(CL_DE) << endl;

         MYASSERT(CL_DE == client.getResponseContentLanguages());

         //
         //  TEST 2 - Invoke method with UTF-16 in the method name.
         //
         //  This will test the URI encoding and decoding of UTF-8 in the
         //  CIMMethod HTTP header

         cout << endl << "METHOD TEST 2: Invoke Method with UTF-16 method name"
             << endl;

         String methodName(hangugo);

         if (verboseTest)
            cout << "Invoking the method" << endl;

         CIMValue retValue1 = client.invokeMethod(
            NAMESPACE,
            REFERENCE,
            methodName,
            inParams,
            outParams);

         // Check UTF-16 in the return value just to make sure that the method
         // was called on the provider.
         String expectedRtnString1(hangugo);
         String rtnString1;

         if (verboseTest)
            cout << "Checking for UTF-16 chars returned" << endl;

         retValue1.get(rtnString1);
         MYASSERT (expectedRtnString1 == rtnString1);

    }
    catch(Exception& e)
    {
        PEGASUS_STD(cerr) << "Error in TestLocalizedMethod: " << e.getMessage()
            << PEGASUS_STD(endl);
        throw;
    }
}

/* Enumerates instances of the sample classes using a given AcceptLanguageList,
   and checks the language of the response.
*/
static Array<CIMInstance> EnumerateTestInstances(CIMClient &client,
                                     AcceptLanguageList & acceptLangs,
                                     ContentLanguageList & contentLangs,
                                     String & expectedStr,
                                     Boolean verboseTest)
{
    const CIMNamespaceName NAMESPACE = CIMNamespaceName ("test/TestProvider");
    const CIMName CLASSNAME = CIMName ("Test_LocalizedProviderClass");
    Boolean deepInheritance = false;
    Boolean localOnly = false;
    Boolean includeQualifiers = true;
    Boolean includeClassOrigin = false;
    ContentLanguageList CL_EN;
    CL_EN.append(LanguageTag("en"));
    const String RBPROP = "ResourceBundleString";
    String expectedDftString = "ResourceBundleString DEFAULT";

    client.setRequestAcceptLanguages(acceptLangs);

    Array<CIMInstance> cimNInstances =
                     client.enumerateInstances(NAMESPACE,  CLASSNAME,
                                               deepInheritance,
                                               localOnly,  includeQualifiers,
                                               includeClassOrigin );

    if (verboseTest)
        cout << "Found " << cimNInstances.size() << " Instances of "
             << CLASSNAME.getString() << endl;

    MYASSERT(cimNInstances.size() == 3);

    if (skipICU)
    {
        if (verboseTest)
        cout << "Checking expected response ContentLanguageList: " <<
            LanguageParser::buildContentLanguageHeader(CL_EN) << endl;

        // Note - the LocalizedProvider is setting en in Content-Languages
        // to override the MessageLoader.
        // Otherwise, Content-Languages would be empty since Accept-Languages
        // was empty.
        MYASSERT(CL_EN == client.getResponseContentLanguages());
    }
    else
    {
        if (verboseTest)
        cout << "Checking expected response ContentLanguageList: " <<
            LanguageParser::buildContentLanguageHeader(contentLangs) << endl;

        MYASSERT(contentLangs == client.getResponseContentLanguages());
    }

    if (verboseTest)
      cout << "Checking the returned string in each instance" << endl;

    for (Uint32 k = 0; k < cimNInstances.size(); k++)
    {
        String enumString;
        cimNInstances[k].getProperty (cimNInstances[k].findProperty(RBPROP)).
                      getValue().
                      get(enumString);

        if (skipICU) {
            MYASSERT(expectedDftString == enumString);
        }
        else
        {
            MYASSERT(expectedStr == enumString);
        }
    }

  return cimNInstances;
}

/*
   Tests the globalization support of the LocalizedProvider
   for the instance operations
*/
static void TestLocalizedInstances( CIMClient& client,
                                    Boolean activeTest,
                                    Boolean verboseTest )
{
    const CIMNamespaceName NAMESPACE = CIMNamespaceName ("test/TestProvider");
    const CIMName CLASSNAME = CIMName ("Test_LocalizedProviderClass");
    const CIMName SUBCLASSNAME = CIMName ("Test_LocalizedProviderSubClass");
    const String INSTANCE0 = "instance 0Test_LocalizedProviderClass";
    const String INSTANCE1 = "instance 1Test_LocalizedProviderClass";
    const String RBPROP = "ResourceBundleString";
    const String ROUNDTRIPSTRINGPROP = "RoundTripString";
    const String ROUNDTRIPCHARPROP = "RoundTripChar";
    const String CLPROP = "ContentLanguageString";
    const String TESTSTRINGPROP = "TestString";
    const String IDPROP = "Identifier";

    ContentLanguageList CL_DE;
    CL_DE.append(LanguageTag("de"));
    String expectedDEString = "ResourceBundleString DE";
    ContentLanguageList CL_FR;
    CL_FR.append(LanguageTag("fr"));
    String expectedFRString = "ResourceBundleString FR";
    ContentLanguageList CL_Dft;
    ContentLanguageList CL_EN;
    CL_EN.append(LanguageTag("en"));
    String expectedDftString = "ResourceBundleString DEFAULT";

    ContentLanguageList CL_HOMER;
    CL_HOMER.append(LanguageTag("x-homer"));
    ContentLanguageList CL_ES;
    CL_ES.append(LanguageTag("es"));

    String expectedUTF16String(utf16Chars);

    try
    {
        const CIMName className = CLASSNAME;
        Boolean localOnly = false;
        Boolean includeQualifiers = true;
        Boolean includeClassOrigin = false;
        Uint32 testRepeat = 1;

        if (verboseTest)
            cout << "Getting class: " << CLASSNAME.getString() << endl;

        CIMClass sampleClass = client.getClass(
            NAMESPACE,
            CLASSNAME,
            localOnly,
            includeQualifiers,
            includeClassOrigin);

        CIMClass sampleSubClass = client.getClass(
            NAMESPACE,
            SUBCLASSNAME,
            localOnly,
            includeQualifiers,
            includeClassOrigin);

        // Note:  When Pegasus has external message files enabled,
        // then the provider has message files for fr, de,
        // and es, but no root message file.  The missing root
        // message file will cause us to test the default message
        // strings compiled into the code.

        for (Uint32 q = 0; q < testRepeat; q++)        // repeat the test x time
        {
            //
            //  TEST 1 - Enumerate Instances of the base class,
            //  deepInheritance == false.
            //  de is supported by the provider, and is the most preferred
            //  by us.
            //
            AcceptLanguageList acceptLangs1;
            if (skipICU)
            {
                 // Not requesting translated messages from the provider
                 acceptLangs1.clear();
            }
            else
            {
                 acceptLangs1.insert(LanguageTag("fr"), Real32(0.5));
                 acceptLangs1.insert(LanguageTag("de"), Real32(0.8));
                 acceptLangs1.insert(LanguageTag("es"), Real32(0.4));
            }

            cout << endl
                << "INSTANCE TEST 1: Enumerate Instances with"
                        " AcceptLanguageList = "
                << LanguageParser::buildAcceptLanguageHeader(acceptLangs1)
                << endl;

            Array<CIMInstance> cimNInstances = EnumerateTestInstances(
                                               client,
                                               acceptLangs1,
                                               CL_DE,
                                               expectedDEString,
                                               verboseTest);


            //
            //  TEST 2 - Aggregation Test -
            //  Enumerate Instances starting at the base class,
            //  with deep inheritance, where each instance has
            //  the same language.
            //  This will test that the languages of the instances are
            //  aggregated into the Content-Language of the whole response.
            //
            //  (tests ContentLanguage aggregation in
            //  CIMOperationRequestDispatcher)

            cout << endl
                << "INSTANCE TEST 2: Enumerate Instances with"
                        " Content-Language match"
                << endl;

            AcceptLanguageList acceptLangs2;
            if (skipICU)
            {
                // Not requesting translated messages from the provider
                acceptLangs2.clear();
            }
            else
            {
                acceptLangs2.insert(LanguageTag("de"), Real32(0.8));
            }

            client.setRequestAcceptLanguages(acceptLangs2);

            Array<CIMInstance> cimNInstances2 =
                              client.enumerateInstances(NAMESPACE,
                                                        className,
                                                        true,
                                                        false,
                                                        includeQualifiers,
                                                        includeClassOrigin);

            if (verboseTest)
                cout << "Found " << cimNInstances2.size() << " Instances of "
                    << className.getString() << endl;

            MYASSERT(cimNInstances2.size() == 3);

            if (skipICU)
            {
                if (verboseTest)
                    cout << "Checking expected response ContentLanguageList: "
                        <<  LanguageParser::buildContentLanguageHeader(CL_EN)
                        << endl;

                MYASSERT(CL_EN == client.getResponseContentLanguages());
            }
            else
            {
                if (verboseTest)
                   cout << "Checking expected response ContentLanguageList: " <<
                       LanguageParser::buildContentLanguageHeader(CL_DE)
                       << endl;

                MYASSERT(CL_DE == client.getResponseContentLanguages());
            }

            //
            //  TEST 3 - Aggregation Test - Enumerate Instances with deep
            //  inheritance, where the instances do not have the same language.
            //  Since the languages of the instances
            //  do not match, we expect the ContentLanguage of the
            //  whole response to be empty.
            //  This will test that the languages of the instances are
            //  aggregated into the Content-Language of the whole response.
            //
            //  We will modify the language of the sub-class instance,
            //  and then do the enumerate.
            //
            //  (tests ContentLanguage aggregation in
            //  CIMOperationRequestDispatcher)
            //

            cout << endl
                << "INSTANCE TEST 3: Enumerate Instances with"
                        " Content-Language mismatch"
                << endl;

            if (!activeTest)
            {
                cout << "Active tests are disabled. Nothing to do for"
                            " this test."
                  << endl;
            }
            else
            {
                //  First, find the instance of the sub-class
                Uint32 i = 0;
                for (; i < cimNInstances2.size(); i++)
                {
                    if (cimNInstances2[i].getClassName() == SUBCLASSNAME)
                    {
                        break;
                    }
                }

            MYASSERT(i != cimNInstances2.size());

            if (verboseTest)
                cout << "Modifying the language of an instance of the"
                        " subclass to "
                     << LanguageParser::buildContentLanguageHeader(CL_HOMER)
                     << endl;

            client.setRequestContentLanguages(CL_HOMER);

            client.modifyInstance(NAMESPACE,
                              cimNInstances2[i],
                              includeQualifiers);

            //  Enumerate the instances starting at the base class,
            //  with deep inheritance.
            AcceptLanguageList acceptLangs3;
            acceptLangs3.insert(LanguageTag("x-homer"), Real32(0.8));

            client.setRequestAcceptLanguages(acceptLangs3);

            Array<CIMInstance> cimNInstances3 =
                          client.enumerateInstances(NAMESPACE,
                            className, true,
                            false,  includeQualifiers,
                            includeClassOrigin );

            if (verboseTest)
               cout << "Found " << cimNInstances3.size() << " Instances of "
                    << className.getString() << endl;

            MYASSERT(cimNInstances3.size() == 3);

            if (verboseTest)
               cout << "Checking expected empty response"
                          " ContentLanguageList"
                   << endl;

            MYASSERT(client.getResponseContentLanguages().size() == 0);
            } // else active tests

            //
            //  TEST 4 - Get Instance, check default language returned.
            //  None of our preferred languages is supported by the
            //  provider, and the root message file is empty.  Expect
            //  the compiled-in default strings from the provider.
            //

            AcceptLanguageList acceptLangs4;
            acceptLangs4.insert(LanguageTag("x-martian"), Real32(0.8));
            acceptLangs4.insert(LanguageTag("x-pig-latin"), Real32(0.1));
            acceptLangs4.insert(LanguageTag("x-men"), Real32(0.4));
            client.setRequestAcceptLanguages(acceptLangs4);

            cout << endl
                << "INSTANCE TEST 4: Get Instance with AcceptLanguageList = "
                << LanguageParser::buildAcceptLanguageHeader(acceptLangs4)
                << endl;

            //  Find an instance of the base-class from the first enumerate
            Uint32 j = 0;
            for (; j < cimNInstances.size(); j++)
            {
                if (cimNInstances[j].getClassName() == CLASSNAME)
                {
                    break;
                }
            }

            MYASSERT(j != cimNInstances.size());

            // Get the instance.  Note, baseInstance is used later
            CIMObjectPath baseObjectPath =
              cimNInstances[j].buildPath(sampleClass);

            CIMInstance baseInstance = client.getInstance(
                NAMESPACE,
                baseObjectPath,
                localOnly,
                includeQualifiers,
                includeClassOrigin);

            // update base instance object path
            // (it's not set by the cimom or client)
            baseInstance.setPath(baseObjectPath);

            String returnedString;
            Char16 returnedChar16;

            baseInstance.getProperty (baseInstance.findProperty(RBPROP)).
                getValue().
                get(returnedString);

            if (verboseTest)
               cout << "Checking for default language returned" << endl;

            MYASSERT(expectedDftString == returnedString);

            //
            //  TEST 5 - Get Instance, check fr language returned.
            //  fr is the lowest preferred language from us, but
            //  is the only one in the list supported by the provider.
            //

            AcceptLanguageList acceptLangs5;
            if (skipICU)
            {
                // Not requesting translated messages from the provider
                acceptLangs5.clear();
            }
            else
            {
                acceptLangs5.insert(LanguageTag("x-martian"), Real32(0.8));
                acceptLangs5.insert(LanguageTag("fr"), Real32(0.1));
                acceptLangs5.insert(LanguageTag("x-men"), Real32(0.4));
            }

            client.setRequestAcceptLanguages(acceptLangs5);

            cout << endl
                << "INSTANCE TEST 5: Get Instance with AcceptLanguageList = "
                << LanguageParser::buildAcceptLanguageHeader(acceptLangs5)
                << endl;

            CIMInstance instance5 = client.getInstance(
                    NAMESPACE,
                    cimNInstances[1].buildPath(sampleClass),
                    localOnly,
                    includeQualifiers,
                    includeClassOrigin);

            instance5.getProperty (instance5.findProperty(RBPROP)).
               getValue().
               get(returnedString);

            if (skipICU)
            {
                if (verboseTest)
                  cout << "Checking for default language returned" << endl;

                MYASSERT(CL_EN == client.getResponseContentLanguages());
                MYASSERT(expectedDftString == returnedString);
            }
            else
            {
                if (verboseTest)
                  cout << "Checking for fr returned" << endl;

                MYASSERT(CL_FR == client.getResponseContentLanguages());
                MYASSERT(expectedFRString == returnedString);
            }

            //
            //  TEST 6 - Round trip Test.
            //  Create Instance followed by Get Instance.
            //
            //  Create with an fr string and expect that
            //  the fr string is preserved on get instance.
            //  (fr is one of the supported languages of the provider)
            //
            //  Create with a UTF-16 string and char16 and expect that they
            //  are preserved on get instance
            //

            cout << endl << "INSTANCE TEST 6: Create Instance with"
                            " ContentLanguageList = "
                << LanguageParser::buildContentLanguageHeader(CL_FR)
                << "  and UTF-16 string." << endl;

            if (!activeTest)
            {
                cout << "Active tests are disabled. Nothing to do for"
                            " this test."
                     << endl;
            }
            else
            {
                String oui = "Oui";

                CIMInstance frInstance(CLASSNAME);
                frInstance.addProperty(CIMProperty(CIMName(CLPROP), oui));
                frInstance.addProperty(
                  CIMProperty(
                      CIMName(ROUNDTRIPSTRINGPROP), String(utf16Chars)));
                frInstance.addProperty(
                  CIMProperty(CIMName(ROUNDTRIPCHARPROP), utf16Chars[0]));
                frInstance.addProperty(CIMProperty(CIMName(IDPROP), Uint8(3)));

                CIMObjectPath frInstanceName =
                    frInstance.buildPath(sampleClass);
                frInstance.setPath(frInstanceName);

                client.setRequestContentLanguages(CL_FR);

                if (verboseTest)
                    cout << "Creating the instance" << endl;

                client.createInstance(NAMESPACE, frInstance);

                AcceptLanguageList acceptLangs6;
                acceptLangs6.insert(LanguageTag("x-martian"), Real32(0.8));
                acceptLangs6.insert(LanguageTag("fr"), Real32(0.1));
                acceptLangs6.insert(LanguageTag("x-men"), Real32(0.4));
                client.setRequestAcceptLanguages(acceptLangs6);

                if (verboseTest)
                    cout << "Getting the instance just created,"
                                " using AcceptLanguageList = "
                        << LanguageParser::buildAcceptLanguageHeader(
                                acceptLangs6)
                        << endl;

                CIMInstance instance6 = client.getInstance(
                    NAMESPACE,
                    frInstanceName,
                    localOnly,
                    includeQualifiers,
                    includeClassOrigin);

                instance6.getProperty (
                    instance6.findProperty(ROUNDTRIPSTRINGPROP)).
                    getValue().
                    get(returnedString);

                instance6.getProperty (
                        instance6.findProperty(ROUNDTRIPCHARPROP)).
                    getValue().
                    get(returnedChar16);

                if (verboseTest)
                    cout << "Checking the UTF-16 chars were preserved"
                      << endl;

                MYASSERT(expectedUTF16String == returnedString);
                MYASSERT(utf16Chars[0] == returnedChar16);

                instance6.getProperty (instance6.findProperty(CLPROP)).
                    getValue().
                    get(returnedString);

                if (verboseTest)
                    cout << "Checking for fr returned" << endl;

                MYASSERT(oui == returnedString);
                MYASSERT(CL_FR == client.getResponseContentLanguages());

                // Delete the instance for the next pass
                if (verboseTest)
                   cout << "Deleting the instance" << endl;

                client.deleteInstance(
                NAMESPACE,
                frInstanceName);
            } // end else active tests

            //
            //  TEST 7 - Round trip Test.
            //  Modify Instance followed by Get Instance.
            //
            //  Modify an instance of the base class with an x-homer language
            //  string and expect that the x-homer string is preserved on get
            //  instance.
            //  (x-homer is not one of the supported languages of the provider
            //   but still should be returned when asked for)
            //
            //  Modify baseInstance with a UTF-16 string and
            //  char16 and expect that they are preserved on get instance
            //

            cout << endl
                << "INSTANCE TEST 7: Modify Instance with"
                        " ContentLanguageList = "
                << LanguageParser::buildContentLanguageHeader(CL_HOMER)
                << " and UTF-16 string." << endl;

            if (!activeTest)
            {
                cout
                   << "Active tests are disabled. Nothing to do for this test."
                   << endl;
            }
            else
            {
                String doh = "doh!";
                baseInstance.removeProperty (baseInstance.findProperty(CLPROP));
                baseInstance.addProperty(CIMProperty(CIMName(CLPROP), doh));

                baseInstance.removeProperty (
                baseInstance.findProperty(ROUNDTRIPSTRINGPROP));
                baseInstance.addProperty(CIMProperty(
                CIMName(ROUNDTRIPSTRINGPROP), String(utf16Chars)));

                baseInstance.removeProperty (
                baseInstance.findProperty(ROUNDTRIPCHARPROP));
                baseInstance.addProperty(
                CIMProperty(CIMName(ROUNDTRIPCHARPROP),utf16Chars[0]));

                client.setRequestContentLanguages(CL_HOMER);

                if (verboseTest)
                   cout << "Modifying the instance" << endl;

                client.modifyInstance(NAMESPACE,
                                      baseInstance,
                                      includeQualifiers);

                AcceptLanguageList acceptLangs7;
                acceptLangs7.insert(LanguageTag("x-homer"), Real32(0.8));
                client.setRequestAcceptLanguages(acceptLangs7);

                if (verboseTest)
                    cout << "Getting the instance just modified,"
                                " using AcceptLanguageList = "
                        << LanguageParser::buildAcceptLanguageHeader(
                                acceptLangs7)
                        << endl;

                CIMInstance instance7 = client.getInstance(
                                           NAMESPACE,
                                           baseInstance.buildPath(sampleClass),
                                           localOnly,
                                           includeQualifiers,
                                           includeClassOrigin);

                instance7.getProperty (
                    instance7.findProperty(ROUNDTRIPSTRINGPROP)).
                    getValue().
                    get(returnedString);

                instance7.getProperty (
                    instance7.findProperty(ROUNDTRIPCHARPROP)).
                    getValue().
                    get(returnedChar16);

                if (verboseTest)
                    cout << "Checking the UTF-16 chars were preserved" << endl;

                MYASSERT(expectedUTF16String == returnedString);
                MYASSERT(utf16Chars[0] == returnedChar16);

                instance7.getProperty (
                    instance7.findProperty(CLPROP)).
                    getValue().
                    get(returnedString);

                if (verboseTest)
                    cout << "Checking for x-homer returned" << endl;

                MYASSERT(returnedString == doh);
                MYASSERT(CL_HOMER == client.getResponseContentLanguages());
            } // end else active tests

            //
            //  TEST 8 - Exception test.
            //  Expect an exception to be returned from the server
            //  with a localized message in es locale.
            //
            //  Note: the provider will throw a not-supported exception
            //  on delete instance requests.
            //

            AcceptLanguageList acceptLangs8;
            if (skipICU)
            {
                // Not requesting translated messages from the provider
                acceptLangs8.clear();
            }
            else
            {
                acceptLangs8.insert(LanguageTag("es"), Real32(1.0));
                acceptLangs8.insert(LanguageTag("fr"), Real32(0.9));
            }

            client.setRequestAcceptLanguages(acceptLangs8);

            cout << endl << "INSTANCE TEST 8: Delete Instance with"
                                " AcceptLanguageList = " <<
                LanguageParser::buildAcceptLanguageHeader(acceptLangs8) << endl;

            // Try to delete baseInstance, expect an exception
            if (verboseTest)
                cout << "Deleting the instance, expecting to get a CIMException"
                     << endl;

            Boolean gotException = false;
            try
            {
              client.deleteInstance(
                                    NAMESPACE,
                                    baseInstance.buildPath(sampleClass));
            } catch (CIMException & ce)
            {
                if (verboseTest)
                    cout << "Got the CIMException" << endl;
                String message = ce.getMessage();

                if(skipICU)
                {
                    if (verboseTest)
                      cout << "Checking for default language returned in the "
                                    "exception"
                           << endl;

                    MYASSERT(CL_Dft == client.getResponseContentLanguages());
                    Uint32 n = message.find("default");
                    MYASSERT(n != PEG_NOT_FOUND);
                }
                else
                {
                    if (verboseTest)
                            cout << "Checking for es returned in the exception"
                                << endl;

                    MYASSERT(CL_ES == client.getResponseContentLanguages());
                    MYASSERT(CL_ES == ce.getContentLanguages());
                    Uint32 n = message.find("ES");
                    MYASSERT(n != PEG_NOT_FOUND);
                }

                gotException = true;
            }

            MYASSERT(gotException == true);

        }  // end for
    }
    catch(Exception& e)
    {
        PEGASUS_STD(cerr) << "Error in TestLocalizedInstances: "
            << e.getMessage() << PEGASUS_STD(endl);
        throw;
    }
}

/*
   Tests the globalization support of the Pegasus server by sending requests
   that cause error messages to be returned.
*/
static void TestServerMessages( CIMClient& client,
                                String reqLang,
                                String rspLang,
                                Boolean activeTest,
                                Boolean verboseTest )
{
    try
    {
        //
        // Vars for the getClass test
        //

        // Build the Accept-Languages for the getClass request using the lang
        // specified by the user
        AcceptLanguageList acceptLangs1;
        if (reqLang.size() > 0)
        {
            acceptLangs1 = LanguageParser::parseAcceptLanguageHeader(reqLang);
        }

        // This is the "class not found" message expected back from the server.
        // Since we don't know what the message text will be for a given
        // langauge, we will use the message returned on the first request
        // as the expected message for subsequent requests.
        String expectedMsg;

        // This is the language of the "class not found" message back
        // from the server,  as specified by the user
        ContentLanguageList expectedCL;
        if (rspLang.size() > 0)
        {
            expectedCL = LanguageParser::parseContentLanguageHeader(rspLang);
        }

        //
        // Vars for enumerate sample instances test
        //

        // Build the Accept-Languages for the enumerate sample instances test
        AcceptLanguageList acceptLangsDE;
        if (skipICU)
        {
           // Not requesting translated messages from the provider
           acceptLangsDE.clear();
        }
        else
        {
           acceptLangsDE.insert(LanguageTag("de"), Real32(0.1));
           acceptLangsDE.insert(LanguageTag("x-aa"), Real32(0.2));
           acceptLangsDE.insert(LanguageTag("x-bb"), Real32(0.3));
        }

        AcceptLanguageList acceptLangsFR;
        if (skipICU)
        {
           // Not requesting translated messages from the provider
           acceptLangsFR.clear();
        }
        else
        {
           acceptLangsFR.insert(LanguageTag("fr"), Real32(0.1));
           acceptLangsFR.insert(LanguageTag("x-aa"), Real32(0.2));
           acceptLangsFR.insert(LanguageTag("x-bb"), Real32(0.3));
        }

        AcceptLanguageList acceptLangsES;
        if (skipICU)
        {
           // Not requesting translated messages from the provider
           acceptLangsES.clear();
        }
        else
        {
           acceptLangsES.insert(LanguageTag("es"), Real32(0.1));
           acceptLangsES.insert(LanguageTag("x-aa"), Real32(0.2));
           acceptLangsES.insert(LanguageTag("x-bb"), Real32(0.3));
        }

        // The expected results from the enumerate
        ContentLanguageList CL_DE;
        CL_DE.append(LanguageTag("de"));
        String expectedDEString = "ResourceBundleString DE";
        ContentLanguageList CL_FR;
        CL_FR.append(LanguageTag("fr"));
        String expectedFRString = "ResourceBundleString FR";
        ContentLanguageList CL_ES;
        CL_ES.append(LanguageTag("es"));
        String expectedESString = "ResourceBundleString ES";

        cout << endl << "SERVER MESSAGE TESTS: Send requests that test"
                            " server language support"
                        << endl;

        // Do a loop of requests.  This is useful when several instances of this
        // program are run simultaneously, each with a different language.
        for (int i = 0; i < 100; i ++)
        {
            //
            // TEST 1 - Cause localized error message.
            //
            // Attempt to create a PG_ProviderModule instance with no Name
            // property.  Expect the error message to be returned in the
            // language specified by the user.
            //

            try
            {
                client.setRequestAcceptLanguages(acceptLangs1);

                CIMInstance inst("PG_ProviderModule");
                client.createInstance(
                    PEGASUS_NAMESPACENAME_INTEROP.getString(),
                    inst);

                // should not get here
                throw Exception(
                    "did not get expected exception for missing Name property");
            }
            catch (CIMException& ce)
            {
                if (ce.getCode() == CIM_ERR_INVALID_PARAMETER)
                {
                    if ((String::compare(
                            ce.getMessage(),
                            "CIM_ERR_INVALID_PARAMETER",
                            strlen("CIM_ERR_INVALID_PARAMETER")) != 0) ||
                        (ce.getMessage().size() ==
                            strlen("CIM_ERR_INVALID_PARAMETER")))
                    {
                        throw;
                    }

                   if (i == 0)
                   {
                       // Since we don't know the text of the server message in
                       // each language, save the message on the first request
                       expectedMsg = ce.getMessage();
                       if (verboseTest)
                           cout << "Note: Expecting message = " << expectedMsg
                                << " from the server" << endl;
                   }

                   // Verify the message and the content languages returned
                   MYASSERT(expectedMsg == ce.getMessage());
                   MYASSERT(expectedCL == client.getResponseContentLanguages());
                   MYASSERT(expectedCL == ce.getContentLanguages());
                }
                else
                {
                   // Didn't get the expected error for "class not found"
                   throw;
                }
            }  // end catch

            //
            // TEST 2 - Enumerate instances of the sample classes using
            // differentlanguages.
            //
            // Enumerate instances of the sample classes, alternating the
            // Accept-Language used among the languages supported by the
            // LocalizedProvider.  When multiple instance of this program
            // are run simultaneously, this test
            // will exercise multi-language support in the server.
            //
        switch (i % 3)
            {
               case 0:
//                 if (verboseTest)
//                     cout << "Sending the enumerate instance request " << i <<
//                              ". Lang = " << acceptLangsDE.toString() << endl;

                   EnumerateTestInstances(
                                           client,
                                           acceptLangsDE,
                                           CL_DE,
                                           expectedDEString,
                                           false); // don't get too verbose
                   break;
               case 1:
//                 if (verboseTest)
//                     cout << "Sending the enumerate instance request " << i <<
//                              ". Lang = " << acceptLangsFR.toString() << endl;

                   EnumerateTestInstances(
                                           client,
                                           acceptLangsFR,
                                           CL_FR,
                                           expectedFRString,
                                           false); // don't get too verbose
                   break;
               case 2:
//                 if (verboseTest)
//                     cout << "Sending the enumerate instance request " << i <<
//                              ". Lang = " << acceptLangsES.toString() << endl;

                   EnumerateTestInstances(
                                           client,
                                           acceptLangsES,
                                           CL_ES,
                                           expectedESString,
                                           false); // don't get too verbose
                   break;
            }
        } // endfor
    }
    catch (InvalidAcceptLanguageHeader& ialh)
    {
        PEGASUS_STD(cerr) << "Invalid lang parameter was entered: "
            << ialh.getMessage() << PEGASUS_STD(endl);
        throw;
    }
    catch (InvalidContentLanguageHeader& iclh)
    {
        PEGASUS_STD(cerr) << "Invalid expectlang parameter was entered: "
            << iclh.getMessage() << PEGASUS_STD(endl);
        throw;
    }
    catch(Exception& e)
    {
        PEGASUS_STD(cerr) << "Error in TestServerMessages: "
            << e.getMessage() << PEGASUS_STD(endl);
        throw;
    }
}

/*
   Builds a filter or handler object path
*/
static CIMObjectPath buildFilterOrHandlerPath
    (const CIMName & className,
     const String & name)
{
    CIMObjectPath path;

    Array <CIMKeyBinding> keyBindings;
    keyBindings.append (CIMKeyBinding ("SystemCreationClassName",
        System::getSystemCreationClassName (), CIMKeyBinding::STRING));
    keyBindings.append (CIMKeyBinding ("SystemName",
        System::getFullyQualifiedHostName (), CIMKeyBinding::STRING));
    keyBindings.append (CIMKeyBinding ("CreationClassName",
        className.getString(), CIMKeyBinding::STRING));
    keyBindings.append (CIMKeyBinding ("Name", name,
        CIMKeyBinding::STRING));
    path.setClassName (className);
    path.setKeyBindings (keyBindings);

    return path;
}

/*
   Builds a subscription object path
*/
static CIMObjectPath buildSubscriptionObjectPath
    (const CIMObjectPath & filterPath,
     const CIMObjectPath & handlerPath)
{
    CIMObjectPath path;

    Array <CIMKeyBinding> keyBindings;
    keyBindings.append (CIMKeyBinding ("Filter", filterPath.toString (),
        CIMKeyBinding::REFERENCE));
    keyBindings.append (CIMKeyBinding ("Handler", handlerPath.toString (),
        CIMKeyBinding::REFERENCE));
    path.setClassName (PEGASUS_CLASSNAME_INDSUBSCRIPTION);
    path.setKeyBindings (keyBindings);

    return path;
}

/*
   Creates the subscriptions, filters, and handlers for the indication tests
*/
static void createSubscriptions(CIMClient& client,
                CIMObjectPath & g11ntestHandlerRef,
                CIMObjectPath & providerHandlerRef,
                CIMObjectPath & filterRef,
                CIMObjectPath & g11ntestSubscriptionRef,
                CIMObjectPath & providerSubscriptionRef,
                String & listenerDest,
                Boolean skipListener,
                Boolean verboseTest)
{
  const CIMNamespaceName TEST_NAMESPACE =
      CIMNamespaceName ("test/TestProvider");
// Interop namespace used with PEGASUS_NAMESPACENAME_INTEROP in Constants.h
  const CIMName METHOD = CIMName("getIndicationResult");
  const CIMObjectPath REFERENCE =
      CIMObjectPath("Test_LocalizedProviderClass.Identifier=0");

  try
  {
    //---------------------------------------------------------
    // Build the paths to the handlers, filter, and subscription
    //---------------------------------------------------------

    // The handler that will have program as the destination
    CIMObjectPath g11ntestHandlerPath = buildFilterOrHandlerPath
        (PEGASUS_CLASSNAME_INDHANDLER_CIMXML, "g11ntest_Handler");

    // The handler that will have LocalizedProvider as the destination
    CIMObjectPath providerHandlerPath = buildFilterOrHandlerPath
        (PEGASUS_CLASSNAME_INDHANDLER_CIMXML, "provider_Handler");

    // The filter
    CIMObjectPath filterPath = buildFilterOrHandlerPath
        (PEGASUS_CLASSNAME_INDFILTER, "g11ntest_Filter");

    // The subscription that will have this program as the destination
    CIMObjectPath g11ntestSubscriptionPath = buildSubscriptionObjectPath
        (filterPath, g11ntestHandlerPath);

    // The subscription that will have LocalizedProvider as the destination
    CIMObjectPath providerSubscriptionPath = buildSubscriptionObjectPath
        (filterPath, providerHandlerPath);

    // Need to delete the old subscription and handler that have this
    // program as the destination.  This is done in case the port changed
    // since the last time.
    if (verboseTest)
      cout << "Deleting the old subscription" << endl;

    try
    {
      client.deleteInstance(PEGASUS_NAMESPACENAME_INTEROP,
              CIMObjectPath(g11ntestSubscriptionPath));
    }
    catch (CIMException & ce)
    {
      if (ce.getCode() != CIM_ERR_NOT_FOUND)
      {
        throw;
      }
    }

    if (verboseTest)
      cout << "Deleting the old handler" << endl;

    try
    {
      client.deleteInstance(PEGASUS_NAMESPACENAME_INTEROP,
              CIMObjectPath(g11ntestHandlerPath));
    }
    catch (CIMException & ce)
    {
      if (ce.getCode() != CIM_ERR_NOT_FOUND)
      {
        throw;
      }
    }

    // Create the new handler instance with this program as the destination
    if (verboseTest)
      cout << "Creating the handlers" << endl;

    if (!skipListener)
    {
      CIMInstance g11ntestHandlerInstance(PEGASUS_CLASSNAME_INDHANDLER_CIMXML);
      g11ntestHandlerInstance.addProperty(
              CIMProperty (CIMName("SystemCreationClassName"),
          System::getSystemCreationClassName ()));
      g11ntestHandlerInstance.addProperty(CIMProperty(CIMName ("SystemName"),
          System::getFullyQualifiedHostName ()));
      g11ntestHandlerInstance.addProperty(
              CIMProperty(CIMName ("CreationClassName"),
                          PEGASUS_CLASSNAME_INDHANDLER_CIMXML.getString()));
      g11ntestHandlerInstance.addProperty(CIMProperty(CIMName ("Name"),
                              String("g11ntest_Handler")));
      g11ntestHandlerInstance.addProperty(
              CIMProperty(CIMName ("Destination"), listenerDest));

      g11ntestHandlerRef = client.createInstance(
              PEGASUS_NAMESPACENAME_INTEROP, g11ntestHandlerInstance);
    }

    // Create the new handler instance with LocalizedProvider as the destination
    CIMInstance providerHandlerInstance(PEGASUS_CLASSNAME_INDHANDLER_CIMXML);
    providerHandlerInstance.addProperty(
            CIMProperty (CIMName("SystemCreationClassName"),
        System::getSystemCreationClassName ()));
    providerHandlerInstance.addProperty(CIMProperty(CIMName ("SystemName"),
        System::getFullyQualifiedHostName ()));
    providerHandlerInstance.addProperty(
            CIMProperty(CIMName ("CreationClassName"),
        PEGASUS_CLASSNAME_INDHANDLER_CIMXML.getString()));
    providerHandlerInstance.addProperty(CIMProperty(CIMName ("Name"),
        String("provider_Handler")));
    providerHandlerInstance.addProperty(CIMProperty(CIMName ("Destination"),
        String("http://localhost:5988/CIMListener/localizedprovider")));

    try
    {
      providerHandlerRef = client.createInstance(PEGASUS_NAMESPACENAME_INTEROP,
                                                 providerHandlerInstance);
    }
    catch (CIMException& ce)
    {
      if (ce.getCode() != CIM_ERR_ALREADY_EXISTS)
        throw;

      providerHandlerRef = CIMObjectPath(providerHandlerPath);
    }

    // Create the filter instance
    if (verboseTest)
      cout << "Creating the filter" << endl;

    CIMInstance filterInstance(PEGASUS_CLASSNAME_INDFILTER);
    filterInstance.addProperty(CIMProperty (CIMName ("SystemCreationClassName"),
        System::getSystemCreationClassName ()));
    filterInstance.addProperty(CIMProperty(CIMName ("SystemName"),
        System::getFullyQualifiedHostName ()));
    filterInstance.addProperty(CIMProperty(CIMName ("CreationClassName"),
        PEGASUS_CLASSNAME_INDFILTER.getString()));
    filterInstance.addProperty(CIMProperty(CIMName ("Name"),
        String("g11ntest_Filter")));
    filterInstance.addProperty (CIMProperty(CIMName ("SourceNamespace"),
        String("test/TestProvider")));
    filterInstance.addProperty (CIMProperty(CIMName ("Query"),
    String("SELECT * FROM LocalizedProvider_TestIndication")));
    filterInstance.addProperty (CIMProperty(CIMName ("QueryLanguage"),
        String("WQL")));

    try
    {
      filterRef = client.createInstance(PEGASUS_NAMESPACENAME_INTEROP,
                                        filterInstance);
    }
    catch (CIMException& ce)
    {
      if (ce.getCode() != CIM_ERR_ALREADY_EXISTS)
        throw;

      filterRef = CIMObjectPath(filterPath);
    }

    // Create the subscription
    if (verboseTest)
      cout << "Creating the subscription" << endl;

    if (!skipListener)
    {
      // The subscription with this program as the destination
      CIMInstance g11ntestSubscriptionInstance(
              PEGASUS_CLASSNAME_INDSUBSCRIPTION);
      g11ntestSubscriptionInstance.addProperty(CIMProperty(CIMName ("Filter"),
                   filterRef, 0, PEGASUS_CLASSNAME_INDFILTER));
      g11ntestSubscriptionInstance.addProperty(CIMProperty(CIMName ("Handler"),
                   g11ntestHandlerRef, 0, PEGASUS_CLASSNAME_INDHANDLER_CIMXML));
      g11ntestSubscriptionInstance.addProperty (CIMProperty
                (CIMName ("SubscriptionState"), CIMValue ((Uint16) 2)));

      try
    {
      g11ntestSubscriptionRef =
          client.createInstance(PEGASUS_NAMESPACENAME_INTEROP,
                                g11ntestSubscriptionInstance);
    }
      catch (CIMException& ce)
    {
      if (ce.getCode() != CIM_ERR_ALREADY_EXISTS)
        throw;

      g11ntestSubscriptionRef = CIMObjectPath(g11ntestSubscriptionPath);
    }
    }

    // The subscription with LocalizedProvider as the destination
    CIMInstance providerSubscriptionInstance(PEGASUS_CLASSNAME_INDSUBSCRIPTION);
    providerSubscriptionInstance.addProperty(CIMProperty(CIMName ("Filter"),
        filterRef, 0, PEGASUS_CLASSNAME_INDFILTER));
    providerSubscriptionInstance.addProperty(CIMProperty(CIMName ("Handler"),
        providerHandlerRef, 0, PEGASUS_CLASSNAME_INDHANDLER_CIMXML));
    providerSubscriptionInstance.addProperty (CIMProperty
        (CIMName ("SubscriptionState"), CIMValue ((Uint16) 2)));

    try
    {
        providerSubscriptionRef = client.createInstance(
            PEGASUS_NAMESPACENAME_INTEROP,
            providerSubscriptionInstance);
    }
    catch (CIMException& ce)
    {
        if (ce.getCode() != CIM_ERR_ALREADY_EXISTS)
            throw;

        providerSubscriptionRef = CIMObjectPath(providerSubscriptionPath);
    }

  }
  catch (Exception& e)
  {
      PEGASUS_STD(cerr) << "Error in createSubscriptions " <<
          e.getMessage() << PEGASUS_STD(endl);
      throw;
  }
}

/*
   Tests the globalization support for indications.
*/
static void TestLocalizedIndications( CIMClient& client,
                      Boolean activeTest,
                      Boolean verboseTest,
                      Boolean skipListener,
                      String listenerHost)
{
  const CIMNamespaceName TEST_NAMESPACE =
      CIMNamespaceName ("test/TestProvider");
  const CIMName GEN_METHOD = CIMName("generateIndication");
  const CIMName RESULT_METHOD = CIMName("getConsumerStatus");
  const CIMObjectPath REFERENCE =
      CIMObjectPath("Test_LocalizedProviderClass.Identifier=0");

  if (!activeTest)
  {
    cout << "Active tests are disabled. Nothing to do for this test." << endl;
    return;
  }

  try
  {
    //
    // TEST 1 - Cause the LocalizedProvider to send an indication to a
    // CIMListener in this process.
    //
    // When the indication arrives then check that the UTF-16 chars in the
    // indication properties were received, and that the Content-Language header
    // in the Export message was received.
    //

    cout << endl
     << "INDICATION TEST 1: Send an indication containing UTF-16 to a"
            " CIMListener"
     << endl;

    // Build the destination path to this program as the CIMListener
    String dest("http://");
    dest.append(listenerHost);
    dest.append("/g11ntest");

    // Get the subscriptions set up to send the indications
    CIMObjectPath g11ntestHandlerRef;
    CIMObjectPath providerHandlerRef;
    CIMObjectPath filterRef;
    CIMObjectPath g11ntestSubscriptionRef;
    CIMObjectPath providerSubscriptionRef;
    createSubscriptions(client,
            g11ntestHandlerRef,
            providerHandlerRef,
            filterRef,
            g11ntestSubscriptionRef,
            providerSubscriptionRef,
            dest,
            skipListener,
            verboseTest);

    // Construct our CIMListener in case we use it.
    Boolean listenerError = false;

    Uint32 portNumber = 2003;
    HostLocator addr(listenerHost);
    if (addr.isPortSpecified())
    {
        portNumber = addr.getPort();
    }

    CIMListener listener(portNumber);

    // Add our comsumer
    MyIndicationConsumer* consumer1 = new MyIndicationConsumer("1");
    listener.addConsumer(consumer1);

    // If we are running the listener test, then finish starting the CIMListener
    if (!skipListener)
    {
      try
    {
      if (verboseTest)
        cout << "Starting the CIMListener at destination " << dest << endl;

      // Start the listener
      listener.start();
    }
      catch (const BindFailedException&)
    {
      // Got a bind error.  The port is probably already in use.
      // Put out a message and keep going.
      // NOTE - pegasus_acceptor (part of monitor_2) does not appear
      // to throw this exception on bind errors.  So, when monitor_2
      // is used, we will get the timeout below rather than a bind error.
      cerr << endl << "==>WARNING: unable to bind to listener port "
          << portNumber << endl;
      cerr << "The listener port may be in use." << endl;
      cerr << "Skipping the CIMListener tests." << endl;
      listenerError = true;
    }
    }
    else
    {
      cout << "skiplistener option is true. Skipping the CIMListener tests."
          << endl;
      listenerError = true;   // forces the validation to be skipped.
    }

    // Cause the indication.
    // Note: because of the subscriptions, the indication will go to the
    // CIMListener (if enabled) and the CIMIndicationConsumerProvider.
    if (verboseTest)
      cout << "Causing the indication" << endl;

    // Set the language objects to be sent to the provider
    // This is required by the provider
    AcceptLanguageList AL_DE;
    AL_DE.insert(LanguageTag("de"), Real32(0.8));
    ContentLanguageList CL_DE;
    CL_DE.append(LanguageTag("de"));
    client.setRequestAcceptLanguages(AL_DE);
    client.setRequestContentLanguages(CL_DE);

    Array<CIMParamValue> inParams;
    Array<CIMParamValue> outParams;
    CIMValue rtnVal = client.invokeMethod(
        TEST_NAMESPACE,
        REFERENCE,
        GEN_METHOD,
        inParams,
        outParams);

    // Block until the indication comes in (let the user know we are waiting)
    if (!listenerError)
    {
      cout << "Waiting to receive the indication (timeout is 10sec)" << endl;

    if (!indicationReceived.time_wait(10 * 1000))
    {
        throw Exception("Timed out waiting for indication.");
    }

    cout << "Received the indication" << endl;

    // Verify that the UTF-16  properties in the indication got to our
    // CIMListener
    if (verboseTest)
      cout << "Checking the indication for valid utf-16 chars" << endl;

    Uint32 pos = indication.findProperty("UnicodeStr");
    MYASSERT(pos != (Uint32)PEG_NOT_FOUND);

    CIMValue val = indication.getProperty(pos).getValue();
    String utf16;
    val.get(utf16);
    MYASSERT(utf16 == String(utf16Chars));

    pos = indication.findProperty("UnicodeChar");
    MYASSERT(pos != (Uint32)PEG_NOT_FOUND);

    val = indication.getProperty(pos).getValue();
    Char16 char16;
    val.get(char16);
    MYASSERT(char16 == utf16Chars[0]);

    // Verify that the Content-Language of the indication got to us
    ContentLanguageList expectedCL;
    expectedCL.append(LanguageTag("x-world"));

    if (verboseTest)
      cout << "Checking the indication for ContentLanguageList = "
           << LanguageParser::buildContentLanguageHeader(expectedCL) << endl;

    ContentLanguageListContainer cntr =
      indicationContext.get(ContentLanguageListContainer::NAME);
    ContentLanguageList cl = cntr.getLanguages();
    MYASSERT(cl == expectedCL);
    }  // endif !skiplistener

    // Clean up the listener
    if (verboseTest && !skipListener)
      cout << "Stopping the listener"  << endl;

    listener.stop();
    listener.removeConsumer(consumer1);
    delete consumer1;

    //
    // TEST 2 - Cause the LocalizedProvider to send an indication to a
    // CIMIndicationConsumerProvider
    //
    // The consumer provider checks that the UTF-16 chars in the
    // indication properties were received, and that the Content-Language header
    // in the Export message was received.  This program calls a method on
    // the provider to get the test result.
    //
    // Note:  LocalizedProvider is the consumer in this test.  So the provider
    // is sending an indication to itself.  However, the destination in the
    // handler is
    // set up so that the indication still goes through the TCP/IP stack.
    // (this is done by setting the host:port to localhost:5988)
    //

    cout << endl
     << "INDICATION TEST 2: Send an indication containing UTF-16 "
            "to a CIMIndicationConsumerProvider"
     << endl;


    // Verify that the indication got to the CIMIndicationConsumerProvider
    if (verboseTest)
      cout << "Getting the status from the CIMIndicationConsumerProvider"
          << endl;

    Threads::sleep(3000);  // give the indication 3 sec to get to the consumer
    rtnVal = client.invokeMethod(
        TEST_NAMESPACE,
        REFERENCE,
        RESULT_METHOD,
        inParams,
        outParams);

    if (verboseTest)
      cout << "Checking the status from the CIMIndicationConsumerProvider"
          << endl;

    Uint8 result;
    rtnVal.get(result);
    switch (result)
    {
    case 0:
      break;
    case 1:
      throw Exception("CIMIndicationConsumer did not receive the indication");
      break;
    case 2:
      throw Exception(
          "CIMIndicationConsumer did not receive the UTF-16 string");
      break;
    case 3:
      throw Exception(
          "CIMIndicationConsumer did not receive the correct value in"
            " UTF-16 string");
      break;
    case 4:
      throw Exception(
          "CIMIndicationConsumer did not receive the UTF-16 character");
      break;
    case 5:
      throw Exception("CIMIndicationConsumer did not receive the correct"
                            " value in UTF-16 character");
      break;
    case 6:
      throw Exception("CIMIndicationConsumer did not receive the correct"
                            " content languages");
      break;
    default:
      throw Exception("CIMIndicationConsumer sent an unknown status value");
      break;
    }

    // Clean up the repository
    if (verboseTest)
      cout << "Removing the handlers, filters,and subscriptions"  << endl;

    if (!skipListener)
      client.deleteInstance(PEGASUS_NAMESPACENAME_INTEROP,
                            g11ntestSubscriptionRef);
    client.deleteInstance(PEGASUS_NAMESPACENAME_INTEROP,
                          providerSubscriptionRef);
    client.deleteInstance(PEGASUS_NAMESPACENAME_INTEROP, filterRef);
    if (!skipListener)
      client.deleteInstance(PEGASUS_NAMESPACENAME_INTEROP, g11ntestHandlerRef);
    client.deleteInstance(PEGASUS_NAMESPACENAME_INTEROP, providerHandlerRef);

    if (verboseTest)
      cout << "Indication test completed successfully " << endl;
  }
  catch (Exception& e)
  {
    PEGASUS_STD(cerr) << "Error in TestLocalizedIndications: " <<
      e.getMessage() << PEGASUS_STD(endl);
    throw;
  }
}


// This function is used to control the default message loading in the server.
// When make poststarttests is run with ICU enabled, the
// PEGASUS_USE_DEFAULT_MESSAGES
// env var is set.  This env var causes the server to return default messages
// even though ICU is used.  This is needed to pass the wbemexec testcases that
// expect the default messages.  However, when this program is run with ICU
// enabled, messages from the resource bundles are expected from the server.
// This function controls whether the MessageLoader in the server loads from
// the resource bundles.
Boolean setServerDefaultMessageLoading(CIMClient & client,
                       Boolean newSetting)
{
    const CIMNamespaceName TEST_NAMESPACE =
        CIMNamespaceName ("test/TestProvider");
    const CIMName SETTER_METHOD = CIMName("setDefaultMessageLoading");
    const CIMObjectPath REFERENCE =
        CIMObjectPath("Test_LocalizedProviderClass.Identifier=0");

    // Set the language objects to be sent to the LocalizedProvider
    // This is required by the provider
    AcceptLanguageList AL_DE;
    AL_DE.insert(LanguageTag("de"), Real32(0.8));
    ContentLanguageList CL_DE;
    CL_DE.append(LanguageTag("de"));
    client.setRequestAcceptLanguages(AL_DE);
    client.setRequestContentLanguages(CL_DE);

    // Set the new default message loading value
    Array<CIMParamValue> inParams;
    inParams.append( CIMParamValue(  "newSetting", CIMValue( newSetting ) ) );
    Array<CIMParamValue> outParams;
    CIMValue rtnVal = client.invokeMethod(
       TEST_NAMESPACE,
       REFERENCE,
       SETTER_METHOD,
       inParams,
       outParams);

    // The old value is returned by the provider
    Boolean oldSetting;
    rtnVal.get(oldSetting);
    return oldSetting;
}

// l10n end


///////////////////////////////////////////////////////////////
//    OPTION MANAGEMENT
///////////////////////////////////////////////////////////////

/** GetOptions function - This function defines the Options Table
    and sets up the options from that table using the option manager.
    const char* optionName;
    const char* defaultValue;
    int required;
    Option::Type type;
    char** domain;
    Uint32 domainSize;
    const char* commandLineOptionName;
    const char* optionHelpMessage;

*/
void GetOptions(
    OptionManager& om,
    int& argc,
    char** argv,
    const String& pegasusHome)
{
    static struct OptionRow optionsTable[] =
        //     optionname defaultvalue rqd  type domain domainsize clname hlpmsg
    {
         {"repeat", "1", false, Option::WHOLE_NUMBER, 0, 0, "r",
                       "Specifies a Repeat Count Entire test repeated this"
                           " many times" },

         {"version", "false", false, Option::BOOLEAN, 0, 0, "version",
                         "Displays TestClient Version "},

         {"verbose", "false", false, Option::BOOLEAN, 0, 0, "verbose",
                         "If set, outputs extra information "},

         {"help", "false", false, Option::BOOLEAN, 0, 0, "h",
                     "Prints help message with command line options "},

         {"debug", "false", false, Option::BOOLEAN, 0, 0, "d",
                      "Not Used "},

         {"local", "false", false, Option::BOOLEAN, 0, 0, "local",
                         "Use local connection mechanism"},

         {"user", "", false, Option::STRING, 0, 0, "user",
                         "Specifies user name" },

         {"password", "", false, Option::STRING, 0, 0, "password",
                         "Specifies password" },

         {"reqlang", "", false, Option::STRING, 0, 0, "reqlang",
                 "Specifies the language list (in RFC 2616 Accept-Language"
                     " form) to request for messages from the server" },

         {"rsplang", "", false, Option::STRING, 0, 0, "rsplang",
                 "Specifies the language expected to be returned by"
                     " the server"},

         {"skipactive", "false", false, Option::BOOLEAN, 0, 0, "skipactive",
                  "If set then skips tests that modify CIM Objects on"
                      " the server" },

         {"skiplistener", "false", false, Option::BOOLEAN, 0, 0, "skiplistener",
                  "If set then skips the CIMListener tests" },

         {"listenerhost", "localhost:2003", false, Option::STRING, 0, 0, "l",
                       "Specifies the listener host:port for the"
                           " CIMListener tests" },

         {"utfrep", "false", false, Option::BOOLEAN, 0, 0, "utfrep",
                  "If set then use class/qualifier/namepaces names with"
                      " UTF-16 in the repository tests." }
    };
    const Uint32 NUM_OPTIONS = sizeof(optionsTable) / sizeof(optionsTable[0]);

    om.registerOptions(optionsTable, NUM_OPTIONS);

    String configFile = pegasusHome + "/cimserver.conf";

    cout << "Config file from " << configFile << endl;

    if (FileSystem::exists(configFile))
         om.mergeFile(configFile);

    om.mergeCommandLine(argc, argv);

    om.checkRequiredOptions();
}


///////////////////////////////////////////////////////////////
//    MAIN
///////////////////////////////////////////////////////////////

int main(int argc, char** argv)
{
    // Get environment variables:

    String pegasusHome;
    pegasusHome = "/";
    // GetEnvironmentVariables(argv[0], pegasusHome);

    // Get options (from command line and from configuration file); this
    // removes corresponding options and their arguments fromt he command
    // line.

    // Get options (from command line and from configuration file); this
    // removes corresponding options and their arguments fromt he command
    // line.

    OptionManager om;

    try
    {
         GetOptions(om, argc, argv, pegasusHome);
    }
    catch (Exception& e)
    {
         cerr << argv[0] << ": " << e.getMessage() << endl;
         exit(1);
    }

    // Check to see if user asked for help (-h otpion):
    if (om.valueEquals("help", "true"))
    {
        String header = "Usage ";
        header.append(argv[0]);
        header.append(" -parameters host [host]");

        String trailer = "Assumes localhost:5988 if host not specified";
        trailer.append("\nHost may be of the form name or name:port");
        trailer.append("\nPort 5988 assumed if port number missing.");
        om.printOptionsHelpTxt(header, trailer);

        exit(0);
    }

    String reqLang;
    om.lookupValue("reqlang", reqLang);
    if (reqLang != String::EMPTY)
    {
       // User is requesting translated msgs from the server
       skipICU = false;
    }
    else
    {
       // User is not requesting translated msgs from the server.
       skipICU = true;
    }
    cout << "Request Language = " << reqLang << endl;

    String rspLang;
    om.lookupValue("rsplang", rspLang);
    cout << "Expected Response Language = " << rspLang << endl;

    if (om.valueEquals("version","true"))
    {
        cout << argv[0] << " version " <<programVersion <<  endl;
        //cout << argv[0] << endl;
        exit(0);
    }

    // Setup the active test flag.  Determines if we change repository.
    Boolean activeTest = true;
    if (om.valueEquals("skipactive", "true"))
        activeTest = false;

    // Setup the flag to decide whether to use UTF-16 in the classes
    // and qualifiers created in the repository tests.
    Boolean utfRepNames = false;
    if (om.valueEquals("utfrep", "true"))
        utfRepNames = true;

    String userName;
    om.lookupValue("user", userName);
    if (userName != String::EMPTY)
    {
       cout << "Username = " << userName << endl;
    }

    Boolean verboseTest = om.isTrue("verbose");

    String password;
    om.lookupValue("password", password);
    if (password != String::EMPTY)
    {
       cout << "password = " << password << endl;
    }

    // Set up number of test repetitions.
    // Will repeat entire test this number of times
    // Default is zero
    // String repeats;
    Uint32 repeatTestCount = 0;
    /* ATTN: KS P0 Test and fix function added to Option Manager
    */
    if (!om.lookupIntegerValue("repeat", repeatTestCount))
        repeatTestCount = 1;

    // Settings for the indication tests
    String listenerHost;
    om.lookupValue("listenerhost",listenerHost);
    cout << "Listener Host:Port = " << listenerHost << endl;
    Boolean skipListener = false;
    if (om.valueEquals("skiplistener", "true"))
        skipListener = true;

    if(verboseTest)
        cout << "Test repeat count " << repeatTestCount << endl;

    Boolean localConnection = (om.valueEquals("local", "true"))? true: false;
    cout << "localConnection " << (localConnection ? "true" : "false") << endl;

    Array<String> connectionList;
    if (argc > 1 && !localConnection)
         for (Sint32 i = 1; i < argc; i++)
             connectionList.append(argv[i]);

    if(argc < 2)
        connectionList.append("localhost:5988");

    // Expand host to add port if not defined

    // Show the connectionlist
    cout << "Connection List size " << connectionList.size() << endl;
    for (Uint32 i = 0; i < connectionList.size(); i++)
    cout << "Connection " << i << " address " << connectionList[i] << endl;

    cout << "==========START GLOBALIZATION TEST============" << endl;

    for(Uint32 numTests = 1; numTests <= repeatTestCount; numTests++)
    {
        cout << "Test Repetition # " << numTests << endl;
        for (Uint32 i = 0; i < connectionList.size(); i++)
        {
            cout << "Start Try Block" << endl;
            try
            {
          cout << "Set Stopwatch" << endl;
          Stopwatch elapsedTime;
          cout << "Create client" << endl;
          CIMClient client;
          client.setTimeout(360 * 1000);
          cout << "Client created" << endl;

          //
          //  Get host and port number from connection list entry
          //
          HostLocator addr(connectionList[i]);
          String host = addr.getHost();
          Uint32 portNumber = 0;
          if (addr.isPortSpecified())
        {
              portNumber = addr.getPort();
        }

          if (om.isTrue("local"))
        {
          cout << "Using local connection mechanism " << endl;
          client.connectLocal();
                }
          else
        {
          cout << "Connecting to " << connectionList[i] << endl;
          client.connect (host, portNumber,
                                   userName, password);
        }
          cout << "Client Connected" << endl;

          if (verboseTest)
        cout << "Disabling default message loading in the server" << endl;
          Boolean oldSetting = setServerDefaultMessageLoading(client,
                                  false);

          try
              {
        testStart("Test Server Error Messages");
        elapsedTime.reset();
        elapsedTime.start();
        TestServerMessages(client, reqLang, rspLang, activeTest, verboseTest);
        elapsedTime.stop();
        testEnd(elapsedTime.getElapsed());

        testStart("Test Instance Operations");
        elapsedTime.reset();
        elapsedTime.start();
        TestLocalizedInstances(client, activeTest, verboseTest);
        elapsedTime.stop();
        testEnd(elapsedTime.getElapsed());

        testStart("Test Method Operations");
        elapsedTime.reset();
        elapsedTime.start();
        TestLocalizedMethods(client, activeTest, verboseTest);
        elapsedTime.stop();
        testEnd(elapsedTime.getElapsed());

        testStart("Test Class, Qualifier, and Namespace Operations");
        elapsedTime.reset();
        elapsedTime.start();
        TestUTFRepository(client, utfRepNames, activeTest, verboseTest);
        elapsedTime.stop();
        testEnd(elapsedTime.getElapsed());

        testStart("Test Indication Operations");
        elapsedTime.reset();
        elapsedTime.start();
        TestLocalizedIndications(client, activeTest, verboseTest,
                          skipListener ,listenerHost);
        elapsedTime.stop();
        testEnd(elapsedTime.getElapsed());
          }
          catch (const Exception&)
          {
        // Restore the default message loading value before we leave
        setServerDefaultMessageLoading(client, oldSetting);
        throw;
          }

          if (verboseTest)
        cout << "Restoring the old default message loading value" << endl;
          setServerDefaultMessageLoading(client, oldSetting);

          client.disconnect();
            }
            catch(Exception& e)
            {
          PEGASUS_STD(cerr) << "Error: " << e.getMessage() <<
          PEGASUS_STD(endl);
          exit(1);
            }
        }
    }

    PEGASUS_STD(cout) << "+++++ "<< argv[0] << " Terminated Normally"
        << PEGASUS_STD(endl);
    return 0;
}


