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
    This class defines a set of functions that support the invocation of
    extrinsic methods on a CIM class or instance.

    <p>A method provider is not required to implement all the methods defined
    for a CIM class.  Multiple method providers may be registered for methods
    of the same class, but not for the same method.  (See the
    SupportedMethods property of the PG_ProviderCapabilities class.)</p>
*/
class PEGASUS_PROVIDER_LINKAGE CIMMethodProvider : public virtual CIMProvider
{
public:
    /**
        Constructs a default CIMMethodProvider object.
    */
    CIMMethodProvider();

    /**
        Destructs a CIMMethodProvider object.
    */
    virtual ~CIMMethodProvider();

    /**
        Invokes an extrinsic specified method on a specified CIM class or
        instance.

        @param context An OperationContext object containing the context for
            the processing of the operation.  The context includes the name of
            the requesting user, language information, and other data.
        @param objectReference A fully qualified CIMObjectPath specifying
            the class or instance on which to invoke the method.
        @param methodName The name of the method to invoke.
        @param inParameters An Array of CIMParamValue objects specifying the
            method input parameters.
        @param handler ResponseHandler object for delivery of results.

        @exception CIMNotSupportedException If the method is not supported.
        @exception CIMInvalidParameterException If a parameter is invalid.
        @exception CIMObjectNotFoundException If the object is not found.
        @exception CIMAccessDeniedException If the user requesting the action
            is not authorized to perform the action.
        @exception CIMOperationFailedException If the operation fails.
    */
    virtual void invokeMethod(
        const OperationContext& context,
        const CIMObjectPath& objectReference,
        const CIMName& methodName,
        const Array<CIMParamValue>& inParameters,
        MethodResultResponseHandler& handler) = 0;
};

PEGASUS_NAMESPACE_END

#endif
