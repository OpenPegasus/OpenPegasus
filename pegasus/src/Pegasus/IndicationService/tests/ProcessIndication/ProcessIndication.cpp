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
#include <Pegasus/Common/System.h>
#include <Pegasus/Common/FileSystem.h>
#include <Pegasus/Common/InternalException.h>
#include <Pegasus/Client/CIMClient.h>

PEGASUS_USING_PEGASUS;
PEGASUS_USING_STD;

// Interop namespace used with PEGASUS_NAMESPACENAME_INTEROP in Constants.h
const CIMNamespaceName SOURCENAMESPACE =
    CIMNamespaceName ("test/TestProvider");

void _createHandlerInstance
    (CIMClient & client,
     const String & name,
     const String & destination)
{
    CIMInstance handlerInstance (PEGASUS_CLASSNAME_INDHANDLER_CIMXML);
    handlerInstance.addProperty (CIMProperty (CIMName
        ("SystemCreationClassName"), System::getSystemCreationClassName()));
    handlerInstance.addProperty (CIMProperty (CIMName ("SystemName"),
        System::getFullyQualifiedHostName()));
    handlerInstance.addProperty (CIMProperty (CIMName ("CreationClassName"),
        PEGASUS_CLASSNAME_INDHANDLER_CIMXML.getString()));
    handlerInstance.addProperty (CIMProperty (CIMName ("Name"), name));
    handlerInstance.addProperty (CIMProperty (CIMName ("Destination"),
        destination));

    CIMObjectPath path = client.createInstance (PEGASUS_NAMESPACENAME_INTEROP,
        handlerInstance);
}

void _createFilterInstance
    (CIMClient & client,
     const String & name,
     const String & query,
     const String & qlang)
{
    CIMInstance filterInstance (PEGASUS_CLASSNAME_INDFILTER);
    filterInstance.addProperty (CIMProperty (CIMName
        ("SystemCreationClassName"), System::getSystemCreationClassName()));
    filterInstance.addProperty (CIMProperty (CIMName ("SystemName"),
        System::getFullyQualifiedHostName()));
    filterInstance.addProperty (CIMProperty (CIMName ("CreationClassName"),
        PEGASUS_CLASSNAME_INDFILTER.getString()));
    filterInstance.addProperty (CIMProperty (CIMName ("Name"), name));
    filterInstance.addProperty (CIMProperty (CIMName ("Query"), query));
    filterInstance.addProperty (CIMProperty (CIMName ("QueryLanguage"),
        String (qlang)));
    filterInstance.addProperty (CIMProperty (CIMName ("SourceNamespace"),
        SOURCENAMESPACE.getString()));

    CIMObjectPath path = client.createInstance (PEGASUS_NAMESPACENAME_INTEROP,
        filterInstance);
}

void _createSubscriptionInstance
    (CIMClient & client,
     const CIMObjectPath & filterPath,
     const CIMObjectPath & handlerPath)
{
    CIMInstance subscriptionInstance (PEGASUS_CLASSNAME_INDSUBSCRIPTION);
    subscriptionInstance.addProperty (CIMProperty (CIMName ("Filter"),
        filterPath, 0, PEGASUS_CLASSNAME_INDFILTER));
    subscriptionInstance.addProperty (CIMProperty (CIMName ("Handler"),
        handlerPath, 0, PEGASUS_CLASSNAME_INDHANDLER_CIMXML));
    subscriptionInstance.addProperty (CIMProperty
        (CIMName ("SubscriptionState"), CIMValue ((Uint16) 2)));

    CIMObjectPath path = client.createInstance (PEGASUS_NAMESPACENAME_INTEROP,
        subscriptionInstance);
}

void _renameLogFile (const String & indicationLogFileName)
{
    String indicationLogFailedFileName;

    //
    //  Rename the indication log file upon verification failure
    //
    indicationLogFailedFileName = INDICATION_DIR;
    indicationLogFailedFileName.append ("/indicationLog_FAILED");
    FileSystem::renameFile (indicationLogFileName, indicationLogFailedFileName);
}

