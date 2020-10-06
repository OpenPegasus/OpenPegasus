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

#include <Pegasus/Common/Config.h>
#include <Pegasus/Common/CIMName.h>
#include <Pegasus/Common/CIMObjectPath.h>
#include <Pegasus/Common/CIMInstance.h>
#include <Pegasus/Common/String.h>
#include <Pegasus/Common/Constants.h>
#include <Pegasus/Common/PegasusAssert.h>

#include <Pegasus/Client/CIMClient.h>

PEGASUS_USING_STD;
PEGASUS_USING_PEGASUS;

const CIMNamespaceName interopNamespace = PEGASUS_NAMESPACENAME_INTEROP; 

void exitFailure(const String & msg)
{
    cout << (const char *)msg.getCString() << endl;
    exit(-1);
}

Array<CIMInstance> testAnyClass(CIMClient & client, const CIMName & className)
{
    Array<CIMObjectPath> instanceNames = client.enumerateInstanceNames(
        interopNamespace, className);
    Array<CIMInstance> instances = client.enumerateInstances(
        interopNamespace, className);

    if(instanceNames.size() != instances.size())
    {
        exitFailure(className.getString() +
            String(": number of results of EnumerateInstanceNames does not ") +
            String("match EnumerateInstances"));
    }

    for(unsigned int i = 0, n = instanceNames.size(); i < n; ++i)
    {
        Boolean found = false;
        for(unsigned int j = 0, m = instances.size(); j < m; ++j)
        {
            if(instanceNames[i] == instances[j].getPath())
            {
                found = true;
                break;
            }
        }

        if(!found)
        {
            exitFailure(className.getString() +
                String(": Could not find object path in results of ") +
                String("EnumerateInstances: ") + instanceNames[i].toString());
        }

        try
        {
            client.getInstance(interopNamespace, instanceNames[i]);
        }
        catch(CIMException &)
        {
            exitFailure(className.getString() +
                String(": GetInstance operation failed for object ") +
                instanceNames[i].toString());
        }
    }

    return instances;
}

void testInstanceClass(CIMClient & client, const CIMName & className)
{
    cout << "Testing Instance Class "
        << (const char *)className.getString().getCString()
        << "...";
    Array<CIMInstance> instances = testAnyClass(client, className);

    for(unsigned int i = 0, n = instances.size(); i < n; ++i)
    {
        CIMInstance currentInstance = instances[i];
        CIMObjectPath currentPath = currentInstance.getPath();
        if(currentPath.getNameSpace().isNull())
          currentPath.setNameSpace(interopNamespace);

        //
        // Now test association traversal
        // Note that the "TestAssociationClass" method does a very good job
        // of testing association traversal between references contained in
        // instances of the supplied association class. Therefore, all we
        // really have to do here is make sure that the results of the
        // associators, associatorNames, references, and referenceNames
        // operations are consistent.
        //
        Boolean failure = false;
        try
        {
            Array<CIMObject> associatorsResults = client.associators(
                currentPath.getNameSpace(), currentPath);
            Array<CIMObjectPath> associatorNamesResults =
                client.associatorNames(
                    currentPath.getNameSpace(), currentPath);
            Array<CIMObject> referencesResults = client.references(
                currentPath.getNameSpace(), currentPath);
            Array<CIMObjectPath> referenceNamesResults = client.referenceNames(
                currentPath.getNameSpace(), currentPath);

            Uint32 numResults = associatorsResults.size();
            if(numResults != associatorNamesResults.size() ||
                numResults != referencesResults.size() ||
                numResults != referenceNamesResults.size())
            {
                failure = true;
            }
            else
            {
                // Check that the results for the references and referenceNames
                // operations are consistent.
                unsigned int j = 0;
                for(j = 0; j < numResults; ++j)
                {
                    CIMObjectPath currentReferenceName =
                        referenceNamesResults[j];
                    Boolean found = false;
                    for(unsigned int k = 0; k < numResults; ++k)
                    {
                        if(currentReferenceName ==
                            referencesResults[k].getPath())
                        {
                            found = true;
                            break;
                        }
                    }

                    if(!found)
                    {
                        failure = true;
                        break;
                    }
                }

                // Check that that results for the associatorNames call is
                // consistent with the associators call and the references
                // call.
                for(j = 0; j < numResults; ++j)
                {
                    CIMObjectPath currentAssociatorName =
                        associatorNamesResults[j];
                    Boolean found = false;
                    unsigned int k = 0;
                    for(k = 0; k < numResults; ++k)
                    {
                        if(currentAssociatorName ==
                            associatorsResults[k].getPath())
                        {
                            found = true;
                            break;
                        }
                    }

                    if(!found)
                    {
                        failure = true;
                        break;
                    }

                    found = false;

                    for(k = 0; k < numResults; ++k)
                    {
                        CIMObject referenceInstance = referencesResults[k];
                        for(unsigned int x = 0,
                            m = referenceInstance.getPropertyCount();
                            x < m; ++x)
                        {
                            CIMProperty currentProp =
                                referenceInstance.getProperty(x);
                            if(currentProp.getType() == CIMTYPE_REFERENCE)
                            {
                                CIMObjectPath currentRef;
                                currentProp.getValue().get(currentRef);
                                currentRef.setHost(
                                    currentAssociatorName.getHost());
                                if(currentRef == currentAssociatorName)
                                {
                                    found = true;
                                    break;
                                }
                            }
                        }

                        if(found)
                            break;
                    }

                    if(!found)
                    {
                        failure = true;
                        break;
                    }
                }
            }

            if(failure)
            {
                exitFailure(
                    String("Association Operations returned inconsistent ") +
                    String("results for instance ") +
                    currentPath.toString());
            }
        }
        catch(CIMException & e)
        {
          exitFailure(String("Caught exception while performing ") +
            String("association operations on instance ") +
            currentPath.toString() + String(": ") + e.getMessage());
        }
    }

    cout << "Test Complete" << endl;
}

