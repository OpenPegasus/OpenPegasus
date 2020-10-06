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

#ifndef Pegasus_ProviderException_h
#define Pegasus_ProviderException_h

#include <Pegasus/Common/Config.h>
#include <Pegasus/Common/Exception.h>
#include <Pegasus/Provider/Linkage.h>

PEGASUS_NAMESPACE_BEGIN

/**
    Base class for exceptions thrown by providers.

    <p>The <tt>CIMOperationFailedException</tt> class is an exception class,
    and is the base class from which exceptions that can be thrown
    by providers are derived. It may also be thrown directly by
    providers to signal a generic operation failure.</p>

    <p>Providers do not throw every possible exception that clients
    may receive from the CIM Server. The exceptions which may be thrown
    by providers are a subset of the possible exceptions, and are
    described in their respective sections.</p>

    <p>All of the provider exceptions accept a <tt>message</tt>
    argument that allows the provider to send additional text
    in the string that will be returned to the client. While
    localization of text is not currently supported, it is
    recommended that text strings be structured in message
    catalogs to facilitate future localization.</p>
*/
class PEGASUS_PROVIDER_LINKAGE CIMOperationFailedException
    : public CIMException
{
public:
    /**
        Constructs a CIMOperationFailedException to indicate a generic
        operation failure.  This exception corresponds to the CIM_ERR_FAILED
        status code.
        @param message A message String containing an error description
    */
    CIMOperationFailedException(const String& message);

#ifdef PEGASUS_INTERNALONLY
    /*
    <p>This exception will cause a <tt>CIM_ERR_FAILED</tt>
    status code to be returned to the client.</p>
    */
    CIMOperationFailedException(const MessageLoaderParms& parms);
#endif

protected:
    /**
        Constructs a CIMOperationFailedException with a specified status code
        and error description.
        @param code A CIMStatusCode containing a DMTF defined status code
            specifying the type of the error
        @param message A message String containing an error description
    */
    CIMOperationFailedException(
        const CIMStatusCode code,
        const String& message);

#ifdef PEGASUS_INTERNALONLY
    CIMOperationFailedException(
        const CIMStatusCode code,
        const MessageLoaderParms& parms);
#endif
};

/**
    A CIMAccessDeniedException indicates an access permission error.
    This exception corresponds to the CIM_ERR_ACCESS_DENIED status code.
*/
class PEGASUS_PROVIDER_LINKAGE CIMAccessDeniedException
    : public CIMOperationFailedException
{
public:
    /**
        Constructs a CIMAccessDeniedException with a specified error message.
        @param message A message String containing an error description
    */
    CIMAccessDeniedException(const String& message);

#ifdef PEGASUS_INTERNALONLY
    CIMAccessDeniedException(const MessageLoaderParms& parms);
#endif
};

/**
    A CIMInvalidParameterException indicates an error with an operation
    parameter.  This exception corresponds to the CIM_ERR_INVALID_PARAMETER
    status code.
*/
class PEGASUS_PROVIDER_LINKAGE CIMInvalidParameterException
    : public CIMOperationFailedException
{
public:
    /**
        Constructs a CIMInvalidParameterException with a specified error
        message.
        @param message A message String containing an error description
    */
    CIMInvalidParameterException(const String& message);

#ifdef PEGASUS_INTERNALONLY
    CIMInvalidParameterException(const MessageLoaderParms& parms);
#endif
};

#if 0
/**
    A CIMInvalidClassException indicates a class was specified that does not
    exist.  This exception corresponds to the CIM_ERR_INVALID_CLASS status
    code.
*/
class PEGASUS_PROVIDER_LINKAGE CIMInvalidClassException
    : public CIMOperationFailedException
{
public:
    /**
        Constructs a CIMInvalidClassException with a specified error message.
        @param message A message String containing an error description
    */
    CIMInvalidClassException(const String& message);

#ifdef PEGASUS_INTERNALONLY
    CIMInvalidClassException(const MessageLoaderParms& parms);
#endif
};
#endif

