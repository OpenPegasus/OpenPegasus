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
#include <Pegasus/Common/TimeValue.h>
#include <Pegasus/Common/Threads.h>
#include <Pegasus/Common/PegasusVersion.h>
#include <Pegasus/Common/MessageLoader.h>
#include <Pegasus/Common/HostAddress.h>
#include <Pegasus/ExportClient/ExportClient.h>
#include <Pegasus/Common/Constants.h>

PEGASUS_USING_STD;

PEGASUS_NAMESPACE_BEGIN


ExportClient::ExportClient(
    const char* queueId,
    HTTPConnector* httpConnector,
    Uint32 timeoutMilliseconds,
    Monitor* monitor)
    :MessageQueue(queueId),
    _httpConnector(httpConnector),
    _monitor(monitor),
    _timeoutMilliseconds(timeoutMilliseconds),
    _connected(false),
    _httpConnection(0)
{
    PEG_METHOD_ENTER (TRC_EXPORT_CLIENT,
       "ExportClient::ExportClient()");
    
     if((strcmp(queueId,PEGASUS_QUEUENAME_WSMANEXPORTCLIENT))== 0)
     {
         isWSMANExportIndication = true;
#ifdef PEGASUS_ENABLE_PROTOCOL_WSMAN
         _wsmanRequestEncoder=0;
         _wsmanResponseDecoder=0;
#endif
     }
     else if ((strcmp(queueId,PEGASUS_QUEUENAME_EXPORTCLIENT)) == 0)
     {
         isWSMANExportIndication = false;
         _cimRequestEncoder=0;
         _cimResponseDecoder=0;
         _doReconnect=false;
     }
 
     PEG_METHOD_EXIT();
}

ExportClient::~ExportClient()
{
    PEG_METHOD_ENTER (TRC_EXPORT_CLIENT,
        "ExportClient::~ExportClient()");
    disconnect();
    PEG_METHOD_EXIT();
}

void ExportClient::_connect()
{
    PEG_METHOD_ENTER (TRC_EXPORT_CLIENT,"ExportClient::_connect()");
 
    if(!isWSMANExportIndication)
    {
        // Create response decoder:

        _cimResponseDecoder = new CIMExportResponseDecoder(
            this,
            _cimRequestEncoder,
            &_authenticator);
        
        // Attempt to establish a connection:

        try
        { 
            _httpConnection = _httpConnector->connect(_connectHost,
                _connectPortNumber,
                _connectSSLContext.get(),
                _timeoutMilliseconds,
                _cimResponseDecoder);
        }
        catch (...)
        {
            // Some possible exceptions are CannotCreateSocketException,
            // CannotConnectException, and InvalidLocatorException
            delete _cimResponseDecoder;
            PEG_METHOD_EXIT();
            throw;
        }

    }
    else
    {
#ifdef PEGASUS_ENABLE_PROTOCOL_WSMAN
        // Create response decoder:
        _wsmanResponseDecoder =  new WSMANExportResponseDecoder(
            this,
            _wsmanRequestEncoder,
            &_authenticator);
        
        // Attempt to establish a connection:
        try
        {
           
            _httpConnection = _httpConnector->connect(_connectHost,
                _connectPortNumber,
                _connectSSLContext.get(),
                _timeoutMilliseconds,
                _wsmanResponseDecoder);
        }
        catch(...)
        {
            // Some possible exceptions are CannotCreateSocketException,
            // CannotConnectException, and InvalidLocatorException
            delete _wsmanResponseDecoder;
            PEG_METHOD_EXIT();
            throw;

        }
#endif
    }

    String connectHost = _connectHost;

#ifdef PEGASUS_ENABLE_IPV6
    HostAddress hst;
    hst.setHostAddress(connectHost);
    if (hst.getAddressType() == HostAddress::AT_IPV6)
    {
        connectHost = "[" + connectHost + "]";
    }
#endif

    char portStr[32];
    if (connectHost.size())
    {
        sprintf(portStr, ":%u", _connectPortNumber);
        connectHost.append(portStr);
    }

#ifdef PEGASUS_ENABLE_PROTOCOL_WSMAN
    //Create requestEncoder to encode the exportIndication request.
    if(isWSMANExportIndication)
    {
        _wsmanRequestEncoder= new WSMANExportRequestEncoder(
            _httpConnection,
            _connectHost,
            portStr,
            &_authenticator);
        _wsmanResponseDecoder->setEncoderQueue(_wsmanRequestEncoder); 
    }
    else
#endif
    {
        _cimRequestEncoder = new CIMExportRequestEncoder(
            _httpConnection,
            connectHost,
            &_authenticator);
        _cimResponseDecoder->setEncoderQueue(_cimRequestEncoder);
        _doReconnect = false;
    } 

    _connected = true;

    _httpConnection->setSocketWriteTimeout(_timeoutMilliseconds/1000+1);

    PEG_METHOD_EXIT();
}

