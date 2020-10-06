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

#include <Pegasus/Common/Config.h>
#include <Pegasus/Common/Constants.h>
#include <Pegasus/Common/FileSystem.h>
#include <Pegasus/Common/PegasusAssert.h>
#include <Pegasus/Common/System.h>
#include <Pegasus/Server/ProviderRegistrationManager/\
ProviderRegistrationManager.h>
#include <Pegasus/Client/CIMClient.h>

PEGASUS_USING_PEGASUS;
PEGASUS_USING_STD;

#define MAX_ITERATIONS 300
#define SLEEP_SEC 1

static const CIMNamespaceName NAMESPACE = CIMNamespaceName("test/testProvider");

static Boolean verbose;

void _createModuleInstance(
    CIMClient& client,
    const String& name,
    const String& location,
    Uint16 userContext)
{
    CIMInstance moduleInstance(PEGASUS_CLASSNAME_PROVIDERMODULE);
    moduleInstance.addProperty(CIMProperty(CIMName("Name"), name));
    moduleInstance.addProperty(CIMProperty(CIMName("Vendor"),
        String("OpenPegasus")));
    moduleInstance.addProperty(CIMProperty(CIMName("Version"),
        String("2.0")));
    moduleInstance.addProperty(CIMProperty(CIMName("InterfaceType"),
        String("C++Default")));
    moduleInstance.addProperty(CIMProperty(CIMName("InterfaceVersion"),
        String("2.5.0")));
    moduleInstance.addProperty(CIMProperty(CIMName("Location"), location));
#ifndef PEGASUS_DISABLE_PROV_USERCTXT
    if (userContext != 0)
    {
        moduleInstance.addProperty(CIMProperty(CIMName("UserContext"),
            userContext));
    }
#endif

    CIMObjectPath path = client.createInstance(PEGASUS_NAMESPACENAME_INTEROP,
        moduleInstance);
}

void _createProviderInstance(
    CIMClient& client,
    const String& name,
    const String& providerModuleName)
{
    CIMInstance providerInstance(PEGASUS_CLASSNAME_PROVIDER);
    providerInstance.addProperty(CIMProperty(CIMName("Name"), name));
    providerInstance.addProperty(CIMProperty(CIMName("ProviderModuleName"),
        providerModuleName));

    CIMObjectPath path = client.createInstance(PEGASUS_NAMESPACENAME_INTEROP,
        providerInstance);
}

void _createCapabilityInstance(
    CIMClient& client,
    const String& providerModuleName,
    const String& providerName,
    const String& capabilityID,
    const String& className,
    const Array<String>& namespaces,
    const Array<Uint16>& providerType,
    const CIMPropertyList& supportedProperties,
    const CIMPropertyList& supportedMethods)
{
    CIMInstance capabilityInstance(PEGASUS_CLASSNAME_PROVIDERCAPABILITIES);
    capabilityInstance.addProperty(CIMProperty(CIMName("ProviderModuleName"),
        providerModuleName));
    capabilityInstance.addProperty(CIMProperty(CIMName("ProviderName"),
        providerName));
    capabilityInstance.addProperty(CIMProperty(CIMName("CapabilityID"),
        capabilityID));
    capabilityInstance.addProperty(CIMProperty(CIMName("ClassName"),
        className));
    capabilityInstance.addProperty(CIMProperty(CIMName("Namespaces"),
        namespaces));
    capabilityInstance.addProperty(CIMProperty(CIMName("ProviderType"),
        CIMValue(providerType)));
    if (!supportedProperties.isNull())
    {
        Array<String> propertyNameStrings;
        for (Uint32 i = 0; i < supportedProperties.size(); i++)
        {
            propertyNameStrings.append(supportedProperties [i].getString());
        }
        capabilityInstance.addProperty(CIMProperty(
            CIMName("supportedProperties"), CIMValue(propertyNameStrings)));
    }
    if (!supportedMethods.isNull())
    {
        Array<String> methodNameStrings;
        for (Uint32 i = 0; i < supportedMethods.size(); i++)
        {
            methodNameStrings.append(supportedMethods [i].getString());
        }
        capabilityInstance.addProperty(CIMProperty(
            CIMName("supportedMethods"), CIMValue(methodNameStrings)));
    }

    CIMObjectPath path = client.createInstance(PEGASUS_NAMESPACENAME_INTEROP,
        capabilityInstance);
}

void _deleteCapabilityInstance(
    CIMClient& client,
    const String& providerModuleName,
    const String& providerName,
    const String& capabilityID)
{
    Array<CIMKeyBinding> keyBindings;
    keyBindings.append(CIMKeyBinding("ProviderModuleName",
        providerModuleName, CIMKeyBinding::STRING));
    keyBindings.append(CIMKeyBinding("ProviderName",
        providerName, CIMKeyBinding::STRING));
    keyBindings.append(CIMKeyBinding("CapabilityID",
        capabilityID, CIMKeyBinding::STRING));
    CIMObjectPath path("", CIMNamespaceName(),
        CIMName("PG_ProviderCapabilities"), keyBindings);
    client.deleteInstance(PEGASUS_NAMESPACENAME_INTEROP, path);
}

void _deleteProviderInstance(
    CIMClient& client,
    const String& name,
    const String& providerModuleName)
{
    Array<CIMKeyBinding> keyBindings;
    keyBindings.append(CIMKeyBinding("Name",
        name, CIMKeyBinding::STRING));
    keyBindings.append(CIMKeyBinding("ProviderModuleName",
        providerModuleName, CIMKeyBinding::STRING));
    CIMObjectPath path("", CIMNamespaceName(),
        CIMName("PG_Provider"), keyBindings);
    client.deleteInstance(PEGASUS_NAMESPACENAME_INTEROP, path);
}

void _deleteModuleInstance(
    CIMClient& client,
    const String& name)
{
    Array<CIMKeyBinding> keyBindings;
    keyBindings.append(CIMKeyBinding("Name",
        name, CIMKeyBinding::STRING));
    CIMObjectPath path("", CIMNamespaceName(),
        CIMName("PG_ProviderModule"), keyBindings);
    client.deleteInstance(PEGASUS_NAMESPACENAME_INTEROP, path);
}

void _createFilterInstance(
    CIMClient& client,
    const String& name,
    const String& query,
    const String& qlang)
{
    CIMInstance filterInstance(PEGASUS_CLASSNAME_INDFILTER);
    filterInstance.addProperty(CIMProperty(CIMName
       ("SystemCreationClassName"), System::getSystemCreationClassName()));
    filterInstance.addProperty(CIMProperty(CIMName("SystemName"),
        System::getFullyQualifiedHostName()));
    filterInstance.addProperty(CIMProperty(CIMName("CreationClassName"),
        PEGASUS_CLASSNAME_INDFILTER.getString()));
    filterInstance.addProperty(CIMProperty(CIMName("Name"), name));
    filterInstance.addProperty(CIMProperty(CIMName("Query"), query));
    filterInstance.addProperty(CIMProperty(CIMName("QueryLanguage"),
        String(qlang)));
    filterInstance.addProperty(CIMProperty(CIMName("SourceNamespace"),
        NAMESPACE.getString()));

    CIMObjectPath path = client.createInstance(PEGASUS_NAMESPACENAME_INTEROP,
        filterInstance);
}

void _createHandlerInstance(
    CIMClient& client,
    const String& name,
    const String& destination)
{
    CIMInstance handlerInstance(PEGASUS_CLASSNAME_LSTNRDST_CIMXML);
    handlerInstance.addProperty(CIMProperty(CIMName
       ("SystemCreationClassName"), System::getSystemCreationClassName()));
    handlerInstance.addProperty(CIMProperty(CIMName("SystemName"),
        System::getFullyQualifiedHostName()));
    handlerInstance.addProperty(CIMProperty(CIMName("CreationClassName"),
        PEGASUS_CLASSNAME_LSTNRDST_CIMXML.getString()));
    handlerInstance.addProperty(CIMProperty(CIMName("Name"), name));
    handlerInstance.addProperty(CIMProperty(CIMName("Destination"),
        destination));

    CIMObjectPath path = client.createInstance(PEGASUS_NAMESPACENAME_INTEROP,
        handlerInstance);
}

CIMObjectPath _buildFilterOrHandlerPath(
    const CIMName& className,
    const String& name,
    const String& host,
    const CIMNamespaceName& namespaceName = CIMNamespaceName())
{
    CIMObjectPath path;

    Array<CIMKeyBinding> keyBindings;
    keyBindings.append(CIMKeyBinding("SystemCreationClassName",
        System::getSystemCreationClassName(), CIMKeyBinding::STRING));
    keyBindings.append(CIMKeyBinding("SystemName",
        System::getFullyQualifiedHostName(), CIMKeyBinding::STRING));
    keyBindings.append(CIMKeyBinding("CreationClassName",
        className.getString(), CIMKeyBinding::STRING));
    keyBindings.append(CIMKeyBinding("Name", name, CIMKeyBinding::STRING));
    path.setClassName(className);
    path.setKeyBindings(keyBindings);
    path.setNameSpace(namespaceName);
    path.setHost(host);

    return path;
}

