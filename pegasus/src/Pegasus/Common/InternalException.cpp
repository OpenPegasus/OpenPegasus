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
#include "InternalException.h"
#include "Constants.h"
#include <Pegasus/Common/CIMExceptionRep.h>
#include <Pegasus/Common/ContentLanguageList.h>
#include "Tracer.h"

PEGASUS_NAMESPACE_BEGIN

AssertionFailureException::AssertionFailureException(
    const char* file,
    size_t line,
    const String& message) : Exception(String::EMPTY)
{
    char lineStr[32];
    sprintf(lineStr, "%u", Uint32(line));

    _rep->message = file;
    _rep->message.append("(");
    _rep->message.append(lineStr);
    _rep->message.append("): ");
    _rep->message.append(message);

    PEG_TRACE_CSTRING(TRC_DISCARDED_DATA, Tracer::LEVEL1,
        (const char*)_rep->message.getCString());
}

AssertionFailureException::~AssertionFailureException()
{
}

const char NullPointer::MSG[] = "null pointer";
const char NullPointer::KEY[] = "Common.InternalException.NULL_POINTER";

const char UndeclaredQualifier::MSG[] = "undeclared qualifier: $0";
const char UndeclaredQualifier::KEY[] =
    "Common.InternalException.UNDECLARED_QUALIFIER";

const char BadQualifierScope::MSG[] =
    "qualifier invalid in this scope: $0 scope=$1";
const char BadQualifierScope::KEY[] =
    "Common.InternalException.BAD_QUALIFIER_SCOPE";

const char BadQualifierOverride::MSG[] = "qualifier not overridable: $0";
const char BadQualifierOverride::KEY[] =
    "Common.InternalException.BAD_QUALIFIER_OVERRIDE";

const char BadQualifierType::MSG[] =
    "CIMType of qualifier different than its declaration: $0";
const char BadQualifierType::KEY[] =
    "Common.InternalException.BAD_QUALIFIER_TYPE";

const char InstantiatedAbstractClass::MSG[] =
    "attempted to instantiate an abstract class $0";
const char InstantiatedAbstractClass::KEY[] =
    "Common.InternalException.INSTANTIATED_ABSTRACT_CLASS";

const char NoSuchProperty::MSG[] = "no such property: $0";
const char NoSuchProperty::KEY[] = "Common.InternalException.NO_SUCH_PROPERTY";

const char NoSuchFile::MSG[] = "no such file: $0";
const char NoSuchFile::KEY[] = "Common.InternalException.NO_SUCH_FILE";

const char FileNotReadable::MSG[] = "file not readable: $0";
const char FileNotReadable::KEY[] =
    "Common.InternalException.FILE_NOT_READABLE";

const char CannotRemoveDirectory::MSG[] = "cannot remove directory: $0";
const char CannotRemoveDirectory::KEY[] =
    "Common.InternalException.CANNOT_REMOVE_DIRECTORY";

const char CannotRemoveFile::MSG[] = "cannot remove file: $0";
const char CannotRemoveFile::KEY[] =
    "Common.InternalException.CANNOT_REMOVE_FILE";

const char CannotRenameFile::MSG[] = "cannot rename file: $0";
const char CannotRenameFile::KEY[] =
    "Common.InternalException.CANNOT_RENAME_FILE";

const char NoSuchDirectory::MSG[] = "no such directory: $0";
const char NoSuchDirectory::KEY[] =
    "Common.InternalException.NO_SUCH_DIRECTORY";

const char CannotCreateDirectory::MSG[] = "cannot create directory: $0";
const char CannotCreateDirectory::KEY[] =
    "Common.InternalException.CANNOT_CREATE_DIRECTORY";

const char CannotOpenFile::MSG[] = "cannot open file: $0";
const char CannotOpenFile::KEY[] = "Common.InternalException.CANNOT_OPEN_FILE";

const char CannotChangeFilePerm::MSG[] =
    "cannot change permissions of file: $0";
const char CannotChangeFilePerm::KEY[] =
    "Common.InternalException.CANNOT_CHANGE_FILE_PERM";

const char NotImplemented::MSG[] = "not implemented: $0";
const char NotImplemented::KEY[] = "Common.InternalException.NOT_IMPLEMENTED";

