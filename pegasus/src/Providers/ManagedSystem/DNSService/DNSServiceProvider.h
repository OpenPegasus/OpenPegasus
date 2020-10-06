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

#ifndef Pegasus_DNSServiceProvider_h
#define Pegasus_DNSServiceProvider_h

#include <Pegasus/Common/Config.h>
#include <Pegasus/Provider/CIMInstanceProvider.h>
#include <Pegasus/Provider/CIMMethodProvider.h>
#include <Pegasus/Common/OperationContext.h>
#include <Pegasus/Common/Pair.h>
#include "DNSService.h"

PEGASUS_USING_PEGASUS;
PEGASUS_USING_STD;

Boolean DNSFileOk();

class DNSServiceProvider :
    public CIMInstanceProvider
{
public:
    DNSServiceProvider();
    virtual ~DNSServiceProvider();

    //-- CIMInstanceProvider methods
    /** Given a reference to an instance of the CIM class, fills in the data
     *  elements of the class with the details gleaned from the system. */
    void getInstance(
        const OperationContext & context,
        const CIMObjectPath & ref,
        const Boolean includeQualifiers,
        const Boolean includeClassOrigin,
        const CIMPropertyList & propertyList,
        InstanceResponseHandler & handler);

    /** Returns filled instances for all instances of the CIM class detected
     *  on the system. */
    void enumerateInstances(
        const OperationContext & context,
        const CIMObjectPath & ref,
        const Boolean includeQualifiers,
        const Boolean includeClassOrigin,
        const CIMPropertyList & propertyList,
        InstanceResponseHandler & handler);

    /** Produces a list of references to all instances of the CIM class
     *  detected on the system, but does not fill the instances
     *  themselves. */
    void enumerateInstanceNames(
        const OperationContext & context,
        const CIMObjectPath & ref,
        ObjectPathResponseHandler & handler);

    /** Currently unimplemented in the provider, this is a no-op
     *  here. */
    void modifyInstance(
        const OperationContext & context,
        const CIMObjectPath & ref,
        const CIMInstance & obj,
        const Boolean includeQualifiers,
        const CIMPropertyList & propertyList,
        ResponseHandler & handler);

    /** Currently unimplemented in the provider, this is a no-op
     *  here. */
    void createInstance(
        const OperationContext & context,
        const CIMObjectPath & ref,
        const CIMInstance & obj,
        ObjectPathResponseHandler & handler);

    /** Currently unimplemented in the provider, this is a no-op
     *  here. */
    void deleteInstance(
        const OperationContext & context,
        const CIMObjectPath & ref,
        ResponseHandler & handler);

    void initialize(CIMOMHandle & cimom);
    void terminate();

private:
    // Builds a reference (a set of Key,Value pairs)
    CIMObjectPath _fill_reference(
        const CIMNamespaceName &nameSpace,
        const CIMName &className,
        DNSService dns);
    // Builds a filled-in instance.
    CIMInstance _build_instance(
        const CIMName & classname,
        const CIMNamespaceName & nameSpace,
        const Array<CIMKeyBinding> keys,
        DNSService dns);
};

#endif
