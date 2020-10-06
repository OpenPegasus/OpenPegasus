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
/* NOCHKSRC */

#ifndef Pegasus_ProviderException_h
#define Pegasus_ProviderException_h

#include <Pegasus/Common/Config.h>
#include <Pegasus/Common/Exception.h>
#include <Pegasus/Provider/Linkage.h>

PEGASUS_NAMESPACE_BEGIN

/**
Parent class for exceptions thrown by providers.

<p>The <tt>CIMOperationFailedException</tt> class is an exception class,
and the parent class from which exceptions that can be thrown
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

// l10n - added constructors with MessageLoaderParms

class PEGASUS_PROVIDER_LINKAGE CIMOperationFailedException
    : public CIMException
{
public:
    /**
    Generic operation failure.
    
    <p>This exception will cause a <tt>CIM_ERR_FAILED</tt>
    status code to be returned to the client.</p>
    */
    CIMOperationFailedException(const String & message);
    
    /*
    <p>This exception will cause a <tt>CIM_ERR_FAILED</tt>
    status code to be returned to the client.</p>
    */
#ifdef PEGASUS_USE_EXPERIMENTAL_INTERFACES
    /** <I><B>Experimental Interface</B></I><BR>
    */
    CIMOperationFailedException(const MessageLoaderParms & parms);    
#endif

protected:
    CIMOperationFailedException(const CIMStatusCode code, const String & message);
    
#ifdef PEGASUS_USE_EXPERIMENTAL_INTERFACES
    /** <I><B>Experimental Interface</B></I><BR>
    */
    CIMOperationFailedException(const CIMStatusCode code, const MessageLoaderParms & parms);    
#endif
};

/**
Cause a <tt>CIM_ERR_ACCESS_DENIED</tt> status code to be
returned to the client.
*/
class PEGASUS_PROVIDER_LINKAGE CIMAccessDeniedException
    : public CIMOperationFailedException
{
public:
    ///
    CIMAccessDeniedException(const String & message);
    
#ifdef PEGASUS_USE_EXPERIMENTAL_INTERFACES
    /** <I><B>Experimental Interface</B></I><BR>
    */
    CIMAccessDeniedException(const MessageLoaderParms & parms);    
#endif
};

/**
Cause a <tt>CIM_ERR_INVALID_PARAMETER</tt> status code to be
returned to the client.
*/
class PEGASUS_PROVIDER_LINKAGE CIMInvalidParameterException
    : public CIMOperationFailedException
{
public:
    ///
    CIMInvalidParameterException(const String & message);
    
#ifdef PEGASUS_USE_EXPERIMENTAL_INTERFACES
    /** <I><B>Experimental Interface</B></I><BR>
    */
    CIMInvalidParameterException(const MessageLoaderParms & parms);    
#endif
};

#if 0
/**
Cause a <tt>CIM_ERR_INVALID_CLASS</tt> status code to be
returned to the client.
*/
class PEGASUS_PROVIDER_LINKAGE CIMInvalidClassException
    : public CIMOperationFailedException
{
public:
    CIMInvalidClassException(const String & message);
    
#ifdef PEGASUS_USE_EXPERIMENTAL_INTERFACES
    /** <I><B>Experimental Interface</B></I><BR>
    */
    CIMInvalidClassException(const MessageLoaderParms & parms);    
#endif
};
#endif

/**
Cause a <tt>CIM_ERR_NOT_FOUND</tt> status code to be
returned to the client.
*/
class PEGASUS_PROVIDER_LINKAGE CIMObjectNotFoundException
    : public CIMOperationFailedException
{
public:
    ///
    CIMObjectNotFoundException(const String & message);
    
#ifdef PEGASUS_USE_EXPERIMENTAL_INTERFACES
    /** <I><B>Experimental Interface</B></I><BR>
    */
    CIMObjectNotFoundException(const MessageLoaderParms & parms); 
#endif   
};

/**
Cause a <tt>CIM_ERR_NOT_SUPPORTED</tt> status code to be
returned to the client.
*/
class PEGASUS_PROVIDER_LINKAGE CIMNotSupportedException
    : public CIMOperationFailedException
{
public:
    ///
    CIMNotSupportedException(const String & message);
    
#ifdef PEGASUS_USE_EXPERIMENTAL_INTERFACES
    /** <I><B>Experimental Interface</B></I><BR>
    */
    CIMNotSupportedException(const MessageLoaderParms & parms);   
#endif 
};

/**
Cause a <tt>CIM_ERR_ALREADY_EXISTS</tt> status code to be
returned to the client.
*/
class PEGASUS_PROVIDER_LINKAGE CIMObjectAlreadyExistsException
    : public CIMOperationFailedException
{
public:
    ///
    CIMObjectAlreadyExistsException(const String & message);
    
#ifdef PEGASUS_USE_EXPERIMENTAL_INTERFACES
    /** <I><B>Experimental Interface</B></I><BR>
    */
    CIMObjectAlreadyExistsException(const MessageLoaderParms & parms);  
#endif  
};

/**
Cause a <tt>CIM_ERR_NO_SUCH_PROPERTY</tt> status code to be
returned to the client.
*/
class PEGASUS_PROVIDER_LINKAGE CIMPropertyNotFoundException
    : public CIMOperationFailedException
{
public:
    ///
    CIMPropertyNotFoundException(const String & message);
    
#ifdef PEGASUS_USE_EXPERIMENTAL_INTERFACES
    /** <I><B>Experimental Interface</B></I><BR>
    */
    CIMPropertyNotFoundException(const MessageLoaderParms & parms); 
#endif   
};

#if 0
// Query operations are not yet supported in Pegasus
/**
Cause a <tt>CIM_ERR_INVALID_QUERY</tt> status code to be
returned to the client.
*/
class PEGASUS_PROVIDER_LINKAGE CIMInvalidQueryException
    : public CIMOperationFailedException
{
public:
    CIMInvalidQueryException(const String & message);
    
#ifdef PEGASUS_USE_EXPERIMENTAL_INTERFACES
    /** <I><B>Experimental Interface</B></I><BR>
    */
    CIMInvalidQueryException(const MessageLoaderParms & parms);    
#endif
};
#endif

/**
Cause a <tt>CIM_ERR_METHOD_NOT_FOUND</tt> status code to be
returned to the client.
*/
class PEGASUS_PROVIDER_LINKAGE CIMMethodNotFoundException
    : public CIMOperationFailedException
{
public:
    ///
    CIMMethodNotFoundException(const String & message);
    
#ifdef PEGASUS_USE_EXPERIMENTAL_INTERFACES
    /** <I><B>Experimental Interface</B></I><BR>
    */
    CIMMethodNotFoundException(const MessageLoaderParms & parms);    
#endif
};

PEGASUS_NAMESPACE_END

#endif