Boolean _checkIndicationLog
    (const String & methodName,
     Boolean allProperties,
     Boolean noIndications,
     const String & qlang)
{
    String indicationLogFileName;

    indicationLogFileName = INDICATION_DIR;
    indicationLogFileName.append ("/indicationLog");
    if (FileSystem::exists (indicationLogFileName))
    {
        if (noIndications)
        {
            _renameLogFile (indicationLogFileName);
            return false;
        }

        try
        {
            Buffer contents;
            FileSystem::loadFileToMemory (contents, indicationLogFileName);
            const char * theLog = contents.getData();
            String log (theLog);
            Uint32 newline;
            newline = log.find ('\n');
            if (newline == PEG_NOT_FOUND)
            {
                _renameLogFile (indicationLogFileName);
                return false;
            }
            String header = log.subString (0, newline);
            if (header [header.size() - 1] == '\r')
            {
                header = header.subString (0, newline - 1);
            }
            if (!String::equal (header,
                "++++++++++++++ Received Indication +++++++++++++++++"))
            {
                _renameLogFile (indicationLogFileName);
                return false;
            }
            if (log.size() > (newline + 1))
            {
                log = log.subString (newline + 1);
            }
            else
            {
                _renameLogFile (indicationLogFileName);
                return false;
            }
            Uint32 numProperties;
            if (allProperties)
            {
                numProperties = 4;
            }
            else
            {
                numProperties = 2;
            }

            if (String::equal (methodName,
                String ("SendTestIndicationMissingProperty")))
            {
                //
                //  indication instance will contain one fewer property
                //
                numProperties--;
            }

            String propertyName;
            String propertyValue;
            for (Uint32 i = 0; i < numProperties; i++)
            {
                newline = log.find ('\n');
                if (newline == PEG_NOT_FOUND)
                {
                    _renameLogFile (indicationLogFileName);
                    return false;
                }
                String line = log.subString (0, newline);
                if (line [line.size() - 1] == '\r')
                {
                    line = line.subString (0, newline - 1);
                }
                Uint32 eq = line.find (String (" = "));
                if (eq == PEG_NOT_FOUND)
                {
                    _renameLogFile (indicationLogFileName);
                    return false;
                }
                propertyName.clear();
                propertyValue.clear();
                propertyName = line.subString (0, eq);
                if (line.size() > (eq + 3))
                {
                    propertyValue = line.subString (eq + 3);
                }
                if (String::equalNoCase (propertyName, "IndicationIdentifier"))
                {
                    if (!allProperties)
                    {
                        _renameLogFile (indicationLogFileName);
                        return false;
                    }
                }
                else if (String::equalNoCase (propertyName,
                    "CorrelatedIndications"))
                {
                    //
                    //  CorrelatedIndications property should not be present for
                    //  SendTestIndicationMissingProperty method
                    //
                    if (String::equal (methodName,
                        String ("SendTestIndicationMissingProperty")))
                    {
                        _renameLogFile (indicationLogFileName);
                        return false;
                    }

                    else if (propertyValue.size() != 0)
                    {
                        _renameLogFile (indicationLogFileName);
                        return false;
                    }
                }
                else if (String::equalNoCase (propertyName,
                    "MethodName"))
                {
                    if (!String::equal (propertyValue, methodName))
                    {
                        _renameLogFile (indicationLogFileName);
                        return false;
                    }
                }
                else if (String::equalNoCase (propertyName, "IndicationTime"))
                {
                    if (!allProperties)
                    {
                        _renameLogFile (indicationLogFileName);
                        return false;
                    }
                }
                else
                {
                    _renameLogFile (indicationLogFileName);
                    return false;
                }
                if (log.size() > (newline + 1))
                {
                    log = log.subString (newline + 1);
                }
                else
                {
                    _renameLogFile (indicationLogFileName);
                    return false;
                }
            }

            newline = log.find ('\n');
            if (newline == PEG_NOT_FOUND)
            {
                _renameLogFile (indicationLogFileName);
                return false;
            }
            String footer = log.subString (0, newline);
            if (footer [footer.size() - 1] == '\r')
            {
                footer = footer.subString (0, newline - 1);
            }
            if (!String::equal (footer,
                "++++++++++++++++++++++++++++++++++++++++++++++++++++"))
            {
                _renameLogFile (indicationLogFileName);
                return false;
            }
            if (log.size() > newline + 1)
            {
                log = log.subString (newline + 1);
                if (log [0] == '\r')
                {
                    log = log.subString (1);
                }
                if ((log.size() != 1) || (log [0] != '\n'))
                {
                    _renameLogFile (indicationLogFileName);
                    return false;
                }
            }
            else
            {
                _renameLogFile (indicationLogFileName);
                return false;
            }

            //
            //  Remove the indication log file on successful verification
            //
            FileSystem::removeFile (indicationLogFileName);
            return true;
        }
        catch (CannotOpenFile &)
        {
            _renameLogFile (indicationLogFileName);
            return false;
        }
        catch (...)
        {
            _renameLogFile (indicationLogFileName);
            return false;
        }
    }
    else if (noIndications)
    {
        return true;
    }
    else
    {
        return false;
    }
}

