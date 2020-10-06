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
#include <Pegasus/Common/PegasusAssert.h>
#include <Pegasus/Common/StrLit.h>
#include <Pegasus/Client/CIMClientException.h>
#include <Pegasus/ExportClient/HTTPExportResponseDecoder.h>

PEGASUS_USING_PEGASUS;
PEGASUS_USING_STD;

const CIMName MALFORMED_HTTP = CIMName ("CIMClientMalformedHTTPException");
const CIMName HTTP_ERROR = CIMName ("CIMClientHTTPErrorException");
const CIMName XML_EXCEPTION = CIMName ("CIMClientXmlException");
const CIMName RESPONSE_EXCEPTION = CIMName ("CIMClientResponseException");

void testParseHTTPHeaders(
    Buffer httpBuffer,
    Boolean expectedValid,
    Boolean expectedReconnect,
    MessageType expectedType)
{
    ClientExceptionMessage* exceptionMessage;
    Array<HTTPHeader> headers;
    Uint32 contentLength;
    Uint32 statusCode;
    String reasonPhrase;
    Boolean cimReconnect;
    Boolean valid;

    HTTPMessage httpMessage(httpBuffer);

    HTTPExportResponseDecoder::parseHTTPHeaders(&httpMessage, exceptionMessage,
        headers, contentLength, statusCode, reasonPhrase, cimReconnect,
        valid);

    PEGASUS_TEST_ASSERT(valid == expectedValid);
    PEGASUS_TEST_ASSERT(cimReconnect == expectedReconnect);
    PEGASUS_TEST_ASSERT(exceptionMessage->getType() == expectedType);
    PEGASUS_TEST_ASSERT(exceptionMessage);
    CIMClientMalformedHTTPException* malformedHTTPException =
        dynamic_cast<CIMClientMalformedHTTPException*>(
            exceptionMessage->clientException);
    PEGASUS_TEST_ASSERT (malformedHTTPException);
    delete exceptionMessage->clientException;
    delete exceptionMessage;
}

void testValidateHTTPHeaders(
    Buffer httpBuffer,
    Boolean expectedValid,
    Boolean expectedReconnect,
    MessageType expectedType,
    CIMName expectedClass)
{
    ClientExceptionMessage* exceptionMessage;
    Array<HTTPHeader> headers;
    char* content;
    Uint32 contentLength;
    Uint32 statusCode;
    String reasonPhrase;
    Boolean cimReconnect;
    Boolean valid;

    HTTPMessage httpMessage(httpBuffer);

    HTTPExportResponseDecoder::parseHTTPHeaders(&httpMessage, exceptionMessage,
        headers, contentLength, statusCode, reasonPhrase, cimReconnect,
        valid);

    HTTPExportResponseDecoder::validateHTTPHeaders(&httpMessage, headers,
        contentLength, statusCode, cimReconnect, reasonPhrase, content,
        exceptionMessage, valid);

    PEGASUS_TEST_ASSERT(valid == expectedValid);
    PEGASUS_TEST_ASSERT(cimReconnect == expectedReconnect);
    PEGASUS_TEST_ASSERT(exceptionMessage->getType() == expectedType);

    if (expectedClass == MALFORMED_HTTP)
    {
        CIMClientMalformedHTTPException* malformedHTTPException =
            dynamic_cast<CIMClientMalformedHTTPException*>(
                exceptionMessage->clientException);
        PEGASUS_TEST_ASSERT (malformedHTTPException);
    }
    else if (expectedClass == HTTP_ERROR)
    {
        CIMClientHTTPErrorException* httpErrorException =
            dynamic_cast<CIMClientHTTPErrorException*>(
                exceptionMessage->clientException);
        PEGASUS_TEST_ASSERT (httpErrorException);
    }

    delete exceptionMessage->clientException;
    delete exceptionMessage;
}