const char StackUnderflow::MSG[] = "stack underflow";
const char StackUnderflow::KEY[] = "Common.InternalException.STACK_UNDERFLOW";

const char StackOverflow::MSG[] = "stack overflow";
const char StackOverflow::KEY[] = "Common.InternalException.STACK_OVERFLOW";

const char DynamicLoadFailed::MSG[] = "load of dynamic library failed: $0";
const char DynamicLoadFailed::KEY[] =
    "Common.InternalException.DYNAMIC_LOAD_FAILED";

const char DynamicLookupFailed::MSG[] =
    "lookup of symbol in dynamic library failed: $0";
const char DynamicLookupFailed::KEY[] =
    "Common.InternalException.DYNAMIC_LOOKUP_FAILED";

const char CannotOpenDirectory::MSG[] = "cannot open directory: $0";
const char CannotOpenDirectory::KEY[] =
    "Common.InternalException.CANNOT_OPEN_DIRECTORY";

const char ParseError::MSG[] = "parse error: $0";
const char ParseError::KEY[] = "Common.InternalException.PARSE_ERROR";

const char MissingNullTerminator::MSG[] = "missing null terminator: $0";
const char MissingNullTerminator::KEY[] =
    "Common.InternalException.MISSING_NULL_TERMINATOR";

const char MalformedLanguageHeader::MSG[] = "malformed language header: $0";
const char MalformedLanguageHeader::KEY[] =
    "Common.InternalException.MALFORMED_LANGUAGE_HEADER";

const char InvalidAcceptLanguageHeader::MSG[] =
    "invalid acceptlanguage header: $0";
const char InvalidAcceptLanguageHeader::KEY[] =
    "Common.InternalException.INVALID_ACCEPTLANGUAGE_HEADER";

const char InvalidContentLanguageHeader::MSG[] =
    "invalid contentlanguage header: $0";
const char InvalidContentLanguageHeader::KEY[] =
    "Common.InternalException.INVALID_CONTENTLANGUAGE_HEADER";

const char InvalidAuthHeader::MSG[] = "Invalid Authorization header";
const char InvalidAuthHeader::KEY[] =
    "Common.InternalException.INVALID_AUTH_HEADER";

const char UnauthorizedAccess::MSG[] = "Unauthorized access";
const char UnauthorizedAccess::KEY[] =
    "Common.InternalException.UNAUTHORIZED_ACCESS";

const char InternalSystemError::MSG[] = "Unable to authenticate user";

const char SocketWriteError::MSG[] =  "Could not write response to client. "
                                      "Client may have timed out. "
                                      "Socket write failed with error: $0";
const char SocketWriteError::KEY[] =
    "Common.InternalException.SOCKET_WRITE_ERROR";

const char TooManyElementsException::KEY[]=
    "Common.InternalException.TOO_MANY_ELEMENTS";

const char TooManyElementsException::MSG[]=
    "More than $0 elements in a container are not supported.";

////////////////////////////////////////////////////////////////////////////////
//
// TraceableCIMException
//
////////////////////////////////////////////////////////////////////////////////

//
// Creates a description without source file name and line number.
//
static String _makeCIMExceptionDescription(
    CIMStatusCode code,
    const String& message)
{
    String tmp;
    tmp.append(cimStatusCodeToString(code));
    if (message != String::EMPTY)
    {
        tmp.append(": ");
        tmp.append(message);
    }
    return tmp;
}

//
// Creates a description without source file name and line number.
//
static String _makeCIMExceptionDescription(
    CIMStatusCode code,
    const String& message,
    ContentLanguageList& contentLanguages)
{
    String tmp;
    tmp = cimStatusCodeToString(code, contentLanguages);
    if (message != String::EMPTY)
    {
        tmp.append(": ");
        tmp.append(message);
    }
    return tmp;
}

//
// Creates a description without source file name and line number.
//
static String _makeCIMExceptionDescription(
    const String& cimMessage,
    const String& extraMessage)
{
    String tmp;
    tmp = cimMessage;
    if (extraMessage != String::EMPTY)
    {
        tmp.append(": ");
        tmp.append(extraMessage);
    }
    return tmp;
}