/**
    A CIMObjectNotFoundException indicates that a requested object was not
    found.  This exception corresponds to the CIM_ERR_NOT_FOUND status code.
*/
class PEGASUS_PROVIDER_LINKAGE CIMObjectNotFoundException
    : public CIMOperationFailedException
{
public:
    /**
        Constructs a CIMObjectNotFoundException with a specified error message.
        @param message A message String containing an error description
    */
    CIMObjectNotFoundException(const String& message);

#ifdef PEGASUS_INTERNALONLY
    CIMObjectNotFoundException(const MessageLoaderParms& parms);
#endif
};

/**
    A CIMNotSupportedException indicates that a requested operation is not
    supported.  This exception corresponds to the CIM_ERR_NOT_SUPPORTED status
    code.
*/
class PEGASUS_PROVIDER_LINKAGE CIMNotSupportedException
    : public CIMOperationFailedException
{
public:
    /**
        Constructs a CIMNotSupportedException with a specified error message.
        @param message A message String containing an error description
    */
    CIMNotSupportedException(const String& message);

#ifdef PEGASUS_INTERNALONLY
    CIMNotSupportedException(const MessageLoaderParms& parms);
#endif
};

/**
    A CIMObjectAlreadyExistsException indicates that an operation could not be
    completed because a specified object already exists.  This exception
    corresponds to the CIM_ERR_ALREADY_EXISTS status code.
*/
class PEGASUS_PROVIDER_LINKAGE CIMObjectAlreadyExistsException
    : public CIMOperationFailedException
{
public:
    /**
        Constructs a CIMObjectAlreadyExistsException with a specified error
        message.
        @param message A message String containing an error description
    */
    CIMObjectAlreadyExistsException(const String& message);

#ifdef PEGASUS_INTERNALONLY
    CIMObjectAlreadyExistsException(const MessageLoaderParms& parms);
#endif
};

/**
    A CIMPropertyNotFoundException indicates that a property was specified
    which does not exist.  This exception corresponds to the
    CIM_ERR_NO_SUCH_PROPERTY status code.
*/
class PEGASUS_PROVIDER_LINKAGE CIMPropertyNotFoundException
    : public CIMOperationFailedException
{
public:
    /**
        Constructs a CIMPropertyNotFoundException with a specified error
        message.
        @param message A message String containing an error description
    */
    CIMPropertyNotFoundException(const String& message);

#ifdef PEGASUS_INTERNALONLY
    CIMPropertyNotFoundException(const MessageLoaderParms& parms);
#endif
};

#if 0
// Query operations are not yet supported in Pegasus
/**
    A CIMInvalidQueryException indicates that a query is not valid for a
    specified query language.  This exception corresponds to the
    CIM_ERR_INVALID_QUERY status code.
*/
class PEGASUS_PROVIDER_LINKAGE CIMInvalidQueryException
    : public CIMOperationFailedException
{
public:
    /**
        Constructs a CIMInvalidQueryException with a specified error message.
        @param message A message String containing an error description
    */
    CIMInvalidQueryException(const String& message);

#ifdef PEGASUS_INTERNALONLY
    CIMInvalidQueryException(const MessageLoaderParms& parms);
#endif
};
#endif

/**
    A CIMMethodNotFoundException indicates that a specified extrinsic method
    does not exist.  This exception corresponds to the CIM_ERR_METHOD_NOT_FOUND
    status code.
*/
class PEGASUS_PROVIDER_LINKAGE CIMMethodNotFoundException
    : public CIMOperationFailedException
{
public:
    /**
        Constructs a CIMMethodNotFoundException with a specified error message.
        @param message A message String containing an error description
    */
    CIMMethodNotFoundException(const String& message);

#ifdef PEGASUS_INTERNALONLY
    CIMMethodNotFoundException(const MessageLoaderParms& parms);
#endif
};

PEGASUS_NAMESPACE_END

#endif
