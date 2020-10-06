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

///////////////////////////////////////////////////////////////////////////
// Dispatcher.cpp: implementation of the CDispatcher class.
//
//////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "Dispatcher.h"

#include <Pegasus/Common/CIMInstance.h>
#include <Pegasus/Common/CIMIndication.h>
#include <Pegasus/Common/SSLContext.h>
#include <Pegasus/Common/FileSystem.h>
#include <Pegasus/ExportClient/CIMExportClient.h>
#include <Pegasus/Common/Config.h>
#include <Pegasus/Common/Tracer.h>
#include <Pegasus/Common/ContentLanguageList.h>

#include "ConsumerTracer.h"

PEGASUS_NAMESPACE_BEGIN

/////////////////////////////////////////////////////////////////////////////
// verifyCertificate
//
// If server certificate was found in CA trust store and validated, then
// return 'true' to accept the certificate, otherwise return 'false'.
//
/////////////////////////////////////////////////////////////////////////////
static Boolean verifyCertificate(SSLCertificateInfo &certInfo)
{
    return true;
    if (certInfo.getResponseCode() == 1)
    {
        return true;
    }
    else
    {
        return false;
    }
}

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
CDispatcher::CDispatcher()
{

}

CDispatcher::~CDispatcher()
{

}

/////////////////////////////////////////////////////////////////////////////
// _getRegInfo()
//
/////////////////////////////////////////////////////////////////////////////
bool CDispatcher::_getPegasusHome(char *lpchRetValue)
{
    HKEY hKey;
    DWORD dw = _MAX_PATH;
    char subKey[_MAX_PATH] = {0};
  
    CNS_METHOD_ENTER(TRC_WMI_MAPPER_CONSUMER,
        "CDispatcher::_getPegasusHome");

    sprintf(subKey, 
        TEXT("SYSTEM\\CurrentControlSet\\"
             "Control\\Session Manager\\Environment"));

    if ((RegOpenKeyEx(HKEY_LOCAL_MACHINE,
        subKey, 
        0,
        KEY_READ, 
        &hKey)) != ERROR_SUCCESS)
    {

        CNS_TRACE((Pegasus::TRC_WMI_MAPPER_CONSUMER, 
            Pegasus::Tracer::LEVEL4,
            "CDispatcher::_getPegasusHome, \
            error in RegOpenKeyEx"));  

        CNS_METHOD_EXIT();
        return false;
    }

    if ((RegQueryValueEx(hKey, 
        TEXT("PEGASUS_HOME"), 
        NULL, 
        NULL, 
        (LPBYTE)lpchRetValue,
        &dw)) != ERROR_SUCCESS)
    {
        RegCloseKey(hKey);

        CNS_TRACE((Pegasus::TRC_WMI_MAPPER_CONSUMER, 
            Pegasus::Tracer::LEVEL4,
            "CDispatcher::_getPegasusHome, \
            error in RegQueryValueEx")); 

        CNS_METHOD_EXIT();
        return false;
    }

    RegCloseKey(hKey);

    CNS_METHOD_EXIT();
    return true;
}


/////////////////////////////////////////////////////////////////////////////
// String _getMalformedExceptionMsg()
//
/////////////////////////////////////////////////////////////////////////////
String CDispatcher::_getMalformedExceptionMsg()
{
    MessageLoaderParms param(
        "Handler.CIMxmlIndicationHandler.CIMxmlIndicationHandler.ERROR", 
        "CIMxmlIndicationHandler Error: ");

    MessageLoaderParms param1(
        "Handler.CIMxmlIndicationHandler.CIMxmlIndicationHandler."
        "MALFORMED_HANDLER_INSTANCE", 
        "Malformed handler instance.");

    return (String(MessageLoader::getMessage(param) + 
        MessageLoader::getMessage(param1)));
}