void _createSubscriptionInstance(
    CIMClient& client,
    const CIMObjectPath& filterPath,
    const CIMObjectPath& handlerPath,
    Uint16 onFatalErrorPolicy)
{
    CIMInstance subscriptionInstance(PEGASUS_CLASSNAME_INDSUBSCRIPTION);
    subscriptionInstance.addProperty(CIMProperty(CIMName("Filter"),
        filterPath, 0, PEGASUS_CLASSNAME_INDFILTER));
    subscriptionInstance.addProperty(CIMProperty(CIMName("Handler"),
        handlerPath, 0, PEGASUS_CLASSNAME_LSTNRDST_CIMXML));
    subscriptionInstance.addProperty(CIMProperty(
        CIMName("SubscriptionState"), CIMValue((Uint16) 2)));
    subscriptionInstance.addProperty(
        CIMProperty(
            CIMName("OnFatalErrorPolicy"),
            CIMValue((Uint16) onFatalErrorPolicy)));
    CIMObjectPath path = client.createInstance(PEGASUS_NAMESPACENAME_INTEROP,
        subscriptionInstance);
}

void _createSubscription(
    CIMClient& client,
    const String& filterName,
    Uint16 onFatalErrorPolicy = 2)
{
    CIMObjectPath filterPath;
    CIMObjectPath handlerPath;
    filterPath = _buildFilterOrHandlerPath(
        PEGASUS_CLASSNAME_INDFILTER, filterName, String::EMPTY,
        CIMNamespaceName());
    handlerPath = _buildFilterOrHandlerPath(
        PEGASUS_CLASSNAME_LSTNRDST_CIMXML, "OOPHandler01", String::EMPTY,
        CIMNamespaceName());
    _createSubscriptionInstance(
        client,
        filterPath,
        handlerPath,
        onFatalErrorPolicy);
}

CIMObjectPath _getSubscriptionPath(
    const String& filterName,
    const String& handlerName)
{
    Array<CIMKeyBinding> filterKeyBindings;
    filterKeyBindings.append(CIMKeyBinding("SystemCreationClassName",
        System::getSystemCreationClassName(), CIMKeyBinding::STRING));
    filterKeyBindings.append(CIMKeyBinding("SystemName",
        System::getFullyQualifiedHostName(), CIMKeyBinding::STRING));
    filterKeyBindings.append(CIMKeyBinding("CreationClassName",
        PEGASUS_CLASSNAME_INDFILTER.getString(), CIMKeyBinding::STRING));
    filterKeyBindings.append(CIMKeyBinding("Name", filterName,
        CIMKeyBinding::STRING));
    CIMObjectPath filterPath("", CIMNamespaceName(),
        PEGASUS_CLASSNAME_INDFILTER, filterKeyBindings);

    Array<CIMKeyBinding> handlerKeyBindings;
    handlerKeyBindings.append(CIMKeyBinding("SystemCreationClassName",
        System::getSystemCreationClassName(), CIMKeyBinding::STRING));
    handlerKeyBindings.append(CIMKeyBinding("SystemName",
        System::getFullyQualifiedHostName(), CIMKeyBinding::STRING));
    handlerKeyBindings.append(CIMKeyBinding("CreationClassName",
        PEGASUS_CLASSNAME_LSTNRDST_CIMXML.getString(),
        CIMKeyBinding::STRING));
    handlerKeyBindings.append(CIMKeyBinding("Name", handlerName,
        CIMKeyBinding::STRING));
    CIMObjectPath handlerPath("", CIMNamespaceName(),
        PEGASUS_CLASSNAME_LSTNRDST_CIMXML, handlerKeyBindings);

    Array<CIMKeyBinding> subscriptionKeyBindings;
    subscriptionKeyBindings.append(CIMKeyBinding("Filter",
        filterPath.toString(), CIMKeyBinding::REFERENCE));
    subscriptionKeyBindings.append(CIMKeyBinding("Handler",
        handlerPath.toString(), CIMKeyBinding::REFERENCE));

    return CIMObjectPath(
               "",
               CIMNamespaceName(),
               PEGASUS_CLASSNAME_INDSUBSCRIPTION,
               subscriptionKeyBindings);
}

void _deleteSubscriptionInstance(
    CIMClient& client,
    const String& filterName,
    const String& handlerName)
{
    client.deleteInstance(
        PEGASUS_NAMESPACENAME_INTEROP,
        _getSubscriptionPath(filterName, handlerName));
}

void _deleteHandlerInstance(
    CIMClient& client,
    const String& name)
{
    Array<CIMKeyBinding> keyBindings;
    keyBindings.append(CIMKeyBinding("SystemCreationClassName",
        System::getSystemCreationClassName(), CIMKeyBinding::STRING));
    keyBindings.append(CIMKeyBinding("SystemName",
        System::getFullyQualifiedHostName(), CIMKeyBinding::STRING));
    keyBindings.append(CIMKeyBinding("CreationClassName",
        PEGASUS_CLASSNAME_LSTNRDST_CIMXML.getString(),
        CIMKeyBinding::STRING));
    keyBindings.append(CIMKeyBinding("Name", name,
        CIMKeyBinding::STRING));
    CIMObjectPath path("", CIMNamespaceName(),
        PEGASUS_CLASSNAME_LSTNRDST_CIMXML, keyBindings);
    client.deleteInstance(PEGASUS_NAMESPACENAME_INTEROP, path);
}

void _deleteFilterInstance(
    CIMClient& client,
    const String& name)
{
    Array<CIMKeyBinding> keyBindings;
    keyBindings.append(CIMKeyBinding("SystemCreationClassName",
        System::getSystemCreationClassName(), CIMKeyBinding::STRING));
    keyBindings.append(CIMKeyBinding("SystemName",
        System::getFullyQualifiedHostName(), CIMKeyBinding::STRING));
    keyBindings.append(CIMKeyBinding("CreationClassName",
        PEGASUS_CLASSNAME_INDFILTER.getString(), CIMKeyBinding::STRING));
    keyBindings.append(CIMKeyBinding("Name", name,
        CIMKeyBinding::STRING));
    CIMObjectPath path("", CIMNamespaceName(),
        PEGASUS_CLASSNAME_INDFILTER, keyBindings);
    client.deleteInstance(PEGASUS_NAMESPACENAME_INTEROP, path);
}

void _invokeMethod(
    CIMClient& client,
    const CIMName& methodName,
    const String& identifier)
{
    //
    //  Remove previous indication log file, if there
    //
    String previousIndicationFile, oldIndicationFile;

    previousIndicationFile = INDICATION_DIR;
    previousIndicationFile.append("/indicationLog");

    if (FileSystem::exists(previousIndicationFile))
    {
        oldIndicationFile = INDICATION_DIR;
        oldIndicationFile.append("/oldIndicationLog");
        if (FileSystem::exists(oldIndicationFile))
        {
            FileSystem::removeFile(oldIndicationFile);
        }
        if (!FileSystem::renameFile(previousIndicationFile, oldIndicationFile))
        {
            FileSystem::removeFile(previousIndicationFile);
        }
    }

    //
    //  Invoke method to send test indication or cause failure
    //
    Array<CIMParamValue> inParams;
    CIMParamValue inValue(String("identifier"), CIMValue(identifier), true);
    inParams.append(inValue);
    Array<CIMParamValue> outParams;
    Array<CIMKeyBinding> keyBindings;
    Sint32 result;

    CIMValue retValue;

    CIMObjectPath className(String::EMPTY, CIMNamespaceName(),
        CIMName("FailureTestIndication"), keyBindings);

    retValue = client.invokeMethod(
        NAMESPACE,
        className,
        methodName,
        inParams,
        outParams);

    retValue.get(result);
    PEGASUS_TEST_ASSERT(result == 0);
}

void _renameLogFile(const String& indicationLogFileName)
{
    String indicationLogFailedFileName;

    //
    //  Rename the indication log file upon verification failure
    //
    indicationLogFailedFileName = INDICATION_DIR;
    indicationLogFailedFileName.append("/indicationLog_FAILED");
    FileSystem::renameFile(indicationLogFileName, indicationLogFailedFileName);
}