//
// Creates a description with source file name and line number.
//
static String _makeCIMExceptionDescription(
    CIMStatusCode code,
    const String& message,
    const String& file,
    Uint32 line)
{
    String tmp = file;
    tmp.append("(");
    char buffer[32];
    sprintf(buffer, "%u", line);
    tmp.append(buffer);
    tmp.append("): ");
    tmp.append(_makeCIMExceptionDescription(code, message));
    return tmp;
}

TraceableCIMException::TraceableCIMException(
    CIMStatusCode code,
    const MessageLoaderParms& parms,
    const String& file,
    Uint32 line)
    : CIMException(code, parms)
{
    CIMExceptionRep* rep;
    rep = reinterpret_cast<CIMExceptionRep*>(_rep);
    rep->file = file;
    rep->line = line;

    // Get the cim message from the code.  Ignore the content languages from
    // this operation, since the cimMessage string is only localized when the
    // code is invalid.
    cimStatusCodeToString_Thread(rep->cimMessage, code);
}

TraceableCIMException::TraceableCIMException(
    CIMStatusCode code,
    const String& message,
    const String& file,
    Uint32 line)
    : CIMException(code, message)
{
    CIMExceptionRep* rep;
    rep = reinterpret_cast<CIMExceptionRep*>(_rep);
    rep->file = file;
    rep->line = line;

    // Get the cim message from the code.  Ignore the content languages from
    // this operation, since the cimMessage string is only localized when the
    // code is invalid.
    cimStatusCodeToString_Thread(rep->cimMessage, code);
}

TraceableCIMException::TraceableCIMException(
    const ContentLanguageList& langs,
    CIMStatusCode code,
    const String& message,
    const String& file,
    Uint32 line)
    : CIMException(code, message)
{
    CIMExceptionRep* rep;
    rep = reinterpret_cast<CIMExceptionRep*>(_rep);
    rep->file = file;
    rep->line = line;

    rep->contentLanguages = langs;
}

TraceableCIMException::TraceableCIMException(const CIMException & cimException)
    : CIMException(cimException.getCode(), cimException.getMessage())
{
    TraceableCIMException * t = (TraceableCIMException *)&cimException;
    CIMExceptionRep* left;
    CIMExceptionRep* right;
    left = reinterpret_cast<CIMExceptionRep*>(_rep);
    right = reinterpret_cast<CIMExceptionRep*>(t->_rep);
    left->file = right->file;
    left->line = right->line;
    left->contentLanguages = right->contentLanguages;
    left->cimMessage = right->cimMessage;
}

//
// Returns a description string fit for human consumption
//
String TraceableCIMException::getDescription() const
{
#ifdef PEGASUS_DEBUG_CIMEXCEPTION
    return getTraceDescription();
#else
    CIMExceptionRep* rep;
    rep = reinterpret_cast<CIMExceptionRep*>(_rep);

    if (rep->cimMessage == String::EMPTY)
    {
        return _makeCIMExceptionDescription(
            rep->code, getMessage(), rep->contentLanguages);
    }
    else
    {
        return _makeCIMExceptionDescription(rep->cimMessage, getMessage());
    }
#endif
}

//
// Returns a description string with filename and line number information
// specifically for tracing.
//
String TraceableCIMException::getTraceDescription() const
{
    CIMExceptionRep* rep;
    rep = reinterpret_cast<CIMExceptionRep*>(_rep);
    String traceDescription =
        _makeCIMExceptionDescription(
            rep->code, getMessage(), rep->file, rep->line);

    return traceDescription;
}

String TraceableCIMException::getCIMMessage() const
{
    CIMExceptionRep* rep;
    rep = reinterpret_cast<CIMExceptionRep*>(_rep);
    return rep->cimMessage;
}

void TraceableCIMException::setCIMMessage(const String& cimMessage)
{
    CIMExceptionRep* rep;
    rep = reinterpret_cast<CIMExceptionRep*>(_rep);
    rep->cimMessage = cimMessage;
}

String TraceableCIMException::getFile() const
{
    CIMExceptionRep* rep;
    rep = reinterpret_cast<CIMExceptionRep*>(_rep);
    return rep->file;
}

Uint32 TraceableCIMException::getLine() const
{
    CIMExceptionRep* rep;
    rep = reinterpret_cast<CIMExceptionRep*>(_rep);
    return rep->line;
}

