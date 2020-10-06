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
#include "Exception.h"
#include <Pegasus/Common/ExceptionRep.h>
#include <Pegasus/Common/CIMExceptionRep.h>
#include "Tracer.h"

PEGASUS_NAMESPACE_BEGIN

Exception::Exception(const String& message)
{
    _rep = new ExceptionRep();
    _rep->message = message;
    _rep->contentLanguages.clear();
}

Exception::Exception(const Exception& exception)
{
    _rep = new ExceptionRep();
    _rep->message = exception._rep->message;
    _rep->contentLanguages = exception._rep->contentLanguages;
}

Exception::Exception(const MessageLoaderParms& msgParms)
{
    _rep = new ExceptionRep();
    _rep->message = MessageLoader::getMessage(
        const_cast<MessageLoaderParms &>(msgParms));
    // Must be after MessageLoader::getMessage call
    _rep->contentLanguages = msgParms.contentlanguages;
}

Exception::Exception()
{
    _rep = NULL;
}

Exception::~Exception()
{
    delete _rep;
}

Exception& Exception::operator=(const Exception& exception)
{
    if (&exception != this)
    {
        *this->_rep = *exception._rep;
    }
    return *this;
}

const String& Exception::getMessage() const
{
    return _rep->message;
}

const ContentLanguageList& Exception::getContentLanguages() const
{
    return _rep->contentLanguages;
}

void Exception::setContentLanguages(const ContentLanguageList& langs)
{
    _rep->contentLanguages = langs;
}

IndexOutOfBoundsException::IndexOutOfBoundsException()
    : Exception(MessageLoaderParms(
          "Common.Exception.INDEX_OUT_OF_BOUNDS_EXCEPTION",
          "index out of bounds"))
{
}

AlreadyExistsException::AlreadyExistsException(const String& message)
    : Exception(MessageLoaderParms(
          "Common.Exception.ALREADY_EXISTS_EXCEPTION",
          "already exists: $0",
          message))
{
}

AlreadyExistsException::AlreadyExistsException(MessageLoaderParms& msgParms)
    : Exception(MessageLoaderParms(
          "Common.Exception.ALREADY_EXISTS_EXCEPTION",
          "already exists: $0",
          MessageLoader::getMessage(msgParms)))
{
}

InvalidNameException::InvalidNameException(const String& message)
    : Exception(MessageLoaderParms(
          "Common.Exception.INVALID_NAME_EXCEPTION",
          "The CIM name is not valid: $0",
          message))
{
}

InvalidNameException::InvalidNameException(MessageLoaderParms& msgParms)
    : Exception(MessageLoaderParms(
          "Common.Exception.INVALID_NAME_EXCEPTION",
          "invalid CIM name: $0",
          MessageLoader::getMessage(msgParms)))
{
}

InvalidNamespaceNameException::InvalidNamespaceNameException(const String& name)
    : Exception(MessageLoaderParms(
          "Common.Exception.INVALID_NAMESACE_NAME_EXCEPTION",
          "invalid CIM namespace name: $0",
          name))
{
}

InvalidNamespaceNameException::InvalidNamespaceNameException(
    MessageLoaderParms& msgParms)
    : Exception(MessageLoaderParms(
          "Common.Exception.INVALID_NAMESPACE_NAME_EXCEPTION",
          "invalid CIM namespace name: $0",
          MessageLoader::getMessage(msgParms)))
{
}

UninitializedObjectException::UninitializedObjectException()
    : Exception(MessageLoaderParms(
          "Common.Exception.UNINITIALIZED_OBJECT_EXCEPTION",
          "uninitialized object"))
{
}

TypeMismatchException::TypeMismatchException()
    : Exception(MessageLoaderParms(
          "Common.Exception.TYPE_MISMATCH_EXCEPTION",
          "type mismatch"))
{
}

TypeMismatchException::TypeMismatchException(const String& message)
    : Exception(MessageLoaderParms(
          "Common.Exception.TYPE_MISMATCH_EXCEPTION",
          "type mismatch: $0",message))
{

}

TypeMismatchException::TypeMismatchException(MessageLoaderParms& msgParms)
    : Exception(MessageLoaderParms(
          "Common.Exception.TYPE_MISMATCH_EXCEPTION",
          "type mismatch: $0",
          MessageLoader::getMessage(msgParms)))
{
}

DynamicCastFailedException::DynamicCastFailedException()
    : Exception(MessageLoaderParms(
          "Common.Exception.DYNAMIC_CAST_FAILED_EXCEPTION",
          "dynamic cast failed"))
{
}

InvalidDateTimeFormatException::InvalidDateTimeFormatException()
    : Exception(MessageLoaderParms(
          "Common.Exception.INVALID_DATETIME_FORMAT_EXCEPTION",
          "invalid datetime format"))
{
}

