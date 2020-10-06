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
#include <Pegasus/Common/TimeValue.h>
#include <Pegasus/Common/XmlReader.h>
#include <Pegasus/Common/XmlWriter.h>
#include <Pegasus/Common/XmlConstants.h>
#include "HttpConstants.h"
#include "XMLProcess.h"

PEGASUS_NAMESPACE_BEGIN

/**

    Encapsulates the XML request in an HTTP M-POST or POST request message.
    Generates the appropriate HTTP extension headers corresponding to the
    XML request, in accordance with Specifications for CIM Operations over
    HTTP, Version 1.0, Section 3.  This method should be called only when
    the current parser location is the xml declaration.  If the xml
    declaration is not found, but the first token in the input is
    HTTP_METHOD_MPOST or HTTP_METHOD_POST, it is assumed that the request is
    already encapsulated in an HTTP request, and the message is returned
    unchanged.  No attempt is made to validate the complete HTTP request.
    If the useMPost parameter is TRUE, the headers are generated for an
    M-POST request.  Otherwise, the headers are generated for a POST
    request.  If the useHTTP11 parameter is TRUE, the headers are generated
    for an HTTP/1.1 request.  Otherwise, the headers are generated for an
    HTTP/1.0 request.  The combination of useMPost true and useHTTP11 false
    is invalid, but this function does not check for this case.  The XML
    request is examined only as much as necessary to generate the required
    headers.  This method does not attempt to validate the entire XML request.

    @param   parser              XmlParser instance corresponding to the
                                 XML request
    @param   hostName            host name to be used in HTTP Host header
    @param   useMPost            Boolean indicating that headers should be
                                 generated for an M-POST request
    @param   useHTTP11           Boolean indicating that headers should be
                                 generated for an HTTP/1.1 request
    @param   content             Buffer containing XML request
    @param   httpHeaders         Buffer returning the HTTP headers

    @return  Buffer containing the XML request encapsulated in an
             HTTP request message

    @exception  XmlValidationError  if the XML input is invalid
    @exception  XmlSemanticError    if the XML input contains a semantic error
    @exception  WbemExecException   if the input contains neither XML nor HTTP
                                    M-POST or POST method request

 */
Buffer XMLProcess::encapsulate( XmlParser& parser,
                                       const String& hostName,
                                       Boolean useMPost,
                                       Boolean useHTTP11,
                                       Buffer& content,
                                       Buffer& httpHeaders
                                       )
