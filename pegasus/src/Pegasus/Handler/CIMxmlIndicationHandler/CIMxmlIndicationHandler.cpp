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

#include <Pegasus/ExportClient/CIMExportClient.h>
#include <Pegasus/Handler/CIMHandler.h>
#include <Pegasus/Repository/CIMRepository.h>
#include <Pegasus/Config/ConfigManager.h>
#include <Pegasus/Common/Config.h>
#include <Pegasus/Common/PegasusVersion.h>
#include <Pegasus/Common/Constants.h>
#include <Pegasus/Common/SSLContext.h>
#include <Pegasus/Common/System.h>
#include <Pegasus/Common/Tracer.h>
#include <Pegasus/Common/HostLocator.h>

PEGASUS_NAMESPACE_BEGIN

PEGASUS_USING_STD;

class CIMXMLExportConnection : public IndicationExportConnection
{
public:
    CIMXMLExportConnection(
        Monitor         *monitor,
        HTTPConnector   *connector,
        CIMExportClient *client,
        String           uri):
            _monitor(monitor),_connector(connector),_client(client),_uri(uri)
    {
    };

    ~CIMXMLExportConnection()
    {
        delete _client;
        delete _connector;
        delete _monitor;
    }

    CIMExportClient* getClient() const
    {
        return _client;
    }

    String getURI() const
    {
        return _uri;
    }
 
private:
    Monitor         *_monitor;
    HTTPConnector   *_connector;
    CIMExportClient *_client;
    String           _uri;
};

static Boolean verifyListenerCertificate(SSLCertificateInfo& certInfo)
{
    // ATTN: Add code to handle listener certificate verification.
    //
    return true;
}


class PEGASUS_HANDLER_LINKAGE CIMxmlIndicationHandler: public CIMHandler
{
public:

    CIMxmlIndicationHandler()
    {
        PEG_METHOD_ENTER(TRC_IND_HANDLER,
            "CIMxmlIndicationHandler::CIMxmlIndicationHandler");
        PEG_METHOD_EXIT();
    }

    virtual ~CIMxmlIndicationHandler()
    {
        PEG_METHOD_ENTER(TRC_IND_HANDLER,
            "CIMxmlIndicationHandler::~CIMxmlIndicationHandler");
        PEG_METHOD_EXIT();
    }

    void initialize(CIMRepository* repository)
    {

    }

    void terminate()
    {

    }

    void handleIndication(
        const OperationContext& context,
        const String nameSpace,
        CIMInstance& indicationInstance,
        CIMInstance& indicationHandlerInstance,
        CIMInstance& indicationSubscriptionInstance,
        ContentLanguageList& contentLanguages)
    {

        handleIndication(
            context,
            nameSpace,
            indicationInstance,
            indicationHandlerInstance,
            indicationSubscriptionInstance,
            contentLanguages,
            0);
    }