const ContentLanguageList& TraceableCIMException::getContentLanguages() const
{
    CIMExceptionRep* rep;
    rep = reinterpret_cast<CIMExceptionRep*>(_rep);
    return rep->contentLanguages;
}

//==============================================================================
//
// NullPointer
//
//==============================================================================

NullPointer::NullPointer()
    : Exception(MessageLoaderParms(
          NullPointer::KEY,
          NullPointer::MSG))
{
}

NullPointer::~NullPointer()
{
}

//==============================================================================
//
// UndeclaredQualifier
//
//==============================================================================

UndeclaredQualifier::UndeclaredQualifier(const String& qualifierName)
    : Exception(MessageLoaderParms(
          UndeclaredQualifier::KEY,
          UndeclaredQualifier::MSG,
          qualifierName))
{
}

UndeclaredQualifier::~UndeclaredQualifier()
{
}

//==============================================================================
//
// BadQualifierScope
//
//==============================================================================

BadQualifierScope::BadQualifierScope(
    const String& qualifierName,
    const String& scopeString)
    : Exception(MessageLoaderParms(KEY, MSG, qualifierName, scopeString))
{
}

BadQualifierScope::~BadQualifierScope()
{
}

//==============================================================================
//
// BadQualifierOverride
//
//==============================================================================

BadQualifierOverride::BadQualifierOverride(const String& qualifierName)
    : Exception(MessageLoaderParms(
          BadQualifierOverride::KEY,
          BadQualifierOverride::MSG,
          qualifierName))
{
}

BadQualifierOverride::~BadQualifierOverride()
{
}

//==============================================================================
//
// BadQualifierType
//
//==============================================================================
BadQualifierType::BadQualifierType(
    const String& qualifierName, const String& className):
        Exception(
            MessageLoaderParms(
                KEY,
                MSG,
                className.size()==0?qualifierName:
                    qualifierName+"(\""+className+"\")")),
        _qualifierName(qualifierName),
        _className(className)
{
}
const String& BadQualifierType:: getQualifierName() const
{
    return _qualifierName;
}
const String& BadQualifierType:: getClassName() const
{
    return _className;
}
BadQualifierType::~BadQualifierType()
{
}

//==============================================================================
//
// InstantiatedAbstractClass
//
//==============================================================================

InstantiatedAbstractClass::InstantiatedAbstractClass(const String& className)
    : Exception(MessageLoaderParms(
          InstantiatedAbstractClass::KEY,
          InstantiatedAbstractClass::MSG,
          className))
{
}

InstantiatedAbstractClass::~InstantiatedAbstractClass()
{
}

//==============================================================================
//
// NoSuchProperty
//
//==============================================================================

NoSuchProperty::NoSuchProperty(const String& propertyName)
    : Exception(MessageLoaderParms(
          NoSuchProperty::KEY,
          NoSuchProperty::MSG,
          propertyName))
{
}

NoSuchProperty::~NoSuchProperty()
{
}

//==============================================================================
//
// NoSuchFile
//
//==============================================================================

NoSuchFile::NoSuchFile(const String& fileName)
    : Exception(MessageLoaderParms(
          NoSuchFile::KEY,
          NoSuchFile::MSG,
          fileName))
{
}

NoSuchFile::~NoSuchFile()
{
}

//==============================================================================
//
// FileNotReadable
//
//==============================================================================

FileNotReadable::FileNotReadable(const String& fileName)
    : Exception(MessageLoaderParms(
          FileNotReadable::KEY,
          FileNotReadable::MSG,
          fileName))
{
}

FileNotReadable::~FileNotReadable()
{
}

//==============================================================================
//
// CannotRemoveDirectory
//
//==============================================================================

CannotRemoveDirectory::CannotRemoveDirectory(const String& arg)
    : Exception(MessageLoaderParms(
          CannotRemoveDirectory::KEY,
          CannotRemoveDirectory::MSG,
          arg))
{
}

CannotRemoveDirectory::~CannotRemoveDirectory()
{
}

//==============================================================================
//
// CannotRemoveFile
//
//==============================================================================

CannotRemoveFile::CannotRemoveFile(const String& path)
    : Exception(MessageLoaderParms(
          CannotRemoveFile::KEY,
          CannotRemoveFile::MSG,
          path))
{
}

CannotRemoveFile::~CannotRemoveFile()
{
}