//
//  Reads the contents of the indication log file, verifying if the
//  indication contents are correct.  Returns 1 if correct.  Returns 0 if
//  incorrect.  Returns -1 if result cannot yet be determined (writing of log
//  file may not yet have been completed).
//
Sint16 _verifyIndication(
    const String& indicationLogFileName,
    const String& identifier)
{
    try
    {
        Buffer contents;
        FileSystem::loadFileToMemory(contents, indicationLogFileName);
        const char* theLog = contents.getData();
        String log(theLog);
        Uint32 newline;
        newline = log.find('\n');
        if (newline == PEG_NOT_FOUND)
        {
            cerr << "Expected newline" << endl;
            return -1;
        }
        String header = log.subString(0, newline);
        if (header [header.size() - 1] == '\r')
        {
            header = header.subString(0, newline - 1);
        }
        if (!String::equal(header,
            "++++++++++++++ Received Indication +++++++++++++++++"))
        {
            cerr << "Expected Received Indication header" << endl;
            return -1;
        }
        if (log.size() > (newline + 1))
        {
            log = log.subString(newline + 1);
        }
        else
        {
            cerr << "Expected additional contents after header" << endl;
            return -1;
        }
        Uint32 numProperties = 4;

        String propertyName;
        String propertyValue;
        for (Uint32 i = 0; i < numProperties; i++)
        {
            newline = log.find('\n');
            if (newline == PEG_NOT_FOUND)
            {
                cerr << "Expected newline" << endl;
                return -1;
            }
            String line = log.subString(0, newline);
            if (line [line.size() - 1] == '\r')
            {
                line = line.subString(0, newline - 1);
            }
            Uint32 eq = line.find(String(" = "));
            if (eq == PEG_NOT_FOUND)
            {
                cerr << "Expected =" << endl;
                return -1;
            }
            propertyName.clear();
            propertyValue.clear();
            propertyName = line.subString(0, eq);
            if (line.size() > (eq + 3))
            {
                propertyValue = line.subString(eq + 3);
            }
            if (String::equalNoCase(propertyName,
                "AlertType"))
            {
                if (!String::equal(propertyValue, String("1")))
                {
                    //
                    //  Unexpected property value
                    //
                    cerr << "Unexpected AlertType value: " << propertyValue <<
                         endl;
                    return 0;
                }
            }
            else if (String::equalNoCase(propertyName, "PerceivedSeverity"))
            {
                if (!String::equal(propertyValue, String("2")))
                {
                    //
                    //  Unexpected property value
                    //
                    cerr << "Unexpected PerceivedSeverity value: " <<
                         propertyValue << endl;
                    return 0;
                }
            }
            else if (String::equalNoCase(propertyName, "IndicationIdentifier"))
            {
                if (!String::equal(propertyValue, identifier))
                {
                    //
                    //  Unexpected property value
                    //
                    cerr << "Unexpected IndicationIdentifier value: " <<
                         propertyValue << endl;
                    cerr << "Expected IndicationIdentifier value: " <<
                         identifier << endl;
                    return 0;
                }
            }
            else if (String::equalNoCase(propertyName, "IndicationTime"))
            {
                //  Don't try to validate the value
            }
            else
            {
                //
                //  Unexpected property name
                //
                cerr << "Unexpected property name: " << propertyName << endl;
                return 0;
            }
            if (log.size() > (newline + 1))
            {
                log = log.subString(newline + 1);
            }
            else
            {
                cerr <<
                     "Expected additional contents after indication properties"
                     << endl;
                return -1;
            }
        }

        newline = log.find('\n');
        if (newline == PEG_NOT_FOUND)
        {
            cerr << "Expected newline" << endl;
            return -1;
        }
        String footer = log.subString(0, newline);
        if (footer [footer.size() - 1] == '\r')
        {
            footer = footer.subString(0, newline - 1);
        }
        if (!String::equal(footer,
            "++++++++++++++++++++++++++++++++++++++++++++++++++++"))
        {
            cerr << "Expected footer" << endl;
            return -1;
        }
        if (log.size() > newline + 1)
        {
            log = log.subString(newline + 1);
            if (log [0] == '\r')
            {
                log = log.subString(1);
            }
            if ((log.size() != 1) || (log [0] != '\n'))
            {
                cerr << "Extra contents in log after indication" << endl;
                return 0;
            }
        }
        else
        {
            cerr << "Expected final return/newline" << endl;
            return -1;
        }

        //
        //  Successful verification
        //
        return 1;
    }
    catch (CannotOpenFile&)
    {
        cerr << "Could not open indication log file" << endl;
        return -1;
    }
    catch (...)
    {
        cerr << "Unknown error validating indication log file" << endl;
        return -1;
    }
}

Boolean _validateIndicationReceipt(
    const String& identifier)
{
    String indicationLogFileName;
    indicationLogFileName = INDICATION_DIR;
    indicationLogFileName.append("/indicationLog");

    //
    //  Wait for indication to be logged to file
    //
    Uint32 iteration = 0;
    Boolean fileExists = false;
    while (iteration < MAX_ITERATIONS)
    {
        iteration++;
        if (FileSystem::exists(indicationLogFileName))
        {
            fileExists = true;
            break;
        }
        else
        {
            System::sleep(SLEEP_SEC);
        }
    }

    if (!fileExists)
    {
        return false;
    }

    //
    //  Once the file exists, allow time for the indication to be written
    //
    Boolean indicationVerified = false;
    while (iteration < MAX_ITERATIONS)
    {
        iteration++;
        Sint16 verifyResult = _verifyIndication(indicationLogFileName,
            identifier);

        //
        //  Indication verified to be correct
        //
        if (verifyResult == 1)
        {
            //
            //  Remove the indication log file on successful verification
            //
            indicationVerified = true;
            FileSystem::removeFile(indicationLogFileName);
            break;
        }
        //
        //  Indication verified to be incorrect
        //
        else if (verifyResult == 0)
        {
            //
            //  Rename the indication log file on unsuccessful verification
            //
            _renameLogFile(indicationLogFileName);
            break;
        }
        //
        //  Indication log file may not yet have been completely written
        //
        else  //  verifyResult == -1
        {
            //
            //  Retry
            //
            System::sleep(SLEEP_SEC);
        }
    }

    return indicationVerified;
}

CIMInstance _getModuleInstance(
    CIMClient& client,
    const String& providerModuleName)
{
    CIMInstance moduleInstance;
    CIMKeyBinding keyBinding(CIMName("Name"), providerModuleName,
        CIMKeyBinding::STRING);
    Array<CIMKeyBinding> kbArray;
    kbArray.append(keyBinding);
    CIMObjectPath modulePath("", PEGASUS_NAMESPACENAME_INTEROP,
        PEGASUS_CLASSNAME_PROVIDERMODULE, kbArray);

    moduleInstance = client.getInstance(PEGASUS_NAMESPACENAME_INTEROP,
        modulePath);

    return moduleInstance;
}

Sint16 _invokeModuleMethod(
    CIMClient& client,
    const CIMInstance& moduleInstance,
    const String& providerModuleName,
    const CIMName& methodName)
{
    CIMObjectPath path = moduleInstance.getPath();
    path.setNameSpace(PEGASUS_NAMESPACENAME_INTEROP);
    path.setClassName(PEGASUS_CLASSNAME_PROVIDERMODULE);

    CIMKeyBinding kb(CIMName("Name"), providerModuleName,
        CIMKeyBinding::STRING);
    Array<CIMKeyBinding> keys;
    keys.append(kb);
    path.setKeyBindings(keys);

    Array<CIMParamValue> inParams;
    Array<CIMParamValue> outParams;

    CIMValue ret_value = client.invokeMethod(
        PEGASUS_NAMESPACENAME_INTEROP,
        path,
        methodName,
        inParams,
        outParams);

    Sint16 retValue;
    ret_value.get(retValue);

    return retValue;
}

void _disableModule(
    CIMClient& client,
    const String& providerModuleName)
{
    //
    //  Get the module instance
    //
    CIMInstance moduleInstance =
        _getModuleInstance(client, providerModuleName);

    //
    //  Invoke method to disable the module
    //
    Sint16 returnValue = _invokeModuleMethod(client, moduleInstance,
        providerModuleName, CIMName("stop"));
    PEGASUS_TEST_ASSERT(returnValue == 0);
}

void _enableModule(
    CIMClient& client,
    const String& providerModuleName)
{
    //
    //  Get the module instance
    //
    CIMInstance moduleInstance =
        _getModuleInstance(client, providerModuleName);

    //
    //  Invoke method to enable the module
    //
    Sint16 returnValue = _invokeModuleMethod(client, moduleInstance,
        providerModuleName, CIMName("start"));
    PEGASUS_TEST_ASSERT(returnValue == 0);
}

Boolean _validateStatus(
    CIMClient& client,
    const String& providerModuleName,
    Uint16 expectedStatus)
{
    Boolean result = false;

    try
    {
        //
        //  Get instance for module
        //
        CIMInstance moduleInstance;
        CIMKeyBinding keyBinding(CIMName("Name"), providerModuleName,
            CIMKeyBinding::STRING);
        Array<CIMKeyBinding> kbArray;
        kbArray.append(keyBinding);
        CIMObjectPath modulePath("", PEGASUS_NAMESPACENAME_INTEROP,
            PEGASUS_CLASSNAME_PROVIDERMODULE, kbArray);

        moduleInstance = client.getInstance(PEGASUS_NAMESPACENAME_INTEROP,
            modulePath);

        //
        //  Get status from instance
        //
        Array<Uint16> operationalStatus;
        Uint32 index = moduleInstance.findProperty(
            CIMName("OperationalStatus"));
        if (index != PEG_NOT_FOUND)
        {
            CIMValue statusValue =
                moduleInstance.getProperty(index).getValue();
            if (!statusValue.isNull())
            {
                statusValue.get(operationalStatus);
                for (Uint32 i = 0; i < operationalStatus.size(); i++)
                {
                    if (operationalStatus [i] == expectedStatus)
                    {
                        result = true;
                        break;
                    }
                }
            }
        }
    }
    catch (...)
    {
        //
        //  Result remains false if fail to get status
        //
    }

    return result;
}