void testAssociationClass(CIMClient & client, const CIMName & className)
{
    cout << "Testing Association Class "
        << (const char *)className.getString().getCString()
        << "...";
    Array<CIMInstance> instances = testAnyClass(client, className);

    for(unsigned int i = 0, n = instances.size(); i < n; ++i)
    {
        //
        // Now make sure that the references are valid and that association
        // traversal between them is working properly.
        //
        CIMObjectPath referenceA;
        CIMObjectPath referenceB;
        CIMInstance currentInstance = instances[i];
        CIMObjectPath currentInstanceName = currentInstance.getPath();
        if(currentInstanceName.getNameSpace().isNull())
            currentInstanceName.setNameSpace(interopNamespace);

        for(unsigned int j = 0, m = currentInstance.getPropertyCount();
            j < m; ++j)
        {
            CIMProperty currentProp = currentInstance.getProperty(j);
            if(currentProp.getValue().getType() == CIMTYPE_REFERENCE)
            {
                if(referenceA.getKeyBindings().size() == 0)
                {
                    currentProp.getValue().get(referenceA);
                }
                else
                {
                    currentProp.getValue().get(referenceB);
                    break;
                }
            }
        }

        if(referenceA.getKeyBindings().size() == 0 ||
            referenceB.getKeyBindings().size() == 0)
        {
            exitFailure(
                String("Could not find reference properties for ") +
                String("association: ") +
                currentInstanceName.toString());
        }

        try
        {
            client.getInstance(referenceA.getNameSpace(), referenceA);
            client.getInstance(referenceB.getNameSpace(), referenceB);
        }
        catch(CIMException &)
        {
            exitFailure(String("Could not get instances for association : ") +
                currentInstanceName.toString());
        }

        Boolean associationFailure = false;
        try
        {
            Array<CIMObjectPath> results = client.associatorNames(
                referenceA.getNameSpace(), referenceA, className);
            Boolean found = false;
            for(unsigned int j = 0, m = results.size(); j < m; ++j)
            {
                CIMObjectPath result = results[j];
                result.setHost(referenceB.getHost());
                result.setNameSpace(referenceB.getNameSpace());
                if(result == referenceB)
                {
                    found = true;
                    break;
                }
            }

            if(found)
            {
                results = client.associatorNames(referenceB.getNameSpace(),
                    referenceB, className);
                for(unsigned int j = 0, m = results.size(); j < m; ++j)
                {
                    CIMObjectPath result = results[j];
                    result.setHost(referenceA.getHost());
                    result.setNameSpace(referenceA.getNameSpace());
                    if(result == referenceA)
                    {
                        found = true;
                        break;
                    }
                }
            }

            if(!found)
            {
                associationFailure = true;
            }
        }
        catch(CIMException & e)
        {
            cout << "Exception: " << e.getMessage() << endl;
            associationFailure = true;
        }

        if(associationFailure)
        {
            exitFailure(String("Association traversal failed between ") +
                String("instances of association: ") +
                currentInstanceName.toString());
        }

        Boolean referencesFailure = false;
        try
        {
            Array<CIMObjectPath> results = client.referenceNames(
                referenceA.getNameSpace(), referenceA, className);
            Boolean found = false;
            for(unsigned int j = 0, m = results.size(); j < m; ++j)
            {
                CIMObjectPath currentPath = results[j];
                if(currentPath.getNameSpace().isNull())
                    currentPath.setNameSpace(interopNamespace);
                if(currentPath.getHost().size() != 0)
                    currentPath.setHost(String::EMPTY);
                if(currentPath == currentInstanceName)
                {
                    found = true;
                    break;
                }
            }

            if(found)
            {
                results = client.referenceNames(referenceB.getNameSpace(),
                    referenceB, className);
                for(unsigned int j = 0, m = results.size(); j < m; ++j)
                {
                    CIMObjectPath currentPath = results[j];
                    if(currentPath.getNameSpace().isNull())
                        currentPath.setNameSpace(interopNamespace);
                    if(currentPath.getHost().size() != 0)
                        currentPath.setHost(String::EMPTY);
                    if(currentPath == currentInstanceName)
                    {
                        found = true;
                        break;
                    }
                }
            }

            if(!found)
            {
                referencesFailure = true;
            }
        }
        catch(CIMException &)
        {
            referencesFailure = true;
        }

        if(referencesFailure)
        {
            exitFailure(String("References operation failed for ") +
                String("instances of association: ") +
                currentInstanceName.toString());
        }
    }

    cout << "Test Complete" << endl;
}

