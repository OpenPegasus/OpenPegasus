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

#include <Pegasus/Common/Constants.h>
#include <Pegasus/WsmServer/WsmWriter.h>
#include <Pegasus/WsmServer/WsmRequest.h>
#include <Pegasus/WsmServer/CimToWsmResponseMapper.h>
#include <Pegasus/ExportClient/WSMANExportClient.h>
#include <Pegasus/Common/TimeValue.h>

PEGASUS_USING_STD;

PEGASUS_NAMESPACE_BEGIN

WSMANExportClient::WSMANExportClient(
    HTTPConnector* httpConnector,
    Monitor* monitor,
    Uint32 timeoutMilliseconds)
    :
    ExportClient(PEGASUS_QUEUENAME_WSMANEXPORTCLIENT,
        httpConnector,
        timeoutMilliseconds,
        monitor)
{
    PEG_METHOD_ENTER (TRC_EXPORT_CLIENT, 
        "WSMANExportClient::WSMANExportClient()");
    PEG_METHOD_EXIT();
}

WSMANExportClient::~WSMANExportClient()
{
    PEG_METHOD_ENTER (TRC_EXPORT_CLIENT, 
        "WSMANExportClient::~WSMANExportClient()");
    disconnect();
    PEG_METHOD_EXIT();
}

void WSMANExportClient::exportIndication(
    const String& url,
    const CIMInstance& instanceName,
    const ContentLanguageList& contentLanguages,
    const String& toPath)    
{
    PEG_METHOD_ENTER (TRC_EXPORT_CLIENT, 
        "WSMANExportClient::exportIndication()");
    try
    {
        CimToWsmResponseMapper wsmMapper;
        WsmInstance wsmInstance;
        wsmMapper.convertCimToWsmInstance(url, instanceName, 
            wsmInstance, PEGASUS_INSTANCE_NS);
        WsmRequest * request = new WsExportIndicationRequest(
            WsmUtils::getMessageId(),
            url,
            toPath,
            wsmInstance);

        request->contentLanguages = contentLanguages;
        if(_wsmanResponseDecoder != NULL)
        {
            _wsmanResponseDecoder->setWsmRequest(request);
            _wsmanResponseDecoder->setContentLanguages(contentLanguages);
        }

        PEG_TRACE ((TRC_EXPORT_CLIENT, Tracer::LEVEL4,
            "Exporting %s Indication for destination %s:%d%s",
            (const char*)(instanceName.getClassName().getString().
            getCString()),
            (const char*)(_connectHost.getCString()), _connectPortNumber,
            (const char*)(url.getCString())));


        Boolean ackReceived = _doRequest(request,WS_EXPORT_INDICATION);
        //ackReceived flag will be true only if the delivery mode is
        // PUSH_WITH_ACK and if we get acknowledgement from the listner. 
        if(ackReceived)
        {
            PEG_TRACE ((TRC_EXPORT_CLIENT, Tracer::LEVEL4,
                "%s Indication for destination %s:%d%s exported successfully"
                "and got acknowledgement from the listner",
                (const char*)(instanceName.getClassName().getString().
                getCString()),
                (const char*)(_connectHost.getCString()), _connectPortNumber,
                (const char*)(url.getCString())));
        }
    }
    catch (const Exception& e)
    {
        PEG_TRACE((TRC_DISCARDED_DATA, Tracer::LEVEL1,
            "Failed to export indication: %s",
            (const char*)e.getMessage().getCString()));
        throw;
    }
    catch (...)
    {
        PEG_TRACE((TRC_DISCARDED_DATA, Tracer::LEVEL1,
            "Failed to export indication for class %s",
            (const char*)(instanceName.getClassName().getString().
            getCString())));
        throw;
    }
    PEG_METHOD_EXIT();
}

Boolean WSMANExportClient::_doRequest(
    WsmRequest * request,
    WsmOperationType expectedResponseMessageType)
    
