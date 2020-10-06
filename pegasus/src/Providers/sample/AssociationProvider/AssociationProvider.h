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

#ifndef Pegasus_AssociationProvider_h
#define Pegasus_AssociationProvider_h

#include <Pegasus/Common/Config.h>
#include <Pegasus/Provider/CIMInstanceProvider.h>
#include <Pegasus/Provider/CIMAssociationProvider.h>

class AssociationProvider:
    public PEGASUS_NAMESPACE(CIMInstanceProvider),
    public PEGASUS_NAMESPACE(CIMAssociationProvider)
{
public:
    AssociationProvider();
    virtual ~AssociationProvider();

    // CIMProvider interface
    virtual void initialize(PEGASUS_NAMESPACE(CIMOMHandle)& cimom);
    virtual void terminate();

    // CIMInstanceProvider interfaces
    virtual void getInstance(
        const PEGASUS_NAMESPACE(OperationContext)& context,
        const PEGASUS_NAMESPACE(CIMObjectPath)& ref,
        const PEGASUS_NAMESPACE(Boolean) includeQualifiers,
        const PEGASUS_NAMESPACE(Boolean) includeClassOrigin,
        const PEGASUS_NAMESPACE(CIMPropertyList)& propertyList,
        PEGASUS_NAMESPACE(InstanceResponseHandler)& handler);

    virtual void enumerateInstances(
        const PEGASUS_NAMESPACE(OperationContext)& context,
        const PEGASUS_NAMESPACE(CIMObjectPath)& ref,
        const PEGASUS_NAMESPACE(Boolean) includeQualifiers,
        const PEGASUS_NAMESPACE(Boolean) includeClassOrigin,
        const PEGASUS_NAMESPACE(CIMPropertyList)& propertyList,
        PEGASUS_NAMESPACE(InstanceResponseHandler)& handler);

    virtual void enumerateInstanceNames(
        const PEGASUS_NAMESPACE(OperationContext)& context,
        const PEGASUS_NAMESPACE(CIMObjectPath)& ref,
        PEGASUS_NAMESPACE(ObjectPathResponseHandler)& handler);

    virtual void modifyInstance(
        const PEGASUS_NAMESPACE(OperationContext)& context,
        const PEGASUS_NAMESPACE(CIMObjectPath)& ref,
        const PEGASUS_NAMESPACE(CIMInstance)& obj,
        const PEGASUS_NAMESPACE(Boolean) includeQualifiers,
        const PEGASUS_NAMESPACE(CIMPropertyList)& propertyList,
        PEGASUS_NAMESPACE(ResponseHandler)& handler);

    virtual void createInstance(
        const PEGASUS_NAMESPACE(OperationContext)& context,
        const PEGASUS_NAMESPACE(CIMObjectPath)& ref,
        const PEGASUS_NAMESPACE(CIMInstance)& obj,
        PEGASUS_NAMESPACE(ObjectPathResponseHandler)& handler);

    virtual void deleteInstance(
        const PEGASUS_NAMESPACE(OperationContext)& context,
        const PEGASUS_NAMESPACE(CIMObjectPath)& ref,
        PEGASUS_NAMESPACE(ResponseHandler)& handler);

    // CIMAssociationProvider interface
    virtual void associators(
        const PEGASUS_NAMESPACE(OperationContext)& context,
        const PEGASUS_NAMESPACE(CIMObjectPath)& objectName,
        const PEGASUS_NAMESPACE(CIMName)& associationClass,
        const PEGASUS_NAMESPACE(CIMName)& resultClass,
        const PEGASUS_NAMESPACE(String)& role,
        const PEGASUS_NAMESPACE(String)& resultRole,
        const PEGASUS_NAMESPACE(Boolean) includeQualifiers,
        const PEGASUS_NAMESPACE(Boolean) includeClassOrigin,
        const PEGASUS_NAMESPACE(CIMPropertyList)& propertyList,
        PEGASUS_NAMESPACE(ObjectResponseHandler)& handler);

    virtual void associatorNames(
        const PEGASUS_NAMESPACE(OperationContext)& context,
        const PEGASUS_NAMESPACE(CIMObjectPath)& objectName,
        const PEGASUS_NAMESPACE(CIMName)& associationClass,
        const PEGASUS_NAMESPACE(CIMName)& resultClass,
        const PEGASUS_NAMESPACE(String)& role,
        const PEGASUS_NAMESPACE(String)& resultRole,
        PEGASUS_NAMESPACE(ObjectPathResponseHandler)& handler);

    virtual void references(
        const PEGASUS_NAMESPACE(OperationContext)& context,
        const PEGASUS_NAMESPACE(CIMObjectPath)& objectName,
        const PEGASUS_NAMESPACE(CIMName)& resultClass,
        const PEGASUS_NAMESPACE(String)& role,
        const PEGASUS_NAMESPACE(Boolean) includeQualifiers,
        const PEGASUS_NAMESPACE(Boolean) includeClassOrigin,
        const PEGASUS_NAMESPACE(CIMPropertyList)& propertyList,
        PEGASUS_NAMESPACE(ObjectResponseHandler)& handler);

    virtual void referenceNames(
        const PEGASUS_NAMESPACE(OperationContext)& context,
        const PEGASUS_NAMESPACE(CIMObjectPath)& objectName,
        const PEGASUS_NAMESPACE(CIMName)& resultClass,
        const PEGASUS_NAMESPACE(String)& role,
        PEGASUS_NAMESPACE(ObjectPathResponseHandler)& handler);

private:

    PEGASUS_NAMESPACE(Array)<PEGASUS_NAMESPACE(CIMInstance)> _teacherInstances;
    PEGASUS_NAMESPACE(Array)<PEGASUS_NAMESPACE(CIMInstance)> _studentInstances;
    PEGASUS_NAMESPACE(Array)<PEGASUS_NAMESPACE(CIMInstance)>
        _TSassociationInstances;
    PEGASUS_NAMESPACE(Array)<PEGASUS_NAMESPACE(CIMInstance)>
        _ASassociationInstances;

    void _createDefaultInstances();

    PEGASUS_NAMESPACE(CIMInstance) _createInstance(
        const PEGASUS_NAMESPACE(CIMName)& className,
        const PEGASUS_NAMESPACE(String)& name,
        PEGASUS_NAMESPACE(Uint8) id);

    PEGASUS_NAMESPACE(CIMInstance) _createTSAssociationInstance(
        PEGASUS_NAMESPACE(CIMObjectPath) ref1,
        PEGASUS_NAMESPACE(CIMObjectPath) ref2);

    PEGASUS_NAMESPACE(CIMInstance) _createASAssociationInstance(
        PEGASUS_NAMESPACE(CIMObjectPath) ref1,
        PEGASUS_NAMESPACE(CIMObjectPath) ref2);

    PEGASUS_NAMESPACE(Array)<PEGASUS_NAMESPACE(CIMInstance)>
        _filterAssociationInstancesByRole(
            const PEGASUS_NAMESPACE(Array)<PEGASUS_NAMESPACE(CIMInstance)>&
                associationInstances,
            const PEGASUS_NAMESPACE(CIMObjectPath)& targetObjectPath,
            const PEGASUS_NAMESPACE(String)& role);

    PEGASUS_NAMESPACE(Array)<PEGASUS_NAMESPACE(CIMObjectPath)>
        _filterAssociationInstances(
            PEGASUS_NAMESPACE(CIMInstance)& associationInstance,
            const PEGASUS_NAMESPACE(CIMObjectPath)& sourceObjectPath,
            PEGASUS_NAMESPACE(CIMName) resultClass,
            PEGASUS_NAMESPACE(String) resultRole);

    void _getInstance(
        const PEGASUS_NAMESPACE(Array)<PEGASUS_NAMESPACE(CIMInstance)>&
            instances,
        const PEGASUS_NAMESPACE(CIMObjectPath)& localReference,
        PEGASUS_NAMESPACE(InstanceResponseHandler)& handler);

    void _enumerateInstances(
        const PEGASUS_NAMESPACE(Array)<PEGASUS_NAMESPACE(CIMInstance)>&
            instances,
        PEGASUS_NAMESPACE(InstanceResponseHandler)& handler);

    void _enumerateInstanceNames(
        const PEGASUS_NAMESPACE(Array)<PEGASUS_NAMESPACE(CIMInstance)>&
            instances,
        PEGASUS_NAMESPACE(ObjectPathResponseHandler)& handler);

    void _associators(
        const PEGASUS_NAMESPACE(Array)<PEGASUS_NAMESPACE(CIMInstance)>&
            associationInstances,
        const PEGASUS_NAMESPACE(CIMObjectPath)& localReference,
        const PEGASUS_NAMESPACE(String)& role,
        const PEGASUS_NAMESPACE(CIMName)& resultClass,
        const PEGASUS_NAMESPACE(String)& resultRole,
        PEGASUS_NAMESPACE(ObjectResponseHandler)& handler);

    void _associatorNames(
        const PEGASUS_NAMESPACE(Array)<PEGASUS_NAMESPACE(CIMInstance)>&
            associationInstances,
        const PEGASUS_NAMESPACE(CIMObjectPath)& localReference,
        const PEGASUS_NAMESPACE(String)& role,
        const PEGASUS_NAMESPACE(CIMName)& resultClass,
        const PEGASUS_NAMESPACE(String)& resultRole,
        PEGASUS_NAMESPACE(ObjectPathResponseHandler)& handler);
};

#endif
