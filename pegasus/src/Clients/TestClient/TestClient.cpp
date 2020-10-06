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

#include <Pegasus/Common/Thread.h>
#include <Pegasus/Common/Config.h>
#include <Pegasus/Common/Constants.h>
#include <Pegasus/Common/PegasusVersion.h>
#include <Pegasus/Common/PegasusAssert.h>
#include <Pegasus/Common/CIMName.h>
#include <Pegasus/Common/FileSystem.h>
#include <Pegasus/Common/Exception.h>
#include <Pegasus/Common/XmlWriter.h>
#include <Pegasus/Common/AutoPtr.h>
#include <Pegasus/Common/HostLocator.h>

#include <Pegasus/Client/CIMClient.h>

#include <Pegasus/General/OptionManager.h>
#include <Pegasus/General/Stopwatch.h>

/**
  Here, exit(2) is being used to signal a timeout exception,
  and exit(1) to any other exception, everywhere an exception
  is required to exit.

  The exit(0), is being used to signal that TestClient
  terminated normally
*/

PEGASUS_USING_PEGASUS;
PEGASUS_USING_STD;

CIMNamespaceName globalNamespace = CIMNamespaceName ("root/cimv2");
static const CIMNamespaceName __NAMESPACE_NAMESPACE = CIMNamespaceName ("root");

static const char* programVersion =  "2.0";

static AtomicInt errorCount(0);

/**
  Thread Parameters Class
*/
class T_Parms
{
public:
    CIMClient* client;
    int verboseTest;
    int activeTest;
    int testCount;
    int uniqueID;
    String ignoreErrors;
};

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


/*****************************************************************
//    Testing Namespace operations
******************************************************************/

static void TestNameSpaceOperations(CIMClient* client, Boolean activeTest,
                 Boolean verboseTest, String uniqueID)
{
    // Get all namespaces for display using the __Namespaces function.
    CIMName className = "__NameSpace";
    Array<CIMNamespaceName> namespaceNames;

    // Build the namespaces incrementally starting at the root
    // ATTN: 20030319 KS today we start with the "root" directory but this is
    // wrong. We should be starting with null (no directory) but today we get
    // an xml error return in Pegasus returned for this call. Note that the
    // specification requires that the root namespace be used
    // when __namespace is defined but does not require that it be the root
    // for all namespaces. That is a hole in the spec, not in our code.
    namespaceNames.append("root");
    Uint32 start = 0;
    Uint32 end = namespaceNames.size();
    do
    {
        // for all new elements in the output array
        for (Uint32 range = start; range < end; range ++)
        {
            // Get the next increment in naming for all a name element in the
            // array
            Array<CIMInstance> instances =
                client->enumerateInstances(namespaceNames[range], className);
            for (Uint32 i = 0 ; i < instances.size(); i++)
            {
                Uint32 pos;
                // if we find the property and it is a string, use it.
                if ((pos = instances[i].findProperty("name")) != PEG_NOT_FOUND)
                {
                    CIMValue value;
                    String namespaceComponent;
                    value = instances[i].getProperty(pos).getValue();
                    if (value.getType() == CIMTYPE_STRING)
                    {
                        value.get(namespaceComponent);
                        String ns = namespaceNames[range].getString();
                        ns.append("/");
                        ns.append(namespaceComponent);
                        namespaceNames.append(ns);
                    }
                }
            }
            start = end;
            end = namespaceNames.size();
        }
    }
    while (start != end);
    // Validate that all of the returned entities are really namespaces.
    // It is legal for us to have an name component that is really not a
    // namespace (ex. root/fred/john is a namespace
    // but root/fred is not.
    // There is no clearly defined test for this so we will simply try to get
    // something, in this case a well known association
    Array<CIMNamespaceName> returnNamespaces;

    for (Uint32 i = 0 ; i < namespaceNames.size() ; i++)
    {
        try
        {
            CIMQualifierDecl cimQualifierDecl;
            cimQualifierDecl = client->getQualifier(namespaceNames[i],
                                           "Association");

            returnNamespaces.append(namespaceNames[i]);
        }
        catch(CIMException& e)
        {
            if (e.getCode() != CIM_ERR_INVALID_NAMESPACE)
                returnNamespaces.append(namespaceNames[i]);
        }
    }

    cout << returnNamespaces.size() << " namespaces " << " returned." << endl;
    for( Uint32 cnt = 0 ; cnt < returnNamespaces.size(); cnt++ )
    {
        cout << returnNamespaces[cnt].getString() << endl;;
    }

    // ATTN: The following code is probably no good. KS April 2003
    // If conducting active test, try to create and delete a namespace.
    if(activeTest)
    {
    String testNamespace = uniqueID.append("_Namespace");
        if(verboseTest)
            cout << "Conducting Create / Delete namespace test " << endl;

        // Build the instance name for __namespace
        CIMNamespaceName testNamespaceName = CIMNamespaceName (testNamespace);
        String instanceName = className.getString();
        instanceName.append( ".Name=\"");
        instanceName.append(testNamespaceName.getString());
        instanceName.append("\"");

        if(verboseTest)
        {
            cout << "Creating " << instanceName << endl;
        }
        //if(testNamespace)      Add the test for existance code here ATTN
        //{
        //    instanceNames
        //}
        // Create the new instance

        CIMObjectPath newInstanceName;
        try
        {
                // Build the new instance
        CIMName name = "__NameSpace";
            //CIMInstance newInstance(instanceName);
        CIMInstance newInstance(name);
            newInstance.addProperty(
                CIMProperty(CIMName ("name"), testNamespaceName.getString()));
            newInstanceName =
                client->createInstance(__NAMESPACE_NAMESPACE, newInstance);
        }
        catch(CIMException& e)
        {
             if (e.getCode() == CIM_ERR_ALREADY_EXISTS)
             {
                   newInstanceName = CIMObjectPath(instanceName);
             }
             else
             {
                  PEGASUS_STD(cerr) << "CIMException NameSpace Creation: "
                      << e.getMessage() << " Creating " << instanceName
                      << PEGASUS_STD(endl);
                  errorCount++;
                  return;
             }
        }
        catch(Exception& e)
        {
            PEGASUS_STD(cerr) << "Exception NameSpace Creation: "
                << e.getMessage() << PEGASUS_STD(endl);
            errorCount++;
            return;
        }
            // Now try getting the instance.
        try
        {
            client->getInstance(__NAMESPACE_NAMESPACE, newInstanceName);
        }
        catch(Exception& e)
        {
             PEGASUS_STD(cerr) << "Exception NameSpace getInstance: "
                << e.getMessage() << " Retrieving " << instanceName
                << PEGASUS_STD(endl);
             errorCount++;
             return;
        }

        // Now delete the namespace
        try
        {
            CIMObjectPath myReference(instanceName);
            if(verboseTest)
                    cout << "Deleting namespace = " << instanceName << endl;

           client->deleteInstance(__NAMESPACE_NAMESPACE, myReference);
        }
        catch(Exception& e)
        {
             PEGASUS_STD(cerr) << "Exception NameSpace Deletion: "
                << e.getMessage() << " Deleting " << instanceName
                    << PEGASUS_STD(endl);
             errorCount++;
             return;
        }
    }
}


