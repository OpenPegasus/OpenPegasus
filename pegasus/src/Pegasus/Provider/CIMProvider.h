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

#ifndef Pegasus_CIMProvider_h
#define Pegasus_CIMProvider_h

#include <Pegasus/Common/Config.h>
#include <Pegasus/Common/OperationContext.h>
#include <Pegasus/Provider/CIMOMHandle.h>
#include <Pegasus/Provider/ProviderException.h>
#include <Pegasus/Common/ResponseHandler.h>
#include <Pegasus/Provider/Linkage.h>

PEGASUS_NAMESPACE_BEGIN

/**
    CIMProvider is the base class for the provider interface.  A provider
    must implement all the interfaces of the CIMProvider class and one or
    more of its subclasses (such as CIMInstanceProvider).  If a provider
    does not support any of the operations specified by the interfaces,
    it may just throw a CIMNotSupportedException.

    <p>Some of the parameters that are common to the provider operation
    interfaces are:

    <ul>
        <li>The OperationContext parameter contains information about the
        context for the specified operation, including the user name of the
        client requesting the operation.  It is the provider's responsibility
        to determine whether the user is authorized to perform the operation.
        If the operation should not be permitted, the provider must throw
        a CIMAccessDeniedException.</li>

        <li>A CIMObjectPath specifies the CIM object on which the operation
        is to be performed.  This parameter specifies the hostname, namespace,
        classname, and key values that uniquely identify a CIM instance.</li>

        <li>A ResponseHandler is a callback handle used by the provider to
        deliver response data to the CIM Server.</li>
    </ul>
*/
class PEGASUS_PROVIDER_LINKAGE CIMProvider
{
public:
    /**
        Default constructor for the abstract CIMProvider class.
    */
    CIMProvider();

    /**
        Destructs the CIMProvider instance.
    */
    virtual ~CIMProvider();

    /**
        Initializes the provider.  This method is called each time the
        provider is loaded and must complete before any of the other
        provider methods (other than terminate) are called.

        An exception thrown from this method indicates a provider
        initialization failure which prevents it from processing operation
        requests.

        @param cimom Reserved for future use.
    */
    virtual void initialize(CIMOMHandle & cimom) = 0;

    /**
        Performs any cleanup required before the provider is unloaded.
        This method may be called by the CIM Server at any time, including
        during initialization.  No other provider methods are called after
        this method is invoked.

        <p>Examples of cleanup a provider may perform in this method include:
        <ul>
            <li>Closing files and/or I/O streams</li>
            <li>Releasing resources such as shared memory</li>
            <li>Informing concurrently executing requests to complete
                immediately (perhaps by setting a global flag)</li>
            <li>Terminating subprocesses</li>
        </ul>

        <p>If the PegasusCreateProvider function dynamically instantiates the
        provider with the "new" operator, then the terminate() method must
        "delete" it:

            <pre>
            void MyProvider::terminate()
            {
                ...
                delete this;
            }
            </pre>

        <p>An exception thrown from this method is considered to be a provider
        error and is ignored.
    */
    virtual void terminate() = 0;
};

PEGASUS_NAMESPACE_END

#endif
