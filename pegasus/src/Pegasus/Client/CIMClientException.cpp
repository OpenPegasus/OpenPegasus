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

#include <cstdio>
#include "CIMClientException.h"
#include <Pegasus/Common/Tracer.h>
#include <Pegasus/Common/ExceptionRep.h>

PEGASUS_NAMESPACE_BEGIN

////////////////////////////////////////////////////////////////////////////////
//
// CIMClientMalformedHTTPException
//
////////////////////////////////////////////////////////////////////////////////

CIMClientMalformedHTTPException::CIMClientMalformedHTTPException(
    const String& message)
    : Exception(message)
{
}


////////////////////////////////////////////////////////////////////////////////
//
// CIMClientHTTPErrorException
//
////////////////////////////////////////////////////////////////////////////////

class CIMClientHTTPErrorExceptionRep : public ExceptionRep
{
public:

    // Note:  The default implementations of the default constructor, copy
    // constructor, and assignment operator are used.

    Uint32 httpStatusCode;
    String reasonPhrase;
    String cimError;
    String cimErrorDetail;
};

static String _makeHTTPErrorMessage(
    Uint32 httpStatusCode,
    const String& reasonPhrase,
    const String& cimError,
    const String& cimErrorDetail)
{
    String tmp = "HTTP Error (";
    char buffer[32];
    sprintf(buffer, "%u", httpStatusCode);
    tmp.append(buffer);
    if (reasonPhrase != String::EMPTY)
    {
        tmp.append(" ");
        tmp.append(reasonPhrase);
    }
    tmp.append(")");

    if ((cimError != String::EMPTY) || (cimErrorDetail != String::EMPTY))
    {
        tmp.append(":");
        if (cimError != String::EMPTY)
        {
            tmp.append(" CIMError = \"");
            tmp.append(cimError);
            tmp.append("\"");
        }
        if (cimErrorDetail != String::EMPTY)
        {
            tmp.append(" Detail = \"");
            tmp.append(cimErrorDetail);
            tmp.append("\"");
        }
    }
    tmp.append(".");
    return tmp;
}

CIMClientHTTPErrorException::CIMClientHTTPErrorException(
    Uint32 httpStatusCode,
    const String& cimError,
    const String& cimErrorDetail)
    : Exception()
{
    CIMClientHTTPErrorExceptionRep* tmp =
        new CIMClientHTTPErrorExceptionRep ();
    tmp->message = _makeHTTPErrorMessage (httpStatusCode, String::EMPTY,
                                          cimError, cimErrorDetail);
    tmp->httpStatusCode = httpStatusCode;
    tmp->reasonPhrase = String::EMPTY;
    tmp->cimError = cimError;
    tmp->cimErrorDetail = cimErrorDetail;
    _rep = tmp;
}

CIMClientHTTPErrorException::CIMClientHTTPErrorException(
    Uint32 httpStatusCode,
    const String& reasonPhrase,
    const String& cimError,
    const String& cimErrorDetail)
    : Exception()
{
    CIMClientHTTPErrorExceptionRep* tmp =
        new CIMClientHTTPErrorExceptionRep ();
    tmp->message = _makeHTTPErrorMessage (httpStatusCode, reasonPhrase,
                                          cimError, cimErrorDetail);
    tmp->httpStatusCode = httpStatusCode;
    tmp->reasonPhrase = reasonPhrase;
    tmp->cimError = cimError;
    tmp->cimErrorDetail = cimErrorDetail;
    _rep = tmp;
}

CIMClientHTTPErrorException::CIMClientHTTPErrorException(
    const CIMClientHTTPErrorException& httpError)
    : Exception()
{
    _rep = new CIMClientHTTPErrorExceptionRep(
        *reinterpret_cast<CIMClientHTTPErrorExceptionRep*>(httpError._rep));
}

CIMClientHTTPErrorException::~CIMClientHTTPErrorException()
{
}

CIMClientHTTPErrorException& CIMClientHTTPErrorException::operator=(
    const CIMClientHTTPErrorException& httpError)
{
    if (&httpError != this)
    {
        CIMClientHTTPErrorExceptionRep* left =
            reinterpret_cast<CIMClientHTTPErrorExceptionRep*>(this->_rep);
        CIMClientHTTPErrorExceptionRep* right =
            reinterpret_cast<CIMClientHTTPErrorExceptionRep*>(httpError._rep);
        *left = *right;
    }
    return *this;
}

Uint32 CIMClientHTTPErrorException::getCode() const
{
    CIMClientHTTPErrorExceptionRep* rep;
    rep = reinterpret_cast<CIMClientHTTPErrorExceptionRep*>(_rep);
    return rep->httpStatusCode;
}

String CIMClientHTTPErrorException::getCIMError() const
{
    CIMClientHTTPErrorExceptionRep* rep;
    rep = reinterpret_cast<CIMClientHTTPErrorExceptionRep*>(_rep);
    return rep->cimError;
}

String CIMClientHTTPErrorException::getCIMErrorDetail() const
{
    CIMClientHTTPErrorExceptionRep* rep;
    rep = reinterpret_cast<CIMClientHTTPErrorExceptionRep*>(_rep);
    return rep->cimErrorDetail;
}


////////////////////////////////////////////////////////////////////////////////
//
// CIMClientXmlException
//
////////////////////////////////////////////////////////////////////////////////

CIMClientXmlException::CIMClientXmlException(const String& message)
    : Exception(message)
{
}


////////////////////////////////////////////////////////////////////////////////
//
// CIMClientResponseException
//
////////////////////////////////////////////////////////////////////////////////

CIMClientResponseException::CIMClientResponseException(const String& message)
    : Exception(message)
{
}

PEGASUS_NAMESPACE_END