static void TestEnumerateClassNames (CIMClient* client, Boolean activeTest,
                 Boolean verboseTest, String uniqueID)
{
    try
    {
         Boolean deepInheritance = true;
         CIMName className;
         Array<CIMName> classNames = client->enumerateClassNames(
             globalNamespace, className, deepInheritance);

         if (verboseTest)
         {
             for (Uint32 i = 0, n = classNames.size(); i < n; i++)
                 cout << classNames[i].getString() << endl;
         }

         cout << classNames.size() << " ClassNames" << endl;
    }
    catch(Exception& e)
    {
         cout << "Error NameSpace Enumeration:" << endl;
         cout << e.getMessage() << endl;
         errorCount++;
    }

}

/* This is both an active and passive class test and so uses the
    activetest variable
*/
static void TestClassOperations(CIMClient* client, Boolean ActiveTest,
                             Boolean verboseTest, String uniqueID) {

    // Name of Class to use in create/delete test
    String cimName = uniqueID.append("_").append("PEG_TestSubClass");
    CIMName testClass(cimName);
    // NOte that this creates a subclass of ManagedElement so will fail if
    // if managedelement not intalled.

    //Test for class already existing
    Array<CIMName> classNames = client->enumerateClassNames(
         globalNamespace, CIMName ("CIM_ManagedElement"), false);
    if (ActiveTest)
    {
        if (verboseTest)
        {
            cout << "Test to create, modify and delete test class "
                << testClass.getString() << endl;
        }
        for (Uint32 i = 0; i < classNames.size(); i++)
        {
             if (classNames[i].equal(testClass))
                 try
                 {
                 client->deleteClass(globalNamespace, testClass);
                 }
                catch (CIMException& e)
                {
                        cout << "TestClass " << testClass.getString()
                            << " delete failed "
                            << e.getMessage() << endl;
                        errorCount++;
                        return;
                }
        }
        // CreateClass:

        CIMClass c1(testClass, CIMName ("CIM_ManagedElement"));
        c1.addProperty(CIMProperty(CIMName ("count"), Uint32(99)));
        c1.addProperty(CIMProperty(CIMName ("ratio"), Real64(66.66)));
        c1.addProperty(CIMProperty(CIMName ("message"), String("Hello World")));

        try
        {
            client->createClass(globalNamespace, c1);
        }
        catch (CIMException& e)
        {
            CIMStatusCode code = e.getCode();
            if (code == CIM_ERR_ALREADY_EXISTS)
                cout << "TestClass " << testClass.getString()
                    << " already exists during create " << endl;
            else
            {
                cout << "TestClass " << testClass.getString()
                    << " create failed "
                    <<  e.getMessage() << endl;
                errorCount++;
                return;
            }
        }

        // GetClass:

        CIMClass c2 = client->getClass(globalNamespace, testClass, true);
        if (!c1.identical(c2))
        {
        cout << "Class SubClass Returned not equal to created" << endl;
        }
        // ATTN: This test should be uncommented when the repository implements
        // the localOnly flag.
        //PEGASUS_TEST_ASSERT(c1.identical(c2));

        // Modify the class:
        c2.removeProperty(c2.findProperty(CIMName ("message")));
        try
        {
            client->modifyClass(globalNamespace, c2);
        }
        catch (CIMException& e)
        {
            cout << "Testclass Modification failed " << e.getMessage() << endl;
            errorCount++;
            return;
        }
        // GetClass:
        CIMClass c3 = client->getClass(globalNamespace, testClass, true);

        if (!c3.identical(c2))
        {
        cout << "Test Failed. Rtned class c3 not equal to c2" << endl;
            errorCount++;
            return;
        }
        //PEGASUS_TEST_ASSERT(c3.identical(c2));

        // Determine if the new Class exists in Enumerate

        classNames = client->enumerateClassNames(
             globalNamespace, CIMName ("CIM_ManagedElement"), false);

        Boolean found = false;

        for (Uint32 i = 0; i < classNames.size(); i++)
        {
             if (classNames[i].equal(testClass))
                 found = true;
        }
        if (!found)
        {
            cout << "Test Class " << testClass.getString()
                << " Not found in enumeration "
                << endl;
            errorCount++;
            return;
        }
        //PEGASUS_TEST_ASSERT(found);

        // DeleteClass:
        try
        {
            client->deleteClass(globalNamespace, testClass);
        }
        catch (CIMException& e)
        {
            cout << "Testclass delete failed " << e.getMessage() << endl;
            errorCount++;
            return;
        }
    }
    // Get all the classes and compare enum names with enum classes from before

    classNames = client->enumerateClassNames(globalNamespace, CIMName(), false);
    Array<CIMClass> classDecls = client->enumerateClasses(
         globalNamespace, CIMName(), false, false, true, true);
    //PEGASUS_TEST_ASSERT(classDecls.size() == classNames.size());
    if (classDecls.size() != classNames.size())
    {
        cout << "Error: Class total count before and after test. Before = " <<
            classNames.size() << ", after = " <<  classDecls.size() << endl;
        errorCount++;
        return;
    }
    for (Uint32 i = 0; i < classNames.size(); i++)
    {
         CIMClass tmp = client->getClass(
             globalNamespace, classNames[i], false, true, true);

         PEGASUS_TEST_ASSERT(classNames[i].equal(classDecls[i].getClassName()));

         PEGASUS_TEST_ASSERT(tmp.identical(classDecls[i]));
    }
}

