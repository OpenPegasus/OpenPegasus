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
//%/////////////////////////////////////////////////////////////////////////////

#include <Pegasus/Common/Config.h>
#include <Pegasus/Common/Constants.h>
#include <Pegasus/Common/PegasusVersion.h>
#include <Pegasus/Common/PegasusAssert.h>
#include <Pegasus/Common/TLS.h>
#include <Pegasus/Client/CIMClient.h>
#include <Pegasus/Common/CIMName.h>
#include <Pegasus/Common/FileSystem.h>
#include <Pegasus/Common/Exception.h>
#include <Pegasus/Common/XmlWriter.h>
#include <Pegasus/Config/ConfigManager.h>
#include <Pegasus/Common/HostLocator.h>
#include <Pegasus/Common/Tracer.h>

#include <Pegasus/General/OptionManager.h>

/* This is a test program for the Interop Control Provider.
 * It provides tests for the functions in this provider including;

    The CIM_ObjectManager Class and its single instance
    The CIM_CommunicationsMechanism class and its PG subclass.
    The CIM_Namespace Class
    The corresponding association
*/
PEGASUS_USING_PEGASUS;
PEGASUS_USING_STD;

// Macro puts out message and then does assert error out.
#define TERMINATE(X) {PEGASUS_STD(cout) << "TestInterop " \
    << X << PEGASUS_STD(endl); PEGASUS_TEST_ASSERT(false);}
#define CDEBUG(X)

#if 0
#define CDEBUG(X) PEGASUS_STD(cout) << "InteropTest(" << __LINE__ << ")" << \
    X << PEGASUS_STD(endl)
#endif

#include <cstring>
#include <stdcxx/stream/strstream>

/* Test the namespace manipulation functions.
Creates, deletes, queries namespaces using both the
__Namespace and CIM_Namespace functions
*/
char * pgmName;
static Boolean verbose;

static const CIMNamespaceName __NAMESPACE_NAMESPACE = CIMNamespaceName ("root");

// Property Names for __Namespace Class
static const CIMName NAMESPACE_PROPERTYNAME  = CIMName ("Name");
static const CIMNamespaceName ROOTNS  = CIMNamespaceName ("root");
static const CIMName __NAMESPACE_CLASSNAME  = CIMName ("__Namespace");
static const CIMName CIM_NAMESPACE_CLASSNAME  = CIMName ("CIM_Namespace");

static const CIMName PG_NAMESPACE_CLASSNAME  = CIMName ("PG_Namespace");
static const CIMName CIM_OBJECTMANAGER_CLASSNAME  =
        CIMName ("CIM_ObjectManager");

static const CIMName CIM_WBEMSERVICE_CLASSNAME  = CIMName ("CIM_WBEMService");

static const CIMName CIM_OBJECTMANAGERCOMMUNICATIONMECHANISM_CLASSNAME  =
        CIMName ("CIM_ObjectManagerCommunicationMechanism");
static const CIMName PG_CIMXMLCOMMUNICATIONMECHANISM_CLASSNAME  =
        CIMName ("PG_CIMXMLCommunicationMechanism");

static const CIMName CIM_CIMXMLCOMMUNICATIONMECHANISM_CLASSNAME  =
        CIMName ("CIM_CIMXMLCommunicationMechanism");

static const CIMName CIM_COMMMECHANISMFORMANAGER_CLASSNAME  =
        CIMName ("CIM_CommMechanismForManager");
static const CIMName CIM_NAMESPACEINMANAGER_CLASSNAME  =
        CIMName ("CIM_NamespaceInManager");


String _toString(Boolean x)
{
    return(x ? "true" : "false");
}
 /* local support for display of propertyLists. Converts a property list
   to a String for display. This function is only for diagnostic support.
   Assumes that there is a propertylist and it is not empty or null.
   @param pl CIMPropertyList to convert.
   @return String representation of property list for display.
*/
String _toStringPropertyList(const CIMPropertyList& pl)
{
    String tmp;
    for (Uint32 i = 0; i < pl.size() ; i++)
    {
        if (i > 0)
            tmp.append(", ");
        tmp.append(pl[i].getString());
    }
    return(tmp);
}

String _showPathArray(Array<CIMObjectPath>& p)
{
    String rtn;
    for (Uint32 i = 0 ; i < p.size() ; i++)
    {
        if (i > 0)
            rtn.append("\n");
        rtn.append(p[i].toString());
    }
    return(rtn);
}
/* _showPropertyList is local support for displaying the propertylist
   For display only. Generates String with property list names
   or "empty" or "NULL" if that is the case.
   @param pl CIMPropertyList to convert
   @return String containing the list of properties comma separated
   or the keywords NULL or Empty.
 */
String _showPropertyList(const CIMPropertyList& pl)
{
    if (pl.isNull())
        return("NULL");

    return((pl.size() == 0) ? String("EMPTY") : _toStringPropertyList(pl));
}

/** Check to see if the specified path is in the path list
    @param CIMObjectPath which is target for search
    @param Array of CIMObjectPaths to be searched
    @return true if the path is in the list otherwise false.
*/
Boolean _containsObjectPath(const CIMObjectPath& path,
    const Array<CIMObjectPath>& pathList)
{
    for (Uint32 p = 0; p < pathList.size(); p++)
    {
        if (pathList[p].identical(path))
            return true;
    }
    return false;
}

Boolean _checkExceptionCode
    (const CIMException & e,
     const CIMStatusCode expectedCode)
{
    if (verbose)
    {
        if (e.getCode () != expectedCode)
        {
            cerr << "CIMException comparison failed.  ";
            cerr << "Expected " << cimStatusCodeToString (expectedCode) << "; ";
            cerr << "Actual exception was " << e.getMessage () << "." << endl;
        }
    }

    if (e.getCode () == expectedCode)
        return(true);
    else
        return(false);
}
/* Class created to provide cover for all of the tests in this
    test program.
*/
class InteropTest
{
public:
    //InteropTest();

    InteropTest();

    ~InteropTest();

    // Methods associated with Namespace testing
    Array<CIMNamespaceName> _getNamespacesOld();

    Boolean _deleteOneLevelOfNamespace(
        const CIMNamespaceName& parent,
        const String & child);

    Boolean _deleteNamespaceOld(const String & name);

    Boolean _validateNamespaces(
        Array<CIMNamespaceName>& namespaceNames);

    Array<CIMInstance> _getCIMNamespaceInstances();

    Array<CIMInstance> _getPGNamespaceInstances();

    Array<CIMObjectPath> _getCIMNamespaceInstanceNames();

    Array<CIMObjectPath> _getPGNamespaceInstanceNames();

    Array<CIMNamespaceName> _getNamespacesNew();

    void _showNamespaceInfo(const String& title);

    Boolean _existsNew(const CIMNamespaceName& name);

    Boolean _namespaceCreate__Namespace(
                            const CIMNamespaceName& parent,
                            const String& child);

    Boolean _namespaceCreatePG_Namespace(const CIMNamespaceName& name);

    Boolean _namespaceCreateCIM_Namespace(const CIMNamespaceName& name,
                            const CIMNamespaceName& targetNamespace);

    Boolean _testPGNamespace(const CIMNamespaceName& name,
                            Boolean shared,
                            Boolean updatesAllowed,
                            const String& parent);

    Boolean _namespaceCreatePG_Namespace(const CIMNamespaceName& name,
                            const Boolean shareable,
                            const Boolean updatesAllowed,
                            const String& parent);

    Boolean _namespaceDeleteCIM_Namespace(const CIMNamespaceName& name);

    Boolean testClassExists(const CIMName & className);

    // Methods associated with overall testing
    void testPGProviderProfileCapabilities();

    void testNameSpacesManagement();

    void testSharedNameSpacesManagement();

    CIMInstance getInstanceObjMgr();

    CIMObjectPath getObjMgrPath();

    void testObjectManagerClass();

    void setStatisticsState(const Boolean flag);

    Boolean testStatisticsSetOperationError(
            const CIMInstance & instance,
            const CIMPropertyList& list,
            Boolean shouldRespondGood,
            Boolean includeQualifiers,
            const CIMStatusCode expectedCode);

    Boolean getStatisticsPropertyState(
            CIMInstance & objMgrInstance);

    String getCurrentConfigProperty(
            const CIMName& propName);

    Boolean getCurrentBoolConfigProperty(
            const CIMName& propName);

    Uint32 getCurrentValueConfigProperty(
           const CIMName& propName);

    Boolean getStatisticsState();

    void showStatisticsState();

    void testStatisticsEnable();

    void testCommunicationClass();

    void testNameSpaceInObjectManagerAssocClass();

    void testCommMechinManagerAssocClass();

    Boolean testEnumerateOptions(
        const CIMName& className,
        Boolean deepInheritance,
        const CIMPropertyList propertyList,
        const Uint32 expectedPropertyCount);

    // Methods associated with general instance testing
    Boolean matchPathsAndInstancePaths(
        Array<CIMObjectPath>& paths,
        const Array<CIMInstance> instances);

    // Methods associated with general instance testing
    Boolean matchPathsAndObjectPaths(
        Array<CIMObjectPath>& paths,
        const Array<CIMObject> instanceObjectss);

    Boolean testGetInstancesForEnum(
        const Array<CIMObjectPath>& paths,
        const Array<CIMInstance>& instances,
        const Boolean includeQualifiers,
        const Boolean includeClassOrigin,
        const CIMPropertyList& propertyList);

    Boolean testEnumAgainstEnumNames(
        const CIMNamespaceName& nameSpace,
        const CIMName& className);
private:

    CIMClient _client;
    CIMInstance objectManagerInstance;
};
/**
    Get property values for the specified configuration property from
    the CIM Server.
*/

static const CIMName PROPERTY_NAME  = CIMName ("PropertyName");

String InteropTest::getCurrentConfigProperty(
    const CIMName& propertyName)
{
    // The following assumes localconnect.
    String _hostName;
    _hostName.assign(System::getHostName());

    CIMProperty prop;
    Array<CIMKeyBinding> kbArray;
    CIMKeyBinding kb;

    kb.setName(PROPERTY_NAME);
    kb.setValue(propertyName.getString());
    kb.setType(CIMKeyBinding::STRING);

    kbArray.append(kb);

    String propertyNameValue;
    String currentValue;
    //String defaultValue;
    //String plannedValue;
    try
    {
        CIMObjectPath reference(
            _hostName, PEGASUS_NAMESPACENAME_CONFIG,
            PEGASUS_CLASSNAME_CONFIGSETTING, kbArray);

        CIMInstance cimInstance =
            _client.getInstance(PEGASUS_NAMESPACENAME_CONFIG, reference);

        Uint32 pos = cimInstance.findProperty(PROPERTY_NAME);
        prop = (CIMProperty)cimInstance.getProperty(pos);
        propertyNameValue = prop.getValue().toString();

        pos = cimInstance.findProperty(CIMName ("CurrentValue"));
        prop = (CIMProperty)cimInstance.getProperty(pos);
        currentValue = prop.getValue().toString();

        //pos = cimInstance.findProperty(CIMName ("DefaultValue"));
        //prop = (CIMProperty)cimInstance.getProperty(pos);
        //defaultValue = prop.getValue().toString();

        //pos = cimInstance.findProperty(CIMName ("PlannedValue"));
        //prop = (CIMProperty)cimInstance.getProperty(pos);
        //plannedValue = prop.getValue().toString();

        if (verbose)
        {
            cout << " Config return: "
                << " Requested Name: " <<  propertyName.getString()
                << " Returned Name: " <<  propertyNameValue
                //<< " Default: " << defaultValue
                //<< " planned: " << plannedValue
                << " current: " << currentValue << endl;
        }
    }
    catch(const CIMException& e)
    {
        cout << "CIM Exception during get Config " << e.getMessage() << endl;
        return(String::EMPTY);
    }

    return (currentValue);
}

Boolean InteropTest::getCurrentBoolConfigProperty(
    const CIMName& propName)
{
    return ConfigManager::parseBooleanValue(getCurrentConfigProperty(propName));
}

Uint32 InteropTest::getCurrentValueConfigProperty(
    const CIMName& propName)
{
    String strValue = getCurrentConfigProperty(propName);

    long longValue;
    if (!sscanf(strValue.getCString(), "%ld", &longValue))
       return 0;

    return (Uint32)longValue;
}
/** Test a single class to assure that the returned reference
    names from an enumerateInstanceNames and the paths in
    the instances match. Then get all classes and test against
    the returned instances to assure that they match.
    @parm - namespace - Namespace for this test.
    @param - className CIMName of class to test.
    @return returns true if tests pass. False if there is
    a difference.
    @any exceptions caught are displayed and the error
    return executed.
*/
Boolean InteropTest::testEnumAgainstEnumNames(
    const CIMNamespaceName& nameSpace,
    const CIMName& className)
{
    try
    {
        Array<CIMObjectPath> paths;
        Array<CIMInstance> instances;
        paths = _client.enumerateInstanceNames(nameSpace, className);

        instances = _client.enumerateInstances(nameSpace,
                                className,
                                true,           // deepinheritance
                                false,          // localonly
                                true,           //includeQualifiers
                                true,           //includeClassOrigin
                                CIMPropertyList());

        if(!(paths.size() == instances.size()))
        {
            cout << "Error: Class " << className.getString() <<
                " referenceCount: " << paths.size() <<
                " instanceCount: " << instances.size() << endl;
            return(false);
        }

        if( !matchPathsAndInstancePaths(paths, instances))
        {
            /* Assumes that the match function will flag errors
            cout << "Error: Class " << className.getString() <<
                " reference and Instance paths do not match " << endl;
            */
            return(false);
        }

        for (Uint32 i = 0 ; i < instances.size() ; i++ )
        {
            CIMInstance instance = _client.getInstance(nameSpace,
                            instances[i].getPath(),
                            false,                  //lo
                            true,                   //includeQualifiers
                            true,                   //includeClassOrigin
                            CIMPropertyList());

            instance.setPath(instances[i].getPath());
            if (!(instances[i].getPath() == instance.getPath()))
            {
                cout << "Error: Class " << className.getString() <<
                    " Instance path does not match enumerate for instance.\n\n"
                    << instances[i].getPath().toString() <<
                    "\n\n" << instance.getPath().toString() << endl << endl;
                return false;
            }

            // TODO DELETE CIMObjectPath temp = instances[i].getPath();
            if (!instance.identical(instances[i]))
            {
                cout << "Error: Class " << className.getString() <<
                    " Instance identical test faild for instance " <<
                    instances[i].getPath().toString() << endl;
                return(false);
            }
        }
    }
    catch(CIMException& e)
    {
        cerr << "testEnumAgainstEnumNames CIMException: " << e.getMessage()
             << endl;
    }
    catch(Exception& e)
    {
        cerr << "testEnumAgainstEnumNames Pegasus Exception: "
             << e.getMessage()  << endl;
    }
    catch(...)
    {
        cerr << "testEnumAgainstEnumNames Caught General Exception:" << endl;
    }
    return(false);
}
/* test getting instances from an enum list. Compares returned instances to
    those in the corresponding instances list. We assume that the instance list
   was from an enum.  This confirms that the instances returned from getInstance
   are the same as those from enumerateinstances.
   Note: Since this tests the instance for equality with the instance from
   the enumerate, the enumerate must have matching attributes AND must be
   deepInheritance to assure that all properties are returned.

   @param paths Array of CIMObjectPaths representing the enumerateinstanceNames
   result
   @param instances Array of CIMInstances representing the enumerateInstances
   result
   @return True if passes test. False if any of the tests fail
   @Exceptions Any number of uncaught exceptions from the calls.
*/
Boolean InteropTest::testGetInstancesForEnum(const Array<CIMObjectPath>& paths,
                                        const Array<CIMInstance>& instances,
                                        const Boolean includeQualifiers,
                                        const Boolean includeClassOrigin,
                                        const CIMPropertyList& propertyList)
{
    // Get every instance to confirm that it is gettable
    for (Uint32 i = 0 ; i < paths.size() ; i++)
    {
        CIMInstance instance =
            _client.getInstance(PEGASUS_NAMESPACENAME_INTEROP,
                                          paths[i],
                                          false,
                                          includeQualifiers,
                                          includeClassOrigin,
                                          propertyList);
        for (Uint32 j = 0; instances.size() ; j++)
        {
            if (instance.getPath() == instances[j].getPath())
            {
                if (!instances[j].identical(instance))
                {
                    cout << " Instances not equal "
                        << instances[j].getPath().toString() << endl;
                    return (false);
                }
                else
                    break;
            }
        }
    }
    return(true);
}