void _checkStatus(
    CIMClient& client,
    const String& providerModuleName,
    Uint16 expectedStatus)
{
    //
    //  Status may not be updated yet, since operation is responded to
    //  before status update occurs
    //  Test for expected status in a loop until either the expected status is
    //  observed, or the maximum number of iterations have expired
    //
    Uint32 iteration = 0;
    Boolean expectedStatusObserved = false;
    while (iteration < MAX_ITERATIONS)
    {
        iteration++;
        if (_validateStatus(client, providerModuleName, expectedStatus))
        {
            expectedStatusObserved = true;
            break;
        }
        else
        {
            System::sleep(SLEEP_SEC);
        }
    }

    PEGASUS_TEST_ASSERT(expectedStatusObserved);
}

void _checkExceptionCode(
    const CIMException& e,
    const CIMStatusCode expectedCode)
{
    if (e.getCode() != expectedCode)
    {
        cerr << "CIMException comparison failed.  ";
        cerr << "Expected " << cimStatusCodeToString(expectedCode) << "; ";
        cerr << "Actual exception was " << e.getMessage() << "." << endl;
    }

    PEGASUS_TEST_ASSERT(e.getCode() == expectedCode);
}

void _setup(CIMClient& client)
{
    //
    //  Create Filters and Handler for subscriptions
    //
    _createFilterInstance(client, String("OOPFilter01"),
        String("SELECT AlertType, PerceivedSeverity, "
            "IndicationIdentifier, IndicationTime FROM FailureTestIndication"),
        "WQL");

    _createFilterInstance(client, String("OOPFilter02"),
        String("SELECT PerceivedSeverity FROM FailureTestIndication"),
        "WQL");

    _createHandlerInstance(client, String("OOPHandler01"),
        String("localhost/CIMListener/Pegasus_SimpleDisplayConsumer"));
}

void _register(
    CIMClient& client,
    Uint16 userContext,
    const String& providerModuleName,
    const String& providerName,
    const String& capabilityID,
    const String& className,
    const Array<String>& namespaces,
    const Array<Uint16>& providerType,
    const CIMPropertyList& supportedProperties,
    const CIMPropertyList& supportedMethods)
{
    //
    //  Create provider module instance
    //
    _createModuleInstance(
        client,
        providerModuleName,
        String("OOPModuleFailureTestProvider"),
        userContext);

    //
    //  Create the provider and capability instances
    //
    _createProviderInstance(
        client,
        providerName,
        providerModuleName);
    _createCapabilityInstance(
        client,
        providerModuleName,
        providerName,
        capabilityID,
        className,
        namespaces,
        providerType,
        supportedProperties,
        supportedMethods);
}

void _deregister(
    CIMClient& client,
    const String& providerModuleName,
    const String& providerName,
    const String& capabilityID)
{
    _deleteCapabilityInstance(
        client,
        providerModuleName,
        providerName,
        capabilityID);
    _deleteProviderInstance(
        client,
        providerName,
        providerModuleName);
    _deleteModuleInstance(
        client,
        providerModuleName);
}

void _cleanup(CIMClient& client)
{
    //
    //  Delete Filters and Handler for subscriptions
    //
    _deleteHandlerInstance(client, String("OOPHandler01"));
    _deleteFilterInstance(client, String("OOPFilter01"));
    _deleteFilterInstance(client, String("OOPFilter02"));
}

//
//  Scenario 1: Provider failure of a module with no indication providers
//              Provider fails upon "Fail" invokeMethod call
//
//  Register the provider
//  Invoke method to cause provider failure
//  Verify provider module status is OK after provider failure
//  De-register the provider
//
//  Test providers:
//      OOPModuleInvokeFailureTestProvider
//
void _testScenario1(
    CIMClient& client,
    Uint16 userContext,
    const String& providerModuleName,
    const String& providerName,
    const String& capabilityID,
    const String& className,
    const Array<String>& namespaces,
    const Array<Uint16>& providerType)
{
    //
    //  Register the provider
    //
    _register(
        client,
        userContext,
        providerModuleName,
        providerName,
        capabilityID,
        className,
        namespaces,
        providerType,
        CIMPropertyList(),
        CIMPropertyList());

    //
    //  Invoke method to cause provider failure
    //
    try
    {
        String identifier = "Scenario 1: " + providerName;
        _invokeMethod(client, String("Fail"), identifier);
        PEGASUS_TEST_ASSERT(false);
    }
    catch (const CIMException& e)
    {
        _checkExceptionCode(e, CIM_ERR_FAILED);
    }

    //
    //  Verify module status is OK
    //
    _checkStatus(client, providerModuleName, CIM_MSE_OPSTATUS_VALUE_OK);

    //
    //  De-register the provider
    //
    _deregister(
        client,
        providerModuleName,
        providerName,
        capabilityID);
}

//
//  Scenario 2: Provider failure before successful creation of a subscription,
//                  with no pre-existing subscriptions
//              Provider fails in initialize call or
//              Provider fails in createSubscription call or
//              Provider fails in enableIndications call
//
//  Register the provider
//  Attempt to create a subscription fails and results in provider failure
//  Verify module status is OK
//  De-register the provider
//
//  Test providers:
//      OOPModuleInitFailureTestProvider
//      OOPModuleCreateFailureTestProvider
//      OOPModuleEnableFailureTestProvider
//
void _testScenario2(
    CIMClient& client,
    Uint16 userContext,
    const String& providerModuleName,
    const String& providerName,
    const String& capabilityID,
    const String& className,
    const Array<String>& namespaces,
    const Array<Uint16>& providerType)
{
    //
    //  Register the provider
    //
    _register(
        client,
        userContext,
        providerModuleName,
        providerName,
        capabilityID,
        className,
        namespaces,
        providerType,
        CIMPropertyList(),
        CIMPropertyList());

    //
    //  Create a subscription
    //
    try
    {
        _createSubscription(client, String("OOPFilter01"));
        PEGASUS_TEST_ASSERT(false);
    }
    catch (const CIMException& e)
    {
        _checkExceptionCode(e, CIM_ERR_NOT_SUPPORTED);
    }

    //
    //  Verify module status is OK
    //
    _checkStatus(client, providerModuleName, CIM_MSE_OPSTATUS_VALUE_OK);

    //
    //  De-register the provider
    //
    _deregister(
        client,
        providerModuleName,
        providerName,
        capabilityID);
}

//
//  Scenario 3: Provider failure on creation of a subscription,
//                  with a pre-existing subscription
//              Provider fails in createSubscription call
//
//  Register the provider
//  Create a subscription
//  Invoke method to send test indication
//  Attempt to create second subscription fails and results in provider failure
//  Verify module status is Degraded
//  Disable provider and verify module status is Stopped
//  Re-enable provider and verify module status is OK
//  Invoke method to send test indication
//  Delete the subscription
//  De-register the provider
//
//  Test providers:
//      OOPModuleCreate2FailureTestProvider
//
void _testScenario3(
    CIMClient& client,
    Uint16 userContext,
    const String& providerModuleName,
    const String& providerName,
    const String& capabilityID,
    const String& className,
    const Array<String>& namespaces,
    const Array<Uint16>& providerType)
{
    //
    //  Register the provider
    //
    _register(
        client,
        userContext,
        providerModuleName,
        providerName,
        capabilityID,
        className,
        namespaces,
        providerType,
        CIMPropertyList(),
        CIMPropertyList());

    //
    //  Create a subscription
    //
    _createSubscription(client, String("OOPFilter01"));

    //
    //  Invoke method to send test indication
    //
    String identifier = "Scenario 3a: " + providerName;
    _invokeMethod(client, String("SendTestIndication"), identifier);
    PEGASUS_TEST_ASSERT(_validateIndicationReceipt(identifier));

    //
    //  Create a subscription that will cause provider failure
    //
    try
    {
        _createSubscription(client, String("OOPFilter02"));
        PEGASUS_TEST_ASSERT(false);
    }
    catch (const CIMException& e)
    {
        _checkExceptionCode(e, CIM_ERR_NOT_SUPPORTED);
    }

    //
    //  Verify module status is Degraded
    //
    _checkStatus(client, providerModuleName, CIM_MSE_OPSTATUS_VALUE_DEGRADED);

    //
    //  Disable provider and verify module status is Stopped
    //
    _disableModule(client, providerModuleName);
    _checkStatus(client, providerModuleName, CIM_MSE_OPSTATUS_VALUE_STOPPED);

    //
    //  Re-enable provider and verify module status is OK
    //
    _enableModule(client, providerModuleName);
    _checkStatus(client, providerModuleName, CIM_MSE_OPSTATUS_VALUE_OK);

    //
    //  Invoke method to send test indication
    //
    identifier = "Scenario 3b: " + providerName;
    _invokeMethod(client, String("SendTestIndication"), identifier);
    PEGASUS_TEST_ASSERT(_validateIndicationReceipt(identifier));

    //
    //  Delete the subscription
    //
    _deleteSubscriptionInstance(client, String("OOPFilter01"),
        String("OOPHandler01"));

    //
    //  De-register the provider
    //
    _deregister(
        client,
        providerModuleName,
        providerName,
        capabilityID);
}