throw (XmlValidationError, XmlSemanticError, WbemExecException,
               XmlException, Exception)
{
    XmlEntry                     entry;
    Buffer                    message;
    String                       messageId;
    const char*                  cimVersion            = 0;
    const char*                  dtdVersion            = 0;
    String                       protocolVersion;
    CIMName                      className;
    CIMName                      methodName;
    CIMObjectPath                objectName;
    Buffer                    encoded;
    String                    objPath;
    Array<CIMKeyBinding>         keyBindings;
    Boolean                      multireq              = false;
    static Uint32                BUFFERSIZE            = 1024;
    Boolean hasXmlDeclaration = false;

    //
    //  xml declaration
    //
    if (!(hasXmlDeclaration = XmlReader::testXmlDeclaration (parser, entry)))
    {
        //
        //  No xml declaration
        //  Request may already be encapsulated in HTTP
        //  Check for HTTP method
        //
        char tmp [8];
        char* tmpp = & (tmp [0]);
        strncpy (tmpp, entry.text, 8);
        tmpp[7] = 0;
#if !defined(PEGASUS_COMPILER_MSVC) && !defined(PEGASUS_OS_ZOS)
        char *last;
        char* p = strtok_r (tmpp, HTTP_SP, &last);
#else
        char* p = strtok (tmpp, HTTP_SP);
#endif
        if (p != NULL)
        {
            if ((strcmp (p, HTTP_METHOD_MPOST) == 0) ||
                (strcmp (p, HTTP_METHOD_POST) == 0) ||
            //  This is a special request used for testing.
            //  It includes the HTTP header.
            //  Return the message as is.
                (strcmp (p, HTTP_METHOD_BOGUS) == 0))
            {
                return content;
            }
        }
     }

#ifdef PEGASUS_ENABLE_PROTOCOL_WSMAN

     // Check if the next tag is a SOAP envelope.
     if (parser.next(entry) &&
             entry.type == XmlEntry::START_TAG &&
             strcmp(entry.localName, "Envelope") == 0)
     {
          //
          //  Set WSMAN headers and content.
          //
          message << HTTP_METHOD_POST << HTTP_SP
                  << "/wsman" << HTTP_SP
                  << HTTP_PROTOCOL << HTTP_VERSION_11 << HTTP_CRLF;
          message << HEADER_NAME_HOST << HEADER_SEPARATOR
                  << HTTP_SP << hostName << HTTP_CRLF;
          message << HEADER_NAME_CONTENTTYPE << HEADER_SEPARATOR
                  << HTTP_SP << WSMAN_HEADER_VALUE_CONTENTTYPE
                  << HTTP_CRLF;
          message << HEADER_NAME_CONTENTLENGTH << HEADER_SEPARATOR
                  << HTTP_SP << (Uint32)content.size () << HTTP_CRLF;

          httpHeaders << message;
          message << HTTP_CRLF;
          message << content;
          return message;
     }
     else
     {
         parser.putBack(entry);
     }

#endif

    if (!hasXmlDeclaration)
    {
        //
        //  Input contains neither XML declaration nor HTTP M-POST or
        //  POST method request
        //
        throw WbemExecException(WbemExecException::INVALID_INPUT);
    }

    //
    //  CIM element
    //
    XmlReader::getCimStartTag (parser, cimVersion, dtdVersion);


    //
    //  MESSAGE element
    //
    if (!XmlReader::getMessageStartTag (parser, messageId, protocolVersion))
    {
        throw XmlValidationError(parser.getLine(), "expected MESSAGE element");

        // l10n TODO
        //MessageLoaderParms mlParms(
        //  "Server.CIMOperationRequestDecoder.EXPECTED_MESSAGE_ELEMENT",
        //  "expected MESSAGE element");
        //throw XmlValidationError(parser.getLine(), mlParms);
    }

    //
    //  MULTIREQ or SIMPLEREQ element
    //
    if (XmlReader::testStartTag (parser, entry, XML_ELEMENT_MULTIREQ))
    {
        multireq = true;
    }
    else if (!XmlReader::testStartTag (parser, entry, XML_ELEMENT_SIMPLEREQ))
    {
        // l10n TODO
        throw XmlValidationError (parser.getLine (), MISSING_ELEMENT_REQ);
    }

    //
    //  SIMPLEREQ element
    //
    else
    {
        //
        //  IMETHODCALL element
        //
        if (XmlReader::testStartTag (parser, entry, XML_ELEMENT_IMETHODCALL))
        {
            //
            //  Get NAME attribute of IMETHODCALL element
            //
            methodName = XmlReader::getCimNameAttribute (parser.getLine (),
                entry, XML_ELEMENT_IMETHODCALL);

            //
            //  Construct the object path from the LOCALNAMESPACEPATH
            //  subelements (NAMESPACE elements)
            //
            String namespaceName;

            if (!XmlReader::getLocalNameSpacePathElement(parser, namespaceName))
            {
                throw XmlValidationError(parser.getLine(),
                    "expected LOCALNAMESPACEPATH element");

                // l10n TODO
                //MessageLoaderParms mlParms(
                //    "Server.CIMOperationRequestDecoder."
                //        "EXPECTED_LOCALNAMESPACEPATH_ELEMENT",
                //    "expected LOCALNAMESPACEPATH element");
                //throw XmlValidationError(parser.getLine(),mlParms);
            }
            objPath.append(namespaceName);
        }

        //
        //  METHODCALL element
        //
        else if (XmlReader::testStartTag (parser, entry,
            XML_ELEMENT_METHODCALL))
        {
            //
            //  Get NAME attribute of METHODCALL element
            //
            methodName = XmlReader::getCimNameAttribute (parser.getLine (),
                entry, XML_ELEMENT_METHODCALL);

            //
            //  LOCALCLASSPATH or LOCALINSTANCEPATH element
            //
            if (XmlReader::getLocalClassPathElement (parser, objectName))
            {
                objPath.append(objectName.toString());
            }
            else if (XmlReader::getLocalInstancePathElement (parser,
                objectName))
            {
                objPath.append(objectName.toString());
            }
            else
            {
                // l10n TODO - Use CIMOperationRequestDecoder message when it
                // is available (or perhaps use Common.XmlReader.
                // EXPECTED_LOCALINSTANCEPATH_OR_LOCALCLASSPATH_ELEMENT)
                throw XmlValidationError (parser.getLine (),
                    MISSING_ELEMENT_LOCALPATH);
            }
        }
        else
        {
            throw XmlValidationError(parser.getLine(),
                "expected IMETHODCALL or METHODCALL element");

            // l10n TODO
            //MessageLoaderParms mlParms(
            // "Server.CIMOperationRequestDecoder.EXPECTED_IMETHODCALL_ELEMENT",
            // "expected IMETHODCALL or METHODCALL element");
            //throw XmlValidationError(parser.getLine(),mlParms);
        }
    }

    //
    //  Set headers
    //
    message.reserveCapacity (BUFFERSIZE);

    //
    //  Generate header prefix
    //
    srand(TimeValue::getCurrentTime().toMilliseconds() & 0xFFFFFFFF);
    char nn[3];
    nn[0] = '0' + (rand() % 10);
    nn[1] = '0' + (rand() % 10);
    nn[2] = 0;

    if (useMPost)
    {
        message << HTTP_METHOD_MPOST << HTTP_SP << HTTP_REQUEST_URI_CIMOM
                << HTTP_SP << HTTP_PROTOCOL << HTTP_VERSION_11 << HTTP_CRLF;
    }
    else
    {
        message << HTTP_METHOD_POST << HTTP_SP << HTTP_REQUEST_URI_CIMOM
                << HTTP_SP << HTTP_PROTOCOL;
        if (useHTTP11)
        {
            message << HTTP_VERSION_11;
        }
        else
        {
            message << HTTP_VERSION_10;
        }
        message << HTTP_CRLF;
    }
    message << HEADER_NAME_HOST << HEADER_SEPARATOR << HTTP_SP << hostName
            << HTTP_CRLF;
    message << HEADER_NAME_CONTENTTYPE << HEADER_SEPARATOR << HTTP_SP
            << HEADER_VALUE_CONTENTTYPE
            << HTTP_CRLF;
    message << HEADER_NAME_CONTENTLENGTH << HEADER_SEPARATOR << HTTP_SP
            << (Uint32)content.size () << HTTP_CRLF;

    if (useMPost)
    {
        message << HEADER_NAME_MAN << HEADER_SEPARATOR << HTTP_SP
                << HEADER_VALUE_MAN << nn << HTTP_CRLF;
        message << nn << HEADER_PREFIX_DELIMITER
                << HEADER_NAME_CIMPROTOCOLVERSION << HEADER_SEPARATOR
                << HTTP_SP << protocolVersion << HTTP_CRLF;
        message << nn << HEADER_PREFIX_DELIMITER << HEADER_NAME_CIMOPERATION
                << HEADER_SEPARATOR << HTTP_SP << HEADER_VALUE_CIMOPERATION
                << HTTP_CRLF;
        if (multireq)
        {
            message << nn << HEADER_PREFIX_DELIMITER << HEADER_NAME_CIMBATCH
                << HEADER_SEPARATOR << HTTP_CRLF;
        }
        else
        {
            message << nn << HEADER_PREFIX_DELIMITER << HEADER_NAME_CIMMETHOD
                << HEADER_SEPARATOR << HTTP_SP
                << XmlWriter::encodeURICharacters(methodName.getString())
                << HTTP_CRLF;
            message << nn << HEADER_PREFIX_DELIMITER << HEADER_NAME_CIMOBJECT
                << HEADER_SEPARATOR << HTTP_SP
                << XmlWriter::encodeURICharacters(objPath) << HTTP_CRLF;
        }
    }
    else
    {
        message << HEADER_NAME_CIMPROTOCOLVERSION << HEADER_SEPARATOR
                << HTTP_SP << protocolVersion << HTTP_CRLF;
        message << HEADER_NAME_CIMOPERATION << HEADER_SEPARATOR << HTTP_SP
                << HEADER_VALUE_CIMOPERATION << HTTP_CRLF;
        if (multireq)
        {
            message << HEADER_NAME_CIMBATCH << HEADER_SEPARATOR << HTTP_CRLF;
        }
        else
        {
            message << HEADER_NAME_CIMMETHOD << HEADER_SEPARATOR << HTTP_SP
                    << XmlWriter::encodeURICharacters(methodName.getString())
                    << HTTP_CRLF;
            message << HEADER_NAME_CIMOBJECT << HEADER_SEPARATOR << HTTP_SP
                    << XmlWriter::encodeURICharacters(objPath) << HTTP_CRLF;
        }
    }

    httpHeaders << message;
    message << HTTP_CRLF;
    message << content;

    return message;
}

PEGASUS_NAMESPACE_END