void testDMTFProfileInstances(CIMClient &client)
{
    cout << "Testing DMTF Profiles instances...";

    // Get All Registered profile names
    Array<CIMObjectPath> regInstanceNames = client.enumerateInstanceNames(
        interopNamespace,
        CIMName("CIM_RegisteredProfile"));

    // Find out DMTF autonomous and component profiles.
    for(Uint32 i = 0, n = regInstanceNames.size() ; i < n ; ++i)
    {
        // Filter SNIA sub profile names.
        if (regInstanceNames[i].getClassName().equal("PG_RegisteredSubProfile"))
        {
            continue;
        }

        Array<CIMObjectPath> result = client.associatorNames(
            interopNamespace,
            regInstanceNames[i],
            CIMName("CIM_ReferencedProfile"));

        Uint32 dmtfProfiles = 0;
        for (Uint32 j = 0, k = result.size(); j < k ; ++j)
        {
            // Get only DMTF component profiles.
            if (result[j].getClassName().equal("PG_RegisteredProfile"))
            {
                Array<CIMKeyBinding> keys = result[j].getKeyBindings();
                String value = keys[0].getValue();
                Uint32 index = value.find("DMTF");
                if (index != PEG_NOT_FOUND)
                {
                    dmtfProfiles++;
                }
            }
        }
        if (dmtfProfiles && dmtfProfiles != result.size())
        {
            exitFailure(
                String("Invalid component profiles for ")
                    + regInstanceNames[i].toString());
        }
    }

    cout << "Test Complete" << endl;
}