//
//  Scenario 4: Provider failure after successful creation of a subscription
//              Provider fails in modifySubscription call or
//              Provider fails in invokeMethod call
//
//  Register the provider
//  Create a subscription
//  Invoke method to send test indication
//  Cause provider failure (invokeMethod "Fail" or create Capability instance)
//  Verify module status is Degraded
//  Disable provider and verify module status is Stopped
//  Re-enable provider and verify module status is OK
//  Invoke method to send test indication
//  Delete the subscription
//  De-register the provider
//
//  Test providers:
//      OOPModuleModifyFailureTestProvider
//      OOPModuleInvokeFailureTestProvider
//
void _testScenario4(
    CIMClient& client,
    Uint16 userContext,
    const String& providerModuleName,
    const String& providerName,
    const String& capabilityID,
    const String& className,
    const Array<String>& namespaces,
    const Array<Uint16>& providerType)
{
    //
    //  Register the provider
    //
    _register(
        client,
        userContext,
        providerModuleName,
        providerName,
        capabilityID,
        className,
        namespaces,
        providerType,
        CIMPropertyList(),
        CIMPropertyList());

    //
    //  Create a subscription
    //
    _createSubscription(client, String("OOPFilter01"));

    //
    //  Invoke method to send test indication
    //
    String identifier = "Scenario 4a: " + providerName;
    _invokeMethod(client, String("SendTestIndication"), identifier);
    PEGASUS_TEST_ASSERT(_validateIndicationReceipt(identifier));

    //
    //  Cause provider failure
    //
    if (String::equalNoCase(
            providerName, "OOPModuleInvokeFailureTestProvider"))
    {
        //
        //  Invoke method to cause provider failure
        //
        try
        {
            identifier = "Scenario 4b: " + providerName;
            _invokeMethod(client, String("Fail"), identifier);
            PEGASUS_TEST_ASSERT(false);
        }
        catch (const CIMException& e)
        {
            _checkExceptionCode(e, CIM_ERR_FAILED);
        }
    }
    else if (String::equalNoCase(
                 providerName, "OOPModuleModifyFailureTestProvider"))
    {
        //
        //  Create additional provider capability that will result in a
        //  modifySubscription call to the provider
        //
        _createCapabilityInstance(
            client,
            providerModuleName,
            providerName,
            String("OOPCapability010"),
            String("FailureTestIndicationSubclass"),
            namespaces,
            providerType,
            CIMPropertyList(),
            CIMPropertyList());
    }

    //
    //  Verify module status is Degraded
    //
    _checkStatus(client, providerModuleName, CIM_MSE_OPSTATUS_VALUE_DEGRADED);

    //
    //  Disable provider and verify module status is Stopped
    //
    _disableModule(client, providerModuleName);
    _checkStatus(client, providerModuleName, CIM_MSE_OPSTATUS_VALUE_STOPPED);

    //
    //  Re-enable provider and verify module status is OK
    //
    _enableModule(client, providerModuleName);
    _checkStatus(client, providerModuleName, CIM_MSE_OPSTATUS_VALUE_OK);

    //
    //  Invoke method to send test indication
    //
    identifier = "Scenario 4c: " + providerName;
    _invokeMethod(client, String("SendTestIndication"), identifier);
    PEGASUS_TEST_ASSERT(_validateIndicationReceipt(identifier));

    //
    //  Delete the subscription
    //
    _deleteSubscriptionInstance(client, String("OOPFilter01"),
        String("OOPHandler01"));

    //
    //  De-register the provider
    //
    _deregister(
        client,
        providerModuleName,
        providerName,
        capabilityID);
}

//
//  Scenario 5: Provider failure on deletion of a subscription
//                  with a subscription remaining after the delete
//              Provider fails in deleteSubscription call
//
//  Register the provider
//  Create a subscription
//  Invoke method to send test indication
//  Create a second subscription
//  Delete the second subscription to cause provider failure
//  Verify module status is Degraded
//  Disable provider and verify module status is Stopped
//  Re-enable provider and verify module status is OK
//  Invoke method to send test indication
//  Delete the first subscription
//  De-register the provider
//
//  Test providers:
//      OOPModuleDelete2FailureTestProvider
//
void _testScenario5(
    CIMClient& client,
    Uint16 userContext,
    const String& providerModuleName,
    const String& providerName,
    const String& capabilityID,
    const String& className,
    const Array<String>& namespaces,
    const Array<Uint16>& providerType)
{
    //
    //  Register the provider
    //
    _register(
        client,
        userContext,
        providerModuleName,
        providerName,
        capabilityID,
        className,
        namespaces,
        providerType,
        CIMPropertyList(),
        CIMPropertyList());

    //
    //  Create a subscription
    //
    _createSubscription(client, String("OOPFilter01"));

    //
    //  Invoke method to send test indication
    //
    String identifier = "Scenario 5a: " + providerName;
    _invokeMethod(client, String("SendTestIndication"), identifier);
    PEGASUS_TEST_ASSERT(_validateIndicationReceipt(identifier));

    //
    //  Create a second subscription
    //
    _createSubscription(client, String("OOPFilter02"));

    //
    //  Delete the second subscription to cause provider failure
    //
    _deleteSubscriptionInstance(client, String("OOPFilter02"),
        String("OOPHandler01"));

    //
    //  Verify module status is Degraded
    //
    _checkStatus(client, providerModuleName, CIM_MSE_OPSTATUS_VALUE_DEGRADED);

    //
    //  Disable provider and verify module status is Stopped
    //
    _disableModule(client, providerModuleName);
    _checkStatus(client, providerModuleName, CIM_MSE_OPSTATUS_VALUE_STOPPED);

    //
    //  Re-enable provider and verify module status is OK
    //
    _enableModule(client, providerModuleName);
    _checkStatus(client, providerModuleName, CIM_MSE_OPSTATUS_VALUE_OK);

    //
    //  Invoke method to send test indication
    //
    identifier = "Scenario 5b: " + providerName;
    _invokeMethod(client, String("SendTestIndication"), identifier);
    PEGASUS_TEST_ASSERT(_validateIndicationReceipt(identifier));

    //
    //  Delete the first subscription
    //
    _deleteSubscriptionInstance(client, String("OOPFilter01"),
        String("OOPHandler01"));

    //
    //  De-register the provider
    //
    _deregister(
        client,
        providerModuleName,
        providerName,
        capabilityID);
}

//
//  Scenario 6: Provider failure after successful deletion of a subscription
//              Provider fails in deleteSubscription call or
//              Provider fails in disableIndications call
//
//  Register the provider
//  Create a subscription
//  Invoke method to send test indication
//  Delete the subscription to cause provider failure
//  Verify module status is OK
//  De-register the provider
//
//  Test providers:
//      OOPModuleDeleteFailureTestProvider
//      OOPModuleDisableFailureTestProvider
//
void _testScenario6(
    CIMClient& client,
    Uint16 userContext,
    const String& providerModuleName,
    const String& providerName,
    const String& capabilityID,
    const String& className,
    const Array<String>& namespaces,
    const Array<Uint16>& providerType)
{
    //
    //  Register the provider
    //
    _register(
        client,
        userContext,
        providerModuleName,
        providerName,
        capabilityID,
        className,
        namespaces,
        providerType,
        CIMPropertyList(),
        CIMPropertyList());

    //
    //  Create a subscription
    //
    _createSubscription(client, String("OOPFilter01"));

    //
    //  Invoke method to send test indication
    //
    String identifier = "Scenario 6: " + providerName;
    _invokeMethod(client, String("SendTestIndication"), identifier);
    PEGASUS_TEST_ASSERT(_validateIndicationReceipt(identifier));

    //
    //  Delete the subscription to cause provider failure
    //
    _deleteSubscriptionInstance(client, String("OOPFilter01"),
        String("OOPHandler01"));

    //
    //  Verify module status is OK
    //
    _checkStatus(client, providerModuleName, CIM_MSE_OPSTATUS_VALUE_OK);

    //
    //  De-register the provider
    //
    _deregister(
        client,
        providerModuleName,
        providerName,
        capabilityID);
}

