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
#include <Pegasus/Common/XmlWriter.h>
#include <Pegasus/Common/TimeValue.h>
#include <Pegasus/Common/MessageLoader.h>

#include "CIMExportClient.h"


PEGASUS_USING_STD;

PEGASUS_NAMESPACE_BEGIN

CIMExportClient::CIMExportClient(
    Monitor* monitor,
    HTTPConnector* httpConnector,
    Uint32 timeoutMilliseconds)
    :
    ExportClient(PEGASUS_QUEUENAME_EXPORTCLIENT,
        httpConnector,
        timeoutMilliseconds,
        monitor)
{
    PEG_METHOD_ENTER (TRC_EXPORT_CLIENT, "CIMExportClient::CIMExportClient()");
    PEG_METHOD_EXIT();
}

CIMExportClient::~CIMExportClient()
{
    PEG_METHOD_ENTER (TRC_EXPORT_CLIENT, "CIMExportClient::~CIMExportClient()");

    disconnect();

    PEG_METHOD_EXIT();
}

void CIMExportClient::exportIndication(
    const String& url,
    const CIMInstance& instanceName,
    const ContentLanguageList& contentLanguages)
{
    PEG_METHOD_ENTER (TRC_EXPORT_CLIENT, "CIMExportClient::exportIndication()");

    try
    {
        // encode request
        CIMRequestMessage* request = new CIMExportIndicationRequestMessage(
            String::EMPTY,
            url,
            instanceName,
            QueueIdStack(),
            String::EMPTY,
            String::EMPTY);

        request->operationContext.set
            (ContentLanguageListContainer(contentLanguages));

        PEG_TRACE ((TRC_INDICATION_GENERATION, Tracer::LEVEL4,
            "Exporting %s Indication for destination %s:%d%s",
            (const char*)(instanceName.getClassName().getString().
            getCString()),
            (const char*)(_connectHost.getCString()), _connectPortNumber,
            (const char*)(url.getCString())));

        Message* message = _doRequest(request,
            CIM_EXPORT_INDICATION_RESPONSE_MESSAGE);

        PEG_TRACE ((TRC_INDICATION_GENERATION, Tracer::LEVEL4,
            "%s Indication for destination %s:%d%s exported successfully",
            (const char*)(instanceName.getClassName().getString().
            getCString()),
            (const char*)(_connectHost.getCString()), _connectPortNumber,
            (const char*)(url.getCString())));

        CIMExportIndicationResponseMessage* response =
            (CIMExportIndicationResponseMessage*)message;

        AutoPtr<CIMExportIndicationResponseMessage> ap(response);
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
        PEG_TRACE_CSTRING (TRC_DISCARDED_DATA, Tracer::LEVEL1,
            "Failed to export indication");
        throw;
    }
    PEG_METHOD_EXIT();
}