//==============================================================================
//
// CannotRenameFile
//
//==============================================================================

CannotRenameFile::CannotRenameFile(const String& path)
    : Exception(MessageLoaderParms(
          CannotRenameFile::KEY,
          CannotRenameFile::MSG,
          path))
{
}

CannotRenameFile::~CannotRenameFile()
{
}

//==============================================================================
//
// NoSuchDirectory
//
//==============================================================================

NoSuchDirectory::NoSuchDirectory(const String& dirName)
    : Exception(MessageLoaderParms(
          NoSuchDirectory::KEY,
          NoSuchDirectory::MSG,
          dirName))
{
}

NoSuchDirectory::~NoSuchDirectory()
{
}

//==============================================================================
//
// CannotCreateDirectory
//
//==============================================================================

CannotCreateDirectory::CannotCreateDirectory(const String& path)
    : Exception(MessageLoaderParms(
          CannotCreateDirectory::KEY,
          CannotCreateDirectory::MSG,
          path))
{
}

CannotCreateDirectory::~CannotCreateDirectory()
{
}

//==============================================================================
//
// CannotOpenFile
//
//==============================================================================

CannotOpenFile::CannotOpenFile(const String& path)
    : Exception(MessageLoaderParms(
          CannotOpenFile::KEY,
          CannotOpenFile::MSG,
          path))
{
}

CannotOpenFile::~CannotOpenFile()
{
}

//==============================================================================
//
// CannotChangeFilePerm
//
//==============================================================================

CannotChangeFilePerm::CannotChangeFilePerm(const String& path)
    : Exception(MessageLoaderParms(
          CannotChangeFilePerm::KEY,
          CannotChangeFilePerm::MSG,
          path))
{
}

CannotChangeFilePerm::~CannotChangeFilePerm()
{
}

//==============================================================================
//
// NotImplemented
//
//==============================================================================

NotImplemented::NotImplemented(const String& method)
    : Exception(MessageLoaderParms(
          NotImplemented::KEY,
          NotImplemented::MSG,
          method))
{
}

NotImplemented::~NotImplemented()
{
}

//==============================================================================
//
// StackUnderflow
//
//==============================================================================

StackUnderflow::StackUnderflow()
    : Exception(MessageLoaderParms(StackUnderflow::KEY, StackUnderflow::MSG))
{
}

StackUnderflow::~StackUnderflow()
{
}

//==============================================================================
//
// StackOverflow
//
//==============================================================================

StackOverflow::StackOverflow()
    : Exception(MessageLoaderParms(StackOverflow::KEY, StackOverflow::MSG))
{
}

StackOverflow::~StackOverflow()
{
}

//==============================================================================
//
// DynamicLoadFailed
//
//==============================================================================

DynamicLoadFailed::DynamicLoadFailed(const String& path)
    : Exception(MessageLoaderParms(
          DynamicLoadFailed::KEY,
          DynamicLoadFailed::MSG,
          path))
{
}

DynamicLoadFailed::~DynamicLoadFailed()
{
}

//==============================================================================
//
// DynamicLookupFailed
//
//==============================================================================

DynamicLookupFailed::DynamicLookupFailed(const String& symbol)
    : Exception(MessageLoaderParms(
          DynamicLookupFailed::KEY,
          DynamicLookupFailed::MSG,
          symbol))
{
}

DynamicLookupFailed::~DynamicLookupFailed()
{
}

//==============================================================================
//
// CannotOpenDirectory
//
//==============================================================================

CannotOpenDirectory::CannotOpenDirectory(const String& path)
    : Exception(MessageLoaderParms(
          CannotOpenDirectory::KEY,
          CannotOpenDirectory::MSG,
          path))
{
}

CannotOpenDirectory::~CannotOpenDirectory()
{
}

//==============================================================================
//
// ParseError
//
//==============================================================================

ParseError::ParseError(const String& message)
    : Exception(MessageLoaderParms(
          ParseError::KEY,
          ParseError::MSG,
          message))
{
}

ParseError::~ParseError()
{
}

//==============================================================================
//
// MissingNullTerminator
//
//==============================================================================

MissingNullTerminator::MissingNullTerminator()
    : Exception(MessageLoaderParms(
          MissingNullTerminator::KEY,
          MissingNullTerminator::MSG))
{
}