//
//  Scenario 7: Provider failure upon disable of provider
//              Provider fails in terminate call
//
//  Register the provider
//  Create a subscription
//  Invoke method to send test indication
//  Disable provider to cause provider failure
//  Verify provider status is Degraded
//  Disable provider and verify module status is Stopped
//  Re-enable provider and verify module status is OK
//  Invoke method to send test indication
//  Delete the subscription
//  De-register the provider
//
//  Test providers:
//      OOPModuleTermFailureTestProvider
//
void _testScenario7(
    CIMClient& client,
    Uint16 userContext,
    const String& providerModuleName,
    const String& providerName,
    const String& capabilityID,
    const String& className,
    const Array<String>& namespaces,
    const Array<Uint16>& providerType)
{
    //
    //  Register the provider
    //
    _register(
        client,
        userContext,
        providerModuleName,
        providerName,
        capabilityID,
        className,
        namespaces,
        providerType,
        CIMPropertyList(),
        CIMPropertyList());

    //
    //  Create a subscription
    //
    _createSubscription(client, String("OOPFilter01"));

    //
    //  Invoke method to send test indication
    //
    String identifier = "Scenario 7a: " + providerName;
    _invokeMethod(client, String("SendTestIndication"), identifier);
    PEGASUS_TEST_ASSERT(_validateIndicationReceipt(identifier));

    //
    //  Disable provider to cause provider failure
    //  Verify provider status is Degraded
    //
    try
    {
        _disableModule(client, providerModuleName);
    }
    catch (const CIMException& e)
    {
        _checkExceptionCode(e, CIM_ERR_FAILED);
    }
    _checkStatus(client, providerModuleName, CIM_MSE_OPSTATUS_VALUE_DEGRADED);

    //
    //  Disable provider and verify module status is Stopped
    //
    _disableModule(client, providerModuleName);
    _checkStatus(client, providerModuleName, CIM_MSE_OPSTATUS_VALUE_STOPPED);

    //
    //  Re-enable provider and verify module status is OK
    //
    _enableModule(client, providerModuleName);
    _checkStatus(client, providerModuleName, CIM_MSE_OPSTATUS_VALUE_OK);

    //
    //  Invoke method to send test indication
    //
    identifier = "Scenario 7b: " + providerName;
    _invokeMethod(client, String("SendTestIndication"), identifier);
    PEGASUS_TEST_ASSERT(_validateIndicationReceipt(identifier));

    //
    //  Delete the subscription
    //
    _deleteSubscriptionInstance(client, String("OOPFilter01"),
        String("OOPHandler01"));

    //
    //  De-register the provider
    //
    _deregister(
        client,
        providerModuleName,
        providerName,
        capabilityID);
}

//
//  Scenario 8: Provider failure with a provider in another module continuing
//              to successfully serve the subscription
//
//  Register the provider
//  Register additional provider in another module
//  Create a subscription
//  Invoke method to send test indication
//  Cause provider failure (invokeMethod "Fail")
//  Verify module status is Degraded
//  Disable provider and verify module status is Stopped
//  Re-enable provider and verify module status is OK
//  Invoke method to send test indication
//  Delete the subscription
//  De-register the providers
//
void _testScenario8(
    CIMClient& client,
    Uint16 userContext,
    const String& providerModuleName,
    const String& providerName,
    const String& capabilityID,
    const String& className,
    const Array<String>& namespaces,
    const Array<Uint16>& providerType)
{
    //
    //  Register the provider
    //
    _register(
        client,
        userContext,
        providerModuleName,
        providerName,
        capabilityID,
        className,
        namespaces,
        providerType,
        CIMPropertyList(),
        CIMPropertyList());

    //
    //  Register additional provider in another module
    //
    _createModuleInstance(
        client,
        String("AlertIndicationProviderModule"),
        String("AlertIndicationProvider"),
        0);
    _createProviderInstance(
        client,
        String("AlertIndicationProvider"),
        String("AlertIndicationProviderModule"));
    Array<Uint16> myProviderType;
    myProviderType.append(_INDICATION_PROVIDER);
    _createCapabilityInstance(
        client,
        String("AlertIndicationProviderModule"),
        String("AlertIndicationProvider"),
        String("OOPCapability012"),
        String("FailureTestIndication"),
        namespaces,
        myProviderType,
        CIMPropertyList(),
        CIMPropertyList());

    //
    //  Create a subscription
    //
    _createSubscription(client, String("OOPFilter01"));

    //
    //  Invoke method to send test indication
    //
    String identifier = "Scenario 8a: " + providerName;
    _invokeMethod(client, String("SendTestIndication"), identifier);
    PEGASUS_TEST_ASSERT(_validateIndicationReceipt(identifier));

    //
    //  Invoke method to cause provider failure
    //
    try
    {
        identifier = "Scenario 8b: " + providerName;
        _invokeMethod(client, String("Fail"), identifier);
        PEGASUS_TEST_ASSERT(false);
    }
    catch (const CIMException& e)
    {
        _checkExceptionCode(e, CIM_ERR_FAILED);
    }

    //
    //  Verify module status is Degraded
    //
    _checkStatus(client, providerModuleName, CIM_MSE_OPSTATUS_VALUE_DEGRADED);

    //
    //  Disable provider and verify module status is Stopped
    //
    _disableModule(client, providerModuleName);
    _checkStatus(client, providerModuleName, CIM_MSE_OPSTATUS_VALUE_STOPPED);

    //
    //  Re-enable provider and verify module status is OK
    //
    _enableModule(client, providerModuleName);
    _checkStatus(client, providerModuleName, CIM_MSE_OPSTATUS_VALUE_OK);

    //
    //  Invoke method to send test indication
    //
    identifier = "Scenario 8c: " + providerName;
    _invokeMethod(client, String("SendTestIndication"), identifier);
    PEGASUS_TEST_ASSERT(_validateIndicationReceipt(identifier));

    //
    //  Delete the subscription
    //
    _deleteSubscriptionInstance(client, String("OOPFilter01"),
        String("OOPHandler01"));

    //
    //  De-register the providers
    //
    _deregister(
        client,
        providerModuleName,
        providerName,
        capabilityID);

    _deregister(
        client,
        String("AlertIndicationProviderModule"),
        String("AlertIndicationProvider"),
        String("OOPCapability012"));
}

//
//  Scenario 9: Testing Subscription's OnFatalErrorPolicy implementation.
//
//  Register the provider
//  Create a subscription with OnFatalErrorPolicy 'Remove'.
//  Cause provider failure (invokeMethod "Fail")
//  Verify module status is Degraded
//  Disable provider and verify module status is Stopped
//  Re-enable provider and verify module status is OK
//  Verify the Subscription is deleted.
//  Create a Subscription with OnFatalErrorPolicy 'Disable'.
//  Cause provider failure (invokeMethod "Fail")
//  Verify module status is Degraded
//  Verify Subscription is disabled.
//  Delete the Subscription
//  De-register the provider
//

void _testScenario9(
    CIMClient& client,
    Uint16 userContext,
    const String& providerModuleName,
    const String& providerName,
    const String& capabilityID,
    const String& className,
    const Array<String>& namespaces,
    const Array<Uint16>& providerType)
{
    //
    //  Register the provider
    //
    _register(
        client,
        userContext,
        providerModuleName,
        providerName,
        capabilityID,
        className,
        namespaces,
        providerType,
        CIMPropertyList(),
        CIMPropertyList());

    //
    //  Create a subscription with OnFatalErrorPolicy 'Remove'.
    //
    _createSubscription(client, String("OOPFilter01"), 4);

    //
    //  Invoke method to cause provider failure
    //
    try
    {
        String identifier = "Scenario 9a: " + providerName;
        _invokeMethod(client, String("Fail"), identifier);
        PEGASUS_TEST_ASSERT(false);
    }
    catch (const CIMException& e)
    {
        _checkExceptionCode(e, CIM_ERR_FAILED);
    }

    //
    //  Verify module status is Degraded
    //
    _checkStatus(client, providerModuleName, CIM_MSE_OPSTATUS_VALUE_DEGRADED);

    //
    //  Disable provider and verify module status is Stopped
    //
    _disableModule(client, providerModuleName);
    _checkStatus(client, providerModuleName, CIM_MSE_OPSTATUS_VALUE_STOPPED);

    //
    //  Re-enable provider and verify module status is OK
    //
    _enableModule(client, providerModuleName);
    _checkStatus(client, providerModuleName, CIM_MSE_OPSTATUS_VALUE_OK);

    //
    //  Try to get the subscription, subscription should not exist.
    //
    try
    {
        client.getInstance(
            PEGASUS_NAMESPACENAME_INTEROP,
            _getSubscriptionPath(
                String("OOPFilter01"),
                String("OOPHandler01")));
        PEGASUS_TEST_ASSERT(false);
    }
    catch(CIMException &e)
    {
        _checkExceptionCode(e, CIM_ERR_NOT_FOUND);
    }

    //
    //  Create a subscription with OnFatalErrorPolicy 'Disable'.
    //
    _createSubscription(client, String("OOPFilter01"), 3);

    //
    //  Invoke method to cause provider failure
    //
    try
    {
        String identifier = "Scenario 9b: " + providerName;
        _invokeMethod(client, String("Fail"), identifier);
        PEGASUS_TEST_ASSERT(false);
    }
    catch (const CIMException& e)
    {
        _checkExceptionCode(e, CIM_ERR_FAILED);
    }

    //
    //  Verify module status is Degraded
    //
    _checkStatus(client, providerModuleName, CIM_MSE_OPSTATUS_VALUE_DEGRADED);

    //
    //  Get the subscription.
    //
    CIMInstance inst = client.getInstance(
        PEGASUS_NAMESPACENAME_INTEROP,
        _getSubscriptionPath(String("OOPFilter01"), String("OOPHandler01")));

    // Check Subscription state.
    CIMValue value = inst.getProperty(
        inst.findProperty(CIMName("SubscriptionState"))).getValue();
    Uint16 subscriptionState;
    value.get(subscriptionState);
    PEGASUS_TEST_ASSERT(subscriptionState == 4); // Should be disabled.

    //
    //  Delete the subscription
    //
    _deleteSubscriptionInstance(client, String("OOPFilter01"),
        String("OOPHandler01"));

    //
    //  De-register the provider
    //
    _deregister(
        client,
        providerModuleName,
        providerName,
        capabilityID);
}