void _sendTestIndication
    (CIMClient & client,
    const CIMName & methodName)
{
    //
    //  Remove previous indication log file, if there
    //
    String previousIndicationFile, oldIndicationFile;

    previousIndicationFile = INDICATION_DIR;
    previousIndicationFile.append ("/indicationLog");

    if (FileSystem::exists (previousIndicationFile))
    {
        oldIndicationFile = INDICATION_DIR;
        oldIndicationFile.append ("/oldIndicationFile");
        if (FileSystem::exists (oldIndicationFile))
        {
            FileSystem::removeFile (oldIndicationFile);
        }
        if (!FileSystem::renameFile (previousIndicationFile, oldIndicationFile))
        {
            FileSystem::removeFile (previousIndicationFile);
        }
    }

    //
    //  Invoke method to send test indication
    //
    Array <CIMParamValue> inParams;
    Array <CIMParamValue> outParams;
    Array <CIMKeyBinding> keyBindings;
    Sint32 result;

    CIMValue retValue;

    if (methodName.equal ("SendTestIndicationSubclass"))
    {
        CIMObjectPath className (String::EMPTY, CIMNamespaceName(),
            CIMName ("Test_IndicationProviderSubclass"), keyBindings);

        retValue = client.invokeMethod
            (SOURCENAMESPACE,
            className,
            methodName,
            inParams,
            outParams);
    }
    else
    {
        CIMObjectPath className (String::EMPTY, CIMNamespaceName(),
            CIMName ("Test_IndicationProviderClass"), keyBindings);

        retValue = client.invokeMethod
            (SOURCENAMESPACE,
            className,
            methodName,
            inParams,
            outParams);
    }

    retValue.get (result);
    PEGASUS_TEST_ASSERT (result == 0);

    //
    //  Allow time for the indication to be received and forwarded
    //
    System::sleep (5);
}

void _sendTestIndicationNormal
    (CIMClient & client)
{
    _sendTestIndication (client, CIMName ("SendTestIndicationNormal"));
}

void _sendTestIndicationSubclass
    (CIMClient & client)
{
    _sendTestIndication (client, CIMName ("SendTestIndicationSubclass"));
}

void _sendTestIndicationMissing
    (CIMClient & client)
{
    _sendTestIndication (client, CIMName ("SendTestIndicationMissingProperty"));
}

void _sendTestIndicationExtra
    (CIMClient & client)
{
    _sendTestIndication (client, CIMName ("SendTestIndicationExtraProperty"));
}

void _sendTestIndicationMatching
    (CIMClient & client)
{
    _sendTestIndication (client,
        CIMName ("SendTestIndicationMatchingInstance"));
}

void _sendTestIndicationUnmatchingNamespace
    (CIMClient & client)
{
    _sendTestIndication (client,
        CIMName ("SendTestIndicationUnmatchingNamespace"));
}

void _sendTestIndicationUnmatchingClassName
    (CIMClient & client)
{
    _sendTestIndication (client,
        CIMName ("SendTestIndicationUnmatchingClassName"));
}

void _deleteSubscriptionInstance
    (CIMClient & client,
     const String & filterName,
     const String & handlerName)
{
    Array<CIMKeyBinding> filterKeyBindings;
    filterKeyBindings.append (CIMKeyBinding ("SystemCreationClassName",
        System::getSystemCreationClassName(), CIMKeyBinding::STRING));
    filterKeyBindings.append (CIMKeyBinding ("SystemName",
        System::getFullyQualifiedHostName(), CIMKeyBinding::STRING));
    filterKeyBindings.append (CIMKeyBinding ("CreationClassName",
        PEGASUS_CLASSNAME_INDFILTER.getString(), CIMKeyBinding::STRING));
    filterKeyBindings.append (CIMKeyBinding ("Name", filterName,
        CIMKeyBinding::STRING));
    CIMObjectPath filterPath ("", CIMNamespaceName(),
        PEGASUS_CLASSNAME_INDFILTER, filterKeyBindings);

    Array<CIMKeyBinding> handlerKeyBindings;
    handlerKeyBindings.append (CIMKeyBinding ("SystemCreationClassName",
        System::getSystemCreationClassName(), CIMKeyBinding::STRING));
    handlerKeyBindings.append (CIMKeyBinding ("SystemName",
        System::getFullyQualifiedHostName(), CIMKeyBinding::STRING));
    handlerKeyBindings.append (CIMKeyBinding ("CreationClassName",
        PEGASUS_CLASSNAME_INDHANDLER_CIMXML.getString(),
        CIMKeyBinding::STRING));
    handlerKeyBindings.append (CIMKeyBinding ("Name", handlerName,
        CIMKeyBinding::STRING));
    CIMObjectPath handlerPath ("", CIMNamespaceName(),
        PEGASUS_CLASSNAME_INDHANDLER_CIMXML, handlerKeyBindings);

    Array<CIMKeyBinding> subscriptionKeyBindings;
    subscriptionKeyBindings.append (CIMKeyBinding ("Filter",
        filterPath.toString(), CIMKeyBinding::REFERENCE));
    subscriptionKeyBindings.append (CIMKeyBinding ("Handler",
        handlerPath.toString(), CIMKeyBinding::REFERENCE));
    CIMObjectPath subscriptionPath ("", CIMNamespaceName(),
        PEGASUS_CLASSNAME_INDSUBSCRIPTION, subscriptionKeyBindings);
    client.deleteInstance (PEGASUS_NAMESPACENAME_INTEROP, subscriptionPath);
}