//
// ATTN: The following indications profile tests will be removed once the
// association opertaions are implemented for these classes. At present these
// tests are pegasus specific.
//
#ifdef PEGASUS_ENABLE_DMTF_INDICATION_PROFILE_SUPPORT
void _testHostedIndicationServiceInstance(CIMClient &client)
{
    cout << "Testing Association Class "
        << (const char *)PEGASUS_CLASSNAME_PG_HOSTEDINDICATIONSERVICE.
             getString().getCString()
        << "...";
    // Get PG_HostedIndicationService Instances
    Array<CIMInstance> hostedInstances = client.enumerateInstances(
        PEGASUS_NAMESPACENAME_INTEROP,
        PEGASUS_CLASSNAME_PG_HOSTEDINDICATIONSERVICE);
    PEGASUS_TEST_ASSERT(hostedInstances.size() == 1);

    // Get PG_HostedIndicationService Instance names
    Array<CIMObjectPath> hostedPaths = client.enumerateInstanceNames(
        PEGASUS_NAMESPACENAME_INTEROP,
        PEGASUS_CLASSNAME_PG_HOSTEDINDICATIONSERVICE);
    PEGASUS_TEST_ASSERT(hostedPaths.size() == 1);

    // Get CIM_IndicationService instance names
    Array<CIMObjectPath> servicePaths = client.enumerateInstanceNames(
        PEGASUS_NAMESPACENAME_INTEROP,
        PEGASUS_CLASSNAME_CIM_INDICATIONSERVICE);
    PEGASUS_TEST_ASSERT(servicePaths.size() == 1);

    // Test the CIM_IndicationService value.
    CIMValue capValue = hostedInstances[0].getProperty(
        hostedInstances[0].findProperty("Dependent")).getValue();
    CIMObjectPath testPath;
    capValue.get(testPath);
    testPath.setNameSpace(CIMNamespaceName());
    PEGASUS_TEST_ASSERT(testPath.identical(servicePaths[0]));

    cout << "Test Complete" << endl;
}

void _testElementCapabilityInstance(CIMClient &client)
{
    cout << "Testing Association Class "
        << (const char *)PEGASUS_CLASSNAME_CIM_INDICATIONSERVICECAPABILITIES.
             getString().getCString()
        << "...";

    // Get CIM_IndicationServiceCapabilities instance names
    Array<CIMObjectPath> capPaths = client.enumerateInstanceNames(
        PEGASUS_NAMESPACENAME_INTEROP,
        PEGASUS_CLASSNAME_CIM_INDICATIONSERVICECAPABILITIES);
    PEGASUS_TEST_ASSERT(capPaths.size() == 1);

    // Get CIM_IndicationService instance names
    Array<CIMObjectPath> servicePaths = client.enumerateInstanceNames(
        PEGASUS_NAMESPACENAME_INTEROP,
        PEGASUS_CLASSNAME_CIM_INDICATIONSERVICE);
    PEGASUS_TEST_ASSERT(servicePaths.size() == 1);


    // Get PG_ElementCapabilities instances
    Array<CIMInstance> eleInstances = client.enumerateInstances(
        PEGASUS_NAMESPACENAME_INTEROP,
        PEGASUS_CLASSNAME_PG_ELEMENTCAPABILITIES);
    PEGASUS_TEST_ASSERT(eleInstances.size() == 1);

    // Test PG_ElementCapabilities instance.
    CIMValue capValue = eleInstances[0].getProperty(
        eleInstances[0].findProperty("Capabilities")).getValue();

    CIMValue meValue = eleInstances[0].getProperty(
        eleInstances[0].findProperty("ManagedElement")).getValue();

    // Now test the instance names of CIM_IndicationService instance and
    // CIM_IndicationServiceCapabilities instance.
    CIMObjectPath testPath;
    capValue.get(testPath);
    testPath.setNameSpace(CIMNamespaceName());
    PEGASUS_TEST_ASSERT(testPath.identical(capPaths[0]));

    meValue.get(testPath);
    testPath.setNameSpace(CIMNamespaceName());
    PEGASUS_TEST_ASSERT(testPath.identical(servicePaths[0]));

    cout << "Test Complete" << endl;
}