/* Validate path definitions in results of enumerateInstances against that in
   enumerateinstance paths.  Confirms that the same number of items is returned
   and  they have the same path definitions. Note that this comparison is
   independent of hostname and namespaces because they are not returned with
   instances.
   @param paths Array<CIMObjectPath> from an enumerateinstanceNames
   @param instances Array of CIMInstance from enumerateInstanceN.
*/

Boolean InteropTest::matchPathsAndInstancePaths(Array<CIMObjectPath>& paths,
        const Array<CIMInstance> instances)
{
    PEGASUS_TEST_ASSERT(instances.size() == paths.size());
    if (paths.size() == 0)
        return true;

    Array<CIMObjectPath> instancePaths;
    // create path list from instances
    for (Uint32 i = 0 ; i < instances.size() ; i++)
    {
        instancePaths.append(instances[i].getPath());
    }
    String host = paths[0].getHost();
    CIMNamespaceName ns = paths[0].getNameSpace();

    for (Uint32 i = 0 ; i < instancePaths.size() ; i++)
    {
        if (instancePaths[i].getHost() == String::EMPTY)
        {
            instancePaths[i].setHost(host);
        }
        if (instancePaths[i].getNameSpace().isNull())
        {
            instancePaths[i].setNameSpace(ns);
        }
    }
    for (Uint32 i = 0 ; i < paths.size() ; i++)
    {
        if (!_containsObjectPath(instancePaths[i], paths))
        {
            // Failed test. Display error and rtn false.
            if (true)
            {
                cout << "matchPathsAndInstancePaths Error in path match. Path\n"
                    << instances[i].getPath().toString()
                    << "\nNot found in the following array of paths" << endl;
                for (Uint32 j = 0 ; j < paths.size() ; j++)
                {
                    cout << paths[j].toString() << endl;;
                }
            }
            return (false);
        }
    }
    return (true);
}

Boolean InteropTest::matchPathsAndObjectPaths(Array<CIMObjectPath>& paths,
        const Array<CIMObject> ObjectInstances)
{
    PEGASUS_TEST_ASSERT(ObjectInstances.size() == paths.size());
    if (paths.size() == 0)
        return true;

    Array<CIMObjectPath> instancePaths;
    // create path list from instances
    for (Uint32 i = 0 ; i < ObjectInstances.size() ; i++)
    {
        instancePaths.append(ObjectInstances[i].getPath());
    }
    String host = paths[0].getHost();
    CIMNamespaceName ns = paths[0].getNameSpace();

    for (Uint32 i = 0 ; i < instancePaths.size() ; i++)
    {
        if (instancePaths[i].getHost() == String::EMPTY)
        {
            instancePaths[i].setHost(host);
        }
        if (instancePaths[i].getNameSpace().isNull())
        {
            instancePaths[i].setNameSpace(ns);
        }
    }
    for (Uint32 i = 0 ; i < paths.size() ; i++)
    {
        if (!_containsObjectPath(instancePaths[i], paths))
        {
            // Failed test. Display error and rtn false.
            if (true)
            {
                cout << "matchPathsAndInstancePaths Error in path match. Path\n"
                    << ObjectInstances[i].getPath().toString() <<
                    "\nNot found in the following array of paths" << endl;
                for (Uint32 j = 0 ; j < paths.size() ; j++)
                {
                    cout << paths[j].toString() << endl;;
                }
            }
            return (false);
        }
    }
    return (true);
}

Boolean InteropTest::testEnumerateOptions(
        const CIMName& className,
        Boolean deepInheritance,
        const CIMPropertyList propertyList,
        const Uint32 expectedPropertyCount)
{

    CDEBUG("testEnumerations 0" );
    Array<CIMInstance> instancesObjMgr = _client.enumerateInstances(
                                 PEGASUS_NAMESPACENAME_INTEROP,
                                 className,
                                 deepInheritance,
                                 false,  // localOnly
                                 false,  // include qualifiers = false
                                 false,
                                 propertyList);

    PEGASUS_TEST_ASSERT(instancesObjMgr.size() == 1);
    CIMInstance instance = instancesObjMgr[0];
    CIMPropertyList rtnd;
    Array<CIMName> nameList;
    CDEBUG("testEnumerations 1" << " prpertycount= "
            << instance.getPropertyCount());
    for (Uint32 i = 0 ; i < instance.getPropertyCount() ; i++)
    {

        CDEBUG("testEnumerations 1a" << " propertycount= "
                << instance.getProperty(i).getName().getString());
        nameList.append(instance.getProperty(i).getName());
    }

    CDEBUG("testEnumerations 2" );
    rtnd.set(nameList);
    CDEBUG("testEnumerations 3" );
    if (instance.getPropertyCount() != expectedPropertyCount)
    {
        cout << "Error in enumerate options. "
             << " Class " << className.getString()
             << " di= "   << _toString(deepInheritance)
             << " Expected Property count " << expectedPropertyCount
             << " Received Property Count " << instance.getPropertyCount()
             << "\nPl Sent= " << _showPropertyList(propertyList)
             << "\nPl Rcvd= " << _showPropertyList(rtnd)
             << endl;

        PEGASUS_TEST_ASSERT(false);
        return(false);
    }
    CDEBUG("testEnumerations rtn" );
    return(true);
}

InteropTest::InteropTest()
{
    try
    {
        _client.connectLocal();
    }
    catch (Exception& e)
    {
        cerr << "Connection error: " << e.getMessage() << endl;
        exit(1);
    }
}

InteropTest::~InteropTest()
{
    _client.disconnect();
}