void _deleteHandlerInstance
    (CIMClient & client,
     const String & name)
{
    Array<CIMKeyBinding> keyBindings;
    keyBindings.append (CIMKeyBinding ("SystemCreationClassName",
        System::getSystemCreationClassName(), CIMKeyBinding::STRING));
    keyBindings.append (CIMKeyBinding ("SystemName",
        System::getFullyQualifiedHostName(), CIMKeyBinding::STRING));
    keyBindings.append (CIMKeyBinding ("CreationClassName",
        PEGASUS_CLASSNAME_INDHANDLER_CIMXML.getString(),
        CIMKeyBinding::STRING));
    keyBindings.append (CIMKeyBinding ("Name", name,
        CIMKeyBinding::STRING));
    CIMObjectPath path ("", CIMNamespaceName(),
        PEGASUS_CLASSNAME_INDHANDLER_CIMXML, keyBindings);
    client.deleteInstance (PEGASUS_NAMESPACENAME_INTEROP, path);
}

void _deleteFilterInstance
    (CIMClient & client,
     const String & name)
{
    Array<CIMKeyBinding> keyBindings;
    keyBindings.append (CIMKeyBinding ("SystemCreationClassName",
        System::getSystemCreationClassName(), CIMKeyBinding::STRING));
    keyBindings.append (CIMKeyBinding ("SystemName",
        System::getFullyQualifiedHostName(), CIMKeyBinding::STRING));
    keyBindings.append (CIMKeyBinding ("CreationClassName",
        PEGASUS_CLASSNAME_INDFILTER.getString(), CIMKeyBinding::STRING));
    keyBindings.append (CIMKeyBinding ("Name", name,
        CIMKeyBinding::STRING));
    CIMObjectPath path ("", CIMNamespaceName(),
        PEGASUS_CLASSNAME_INDFILTER, keyBindings);
    client.deleteInstance (PEGASUS_NAMESPACENAME_INTEROP, path);
}

void _usage()
{
    PEGASUS_STD (cerr)
        << "Usage: TestProcessIndication "
        << "{setup | create1 | create2 | create3 | create4 "
        << "| sendNormal | sendMissing | sendExtra "
        << "| sendMatching | sendUnmatchingNamespace | sendUnmatchingClassName "
        << "| checkNormal | checkMissing | checkExtra | checkMatching "
        << "| checkUnmatchingNamespace | checkUnmatchingClassName "
        << "| checkNormalAll | checkMissingAll | checkExtraAll "
        << "| checkNormalWhere | checkMissingWhere "
        << "| checkNormalWhereNotSatisfied "
        << "| delete1 | delete2 | delete3 | delete4 | cleanup} {WQL | DMTF:CQL}"
        << PEGASUS_STD (endl);
}

void _setup (CIMClient & client, String& qlang)
{
    try
    {
        _createFilterInstance (client, String ("PIFilter01"), String
            ("SELECT MethodName, CorrelatedIndications "
             "FROM Test_IndicationProviderClass"),
            qlang);
        _createFilterInstance (client, String ("PIFilter02"),
            String ("SELECT * FROM Test_IndicationProviderClass"),
            qlang);

        //
        //  Filter03 and Filter04 are not created for WQL because WQL does not
        //  support array properties in the WHERE clause
        //
        if (qlang != "WQL")
        {
            //
            //  The following filters are used to test that only properties
            //  required for the WHERE clause are treated as required for
            //  indications.
            //  Normally, the IndicationTestProvider generates an
            //  indication that includes a non-NULL CorrelatedIndications
            //  property value.
            //  When the SendTestIndicationMissingProperty method is invoked,
            //  the IndicationTestProvider generates an indication that
            //  is missing the CorrelatedIndications property.
            //  For PIFilter03, normally the generated indication includes all
            //  required properties, and the generated indication satisfies the
            //  query condition (WHERE clause).
            //  However, when the SendTestIndicationMissingProperty method is
            //  used, the generated indication does not include all the required
            //  properties, and the indication is not forwarded.
            //  For PIFilter04, normally the generated indication includes all
            //  required properties, but the generated indication does not
            //  satisfy the query condition (WHERE clause), because the
            //  CorrelatedIndications property has a non-NULL value, so the
            //  indication is not forwarded.
            //  When the SendTestIndicationMissingProperty method is used, the
            //  generated indication does not include all the required
            //  properties, and the indication is not forwarded.
            //
            _createFilterInstance (client, String ("PIFilter03"), String
                ("SELECT MethodName, "
                 "CorrelatedIndications "
                 "FROM Test_IndicationProviderClass "
                 "WHERE CorrelatedIndications IS NOT NULL"),
                qlang);
            _createFilterInstance (client, String ("PIFilter04"), String
                ("SELECT MethodName, "
                 "CorrelatedIndications "
                 "FROM Test_IndicationProviderClass "
                 "WHERE CorrelatedIndications IS NULL"),
                qlang);
        }

        _createHandlerInstance (client, String ("PIHandler01"),
            String ("localhost/CIMListener/Pegasus_SimpleDisplayConsumer"));
    }
    catch (Exception & e)
    {
        PEGASUS_STD (cerr) << "setup failed: " << e.getMessage()
                           << PEGASUS_STD (endl);
        exit (-1);
    }

    PEGASUS_STD (cout) << "+++++ setup completed successfully"
                       << PEGASUS_STD (endl);
}