void _testServiceAffectsElementInstances(CIMClient &client)
{
    cout << "Testing Association Class "
        << (const char *)PEGASUS_CLASSNAME_PG_SERVICEAFFECTSELEMENT.
             getString().getCString()
        << "...";

    // Get PG_ServiceAffectsElement instances.
    Array<CIMInstance> sfInstances = client.enumerateInstances(
        PEGASUS_NAMESPACENAME_INTEROP,
        PEGASUS_CLASSNAME_PG_SERVICEAFFECTSELEMENT);

    // Get PG_ServiceAffectsElement instance names
    Array<CIMObjectPath> sfPaths = client.enumerateInstanceNames(
        PEGASUS_NAMESPACENAME_INTEROP,
        PEGASUS_CLASSNAME_PG_SERVICEAFFECTSELEMENT);

    PEGASUS_TEST_ASSERT(sfInstances.size() == sfPaths.size());

    // Get CIM_IndicationFilter instance names
    Array<CIMObjectPath> filterPaths = client.enumerateInstanceNames(
        PEGASUS_NAMESPACENAME_INTEROP,
        PEGASUS_CLASSNAME_INDFILTER);

    // Get CIM_ListenerDestination instance names
    Array<CIMObjectPath> handlerPaths = client.enumerateInstanceNames(
        PEGASUS_NAMESPACENAME_INTEROP,
        PEGASUS_CLASSNAME_LSTNRDST);

    // Count only handlers and filters from interop namespace
    Uint32 elements = 0;
    for (Uint32 i = 0; i < sfInstances.size() ; ++i)
    {
        CIMValue value = sfInstances[i].getProperty(
            sfInstances[i].findProperty("AffectedElement")).getValue();
        CIMObjectPath path;
        value.get(path);
        PEGASUS_TEST_ASSERT(path.getNameSpace() != CIMNamespaceName());
        if (path.getNameSpace() == PEGASUS_NAMESPACENAME_INTEROP)
        {
             elements++;
        }
    }
    PEGASUS_TEST_ASSERT(
        elements == (filterPaths.size() + handlerPaths.size()));

    cout << "Test Complete" << endl;
}

void testIndicationProfileInstances(CIMClient &client)
{
    _testHostedIndicationServiceInstance(client);
    _testElementCapabilityInstance(client);
    _testServiceAffectsElementInstances(client);
}
#endif

///////////////////////////////////////////////////////////////
//    MAIN
///////////////////////////////////////////////////////////////

int main()
{
    cout << "Starting Server Profile Tests" << endl << endl;
    // Create a locally-connected client
    CIMClient client;

    try
    {
        client.connectLocal();
        client.setTimeout(60000); // Set the timeout to one minute
    }
    catch(Exception &)
    {
        exitFailure(String("Could not connect to server"));
    }

    CIMName currentClass;
    try
    {
        currentClass = CIMName("CIM_ComputerSystem");
        testInstanceClass(client, currentClass);
        currentClass = CIMName("CIM_ObjectManager");
        testInstanceClass(client, currentClass);
        currentClass = CIMName("CIM_RegisteredSubProfile");
        testInstanceClass(client, currentClass);
        currentClass = CIMName("CIM_RegisteredProfile");
        testInstanceClass(client, currentClass);
        currentClass = CIMName("CIM_CIMXMLCommunicationMechanism");
        testInstanceClass(client, currentClass);
        currentClass = CIMName("CIM_Namespace");
        testInstanceClass(client, currentClass);
        currentClass = CIMName("CIM_SoftwareIdentity");
        testInstanceClass(client, currentClass);

        currentClass = CIMName("CIM_HostedService");
        testAssociationClass(client, currentClass);
        currentClass = CIMName("CIM_ElementConformsToProfile");
        testAssociationClass(client, currentClass);
        currentClass = CIMName("CIM_SubprofileRequiresProfile");
        testAssociationClass(client, currentClass);
        currentClass = CIMName("CIM_ReferencedProfile");
        testAssociationClass(client, currentClass);
        currentClass = CIMName("CIM_ElementSoftwareIdentity");
        testAssociationClass(client, currentClass);
        currentClass = CIMName("CIM_CommMechanismForManager");
        testAssociationClass(client, currentClass);
    }
    catch(Exception & e)
    {
        exitFailure(String("Caught exception while testing class ") +
            currentClass.getString() + String(": ") + e.getMessage());
    }
    catch(...)
    {
        exitFailure(String("Caught unknown exception while testing class ") +
            currentClass.getString());
    }

    testDMTFProfileInstances(client);
#ifdef PEGASUS_ENABLE_DMTF_INDICATION_PROFILE_SUPPORT
    testIndicationProfileInstances(client);
#endif
    //testAssociationTraversal(client);

    cout << endl << "Server Profile Tests complete" << endl;
    return 0;
}

// END OF FILE