void testDecodeExportResponse(
    Buffer httpBuffer,
    MessageType expectedType,
    // expectedClass is used only if expectedType is ClientExceptionMessage
    // otherwise, it is ignored
    CIMName expectedClass)
{
    ClientExceptionMessage* exceptionMessage;
    Array<HTTPHeader> headers;
    char* content;
    Uint32 contentLength;
    Uint32 statusCode;
    String reasonPhrase;
    Boolean cimReconnect;
    Boolean valid;
    Message* responseMessage;

    HTTPMessage httpMessage(httpBuffer);

    HTTPExportResponseDecoder::parseHTTPHeaders(&httpMessage, exceptionMessage,
        headers, contentLength, statusCode, reasonPhrase, cimReconnect,
        valid);

    HTTPExportResponseDecoder::validateHTTPHeaders(&httpMessage, headers,
        contentLength, statusCode, cimReconnect, reasonPhrase, content,
        exceptionMessage, valid);

    HTTPExportResponseDecoder::decodeExportResponse(content, cimReconnect,
        responseMessage);

    PEGASUS_TEST_ASSERT(responseMessage->getType() == expectedType);

    if (responseMessage->getType() == CLIENT_EXCEPTION_MESSAGE)
    {
        if (expectedClass == XML_EXCEPTION)
        {
            CIMClientXmlException* xmlException =
                dynamic_cast<CIMClientXmlException*>(((ClientExceptionMessage*)
                    responseMessage)->clientException);
            PEGASUS_TEST_ASSERT (xmlException);
        }
        else if (expectedClass == RESPONSE_EXCEPTION)
        {
            CIMClientResponseException* responseException =
                dynamic_cast<CIMClientResponseException*>(
                    ((ClientExceptionMessage*)
                    responseMessage)->clientException);
            PEGASUS_TEST_ASSERT (responseException);
        }

        delete ((ClientExceptionMessage*)responseMessage)->clientException;
    }

    delete responseMessage;
}


////////////////////////////////////////////////////////////////
//                                                            //
//  Test Cases for parseHTTPHeaders method                    //
//                                                            //
////////////////////////////////////////////////////////////////

void parseHTTPHeadersTestCases()
{
    //
    //  Test empty HTTP response message
    //
    //  Expected result:
    //    valid is false
    //    cimReconnect is false
    //    exceptionMessage type is CLIENT_EXCEPTION_MESSAGE (75)
    //    clientException is CIMClientMalformedHTTPException
    //
    Buffer buffer1(STRLIT_ARGS(""));
    testParseHTTPHeaders(buffer1, false, false, CLIENT_EXCEPTION_MESSAGE);

    //
    //  Test HTTP response message including Connection: Close header
    //  and a non-parsable status line
    //
    //  Expected result:
    //    valid is false
    //    cimReconnect is true
    //    exceptionMessage type is CLIENT_EXCEPTION_MESSAGE (75)
    //    clientException is CIMClientMalformedHTTPException
    //
    Buffer buffer2(STRLIT_ARGS(
                   "HTTP/1.1200OK\r\n"
                   "Connection: Close\r\n"));
    testParseHTTPHeaders(buffer2, false, true, CLIENT_EXCEPTION_MESSAGE);

    //
    //  Test HTTP response message including Connection: header
    //  with a connection-token other than "Close"
    //  and a non-parsable status line
    //
    //  Expected result:
    //    valid is false
    //    cimReconnect is false
    //    exceptionMessage type is CLIENT_EXCEPTION_MESSAGE (75)
    //    clientException is CIMClientMalformedHTTPException
    //
    Buffer buffer3(STRLIT_ARGS(
                   "HTTP/1.1200OK\r\n"
                   "Connection: Keep-Alive\r\n"));
    testParseHTTPHeaders(buffer3, false, false, CLIENT_EXCEPTION_MESSAGE);
}


////////////////////////////////////////////////////////////////
//                                                            //
//  Test Cases for validateHTTPHeaders method                 //
//                                                            //
////////////////////////////////////////////////////////////////