static void TestQualifierOperations(CIMClient* client, Boolean activeTest,
                         Boolean verboseTest, String uniqueID) {

    Array<CIMQualifierDecl> qualifierDecls =
        client->enumerateQualifiers(globalNamespace);

    if (verboseTest)
    {
         for (Uint32 i = 0; i < qualifierDecls.size(); i++)
         {
             cout << qualifierDecls[i].getName().getString() << endl;
         }

    }
    cout << qualifierDecls.size() << " Qualifiers" <<endl;

    if (activeTest)
    {
        // create unique qualifiers
        String prefix = uniqueID;
        CIMName qd1_name = prefix.append("_Q1");
        CIMName qd2_name = prefix.append("_Q2");

         // Create two qualifier declarations:

         CIMQualifierDecl qd1(qd1_name, false, CIMScope::CLASS,
                     CIMFlavor::TOSUBCLASS);
         client->setQualifier(globalNamespace, qd1);

         CIMQualifierDecl qd2(qd2_name, String("Hello"),
                     CIMScope::PROPERTY + CIMScope::CLASS,
             CIMFlavor::OVERRIDABLE);
         client->setQualifier(globalNamespace, qd2);

         // Get them and compare:

         CIMQualifierDecl tmp1 =
             client->getQualifier(globalNamespace, qd1_name);
         PEGASUS_TEST_ASSERT(tmp1.identical(qd1));

         CIMQualifierDecl tmp2 =
             client->getQualifier(globalNamespace, qd2_name);
         PEGASUS_TEST_ASSERT(tmp2.identical(qd2));

         // Enumerate the qualifiers:

         qualifierDecls = client->enumerateQualifiers(globalNamespace);

         for (Uint32 i = 0; i < qualifierDecls.size(); i++)
         {
             CIMQualifierDecl tmp = qualifierDecls[i];

             if (tmp.getName().equal(qd1_name))
                 PEGASUS_TEST_ASSERT(tmp1.identical(tmp));

             if (tmp.getName().equal(qd2_name))
                 PEGASUS_TEST_ASSERT(tmp2.identical(tmp));
         }

         // Delete the qualifiers:

         client->deleteQualifier(globalNamespace, qd1_name);
         client->deleteQualifier(globalNamespace, qd2_name);

    }

}

static Boolean lookupError(const String& ignoreErrors, CIMStatusCode lookupCode)
{
    static const char ERROR_DELIMETER = ',';

    Uint32 startPos = 0;
    Uint32 strSize = ignoreErrors.size();

    while (startPos < strSize)
    {
        Uint32 pos = ignoreErrors.find(startPos, ERROR_DELIMETER);
        if (pos == PEG_NOT_FOUND)
        {
            pos = strSize;
        }

        String errorStr = ignoreErrors.subString(startPos, pos - startPos);
        if (errorStr == cimStatusCodeToString(lookupCode))
        {
            return true;
        }

        startPos = pos + 1;
    }

    return false;
}