void _testScenarios(
    CIMClient& client,
    Uint16 userContext)
{
    if (verbose)
    {
        cout << "+++++ Testing scenarios with ";
        if (userContext == 0)
        {
            cout << "no user context";
        }
        else if (userContext == PG_PROVMODULE_USERCTXT_PRIVILEGED)
        {
            cout << "user context Privileged";
        }
        else if (userContext == PG_PROVMODULE_USERCTXT_REQUESTOR)
        {
            cout << "user context Requestor";
        }
        cout << endl;
    }

    Array<String> namespaces;
    namespaces.append(NAMESPACE.getString());
    Array<Uint16> providerType;
    providerType.append(_METHOD_PROVIDER);
    providerType.append(_INDICATION_PROVIDER);

    //
    //  Test failure of a provider in invokeMethod
    //
    _testScenario4(
        client,
        userContext,
        String("OOPModuleFailureTestProviderModule"),
        String("OOPModuleInvokeFailureTestProvider"),
        String("OOPCapability007"),
        String("FailureTestIndication"),
        namespaces,
        providerType);

    if (verbose)
    {
        cout <<
        "+++++ Test of failure of provider in invokeMethod method "
        "completed successfully" <<
        endl;
    }

    providerType.clear();
    providerType.append(_METHOD_PROVIDER);

    //
    //  Test failure of provider module with no indication providers
    //
    _testScenario1(
        client,
        userContext,
        String("OOPModuleFailureTestProviderModule"),
        String("OOPModuleInvokeFailureTestProvider"),
        String("OOPCapability001"),
        String("FailureTestIndication"),
        namespaces,
        providerType);

    if (verbose)
    {
        cout <<
        "+++++ Test of failure of provider module with no indication providers "
        "completed successfully" <<
        endl;
    }

    providerType.clear();
    providerType.append(_INDICATION_PROVIDER);

    //
    //  Test failure of a provider in initialize
    //
    _testScenario2(
        client,
        userContext,
        String("OOPModuleFailureTestProviderModule"),
        String("OOPModuleInitFailureTestProvider"),
        String("OOPCapability002"),
        String("FailureTestIndication"),
        namespaces,
        providerType);

    if (verbose)
    {
        cout <<
        "+++++ Test of failure of provider in initialize method "
        "completed successfully" <<
        endl;
    }

    //
    //  Test failure of a provider in createSubscription
    //
    _testScenario2(
        client,
        userContext,
        String("OOPModuleFailureTestProviderModule"),
        String("OOPModuleCreateFailureTestProvider"),
        String("OOPCapability003"),
        String("FailureTestIndication"),
        namespaces,
        providerType);

    if (verbose)
    {
        cout <<
        "+++++ Test of failure of provider in createSubscription method "
        "completed successfully" <<
        endl;
    }

    //
    //  Test failure of a provider in enableIndications
    //
    _testScenario2(
        client,
        userContext,
        String("OOPModuleFailureTestProviderModule"),
        String("OOPModuleEnableFailureTestProvider"),
        String("OOPCapability004"),
        String("FailureTestIndication"),
        namespaces,
        providerType);

    if (verbose)
    {
        cout <<
        "+++++ Test of failure of provider in enableIndications method "
        "completed successfully" <<
        endl;
    }

    providerType.append(_METHOD_PROVIDER);

    //
    //  Test failure of a provider in modifySubscription
    //
    _testScenario4(
        client,
        userContext,
        String("OOPModuleFailureTestProviderModule"),
        String("OOPModuleModifyFailureTestProvider"),
        String("OOPCapability005"),
        String("FailureTestIndication"),
        namespaces,
        providerType);

    if (verbose)
    {
        cout <<
        "+++++ Test of failure of provider in modifySubscription method "
        "completed successfully" <<
        endl;
    }

    //
    //  Test failure of a provider in createSubscription
    //  with an existing subscription
    //
    _testScenario3(
        client,
        userContext,
        String("OOPModuleFailureTestProviderModule"),
        String("OOPModuleCreate2FailureTestProvider"),
        String("OOPCapability006"),
        String("FailureTestIndication"),
        namespaces,
        providerType);

    if (verbose)
    {
        cout <<
        "+++++ Test of failure of provider in createSubscription method "
        "with an existing subscription completed successfully" <<
        endl;
    }

    //
    //  Test failure of a provider in deleteSubscription
    //  with a remaining subscription
    //
    _testScenario5(
        client,
        userContext,
        String("OOPModuleFailureTestProviderModule"),
        String("OOPModuleDelete2FailureTestProvider"),
        String("OOPCapability008"),
        String("FailureTestIndication"),
        namespaces,
        providerType);

    if (verbose)
    {
        cout <<
        "+++++ Test of failure of provider in deleteSubscription method "
        "with a remaining subscription completed successfully" <<
        endl;
    }

    //
    //  Test failure of a provider in deleteSubscription
    //
    _testScenario6(
        client,
        userContext,
        String("OOPModuleFailureTestProviderModule"),
        String("OOPModuleDeleteFailureTestProvider"),
        String("OOPCapability009"),
        String("FailureTestIndication"),
        namespaces,
        providerType);

    if (verbose)
    {
        cout <<
        "+++++ Test of failure of provider in deleteSubscription method "
        "completed successfully" <<
        endl;
    }

    //
    //  Test failure of a provider in disableIndications
    //
    _testScenario6(
        client,
        userContext,
        String("OOPModuleFailureTestProviderModule"),
        String("OOPModuleDisableFailureTestProvider"),
        String("OOPCapability010"),
        String("FailureTestIndication"),
        namespaces,
        providerType);

    if (verbose)
    {
        cout <<
        "+++++ Test of failure of provider in disableIndications method "
        "completed successfully" <<
        endl;
    }

    //
    //  Test failure of a provider in terminate
    //
    _testScenario7(
        client,
        userContext,
        String("OOPModuleFailureTestProviderModule"),
        String("OOPModuleTermFailureTestProvider"),
        String("OOPCapability011"),
        String("FailureTestIndication"),
        namespaces,
        providerType);

    if (verbose)
    {
        cout <<
        "+++++ Test of failure of provider in terminate method "
        "completed successfully" <<
        endl;
    }

    //
    //  Test failure with a provider in another module continuing to
    //  serve the subscription
    //
    _testScenario8(
        client,
        userContext,
        String("OOPModuleFailureTestProviderModule"),
        String("OOPModuleInvokeFailureTestProvider"),
        String("OOPCapability007"),
        String("FailureTestIndication"),
        namespaces,
        providerType);

    if (verbose)
    {
        cout <<
        "+++++ Test of failure of provider with provider in another module "
        "continuing to serve the subscription completed successfully" <<
        endl;
    }

    //
    //  Check Subscription's OnFatalErrorPolicy implementation.
    //
    _testScenario9(
        client,
        userContext,
        String("OOPModuleFailureTestProviderModule"),
        String("OOPModuleInvokeFailureTestProvider"),
        String("OOPCapability012"),
        String("FailureTestIndication"),
        namespaces,
        providerType);

    if (verbose)
    {
        cout <<
        "+++++ Test of Subscription's OnFatalErrorPolicy implementation "
            "completed successfully" <<
        endl;
    }
}