/////////////////////////////////////////////////////////////////////////////
//dispatchIndication
//
// Connect and send indication over HTTP. The property "destination" of 
// indicationHandlerInstance indicates the host and port that will receive the 
// indicatioInstance. The method verfies if the host will be acessed by http or
// https using "destination" and use SSL if necessary.
//
/////////////////////////////////////////////////////////////////////////////
void CDispatcher::dispatchIndication(
    CIMInstance& indicationHandlerInstance, 
    CIMInstance& indicationInstance)
{
    CNS_METHOD_ENTER(TRC_WMI_MAPPER_CONSUMER,
        "CDispatcher::_dispatchIndication");
    //get destination for the indication
    Uint32 pos = indicationHandlerInstance.findProperty(CIMName("Destination"));
    if (pos == PEG_NOT_FOUND)
    {
        String msg = _getMalformedExceptionMsg();

        CNS_METHOD_EXIT();
        throw PEGASUS_CIM_EXCEPTION(CIM_ERR_FAILED, msg);
    }

    CIMProperty prop = indicationHandlerInstance.getProperty(pos);

    String dest;
    try
    {
        prop.getValue().get(dest);
    }
    catch (TypeMismatchException& e)
    {
        MessageLoaderParms param(
            "Handler.CIMxmlIndicationHandler.CIMxmlIndicationHandler.ERROR", 
            "CIMxmlIndicationHandler Error: ");

        String msg = String(MessageLoader::getMessage(param) + e.getMessage());

        CNS_METHOD_EXIT();
        throw PEGASUS_CIM_EXCEPTION(CIM_ERR_FAILED, msg);
    }
    
    try
    {
        const char* pegasusHome;
        char tmpHome[512];

        if (GetEnvironmentVariable("PEGASUS_HOME", tmpHome, 512)) {            
            pegasusHome = tmpHome;
            CNS_TRACE((Pegasus::TRC_WMI_MAPPER_CONSUMER, 
                Pegasus::Tracer::LEVEL4,
                "CDispatcher::_dispatchIndication, PEGASUS_HOME: %s", 
                pegasusHome)); 
        }
        else if (_getPegasusHome(tmpHome))
        {
            pegasusHome = tmpHome;
            CNS_TRACE((Pegasus::TRC_WMI_MAPPER_CONSUMER, 
                Pegasus::Tracer::LEVEL4,
                "CDispatcher::_dispatchIndication, PEGASUS_HOME: %s",
                pegasusHome)); 
        }
        else 
        {
            CNS_TRACE((Pegasus::TRC_WMI_MAPPER_CONSUMER, 
                Pegasus::Tracer::LEVEL4,
                "CDispatcher::_dispatchIndication, PEGASUS_HOME env variable "
                "not found"));
               
            pegasusHome = "";
        }
        
        String trustPath = FileSystem::getAbsolutePath(
            pegasusHome, 
            PEGASUS_SSLCLIENT_CERTIFICATEFILE);
   
        String certPath = FileSystem::getAbsolutePath(
            pegasusHome,
            "server.pem");    
   
        String keyPath = FileSystem::getAbsolutePath(
            pegasusHome, "file.pem");
   
        String randFile = String::EMPTY;

#ifdef PEGASUS_SSL_RANDOMFILE
        randFile = FileSystem::getAbsolutePath(
            pegasusHome,
            PEGASUS_SSLCLIENT_RANDOMFILE);
#endif

        SSLContext sslContext(
            trustPath,
            certPath,
            keyPath,
            verifyCertificate,
            randFile);

#ifdef PEGASUS_USE_23HTTPMONITOR_CLIENT
        Monitor monitor;
        HTTPConnector httpConnector( &monitor );
#else
        monitor_2 monitor;
        HTTPConnector2 httpConnector( &monitor );
#endif

        CIMExportClient exportclient( &monitor, &httpConnector);
        Uint32 colon = dest.find (":");
        Uint32 portNumber = 0;
        Boolean useHttps = false;
        String destStr = dest;
        String hostStr;
        Uint32 openBracket = PEG_NOT_FOUND;
        Uint32 closeBracket = PEG_NOT_FOUND;
        

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
                String msg = _getMalformedExceptionMsg();

                CNS_METHOD_EXIT();
                throw PEGASUS_CIM_EXCEPTION(CIM_ERR_NOT_SUPPORTED, msg + dest); 
            }
        }
        else
        {
            String msg = _getMalformedExceptionMsg();

            CNS_METHOD_EXIT();
            throw PEGASUS_CIM_EXCEPTION(CIM_ERR_NOT_SUPPORTED, msg + dest); 
        }

        String doubleSlash = dest.subString(colon + 1, 2); 

        if (String::equalNoCase(doubleSlash, "//"))
        {
            destStr = dest.subString(colon + 3, PEG_NOT_FOUND);
        }
        else
        {
            String msg = _getMalformedExceptionMsg();

            CNS_METHOD_EXIT();
            throw PEGASUS_CIM_EXCEPTION(CIM_ERR_NOT_SUPPORTED, msg + dest); 
        }
        
        char dummy[64];
        dummy[0] = 0;
        openBracket = destStr.find ("[");
        
        if (openBracket != PEG_NOT_FOUND)   // IPv6 Address
        {
            closeBracket = destStr.find ("]");            
            hostStr = destStr.subString(openBracket + 1, closeBracket - 1);
            destStr = destStr.subString(closeBracket + 1, PEG_NOT_FOUND);
            colon = destStr.find (":");
            
            destStr = destStr.subString(colon + 1, PEG_NOT_FOUND);
        }
        else // IPv4 / hostname Address
        {
            colon = destStr.find (":");
            hostStr = destStr.subString(0, colon);
            destStr = destStr.subString(colon + 1, PEG_NOT_FOUND);
        }
        
        if (colon != PEG_NOT_FOUND)
        {
            Uint32 slash = destStr.find ("/");
            String portStr;

            if (slash != PEG_NOT_FOUND)
            {
                portStr = destStr.subString (0, slash);
            }
            else
            {
                portStr = destStr.subString (0, PEG_NOT_FOUND);
            }

            sscanf (portStr.getCString (), "%u%s", &portNumber, dummy);         
        }

        //
        // There is no port number in the destination string,
        // get port number from system
        //
        else
        {
            Uint32 slash = destStr.find ("/");
            if (slash != PEG_NOT_FOUND)
            { 
                hostStr = destStr.subString (0, slash);
            }
            else
            {
                hostStr = destStr.subString (0, PEG_NOT_FOUND);
            }
          
            if (useHttps)
            {
                portNumber = System::lookupPort(
                    WBEM_HTTPS_SERVICE_NAME,
                    WBEM_DEFAULT_HTTPS_PORT); 
            }
            else
            {
                portNumber = System::lookupPort(
                    WBEM_HTTP_SERVICE_NAME,
                    WBEM_DEFAULT_HTTP_PORT);
            }
        }    

        char hostName[64]; 
        char dummy2[64];
        dummy2[0] = 0;

        sscanf (hostStr.getCString (), "%s%s", hostName, dummy2);  

        if (dummy[0] != 0 || dummy2[0] != 0)
        {
            String msg = _getMalformedExceptionMsg();

            CNS_METHOD_EXIT();
            throw PEGASUS_CIM_EXCEPTION(CIM_ERR_NOT_SUPPORTED, msg + dest); 
        }

        CNS_TRACE((
            Pegasus::TRC_WMI_MAPPER_CONSUMER, 
            Pegasus::Tracer::LEVEL4,
            "CDispatcher::_dispatchIndication, hostname:%s  portnumer:%u", 
             hostName, portNumber));

        if (useHttps)
        {
#ifdef PEGASUS_HAS_SSL
            exportclient.connect (hostName, portNumber, sslContext);
#else

            MessageLoaderParms param(
                "Handler.CIMxmlIndicationHandler."
                "CIMxmlIndicationHandler.ERROR", 
                "CIMxmlIndicationHandler Error: ");
            MessageLoaderParms param1(
                "Handler.CIMxmlIndicationHandler.CIMxmlIndicationHandler."
                "CANNOT_DO_HTTPS_CONNECTION", 
                "Cannot do https connection.");

            PEG_TRACE_STRING(
                TRC_WMI_MAPPER_CONSUMER,
                Tracer::LEVEL3,
                MessageLoader::getMessage(param) +
                MessageLoader::getMessage(param1));

            String msg = String(
                MessageLoader::getMessage(param) + 
                MessageLoader::getMessage(param1));

            CNS_METHOD_EXIT();
            throw PEGASUS_CIM_EXCEPTION(CIM_ERR_FAILED, msg);
#endif
        }
        else
        {
            exportclient.connect (hostName, portNumber);
        }


        // check destStr, if no path is specified, use "/" for the URI
        Uint32 slash = destStr.find ("/");
        ContentLanguageList contentLanguages = ContentLanguageList();
        if (slash != PEG_NOT_FOUND)
        {
            exportclient.exportIndication(
            destStr.subString(slash), indicationInstance,
            contentLanguages);
        }
        else
        {
            exportclient.exportIndication(
                "/", indicationInstance, contentLanguages);
        }
    }
    catch(Exception& e)
    {
        //ATTN: Catch specific exceptions and log the error message 
        // as Indication delivery failed.

        String msg = String(e.getMessage());
        
        CNS_TRACE((
            Pegasus::TRC_WMI_MAPPER_CONSUMER, 
            Pegasus::Tracer::LEVEL4,
            "CDispatcher::_dispatchIndication, Dispatch failed: %s", msg));

        CNS_METHOD_EXIT();
        throw PEGASUS_CIM_EXCEPTION(CIM_ERR_FAILED, msg);
    }

    CNS_METHOD_EXIT();
}

PEGASUS_NAMESPACE_END