static void TestInstanceGetOperations(CIMClient* client, Boolean activeTest,
                                    Boolean verboseTest, String uniqueID,
                                    String ignoreErrors)
{
    // Get all instances
    // Get all classes

    //Array<CIMName> classNames = client.enumerateClassNames(
    //    globalNamespace, "CIM_ManagedElement", false);

    Array<CIMName> classNames = client->enumerateClassNames(
         globalNamespace, CIMName(), true);

    cout <<  classNames.size() << " Classes found " << endl;

    Array<CIMObjectPath> instanceNames;
    Uint16 numberOfNotSupportedClassesFound = 0;

    for (Uint32 i = 0; i < classNames.size(); i++)
    {
       //if (classNames[i] == "PG_ShutdownService")
       //{
       //    // Skip the PG_ObjectManager class.  It currently has no
       //    // instance provider and no instances.
       //}
       //else
       //{
           try
           {
               instanceNames =
                  client->enumerateInstanceNames(globalNamespace,classNames[i]);
               if (instanceNames.size() > 0)
                   cout << "Class " << classNames[i].getString() << " "
                        << instanceNames.size() << " Instances" << endl;
           }
           catch(CIMException& e)
           {
                 if (e.getCode() == CIM_ERR_NOT_SUPPORTED)
                 {
                    numberOfNotSupportedClassesFound++;
                 }
                 else if (lookupError(ignoreErrors, e.getCode()))
                 {
                     cerr << "CIMException : " << classNames[i].getString()
                          << endl;
                     cerr << e.getMessage() << endl;
                 }
                 else
                 {
                    cerr << "CIMException : " << classNames[i].getString()
                         << endl;
                    cerr << e.getMessage() << endl;
                    errorCount++;
                    return;
                 }
           }
       catch(ConnectionTimeoutException& e)
       {
           PEGASUS_STD(cerr) << "Warning: " << e.getMessage()
               << PEGASUS_STD(endl);
           exit(2);
       }
       catch(Exception& e)
       {
           PEGASUS_STD(cerr) << "Error: " << e.getMessage()
               << PEGASUS_STD(endl);
           exit(1);
       }
       //}
    }
    if (numberOfNotSupportedClassesFound > 0)
    {
       cout << "Number of Not Supported Classes Found = "
            << numberOfNotSupportedClassesFound << endl;
    }
    /*
    virtual Array<CIMObjectPath> enumerateInstanceNames(
        const CIMNamespaceName& nameSpace,
        const CIMName& className) = 0;

    virtual Array<CIMInstance> enumerateInstances(
        const CIMNamespaceName& nameSpace,
        const CIMName& className,
        Boolean deepInheritance = true,
        Boolean localOnly = true,
        Boolean includeQualifiers = false,
        Boolean includeClassOrigin = false,
    */

}
static void TestInstanceModifyOperations(CIMClient* client, Boolean
                     activeTest, Boolean verboseTest, String uniqueID)
{
    if (!activeTest)
    {
        cout << "InstanceModify bypassed because it modifies repository."
                    " Set active to execute."
             << endl;
        return;
    }
    // name of class to play with
    String classname = uniqueID.append("_").append("PEG_TEST_LocalCLASS");
    CIMName className(classname);

    // Delete the class if it already exists:
    try
    {
         client->deleteClass(globalNamespace, className);
    }
    catch (Exception&)
    {
         // Ignore delete class!
    }

    // Create a new class:

    CIMClass cimClass(className);
    cimClass
         .addProperty(CIMProperty(CIMName ("last"), String())
             .addQualifier(CIMQualifier(CIMName ("key"), true)))
         .addProperty(CIMProperty(CIMName ("first"), String())
             .addQualifier(CIMQualifier(CIMName ("key"), true)))
         .addProperty(CIMProperty(CIMName ("age"), Uint32(0))
             .addQualifier(CIMQualifier(CIMName ("key"), true)))
         .addProperty(CIMProperty(CIMName ("nick"), String())
              .addQualifier(CIMQualifier(CIMName ("key"), false)));
    client->createClass(globalNamespace, cimClass);

    // Create an instance of that class:
    cout << "Create one Instance of class " << className.getString() << endl;

    CIMInstance cimInstance(className);
    cimInstance.addProperty(CIMProperty(CIMName ("last"), String("Smith")));
    cimInstance.addProperty(CIMProperty(CIMName ("first"), String("John")));
    cimInstance.addProperty(CIMProperty(CIMName ("age"), Uint32(1010)));
    cimInstance.addProperty(CIMProperty(CIMName ("nick"), String("Duke")));
    CIMObjectPath instanceName = cimInstance.buildPath(cimClass);
    client->createInstance(globalNamespace, cimInstance);

    // Get the instance and compare with created one:

    //CIMObjectPath ref;
    //CIMObjectPath::instanceNameToReference(instanceName, ref);
    CIMInstance tmp = client->getInstance(globalNamespace, instanceName);
    // XmlWriter::printInstanceElement(cimInstance);
    // XmlWriter::printInstanceElement(tmp);
    // PEGASUS_TEST_ASSERT(cimInstance.identical(tmp));

    // Test timeout methods

    const Uint32 TEST_TIMEOUT = 10000;
    Uint32 origTimeout = client->getTimeout();
    client->setTimeout( TEST_TIMEOUT );
    Uint32 newTimeout = client->getTimeout();
    PEGASUS_TEST_ASSERT( newTimeout == TEST_TIMEOUT );
    client->setTimeout( origTimeout );

    // Test get/set property methods

    const String TESTPROPVAL    = "JR";
    const String TESTPROPVALNAME = "nick";
    cout << "Set property " << endl;
    const CIMValue testPropVal = CIMValue( TESTPROPVAL );
    client->setProperty( globalNamespace, instanceName, TESTPROPVALNAME,
            testPropVal );
    cout << "Get property " << endl;
    CIMValue returnedPropVal = client->getProperty(  globalNamespace,
                            instanceName,
                            TESTPROPVALNAME );
    PEGASUS_TEST_ASSERT( returnedPropVal == testPropVal );

    // Test modify instance client method
    // Change the "nick" property and compare.

    CIMInstance testInstance =
      client->getInstance(globalNamespace, instanceName);
    Uint32 propertyPos = testInstance.findProperty( TESTPROPVALNAME );
    testInstance.removeProperty( propertyPos );
    CIMProperty nickProperty = CIMProperty( TESTPROPVALNAME, String("Duke") );
    nickProperty.setClassOrigin( className );
    testInstance.addProperty( nickProperty );
    testInstance.setPath (instanceName);
    client->modifyInstance( globalNamespace,
                           testInstance
                           );
    CIMInstance currentInstance =
      client->getInstance( globalNamespace, instanceName );
    currentInstance.setPath( instanceName );
   //PEGASUS_TEST_ASSERT( currentInstance.identical( testInstance ) );

    client->deleteInstance(globalNamespace, instanceName);

    // Repeat to create multiple instances
    Uint32 repeatCount = 30;
    Array<CIMObjectPath> instanceNames;
    cout << "Create " << repeatCount << " Instances" << endl;
    for (Uint32 i = 0; i < repeatCount; i++)
    {
        CIMInstance cimInstance(className);
        cimInstance.addProperty(CIMProperty(CIMName ("last"), String("Smith")));
        cimInstance.addProperty(CIMProperty(CIMName ("first"), String("John")));
        cimInstance.addProperty(CIMProperty(CIMName ("age"), Uint32(i)));
        instanceNames.append( cimInstance.buildPath(cimClass) );
        client->createInstance(globalNamespace, cimInstance);
    }
    cout << "Delete the Instances " << endl;
    for (Uint32 i = 0; i < repeatCount; i++)
    {
        client->deleteInstance(globalNamespace,instanceNames[i]);
    }

    cout << "Delete the Class " << endl;
    client->deleteClass(globalNamespace,className);
}
/* testRefandAssoc - issues a set of reference and association calls
    for the input parameters.
    It does not capture exceptions
*/
static void testRefandAssoc(CIMClient* client, CIMNamespaceName& nameSpace,
        Boolean verboseTest,
        CIMObjectPath& objectName,
        CIMName assocClass,
        CIMName resultClass,
        String role = String::EMPTY,
        String resultRole = String::EMPTY)
{

    Array<CIMObjectPath> result = client->referenceNames(
            nameSpace,
            objectName,
            resultClass,
            role);

    Array<CIMObject> resultObjects = client->references(
            nameSpace,
            objectName,
            resultClass,
            role);

    if (verboseTest)
    {
        cout << "Test references results for class: "
            << objectName.getClassName().getString()
            << " returned " << result.size() << "reference names" << endl;
    }
    if (result.size() != resultObjects.size())
    {
        cout << "ERROR, Reference and reference Name count difference" << endl;
        errorCount++;
        return;
    }
    for (Uint32 i = 0; i < resultObjects.size(); i++)
    {
      Uint32 matched=0;
      for (Uint32 j = 0; j < result.size(); j++)
      {
        if (resultObjects[i].getPath().toString() == result[j].toString())
        {
          matched=1;
          result.remove(j);
          j--;
          break;
        }
      }
      if(matched)
      {
         resultObjects.remove(i);
         i--;
      }
    }

    for(Uint32 i=0;i<result.size(); i++)
    {
       cout << "ReferencesName response Error: "
            << resultObjects[i].getPath().toString()
            << " != "
            << result[i].toString()
            << endl;
    }

    if (result.size())
    {
        errorCount++;
        return;
    }


    Array<CIMObjectPath> assocResult = client->associatorNames(
            nameSpace,
            objectName,
            assocClass,
            resultClass,
            role,
            resultRole);
    Array<CIMObject> assocResultObjects = client->associators(
            nameSpace,
            objectName,
            assocClass,
            resultClass,
            role,
            resultRole);


    if (verboseTest)
    {
        cout << "Test associations results for class: "
            << objectName.getClassName().getString()
            << " returned " << result.size() << "associator names" << endl;
    }
    if (assocResult.size() != assocResultObjects.size())
    {
        cout << "ERROR, Associator and AssociatorName count returned different"
                    " counts "
            << assocResult.size() << " associator name responses and "
            << assocResultObjects.size() << " associator objects " << endl;
    //#define LOCAL_MAX (a , b) (a > b) ?  a : b )
    // Uint32 maxCount = LOCAL_MAX(assocResult.size() ,
    //                             assocResultObjects.size());
        Uint32 maxCount = (assocResult.size() ,assocResultObjects.size() ) ?
      assocResult.size()  : assocResultObjects.size();
        cout << "Max " << maxCount << endl;

        for (Uint32 i = 0 ; i < maxCount ; i ++)
        {
            cout << i << " "
                << ((i < assocResult.size())? assocResult[i].toString() : "")
                << endl;

            cout << i << " "
                << ((i < assocResultObjects.size())?
                        assocResultObjects[i].getPath().toString() : "")
                << endl;
        }
        errorCount++;
        return;
    }


    for (Uint32 i = 0; i < assocResultObjects.size(); i++)
    {
      Uint32 matched=0;
      for (Uint32 j = 0; j < assocResult.size(); j++)
      {
        if (assocResultObjects[i].getPath().toString() ==
                assocResult[j].toString())
        {
           matched=1;
           assocResult.remove(j);
           j--;
           break;
        }
      }
      if(matched)
      {
         assocResultObjects.remove(i);
         i--;
      }
    }

    for (Uint32 i = 0; i <assocResult.size(); i++)
    {
       cout << "Association Name response Error"
            << assocResultObjects[i].getPath().toString()
            << " != "
            << assocResult[i].toString()
            << endl;
    }

    if (assocResult.size())
    {
        errorCount++;
        return;
    }
}
static void TestAssociationOperations(CIMClient* client, Boolean
                     activeTest, Boolean verboseTest, String uniqueID)
{
    CIMNamespaceName nameSpace = "test/TestProvider";
    // If the sample provider class is loaded, this function tests the
    // association functions against the FamilyProvider
    Boolean runTest = true;
    // First test against known name in cim namespace.
    {
        CIMObjectPath o1("CIM_ManagedElement");
        CIMObjectPath o2("CIM_ManagedElement.name=\"karl\"");

        testRefandAssoc(client, globalNamespace, verboseTest, o1,
            CIMName(), CIMName());
        try
        {
            testRefandAssoc(client, globalNamespace, verboseTest,
                o2, CIMName(), CIMName());
        }
        catch(CIMException& e)
        {
            // This test will get a not supported exception in the case where
            // the repository is not the default instance provider.
            // We should not flag this as a test error.
            if (e.getCode() == CIM_ERR_NOT_SUPPORTED)
            {
                cerr << "CIMException : " << e.getMessage() << endl;
            }
        }
    }

    // Now Test to see if the namespace and class exist before
    // continueing the test.
    {
        CIMName className = "TST_Person";
        try
        {
        client->getClass(nameSpace, className);
        }
        catch(CIMException& e)
        {
              if (e.getCode() == CIM_ERR_INVALID_NAMESPACE
                      || e.getCode() == CIM_ERR_INVALID_CLASS)
              {
                 runTest = false;
              }
              else
              {
                 cerr << "CIMException : " << className.getString() << endl;
                 cerr << e.getMessage() << endl;
                 errorCount++;
                 return;
              }
        }
    }
    if (runTest)
    {

        CIMObjectPath TST_PersonClass= CIMObjectPath("TST_Person");
        CIMObjectPath TST_PersonInstance =
            CIMObjectPath( "TST_Person.name=\"Mike\"");
        CIMObjectPath TST_PersonDynamicClass =
            CIMObjectPath("TST_PersonDynamic");
        CIMObjectPath TST_PersonDynamicInstance =
            CIMObjectPath( "TST_PersonDynamic.name=\"Father\"");

        testRefandAssoc(client, nameSpace, verboseTest, TST_PersonClass ,
                CIMName(), CIMName());
        testRefandAssoc(client, nameSpace, verboseTest, TST_PersonInstance ,
                CIMName(), CIMName());
        testRefandAssoc(client, nameSpace, verboseTest,
                TST_PersonDynamicClass , CIMName(), CIMName());
        testRefandAssoc(client, nameSpace, verboseTest,
                TST_PersonDynamicInstance , CIMName(), CIMName());
    }

    return;
}