void validateHTTPHeadersTestCases()
{
    //
    //  Test HTTP response message with non-success status
    //
    //  Expected result:
    //    valid is false
    //    cimReconnect is false
    //    exceptionMessage type is CLIENT_EXCEPTION_MESSAGE (75)
    //    clientException is CIMClientHTTPErrorException
    //
    Buffer buffer4(STRLIT_ARGS(
                   "HTTP/1.1 501 Not Implemented\r\n"
                   "CIMError: multiple-requests-unsupported\r\n"));

    testValidateHTTPHeaders(buffer4, false, false,
        CLIENT_EXCEPTION_MESSAGE, HTTP_ERROR);

    //
    //  Test HTTP response message with non-success status
    //  and invalid URI encoding in error
    //
    //  Expected result:
    //    valid is false
    //    cimReconnect is false
    //    exceptionMessage type is CLIENT_EXCEPTION_MESSAGE (75)
    //    clientException is CIMClientHTTPErrorException
    //
    Buffer buffer5(STRLIT_ARGS(
                   "HTTP/1.1 400 Bad Request\r\n"
                   "PGErrorDetail: HTTP%20request%\r\n"));

    testValidateHTTPHeaders(buffer5, false, false,
        CLIENT_EXCEPTION_MESSAGE, HTTP_ERROR);

    //
    //  Test HTTP response message with missing CIMExport HTTP header
    //
    //  Expected result:
    //    valid is false
    //    cimReconnect is false
    //    exceptionMessage type is CLIENT_EXCEPTION_MESSAGE (75)
    //    clientException is CIMClientMalformedHTTPException
    //
    Buffer buffer6(STRLIT_ARGS(
        "HTTP/1.1 200 OK\r\n"
        "Content-Type: application/xml; charset=\"utf-8\"\r\n"
        "transfer-encoding: chunked\r\n"
        "Trailer: CIMStatusCode, CIMStatusCodeDescription, "
            "Content-Language\r\n"
        "\r\n"
        "<?xml version=\"1.0\" encoding=\"utf-8\"?>"
        "<CIM CIMVERSION=\"2.0\" DTDVERSION=\"2.0\">"
        "<MESSAGE ID=\"1001\" PROTOCOLVERSION=\"1.0\">"
        "<SIMPLEEXPRSP>"
        "<EXPMETHODRESPONSE NAME=\"ExportIndication\">"
        "<ERROR CODE=\"7\" DESCRIPTION=\"CIM_ERR_NOT_SUPPORTED\"/>"
        "</EXPMETHODRESPONSE>"
        "</SIMPLEEXPRSP>"
        "</MESSAGE>"
        "</CIM>"));

    testValidateHTTPHeaders(buffer6, false, false,
        CLIENT_EXCEPTION_MESSAGE, MALFORMED_HTTP);

    //
    //  Test HTTP response message with missing CIMContentType HTTP header
    //
    //  Expected result:
    //    valid is false
    //    cimReconnect is false
    //    exceptionMessage type is CLIENT_EXCEPTION_MESSAGE (75)
    //    clientException is CIMClientMalformedHTTPException
    //
    Buffer buffer7(STRLIT_ARGS(
        "HTTP/1.1 200 OK\r\n"
        "transfer-encoding: chunked\r\n"
        "CIMExport: MethodResponse\r\n"
        "Trailer: CIMStatusCode, CIMStatusCodeDescription, "
            "Content-Language\r\n"
        "\r\n"
        "<?xml version=\"1.0\" encoding=\"utf-8\"?>"
        "<CIM CIMVERSION=\"2.0\" DTDVERSION=\"2.0\">"
        "<MESSAGE ID=\"1001\" PROTOCOLVERSION=\"1.0\">"
        "<SIMPLEEXPRSP>"
        "<EXPMETHODRESPONSE NAME=\"ExportIndication\">"
        "<ERROR CODE=\"7\" DESCRIPTION=\"CIM_ERR_NOT_SUPPORTED\"/>"
        "</EXPMETHODRESPONSE>"
        "</SIMPLEEXPRSP>"
        "</MESSAGE>"
        "</CIM>"));

    testValidateHTTPHeaders(buffer7, false, false,
        CLIENT_EXCEPTION_MESSAGE, MALFORMED_HTTP);

    //
    //  Test response message with unexpected CIMExport HTTP header value
    //
    //  Expected result:
    //    valid is false
    //    cimReconnect is false
    //    exceptionMessage type is CLIENT_EXCEPTION_MESSAGE (75)
    //    clientException is CIMClientMalformedHTTPException
    //
    Buffer buffer8(STRLIT_ARGS(
        "HTTP/1.1 200 OK\r\n"
        "Content-Type: application/xml; charset=\"utf-8\"\r\n"
        "transfer-encoding: chunked\r\n"
        "CIMExport: Bogus\r\n"
        "Trailer: CIMStatusCode, CIMStatusCodeDescription, "
            "Content-Language\r\n"
        "\r\n"
        "<?xml version=\"1.0\" encoding=\"utf-8\"?>"
        "<CIM CIMVERSION=\"2.0\" DTDVERSION=\"2.0\">"
        "<MESSAGE ID=\"1001\" PROTOCOLVERSION=\"1.0\">"
        "<SIMPLEEXPRSP>"
        "<EXPMETHODRESPONSE NAME=\"ExportIndication\">"
        "<ERROR CODE=\"7\" DESCRIPTION=\"CIM_ERR_NOT_SUPPORTED\"/>"
        "</EXPMETHODRESPONSE>"
        "</SIMPLEEXPRSP>"
        "</MESSAGE>"
        "</CIM>"));

    testValidateHTTPHeaders(buffer8, false, false,
        CLIENT_EXCEPTION_MESSAGE, MALFORMED_HTTP);
}


