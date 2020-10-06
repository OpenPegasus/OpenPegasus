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

//
// Author: Chip Vincent (cvincent@us.ibm.com)
//
// Modified By:
//
//%/////////////////////////////////////////////////////////////////////////////

#ifndef Pegasus_CIMMethodProvider_h
#define Pegasus_CIMMethodProvider_h

#include <Pegasus/Common/Config.h>
#include <Pegasus/Provider/CIMProvider.h>

#include <Pegasus/Common/Array.h>
#include <Pegasus/Common/CIMName.h>
#include <Pegasus/Common/CIMObjectPath.h>
#include <Pegasus/Common/CIMValue.h>
#include <Pegasus/Common/CIMParamValue.h>
#include <Pegasus/Provider/Linkage.h>

PEGASUS_NAMESPACE_BEGIN

/**
Functions that support the invocation of methods defined
for a CIM class.

<p>The <i>Method Provider</i> supports the client InvokeMethod
operation that invokes any method(s)
defined on a CIM class of object. In addition to the
methods from the {@link CIMProvider CIMProvider}
interface that the functions inherit, the Method Provider interface
defines one function:</p>

<ul>
<li>{@link invokeMethod invokeMethod}</li>
</ul>

<p>The arguments to <tt>invokeMethod</tt> specify the instance
that the method invokes, the method name, and its
parameters.</p>

<p>Providers that derive from this class <i>must</i> implement
all of these functions. A minimal implementation of <tt>invokeMethod</tt>
may throw a {@link CIMNotSupportedException CIMNotSupportedException} exception.</p>

<p>A method provider is not required to implement all of the
CIM methods defined for a class; there can be more than one
method provider for a class. Each provider
may implement a subset of the defined methods, leaving other
methods providers to implemente methods. No method
can be implemented by more than one provider. The
methods that are implemented by a provider must be specified to
the CIM Server through provider registration in the
<tt>SupportedMethods</tt> property of an instance of the
<tt>PG_ProviderCapabilities</tt> class.</p>
*/
class PEGASUS_PROVIDER_LINKAGE CIMMethodProvider : public virtual CIMProvider
{
public:
    /** Constructs a CIMMethodProvider object with null values (default constructor).
	*/
    CIMMethodProvider(void);
    /** CIMMethodProvider destructor.
	*/
    virtual ~CIMMethodProvider(void);

    /**
    Invoke the specified method on the specified instance.

    <p>Instructs the provider to invoke the method specified in the
    <tt>methodName</tt> parameter on the object
    specified in the <tt>objectReference</tt> parameter.

    @param context Specifies the client user's context for this operation,
    including the user ID.

    @param objectReference Specifies the fully qualified object path
    of the class or instance of interest.

    @param methodName Specifies the name of the method of interest.

    @param inParameters Specifies the input parameters of the method.

    @param handler A {@link ResponseHandler ResponseHandler} object used
    to deliver results to the CIM Server.

    @exception CIMNotSupportedException If the method is not supported.
    @exception CIMInvalidParameterException If the parameter is invalid.
    @exception CIMObjectNotFoundException If the object is not found.
    @exception CIMAccessDeniedException If the user requesting the action 
    is not authorized to perform the action.
    @exception CIMOperationFailedException If the operation fails.
    */
    virtual void invokeMethod(
	const OperationContext & context,
	const CIMObjectPath & objectReference,
	const CIMName & methodName,
	const Array<CIMParamValue> & inParameters,
	MethodResultResponseHandler & handler) = 0;

};

PEGASUS_NAMESPACE_END

#endif