/*
   Tests the invoke method request via the sample method provider.
*/

static void TestInvokeMethod(
    CIMClient * client,
    Boolean activeTest,
    Boolean verboseTest,
    const String& uniqueID)
{
    const CIMNamespaceName NAMESPACE = CIMNamespaceName("root/SampleProvider");
    const CIMName classname = CIMName("Sample_MethodProviderClass");
    const CIMName methodName = CIMName("SayHello");
    const CIMObjectPath instanceName = CIMObjectPath(
        "Sample_MethodProviderClass.Identifier=1");

    try
    {
        for (Uint32 testCases = 0; testCases < 3; testCases++)
        {
            Array<CIMParamValue> inParams;
            Array<CIMParamValue> outParams;
            CIMParamValue inParam;

            String goodReply;
            String goodOutParam;

            if (testCases == 0)
            {
                // Test case for a parameter with a non-empty value
                inParams.append(
                    CIMParamValue("Name", CIMValue(String("Yoda"))));
                goodReply = "Hello, Yoda!";
                goodOutParam = "From Neverland";
            }
            else if (testCases == 1)
            {
                // Test case for a parameter with an empty value
                inParams.append(CIMParamValue("Name", CIMValue(String(""))));
                goodReply = "Hello";
                goodOutParam = "From Neverland";
            }
            else
            {
                // Test case for a parameter with a null value
                inParams.append(
                    CIMParamValue("Name", CIMValue(CIMTYPE_STRING, false, 0)));
                goodReply = "Hello";
                goodOutParam = "From Neverland";
            }

            Uint32 testRepeat = 10;
            for (Uint32 i = 0; i < testRepeat; i++)   // repeat the test x time
            {
                CIMValue retValue = client->invokeMethod(
                    NAMESPACE,
                    instanceName,
                    methodName,
                    inParams,
                    outParams);

                if (verboseTest)
                {
                    cout << "Output: " << retValue.toString() << endl;
                    for (Uint32 j = 0; j < outParams.size(); j++)
                    {
                        cout << outParams[j].getParameterName() << ": " <<
                            outParams[j].getValue().toString() << endl;
                    }
                }

                PEGASUS_TEST_ASSERT(retValue.toString() == goodReply);
                PEGASUS_TEST_ASSERT(outParams.size() == 1);

                String outParam;
                outParams[0].getValue().get(outParam);
                PEGASUS_TEST_ASSERT(outParam == goodOutParam);
            }
            cout << "Executed " << testRepeat << " methods" << endl;
        }
    }
    catch(ConnectionTimeoutException& e)
    {
        PEGASUS_STD(cerr) << "Warning: " << e.getMessage() << PEGASUS_STD(endl);
        exit(2);
    }
    catch(Exception& e)
    {
        PEGASUS_STD(cerr) << "Error: " << e.getMessage() << PEGASUS_STD(endl);
        exit(1);
    }
}