void ExportClient::_disconnect( bool keepChallengeStatus)
{
    PEG_METHOD_ENTER(TRC_EXPORT_CLIENT, "ExportClient::_disconnect()");

    if (_connected)
    {
        // destroy response decoder
        if(!isWSMANExportIndication)
        {
            delete _cimResponseDecoder;
            _cimResponseDecoder = 0;
   
            delete _cimRequestEncoder;
            _cimRequestEncoder = 0;  
        }
        else
        {
#ifdef PEGASUS_ENABLE_PROTOCOL_WSMAN
            delete _wsmanRequestEncoder;
            _wsmanRequestEncoder = 0;

            delete _wsmanResponseDecoder;
            _wsmanResponseDecoder = 0;
#endif
        }
        //
        // Close the connection
        //
        if (_httpConnector)
        {
            _httpConnector->disconnect(_httpConnection);
            _httpConnection = 0;
        }

        _connected = false;
    }
    
    // Reconnect no longer applies for CIMExportIndication request.
    if(!isWSMANExportIndication)
    {
        _doReconnect=false;
    }

    // Let go of the cached request message if we have one
    _authenticator.setRequestMessage(0);

    if( keepChallengeStatus == false)
    {
    // Reset the challenge status
    _authenticator.resetChallengeStatus();
    }

    PEG_METHOD_EXIT();
}

void ExportClient::connect(
    const String& host,
    const Uint32 portNumber)
{
    PEG_METHOD_ENTER (TRC_EXPORT_CLIENT, "ExportClient::connect()");

    // If already connected, bail out!
    if (_connected)
    {
        PEG_METHOD_EXIT();
        throw AlreadyConnectedException();
    }

    //
    // If the host is empty, set hostName to "localhost"
    //
    String hostName = host;
    if (host.size()  == 0 )
    {
        hostName = "localhost";
    }

    //
    // Set authentication information
    //
    _authenticator.clear();

    _connectSSLContext.reset(0);
    _connectHost = hostName;
    _connectPortNumber = portNumber;

    _connect();
    PEG_METHOD_EXIT();
}

 void ExportClient::connect(
    const String& host,
    const Uint32 portNumber,
    const SSLContext& sslContext)
{
    PEG_METHOD_ENTER (TRC_EXPORT_CLIENT, "ExportClient::connect()");

    // If already connected, bail out!

    if (_connected)
    {
       PEG_METHOD_EXIT();
       throw AlreadyConnectedException();
    }

    //
    // If the host is empty, set hostName to "localhost"
    //
    String hostName = host;
    if (host.size() == 0 )
    {
        hostName = "localhost";
    }

    _authenticator.clear();

    _connectSSLContext.reset(new SSLContext(sslContext));
    _connectHost = hostName;
    _connectPortNumber = portNumber;

    try
    {
        _connect();
    }
    catch (...)
    {
        _connectSSLContext.reset();
        PEG_METHOD_EXIT();
        throw;
    }
    PEG_METHOD_EXIT();
}

void ExportClient::disconnect()
{
    PEG_METHOD_ENTER (TRC_EXPORT_CLIENT, "ExportClient::disconnect()");
    _disconnect();
    _authenticator.clear();
    _connectSSLContext.reset();
    PEG_METHOD_EXIT();
}
PEGASUS_NAMESPACE_END