void _create1 (CIMClient & client)
{
    try
    {
        String filterPathString;
        filterPathString.append (
            "CIM_IndicationFilter.CreationClassName=\"CIM_IndicationFilter\","
                "Name=\"PIFilter01\",SystemCreationClassName=\"");
        filterPathString.append (System::getSystemCreationClassName());
        filterPathString.append ("\",SystemName=\"");
        filterPathString.append (System::getFullyQualifiedHostName());
        filterPathString.append ("\"");
        String handlerPathString;
        handlerPathString.append (
            "CIM_IndicationHandlerCIMXML."
                "CreationClassName=\"CIM_IndicationHandlerCIMXML\","
                "Name=\"PIHandler01\",SystemCreationClassName=\"");
        handlerPathString.append (System::getSystemCreationClassName());
        handlerPathString.append ("\",SystemName=\"");
        handlerPathString.append (System::getFullyQualifiedHostName());
        handlerPathString.append ("\"");
        _createSubscriptionInstance (client, CIMObjectPath (filterPathString),
            CIMObjectPath (handlerPathString));
    }
    catch (Exception & e)
    {
        PEGASUS_STD (cerr) << "create1 failed: " << e.getMessage()
                           << PEGASUS_STD (endl);
        exit (-1);
    }

    PEGASUS_STD (cout) << "+++++ create1 completed successfully"
                       << PEGASUS_STD (endl);
}

void _create2 (CIMClient & client)
{
    try
    {
        String filterPathString;
        filterPathString.append (
            "CIM_IndicationFilter.CreationClassName=\"CIM_IndicationFilter\","
                "Name=\"PIFilter02\",SystemCreationClassName=\"");
        filterPathString.append (System::getSystemCreationClassName());
        filterPathString.append ("\",SystemName=\"");
        filterPathString.append (System::getFullyQualifiedHostName());
        filterPathString.append ("\"");
        String handlerPathString;
        handlerPathString.append (
            "CIM_IndicationHandlerCIMXML."
                "CreationClassName=\"CIM_IndicationHandlerCIMXML\","
                "Name=\"PIHandler01\",SystemCreationClassName=\"");
        handlerPathString.append (System::getSystemCreationClassName());
        handlerPathString.append ("\",SystemName=\"");
        handlerPathString.append (System::getFullyQualifiedHostName());
        handlerPathString.append ("\"");
        _createSubscriptionInstance (client, CIMObjectPath (filterPathString),
            CIMObjectPath (handlerPathString));
    }
    catch (Exception & e)
    {
        PEGASUS_STD (cerr) << "create2 failed: " << e.getMessage()
                           << PEGASUS_STD (endl);
        exit (-1);
    }

    PEGASUS_STD (cout) << "+++++ create2 completed successfully"
                       << PEGASUS_STD (endl);
}

void _create3 (CIMClient & client)
{
    try
    {
        String filterPathString;
        filterPathString.append (
            "CIM_IndicationFilter.CreationClassName=\"CIM_IndicationFilter\","
                "Name=\"PIFilter03\",SystemCreationClassName=\"");
        filterPathString.append (System::getSystemCreationClassName());
        filterPathString.append ("\",SystemName=\"");
        filterPathString.append (System::getFullyQualifiedHostName());
        filterPathString.append ("\"");
        String handlerPathString;
        handlerPathString.append (
            "CIM_IndicationHandlerCIMXML."
                "CreationClassName=\"CIM_IndicationHandlerCIMXML\","
                "Name=\"PIHandler01\",SystemCreationClassName=\"");
        handlerPathString.append (System::getSystemCreationClassName());
        handlerPathString.append ("\",SystemName=\"");
        handlerPathString.append (System::getFullyQualifiedHostName());
        handlerPathString.append ("\"");
        _createSubscriptionInstance (client, CIMObjectPath (filterPathString),
            CIMObjectPath (handlerPathString));
    }
    catch (Exception & e)
    {
        PEGASUS_STD (cerr) << "create3 failed: " << e.getMessage()
                           << PEGASUS_STD (endl);
        exit (-1);
    }

    PEGASUS_STD (cout) << "+++++ create3 completed successfully"
                       << PEGASUS_STD (endl);
}