/*
   Tests the enumerate instances from the sample instance provider.
*/

static void TestEnumerateInstances( CIMClient * client,
                    Boolean activeTest,
                    Boolean verboseTest, String uniqueID )
{

  const CIMNamespaceName NAMESPACE("root/SampleProvider");
  const CIMName CLASSNAME("Sample_InstanceProviderClass");
  const CIMObjectPath INSTANCE0("Sample_InstanceProviderClass.Identifier=1");
  const CIMObjectPath INSTANCE1("Sample_InstanceProviderClass.Identifier=2");
  const CIMObjectPath INSTANCE2("Sample_InstanceProviderClass.Identifier=3");

  try
    {
      const CIMName className = CLASSNAME;
      Boolean deepInheritance = true;
      Boolean localOnly = true;
      Boolean includeQualifiers = false;
      Boolean includeClassOrigin = false;
      Uint32 testRepeat = 10;
      Uint32 numberInstances;

      for (Uint32 i = 0; i < testRepeat; i++)        // repeat the test x time
        {
      Array<CIMInstance> cimNInstances =
        client->enumerateInstances(NAMESPACE,  className, deepInheritance,
                      localOnly,  includeQualifiers,
                      includeClassOrigin );

      cout << "Found " << cimNInstances.size() << " Instances of "
          << className.getString() << endl;
      //ASSERTTEMP(cimNInstances.size() == 3);
      numberInstances =  cimNInstances.size();

      for (Uint32 j = 0; j < cimNInstances.size(); j++)
      {
          CIMObjectPath instanceRef = cimNInstances[j].getPath();

          if((instanceRef != INSTANCE0) &&
             (instanceRef != INSTANCE1) &&
             (instanceRef != INSTANCE2))
          {
              PEGASUS_STD(cerr)
                  << "Error: EnumInstances failed. Instance Name Error"
                  << " Expected instance name = "
                  << INSTANCE0.toString() << " or "
                  << INSTANCE1.toString() << " or "
                  << INSTANCE2.toString()
                  << "received = " << instanceRef.toString()
                  << PEGASUS_STD(endl);

              errorCount++;
              return;
          }
        }
    }
      cout << "Enumerate " << numberInstances << " instances " << testRepeat
          << " times" << endl;
    }
  catch(Exception& e)
    {
      PEGASUS_STD(cerr) << "Error: " << e.getMessage() << PEGASUS_STD(endl);
      errorCount++;
      return;
    }
}

static Boolean verifyCertificate(SSLCertificateInfo &certInfo)
{
    //ATTN-NB-03-05132002: Add code to handle server certificate verification.
    return true;
}

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
         {"active", "false", false, Option::BOOLEAN, 0, 0, "a",
                      "If set allows test that modify the repository" },

         {"repeat", "1", false, Option::WHOLE_NUMBER, 0, 0, "r",
                       "Specifies a Repeat Count Entire test repeated this"
                           " many times" },

         {"namespace", "root/cimv2", false, Option::STRING, 0, 0, "-n",
                         "specifies namespace to use for test" },

         {"version", "false", false, Option::BOOLEAN, 0, 0, "version",
                         "Displays TestClient Version "},

         {"verbose", "false", false, Option::BOOLEAN, 0, 0, "verbose",
                         "If set, outputs extra information "},

         {"help", "false", false, Option::BOOLEAN, 0, 0, "h",
                     "Prints help message with command line options "},
         {"debug", "false", false, Option::BOOLEAN, 0, 0, "d",
                      "Not Used "},