////////////////////////////////////////////////////////////////
//                                                            //
//  Test Cases for decodeExportResponse method                //
//                                                            //
////////////////////////////////////////////////////////////////

void decodeExportResponseTestCases()
{
    //
    //  Test missing XML MESSAGE element
    //
    //  Expected result:
    //    response Message type is CLIENT_EXCEPTION_MESSAGE (75)
    //    clientException is CIMClientXmlException
    //
    Buffer buffer9(STRLIT_ARGS(
        "HTTP/1.1 200 OK\r\n"
        "Content-Type: application/xml; charset=\"utf-8\"\r\n"
        "transfer-encoding: chunked\r\n"
        "CIMExport: MethodResponse\r\n"
        "Trailer: CIMStatusCode, CIMStatusCodeDescription, "
            "Content-Language\r\n"
        "\r\n"
        "<?xml version=\"1.0\" encoding=\"utf-8\"?>"
        "<CIM CIMVERSION=\"2.0\" DTDVERSION=\"2.0\">"
        "<SIMPLEEXPRSP>"
        "<EXPMETHODRESPONSE NAME=\"ExportIndication\">"
        "<ERROR CODE=\"7\" DESCRIPTION=\"CIM_ERR_NOT_SUPPORTED\"/>"
        "</EXPMETHODRESPONSE>"
        "</SIMPLEEXPRSP>"
        "</CIM>"));

    testDecodeExportResponse(buffer9, CLIENT_EXCEPTION_MESSAGE, XML_EXCEPTION);

    //
    //  Test unsupported protocol version
    //
    //  Expected result:
    //    response Message type is CLIENT_EXCEPTION_MESSAGE (75)
    //    clientException is CIMClientResponseException
    //
    Buffer buffer10(STRLIT_ARGS(
        "HTTP/1.1 200 OK\r\n"
        "Content-Type: application/xml; charset=\"utf-8\"\r\n"
        "transfer-encoding: chunked\r\n"
        "CIMExport: MethodResponse\r\n"
        "Trailer: CIMStatusCode, CIMStatusCodeDescription, "
            "Content-Language\r\n"
        "\r\n"
        "<?xml version=\"1.0\" encoding=\"utf-8\"?>"
        "<CIM CIMVERSION=\"2.0\" DTDVERSION=\"2.0\">"
        "<MESSAGE ID=\"1001\" PROTOCOLVERSION=\"2.0\">"
        "<SIMPLEEXPRSP>"
        "<EXPMETHODRESPONSE NAME=\"ExportIndication\">"
        "<ERROR CODE=\"7\" DESCRIPTION=\"CIM_ERR_NOT_SUPPORTED\"/>"
        "</EXPMETHODRESPONSE>"
        "</SIMPLEEXPRSP>"
        "</MESSAGE>"
        "</CIM>"));

    testDecodeExportResponse(buffer10,
        CLIENT_EXCEPTION_MESSAGE, RESPONSE_EXCEPTION);

    //
    //  Test missing XML EXPMETHODRESPONSE element
    //
    //  Expected result:
    //    response Message type is CLIENT_EXCEPTION_MESSAGE (75)
    //    clientException is CIMClientXmlException
    //
    Buffer buffer11(STRLIT_ARGS(
        "HTTP/1.1 200 OK\r\n"
        "Content-Type: application/xml; charset=\"utf-8\"\r\n"
        "transfer-encoding: chunked\r\n"
        "CIMExport: MethodResponse\r\n"
        "Trailer: CIMStatusCode, CIMStatusCodeDescription, "
            "Content-Language\r\n"
        "\r\n"
        "<?xml version=\"1.0\" encoding=\"utf-8\"?>"
        "<CIM CIMVERSION=\"2.0\" DTDVERSION=\"2.0\">"
        "<MESSAGE ID=\"1001\" PROTOCOLVERSION=\"1.0\">"
        "<SIMPLEEXPRSP>"
        "<ERROR CODE=\"7\" DESCRIPTION=\"CIM_ERR_NOT_SUPPORTED\"/>"
        "</SIMPLEEXPRSP>"
        "</MESSAGE>"
        "</CIM>"));

    testDecodeExportResponse(buffer11, CLIENT_EXCEPTION_MESSAGE, XML_EXCEPTION);

    //
    //  Test unrecognized ExpMethodResponse name attribute
    //
    //  Expected result:
    //    response Message type is CLIENT_EXCEPTION_MESSAGE (75)
    //    clientException is CIMClientXmlException
    //
    Buffer buffer12(STRLIT_ARGS(
        "HTTP/1.1 200 OK\r\n"
        "Content-Type: application/xml; charset=\"utf-8\"\r\n"
        "transfer-encoding: chunked\r\n"
        "CIMExport: MethodResponse\r\n"
        "Trailer: CIMStatusCode, CIMStatusCodeDescription, "
            "Content-Language\r\n"
        "\r\n"
        "<?xml version=\"1.0\" encoding=\"utf-8\"?>"
        "<CIM CIMVERSION=\"2.0\" DTDVERSION=\"2.0\">"
        "<MESSAGE ID=\"1001\" PROTOCOLVERSION=\"1.0\">"
        "<SIMPLEEXPRSP>"
        "<EXPMETHODRESPONSE NAME=\"Bogus\">"
        "<ERROR CODE=\"7\" DESCRIPTION=\"CIM_ERR_NOT_SUPPORTED\"/>"
        "</EXPMETHODRESPONSE>"
        "</SIMPLEEXPRSP>"
        "</MESSAGE>"
        "</CIM>"));

    testDecodeExportResponse(buffer12, CLIENT_EXCEPTION_MESSAGE, XML_EXCEPTION);

    //
    //  Test empty EXPMETHODRESPONSE tag
    //
    //  Expected result:
    //    response Message type is CIM_EXPORT_INDICATION_RESPONSE_MESSAGE
    //        (38)
    //
    Buffer buffer13(STRLIT_ARGS(
        "HTTP/1.1 200 OK\r\n"
        "Content-Type: application/xml; charset=\"utf-8\"\r\n"
        "transfer-encoding: chunked\r\n"
        "CIMExport: MethodResponse\r\n"
        "Trailer: CIMStatusCode, CIMStatusCodeDescription, "
            "Content-Language\r\n"
        "\r\n"
        "<?xml version=\"1.0\" encoding=\"utf-8\"?>"
        "<CIM CIMVERSION=\"2.0\" DTDVERSION=\"2.0\">"
        "<MESSAGE ID=\"1001\" PROTOCOLVERSION=\"1.0\">"
        "<SIMPLEEXPRSP>"
        "<EXPMETHODRESPONSE NAME=\"ExportIndication\"/>"
        "</SIMPLEEXPRSP>"
        "</MESSAGE>"
        "</CIM>"));

    testDecodeExportResponse(buffer13, CIM_EXPORT_INDICATION_RESPONSE_MESSAGE,
        CIMName());

    //
    //  Test EXPMETHODRESPONSE with empty IRETURNVALUE tag
    //
    //  Expected result:
    //    response Message type is CIM_EXPORT_INDICATION_RESPONSE_MESSAGE
    //        (38)
    //
    Buffer buffer14(STRLIT_ARGS(
        "HTTP/1.1 200 OK\r\n"
        "Content-Type: application/xml; charset=\"utf-8\"\r\n"
        "transfer-encoding: chunked\r\n"
        "CIMExport: MethodResponse\r\n"
        "Trailer: CIMStatusCode, CIMStatusCodeDescription, "
            "Content-Language\r\n"
        "\r\n"
        "<?xml version=\"1.0\" encoding=\"utf-8\"?>"
        "<CIM CIMVERSION=\"2.0\" DTDVERSION=\"2.0\">"
        "<MESSAGE ID=\"1001\" PROTOCOLVERSION=\"1.0\">"
        "<SIMPLEEXPRSP>"
        "<EXPMETHODRESPONSE NAME=\"ExportIndication\">"
        "<IRETURNVALUE/>"
        "</EXPMETHODRESPONSE>"
        "</SIMPLEEXPRSP>"
        "</MESSAGE>"
        "</CIM>"));

    testDecodeExportResponse(buffer14, CIM_EXPORT_INDICATION_RESPONSE_MESSAGE,
        CIMName());

    //
    //  Test EXPMETHODRESPONSE with non-empty IRETURNVALUE tag
    //
    //  Expected result:
    //    response Message type is CLIENT_EXCEPTION_MESSAGE (75)
    //    clientException is CIMClientXmlException
    //
    Buffer buffer15(STRLIT_ARGS(
        "HTTP/1.1 200 OK\r\n"
        "Content-Type: application/xml; charset=\"utf-8\"\r\n"
        "transfer-encoding: chunked\r\n"
        "CIMExport: MethodResponse\r\n"
        "Trailer: CIMStatusCode, CIMStatusCodeDescription, "
            "Content-Language\r\n"
        "\r\n"
        "<?xml version=\"1.0\" encoding=\"utf-8\"?>"
        "<CIM CIMVERSION=\"2.0\" DTDVERSION=\"2.0\">"
        "<MESSAGE ID=\"1001\" PROTOCOLVERSION=\"1.0\">"
        "<SIMPLEEXPRSP>"
        "<EXPMETHODRESPONSE NAME=\"ExportIndication\">"
        "<IRETURNVALUE>0</IRETURNVALUE>"
        "</EXPMETHODRESPONSE>"
        "</SIMPLEEXPRSP>"
        "</MESSAGE>"
        "</CIM>"));

    testDecodeExportResponse(buffer15, CLIENT_EXCEPTION_MESSAGE, XML_EXCEPTION);
}


int main(int, char** argv)
{
    try
    {
        parseHTTPHeadersTestCases();
        validateHTTPHeadersTestCases();
        decodeExportResponseTestCases();
    }
    catch(const Exception& e)
    {
        cerr << argv[0] << ": Exception " << e.getMessage() << endl;
        return 1;
    }
    catch(...)
    {
        cerr << argv[0] << ": " << "Unknown error" << endl;
        return 1;
    }

    cout << "+++++ passed all tests" << endl;
    return 0;
}