void _create4 (CIMClient & client)
{
    try
    {
        String filterPathString;
        filterPathString.append (
            "CIM_IndicationFilter.CreationClassName=\"CIM_IndicationFilter\","
                "Name=\"PIFilter04\",SystemCreationClassName=\"");
        filterPathString.append (System::getSystemCreationClassName());
        filterPathString.append ("\",SystemName=\"");
        filterPathString.append (System::getFullyQualifiedHostName());
        filterPathString.append ("\"");
        String handlerPathString;
        handlerPathString.append (
            "CIM_IndicationHandlerCIMXML."
                "CreationClassName=\"CIM_IndicationHandlerCIMXML\","
                "Name=\"PIHandler01\",SystemCreationClassName=\"");
        handlerPathString.append (System::getSystemCreationClassName());
        handlerPathString.append ("\",SystemName=\"");
        handlerPathString.append (System::getFullyQualifiedHostName());
        handlerPathString.append ("\"");
        _createSubscriptionInstance (client, CIMObjectPath (filterPathString),
            CIMObjectPath (handlerPathString));
    }
    catch (Exception & e)
    {
        PEGASUS_STD (cerr) << "create4 failed: " << e.getMessage()
                           << PEGASUS_STD (endl);
        exit (-1);
    }

    PEGASUS_STD (cout) << "+++++ create4 completed successfully"
                       << PEGASUS_STD (endl);
}

void _sendNormal (CIMClient & client)
{
    try
    {
        _sendTestIndicationNormal (client);
    }
    catch (Exception & e)
    {
        PEGASUS_STD (cerr) << "sendNormal failed: " << e.getMessage()
                           << PEGASUS_STD (endl);
        exit (-1);
    }

    PEGASUS_STD (cout) << "+++++ sendNormal completed successfully"
                       << PEGASUS_STD (endl);
}

void _sendSubclass (CIMClient & client)
{
    try
    {
        _sendTestIndicationSubclass (client);
    }
    catch (Exception & e)
    {
        PEGASUS_STD (cerr) << "sendSubclass failed: " << e.getMessage()
                           << PEGASUS_STD (endl);
        exit (-1);
    }

    PEGASUS_STD (cout) << "+++++ sendSubclass completed successfully"
                       << PEGASUS_STD (endl);
}

void _sendMissing (CIMClient & client)
{
    try
    {
        _sendTestIndicationMissing (client);
    }
    catch (Exception & e)
    {
        PEGASUS_STD (cerr) << "sendMissing failed: " << e.getMessage()
                           << PEGASUS_STD (endl);
        exit (-1);
    }

    PEGASUS_STD (cout) << "+++++ sendMissing completed successfully"
                       << PEGASUS_STD (endl);
}

void _sendExtra (CIMClient & client)
{
    try
    {
        _sendTestIndicationExtra (client);
    }
    catch (Exception & e)
    {
        PEGASUS_STD (cerr) << "sendExtra failed: " << e.getMessage()
                           << PEGASUS_STD (endl);
        exit (-1);
    }

    PEGASUS_STD (cout) << "+++++ sendExtra completed successfully"
                       << PEGASUS_STD (endl);
}

void _sendMatching (CIMClient & client)
{
    try
    {
        _sendTestIndicationMatching (client);
    }
    catch (Exception & e)
    {
        PEGASUS_STD (cerr) << "sendMatching failed: " << e.getMessage()
                           << PEGASUS_STD (endl);
        exit (-1);
    }

    PEGASUS_STD (cout) << "+++++ sendMatching completed successfully"
                       << PEGASUS_STD (endl);
}

void _sendUnmatchingNamespace (CIMClient & client)
{
    try
    {
        _sendTestIndicationUnmatchingNamespace (client);
    }
    catch (Exception & e)
    {
        PEGASUS_STD (cerr) << "sendUnmatchingNamespace failed: "
                           << e.getMessage()
                           << PEGASUS_STD (endl);
        exit (-1);
    }

    PEGASUS_STD (cout) << "+++++ sendUnmatchingNamespace completed successfully"
                       << PEGASUS_STD (endl);
}

void _sendUnmatchingClassName (CIMClient & client)
{
    try
    {
        _sendTestIndicationUnmatchingClassName (client);
    }
    catch (Exception & e)
    {
        PEGASUS_STD (cerr) << "sendUnmatchingClassName failed: "
                           << e.getMessage()
                           << PEGASUS_STD (endl);
        exit (-1);
    }

    PEGASUS_STD (cout) << "+++++ sendUnmatchingClassName completed successfully"
                       << PEGASUS_STD (endl);
}

void _check
    (const String & opt,
     const String & methodName,
     Boolean allProperties,
     Boolean noIndications,
     const String & qlang)
{
    Boolean result = _checkIndicationLog (methodName, allProperties,
        noIndications, qlang);

    if (!result)
    {
        PEGASUS_STD (cerr) << opt << " failed" << PEGASUS_STD (endl);
        exit (-1);
    }

    PEGASUS_STD (cout) << "+++++ " << opt << " completed successfully"
                       << PEGASUS_STD (endl);
}

void _delete1 (CIMClient & client)
{
    try
    {
        _deleteSubscriptionInstance (client, String ("PIFilter01"),
            String ("PIHandler01"));
    }
    catch (Exception & e)
    {
        PEGASUS_STD (cerr) << "delete1 failed: " << e.getMessage()
                           << PEGASUS_STD (endl);
        exit (-1);
    }

    PEGASUS_STD (cout) << "+++++ delete1 completed successfully"
                       << PEGASUS_STD (endl);
}