// remove slp        {"slp", "false", false, Option::BOOLEAN, 0, 0, "slp",
//                       "use SLP to find cim servers to test"},
#ifdef PEGASUS_HAS_SSL
         {"ssl", "false", false, Option::BOOLEAN, 0, 0, "ssl",
                         "use SSL"},
#endif
         {"local", "false", false, Option::BOOLEAN, 0, 0, "local",
                         "Use local connection mechanism"},
         {"user", "", false, Option::STRING, 0, 0, "user",
                         "Specifies user name" },

         {"password", "", false, Option::STRING, 0, 0, "password",
                         "Specifies password" },

        {"simultaneous", "1", false, Option::WHOLE_NUMBER, 0, 0, "t",
                         "Number of simultaneous client threads" },

        {"timeout","30000", false, Option::WHOLE_NUMBER, 0, 0, "to",
                        "Client connection timeout value" },

        {"ignoreErrors", "", false, Option::STRING, 0, 0, "ignoreErrors",
                         "Specifies a comma separated list of CIMError status"
                         " code strings that should be ignored." }

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


ThreadReturnType PEGASUS_THREAD_CDECL executeTests(void *parm)
{
    Thread *my_thread = (Thread *)parm;
    AutoPtr<T_Parms> parms((T_Parms *)my_thread->get_parm());
    CIMClient* client = parms->client;
    Uint32 testCount = parms->testCount;
    Boolean verboseTest = parms->verboseTest == 0 ? false : true;
    Boolean activeTest = parms->activeTest == 0 ? false : true;
    String ignoreErrors = parms->ignoreErrors;
    int id = parms->uniqueID;
    char id_[4];
    memset(id_,0x00,sizeof(id_));
    sprintf(id_,"%i",id);
    String uniqueID = "_";
    uniqueID.append(id_);

    for(Uint32 i=0; i<testCount; i++){
    try{
        Stopwatch elapsedTime;

        testStart("Test NameSpace Operations");
        elapsedTime.reset();
        elapsedTime.start();
        TestNameSpaceOperations(client, activeTest, verboseTest, uniqueID);
        elapsedTime.stop();
        testEnd(elapsedTime.getElapsed());

        testStart("Test Qualifier Operations");
        elapsedTime.reset();
        elapsedTime.start();
        TestQualifierOperations(client,activeTest,verboseTest, uniqueID);
        elapsedTime.stop();
        testEnd(elapsedTime.getElapsed());

        testStart("Test EnumerateClassNames");
        elapsedTime.reset();
        elapsedTime.start();
        TestEnumerateClassNames(client,activeTest,verboseTest, uniqueID);
        elapsedTime.stop();
        testEnd(elapsedTime.getElapsed());

        testStart("Test Class Operations");
        elapsedTime.reset();
        elapsedTime.start();
        TestClassOperations(client,activeTest,verboseTest, uniqueID);
        elapsedTime.stop();
        testEnd(elapsedTime.getElapsed());

        testStart("Test Instance Get Operations");
        elapsedTime.reset();
        elapsedTime.start();
        TestInstanceGetOperations(client,activeTest,verboseTest, uniqueID,
            ignoreErrors);
        elapsedTime.stop();
        testEnd(elapsedTime.getElapsed());

        testStart("Test Instance Modification Operations");
        elapsedTime.reset();
        elapsedTime.start();
        TestInstanceModifyOperations(client, activeTest, verboseTest, uniqueID);
        elapsedTime.stop();
        testEnd(elapsedTime.getElapsed());

        testStart("Test Associations");
        elapsedTime.reset();
        elapsedTime.start();
        TestAssociationOperations(client, activeTest, verboseTest, uniqueID);
        elapsedTime.stop();
        testEnd(elapsedTime.getElapsed());

        testStart("Test Invoke Method Execution");
        elapsedTime.reset();
        elapsedTime.start();
        TestInvokeMethod(client, activeTest, verboseTest, uniqueID);
        elapsedTime.stop();
        testEnd(elapsedTime.getElapsed());

        testStart("Test Enumerate Instances Execution");
        elapsedTime.reset();
        elapsedTime.start();
        TestEnumerateInstances(client, activeTest, verboseTest, uniqueID);
        elapsedTime.stop();
        testEnd(elapsedTime.getElapsed());
    }catch(Exception& e){
      cout << e.getMessage() << endl;
      errorCount++;
    }
    }
    return ThreadReturnType(0);
}

Thread* runTests(
    CIMClient* client,
    Uint32 testCount,
    Boolean activeTest,
    Boolean verboseTest,
    int uniqueID,
    String ignoreErrors)
{
    // package parameters, create thread and run...
    AutoPtr<T_Parms> parms(new T_Parms());
    parms->client = client;
    parms->testCount = testCount;
    parms->activeTest = (activeTest) ? 1 : 0;
    parms->verboseTest = (verboseTest) ? 1 : 0;
    parms->uniqueID = uniqueID;
    parms->ignoreErrors = ignoreErrors;
    AutoPtr<Thread> t(new Thread(executeTests, (void*)parms.release(), false));

    // zzzzz... (1 second) zzzzz...
    Threads::sleep(1000);
    t->run();
    return t.release();
}