MalformedObjectNameException::MalformedObjectNameException(
    const String& message)
    : Exception(MessageLoaderParms(
          "Common.Exception.MALFORMED_OBJECT_NAME_EXCEPTION",
          "malformed object name: $0",
          message))
{
}

MalformedObjectNameException::MalformedObjectNameException(
    MessageLoaderParms& msgParms)
    : Exception(MessageLoaderParms(
          "Common.Exception.MALFORMED_OBJECT_NAME_EXCEPTION",
          "malformed object name: $0",
          MessageLoader::getMessage(msgParms)))
{
}

BindFailedException::BindFailedException(const String& message)
    : Exception(MessageLoaderParms(
          "Common.Exception.BIND_FAILED_EXCEPTION",
          "Bind failed: $0",
          message))
{
}

BindFailedException::BindFailedException(MessageLoaderParms& msgParms)
    : Exception(MessageLoaderParms(
          "Common.Exception.BIND_FAILED_EXCEPTION",
          "Bind failed: $0",
          MessageLoader::getMessage(msgParms)))
{
}

InvalidLocatorException::InvalidLocatorException(const String& message)
    : Exception(MessageLoaderParms(
          "Common.Exception.INVALID_LOCATOR_EXCEPTION",
          "Invalid locator: $0",
          message))
{
}

InvalidLocatorException::InvalidLocatorException(MessageLoaderParms& msgParms)
    : Exception(MessageLoaderParms(
          "Common.Exception.INVALID_LOCATOR_EXCEPTION",
          "Invalid locator: $0",
          MessageLoader::getMessage(msgParms)))
{
}

CannotCreateSocketException::CannotCreateSocketException()
    : Exception(MessageLoaderParms(
          "Common.Exception.CANNOT_CREATE_SOCKET_EXCEPTION",
          "Cannot create socket"))
{
}


CannotConnectException::CannotConnectException(const String& message)//???
    : Exception(message)
{
}


CannotConnectException::CannotConnectException(MessageLoaderParms& msgParms)
    : Exception(msgParms)
{
}

AlreadyConnectedException::AlreadyConnectedException()
    : Exception(MessageLoaderParms(
          "Common.Exception.ALREADY_CONNECTED_EXCEPTION",
          "already connected"))
{
}


NotConnectedException::NotConnectedException()
    : Exception(MessageLoaderParms(
          "Common.Exception.NOT_CONNECTED_EXCEPTION",
          "not connected"))
{
}

ConnectionTimeoutException::ConnectionTimeoutException()
    : Exception(MessageLoaderParms(
          "Common.Exception.CONNECTION_TIMEOUT_EXCEPTION",
          "connection timed out"))
{
}
// EXP_PULL_BEGIN
InvalidEnumerationContextException::InvalidEnumerationContextException()
    : Exception(MessageLoaderParms(
          "Common.Exception.INVALID_ENUMERATION_CONTEXT_EXCEPTION",
          "Invalid Enumeration Context, uninitilialized"))
{
}
// EXP_PULL_END

SSLException::SSLException(const String& message)
    : Exception(MessageLoaderParms(
          "Common.Exception.SSL_EXCEPTION",
          "SSL Exception: $0" ,
          message))
{
}

SSLException::SSLException(MessageLoaderParms& msgParms)
    : Exception(MessageLoaderParms(
          "Common.Exception.SSL_EXCEPTION",
          "SSL Exception: $0",
          MessageLoader::getMessage(msgParms)))
{
}

DateTimeOutOfRangeException::DateTimeOutOfRangeException(const String& message)
    : Exception(MessageLoaderParms(
          "Common.Exception.DATETIME_OUT_OF_RANGE_EXCEPTION",
          "DateTime is out of range : $0" ,
          message))
{
}

DateTimeOutOfRangeException::DateTimeOutOfRangeException(
    MessageLoaderParms& msgParms)
    : Exception(MessageLoaderParms(
          "Common.Exception.DATETIME_OUT_OF_RANGE_EXCEPTION",
          "DateTime is out of range : $0",
          MessageLoader::getMessage(msgParms)))
{
}

////////////////////////////////////////////////////////////////////////////////
//
// CIMException
//
////////////////////////////////////////////////////////////////////////////////

// l10n - note - use this when you have an exception with no
// detail message, or one with an untranslated detail message
// The pegasus message associated with code will be translated.
CIMException::CIMException(
    CIMStatusCode code,
    const String& message)
    : Exception()
{
    CIMExceptionRep * tmp = new CIMExceptionRep ();
    tmp->message = message;
    tmp->code = code;
    tmp->file = "";
    tmp->line = 0;
    tmp->contentLanguages.clear();
    tmp->cimMessage = String::EMPTY;
    _rep = tmp;
}