void _delete2 (CIMClient & client)
{
    try
    {
        _deleteSubscriptionInstance (client, String ("PIFilter02"),
            String ("PIHandler01"));
    }
    catch (Exception & e)
    {
        PEGASUS_STD (cerr) << "delete2 failed: " << e.getMessage()
                           << PEGASUS_STD (endl);
        exit (-1);
    }

    PEGASUS_STD (cout) << "+++++ delete2 completed successfully"
                       << PEGASUS_STD (endl);
}

void _delete3 (CIMClient & client)
{
    try
    {
        _deleteSubscriptionInstance (client, String ("PIFilter03"),
            String ("PIHandler01"));
    }
    catch (Exception & e)
    {
        PEGASUS_STD (cerr) << "delete3 failed: " << e.getMessage()
                           << PEGASUS_STD (endl);
        exit (-1);
    }

    PEGASUS_STD (cout) << "+++++ delete3 completed successfully"
                       << PEGASUS_STD (endl);
}

void _delete4 (CIMClient & client)
{
    try
    {
        _deleteSubscriptionInstance (client, String ("PIFilter04"),
            String ("PIHandler01"));
    }
    catch (Exception & e)
    {
        PEGASUS_STD (cerr) << "delete4 failed: " << e.getMessage()
                           << PEGASUS_STD (endl);
        exit (-1);
    }

    PEGASUS_STD (cout) << "+++++ delete4 completed successfully"
                       << PEGASUS_STD (endl);
}

void _cleanup (CIMClient & client, String & qlang)
{
    try
    {
        _deleteHandlerInstance (client, String ("PIHandler01"));
        _deleteFilterInstance (client, String ("PIFilter01"));
        _deleteFilterInstance (client, String ("PIFilter02"));

        //
        //  Filter03 and Filter04 are not created for WQL because WQL does not
        //  support array properties in the WHERE clause
        //
        if (qlang != "WQL")
        {
            _deleteFilterInstance (client, String ("PIFilter03"));
            _deleteFilterInstance (client, String ("PIFilter04"));
        }
    }
    catch (Exception & e)
    {
        PEGASUS_STD (cerr) << "cleanup failed: " << e.getMessage()
                           << PEGASUS_STD (endl);
        exit (-1);
    }

    PEGASUS_STD (cout) << "+++++ cleanup completed successfully"
                       << PEGASUS_STD (endl);
}