{
    PEG_METHOD_ENTER (TRC_EXPORT_CLIENT, "WSMANExportClient::_doRequest()");

    AutoPtr<WsmRequest> indicationRequest(request);
    
    if (!_connected )
    {
        PEG_METHOD_EXIT();
        throw NotConnectedException();
    }

    _authenticator.setRequestMessage(0);

    //
    //  Set HTTP method in request to POST
    //
    indicationRequest->httpMethod = HTTP_METHOD__POST;

    //Current WSMAN eventing part supports only PUSH and PUSH_WITH_ACK 
    // delivery mode.So we will deliver the indication if the delivery 
    // mode is one of them. 
    if (( _deliveryMode == Push ) || (_deliveryMode == PushWithAck))
    {
        _wsmanRequestEncoder->setDeliveryMode(_deliveryMode);
        _wsmanRequestEncoder->enqueue(indicationRequest.release());
    }
    else
    {
        PEG_TRACE((TRC_DISCARDED_DATA, Tracer::LEVEL1,
            "Failed to export indication since delivery mode %s"
            " is not supported", (const char*)_deliveryMode));
    }
    Uint64 startupTime = TimeValue::getCurrentTime().toMilliseconds();
    Uint64 currentTime = startupTime;
    Uint64 stopTime = currentTime + _timeoutMilliseconds;
    while (currentTime < stopTime)
    {
        //Wait until the timeout expires or an event occurs:
        _monitor->run(Uint32(stopTime- currentTime));
                        
        // Check to see if incoming queue has a message
        AutoPtr<WsmResponse> response(dynamic_cast<WsmResponse*>(dequeue()));
        if (response.get() != 0)
        {
            //Shouldn't be any more messages in our queue
            PEGASUS_ASSERT(getCount() == 0);
            if(response->getCloseConnect() == true)
            {
                _disconnect(true);
                _doReconnect = true;
                response->setCloseConnect(false);
            }
            if (response->getType() == CLIENT_EXCEPTION_MESSAGE)
            {
                Exception* clientException =
                    ((ClientExceptionMessage*)response.get())->clientException;

                PEG_TRACE_CSTRING(TRC_EXPORT_CLIENT, Tracer::LEVEL2,
                    "Client Exception Message received.");
                AutoPtr<Exception> d(clientException);
                CIMClientMalformedHTTPException* malformedHTTPException =
                    dynamic_cast<CIMClientMalformedHTTPException*>(
                    clientException);
                if (malformedHTTPException)
                {
                    PEG_METHOD_EXIT();
                    throw *malformedHTTPException;
                }

                CIMClientHTTPErrorException* httpErrorException =
                    dynamic_cast<CIMClientHTTPErrorException*>(
                    clientException);
                if (httpErrorException)
                {
                    PEG_METHOD_EXIT();
                    throw *httpErrorException;
                }

                PEG_METHOD_EXIT();
                throw *clientException; 

            }
            else if(response->getOperationType() == expectedResponseMessageType)
            {
                PEG_TRACE_CSTRING(TRC_EXPORT_CLIENT, Tracer::LEVEL4,
                    "Received expected indication response message.");
                PEG_METHOD_EXIT();
                return true;
            }
            else if (response->getOperationType() == WSM_FAULT)
            {
                PEG_TRACE_CSTRING(TRC_EXPORT_CLIENT, Tracer::LEVEL1,
                        "Received indication failure message.");
                PEG_METHOD_EXIT();
                return false;
            }
            else
            {
                MessageLoaderParms mlParms(
                    "ExportClient.WSMANExportClient.MISMATCHED_RESPONSE",
                    "Mismatched response message type.");
                String mlString(MessageLoader::getMessage(mlParms));

                CIMClientResponseException responseException(mlString);

                PEG_TRACE_CSTRING(TRC_EXPORT_CLIENT, Tracer::LEVEL1,
                    (const char*)mlString.getCString());

                PEG_METHOD_EXIT();
                throw responseException;

            } 
 
        }
        currentTime = TimeValue::getCurrentTime().toMilliseconds();
    }
    PEG_METHOD_EXIT();
    return false;
}

void WSMANExportClient ::setDeliveryMode(deliveryMode &deliveryMode)
{
    _deliveryMode = deliveryMode;
}
    
PEGASUS_NAMESPACE_END