void _testISInita(CIMClient& client)
{
    if (verbose)
    {
        cout <<
        "+++++ Testing OOP provider module failure during Indication Service "
        "initialization (test setup) " <<
        endl;
    }

    _setup(client);

    Array<String> namespaces;
    namespaces.append(NAMESPACE.getString());
    Array<Uint16> providerType;
    providerType.append(_INDICATION_PROVIDER);

    //
    //  Register the OOPModuleModifyFailureTestProvider provider
    //
    _register(
        client,
        PG_PROVMODULE_USERCTXT_REQUESTOR,
        String("OOPModuleFailureTestProviderModule"),
        String("OOPModuleModifyFailureTestProvider"),
        String("OOPCapability005"),
        String("FailureTestIndication"),
        namespaces,
        providerType,
        CIMPropertyList(),
        CIMPropertyList());

    //
    //  Create subscription
    //
    _createSubscription(client, String("OOPFilter01"));

    //
    //  De-register OOPModuleModifyFailureTestProvider
    //  Subscription remains enabled but has no provider to serve it
    //
    _deleteCapabilityInstance(
        client,
        String("OOPModuleFailureTestProviderModule"),
        String("OOPModuleModifyFailureTestProvider"),
        String("OOPCapability005"));
    _deleteProviderInstance(
        client,
        String("OOPModuleModifyFailureTestProvider"),
        String("OOPModuleFailureTestProviderModule"));

    //
    //  Register the OOPModuleInitFailureTestProvider provider
    //  The OOPModuleInitFailureTestProvider provider can serve the subscription
    //  but will fail in the initialize method
    //
    _createProviderInstance(
        client,
        String("OOPModuleInitFailureTestProvider"),
        String("OOPModuleFailureTestProviderModule"));
    _createCapabilityInstance(
        client,
        String("OOPModuleFailureTestProviderModule"),
        String("OOPModuleInitFailureTestProvider"),
        String("OOPCapability002"),
        String("FailureTestIndication"),
        namespaces,
        providerType,
        CIMPropertyList(),
        CIMPropertyList());

    //
    //  Verify module status is OK
    //
    _checkStatus(client, String("OOPModuleFailureTestProviderModule"),
        CIM_MSE_OPSTATUS_VALUE_OK);

    if (verbose)
    {
        cout <<
        "+++++ Test of OOP provider module failure during Indication Service "
        "initialization (test setup) "
        "completed successfully" <<
        endl;
    }
}

void _testISInitb(CIMClient& client)
{
    if (verbose)
    {
        cout <<
        "+++++ Testing OOP provider module failure during Indication Service "
        "initialization (test) " <<
        endl;
    }

    //
    //  Upon IndicationService initialization, a createSubscription request is
    //  made to the OOPModuleInitFailureTestProvider, causing the
    //  OOPModuleInitFailureTestProvider to fail
    //

    //
    //  Verify module status is OK
    //
    _checkStatus(client, String("OOPModuleFailureTestProviderModule"),
        CIM_MSE_OPSTATUS_VALUE_OK);

    //
    //  Delete subscription
    //
    _deleteSubscriptionInstance(client, String("OOPFilter01"),
        String("OOPHandler01"));

    //
    //  De-register the OOPModuleInitFailureTestProvider
    //
    _deregister(
        client,
        String("OOPModuleFailureTestProviderModule"),
        String("OOPModuleInitFailureTestProvider"),
        String("OOPCapability002"));

    _cleanup(client);

    if (verbose)
    {
        cout <<
        "+++++ Test of OOP provider module failure during Indication Service "
        "initialization (test) "
        "completed successfully" <<
        endl;
    }
}

void _testCSRestarta(CIMClient& client)
{
    if (verbose)
    {
        cout <<
        "+++++ Testing that OOP provider module failure degraded status does "
        "not persist across CIM Server restarts " <<
        endl;
    }

    _setup(client);

    Array<String> namespaces;
    namespaces.append(NAMESPACE.getString());
    Array<Uint16> providerType;
    providerType.append(_INDICATION_PROVIDER);
    providerType.append(_METHOD_PROVIDER);

    //
    //  Register the OOPModuleCreate2FailureTestProvider provider
    //
    _register(
        client,
        PG_PROVMODULE_USERCTXT_REQUESTOR,
        String("OOPModuleFailureTestProviderModule"),
        String("OOPModuleCreate2FailureTestProvider"),
        String("OOPCapability006"),
        String("FailureTestIndication"),
        namespaces,
        providerType,
        CIMPropertyList(),
        CIMPropertyList());

    //
    //  Create a subscription
    //
    _createSubscription(client, String("OOPFilter01"));

    //
    //  Invoke method to send test indication
    //
    String identifier = "CSRestarta: OOPModuleCreate2FailureTestProvider";
    _invokeMethod(client, String("SendTestIndication"), identifier);
    PEGASUS_TEST_ASSERT(_validateIndicationReceipt(identifier));

    //
    //  Create a subscription that will cause provider failure
    //
    try
    {
        _createSubscription(client, String("OOPFilter02"));
        PEGASUS_TEST_ASSERT(false);
    }
    catch (const CIMException& e)
    {
        _checkExceptionCode(e, CIM_ERR_NOT_SUPPORTED);
    }

    //
    //  Verify module status is Degraded
    //
    _checkStatus(client, String("OOPModuleFailureTestProviderModule"),
        CIM_MSE_OPSTATUS_VALUE_DEGRADED);

    if (verbose)
    {
        cout <<
        "+++++ Test that OOP provider module failure degraded status does "
        "not persist across CIM Server restarts " <<
        "completed successfully" <<
        endl;
    }
}

void _testCSRestartb(CIMClient& client)
{
    if (verbose)
    {
        cout <<
        "+++++ Testing that OOP provider module failure degraded status does "
        "not persist across CIM Server restarts " <<
        endl;
    }

    //
    //  Upon IndicationService initialization, a createSubscription request is
    //  made to the OOPModuleCreate2FailureTestProvider
    //

    //
    //  Verify module status is OK
    //
    _checkStatus(client, String("OOPModuleFailureTestProviderModule"),
        CIM_MSE_OPSTATUS_VALUE_OK);

    //
    //  Delete subscription
    //
    _deleteSubscriptionInstance(client, String("OOPFilter01"),
        String("OOPHandler01"));

    //
    //  De-register the OOPModuleCreate2FailureTestProvider
    //
    _deregister(
        client,
        String("OOPModuleFailureTestProviderModule"),
        String("OOPModuleCreate2FailureTestProvider"),
        String("OOPCapability006"));

    _cleanup(client);

    if (verbose)
    {
        cout <<
        "+++++ Test of OOP provider module failure during Indication Service "
        "initialization (test) "
        "completed successfully" <<
        endl;
    }
}

void _test(CIMClient& client, char * argv0)
{
    _setup(client);

    _testScenarios(client, 0);

#ifndef PEGASUS_DISABLE_PROV_USERCTXT
    _testScenarios(client, PG_PROVMODULE_USERCTXT_PRIVILEGED);
    _testScenarios(client, PG_PROVMODULE_USERCTXT_REQUESTOR);
#else
    cout << argv0 <<
        ": user context test scenarios skipped because "
        "PEGASUS_DISABLE_PROV_USERCTXT is defined" << endl;
#endif

    _cleanup(client);
}

int main(int argc, char** argv)
{
    verbose = getenv("PEGASUS_TEST_VERBOSE") ? true : false;

    CIMClient client;
    client.connectLocal();

    try
    {
        if ((argc == 2) && !strcmp(argv[1], "inita"))
        {
#ifndef PEGASUS_DISABLE_PROV_USERCTXT
            _testISInita(client);
#else
            cout << argv[0] <<
                ": Indication Service initialization tests skipped because"
                " PEGASUS_DISABLE_PROV_USERCTXT is defined" << endl;
#endif
        }

        else if ((argc == 2) && !strcmp(argv[1], "initb"))
        {
#ifndef PEGASUS_DISABLE_PROV_USERCTXT
            _testISInitb(client);
#else
            cout << argv [0] <<
                ": Indication Service initialization tests skipped because"
                " PEGASUS_DISABLE_PROV_USERCTXT is defined" << endl;
#endif
        }

        else if ((argc == 2) && !strcmp(argv[1], "restarta"))
        {
#ifndef PEGASUS_DISABLE_PROV_USERCTXT
            _testCSRestarta(client);
#else
            cout << argv [0] <<
                ": CIM Server restart tests skipped because"
                " PEGASUS_DISABLE_PROV_USERCTXT is defined" << endl;
#endif
        }

        else if ((argc == 2) && !strcmp(argv[1], "restartb"))
        {
#ifndef PEGASUS_DISABLE_PROV_USERCTXT
            _testCSRestartb(client);
#else
            cout << argv [0] <<
                ": CIM Server restart tests skipped because"
                " PEGASUS_DISABLE_PROV_USERCTXT is defined" << endl;
#endif
        }

        else if (argc == 1)
        {
            _test(client, argv [0]);
        }
    }
    catch (Exception& e)
    {
        cerr << argv [0] << " failed: " << e.getMessage() << endl;
        return 1;
    }

    cout << argv [0] << " +++++ passed all tests" << endl;
    return 0;
}