/* Get all namespaces and return as an array
*/
Array<CIMNamespaceName> InteropTest::_getNamespacesOld()
{
    CIMName className = "__NameSpace";
    Array<CIMNamespaceName> namespaceNames;

    // Build the namespaces incrementally starting at the root
    // ATTN: 20030319 KS today we start with the "root" directory but this
    // is wrong. We should be
    // starting with null (no directory) but today we get an xml error
    // return in Pegasus
    // returned for this call. Note that the specification requires that
    // the root namespace be used
    // when __namespace is defined but does not require that it be the root
    // for allnamespaces. That
    // is a hole is the spec, not in our code.
    namespaceNames.append("root");
    namespaceNames.append("test");
    namespaceNames.append("SDKExamples");
    Uint32 start = 0;
    Uint32 end = namespaceNames.size();
    do
    {
        // for all new elements in the output array
        for (Uint32 range = start; range < end; range ++)
        {
            // Get the next increment in naming for all a name elements
            // in the array
            CIMNamespaceName next;
            if (end != 0)
            {
                next = namespaceNames[range];
            }
            Array<CIMInstance> instances =
                _client.enumerateInstances(next, className);
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
    // It is legal for us to have a name component that is really not a
    // namespace (ex. root/fred/john is a namespace  but root/fred is not.
    // There is no clearly defined test for this so we will simply try to
    // get something, in this case a well known assoication.
    // But, this means that namespaces that may be valid but that
    // have no contents do not show up on this list.
    Array<CIMNamespaceName> returnNamespaceNames;
    for (Uint32 i = 0 ; i < namespaceNames.size() ; i++)
    {
        try
        {
            CIMQualifierDecl cimQualifierDecl;
            cimQualifierDecl = _client.getQualifier(namespaceNames[i],
                                           "Association");

            returnNamespaceNames.append(namespaceNames[i]);
        }
        catch(CIMException& e)
        {
            if (e.getCode() != CIM_ERR_INVALID_NAMESPACE)
                returnNamespaceNames.append(namespaceNames[i]);
        }
    }
    return(returnNamespaceNames);
}

Boolean InteropTest::_deleteOneLevelOfNamespace(
    const CIMNamespaceName& parent,
    const String & child)
{
    try
    {
        CIMObjectPath myReference(String::EMPTY, parent, child);
        _client.deleteInstance(parent, myReference);
    }
    catch(Exception& e)
    {
        PEGASUS_STD(cerr) << "Exception NameSpace Deletion: "
            << e.getMessage() << " Deleting " << child << " from "
            << parent.getString() << PEGASUS_STD(endl);
        return false;
   }
   return true;
}

/* Delete the namespace defined by the input. This function uses
   the __Namspace tools to do the delete.
*/
Boolean InteropTest::_deleteNamespaceOld(const String & name)
{
    Uint32 pos;
    while ((pos = name.reverseFind('/')) != PEG_NOT_FOUND)
    {
        String parent = name.subString(0, pos);
        String child = name.subString (pos + 1);
        Boolean rtn =
            _deleteOneLevelOfNamespace(CIMNamespaceName(parent), child);
        if (!rtn)
            return false;
    }
    return true;
}

Boolean InteropTest::_validateNamespaces(
    Array<CIMNamespaceName>& namespaceNames)
{
    // Validate that these are all namespaces.  This is not a certain
    // Test but we simply check to see if there is an association
    // qualifier in the namespace. If the call returns the
    // invalidNamespace exception, assume this is not valid namespace.
    Array<CIMNamespaceName> returnNamespaces;

    for (Uint32 i = 0 ; i < namespaceNames.size() ; i++)
    {
        try
        {
            CIMQualifierDecl cimQualifierDecl;
            cimQualifierDecl = _client.getQualifier(namespaceNames[i],
                                           "Association");

            returnNamespaces.append(namespaceNames[i]);
        }
        catch(CIMException& e)
        {
            if (e.getCode() != CIM_ERR_INVALID_NAMESPACE)
                returnNamespaces.append(namespaceNames[i]);
        }
    }

    if (verbose)
    {
        cout << returnNamespaces.size() << " namespaces " << " returned."
             << endl;
        for (Uint32 cnt = 0 ; cnt < returnNamespaces.size(); cnt++ )
        {
            cout << returnNamespaces[cnt].getString() << endl;;
        }
    }

    return (returnNamespaces.size() == namespaceNames.size());
}

/* Deleted for now
Boolean _existsOld(CIMNamespaceName& name)
{
    Uint32 pos;
    String parent;
    String child;
    String nameString = name.getString();
    if((pos = nameString.reverseFind('/')) != PEG_NOT_FOUND)
    {
        parent = nameString.subString(0, pos);
        child = nameString.subString (pos + 1);
    }
    else
    {
        child = nameString;

    }

    Array<CIMKeyBinding> keyBindings;
    // wrong ATTN: This not good.
    keyBindings.append(CIMKeyBinding(NAMESPACE_PROPERTYNAME,
         isRelativeName?childNamespaceName.getString():
                            parentNamespaceName.getString(),
                                 CIMKeyBinding::STRING));
    CIMObjectPath ref(String::EMPTY, parent, child);

    try
    {
        CIMInstance instance.getInstance(parent, ref);
    }
    catch(CIMException& e)
    {
        cerr << "Exception NameSpace query: "
           << e.getMessage() << " querying " << name
               << endl;
    }
    return(true);
}
************************************************/


/* gets the instances for the CIM_Namespace class from host
*/
Array<CIMInstance> InteropTest::_getCIMNamespaceInstances()
{
    Array<CIMInstance> instances;
    try
    {
        instances = _client.enumerateInstances(PEGASUS_NAMESPACENAME_INTEROP,
                                              CIM_NAMESPACE_CLASSNAME);
    }
    catch (Exception& e)
    {
        cerr << "Error enumerating instances of CIM_Namespace: " <<
            e.getMessage() << endl;
        // Instead of this returns emptyexit(1);
    }
    return instances;
}

/* gets the instances for the CIM_Namespace class from host
*/
Array<CIMInstance> InteropTest::_getPGNamespaceInstances()
{
    Array<CIMInstance> instances;
    try
    {
        instances = _client.enumerateInstances(PEGASUS_NAMESPACENAME_INTEROP,
                                              PG_NAMESPACE_CLASSNAME);
    }
    catch (Exception& e)
    {
        cerr << "Error enumerating instances of PG_Namespace: " <<
            e.getMessage() << endl;
        // Instead of this returns emptyexit(1);
    }
    return instances;
}

/* gets the instancenames for the CIM_Namespace class from host
*/
Array<CIMObjectPath> InteropTest::_getPGNamespaceInstanceNames()
{
    Array<CIMObjectPath> objectNames;
    try
    {
        objectNames = _client.enumerateInstanceNames(
                PEGASUS_NAMESPACENAME_INTEROP,
                PG_NAMESPACE_CLASSNAME);
    }
    catch (Exception& e)
    {
        cerr << "Error enumerating instance names of PG_Namespace: " <<
            e.getMessage() << endl;
        // Instead of this returns emptyexit(1);
    }
    return objectNames;

}
/* gets the instancenames for the CIM_Namespace class from host
*/
Array<CIMObjectPath> InteropTest::_getCIMNamespaceInstanceNames()
{
    Array<CIMObjectPath> objectNames;
    try
    {
        objectNames = _client.enumerateInstanceNames(
                PEGASUS_NAMESPACENAME_INTEROP,
                CIM_NAMESPACE_CLASSNAME);
    }
    catch (Exception& e)
    {
        cerr << "Error enumerating instance names of CIM_Namespace: " <<
            e.getMessage() << endl;
        // Instead of this returns emptyexit(1);
    }
    return objectNames;

}

/* find the name key in the keybindings and return the value.
    Executes exception if the key not found
    @param object path we will search
    @param keyName - Name of the key to find.
    @return value of name property
    @exceptions CIMInvalidParameterException
    NOTE: This one is a real NO NO. Should never have been written this
    way.  We should be getting the value from the instance, not the
    keys.
*/
String _getKeyValue(const CIMObjectPath& instanceName, const CIMName& keyName)
{
    Array<CIMKeyBinding> kbArray = instanceName.getKeyBindings();

    // find the correct key binding
    for (Uint32 i = 0; i < kbArray.size(); i++)
    {
        if (kbArray[i].getName() == keyName)
        {
            return (kbArray[i].getValue());
        }
    }
    cerr <<"_getKeyValue: Invalid key property: " << keyName.getString()<< endl;
    return(String::EMPTY);
}

// Get the key value from the instance for the keyName defined
String _getKeyValue(const CIMInstance& instance, const CIMName& keyName)
{
    Uint32 pos;
    CIMValue propertyValue;

    pos = instance.findProperty(keyName);
    if (pos == PEG_NOT_FOUND)
    {
        cerr << "_getKeyValue Error. Property " << keyName.getString()
             << " not found " << endl;
        return(String::EMPTY);
    }

    propertyValue = instance.getProperty(pos).getValue();
    if (propertyValue.getType() != CIMTYPE_STRING)
    {
        cerr << "Key Property " << keyName.getString()
             << " incorrect type" << endl;
        return(String::EMPTY);
    }
    String name;
    propertyValue.get(name);
    return(name);
}

/** get the namespaceNames into an array.
    return array of CIMNamespaceName containing the names of all
    namespaces found through the CIM_Namespace class
*/
Array<CIMNamespaceName> InteropTest::_getNamespacesNew()
{
    Array<CIMObjectPath> instanceNames;
    instanceNames = _getCIMNamespaceInstanceNames();

    Array<CIMNamespaceName> rtns;
    for (Uint32 i = 0 ; i < instanceNames.size(); i++)
    {
        String name = _getKeyValue(instanceNames[i], NAMESPACE_PROPERTYNAME);
        if (name == String::EMPTY)
        {
            continue;
        }
        rtns.append(CIMNamespaceName(name));
    }
    return(rtns);
}

/** get one string property from an instance. Note that these functions simply
    return the default value if the property cannot be found or is of the wrong
    type thus, in reality being a maintenance problem since there is no
    error indication.
    @param instance CIMInstance from which we get property value
    @param propertyName String name of the property containing the value
    @param default String optional parameter that is substituted if the property
    does not exist, is Null, or is not a string type. The substitute is
    String::EMPTY
    @return String value found or defaultValue.
*/
void _getPropertyValue(const CIMInstance& instance, const CIMName& propertyName,
    const String& defaultValue, String & returnValue)
{
    returnValue = defaultValue;
    Uint32 pos;
    if ((pos = instance.findProperty(propertyName)) != PEG_NOT_FOUND)
    {
        CIMConstProperty p1 = instance.getProperty(pos);
        if (p1.getType() == CIMTYPE_STRING)
        {
            CIMValue v1  = p1.getValue();

            if (!v1.isNull())
                v1.get(returnValue);
        }
    }
}

// Overload of _getPropertyValue for boolean type
void _getPropertyValue(const CIMInstance& instance, const CIMName& propertyName,
    const Boolean defaultValue, Boolean returnValue)
{
    returnValue = defaultValue;
    Uint32 pos;
    if ((pos = instance.findProperty(propertyName)) != PEG_NOT_FOUND)
    {
        CIMConstProperty p1 = instance.getProperty(pos);
        if (p1.getType() == CIMTYPE_BOOLEAN)
        {
            CIMValue v1  = p1.getValue();

            if (!v1.isNull())
                v1.get(returnValue);
        }
    }
}

void InteropTest::_showNamespaceInfo(const String& title)
{
    Array<CIMInstance> instances;
    instances = _getCIMNamespaceInstances();

    cout << title << " size = " << instances.size() << endl;
    Array<CIMNamespaceName> rtns;
    for (Uint32 i = 0 ; i < instances.size(); i++)
    {
        String isSharable;
        String updatesAllowed;
        String parent;
        String name;

        // get the namespace name from the name property.
        if ((instances[i].findProperty(NAMESPACE_PROPERTYNAME)) ==
                PEG_NOT_FOUND)
            isSharable = "ERROR: Name Property Not Found";
        else
        {

            _getPropertyValue(instances[i],NAMESPACE_PROPERTYNAME,
                String("ERROR: No Name Property Value"),
                name);
        }

        // if this is a PG_Namespace, get the other characteristics.
        if (instances[i].getClassName() == PG_NAMESPACE_CLASSNAME)
        {
            // Get the sharable property
            if ((instances[i].findProperty("IsShareable")) == PEG_NOT_FOUND)
                isSharable = "Property Not Found";
            else
            {
                Boolean boolRtn = false;
                _getPropertyValue(instances[i],"IsShareable", false, boolRtn);
                isSharable = boolRtn? "true" : "false";

            }

            // get the schemUpdatesAllowed property information
            if ((instances[i].findProperty("SchemaUpdatesAllowed")) ==
                    PEG_NOT_FOUND)
                isSharable = "Property Not Found";
            else
            {
                Boolean boolRtn = false;
                _getPropertyValue(instances[i],"SchemaUpdatesAllowed",
                                  false, boolRtn);
                updatesAllowed = boolRtn ?  "true" : "false";
            }
                        // get the schemUpdatesAllowed property information
            if ((instances[i].findProperty("ParentNamespace")) == PEG_NOT_FOUND)
                isSharable = "Property Not Found";
            else
            {
                _getPropertyValue(instances[i],"ParentNamespace",
                                  String("No parent"), parent);
            }

        }

        printf("%-20s %-10s %-10s %-10s\n", (const char *)name.getCString(),
                               (const char *)isSharable.getCString(),
                               (const char *)updatesAllowed.getCString(),
                               (const char *)parent.getCString());
    }
}

void _showNamespaceList(
    const Array<CIMNamespaceName> names,
    const String& title)
{
    cout << title << " size = " << names.size() << endl;
    for (Uint32 i = 0; i < names.size(); i++)
    {
        cout << "  " << names[i].getString() << endl;
    }
}


// Determine if the named namespace exists in the host.
// gets all namespaces and compares for this one.
Boolean InteropTest::_existsNew(const CIMNamespaceName& name)
{
    //Get all namespace instances

    Array<CIMNamespaceName> namespaceNames =
        _getNamespacesNew();

    for(Uint32 i = 0; i < namespaceNames.size(); i++)
    {
        if(namespaceNames[i].equal ( name ))
           return true;
    }
    return false;
}

/* Create a single namespace with __Namespace Class.
   @param parent CIMNameSpaceName defining the parent namespace for
   this operation.
   @param childname.
*/
Boolean InteropTest::_namespaceCreate__Namespace(const CIMNamespaceName& parent,
                                                 const String& child)
{
    CIMObjectPath newInstanceName;
    try
    {
        // Build the new instance
        CIMInstance newInstance(__NAMESPACE_CLASSNAME);
        newInstance.addProperty(CIMProperty(CIMName (NAMESPACE_PROPERTYNAME),
            child));
        newInstanceName = _client.createInstance(parent,
                                                 newInstance);
    }
    catch(CIMException& e)
    {
         if (e.getCode() == CIM_ERR_ALREADY_EXISTS)
         {
               PEGASUS_STD(cerr) << "CIMException: NameSpace Creation: "
                   << e.getMessage() << ". " << parent.getString() << "/"
                   << child << " Already Exists. Cannot create."
                   << PEGASUS_STD(endl);
         }
         else
         {
              PEGASUS_STD(cerr) << "CIMException: NameSpace Creation: "
                  << e.getMessage() << " Creating " << parent.getString()
                  << "/" << child << PEGASUS_STD(endl);
         }
         return(false);
    }
    catch(Exception& e)
    {
        PEGASUS_STD(cerr) << "Exception: NameSpace Creation with __Namespace: "
            << e.getMessage() << PEGASUS_STD(endl);
        return(true);
    }
    return(true);
}

/** Create a single namespace using PG_Namespace. Creates the namespace with
    the name defined in the call by creating a new instance of PG_Namespace.
    @param name - CIMNamespaceName of namespace to create.
    @param CIMNamespaceName ns is optional parameter that defines the name
    of the CIMServer namespace to be used as the target for the instance
    create.  This option is only used to test for ability to create namespaces
    in other than the defined interop namespace.
    @return - true if creeate. False if not created or if it already exists.
*/
Boolean InteropTest::_namespaceCreatePG_Namespace(const CIMNamespaceName& name)
{
    // Does this namespace exist.
    if (_existsNew(name))
        return(false);

    //Now build the new namespace name instance.  Note that we need to get the
    // collection of keys that are required. Easy way was to simply
    // use an existing instance and change the name field.
    Array<CIMInstance> instances = _getCIMNamespaceInstances();

    if(instances.size() == 0)
    {
        return(false);
    }

    // El cheapo code Modify one existing instance and send it back as
    // method to construct the correct keys.
    CIMInstance instance = instances[0];

    // Modify the name property value for new name
    Uint32 pos = instance.findProperty(NAMESPACE_PROPERTYNAME);
    if (pos == PEG_NOT_FOUND)
    {
            cerr << "Error in property on create. No "
                <<  NAMESPACE_PROPERTYNAME.getString() << " property" << endl;
            return(false);
    }

    // Modify the name field in the instance and resend
    String localName = name.getString();
    CIMProperty p = instance.getProperty(pos);

    // test for correct property type, etc.
    p.setValue(localName);
    instance.removeProperty(pos);
    instance.addProperty(p);

    if (verbose)
    {
        cout << "Show instance used to do namespace create: " << endl;
        XmlWriter::printInstanceElement(instance);
    }
    try
    {
           CIMObjectPath path;
           path = _client.createInstance(PEGASUS_NAMESPACENAME_INTEROP,
                   instance);
    }
    catch(Exception& e)
    {
        cerr << "Error during Creation of " << name.getString()
            << " in Namespace " << PEGASUS_NAMESPACENAME_INTEROP.getString()
            << ": " << e.getMessage()
            << " Instance Creation error" << endl;
        return(false);
    }

    return(true);
}

/** Create a single namespace using CIM_Namespace.
    @param name CIMNamespaceName of namespace to create
    @param target CIMNamespaceName of namespace in which we issue
    the command to create the namespace.  This paramater is primarily
    to test if we can issue this operation in namespaces other than the
    interop namespace.
*/
Boolean InteropTest::_namespaceCreateCIM_Namespace(const CIMNamespaceName& name,
                           const CIMNamespaceName& targetNamespace)
{
    // Does this namespace exist.
    if (_existsNew(name))
    {
        cout << "Namespace " << name.getString() <<
            " already Exists in _namespacCreateCIM_Namespace function." << endl;
        return(false);
    }

    //Now build the new namespace name instance.  Note that we need to get the
    // collection of keys that are required. Easy way was to simply
    // use an existing instance and change the name field.
    Array<CIMInstance> instances = _getCIMNamespaceInstances();

    if(instances.size() == 0)
    {
        return(false);
    }

    // Modify one existing instance and send it back as
    // method to construct the correct keys.
    CIMInstance instance = instances[0];

    CIMInstance newInstance(CIM_NAMESPACE_CLASSNAME);

    for (Uint32 i = 0 ; i < instance.getQualifierCount() ; i++)
    {
        newInstance.addQualifier(instance.getQualifier(i).clone());
    }
    Array<CIMName> droplist;
    droplist.append("SchemaUpdatesAllowed");
    droplist.append("IsShareable");
    droplist.append("ParentNamespace");

    for (Uint32 i = 0 ; i < instance.getPropertyCount() ; i++)
    {
        for (Uint32 j = 0 ; j < droplist.size() ; j++)
        {
            if (Contains(droplist, instance.getProperty(i).getName()))
            {
                break;
            }
        }
        newInstance.addProperty(instance.getProperty(i).clone());
    }

    // Modify the name property value for new name
    Uint32 pos = instance.findProperty(NAMESPACE_PROPERTYNAME);
    if (pos == PEG_NOT_FOUND)
    {
            cerr << "Error in property on create. No "
                <<  NAMESPACE_PROPERTYNAME.getString() << " property" << endl;
            return(false);
    }

    // Modify the name field in the instance and resend
    String localName = name.getString();
    CIMProperty p = instance.getProperty(pos);

    // test for correct property type, etc.
    p.setValue(localName);
    instance.removeProperty(pos);
    instance.addProperty(p);

    if (verbose)
    {
        cout << "Show instance used to do namespace create: " << endl;
        XmlWriter::printInstanceElement(instance);
    }
    try
    {
           CIMObjectPath path;
           path = _client.createInstance(targetNamespace, instance);
    }
    catch(CIMException& e)
    {
        if ((e.getCode() != CIM_ERR_INVALID_CLASS) && (e.getCode() !=
                    CIM_ERR_NOT_SUPPORTED))
        {
            cerr << "CIMException during Creation of " << name.getString()
                << " in namespace " << targetNamespace.getString()
                << ": " << e.getMessage()
                << " CIM_Namespace Instance Creation error" << endl;
        }
        return(false);
    }
    catch(Exception& e)
    {
        cerr << "Exception during Creation of " << name.getString()
            << ": " << e.getMessage()
            << " CIM_Namespace Instance Creation error" << endl;
        return(false);
    }

    return(true);
}
Boolean InteropTest::_testPGNamespace(const CIMNamespaceName& name,
    Boolean shared, Boolean updatesAllowed, const String& parent)
{
    // get the instance
    // We only get PG Namespaces because these characteristics would
    // not exist for any CIMNamespace.
    // TODO: There should NOT be any CIMNamespaces so we should be able
    // to enumerate at that level successfully.
    Array<CIMObjectPath> paths =  _getPGNamespaceInstanceNames();
    CIMInstance instance;
    for (Uint32 i = 0 ; i < paths.size() ; i++ )
    {
        String testString = paths[i].toString();
        //TODO.poor test.
        if (testString.find("CIM_Namespace" != 0))
        {
            continue;
        }
        if (testString.find(name.getString()) != 0)
        {
            // get this instance.
            instance = _client.getInstance(PEGASUS_NAMESPACENAME_INTEROP,
                            paths[i],
                            false,                  //lo
                            true,                   //includeQualifiers
                            true,                   //includeClassOrigin
                            CIMPropertyList());
        }
        //
        // Match the properties in the instance against the method inputs.
        //
        String errorMsg ("NamespaceInstance: ");
        errorMsg.append(paths[i].toString());

        if ((instance.findProperty("IsShareable")) == PEG_NOT_FOUND)
        {
            cerr << errorMsg << ". Property IsShareable not found" << endl;
            return(false);
        }
        else
        {
            Boolean boolRtn = false;
            _getPropertyValue(instance,"IsShareable", false, boolRtn);
            if (boolRtn != shared)
            {
                cout << errorMsg << ". Error in sharing" << endl;
                return(false);
            }

        }
        // test for shared property
        if ((instance.findProperty("SchemaUpdatesAllowed")) == PEG_NOT_FOUND)
            cerr << errorMsg << "Property SchemaUpdatesAllowed not found"
                << endl;
        else
        {
            Boolean boolRtn = false;
            _getPropertyValue(instance,"SchemaUpdatesAllowed", false, boolRtn);
                if (boolRtn != updatesAllowed)
                {
                    cerr << errorMsg << ". Error in SchemaUpdatesAllowed"
                        << endl;
                    return(false);
                }
        }
        if ((instance.findProperty("ParentNamespace")) == PEG_NOT_FOUND)
            cerr << errorMsg << "Property " << "ParentNamespace"
                << " not found" << endl;
        else
        {
            String rtnParent;
            _getPropertyValue(instance, "ParentNamespace",
                    String("No parent"), rtnParent);
            if (parent != rtnParent )
            {
                cerr << errorMsg <<". Error in ParentNamespace" << endl;
                return(false);
            }
        }
    }
    return(true);
}

/** Create a single namespace using PG_Namespace. Creates namespace with the
    options for shareable, updatesAllowed, and parent.  This allows testing
    of creation of shared namespaces
*/
Boolean InteropTest::_namespaceCreatePG_Namespace(const CIMNamespaceName& name,
    const Boolean shareable, const Boolean updatesAllowed, const String& parent)
{
    // Does this namespace exist.
    if (_existsNew(name))
    {
        cout << "Namespace " << name.getString() <<
            " already Exists in _namespacCreatePG_Namespace function." << endl;
        return(false);
    }

    //Now build the new namespace name instance.  Note that we need to get the
    // collection of keys that are required. Easy way was to simply
    // use an existing instance and change the name field.
    Array<CIMInstance> instances = _getPGNamespaceInstances();

    if(instances.size() == 0)
    {
        return(false);
    }

    CIMClass thisClass = _client.getClass(PEGASUS_NAMESPACENAME_INTEROP,
                                        PG_NAMESPACE_CLASSNAME,
                                        false,true,true);

    CIMInstance instance = thisClass.buildInstance(true, false,
            CIMPropertyList());

    // Modify the name property value for new name
    Uint32 pos = instance.findProperty(NAMESPACE_PROPERTYNAME);
    if (pos == PEG_NOT_FOUND)
    {
        cerr << "Error in property on create. No "
            <<  NAMESPACE_PROPERTYNAME.getString() << " property" << endl;
        return false;
    }

    // Modify the name field in the instance
    String localName = name.getString();
    CIMProperty p = instance.getProperty(pos);

    p.setValue(localName);
    instance.removeProperty(pos);
    instance.addProperty(p);

    // Set the sharable, updates allowed, parent properties.
    pos = instance.findProperty("SchemaUpdatesAllowed");
    if (pos == PEG_NOT_FOUND)
    {
         cerr << "Error in property on create. No "
             <<  "SchemaUpdatesAllowed" << " property" << endl;
         return(false);
    }

    // Modify the field in the instance
    String localName1 = name.getString();
    CIMProperty p1 = instance.getProperty(pos);

    p1.setValue(updatesAllowed);
    instance.removeProperty(pos);
    instance.addProperty(p1);


    // Set the sharable, updates allowed, parent properties.
    pos = instance.findProperty("IsShareable");
    if (pos == PEG_NOT_FOUND)
    {
         cerr << "Error in property on create. No "
             <<  "IsShareable" << " property" << endl;
         return(false);
    }

    // Modify the field in the instance
    String localName2 = name.getString();
    CIMProperty p2 = instance.getProperty(pos);

    p1.setValue(shareable);
    instance.removeProperty(pos);
    instance.addProperty(p2);

    CDEBUG("Creating instance for " << localName << " in namespace "
            << PEGASUS_NAMESPACENAME_INTEROP);
    if (verbose)
    {
        cout << "Show instance used to do namespace create: " << endl;
        XmlWriter::printInstanceElement(instance);
    }
    try
    {
           CIMObjectPath path;
           path = _client.createInstance(PEGASUS_NAMESPACENAME_INTEROP,
                   instance);
    }
    catch(Exception& e)
    {
        cerr << "Error during Creation of " << name.getString()
            << ": " << e.getMessage()
            << " Instance Creation error" << endl;
        return(false);
    }

    return(true);
}

Boolean InteropTest::_namespaceDeleteCIM_Namespace(const CIMNamespaceName& name)
{
    // If does not exist, don't try to delete
    if (!_existsNew(name))
        return(false);

    // Get this instancepath from the whole set.
    Array<CIMObjectPath> paths = _getCIMNamespaceInstanceNames();

    // find the entry with the correct key
    Boolean found = false;
    Uint32 i;
    for (i = 0; i < paths.size() ; i++)
    {
        String n = _getKeyValue(paths[i], NAMESPACE_PROPERTYNAME);
        if (n == name.getString())
        {
            found = true;
            break;
        }
    }
    // This should not happen since we found it at top of this function
    if (!found)
    {
        cerr << "Namespace " << name.getString() << " not found" << endl;
        return(false);
    }

    // Delete the namespace
    try
    {
        _client.deleteInstance(PEGASUS_NAMESPACENAME_INTEROP, paths[i]);

    }
    catch(Exception& e)
    {
        cerr << "Error: " << e.getMessage()
            << " Instance deletion error  for " << name.getString() << endl;
        // Instead of this returns emptyexit(1);
        return(false);
    }
    return(true);
}

Boolean InteropTest::testClassExists(const CIMName & className)
{
    try
    {
        CIMClass thisClass = _client.getClass(PEGASUS_NAMESPACENAME_INTEROP,
                                            className,
                                            false,true,true);
        return(true);
    }

    catch(Exception& e)
    {
        cerr << "Error: " << e.getMessage() << " Class "
            << className.getString()
            << " does not exist in Namespace "
            << PEGASUS_NAMESPACENAME_INTEROP.getString() << endl;
    }
    return(false);
}



// This function registers providers dynamically required to test 
// PG_ProviderProfileCapability class
void _registerProvider(
    CIMClient& client,
    String provName,
    String className,
    CIMObjectPath& returnProvRef,
    CIMObjectPath& returnProvCapRef)
{
    CIMInstance cimProvInstance(PEGASUS_CLASSNAME_PROVIDER);
    CIMInstance cimProvCapInstance(PEGASUS_CLASSNAME_PROVIDERCAPABILITIES);

    cimProvInstance.addProperty(CIMProperty(
        CIMName("ProviderModuleName"),
        String("TestServerProfileProvidersModule")));
    cimProvInstance.addProperty(
        CIMProperty(CIMName("Name"),
        provName));

    returnProvRef =
        client.createInstance(PEGASUS_NAMESPACENAME_INTEROP, cimProvInstance);

    Array <String> namespaces;
    Array <Uint16> providerType;
    Array <String> supportedMethods;
    Array <String> supportedProperties;
    namespaces.append("test/TestProvider");
    providerType.append(2);

    cimProvCapInstance.addProperty(CIMProperty(
        CIMName("ProviderModuleName"),
        String("TestServerProfileProvidersModule")));
    cimProvCapInstance.addProperty(CIMProperty(
        CIMName("ProviderName"),
        provName));
    cimProvCapInstance.addProperty(CIMProperty(
        CIMName("CapabilityID"),
        className));
    cimProvCapInstance.addProperty(CIMProperty(
        CIMName("ClassName"),
        className));
    cimProvCapInstance.addProperty(CIMProperty(
        CIMName("Namespaces"),
        namespaces));
    cimProvCapInstance.addProperty(CIMProperty(
        CIMName("ProviderType"),
        providerType));
    cimProvCapInstance.addProperty(CIMProperty(
        CIMName("SupportedMethods"),
        supportedMethods));
    cimProvCapInstance.addProperty(CIMProperty(
        CIMName("SupportedProperties"),
        supportedProperties));

    returnProvCapRef = client.createInstance(
        PEGASUS_NAMESPACENAME_INTEROP,
        cimProvCapInstance);

}

// Create ProviderProfileCapability instance for main profile and test it
void _testProvProfCapInstCreation(
    CIMClient &client,
    CIMObjectPath &provProfCapRef)
{
    String capId;
    CIMInstance cimInstProvProfCap(
        PEGASUS_CLASSNAME_PG_PROVIDERPROFILECAPABILITIES);

    Array<Uint16> registeredSubProfiles;
    registeredSubProfiles.append(0); //Other
    registeredSubProfiles.append(0); //Other

    Array<String> subProfileVersions;
    subProfileVersions.append("1.1.0");
    subProfileVersions.append("0.1.0");

    Array<String> otherRegisteredSubProfiles;
    otherRegisteredSubProfiles.append("DynamicSubprofile1");
    otherRegisteredSubProfiles.append("DynamicSubprofile2");

    Array<String> otherSubProfileOrganizations;
    otherSubProfileOrganizations.append("OpenPegasus");
    otherSubProfileOrganizations.append("OpenPegasus");

    Array<String> subProfileProviderModuleNames;
    subProfileProviderModuleNames.append("TestServerProfileProvidersModule");
    subProfileProviderModuleNames.append("TestServerProfileProvidersModule");

    Array<String> subProfileProviderNames;
    subProfileProviderNames.append("TestDynamicSubProfile1CapabilityProvider");
    subProfileProviderNames.append("TestDynamicSubProfile2CapabilityProvider");

    Array<String> conformingElements;
    conformingElements.append("Test_DynamicProfile");

    cimInstProvProfCap.addProperty(CIMProperty(
        "ProviderModuleName",
        String("TestServerProfileProvidersModule")));
    cimInstProvProfCap.addProperty(CIMProperty(
        "ProviderName",
        String("TestDynamicProfileCapabilityProvider")));
    cimInstProvProfCap.addProperty(CIMProperty(
        "CapabilityID",
        String("DynamicProfileCapability")));
    cimInstProvProfCap.addProperty(
        CIMProperty("RegisteredProfile", Uint16(0)));
    cimInstProvProfCap.addProperty(
        CIMProperty("OtherRegisteredProfile", String("DynamicProfile")));
    cimInstProvProfCap.addProperty(
        CIMProperty("ProfileVersion", String("0.1.0")));
    cimInstProvProfCap.addProperty(
        CIMProperty("OtherProfileOrganization", String("OpenPegasus")));
    cimInstProvProfCap.addProperty(
        CIMProperty("RegisteredSubProfiles", registeredSubProfiles));
    cimInstProvProfCap.addProperty(
        CIMProperty("SubprofileVersions", subProfileVersions));
    cimInstProvProfCap.addProperty(
        CIMProperty("OtherRegisteredSubProfiles", otherRegisteredSubProfiles));
    cimInstProvProfCap.addProperty(CIMProperty(
        "OtherSubProfileOrganizations",
        otherSubProfileOrganizations));
    cimInstProvProfCap.addProperty(CIMProperty(
        "SubProfileProviderModuleNames",
        subProfileProviderModuleNames));
    cimInstProvProfCap.addProperty(
        CIMProperty("SubProfileProviderNames", subProfileProviderNames));
    cimInstProvProfCap.addProperty(
        CIMProperty("ConformingElements", conformingElements));
    provProfCapRef = client.createInstance(
        PEGASUS_NAMESPACENAME_INTEROP,
        cimInstProvProfCap);
    // Test to see if the ProviderProfileCapability instance is created 
    // properly
    CIMInstance instance = client.getInstance(
        PEGASUS_NAMESPACENAME_INTEROP,
        provProfCapRef);
    instance.getProperty(
        instance.findProperty("CapabilityId")).getValue().get(capId);
    PEGASUS_TEST_ASSERT(capId == "DynamicProfileCapability");
    
    // Enumerate PG_ProviderProfileCapability instances
    Array<CIMObjectPath> pathsProfCap = client.enumerateInstanceNames(
        PEGASUS_NAMESPACENAME_INTEROP,
        PEGASUS_CLASSNAME_PG_PROVIDERPROFILECAPABILITIES);

    PEGASUS_TEST_ASSERT(pathsProfCap.size() > 0);
    Uint32 i = 0;
    for (Uint32 n = pathsProfCap.size(); i < n; i++)
    {
        if (pathsProfCap[i] == provProfCapRef)
        {
            break;
        }
    }
    PEGASUS_TEST_ASSERT(i < pathsProfCap.size()); 
}

// Test dynamic profile associations using 
// CIM_ElementConformsToProfile association instances and 
// PG_ElementSoftwareIdentity association instances
void _testDynamicProfileAssociations(CIMClient &client)
{
    Array<CIMInstance> instances = client.enumerateInstances(
        PEGASUS_NAMESPACENAME_INTEROP, "CIM_RegisteredProfile");

    String registeredName;
    Array<CIMObject> associators;
    String name;
    Uint32 count = 0;
    for (Uint32 i = 0, n = instances.size(); i < n; i++)
    {
        instances[i].getProperty(instances[i].findProperty("RegisteredName")).
            getValue().get(registeredName);

        if (String::equal(registeredName, "DynamicProfile") ||
            String::equal(registeredName, "DynamicSubprofile1") ||
            String::equal(registeredName, "DynamicSubprofile2"))
        {
            count++;
            if (String::equal(registeredName, "DynamicProfile"))
            {
                associators = client.associators(PEGASUS_NAMESPACENAME_INTEROP,
                    instances[i].getPath(),
                    PEGASUS_CLASSNAME_CIM_ELEMENTCONFORMSTOPROFILE);
                PEGASUS_TEST_ASSERT(associators.size() == 2);
            }
            associators = client.associators(
                PEGASUS_NAMESPACENAME_INTEROP,
                instances[i].getPath(),
                PEGASUS_CLASSNAME_PG_ELEMENTSOFTWAREIDENTITY);
            PEGASUS_TEST_ASSERT(associators.size() == 1);
        }
    }
    PEGASUS_TEST_ASSERT(count == 3);
}

// Test to cover error conditions in InteropProvider::invokeMethod 
void _testInvokeMethodErrorConditions(
    CIMClient &client,
    CIMObjectPath& provProfCapRef)
{
    Array<CIMParamValue> inParams;
    Array<CIMParamValue> outParams;

    // Passing wrong function name
    const CIMName methodName1("update");
    const CIMName methodName2("updateCache");
    try
    {
        CIMValue returnValue = client.invokeMethod(
            PEGASUS_NAMESPACENAME_INTEROP,
            provProfCapRef,
            methodName1,
            inParams,
            outParams);
    }
    catch(CIMException& e)
    {
        PEGASUS_TEST_ASSERT(e.getCode() == CIM_ERR_NOT_SUPPORTED);
    }

    // Passing unsupported Class name to InteropProvider::invokeMethod 
    CIMObjectPath pgNamespaceObjPath;
    pgNamespaceObjPath.setNameSpace("root/PG_Interop");
    pgNamespaceObjPath.setClassName(PEGASUS_CLASSNAME_PGNAMESPACE);
    try
    {
        CIMValue returnValue = client.invokeMethod(
            PEGASUS_NAMESPACENAME_INTEROP,
            pgNamespaceObjPath,
            methodName2,
            inParams,
            outParams);
    }
    catch(CIMException& e)
    {
        PEGASUS_TEST_ASSERT(e.getCode() == CIM_ERR_NOT_SUPPORTED);
    }
}

//
// Tests the various operations with PG_ProviderProfileCapability class
//
void InteropTest::testPGProviderProfileCapabilities()
{
    CIMObjectPath retRefProvProfCap;
    CIMObjectPath retRefProv, retRefProvCap;
    CIMObjectPath retRefProv1, retRefProvCap1;
    CIMObjectPath retRefProv2, retRefProvCap2;

    // Register required provider for main profile
    _registerProvider(_client,
        "TestDynamicProfileCapabilityProvider",
        "Test_DynamicProfile",
        retRefProv,
        retRefProvCap);

    // Register required providers for sub profiles
    _registerProvider(_client,
        "TestDynamicSubProfile1CapabilityProvider",
        "Test_DynamicSubProfile1",
        retRefProv1,
        retRefProvCap1);
    _registerProvider(_client,
        "TestDynamicSubProfile2CapabilityProvider",
        "Test_DynamicSubProfile2",
        retRefProv2,
        retRefProvCap2);

    // Create PG_ProviderProfileCapability instance and test its successful
    // creation
    _testProvProfCapInstCreation(_client, retRefProvProfCap);
    
    // Test dynamic profile associations using
    // CIM_ElementConformsToProfile association instances and 
    // PG_ElementSoftwareIdentity association instances
    _testDynamicProfileAssociations(_client);

    // Test to cover error conditions in InteropProvider::invokeMethod 
    _testInvokeMethodErrorConditions(_client, retRefProvProfCap);
   
    //Test to check successful deletion of PG_ProviderProfileCapability 
    //instance
    _client.deleteInstance(
        PEGASUS_NAMESPACENAME_INTEROP,
        retRefProvProfCap);
  
    try
    {
        CIMInstance instance = _client.getInstance(
            PEGASUS_NAMESPACENAME_INTEROP,
            retRefProvProfCap);
    }
    catch (CIMException& e)
    {
        PEGASUS_TEST_ASSERT(e.getCode() == CIM_ERR_NOT_FOUND);
    }

    _client.deleteInstance(
        PEGASUS_NAMESPACENAME_INTEROP,
        retRefProv);
    _client.deleteInstance(
        PEGASUS_NAMESPACENAME_INTEROP,
        retRefProv1);
    _client.deleteInstance(
        PEGASUS_NAMESPACENAME_INTEROP,
        retRefProv2);
}

/* Execute the tests to assure that the namespace manager works and
   that the results match what is returned from the __Namespace manager
*/
void InteropTest::testNameSpacesManagement()
{
    // Test for required classes to assure that the interop provider and
    // its corresponding classes exist.
    PEGASUS_TEST_ASSERT(testClassExists(CIM_NAMESPACE_CLASSNAME));
    PEGASUS_TEST_ASSERT(testClassExists(PG_NAMESPACE_CLASSNAME));
    PEGASUS_TEST_ASSERT(testClassExists(CIM_OBJECTMANAGER_CLASSNAME));
    PEGASUS_TEST_ASSERT(testClassExists(
                CIM_OBJECTMANAGERCOMMUNICATIONMECHANISM_CLASSNAME));
    PEGASUS_TEST_ASSERT(
            testClassExists(PG_CIMXMLCOMMUNICATIONMECHANISM_CLASSNAME));
    PEGASUS_TEST_ASSERT(testClassExists(CIM_COMMMECHANISMFORMANAGER_CLASSNAME));

    // Test namespace usage with both __namespace and CIM_Namespace.
    // __namespace is called old
    // CIM_Namespace is called new for these tests.
    Array<CIMNamespaceName> nameListOld = _getNamespacesOld();

    CDEBUG("Got Namespaces with __Namespace. Now Validate");
    if (!_validateNamespaces(nameListOld))
    {
        cout << "Error exit, Invalid namespace returned" << endl;
    }
    Array<CIMNamespaceName> nameListNew;
    nameListNew = _getNamespacesNew();

    CDEBUG("Got Namespaces with CIM_Namespace. Now Validate");
    if (!_validateNamespaces(nameListNew))
    {
        cout << "Error exit, Invalid namespace returned" << endl;
    }

    BubbleSort(nameListNew);
    BubbleSort(nameListOld);
    if(verbose) {
        _showNamespaceList(nameListNew, "Using CIM_Namespace");
        _showNamespaceList(nameListOld, "Using __Namespace");
    }

    PEGASUS_TEST_ASSERT(nameListNew.size() == nameListOld.size());

    // Add assertion that they have the same items in the list
    for (Uint32 i = 0 ; i < nameListNew.size() ; i++)
    {
        PEGASUS_TEST_ASSERT(nameListNew[i] == nameListOld[i]);
    }
    // Create a new namespace with new functions

    CIMNamespaceName testNameNew =
        CIMNamespaceName("root/junk/interoptest/newtype");
    CIMNamespaceName testNameOldRoot = CIMNamespaceName("root");
    CIMNamespaceName testNameOldTail =
        CIMNamespaceName("junk/interoptest/oldtype");
    CIMNamespaceName testNameOldComplete =
        CIMNamespaceName("root/junk/interoptest/oldtype");

    // Create the namespace with the PG_Namespace class
    // Note that this is an assertion and, in fact, we should probably
    // remove the names to clean up the repository after the test.
    // If they do exist here. TODO

    // Clean up the names we will use just to be sure.
    if (_existsNew(testNameNew)) {
        _namespaceDeleteCIM_Namespace(testNameNew);
    }

    if (_existsNew(testNameOldComplete)) {
        _namespaceDeleteCIM_Namespace(testNameOldComplete);
    }

    PEGASUS_TEST_ASSERT( ! _existsNew(testNameNew));

    PEGASUS_TEST_ASSERT( ! _existsNew(testNameOldComplete));

    CDEBUG("Create New Namespace with PG_Namespace. Namespace name = "
            << testNameNew.getString() << ".");

    _namespaceCreatePG_Namespace(CIMNamespaceName(testNameNew));

    if (verbose)
    {
        _showNamespaceList(nameListNew,
                "CIM_Namespace response after add. with PG_Namespace");
    }

    CDEBUG("Test for namespace created. Name = " << testNameNew.getString());
    PEGASUS_TEST_ASSERT(_existsNew(testNameNew));

    PEGASUS_TEST_ASSERT(
            _namespaceDeleteCIM_Namespace(CIMNamespaceName(testNameNew)));

    CDEBUG("Test for Namespace existing = " << testNameOldComplete.getString());
    if(_existsNew(CIMNamespaceName(testNameOldComplete)))
       _namespaceDeleteCIM_Namespace(CIMNamespaceName(testNameOldComplete));

    CDEBUG("Test for Namespace NOT existing = "
            << testNameOldComplete.getString());
    if(_existsNew(testNameOldComplete))
        cerr << "Problem deleting namespace" << testNameOldComplete.getString()
            <<endl;

    //
    // Repeat the creation/deletion test with the CIM_Namespace class.
    //

    PEGASUS_TEST_ASSERT( ! _existsNew(testNameNew));

    CDEBUG("Now Create New Namespace with CIM_Namespace. Namespace name = " <<
        testNameNew.getString() << ".");

    PEGASUS_TEST_ASSERT(_namespaceCreateCIM_Namespace(
        CIMNamespaceName(testNameNew), PEGASUS_NAMESPACENAME_INTEROP));

    if (verbose)
        _showNamespaceList(nameListNew,
                "CIM_Namespace response after add. with PG_Namespace");

    CDEBUG("Test for namespace created. Name = " << testNameNew.getString());
    PEGASUS_TEST_ASSERT(_existsNew(testNameNew));

    PEGASUS_TEST_ASSERT(
            _namespaceDeleteCIM_Namespace(CIMNamespaceName(testNameNew)));

    PEGASUS_TEST_ASSERT( ! _existsNew(testNameNew));

    //
    // Create namespace with __Namespace
    //
    CDEBUG("Creating with __Namespace appending "
            << testNameOldTail.getString() << " to "
            << testNameOldRoot.getString());
    _namespaceCreate__Namespace(CIMNamespaceName(testNameOldRoot),
            String(testNameOldTail.getString()));

    PEGASUS_TEST_ASSERT(_existsNew(testNameOldComplete));

    PEGASUS_TEST_ASSERT(
        _namespaceDeleteCIM_Namespace(CIMNamespaceName(testNameOldComplete)));

    PEGASUS_TEST_ASSERT(!_existsNew(testNameOldComplete));

    if (verbose)
    {
        _showNamespaceList(nameListNew,
                "Namespaces From CIM_Namespace after add.");
    }

    _namespaceCreate__Namespace(CIMNamespaceName(testNameOldRoot),
            String(testNameOldTail.getString()));

    // Note that this tries to delete the multiple levels all at the same time.

    // ATTN: There is apparently a problem here with the following
    // delete.  Fix this and retest.  For the moment, we substituted
    // Deleting the new way.
    //_deleteNamespaceOld(String(testNameOldComplete.getString()));

    _namespaceDeleteCIM_Namespace(testNameOldComplete);

    if (verbose)
    {
        _showNamespaceList(nameListNew, "CIM_Namespace response after add.");
    }
    PEGASUS_TEST_ASSERT(!_existsNew(testNameOldComplete));

    if(_existsNew(CIMNamespaceName(testNameOldComplete)))
       _namespaceDeleteCIM_Namespace(CIMNamespaceName(testNameOldComplete));

    if(_existsNew(CIMNamespaceName(testNameOldComplete)))
        cerr << "Problem deleting namespace" << endl;

    //
    // Test to be sure we cannot create new namespaces anywhere but in the
    // Interop namespace
    //
    Array<CIMNamespaceName> namespaceList = _getNamespacesNew();
    for (Uint32 i = 0 ; i < namespaceList.size() ; i++)
    {
        // The get class test is here simply as a means to assure that no
        // instances exist in the any namespace except the interop namespace
        if (!(namespaceList[i] == PEGASUS_NAMESPACENAME_INTEROP))
        {
            // Error if we can successfully create namespace.
            if (_namespaceCreateCIM_Namespace(CIMNamespaceName(testNameNew),
                        namespaceList[i]))
            {
                cout << "Error, Created new CIM_Namespace "
                    << testNameNew.getString()
                    << " instance in " << namespaceList[i].getString() << endl;
                TERMINATE("Failed test by creating new namespace "
                        "outside of Interop namespace");
            }
        }
    }
    // Test to be sure that we have the same count of namespaces
    //as when we started.  Should also check to be sure it is exactly the
    //same set of namespaces.
    Array<CIMNamespaceName> finalNamespaceList = _getNamespacesNew();
    if (finalNamespaceList.size() !=  nameListNew.size())
    {
        BubbleSort(finalNamespaceList);
        BubbleSort(nameListNew);
        _showNamespaceList(nameListNew, " Namespaces Before Namespace test");
        _showNamespaceList(finalNamespaceList,
                " Namespaces After Namespace test");
        TERMINATE("Error. Did not clean all namespaces");
    }

    if (verbose)
        cout << "Basic Namespace Tests passed" << endl;
}

//****************************************************************
// Test characteristics of shared namespaces.
//***************************************************************
void InteropTest::testSharedNameSpacesManagement()
{
    try
    {
        Array<CIMNamespaceName> nameListBefore = _getNamespacesNew();

        // We will add the following new namespaces
        CIMNamespaceName testNameSharable =
            CIMNamespaceName("root/junk/interoptest/sharable");
        CIMNamespaceName testNameShared =
            CIMNamespaceName("root/junk/interoptest/shared");
        CIMNamespaceName testNameNotShared =
            CIMNamespaceName("root/junk/interoptest/notshared");

        // Create a sharable namespace
        _namespaceCreatePG_Namespace(testNameSharable, true, false,
                String::EMPTY);

        // create a namespace with the previous sharable as parent.
        _namespaceCreatePG_Namespace(testNameShared, false, false,
                testNameSharable.getString());

        // create a namespace with the previous sharable as parent.
        PEGASUS_TEST_ASSERT(_namespaceCreateCIM_Namespace(testNameNotShared,
                    PEGASUS_NAMESPACENAME_INTEROP));
        // Confirm that both exist
        PEGASUS_TEST_ASSERT(_existsNew(testNameSharable));
        PEGASUS_TEST_ASSERT(_existsNew(testNameShared));
        PEGASUS_TEST_ASSERT(_existsNew(testNameNotShared));

        if (verbose)
            _showNamespaceInfo(
                    "Namespaces with one shared and one sharable created");

        // Note: This only tests that the namespaces are marked shareable,
        // shared.  This is not a test to insure that the sharing logic works.
        PEGASUS_TEST_ASSERT(_testPGNamespace(testNameSharable, true, false,
                    String::EMPTY));
        PEGASUS_TEST_ASSERT(_testPGNamespace(testNameShared, false, false,
                    testNameSharable.getString()));
        PEGASUS_TEST_ASSERT(_testPGNamespace(testNameShared, false, false,
                    String::EMPTY));

        // Now delete the two namespaces
        _namespaceDeleteCIM_Namespace(testNameSharable);
        _namespaceDeleteCIM_Namespace(testNameShared);
        _namespaceDeleteCIM_Namespace(testNameNotShared);

        PEGASUS_TEST_ASSERT(!_existsNew(testNameSharable));
        PEGASUS_TEST_ASSERT(!_existsNew(testNameShared));
        PEGASUS_TEST_ASSERT(!_existsNew(testNameNotShared));

        Array<CIMNamespaceName> nameListAfter = _getNamespacesNew();
        if (nameListBefore.size() != nameListAfter.size())
        {
            cout << "SharedNamespace sizes " << nameListBefore.size()
                << " " << nameListAfter.size() << endl;
            BubbleSort(nameListBefore);
            BubbleSort(nameListAfter);
            _showNamespaceList(nameListBefore, "Before");
            _showNamespaceList(nameListAfter, "After");
            PEGASUS_TEST_ASSERT(nameListBefore.size() == nameListAfter.size());
        }
    }
    // Catch block for all of the shared Namespace tests Tests.
    catch(CIMException& e)
    {
        TERMINATE(" Shared Namespace test CIMException: " << e.getMessage());
    }
    catch(Exception& e)
    {
        TERMINATE(" Shared Namespace test Pegasus Exception: "
                << e.getMessage());
    }
    catch(...)
    {
        TERMINATE(" Shared Namespace test Caught General Exception:");
    }
    if (verbose)
        cout << "Shared Namespace Tests passed" << endl;
}

/** get the single instance of the object manager class. Note that this function
    is based on the concept that there is only a single instance of this class
    despite the fact that we have no reason to really control this.  The Interop
    provider generally controls this so we assume it.  There is an assert in
    the test so that we get an error if there is more than one instance.
    @return the single instance of the CIM_ObjectManager class.
*/
CIMInstance InteropTest::getInstanceObjMgr()
{
    Array<CIMInstance> instancesObjMgr = _client.enumerateInstances(
                                         PEGASUS_NAMESPACENAME_INTEROP,
                                         CIM_OBJECTMANAGER_CLASSNAME,
                                         false,  // di = false
                                         false,  // localOnly = false
                                         false,  // include qualifiers = false
                                         false, CIMPropertyList());

    PEGASUS_TEST_ASSERT(instancesObjMgr.size() == 1);

    return(instancesObjMgr[0]);
}

CIMObjectPath InteropTest::getObjMgrPath()
{
    CIMInstance instance = getInstanceObjMgr();
    return(instance.getPath());
}

//****************************************************************
// Test the characteristics of the CIM_ObjectManager Class and Instances
//****************************************************************
void InteropTest::testObjectManagerClass()
{
    if (verbose)
        cout << "testOjectManagerClass() test Start." << endl;
    try
    {

        // The get class test is here simply as a means to assure that the
        // class exists in the defined namespace.
        CIMClass objectManagerClass =
            _client.getClass(
                PEGASUS_NAMESPACENAME_INTEROP,
                CIM_OBJECTMANAGER_CLASSNAME,
                false,
                true,
                true);

        if (verbose)
        {
            cout << "Show the object manager Class element" << endl;
            XmlWriter::printClassElement(objectManagerClass);
        }

        // Test the CIM_ObjectManager Object

        CIMInstance instanceObjectManager =  getInstanceObjMgr();

        if (verbose)
        {
            cout << "Show the object manager instance element" << endl;
            XmlWriter::printInstanceElement(instanceObjectManager);
        }
        // Rebuild the path from the instance
        CIMObjectPath rebuiltObjectManagerPath =
            instanceObjectManager.buildPath(objectManagerClass);

        // test to confirm that both names and instances return same thing.
        Array<CIMObjectPath> pathsObjMgr = _client.enumerateInstanceNames(
                                                 PEGASUS_NAMESPACENAME_INTEROP,
                                                 CIM_OBJECTMANAGER_CLASSNAME);

        // Again assert that there is only one instance
        PEGASUS_TEST_ASSERT(pathsObjMgr.size() == 1);
        CIMObjectPath nameEnumObjectManagerPath = pathsObjMgr[0];

        CIMObjectPath rcvdObjectManagerPath = instanceObjectManager.getPath();

        if (verbose)
        {
        cout << "Object Manager path from enumerateInstancesNames: "
            << nameEnumObjectManagerPath.toString() << endl << endl
             << "Object manager path from enumerateInstances bld : "
             << rebuiltObjectManagerPath.toString() << endl <<endl
             << "Object Manager path form enumerateInstances path: "
             << rcvdObjectManagerPath.toString() << endl   ;
        }

        // compare locally built path from instance with path from
        // enumerateInstances
        PEGASUS_TEST_ASSERT(rcvdObjectManagerPath == nameEnumObjectManagerPath);
        PEGASUS_TEST_ASSERT (rcvdObjectManagerPath == rebuiltObjectManagerPath);
        PEGASUS_TEST_ASSERT (nameEnumObjectManagerPath ==
                rebuiltObjectManagerPath);

        // Test existence of properties in instance returned..
        // NOTE This is a duplication of the loop for all properties test below.

        PEGASUS_TEST_ASSERT (
                instanceObjectManager.findProperty("gatherstatisticaldata") !=
                    PEG_NOT_FOUND);
        PEGASUS_TEST_ASSERT (
                instanceObjectManager.findProperty("Name") !=
                    PEG_NOT_FOUND);
        PEGASUS_TEST_ASSERT (
                instanceObjectManager.findProperty("ElementName") !=
                    PEG_NOT_FOUND);
        PEGASUS_TEST_ASSERT (
                instanceObjectManager.findProperty("CreationClassName") !=
                    PEG_NOT_FOUND);
        PEGASUS_TEST_ASSERT (
                instanceObjectManager.findProperty("SystemName") !=
                    PEG_NOT_FOUND);
        PEGASUS_TEST_ASSERT (
                instanceObjectManager.findProperty("SystemCreationClassName") !=
                    PEG_NOT_FOUND);

        for (Uint32 i = 0 ; i < objectManagerClass.getPropertyCount() ; i++)
        {
            CIMConstProperty pl = objectManagerClass.getProperty(i);
            CIMName propertyName = pl.getName();
            PEGASUS_TEST_ASSERT(
                    instanceObjectManager.findProperty(propertyName) !=
                    PEG_NOT_FOUND);
        }

        // Add a test for the different get parameters on instances.

        // Note that we have no way of testing the persistence of the object
        // ID at this point.

        // test get instance.
        CIMInstance instance =
            _client.getInstance(PEGASUS_NAMESPACENAME_INTEROP,
            rcvdObjectManagerPath);

        // TODO: add test for equality

        // TODO: Add test for get bad object.
    }

    // Catch block for all of the CIM_ObjectManager Tests.
    catch(CIMException& e)
    {
        TERMINATE(" CIM_ObjectManager CIMException: " << e.getMessage());
    }
    catch(Exception& e)
    {
        TERMINATE(" CIM_ObjectManager Pegasus Exception: " << e.getMessage());
    }
    catch(...)
    {
        TERMINATE(" CIM_ObjectManager Caught General Exception:");
    }

    //
    //  Test to be sure we cannot get instances of this class from another
    //  namespace
    //
    Boolean errFound = false;
        Array<CIMNamespaceName> namespaceList = _getNamespacesNew();

        // for all namespaces not the interop namespace, try to get
        // CIM_ObjectManager results should always be zero.
        for (Uint32 i = 0 ; i < namespaceList.size() ; i++)
        {
            // for all namespaces except interop, try to enumerate
            // instances of cimobjectmanager.
            if (namespaceList[i] != PEGASUS_NAMESPACENAME_INTEROP)
            {
                try
                {
                    Array <CIMObjectPath> paths =
                        _client.enumerateInstanceNames(
                            namespaceList[i],
                            CIM_OBJECTMANAGER_CLASSNAME);
                    PEGASUS_TEST_ASSERT(paths.size() == 0);
                }
                // Catch block for this enum test.  We test class not exist.
                catch(CIMException& e)
                {
                    if ((e.getCode() != CIM_ERR_INVALID_CLASS) &&
                            (e.getCode() != CIM_ERR_NOT_SUPPORTED))
                    {
                        cout << "Unexpected CIMException " << e.getMessage()
                            << "namespace " << namespaceList[i].getString()
                            << " EnumerateInstances of CIMObjectManager "
                            << endl;
                        errFound = true;
                    }
                }
                catch(Exception& e)
                {
                    errFound= true;
                    cout <<
                        "Unexpected Exception in look for cimobject manager "
                            "in Namespace "
                        << namespaceList[i].getString() << ". " 
                        << e.getMessage() << endl;
                }
                catch(...)
                {
                    errFound = true;
                    cout << "Unexpected Exception in look for cimobject"
                            " manager in namespace "
                        << namespaceList[i].getString()
                        << endl;
                }
            }
        }
        if (errFound)
        {
            TERMINATE("Error: CImObjectManager instance should not exist "
                    "other than interop namespace");
        }
    if (verbose)
        cout << "ObjectManagerClass Tests passed" << endl;
}

/* Set the CIM_ObjectManager gatherStatisticaldata property in the
   server to the state defined by flag.
   @flag Boolean - state to set.
*/
void InteropTest::setStatisticsState(const Boolean flag)
{
    CIMInstance instanceObjectManagr = getInstanceObjMgr();

    CIMInstance sendInstance = instanceObjectManagr.clone();

    // Filter out everything but our property.
    Array<CIMName> plA;
    plA.append(CIMName("gatherstatisticaldata"));
    CIMPropertyList myPropertyList(plA);

    Uint32 pos;
    if ((pos = sendInstance.findProperty("gatherstatisticaldata")) !=
            PEG_NOT_FOUND)
        sendInstance.getProperty(pos).setValue(CIMValue(flag));
    else
    {
        TERMINATE("gatherstatisticaldata property not found in "
                "setStatisticsState function");
    }
    try
    {
    _client.modifyInstance(PEGASUS_NAMESPACENAME_INTEROP,
                        sendInstance,
                        false,
                        myPropertyList);
    }
    catch(CIMException& e)
    {
        cout <<" setStatistics CIMException: " << e.getMessage() << endl;
    }
}

Boolean InteropTest::getStatisticsPropertyState(CIMInstance & objMgrInstance)
{
    Boolean output;
    Uint32 pos;
    if ((pos = objMgrInstance.findProperty("gatherstatisticaldata")) !=
            PEG_NOT_FOUND)
    {
        CIMConstProperty p1 = objMgrInstance.getProperty(pos);
        if (p1.getType() == CIMTYPE_BOOLEAN)
        {
            CIMValue v1  = p1.getValue();
            if (!v1.isNull())
                v1.get(output);
            else
                output = false;
        }
        else
            TERMINATE("gatherStatisticaldata Property type problem");
    }
    else
        TERMINATE("gatherStatisticaldata Property Not found");
    return(output);
}

Boolean InteropTest::getStatisticsState()
{
    CIMInstance objMgrInstance = getInstanceObjMgr();
    return(getStatisticsPropertyState(objMgrInstance));
}

void InteropTest::showStatisticsState()
{
    cout << "Statistics State = " << (getStatisticsState()? "on" : "off")
        << endl;

}

/** try the modify operation.  Will return true if it passes and false if
    it fails with any error code other than the one defined. This function
    is used to test primarily the error functions of modify instance.
    @param modifiedIns CIMInstance with modifications
    @param list CIMPropertyList of properties to modify
    @param includeQualifiers Boolean that defines whether qualifiers
    are to be modified
    @expectedCode CIMStatusCode defining expected error code.
    @return True if the modification fails AND the expected error is received.
*/
Boolean InteropTest::testStatisticsSetOperationError(
                    const CIMInstance & modifiedIns,
                    const CIMPropertyList& list,
                    Boolean shouldRespondGood,
                    Boolean includeQualifiers,
                    const CIMStatusCode expectedCode)
{
    try
    {
        // modify with full instance but propertylist set to mod
        // only this property.
        _client.modifyInstance(PEGASUS_NAMESPACENAME_INTEROP,
                            modifiedIns,
                            includeQualifiers,
                            list);
        if (shouldRespondGood)
        {
            return(true);
        }
        else
            return(false);  // return false. modification was made
    }
    catch(CIMException& e)
    {
        if (shouldRespondGood)
        {
            // unexpected Error hit.
            return(false);
        }
        else
        {
            if (_checkExceptionCode(e, expectedCode))
                return (true);
            else
                return(false);
        }
    }
}
/** test of the function to enable and disable the boolean statistics
    property in CIMObjectManager using modify instance.
    This tests both correct modification and error cases.
*/
void InteropTest::testStatisticsEnable()
{
    try
    {
        //
        //  Test normal set to true and set to false operations
        //
        setStatisticsState(true);
        // Get Object Manager instance and confirm property changed.
        PEGASUS_TEST_ASSERT(getStatisticsState());
        setStatisticsState(false);
        // Get Object Manager instance and confirm property changed.
        PEGASUS_TEST_ASSERT(!getStatisticsState());

        //
        //  Test modify with multiple properties in modifiedIns.
        //
        // Get original instance for following tests
        CIMInstance instanceObjectManager = getInstanceObjMgr();

        // Create property list that represents correct request
        Array<CIMName> plA;
        plA.append(CIMName("gatherstatisticaldata"));
        CIMPropertyList myPropertyList(plA);

        // Get Object Manager instance as basis for following tests
        CIMInstance sendInstance = instanceObjectManager.clone();

        Uint32 pos;
        if ((pos = sendInstance.findProperty("gatherstatisticaldata")) !=
                PEG_NOT_FOUND)
            sendInstance.getProperty(pos).setValue(CIMValue(true));
        else
            PEGASUS_TEST_ASSERT(false);

        // try modify with multiple properties . Should set statistics true
        if(!testStatisticsSetOperationError(sendInstance, myPropertyList,
                true, false, CIM_ERR_NOT_SUPPORTED))
           TERMINATE("Failed to Set Statistics true");
        PEGASUS_TEST_ASSERT(getStatisticsState());

        // test with multiple properties in instance and qualifiers in instance
        sendInstance = instanceObjectManager.clone();
        if ((pos = sendInstance.findProperty("gatherstatisticaldata")) !=
                PEG_NOT_FOUND)
            sendInstance.getProperty(pos).setValue(CIMValue(false));
        else
            PEGASUS_TEST_ASSERT(false);
        // following should turn statistics off
        if(!testStatisticsSetOperationError(sendInstance, myPropertyList,
                true, false, CIM_ERR_NOT_SUPPORTED))
           TERMINATE("Failed to Set Statistics false");
        PEGASUS_TEST_ASSERT(!getStatisticsState());

        // now assure ourselves that the instances are the same.
        //CIMInstance newInstanceObjectManager = getInstanceObjMgr();
        PEGASUS_TEST_ASSERT(
                instanceObjectManager.identical(getInstanceObjMgr()));

        //
        // Now test possible  modify operations that should fail.
        //
        sendInstance = instanceObjectManager.clone();

        if ((pos = sendInstance.findProperty("gatherstatisticaldata")) !=
                PEG_NOT_FOUND)
            sendInstance.getProperty(pos).setValue(CIMValue(true));
        else
            PEGASUS_TEST_ASSERT(false);

        // Should fail because property list is null
        if(!testStatisticsSetOperationError(sendInstance, CIMPropertyList(),
                                    false, false, CIM_ERR_NOT_SUPPORTED))
           TERMINATE("Set Should fail. CIMPropertyList Null not allowed");
        PEGASUS_TEST_ASSERT(!getStatisticsState());

        // PropertyList contains one property but NOT statistics. It should fail
        Array<CIMName> plA2;
        plA2.append(CIMName("RequestStateChange"));
        CIMPropertyList myPropertyList2(plA2);
        CIMInstance sendInstance2 = sendInstance.clone();
        if(!testStatisticsSetOperationError(sendInstance, myPropertyList2,
                                    false, false, CIM_ERR_NOT_SUPPORTED))
        TERMINATE("Set Should fail. Bad Property in modifiedInstance");
        PEGASUS_TEST_ASSERT(!getStatisticsState());

        // Try with MyProperty list empty.  This should return good but
        // no change
        Array<CIMName> plA3;
        myPropertyList2.set(plA3);
        sendInstance2 = sendInstance.clone();
        if(!testStatisticsSetOperationError(sendInstance, myPropertyList2,
                                    true, false, CIM_ERR_NOT_SUPPORTED))
           TERMINATE("Set with propertylist empty should pass");
        PEGASUS_TEST_ASSERT(!getStatisticsState());

        // Try with includequalifiers and include qualifiers. Should fail
        // NOTE: This test is disabled as the server simply ignores the
        // depricated parameter.
        //if(!testStatisticsSetOperationError(sendInstance, myPropertyList,
        //                            false, true, CIM_ERR_NOT_SUPPORTED))
        //   TERMINATE("Set Should fail. includeQualifiers = true");
        PEGASUS_TEST_ASSERT(!getStatisticsState());

        // try with propertylist that has extra properties.
        plA.append(CIMName("RequestStateChange"));
        myPropertyList.set(plA);
        if(!testStatisticsSetOperationError(sendInstance, myPropertyList,
                                    false, false,CIM_ERR_NOT_SUPPORTED))
           TERMINATE("Set Should fail. Bad Property in modifiedInstance");

        // Confrim that instance is unchanged from original.
        PEGASUS_TEST_ASSERT(!getStatisticsState());
        //newInstanceObjectManager = getInstanceObjMgr();
        PEGASUS_TEST_ASSERT(
                instanceObjectManager.identical(getInstanceObjMgr()));
    }
    // Catch block for all of the Statistics Modification Tests.
    catch(CIMException& e)
    {
        TERMINATE(" testStatisticsEnable Test CIMException: "
                << e.getMessage());
    }
    catch(Exception& e)
    {
        TERMINATE(" testStatisticsEnable Test Pegasus Exception: "
                << e.getMessage());
    }
    catch(...)
    {
        TERMINATE(" testStatisticsEnable Test Caught General Exception:");
    }
    if (verbose)
    {
        cout << "Statistics Enable Tests passed" << endl;
    }
}

//************************************************************
//  Tests on the objectmanager communication classes
//************************************************************
void InteropTest::testCommunicationClass()
{
    try
    {
        // test to confirm that both names and instances return same thing.
        Array<CIMObjectPath> pathsObjMgr = _client.enumerateInstanceNames(
                                                 PEGASUS_NAMESPACENAME_INTEROP,
                                                 CIM_OBJECTMANAGER_CLASSNAME);
        PEGASUS_TEST_ASSERT(pathsObjMgr.size() == 1);

        CIMObjectPath objectManagerPath = pathsObjMgr[0];
        Array<CIMObjectPath> pathsCommMech = _client.enumerateInstanceNames(
                             PEGASUS_NAMESPACENAME_INTEROP,
                             CIM_OBJECTMANAGERCOMMUNICATIONMECHANISM_CLASSNAME);
        Boolean deepInheritance = false;
        Boolean includeQualifiers = true;
        Boolean includeClassOrigin = true;

        Array<CIMInstance> instancesCommMech = _client.enumerateInstances(
                            PEGASUS_NAMESPACENAME_INTEROP,
                            CIM_OBJECTMANAGERCOMMUNICATIONMECHANISM_CLASSNAME,
                            deepInheritance,
                            false,    // localOnly
                            includeQualifiers, includeClassOrigin,
                            CIMPropertyList());


        //Test to confirm that instances and instance names return same thing.
        PEGASUS_TEST_ASSERT( matchPathsAndInstancePaths(pathsCommMech,
                    instancesCommMech));

        // COMMENT KS - There is no reason for this.
        // The whole thing should be covered.
#ifdef PEGASUS_ENABLE_SLP
        PEGASUS_TEST_ASSERT(instancesCommMech.size() > 0);
#endif

        // Test enumerate instances.
        // Note that we do this both with deep inheritance and not
        for (Uint32 i = 0 ; i < instancesCommMech.size() ; i++)
        {
            PEGASUS_TEST_ASSERT (
                    instancesCommMech[i].findProperty(
                        "SystemCreationClassName") != PEG_NOT_FOUND);
            PEGASUS_TEST_ASSERT (
                    instancesCommMech[i].findProperty("SystemName") !=
                    PEG_NOT_FOUND);
            PEGASUS_TEST_ASSERT (
                    instancesCommMech[i].findProperty("CreationClassName") !=
                    PEG_NOT_FOUND);
            PEGASUS_TEST_ASSERT (
                    instancesCommMech[i].findProperty("Name") != PEG_NOT_FOUND);
            PEGASUS_TEST_ASSERT (
                    instancesCommMech[i].findProperty(
                        "SystemCreationClassName") != PEG_NOT_FOUND);
            PEGASUS_TEST_ASSERT (
                    instancesCommMech[i].findProperty(
                        "FunctionalProfilesSupported") != PEG_NOT_FOUND);
            PEGASUS_TEST_ASSERT (
                    instancesCommMech[i].findProperty(
                        "FunctionalProfileDescriptions") != PEG_NOT_FOUND);
            PEGASUS_TEST_ASSERT (
                    instancesCommMech[i].findProperty(
                        "MultipleOperationsSupported") != PEG_NOT_FOUND);
            PEGASUS_TEST_ASSERT (
                    instancesCommMech[i].findProperty(
                        "AuthenticationMechanismsSupported") != PEG_NOT_FOUND);

            // The following property exists only on PG_...,
            // not the superclass and this is no DI
            PEGASUS_TEST_ASSERT (instancesCommMech[i].findProperty("IPAddress")
                    == PEG_NOT_FOUND);
            CIMObjectPath instancePath = instancesCommMech[i].getPath();

            // should be getting only the PG_CIMXMLCOMMUNICATIONM... class
            PEGASUS_TEST_ASSERT (instancePath.getClassName() ==
                    PG_CIMXMLCOMMUNICATIONMECHANISM_CLASSNAME );
        }

        // Repeat with deepInheritance = true.
        deepInheritance = true;
        instancesCommMech =
             _client.enumerateInstances(
                              PEGASUS_NAMESPACENAME_INTEROP,
                              CIM_OBJECTMANAGERCOMMUNICATIONMECHANISM_CLASSNAME,
                              deepInheritance,
                              false,    // localOnly
                              includeQualifiers, includeClassOrigin,
                              CIMPropertyList());

         // Test to assure that getInstances matches enumerateInstanceNames
         // Note: Since this tests the instance for equality with the instance
         // from the enumerate, the enumerate must have matching attributes AND
         // must be deepInheritance to assure that all properties are returned.
         /* TODO
         PEGASUS_TEST_ASSERT(testGetInstancesForEnum(pathsCommMech,
            instancesCommMech,
            includeQualifiers, includeClassOrigin, CIMPropertyList()));
         */
         // COMMENT KS - There is no reason for this.
         // The whole thing should be covered.
#ifdef PEGASUS_ENABLE_SLP
        PEGASUS_TEST_ASSERT(instancesCommMech.size() > 0);
#endif

        // Test the properties in each comm mechanism instance.
        // This tests for existence and correct information as much as possible.
        for (Uint32 i = 0 ; i < instancesCommMech.size() ; i++)
        {
            PEGASUS_TEST_ASSERT (
                    instancesCommMech[i].findProperty(
                        "SystemCreationClassName") != PEG_NOT_FOUND);
            PEGASUS_TEST_ASSERT (
                    instancesCommMech[i].findProperty("SystemName") !=
                    PEG_NOT_FOUND);
            PEGASUS_TEST_ASSERT (
                    instancesCommMech[i].findProperty("CreationClassName") !=
                        PEG_NOT_FOUND);
            PEGASUS_TEST_ASSERT (
                    instancesCommMech[i].findProperty("Name") !=
                        PEG_NOT_FOUND);
            PEGASUS_TEST_ASSERT (
                    instancesCommMech[i].findProperty(
                        "SystemCreationClassName") != PEG_NOT_FOUND);
            PEGASUS_TEST_ASSERT (
                    instancesCommMech[i].findProperty(
                        "FunctionalProfilesSupported") != PEG_NOT_FOUND);
            PEGASUS_TEST_ASSERT (
                    instancesCommMech[i].findProperty(
                        "FunctionalProfileDescriptions") != PEG_NOT_FOUND);

            // testing of characteristics of FunctionalProfile properties
            {
                // get and test FunctionalProfileDescriptions property
                // to determine if correct.
                Uint32 pos = instancesCommMech[i].findProperty(
                        "FunctionalProfilesSupported");
                CIMProperty p = instancesCommMech[i].getProperty(pos);
                CIMValue v1 = p.getValue();
                Array<Uint16> functionalProfile;
                v1.get(functionalProfile);

                // Build an array of entries to test against
                Array<Uint16> testFunctionalProfile;
                testFunctionalProfile.append(2);
                testFunctionalProfile.append(3);
                testFunctionalProfile.append(4);
                testFunctionalProfile.append(5);
                // Optional profile based on config param
                // "enableAssociationTraversal"
                if (getCurrentBoolConfigProperty("enableAssociationTraversal"))
                    testFunctionalProfile.append(6);

                // ExecQuery capability is a compile option.
#ifndef PEGASUS_DISABLE_EXECQUERY
                testFunctionalProfile.append(7);
#endif
                testFunctionalProfile.append(8);

                // enableIndicationService is a config option
                if (getCurrentBoolConfigProperty("enableIndicationService"))
                    testFunctionalProfile.append(9);

                PEGASUS_TEST_ASSERT(testFunctionalProfile.size() ==
                        functionalProfile.size());

                // clone so we can sort to do an identical test on the array.
                Array<Uint16> fpTemp;
                v1.get(fpTemp);
                BubbleSort(fpTemp);
                BubbleSort(testFunctionalProfile);

                PEGASUS_TEST_ASSERT(fpTemp == testFunctionalProfile);

                // get the  functionalProfileDescription to test against
                // profile property.
                pos = instancesCommMech[i].findProperty(
                        "FunctionalProfileDescriptions");
                p = instancesCommMech[i].getProperty(pos);
                v1 = p.getValue();

                Array<String> functionalProfileDescription;
                v1.get(functionalProfileDescription);

                // assert that we have same number descriptions as profiles
                PEGASUS_TEST_ASSERT(functionalProfileDescription.size() ==
                        functionalProfile.size());
                // NOTE: We do not test for correct strings.
                if (verbose)
                    for (Uint32 j = 0 ; j < functionalProfile.size() ; j++)
                        cout << functionalProfile[j] << " " <<
                            functionalProfileDescription[j] << endl;
            }

            PEGASUS_TEST_ASSERT (
                    instancesCommMech[i].findProperty(
                        "MultipleOperationsSupported") != PEG_NOT_FOUND);
            PEGASUS_TEST_ASSERT (
                    instancesCommMech[i].findProperty(
                        "AuthenticationMechanismsSupported") != PEG_NOT_FOUND);

            // The following tests are only for PG_CIMXML... Instances
            if (instancesCommMech[i].getClassName() ==
                    PG_CIMXMLCOMMUNICATIONMECHANISM_CLASSNAME)
            {
                Uint32 pos;

                // test IPaddress property to determine if correct.
                PEGASUS_TEST_ASSERT (
                        (pos = instancesCommMech[i].findProperty("IPAddress"))
                            != PEG_NOT_FOUND);
                CIMProperty p = instancesCommMech[i].getProperty(pos);
                CIMValue v1 = p.getValue();
                String IPAddress;
                v1.get(IPAddress);
                // test to assure that the port number ss part of the value
                PEGASUS_TEST_ASSERT(IPAddress.find(':'));

                // test namespaceAccessProtocol property to determine if correct
                PEGASUS_TEST_ASSERT (
                        (pos = instancesCommMech[i].findProperty(
                            "namespaceAccessProtocol")) != PEG_NOT_FOUND);
                pos = instancesCommMech[i].findProperty(
                        "namespaceAccessProtocol");
                p = instancesCommMech[i].getProperty(pos);
                CIMValue v2 = p.getValue();
                Uint16 accessProtocolValue;
                v2.get(accessProtocolValue);

                // test to assure that this is a 2 or 3 (http or https)
                PEGASUS_TEST_ASSERT(accessProtocolValue == 2 ||
                        accessProtocolValue == 3);

                PEGASUS_TEST_ASSERT (
                    (pos = instancesCommMech[i].findProperty("namespaceType"))
                        != PEG_NOT_FOUND);

                p = instancesCommMech[i].getProperty(pos);
                CIMValue v3 = p.getValue();
                String namespaceTypeValue;
                v3.get(namespaceTypeValue);
                // test to assure that the port number ss part of the value
                if (accessProtocolValue ==2)
                    PEGASUS_TEST_ASSERT(namespaceTypeValue == "http");
                if (accessProtocolValue ==3)
                    PEGASUS_TEST_ASSERT(namespaceTypeValue == "https");

                if (accessProtocolValue == 2)
                {
                    // Test the received IP address.  We do this by
                    // connecting to the cim server and then
                    // disconnecting.
                    CIMClient testClient;
                    HostLocator locator(IPAddress);
                    Uint32 portNumber = locator.getPort();
                    String ipaddress = locator.getHost();
                    // Try to connect to the server.
                    try{
                        testClient.connect(ipaddress, portNumber,
                                String::EMPTY, String::EMPTY);
                        testClient.disconnect();
                    }

                    catch(CIMException& e)
                    {
                        // If node is not in real ip6 network it may fail.
                        // Failsafe for ip6 addrs.
                        if (locator.getAddressType() != HostAddress::AT_IPV6)
                        {
                            TERMINATE(
                                " Cim Exception Error Comm class IP Address: "
                                << IPAddress << " " << e.getMessage());
                        }
                    }
                    catch(Exception& e)
                    {
                        // If node is not in real ip6 network it may fail.
                        // Failsafe for ip6 addrs.
                        if (locator.getAddressType() != HostAddress::AT_IPV6)
                        {
                            TERMINATE("Exception Error Comm class IP Address: "
                                << IPAddress << " " << e.getMessage());
                        }
                    }
                }

                // ATTN: TODO  Test against the valuemap. KS may 05
            }
        }
    }
    // Catch block for all of the CIM_ObjectManager Tests.
    catch(CIMException& e)
    {
        TERMINATE(" testCommunicationClass Test CIMException: "
                << e.getMessage());
    }
    catch(Exception& e)
    {
        cerr << pgmName << " testCommunicationClass Test Pegasus Exception: "
            << e.getMessage()  << endl;
    }
    catch(...)
    {
        cerr << pgmName
            << " testCommunicationClass Test Caught General Exception:"
            << endl;
    }
    if (verbose)
        cout << "test Communication Class successful" << endl;
}

//************************************************************
//  Tests on the NamespaceInObjectManager classes
//************************************************************
void InteropTest::testNameSpaceInObjectManagerAssocClass()
{
    try
    {
        // get all namespace instances to test against
        Array<CIMInstance> namespaceInstances = _getCIMNamespaceInstances();

        Boolean deepInheritance = false;
        Boolean includeQualifiers = true;
        Boolean includeClassOrigin = true;

        // Enumerate NamespaceInManager Instances
        Array<CIMInstance> instancesNamespaceInManager =
            _client.enumerateInstances(
                     PEGASUS_NAMESPACENAME_INTEROP,
                     CIM_NAMESPACEINMANAGER_CLASSNAME,
                     deepInheritance,                    // deepInheritance
                     false,                              // localOnly
                     includeQualifiers,                  // includeQualifiers
                     includeClassOrigin,                 // includeClassOrigin
                     CIMPropertyList());

        // These should be the same (we should be returning a namespace
        // in manager for  each namespace instance.
        PEGASUS_TEST_ASSERT(_getNamespacesNew().size() ==
                instancesNamespaceInManager.size());
        PEGASUS_TEST_ASSERT(_getNamespacesNew().size() ==
                namespaceInstances.size());

        // Test getting reference names.  Should match number of instances

        String role;
        CIMObjectPath objectManagerPath = getObjMgrPath();

        testEnumAgainstEnumNames(PEGASUS_NAMESPACENAME_INTEROP,
                                 CIM_NAMESPACEINMANAGER_CLASSNAME);

        // get the reference names for namespaceinmanager for target
        // objectmanager.
        Array<CIMObjectPath> referenceNames =
            _client.referenceNames(
                PEGASUS_NAMESPACENAME_INTEROP,    // namespace
                objectManagerPath,                // object manager instance
                CIM_NAMESPACEINMANAGER_CLASSNAME, // result class
                role);                            // role

        // should return same number of objects as number of namespaces.
        PEGASUS_TEST_ASSERT(referenceNames.size() ==
                instancesNamespaceInManager.size());

        // test to see that all of the names match
        PEGASUS_TEST_ASSERT( matchPathsAndInstancePaths(referenceNames,
                    instancesNamespaceInManager));

        // Test getting references.  Compare to list of namespaces.
        Array<CIMObject> references =
            _client.references(
                PEGASUS_NAMESPACENAME_INTEROP,       // namespace
                objectManagerPath,                   // object manager instance
                CIM_NAMESPACEINMANAGER_CLASSNAME,    // result class
                role,                                // role
                true,                                // includeQualifiers
                true,                                // includeClassOrigin
                CIMPropertyList());                  // propertyList
        // test if references and referencenames return same size
        PEGASUS_TEST_ASSERT(references.size() == referenceNames.size());
        PEGASUS_TEST_ASSERT( matchPathsAndInstancePaths(referenceNames,
                    instancesNamespaceInManager));

        //
        // Add test for different role properties both for reference and
        // referencenames.
        //
        role = "antecedent";

        references =
            _client.references(
                PEGASUS_NAMESPACENAME_INTEROP,       // namespace
                objectManagerPath,                   // object manager instance
                CIM_NAMESPACEINMANAGER_CLASSNAME,    // result class
                role,                                // role
                true,                                // includeQualifiers
                true,                                // includeClassOrigin
                CIMPropertyList());                  // propertyList

        // get the reference names for namespaceinmanager for target
        // objectmanager.
        referenceNames =
            _client.referenceNames(
                    PEGASUS_NAMESPACENAME_INTEROP,    // namespace
                    objectManagerPath,                // object manager instance
                    CIM_NAMESPACEINMANAGER_CLASSNAME, // result class
                    role);                            // role

        // test if references and referencenames return same size
        PEGASUS_TEST_ASSERT(namespaceInstances.size() == referenceNames.size());
        PEGASUS_TEST_ASSERT(references.size() == referenceNames.size());
        PEGASUS_TEST_ASSERT( matchPathsAndInstancePaths(referenceNames,
                    instancesNamespaceInManager));

        // test with role = dependent.  Should return no instances.
        role = "dependent";
        references = _client.references(
                PEGASUS_NAMESPACENAME_INTEROP,       // namespace
                objectManagerPath,                   // object manager instance
                CIM_NAMESPACEINMANAGER_CLASSNAME,    // result class
                role,                                // role
                true,                                // includeQualifiers
                true,                                // includeClassOrigin
                CIMPropertyList());                  // propertyList

        // get the reference names for namespaceinmanager for target
        // objectmanager.
        referenceNames = _client.referenceNames(
                    PEGASUS_NAMESPACENAME_INTEROP,    // namespace
                    objectManagerPath,                // object manager instance
                    CIM_NAMESPACEINMANAGER_CLASSNAME, // result class
                    role);                            // role

        PEGASUS_TEST_ASSERT(referenceNames.size() == 0);
        PEGASUS_TEST_ASSERT(references.size() == referenceNames.size());

        // TODO Test references in the other direction (from the namespace).

        //
        // test namespaceinmanager associations.
        //

        role = String::EMPTY;
        String resultRole;
        Array<CIMObject> associatorObjects = _client.associators(
                    PEGASUS_NAMESPACENAME_INTEROP,    // namespace
                    objectManagerPath,                // object manager instance
                    CIM_NAMESPACEINMANAGER_CLASSNAME, // association class
                    CIMName(),                        // Result class
                    role,                             // role
                    resultRole);                      // resultRole

        PEGASUS_TEST_ASSERT(namespaceInstances.size() ==
                associatorObjects.size());

        //TODO Test with CIM_NAMESPACE_CLASSNAME in result role.

        Array<CIMObjectPath> associatorNames = _client.associatorNames(
                    PEGASUS_NAMESPACENAME_INTEROP,    // namespace
                    objectManagerPath,                // object manager instance
                    CIM_NAMESPACEINMANAGER_CLASSNAME, // association class
                    PG_NAMESPACE_CLASSNAME,           // Result class
                    role,                             // role
                    resultRole);                      // resultRole

        PEGASUS_TEST_ASSERT(namespaceInstances.size() ==
                associatorNames.size());
        PEGASUS_TEST_ASSERT( matchPathsAndObjectPaths(associatorNames,
                associatorObjects));
    }
    // Catch block for all of the CIM_NamespaceInManager Tests.
    catch(CIMException& e)
    {
        cerr << " CIMException: " << e.getMessage() << endl;
    }
    catch(Exception& e)
    {
        cerr << " Pegasus Exception: " << e.getMessage()  << endl;
    }
    catch(...)
    {
        cerr << " Caught General Exception:" << endl;
    }
}

//************************************************************
//  Tests on the NamespaceInObjectManager classes
//************************************************************
void InteropTest::testCommMechinManagerAssocClass()
{

    CIMObjectPath objectManagerPath = getObjMgrPath();
    try
    {
        Array<CIMObjectPath> referenceNames =
            _client.referenceNames(
                    PEGASUS_NAMESPACENAME_INTEROP,    // namespace
                    objectManagerPath,                // object manager instance
                    CIM_COMMMECHANISMFORMANAGER_CLASSNAME,   // result class
                    String::EMPTY);                   // role


        if (verbose)
        {
            for (Uint32 i = 0 ; i < referenceNames.size() ; i++)
            {
                cout << i << "   " << referenceNames[i].toString() << endl;
            }
        }
    }

    // Catch block for all of the CIM_ObjectManager Tests.
    catch(CIMException& e)
    {
        cerr << " CIMException: " << e.getMessage() << endl;
    }
    catch(Exception& e)
    {
        cerr << " Pegasus Exception: " << e.getMessage()  << endl;
    }
    catch(...)
    {
        cerr << " Caught General Exception:" << endl;
    }
}
///////////////////////////////////////////////////////////////////////////////
//
//  Main-
//
///////////////////////////////////////////////////////////////////////////////
int main(int argc, char** argv)
{
    verbose = getenv("PEGASUS_TEST_VERBOSE") ? true : false;

    pgmName = argv[0];
    if (argc > 1)
    {
        String cmd = argv[1];
        if (cmd == "--help" || cmd == "-help" || cmd == "help")
        {
            cout << "Usage: " << pgmName << "\n"
            << "   or " << pgmName << " parameter where parameter =\n"
            << "         on - turns on the statistics function. \n"
            << "         off - turns off the statistics fucntion.\n"
            << "         status - Shows current status of statistics flag."
            << endl;
            exit (0);
        }
    }

    try
    {
        // Create the InteropTest object.
        InteropTest it;

         /* There are three possible commands.
            on - Turns on the status monitor
            off - Turns off the status monitor
            Status - Shows the state of the status monitor

            If no command is input, it reports the current statistics.
            Note that we do not protect against the user requesting statistics
            if the monitor is off.
        */
        if (argc > 1)
        {
            String cmd = argv[1];
            if (cmd == "on")
            {
                it.setStatisticsState(true);
                exit(0);
            }
            else if (cmd == "off")
            {
                it.setStatisticsState(false);
                exit(0);
            }
            else if (cmd == "status")
            {
                it.showStatisticsState();
                exit(0);
            }
            else if (cmd == "namespaces")
            {
                it._showNamespaceInfo("Current Namespaces");
                exit(0);
            }
            else
            {
                cerr << pgmName << ". Error on command line. "
                    << " Expected on or off or status. Rcvd "<< cmd << endl;
                exit(1);
            }
        }
        it.testPGProviderProfileCapabilities();
        //If PEGASUS_INTEROP_NAMESPACE is set to interop, interop namespace
        //does not come under root.The below test need not to run in 
        //this scenario. 
        if(PEGASUS_NAMESPACENAME_INTEROP.getString() != "interop" )
        {
            it.testNameSpacesManagement();
        }

        it.testSharedNameSpacesManagement();

        it.testObjectManagerClass();

        // Do the enumerate options tests for object manager object
        // classname, di, propertylist, expected rtn count
        it.testEnumerateOptions(
            CIM_OBJECTMANAGER_CLASSNAME, true, CIMPropertyList(), 29);
        it.testEnumerateOptions(
            CIM_OBJECTMANAGER_CLASSNAME, false, CIMPropertyList(), 29);

        CIMPropertyList pl1;
        Array<CIMName> pla1;
        pla1.append("gatherstatisticaldata");
        pl1.set(pla1);

        CIMPropertyList pl2;
        Array<CIMName> pla2;
        pl2.set(pla2);

        it.testEnumerateOptions(CIM_OBJECTMANAGER_CLASSNAME, true, pl1, 1);
        it.testEnumerateOptions(CIM_OBJECTMANAGER_CLASSNAME, false, pl1, 1);

        it.testEnumerateOptions(CIM_OBJECTMANAGER_CLASSNAME, true, pl2, 0);
        it.testEnumerateOptions(CIM_OBJECTMANAGER_CLASSNAME, false, pl2, 0);

        // Repeat the tests for the superclass.
        // classname, di, propertylist, expected rtn count
        it.testEnumerateOptions( CIM_WBEMSERVICE_CLASSNAME, true,
                CIMPropertyList(),29);
        it.testEnumerateOptions( CIM_WBEMSERVICE_CLASSNAME, false,
                CIMPropertyList(), 28);

        it.testEnumerateOptions( CIM_WBEMSERVICE_CLASSNAME, true, pl1, 1);
        // ATTN: The following test case is incorrect.  The
        // GatherStatisticalData property is not defined in the
        // CIM_WBEMService class, so it should not be returned on a
        // non-deep enumeration operation.
        it.testEnumerateOptions( CIM_WBEMSERVICE_CLASSNAME, false, pl1, 1);

        it.testEnumerateOptions( CIM_WBEMSERVICE_CLASSNAME, true, pl2, 0);
        it.testEnumerateOptions( CIM_WBEMSERVICE_CLASSNAME, false, pl2, 0);

#ifndef PEGASUS_DISABLE_PERFINST
        it.testStatisticsEnable();
#endif

#ifdef PEGASUS_ENABLE_SLP
        it.testCommunicationClass();

        it.testNameSpaceInObjectManagerAssocClass();

        it.testCommMechinManagerAssocClass();
#endif
    }

    // Catch block for all of the CIM_ObjectManager Tests.
    catch(CIMException& e)
    {
        TERMINATE(" Program CIMException: " << e.getMessage());
    }
    catch(Exception& e)
    {
        TERMINATE(" Program Pegasus Exception: " << e.getMessage());
    }
    catch(...)
    {
        TERMINATE(" Program Caught General Exception:");
    }
    cout << argv[0] << " +++++ passed all tests" << endl;

    return(0);
}