void connectClient(
    CIMClient *client,
    String host,
    Uint32 portNumber,
    String userName,
    String password,
    Boolean useSSL,
    Boolean localConnection,
    Uint32 timeout)
{
    try
    {
        client->setTimeout(timeout);
        if (useSSL)
        {
#ifdef PEGASUS_HAS_SSL
            if (localConnection)
            {
                cout << "Using local connection mechanism " << endl;
                client->connectLocal();
            }
            else
            {
                //
                // Get environment variables:
                //
                const char* pegasusHome = getenv("PEGASUS_HOME");

                String trustpath = FileSystem::getAbsolutePath(
                                pegasusHome, PEGASUS_SSLCLIENT_CERTIFICATEFILE);

                String randFile;
#ifdef PEGASUS_SSL_RANDOMFILE
                randFile = FileSystem::getAbsolutePath(
                                pegasusHome, PEGASUS_SSLCLIENT_RANDOMFILE);
#endif
                SSLContext sslcontext(
                    trustpath, verifyCertificate, randFile);

                cout << "connecting to " << host << ":" << portNumber
                    << " using SSL" << endl;

                client->connect (host, portNumber, sslcontext, userName,
                        password);
            }
#else
            PEGASUS_TEST_ASSERT(false);
#endif
        } // useSSL
        else
        {
            if (localConnection)
            {
                cout << "Using local connection mechanism " << endl;
                client->connectLocal();
            }
            else
            {
                cout << "Connecting to " << host << ":" << portNumber << endl;
                client->connect (host, portNumber, userName, password);
            }
        }

        cout << "Client Connected" << endl;
    }
    catch(ConnectionTimeoutException& e)
    {
        PEGASUS_STD(cerr) << "Warning: " << e.getMessage() << PEGASUS_STD(endl);
        exit(2);
    }
    catch(Exception& e)
    {
        PEGASUS_STD(cerr) << "Error: " << e.getMessage() << PEGASUS_STD(endl);
        exit(1);
    }
}


///////////////////////////////////////////////////////////////
//    MAIN
///////////////////////////////////////////////////////////////

int main(int argc, char** argv)
{
    // Get options (from command line and from configuration file); this
    // removes corresponding options and their arguments fromt he command
    // line.

    String pegasusHome = "/";
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
    if (om.valueEquals("version","true"))
    {
        cout << argv[0] << " version " <<programVersion <<  endl;
        //cout << argv[0] << endl;
        exit(0);
    }

    String tmp;
    om.lookupValue("namespace", tmp);
    CIMNamespaceName localNameSpace = CIMNamespaceName (tmp);
    globalNamespace = localNameSpace;
    cout << "Namespace = " << localNameSpace.getString() << endl;

    Boolean verboseTest = om.isTrue("verbose");


    // Set up number of test repetitions.
    // Will repeat entire test this number of times
    // Default is zero
    // String repeats;
    Uint32 repeatTestCount = 0;
    /* ATTN: KS P0 Test and fix function added to Option Manager
    */
    if (!om.lookupIntegerValue("repeat", repeatTestCount))
        repeatTestCount = 1;

    if(verboseTest)
        cout << "Test repeat count " << repeatTestCount << endl;

    // Setup the active test flag.  Determines if we change repository.
    Boolean activeTest = false;
    if (om.valueEquals("active", "true"))
         activeTest = true;

    String userName;
    om.lookupValue("user", userName);
    if (userName != String::EMPTY)
    {
       cout << "Username = " << userName << endl;
    }

    String password;
    om.lookupValue("password", password);
    if (password != String::EMPTY)
    {
       cout << "password = " << password << endl;
    }

    String ignoreErrors;
    om.lookupValue("ignoreErrors", ignoreErrors);
    if (ignoreErrors != String::EMPTY)
    {
       cout << "ignoreErrors = " << ignoreErrors << endl;
    }

    Boolean localConnection = (om.valueEquals("local", "true"))? true: false;
    Boolean useSSL =  om.isTrue("ssl");

    if ((argc > 2) || (localConnection && (argc > 1)))
    {
        cerr << argv[0] << ": Error:  Multiple targets specified." << endl;
        exit(1);
    }

    String connectionTarget;

    if (argc > 1)
    {
        connectionTarget = argv[1];
    }
    else
    {
        if (useSSL)
        {
#ifdef PEGASUS_HAS_SSL
            connectionTarget = "localhost:5989";
#else
            PEGASUS_TEST_ASSERT(false);
#endif
        }
        else
        {
            connectionTarget = "localhost:5988";
        }
    }

    // timeout
    Uint32 timeout = 30000;
    om.lookupIntegerValue("timeout",timeout);

    // create clients
    Uint32 clients = 1;
    om.lookupIntegerValue("simultaneous",clients);

    // ----------------------------------------
    // * Get host and port number from hostport
    // ----------------------------------------
    HostLocator addr(connectionTarget);
    String host = addr.getHost();
    Uint32 portNumber = 0;
    if (addr.isPortSpecified())
    {
        portNumber = addr.getPort();
    }

    Array<CIMClient*> clientConnections;

    CIMClient* client;
    for (Uint32 i = 0; i < clients; i++)
    {
        client = new CIMClient();
        clientConnections.append(client);
    }

    // connect the clients
    for (Uint32 i = 0; i < clients; i++)
    {
        connectClient(clientConnections[i], host, portNumber, userName,
            password, useSSL, localConnection, timeout);
    }

    // run tests
    Array<Thread *> clientThreads;
    Stopwatch elapsedTime;
    testStart("Begin tests...");
    elapsedTime.reset();
    elapsedTime.start();

    for (Uint32 i = 0; i < clientConnections.size(); i++)
    {
        clientThreads.append(runTests(
            clientConnections[i], repeatTestCount, activeTest, verboseTest, i,
               ignoreErrors));
    }

    for (Uint32 i = 0; i < clientThreads.size(); i++)
    {
        clientThreads[i]->join();
    }

    cout << "+++++ Overall time taken for the operation +++++" << endl;
    elapsedTime.stop();
    testEnd(elapsedTime.getElapsed());

    // clean up
    for (Uint32 i = 0; i< clientConnections.size(); i++)
    {
        delete clientConnections[i];
    }
    for (Uint32 i = 0; i < clientThreads.size(); i++)
    {
        delete clientThreads[i];
    }

    if (errorCount.get() == 0)
    {
        cout << "+++++ "<< argv[0] << " Terminated Normally" << endl;
        return 0;
    }
    else
    {
        cout << "Test case failures: "<< errorCount.get() << endl;
        cout << "+++++ "<< argv[0] << " Failed" << endl;
        exit(1);
    }
}

/*
    TODO:   3. Make test loop tool
            5. Make test multiple systems.
            6. Get rid of diagnostics and clean display
            7. Add correct successful at end
            8. Make OO
            9. Add associations test
            10. Add cim references test.
            11. Add test all namespaces test.

*/