int _test(CIMClient& client, const char* opt, String& qlang)
{
  if (String::equalNoCase (opt, "setup"))
  {
    _setup (client, qlang);
  }
  else if (String::equalNoCase (opt, "create1"))
  {
    _create1 (client);
  }
  else if (String::equalNoCase (opt, "create2"))
  {
    _create2 (client);
  }
  else if (String::equalNoCase (opt, "create3"))
  {
    _create3 (client);
  }
  else if (String::equalNoCase (opt, "create4"))
  {
    _create4 (client);
  }
  else if (String::equalNoCase (opt, "sendNormal"))
  {
    _sendNormal (client);
  }
  else if (String::equalNoCase (opt, "sendSubclass"))
  {
    _sendSubclass (client);
  }
  else if (String::equalNoCase (opt, "sendMissing"))
  {
    _sendMissing (client);
  }
  else if (String::equalNoCase (opt, "sendExtra"))
  {
    _sendExtra (client);
  }
  else if (String::equalNoCase (opt, "sendMatching"))
  {
    _sendMatching (client);
  }
  else if (String::equalNoCase (opt, "sendUnmatchingNamespace"))
  {
    _sendUnmatchingNamespace (client);
  }
  else if (String::equalNoCase (opt, "sendUnmatchingClassName"))
  {
    _sendUnmatchingClassName (client);
  }
  else if (String::equalNoCase (opt, "checkNormal"))
  {
    //
    //  Check indications received by Simple Display Consumer
    //  Only the properties included in the SELECT list of the filter
    //  query should be included in the indication instance
    //
    _check (opt, String ("SendTestIndicationNormal"),
            false, false, qlang);
  }
  else if (String::equalNoCase (opt, "checkSubclass"))
  {
    //
    //  Check indications received by Simple Display Consumer
    //  Only the properties included in the SELECT list of the filter
    //  query should be included in the indication instance
    //
    _check (opt, String ("SendTestIndicationSubclass"), false, false, qlang);
  }
  else if (String::equalNoCase (opt, "checkMissing"))
  {
    //
    //  Check indications received by Simple Display Consumer
    //  An indication should be received because the missing property is a
    //  project list property, not a property required by the WHERE clause
    //
    _check (opt,
            String ("SendTestIndicationMissingProperty"), false, false, qlang);
  }
  else if (String::equalNoCase (opt, "checkExtra"))
  {
    //
    //  Check indications received by Simple Display Consumer
    //  The extra property added to the indication instance by the
    //  indication provider should not appear in the indication
    //  Only the properties included in the SELECT list of the filter
    //  query should be included in the indication instance
    //
    _check (opt,
            String ("SendTestIndicationExtraProperty"), false, false, qlang);
  }
  else if (String::equalNoCase (opt, "checkMatching"))
  {
    //
    //  Check indications received by Simple Display Consumer
    //  Only the properties included in the SELECT list of the filter
    //  query should be included in the indication instance
    //
    _check (opt,
            String ("SendTestIndicationMatchingInstance"), false, false, qlang);
  }
  else if (String::equalNoCase (opt, "checkUnmatchingNamespace"))
  {
    //
    //  Check indications received by Simple Display Consumer
    //  None should be received in this case, since the namespace of
    //  the generated indication instance does not match the filter
    //  source namespace of the subscription instance name in the
    //  operation context
    //
    _check (opt,
            String ("SendTestIndicationUnmatchingNamespace"),
            false, true, qlang);
  }
  else if (String::equalNoCase (opt, "checkUnmatchingClassName"))
  {
    //
    //  Check indications received by Simple Display Consumer
    //  None should be received in this case, since the class name of
    //  the generated indication instance does not match the filter
    //  query indication class of the subscription instance name in the
    //  operation context
    //
    _check (opt,
            String ("SendTestIndicationUnmatchingClassName"),
            false, true, qlang);
  }
  else if (String::equalNoCase (opt, "checkNormalAll"))
  {
    //
    //  Check indications received by Simple Display Consumer
    //  All properties should be included in the indication instance,
    //  since the filter query specifies SELECT *
    //
    _check (opt,
            String ("SendTestIndicationNormal"), true, false, qlang);
   }
  else if (String::equalNoCase (opt, "checkMissingAll"))
  {
    //
    //  Check indications received by Simple Display Consumer
    //  An indication should be received because the missing property is a
    //  project list property, not a property required by the WHERE clause
    //
    _check (opt,
            String ("SendTestIndicationMissingProperty"), true, false, qlang);
  }
  else if (String::equalNoCase (opt, "checkExtraAll"))
  {
    //
    //  Check indications received by Simple Display Consumer
    //  The extra property added to the indication instance by the
    //  indication provider should not appear in the indication
    //  All properties should be included in the indication instance,
    //  since the filter query specifies SELECT *
    //
    _check (opt,
            String ("SendTestIndicationExtraProperty"), true, false, qlang);
  }
  else if (String::equalNoCase (opt, "checkNormalWhere"))
  {
    //
    //  Check indications received by Simple Display Consumer
    //  An indication should be received because the generated indication
    //  satisfies the WHERE clause condition
    //  Only the properties included in the SELECT list of the filter
    //  query should be included in the indication instance
    //
    _check (opt, String ("SendTestIndicationNormal"),
            false, false, qlang);
  }
  else if (String::equalNoCase (opt, "checkMissingWhere"))
  {
    //
    //  Check indications received by Simple Display Consumer
    //  No indication should be received because the missing property is a
    //  property required by the WHERE clause
    //
    _check (opt,
            String ("SendTestIndicationMissingProperty"), false, true, qlang);
  }
  else if (String::equalNoCase (opt, "checkNormalWhereNotSatisfied"))
  {
    //
    //  Check indications received by Simple Display Consumer
    //  No indication should be received because the generated instance does
    //  not satisfy the WHERE clause condition
    //
    _check (opt,
            String ("SendTestIndicationNormal"), true, true, qlang);
   }
  else if (String::equalNoCase (opt, "delete1"))
  {
    _delete1 (client);
  }
  else if (String::equalNoCase (opt, "delete2"))
  {
    _delete2 (client);
  }
  else if (String::equalNoCase (opt, "delete3"))
  {
    _delete3 (client);
  }
  else if (String::equalNoCase (opt, "delete4"))
  {
    _delete4 (client);
  }
  else if (String::equalNoCase (opt, "cleanup"))
  {
    _cleanup (client, qlang);
  }
  else
  {
    PEGASUS_STD (cerr) << "Invalid option: " << opt
                       << PEGASUS_STD (endl);
    _usage();
    return -1;
  }

  return 0;
}

int main (int argc, char** argv)
{
    CIMClient client;
    try
    {
        client.connectLocal();
    }
    catch (Exception & e)
    {
        PEGASUS_STD (cerr) << e.getMessage() << PEGASUS_STD (endl);
        return -1;
    }

    if (argc != 3)
    {
        _usage();
        return 1;
    }

    const char* opt = argv[1];
    const char* optLang = argv[2];
    String qlang(optLang);

#ifndef PEGASUS_ENABLE_CQL
    if (qlang == "DMTF:CQL")
    {
        PEGASUS_STD(cout) << "+++++ cql test disabled" << PEGASUS_STD(endl);
        return 0;
    }
#endif

    return _test(client, opt, qlang);
}
