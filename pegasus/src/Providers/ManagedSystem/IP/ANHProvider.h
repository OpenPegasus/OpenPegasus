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

#ifndef PG_ANH_PROVIDER_H
#define PG_ANH_PROVIDER_H

#include <Pegasus/Common/Config.h>
#include <Pegasus/Provider/CIMAssociationProvider.h>
#include <Providers/ManagedSystem/IP/IPPlatform.h>
#include <Pegasus/Common/Tracer.h>


PEGASUS_USING_STD;
PEGASUS_USING_PEGASUS;

class ANHProvider: public CIMAssociationProvider
{
public:
    ANHProvider();
    ~ANHProvider();

    void initialize(CIMOMHandle&);

    void terminate();

    // CIMAssociationProvider interface
    void associators(
        const OperationContext& context,
        const CIMObjectPath& objectName,
        const CIMName& associationClass,
        const CIMName& resultClass,
        const String& role,
        const String& resultRole,
        const Boolean includeQualifiers,
        const Boolean includeClassOrigin,
        const CIMPropertyList& propertyList,
        ObjectResponseHandler& handler);

    void associatorNames(
        const OperationContext& context,
        const CIMObjectPath& objectName,
        const CIMName& associationClass,
        const CIMName& resultClass,
        const String& role,
        const String& resultRole,
        ObjectPathResponseHandler& handler);

    void references(
        const OperationContext& context,
        const CIMObjectPath& objectName,
        const CIMName& resultClass,
        const String& role,
        const Boolean includeQualifiers,
        const Boolean includeClassOrigin,
        const CIMPropertyList& propertyList,
        ObjectResponseHandler& handler);

    void referenceNames(
        const OperationContext& context,
        const CIMObjectPath& objectName,
        const CIMName& resultClass,
        const String& role,
        ObjectPathResponseHandler& handler);

private:

    Array<CIMInstance> _nhrInstances;
    Array<CIMInstance> _rsapInstances;
    Array<CIMInstance> _AssociationInstances;

    // private member to store handle passed by initialize()
    CIMOMHandle _cimomHandle;


    Array<CIMInstance>
        _filterAssociationInstancesByRole(
            const Array<CIMInstance>&
                associationInstances,
            const CIMObjectPath& targetObjectPath,
            const String& role);

    Array<CIMObjectPath>
        _filterAssociationInstances(
            CIMInstance& associationInstance,
            const CIMObjectPath& sourceObjectPath,
            CIMName resultClass,
            String resultRole);

    void _associators(
        const Array<CIMInstance>&
            associationInstances,
        const CIMObjectPath& localReference,
        const String& role,
        const CIMName& resultClass,
        const String& resultRole,
        ObjectResponseHandler& handler);

    void _associatorNames(
        const Array<CIMInstance>&
            associationInstances,
        const CIMObjectPath& localReference,
        const String& role,
        const CIMName& resultClass,
        const String& resultRole,
        ObjectPathResponseHandler& handler);

    void _createAssociationInstances(
            Array<CIMInstance>,
            Array<CIMInstance>);

    Array<CIMInstance>
        _RSApInstances();

    Array<CIMInstance>
        _NextHopRouteInstances();
};

#endif  /* #ifndef PG_ANH_PROVIDER_H */
