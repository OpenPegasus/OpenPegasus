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
//
//%/////////////////////////////////////////////////////////////////////////////

#ifndef Pegasus_HttpConstants_h
#define Pegasus_HttpConstants_h

PEGASUS_NAMESPACE_BEGIN

//
//  This include file contains constants related to the HTTP encapsulation
//  of an XML-encoded CIM operation, including HTTP request elements, HTTP
//  header names and values, and special characters used in headers and
//  CIMObject values.
//

//
//  HTTP Request Line elements
//

/**
 *  A string representing the HTTP M-POST method.
 */
#define HTTP_METHOD_MPOST "M-POST"

/**
 *  A string representing the HTTP POST method.
 */
#define HTTP_METHOD_POST "POST"

/**
 *  A string representing a bogus HTTP method, to allow testing of CIM server
 *  handling of unknown HTTP methods.
 */
#define HTTP_METHOD_BOGUS "BOGUS"

/**
 *  A string representing the request URI cimom.
 */
#define HTTP_REQUEST_URI_CIMOM "/cimom"

/**
 *  A string representing the protocol HTTP
 */
#define HTTP_PROTOCOL "HTTP/"

/**
 *  A string representing the HTTP/1.0 version.
 */
#define HTTP_VERSION_10 "1.0"

/**
 *  A string representing the HTTP/1.1 version.
 */
#define HTTP_VERSION_11 "1.1"

/**
 *  A string representing basic authentication
 */
#define HTTP_AUTH_BASIC "Basic"
/**
 *  A string representing the required CRLF sequence.
 */
#define HTTP_CRLF "\r\n"

/**
 *  A character representing the required SP separator.
 */
#define HTTP_SP " "


//
//  Header names
//

/**
 *  The name of the HTTP header specifying the authentication type and value
 */
#define HEADER_NAME_AUTHORIZATION "Authorization"
/**
 *  The name of the HTTP header specifying the Internet host of the resource
 *  being requested.
 */
#define HEADER_NAME_HOST "Host"

/**
 *  The name of the HTTP header specifying the media type of the underlying
 *  data.
 */
#define HEADER_NAME_CONTENTTYPE "Content-Type"

/**
 *  The name of the HTTP header specifying the size of the entity-body.
 */
#define HEADER_NAME_CONTENTLENGTH "Content-Length"

/**
 *  The name of the HTTP header specifying a mandatory extension
 *  declaration.
 */
#define HEADER_NAME_MAN "Man"

/**
 *  The name of the HTTP extension header identifying the version of the
 *  CIM mapping onto HTTP being used by the sending entity.
 */
#define HEADER_NAME_CIMPROTOCOLVERSION "CIMProtocolVersion"

/**
 *  The name of the HTTP extension header identifying the message as a CIM
 *  operation request or response.
 */
#define HEADER_NAME_CIMOPERATION "CIMOperation"

/**
 *  The name of the HTTP extension header identifying the name of the CIM
 *  method to be invoked.
 */
#define HEADER_NAME_CIMMETHOD "CIMMethod"

/**
 *  The name of the HTTP extension header identifying the name of the CIM
 *  object on which the method is to be invoked.
 */
#define HEADER_NAME_CIMOBJECT "CIMObject"

/**
 *  The name of the HTTP extension header identifying the encapsulated
 *  Operation Request Message as containing multiple method invocations.
 */
#define HEADER_NAME_CIMBATCH "CIMBatch"

//
//  Header values
//

/**
 *  The value of the HTTP Content-type header specifying that the media
 *  type of the underlying data is application data in XML format.
 */
#define HEADER_VALUE_CONTENTTYPE "application/xml; charset=\"utf-8\""

#ifdef PEGASUS_ENABLE_PROTOCOL_WSMAN
/**
 *  The value of the WS-Management HTTP Content-type header specifying that the
 *  media type of the underlying data is application data in soap+xml format.
 */
# define WSMAN_HEADER_VALUE_CONTENTTYPE "application/soap+xml; charset=utf-8"
#endif

/**
 *  The value of the HTTP Man header specifying the CIM mapping onto HTTP
 *  extension header name space URI.
 */
#define HEADER_VALUE_MAN "http://www.dmtf.org/cim/mapping/http/v1.0;ns="

/**
 *  The value of the HTTP CIMOperation header identifying the message as
 *  carrying a CIM operation request.
 */
#define HEADER_VALUE_CIMOPERATION "MethodCall"


//
//  Characters used in headers and CIMObject values
//

/**
 *  The character used to separate the header name and header value.
 */
#define HEADER_SEPARATOR ':'

/**
 *  The character used to delimit the header prefix and extension header.
 */
#define HEADER_PREFIX_DELIMITER '-'


PEGASUS_NAMESPACE_END

#endif /* Pegasus_HttpConstants_h */