MissingNullTerminator::~MissingNullTerminator()
{
}

//==============================================================================
//
// MalformedLanguageHeader
//
//==============================================================================

MalformedLanguageHeader::MalformedLanguageHeader(const String& error)
    : Exception(MessageLoaderParms(
          MalformedLanguageHeader::KEY,
          MalformedLanguageHeader::MSG,
          error))
{
}

MalformedLanguageHeader::~MalformedLanguageHeader()
{
}

//==============================================================================
//
// InvalidAcceptLanguageHeader
//
//==============================================================================

InvalidAcceptLanguageHeader::InvalidAcceptLanguageHeader(const String& error)
    : Exception(MessageLoaderParms(
          InvalidAcceptLanguageHeader::KEY,
          InvalidAcceptLanguageHeader::MSG,
          error))
{
}

InvalidAcceptLanguageHeader::~InvalidAcceptLanguageHeader()
{
}

//==============================================================================
//
// InvalidContentLanguageHeader
//
//==============================================================================

InvalidContentLanguageHeader::InvalidContentLanguageHeader(const String& error)
    : Exception(MessageLoaderParms(
          InvalidContentLanguageHeader::KEY,
          InvalidContentLanguageHeader::MSG,
          error))
{
}

InvalidContentLanguageHeader::~InvalidContentLanguageHeader()
{
}

//==============================================================================
//
// InvalidAuthHeader
//
//==============================================================================

InvalidAuthHeader::InvalidAuthHeader()
    : Exception(MessageLoaderParms(
          InvalidAuthHeader::KEY,
          InvalidAuthHeader::MSG))
{
}

InvalidAuthHeader::~InvalidAuthHeader()
{
}

//==============================================================================
//
// UnauthorizedAccess
//
//==============================================================================

UnauthorizedAccess::UnauthorizedAccess()
    : Exception(MessageLoaderParms(
          UnauthorizedAccess::KEY,
          UnauthorizedAccess::MSG))
{
}

UnauthorizedAccess::~UnauthorizedAccess()
{
}

//==============================================================================
//
// IncompatibleTypesException
//
//==============================================================================

IncompatibleTypesException::IncompatibleTypesException()
    : Exception("incompatible types")
{
}

IncompatibleTypesException::~IncompatibleTypesException()
{
}

//==============================================================================
//
// InternalSystemError
//
//==============================================================================

InternalSystemError::InternalSystemError()
    : Exception(InternalSystemError::MSG)
{
}

InternalSystemError::~InternalSystemError()
{
}

//==============================================================================
//
// SocketWriteError
//
//==============================================================================

SocketWriteError::SocketWriteError(const String& error)
    : Exception(MessageLoaderParms(
          SocketWriteError::KEY,
          SocketWriteError::MSG,
          error))
{
}

SocketWriteError::~SocketWriteError()
{
}

//==============================================================================
//
// TooManyHTTPHeadersException
//   - used by HTTPAuthenticatorDelegator to report detection of more than
//     PEGASUS_MAXELEMENTS_NUM HTTP header fields in a single HTTP message
//==============================================================================
TooManyHTTPHeadersException::TooManyHTTPHeadersException()
    : Exception("more than " PEGASUS_MAXELEMENTS
                    " header fields detected in HTTP message")
{
}

TooManyHTTPHeadersException::~TooManyHTTPHeadersException()
{
}

//==============================================================================
//
// TooManyElementsException
//   - used by OrderedSet to report detection of more than
//     PEGASUS_MAXELEMENTS_NUM elements in a single object
//==============================================================================
TooManyElementsException::TooManyElementsException()
    : Exception(MessageLoaderParms(
          TooManyElementsException::KEY,
          TooManyElementsException::MSG,
          PEGASUS_MAXELEMENTS_NUM))
{
}

TooManyElementsException::~TooManyElementsException()
{
}

void ThrowTooManyElementsException()
{
    throw TooManyElementsException();
}

void ThrowIndexOutOfBoundsException()
{
    throw IndexOutOfBoundsException();
}

void ThrowUninitializedObjectException()
{
    throw UninitializedObjectException();
}

void ThrowCannotOpenFileException(const char* path)
{
    throw CannotOpenFile(path);
}

PEGASUS_NAMESPACE_END