Message* CIMExportClient::_doRequest(
    CIMRequestMessage* pRequest,
    MessageType expectedResponseMessageType)
{
    PEG_METHOD_ENTER (TRC_EXPORT_CLIENT, "CIMExportClient::_doRequest()");

    AutoPtr<CIMRequestMessage> request(pRequest);

    if (!_connected && !_doReconnect)
    {
        PEG_METHOD_EXIT();
        throw NotConnectedException();
    }
    
    if (_connected && _httpConnection->needsReconnect())
    {
        _disconnect(true);
        _doReconnect = true;
    }

    if (_doReconnect)
    {
        try
        {
            _connect();
            _doReconnect = false;
        }
        catch (const Exception& e)
        {
            PEG_TRACE((TRC_EXPORT_CLIENT, Tracer::LEVEL1,
                "Failed to connect to indication listener: %s",
                (const char*)e.getMessage().getCString()));
            PEG_METHOD_EXIT();
            throw;
        }
        catch (...)
        {
            PEG_TRACE_CSTRING(TRC_EXPORT_CLIENT, Tracer::LEVEL1,
                "Failed to connect to indication listener.");
            PEG_METHOD_EXIT();
            throw;
        }
    }


    String messageId = XmlWriter::getNextMessageId();
    const_cast<String &>(request->messageId) = messageId;

    _authenticator.setRequestMessage(0);

    // ATTN-RK-P2-20020416: We should probably clear out the queue first.
    PEGASUS_ASSERT(getCount() == 0);  // Shouldn't be any messages in our queue

    //
    //  Set HTTP method in request to POST
    //
    request->setHttpMethod(HTTP_METHOD__POST);

    _cimRequestEncoder->enqueue(request.release());

    Uint64 startMilliseconds = TimeValue::getCurrentTime().toMilliseconds();
    Uint64 nowMilliseconds = startMilliseconds;
    Uint64 stopMilliseconds = nowMilliseconds + _timeoutMilliseconds;

    while (nowMilliseconds < stopMilliseconds)
    {
        //
        // Wait until the timeout expires or an event occurs:
        //
        _monitor->run(Uint32(stopMilliseconds - nowMilliseconds));

        //
        // Check to see if incoming queue has a message
        //

        AutoPtr<Message> response(dequeue());

        if (response.get() != 0)
        {
            // Shouldn't be any more messages in our queue
            PEGASUS_ASSERT(getCount() == 0);

            //
            // Close the connection if response contained a "Connection: Close"
            // header (e.g. at authentication challenge)
            //
            if (response->getCloseConnect() == true)
            {
                _disconnect(true);
                _doReconnect = true;
                response->setCloseConnect(false);
            }

            //
            //  Future:  If M-POST is used and HTTP response is 501 Not
            //  Implemented or 510 Not Extended, retry with POST method
            //

            if (response->getType() == CLIENT_EXCEPTION_MESSAGE)
            {
                Exception* clientException =
                    ((ClientExceptionMessage*)response.get())->clientException;

                PEG_TRACE_CSTRING(TRC_EXPORT_CLIENT, Tracer::LEVEL2,
                    "Client Exception Message received.");

                AutoPtr<Exception> d(clientException);

                //
                // Determine and throw the specific class of client exception
                //

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

                CIMClientXmlException* xmlException =
                    dynamic_cast<CIMClientXmlException*>(clientException);
                if (xmlException)
                {
                    PEG_METHOD_EXIT();
                    throw *xmlException;
                }

                CIMClientResponseException* responseException =
                    dynamic_cast<CIMClientResponseException*>(clientException);
                if (responseException)
                {
                    PEG_METHOD_EXIT();
                    throw *responseException;
                }

                PEG_METHOD_EXIT();
                throw *clientException;
            }
            else if (response->getType() == expectedResponseMessageType)
            {
                PEG_TRACE_CSTRING(TRC_EXPORT_CLIENT, Tracer::LEVEL4,
                    "Received expected indication response message.");
                CIMResponseMessage* cimResponse =
                    (CIMResponseMessage*)response.get();
                if (cimResponse->messageId != messageId)
                {
                    MessageLoaderParms mlParms(
                        "ExportClient.CIMExportClient.MISMATCHED_RESPONSE_ID",
                        "Mismatched response message ID:  Got \"$0\", "
                            "expected \"$1\".",
                        cimResponse->messageId, messageId);
                    String mlString(MessageLoader::getMessage(mlParms));

                    CIMClientResponseException responseException(mlString);

                    PEG_METHOD_EXIT();
                    throw responseException;
                }
                if (cimResponse->cimException.getCode() != CIM_ERR_SUCCESS)
                {
                    PEG_TRACE_CSTRING(TRC_EXPORT_CLIENT, Tracer::LEVEL1,
                        "Received indication failure message.");
                    CIMException cimException(
                        cimResponse->cimException.getCode(),
                        cimResponse->cimException.getMessage());
                    PEG_METHOD_EXIT();
                    throw cimException;
                }
                PEG_METHOD_EXIT();
                return response.release();
            }
            else if (dynamic_cast<CIMRequestMessage*>(response.get()) != 0)
            {
                //
                // Respond to an authentication challenge.
                // Reconnect if the connection was closed.
                //
                if (_doReconnect)
                {
                    _connect();
                }

                _cimRequestEncoder->enqueue(response.release());

                nowMilliseconds = TimeValue::getCurrentTime().toMilliseconds();
                stopMilliseconds = nowMilliseconds + _timeoutMilliseconds;
                continue;
            }
            else
            {
                MessageLoaderParms mlParms(
                    "ExportClient.CIMExportClient.MISMATCHED_RESPONSE",
                    "Mismatched response message type.");
                String mlString(MessageLoader::getMessage(mlParms));

                CIMClientResponseException responseException(mlString);

                PEG_TRACE_CSTRING(TRC_EXPORT_CLIENT, Tracer::LEVEL1,
                    (const char*)mlString.getCString());

                PEG_METHOD_EXIT();
                throw responseException;
            }
        }

        nowMilliseconds = TimeValue::getCurrentTime().toMilliseconds();
    }

    //
    // Reconnect to reset the connection (disregard late response)
    //

    PEG_TRACE_CSTRING(TRC_EXPORT_CLIENT, Tracer::LEVEL2,
        "Connection to the listener timed out.");

    _disconnect(true);
    _doReconnect = true;

    //
    // Throw timed out exception:
    //
    PEG_METHOD_EXIT();
    throw ConnectionTimeoutException();
}

PEGASUS_NAMESPACE_END