    void handleIndication(
    const OperationContext& context,
    const String &nameSpace,
    CIMInstance& indicationInstance,
    CIMInstance& indicationHandlerInstance,
    CIMInstance& indicationSubscriptionInstance,
    ContentLanguageList& contentLanguages,
    IndicationExportConnection **connection)
    {
        PEG_METHOD_ENTER(TRC_IND_HANDLER,
            "CIMxmlIndicationHandler::handleIndication()");

        // If connection already exists, use it for indication delivery.
        // In case of errors (listener closed the connection in between)
        // reconnect (CIMExportClient does this) and try to deliver the
        // indication. If there are any errors, delete the existing
        // connection and report the error.
        if (connection)
        {
            if (*connection)
            {
                CIMXMLExportConnection *conn = 
                    dynamic_cast<CIMXMLExportConnection*> (*connection);
                PEGASUS_ASSERT(conn);
                
                String errorMsg;
                try
                {
                    conn->getClient()->exportIndication(
                        conn->getURI(),
                        indicationInstance,
                        contentLanguages);
                }
                catch(const Exception &e)
                {
                    errorMsg = e.getMessage();
                }
                catch(...)
                {
                    errorMsg = "Unknown error";
                }
                if (errorMsg.size())
                {
                    delete conn;
                    *connection = 0;
                    PEG_TRACE ((TRC_INDICATION_GENERATION, Tracer::LEVEL1,
                        "Failed to deliver indication using the "
                            "existing connection with reconnect : %s ",
                        (const char*)errorMsg.getCString()));

                    PEG_METHOD_EXIT();
                    throw PEGASUS_CIM_EXCEPTION(CIM_ERR_FAILED, errorMsg);
                }
                PEG_METHOD_EXIT();
                return;
            }
            *connection = 0;
        }

        //get destination for the indication
        Uint32 pos = indicationHandlerInstance.findProperty(
                CIMName ("destination"));
        if (pos == PEG_NOT_FOUND)
        {
            MessageLoaderParms param(
                "Handler.CIMxmlIndicationHandler.CIMxmlIndicationHandler."
                    "MALFORMED_HANDLER_INSTANCE",
                "Malformed CIM-XML handler instance, "
                    "\'Destination\' property is not found.");

            PEG_TRACE_CSTRING(TRC_DISCARDED_DATA, Tracer::LEVEL1,
                "Malformed CIM-XML handler instance,"
                "\'Destination\' property is not found.");

            PEG_METHOD_EXIT();
            throw PEGASUS_CIM_EXCEPTION(CIM_ERR_FAILED,
                      MessageLoader::getMessage(param));
        }

        CIMProperty prop = indicationHandlerInstance.getProperty(pos);

        String dest;
        try
        {
            prop.getValue().get(dest);
        }
        catch (TypeMismatchException&)
        {
            MessageLoaderParms param(
                "Handler.CIMxmlIndicationHandler.CIMxmlIndicationHandler."
                    "DESTINATION_TYPE_MISMATCH",
                "Malformed CIM-XML handler instance, "
                    "\'Destination\' property type mismatch.");

            PEG_TRACE_CSTRING(TRC_DISCARDED_DATA, Tracer::LEVEL1,
                "Malformed CIM-XML handler instance, "
                "\'Destination\' property type mismatch.");

            PEG_METHOD_EXIT();
            throw PEGASUS_CIM_EXCEPTION(CIM_ERR_FAILED,
                      MessageLoader::getMessage(param));
        }

        PEG_TRACE ((TRC_INDICATION_GENERATION, Tracer::LEVEL4,
            "CIM_ListenerDestinationCIMXML %s:%s.%s processing %s Indication "
            "for destination %s",
           (const char*)(nameSpace.getCString()),
           (const char*)(indicationHandlerInstance.getClassName().getString().
           getCString()),
           (const char*)(indicationHandlerInstance.getProperty(
           indicationHandlerInstance.findProperty(PEGASUS_PROPERTYNAME_NAME)).
           getValue().toString().getCString()),
           (const char*)(indicationInstance.getClassName().getString().
           getCString()), (const char*)(dest.getCString())));
        try
        {
            static String PROPERTY_NAME__SSLCERT_FILEPATH =
                "sslCertificateFilePath";
            static String PROPERTY_NAME__SSLKEY_FILEPATH  = "sslKeyFilePath";

            //
            // Get the sslCertificateFilePath property from the Config Manager.
            //
            ConfigManager* configManager = ConfigManager::getInstance();

            String certPath;
            certPath = ConfigManager::getHomedPath(
                configManager->getCurrentValue(
                    PROPERTY_NAME__SSLCERT_FILEPATH));

            //
            // Get the sslKeyFilePath property from the Config Manager.
            //
            String keyPath;
            keyPath = ConfigManager::getHomedPath(
                configManager->getCurrentValue(
                    PROPERTY_NAME__SSLKEY_FILEPATH));

            String trustPath;
            String randFile;

#ifdef PEGASUS_SSL_RANDOMFILE
            randFile =
                ConfigManager::getHomedPath(PEGASUS_SSLSERVER_RANDOMFILE);
#endif

            AutoPtr<Monitor> monitor(new Monitor());
            AutoPtr<HTTPConnector> httpConnector(
                new HTTPConnector(monitor.get()));

            AutoPtr<CIMExportClient> exportclient(
                new CIMExportClient(monitor.get(), httpConnector.get()));

            Uint32 colon = dest.find (":");
            Uint32 portNumber = 0;
            Boolean useHttps = false;
            String destStr = dest;
            String hostStr;

            //
            // If the URL has https (https://hostname:port/... or
            // https://hostname/...) then use SSL for Indication delivery.
            // If it has http (http://hostname:port/...
            // or http://hostname/...) then do not use SSL.
            //
            if (colon != PEG_NOT_FOUND)
            {
                String httpStr = dest.subString(0, colon);
                if (String::equalNoCase(httpStr, "https"))
                {
                    useHttps = true;
                }
                else if (String::equalNoCase(httpStr, "http"))
                {
                    useHttps = false;
                }
                else
                {
                    String msg = _getMalformedExceptionMsg(dest);

                    PEG_TRACE((TRC_DISCARDED_DATA,Tracer::LEVEL1,"%s%s",
                        (const char*)msg.getCString(),
                        (const char*)dest.getCString()));

                    PEG_METHOD_EXIT();
                    throw PEGASUS_CIM_EXCEPTION(CIM_ERR_NOT_SUPPORTED,
                        msg);
                }
            }
            else
            {
                String msg = _getMalformedExceptionMsg(dest);

                    PEG_TRACE((TRC_DISCARDED_DATA,Tracer::LEVEL1,"%s%s",
                        (const char*)msg.getCString(),
                        (const char*)dest.getCString()));

                PEG_METHOD_EXIT();
                throw PEGASUS_CIM_EXCEPTION(CIM_ERR_NOT_SUPPORTED, msg);
            }

            String doubleSlash = dest.subString(colon + 1, 2);

            if (String::equalNoCase(doubleSlash, "//"))
            {
                destStr = dest.subString(colon + 3, PEG_NOT_FOUND);
            }
            else
            {
                String msg = _getMalformedExceptionMsg(dest);

                PEG_TRACE((TRC_DISCARDED_DATA, Tracer::LEVEL1,"%s%s",
                    (const char*)msg.getCString(),
                    (const char*)dest.getCString()));

                PEG_METHOD_EXIT();
                throw PEGASUS_CIM_EXCEPTION(CIM_ERR_NOT_SUPPORTED, msg);
            }

            HostLocator addr(destStr.subString(0, destStr.find("/")));
            char hostName[PEGASUS_MAXHOSTNAMELEN];
            if (addr.isValid())
            {
                strcpy(hostName, addr.getHost().getCString());
                if (addr.isPortSpecified())
                {
                    portNumber = addr.getPort();
                }
                else if (useHttps)
                {
                     portNumber = System::lookupPort(WBEM_HTTPS_SERVICE_NAME,
                        WBEM_DEFAULT_HTTPS_PORT);
                }
                else
                {
                    portNumber = System::lookupPort(WBEM_HTTP_SERVICE_NAME,
                        WBEM_DEFAULT_HTTP_PORT);
                }
            }
            else
            {
                String msg = _getMalformedExceptionMsg(dest);

                PEG_TRACE((TRC_DISCARDED_DATA, Tracer::LEVEL1,"%s%s",
                    (const char*)msg.getCString(),
                    (const char*)dest.getCString()));

                PEG_METHOD_EXIT();
                throw PEGASUS_CIM_EXCEPTION(CIM_ERR_NOT_SUPPORTED, msg);
            }

#ifndef PEGASUS_OS_ZOS

            if (useHttps)
            {
#ifdef PEGASUS_HAS_SSL
                PEG_TRACE_CSTRING(TRC_IND_HANDLER, Tracer::LEVEL4,
                    "Build SSL Context...");

                SSLContext sslcontext(trustPath,
                    certPath, keyPath, verifyListenerCertificate, randFile);
                exportclient->connect (hostName, portNumber, sslcontext);
#else
                PEG_TRACE((
                    TRC_DISCARDED_DATA, Tracer::LEVEL1,
                    "CIMxmlIndicationHandler::handleIndication failed to "
                    "deliver indication: "
                    "https not supported "
                    "in Destination %s",
                    (const char*) dest.getCString()));

                MessageLoaderParms param(
                    "Handler.CIMxmlIndicationHandler.CIMxmlIndicationHandler."
                        "CANNOT_DO_HTTPS_CONNECTION",
                    "SSL is not available. "
                        "Cannot support an HTTPS connection.");

                PEG_METHOD_EXIT();
                throw PEGASUS_CIM_EXCEPTION(
                    CIM_ERR_FAILED,
                    MessageLoader::getMessage(param));
#endif
            }
            else
            {
                exportclient->connect (hostName, portNumber);
            }
#else
            // On zOS the ATTLS facility is using the port number(s) defined
            // of the outbound policy to decide if the indication is
            // delivered through a SSL secured socket. This is totally
            // transparent to the CIM Server.
            exportclient->connect (hostName, portNumber);

#endif
            // check destStr, if no path is specified, use "/" for the URI
            Uint32 slash = destStr.find ("/");
            String uri = "/";
            if (slash != PEG_NOT_FOUND)
            {
                uri = destStr.subString(slash);
                exportclient->exportIndication(
                    uri, indicationInstance, contentLanguages);
            }
            else
            {
                exportclient->exportIndication(
                    uri, indicationInstance, contentLanguages);
            }

            // Save the connection if requested for future use.
            if (connection)
            {
                *connection =  new CIMXMLExportConnection(
                    monitor.release(),
                    httpConnector.release(),
                    exportclient.release(),
                    uri);
            }

        }
        catch(Exception& e)
        {
            //ATTN: Catch specific exceptions and log the error message
            // as Indication delivery failed.
            String msg = e.getMessage();

            PEG_TRACE((TRC_DISCARDED_DATA, Tracer::LEVEL1,
                "CIMxmlIndicationHandler::handleIndication failed to deliver "
                "indication due to Exception: %s",
                (const char*)e.getMessage().getCString()));

            PEG_METHOD_EXIT();
            throw PEGASUS_CIM_EXCEPTION(CIM_ERR_FAILED, msg);
        }

        PEG_METHOD_EXIT();
    }

private:
    String _getMalformedExceptionMsg(
        String destinationValue)
    {
        MessageLoaderParms param(
            "Handler.CIMxmlIndicationHandler.CIMxmlIndicationHandler."
                "DESTINATION_NOT_VALID",
            "Malformed CIM-XML handler instance, "
                "\'Destination\' property \"$0\" is not valid.",
                destinationValue);
        return (String(MessageLoader::getMessage(param)));
    }

};

PEGASUS_NAMESPACE_END

PEGASUS_USING_PEGASUS;

// This is the entry point into this dynamic module.

extern "C" PEGASUS_EXPORT CIMHandler* PegasusCreateHandler(
    const String& handlerName)
{
    if (handlerName == "CIMxmlIndicationHandler")
    {
        return new CIMxmlIndicationHandler;
    }

    return 0;
}