// l10n - note - use this when you have an exception
// an untranslated detail message and an attached CIM_Error
// The pegasus message associated with code will be translated.
CIMException::CIMException(
    CIMStatusCode code,
    const String& message,
    const CIMInstance& instance)
    : Exception()
{
    CIMExceptionRep * tmp = new CIMExceptionRep ();
    tmp->message = message;
    tmp->code = code;
    tmp->file = "";
    tmp->errors.append(instance);
    tmp->line = 0;
    tmp->contentLanguages.clear();
    tmp->cimMessage = String::EMPTY;
    _rep = tmp;
}

// l10n - note - use this when you have an exception
// an untranslated detail message and an attached CIM_Error
// array
// The pegasus message associated with code will be translated.
CIMException::CIMException(
    CIMStatusCode code,
    const String& message,
    const Array<CIMInstance>& instances)
    : Exception()
{
    CIMExceptionRep * tmp = new CIMExceptionRep ();
    tmp->message = message;
    tmp->code = code;
    tmp->file = "";
    tmp->errors.appendArray(instances);
    tmp->line = 0;
    tmp->contentLanguages.clear();
    tmp->cimMessage = String::EMPTY;
    _rep = tmp;
}
// l10n - note use this when you have an exception with a translated
// detail message
CIMException::CIMException(
    CIMStatusCode code,
    const MessageLoaderParms& msgParms)
    : Exception()
{
    CIMExceptionRep* tmp = new CIMExceptionRep();
    tmp->message = MessageLoader::getMessage(
        const_cast<MessageLoaderParms &>(msgParms));
    // Must be after MessageLoader::getMessage call
    tmp->contentLanguages = msgParms.contentlanguages;
    tmp->cimMessage = String::EMPTY;
    tmp->code = code;
    tmp->file = "";
    tmp->line = 0;
    _rep = tmp;
}

CIMException::CIMException(
    CIMStatusCode code,
    const MessageLoaderParms& msgParms,
    const CIMInstance& instance)
    : Exception()
{
    CIMExceptionRep* tmp = new CIMExceptionRep();
    tmp->message = MessageLoader::getMessage(
        const_cast<MessageLoaderParms &>(msgParms));
    // Must be after MessageLoader::getMessage call
    tmp->contentLanguages = msgParms.contentlanguages;
    tmp->cimMessage = String::EMPTY;
    tmp->errors.append(instance);
    tmp->code = code;
    tmp->file = "";
    tmp->line = 0;
    _rep = tmp;
}

CIMException::CIMException(
    CIMStatusCode code,
    const MessageLoaderParms& msgParms,
    const Array<CIMInstance>& instances)
    : Exception()
{
    CIMExceptionRep* tmp = new CIMExceptionRep();
    tmp->message = MessageLoader::getMessage(
        const_cast<MessageLoaderParms &>(msgParms));
    // Must be after MessageLoader::getMessage call
    tmp->contentLanguages = msgParms.contentlanguages;
    tmp->cimMessage = String::EMPTY;
    tmp->errors.appendArray(instances);
    tmp->code = code;
    tmp->file = "";
    tmp->line = 0;
    _rep = tmp;
}

CIMException::CIMException(const CIMException & cimException)
    : Exception()
{
    _rep = new CIMExceptionRep(
        *reinterpret_cast<CIMExceptionRep*>(cimException._rep));
}

CIMException& CIMException::operator=(const CIMException & cimException)
{
    if (&cimException != this)
    {
        CIMExceptionRep* left = reinterpret_cast<CIMExceptionRep*>(this->_rep);
        CIMExceptionRep* right =
            reinterpret_cast<CIMExceptionRep*>(cimException._rep);
        *left = *right;
    }
    return *this;
}

CIMException::~CIMException()
{
}

Uint32 CIMException::getErrorCount() const
{
    return reinterpret_cast<CIMExceptionRep*>(_rep)->errors.size();
}
/**************
CIMInstance CIMException::getError(Uint32 index)
{
    return reinterpret_cast<CIMExceptionRep*>(_rep)->errors[index];
}
***************/

CIMConstInstance CIMException::getError(Uint32 index) const
{
    return reinterpret_cast<CIMExceptionRep*>(_rep)->errors[index];
}

void CIMException::addError(const CIMInstance& instance)
{
    reinterpret_cast<CIMExceptionRep*>(_rep)->errors.append(instance);
}

CIMStatusCode CIMException::getCode() const
{
    CIMExceptionRep* rep;
    rep = reinterpret_cast<CIMExceptionRep*>(_rep);
    return rep->code;
}

PEGASUS_NAMESPACE_END
