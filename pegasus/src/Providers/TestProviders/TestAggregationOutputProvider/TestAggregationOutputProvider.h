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
// Author: Aruran, IBM(ashanmug@in.ibm.com)
//
// Modified By:
//
//%/////////////////////////////////////////////////////////////////////////////

#ifndef Pegasus_TestAggregationOutputProvider_h
#define Pegasus_TestAggregationOutputProvider_h

#include <Pegasus/Common/Config.h>
#include <Pegasus/Provider/CIMInstanceProvider.h>
#include <Pegasus/Provider/CIMAssociationProvider.h>


PEGASUS_USING_PEGASUS;

class TestAggregationOutputProvider :
    public CIMInstanceProvider, public CIMAssociationProvider
{
public:
    TestAggregationOutputProvider(void);
    virtual ~TestAggregationOutputProvider(void);

    // CIMProvider interface
    virtual void initialize(CIMOMHandle & cimom);
    virtual void terminate(void);

    // CIMInstanceProvider interfaces
    virtual void getInstance(
        const OperationContext & context,
        const CIMObjectPath & ref,
        const Boolean includeQualifiers,
        const Boolean includeClassOrigin,
        const CIMPropertyList & propertyList,
        InstanceResponseHandler & handler);

    virtual void enumerateInstances(
        const OperationContext & context,
        const CIMObjectPath & ref,
        const Boolean includeQualifiers,
        const Boolean includeClassOrigin,
        const CIMPropertyList & propertyList,
        InstanceResponseHandler & handler);

    virtual void enumerateInstanceNames(
        const OperationContext & context,
        const CIMObjectPath & ref,
        ObjectPathResponseHandler & handler);

    // CIMAssociationProvider interface
    virtual void associators(
        const OperationContext & context,
        const CIMObjectPath & objectName,
        const CIMName & associationClass,
        const CIMName & resultClass,
        const String & role,
        const String & resultRole,
        const Boolean includeQualifiers,
        const Boolean includeClassOrigin,
        const CIMPropertyList & propertyList,
        ObjectResponseHandler & handler);

    virtual void associatorNames(
        const OperationContext & context,
        const CIMObjectPath & objectName,
        const CIMName & associationClass,
        const CIMName & resultClass,
        const String & role,
        const String & resultRole,
        ObjectPathResponseHandler & handler);

    virtual void references(
        const OperationContext & context,
        const CIMObjectPath & objectName,
        const CIMName & resultClass,
        const String & role,
        const Boolean includeQualifiers,
        const Boolean includeClassOrigin,
        const CIMPropertyList & propertyList,
        ObjectResponseHandler & handler);

    virtual void referenceNames(
        const OperationContext & context,
        const CIMObjectPath & objectName,
        const CIMName & resultClass,
        const String & role,
        ObjectPathResponseHandler & handler);

    virtual void modifyInstance(
        const OperationContext & context,
        const CIMObjectPath & ref,
        const CIMInstance & obj,
        const Boolean includeQualifiers,
        const CIMPropertyList & propertyList,
        ResponseHandler & handler);

    virtual void createInstance(
        const OperationContext & context,
        const CIMObjectPath & ref,
        const CIMInstance & obj,
        ObjectPathResponseHandler & handler);

    virtual void deleteInstance(
        const OperationContext & context,
        const CIMObjectPath & ref,
        ResponseHandler & handler);

protected:
    CIMOMHandle _cimomHandle;
    // save the class objects that we use
    CIMClass _personClass;
    CIMClass _teachesClass;
    CIMClass _worksClass;
    CIMClass _marriageClass;
    CIMClass _familyClass;

    // Built arrays of instances for each type
    // corresponds to referencedClass
    Array<CIMObjectPath> _instanceNames;
    Array<CIMInstance> _instances;

    Array<CIMObjectPath> _instanceNamesTeaches;
    Array<CIMInstance> _instancesTeaches;
    Array<CIMObjectPath> _instanceNamesWorks;
    Array<CIMInstance> _instancesWorks;
    Array<CIMObjectPath> _instanceNamesMarriage;
    Array<CIMInstance> _instancesMarriage;
    Array<CIMObjectPath> _instanceNamesFamily;
    Array<CIMInstance> _instancesFamily;


    // internal functions
    CIMClass _getClass(const CIMName& className);
    Boolean _initError;

private:

void _getInstance(
    const Array<CIMInstance> & instanceArray,
    const OperationContext & context,
    const CIMObjectPath & localReference,
    const Boolean includeQualifiers,
    const Boolean includeClassOrigin,
    const CIMPropertyList & propertyList,
    InstanceResponseHandler & handler);

void _enumerateInstances(
    const Array<CIMInstance> & instanceArray,
    const OperationContext & context,
    const CIMObjectPath & classReference,
    const Boolean includeQualifiers,
    const Boolean includeClassOrigin,
    const CIMPropertyList & propertyList,
    InstanceResponseHandler & handler);

void _enumerateInstanceNames(
    const Array<CIMInstance> & instanceArray,
    const OperationContext & context,
    const CIMObjectPath & classReference,
    ObjectPathResponseHandler & handler);

void _associators(
    Array<CIMInstance> & instanceArray,
    Array<CIMInstance> & resultInstanceArray,
    const OperationContext & context,
    const CIMObjectPath & localObjectName,
    const CIMName & associationClass,
    const CIMName & resultClass,
    const String & role,
    const String & resultRole,
    const Boolean includeQualifiers,
    const Boolean includeClassOrigin,
    const CIMPropertyList & propertyList,
    ObjectResponseHandler & handler);

void _associatorNames(
    Array<CIMInstance> & instanceArray,
    const OperationContext & context,
    const CIMObjectPath & localObjectName,
    const CIMName & associationClass,
    const CIMName & resultClass,
    const String & role,
    const String & resultRole,
    ObjectPathResponseHandler & handler);

void _references(
    Array<CIMInstance> & instanceArray,
    const OperationContext & context,
    const CIMObjectPath & objectName,
    const CIMName & resultClass,
    const String & role,
    const Boolean includeQualifiers,
    const Boolean includeClassOrigin,
    const CIMPropertyList & propertyList,
    ObjectResponseHandler & handler);

void _referenceNames(
    Array<CIMInstance> & instanceArray,
    const OperationContext & context,
    const CIMObjectPath & objectName,
    const CIMName & resultClass,
    const String & role,
    ObjectPathResponseHandler & handler);

void _deleteInstance(
    Array<CIMInstance> & instanceArray,
    Array<CIMObjectPath> & pathArray,
    const OperationContext & context,
    const CIMObjectPath & instanceReference,
    ResponseHandler & handler);


void _createInstance(
    Array<CIMInstance> & instanceArray,
    Array<CIMObjectPath> & pathArray,
    const OperationContext & context,
    const CIMObjectPath & instanceReference,
    const CIMInstance & instanceObject,
    ObjectPathResponseHandler & handler);

void _modifyInstance(
    Array<CIMInstance> & instanceArray,
    const OperationContext & context,
    const CIMObjectPath & localReference,
    const CIMInstance & instanceObject,
    const Boolean includeQualifiers,
    const CIMPropertyList & propertyList,
    ResponseHandler & handler);
};

#endif
