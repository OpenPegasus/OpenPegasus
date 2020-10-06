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

//
// This is an association provider. It implements the association methods for
// the following classes:
//
//    CIM_RemoteServiceAccessPoint
//    CIM_NextHopRoute
//    PG_AssociatedNextHop   (association class)
//
// Association methods supported are: associators, associatorNames,
//                                    references, referenceNames
//

#include "ANHProvider.h"

PEGASUS_USING_STD;
PEGASUS_USING_PEGASUS;

//********************************************************
//  Constants
//********************************************************

// Namespace name
static const CIMNamespaceName NAMESPACE = CIMNamespaceName("root/cimv2");

// Class names
static const CIMName CLASS_CIM_REMOTE_SERVICE_ACCESS_POINT = CIMName(
    "CIM_RemoteServiceAccessPoint");
static const CIMName CLASS_CIM_NEXT_HOP_ROUTE = CIMName("CIM_NextHopRoute");
static const CIMName CLASS_PG_ASSOCIATED_NEXT_HOP = CIMName(
    "PG_AssociatedNextHop");

// Property names.
static const CIMName PROPERTY_INSTANCE_ID = CIMName("InstanceID");
static const CIMName PROPERTY_ADDRESS_TYPE = CIMName("AddressType");
static const CIMName PROPERTY_DESTINATION_ADDRESS = CIMName(
    "DestinationAddress");
static const CIMName PROPERTY_DESTINATION_MASK = CIMName("DestinationMask");
static const CIMName PROPERTY_PREFIX_LENGTH = CIMName("PrefixLength");
static const CIMName PROPERTY_ACCESS_INFO = CIMName("AccessInfo");
static const CIMName PROPERTY_INFO_FORMAT = CIMName("InfoFormat");

ANHProvider::ANHProvider()
{
}

ANHProvider::~ANHProvider()
{
}

void ANHProvider::initialize(CIMOMHandle &ch)
{
    _cimomHandle = ch;

    _createAssociationInstances(
        _NextHopRouteInstances(),
        _RSApInstances());

    return;
}

void ANHProvider::terminate()
{
    delete this;
}

void ANHProvider::associators(
        const OperationContext& context,
        const CIMObjectPath& objectName,
        const CIMName& associationClass,
        const CIMName& resultClass,
        const String& role,
        const String& resultRole,
        const Boolean includeQualifiers,
        const Boolean includeClassOrigin,
        const CIMPropertyList& propertyList,
        ObjectResponseHandler& handler)
{

    // validate namespace
    const CIMNamespaceName& nameSpace = objectName.getNameSpace();
    if (!nameSpace.equal(NAMESPACE))
    {
        throw CIMNotSupportedException(
            nameSpace.getString() + " not supported.");
    }


    // Build a host and namespace independent object path
    CIMObjectPath localObjectPath = CIMObjectPath(
        String(),
        CIMNamespaceName(),
        objectName.getClassName(),
        objectName.getKeyBindings());

    // begin processing the request
    handler.processing();

    if (associationClass == CLASS_PG_ASSOCIATED_NEXT_HOP)
    {
        _associators(
            _AssociationInstances,
            localObjectPath,
            role,
            resultClass,
            resultRole,
            handler);
    }
    else
    {
        throw CIMNotSupportedException(
            associationClass.getString() + " is not supported");
    }

    // complete processing the request
    handler.complete();
}

void ANHProvider::associatorNames(
        const OperationContext& context,
        const CIMObjectPath& objectName,
        const CIMName& associationClass,
        const CIMName& resultClass,
        const String& role,
        const String& resultRole,
        ObjectPathResponseHandler& handler)
{
    // validate namespace
    const CIMNamespaceName& nameSpace = objectName.getNameSpace();
    if (!nameSpace.equal(NAMESPACE))
    {
        throw CIMNotSupportedException(
            nameSpace.getString() + " not supported.");
    }

    // Build a host and namespace independent object path
    CIMObjectPath localObjectPath = CIMObjectPath(
        String(),
        CIMNamespaceName(),
        objectName.getClassName(),
        objectName.getKeyBindings());

    // begin processing the request
    handler.processing();

    if (associationClass == CLASS_PG_ASSOCIATED_NEXT_HOP)
    {
        _associatorNames(
            _AssociationInstances,
            localObjectPath,
            role,
            resultClass,
            resultRole,
            handler);
    }
    else
    {
        throw CIMNotSupportedException(
            associationClass.getString() + " is not supported");
    }

    // complete processing the request
    handler.complete();
}

void ANHProvider::references(
        const OperationContext& context,
        const CIMObjectPath& objectName,
        const CIMName& resultClass,
        const String& role,
        const Boolean includeQualifiers,
        const Boolean includeClassOrigin,
        const CIMPropertyList& propertyList,
        ObjectResponseHandler& handler)
{
    // validate namespace
    const CIMNamespaceName& nameSpace = objectName.getNameSpace();
    if (!nameSpace.equal(NAMESPACE))
    {
        throw CIMNotSupportedException(
            nameSpace.getString() + " not supported.");
    }

    // Build a host and namespace independent object path
    CIMObjectPath localObjectPath = CIMObjectPath(
            String(),
            CIMNamespaceName(),
            objectName.getClassName(),
            objectName.getKeyBindings());

    // begin processing the request
    handler.processing();

    // Filter the instances from the list of association instances against
    // the specified role filter
    //
    Array<CIMInstance> resultInstances;
    if (resultClass == CLASS_PG_ASSOCIATED_NEXT_HOP)
    {
        resultInstances =
            _filterAssociationInstancesByRole(
                _AssociationInstances,
                localObjectPath,
                role);
    }
    else
    {
        throw CIMNotSupportedException(
            resultClass.getString() + " is not supported");
    }

    // return the instances
    for (Uint32 i = 0, n = resultInstances.size(); i < n; i++)
    {
        handler.deliver(resultInstances[i]);
    }

    // complete processing the request
    handler.complete();
}

void ANHProvider::referenceNames(
        const OperationContext& context,
        const CIMObjectPath& objectName,
        const CIMName& resultClass,
        const String& role,
        ObjectPathResponseHandler& handler)
{
    // validate namespace
    const CIMNamespaceName& nameSpace = objectName.getNameSpace();
    if (!nameSpace.equal(NAMESPACE))
    {
        throw CIMNotSupportedException(
            nameSpace.getString() + " not supported.");
    }

    // Build a host and namespace independent object path
    CIMObjectPath localObjectPath = CIMObjectPath(
        String(),
        CIMNamespaceName(),
        objectName.getClassName(),
        objectName.getKeyBindings());

    // begin processing the request
    handler.processing();

    // Filter the instances from the list of association instances against
    // the specified role filter
    //
    Array<CIMInstance> resultInstances;
    if (resultClass == CLASS_PG_ASSOCIATED_NEXT_HOP)
    {
        resultInstances =
            _filterAssociationInstancesByRole(
                _AssociationInstances,
                localObjectPath,
                role);
    }
    else
    {
        throw CIMNotSupportedException(
            resultClass.getString() + " is not supported");
    }

    // return the instance names
    for (Uint32 i = 0, n = resultInstances.size(); i < n; i++)
    {
        CIMObjectPath objectPath = resultInstances[i].getPath();
        handler.deliver(objectPath);
    }

    // complete processing the request
    handler.complete();
}

///////////////////////////////////////////////////////////////////////////////
//  Private methods
///////////////////////////////////////////////////////////////////////////////


void ANHProvider::_associators(
    const Array<CIMInstance>& associationInstances,
    const CIMObjectPath& localReference,
    const String& role,
    const CIMName& resultClass,
    const String& resultRole,
    ObjectResponseHandler& handler)
{

    // Filter the instances from the list of association instances against
    // the specified role filter
    //
    Array<CIMInstance> assocInstances = _filterAssociationInstancesByRole(
        associationInstances,
        localReference,
        role);

    // Now filter the result association instances against the specified
    // resultClass and resultRole filters
    //
    for (Uint32 i = 0, m = assocInstances.size(); i < m; i++)
    {
        Array<CIMObjectPath> resultPaths = _filterAssociationInstances(
            assocInstances[i],
            localReference,
            resultClass,
            resultRole);

        for (Uint32 j = 0, n = resultPaths.size(); j < n; j++)
        {
            CIMName className = resultPaths[j].getClassName();
            if (className == CLASS_CIM_NEXT_HOP_ROUTE)
            {
                // find instance that corresponds to the reference
                for (Uint32 k = 0, s = _nhrInstances.size(); k < s; k++)
                {
                    CIMObjectPath path = _nhrInstances[k].getPath();

                    // Build a host and namespace independent object path
                    CIMObjectPath localPath = CIMObjectPath(
                        String(),
                        CIMNamespaceName(),
                        path.getClassName(),
                        path.getKeyBindings());

                    if (resultPaths[j].identical(localPath))
                    {
                        // deliver the instance
                        handler.deliver(_nhrInstances[k]);
                    }
                }
            }
            else if (className == CLASS_CIM_REMOTE_SERVICE_ACCESS_POINT)
            {
                // find instance that corresponds to the reference
                for (Uint32 k = 0, s = _rsapInstances.size(); k < s; k++)
                {
                    CIMObjectPath path = _rsapInstances[k].getPath();

                    // Build a host and namespace independent object path
                    CIMObjectPath localPath = CIMObjectPath(
                        String(),
                        CIMNamespaceName(),
                        path.getClassName(),
                        path.getKeyBindings());

                    if (resultPaths[j].identical(localPath))
                    {
                        // deliver instance
                        handler.deliver(_rsapInstances[k]);
                    }
                }
            }
        }
    }
}

void ANHProvider::_associatorNames(
    const Array<CIMInstance>& associationInstances,
    const CIMObjectPath& localReference,
    const String& role,
    const CIMName& resultClass,
    const String& resultRole,
    ObjectPathResponseHandler& handler)
{
    // Filter the instances from the list of association instances against
    // the specified role filter
    //
    Array<CIMInstance> assocInstances;
    assocInstances= _filterAssociationInstancesByRole(associationInstances,
        localReference, role);

    // Now filter the result association instances against the specified
    // resultClass and resultRole filters
    //
    for (Uint32 i = 0, n = assocInstances.size(); i < n; i++)
    {
        Array<CIMObjectPath> resultPaths;
        resultPaths = _filterAssociationInstances(assocInstances[i],
            localReference, resultClass, resultRole);

        for (Uint32 j = 0, m = resultPaths.size(); j < m; j++)
        {
            handler.deliver(resultPaths[j]);
        }
    }
}

/**
 ***************************************************************************
   _filterAssociationInstancesByRole is used to filter the list of association
   instances against the specified role filter.  It returns a list of
   association instances that pass the filter test.

    @param associationInstance   - The target association instances
    @param targetObjectPath      - The target ObjectPath
    @param role                  - The role filter.  If there is no role, this
                                   is String::EMPTY

    @return   the set of association instances that pass the filter test.
 ***************************************************************************
*/
Array<CIMInstance> ANHProvider::_filterAssociationInstancesByRole(
    const Array<CIMInstance>& associationInstances,
    const CIMObjectPath& targetObjectPath,
    const String& role)
{
    Array<CIMInstance> returnInstances;

    // Filter the instances from the list of association instances against
    // the specified role filter
    //
    for (Uint32 i = 0, n = associationInstances.size(); i < n; i++)
    {
        CIMInstance instance = associationInstances[i];

        // Search the association instance for all reference properties
        for (Uint32 j = 0, m = instance.getPropertyCount(); j < m; j++)
        {
            const CIMProperty p = instance.getProperty(j);
            if (p.getType() == CIMTYPE_REFERENCE)
            {
                CIMValue v = p.getValue();
                CIMObjectPath path;
                v.get(path);

                if ((role == String::EMPTY) ||
                    (p.getName() == CIMName(role)))
                {
                    if (targetObjectPath.identical(path))
                    {
                        returnInstances.append(instance);
                    }
                }
            }
        }
    }
    return returnInstances;
}

/**
 ***************************************************************************
   _filterAssociationInstances is used to filter the set of possible return
   instances against the filters (resultClass and resultRole) provided with
   the associators and associatorNames operations.  It returns the ObjectPaths
   of the set of objects that pass the filter tests.

    @param assocInstance     - The target association class instance
    @param sourceObjectPath  - The source ObjectPath
    @param resultClass       - The result class. If there is no resultClass,
                               this is String::EMPTY.
    @param resultRole        - The result role. If there is no role, this is
                               String::EMPTY

    @return   the ObjectPaths of the set of association instances that pass
              the filter tests.
 ***************************************************************************
*/
Array<CIMObjectPath> ANHProvider::_filterAssociationInstances(
    CIMInstance& assocInstance,
    const CIMObjectPath& sourceObjectPath,
    CIMName resultClass,
    String resultRole)
{
    Array<CIMObjectPath> returnPaths;

    // get all Reference properties
    for (Uint32 i = 0, n = assocInstance.getPropertyCount(); i < n; i++)
    {
        CIMProperty p = assocInstance.getProperty(i);

        if (p.getType() == CIMTYPE_REFERENCE)
        {
            CIMValue v = p.getValue();
            CIMObjectPath path;
            v.get(path);

            if (!sourceObjectPath.identical(path))
            {
                if (resultClass.isNull() || resultClass == path.getClassName())
                {
                    if (resultRole == String::EMPTY ||
                        (p.getName() == CIMName(resultRole)))
                    {
                        returnPaths.append(path);
                    }
                }
            }
        }
    }
    return returnPaths;
}

void ANHProvider::_createAssociationInstances(
    Array<CIMInstance> nhrInst,
    Array<CIMInstance> rsapInst)
{
    PEG_METHOD_ENTER(TRC_PROVIDERAGENT,
        "ANHProvider::_createAssociationInstances()");

    for (Uint16 i = 0; i<nhrInst.size(); i++)  // Routes loop.
    {
        CIMInstance _nhrInst = nhrInst[i];

        for (Uint16 j = 0; j<rsapInst.size(); j++)  // Remote Services loop.
        {
            CIMInstance _rsapInst = rsapInst[j];

            String _accessInfo,
                _address,
                _destAddress;
            CIMProperty _rsapAccessInfo = _rsapInst.getProperty(
                _rsapInst.findProperty(PROPERTY_ACCESS_INFO));
            _rsapAccessInfo.getValue().get(_accessInfo);

            Uint32 index = _accessInfo.find('/');

            if (index == PEG_NOT_FOUND)
            {
                PEG_METHOD_EXIT();
                throw CIMOperationFailedException(
                    String("Property AccessInfo is not in the expected ") +
                    String("\"Address/DestinationAddress\" format."));
            }
            else
            {
                _address = _accessInfo.subString(0,index-1);
                _destAddress = _accessInfo.subString(
                    index+1,
                    _accessInfo.size()-1);
            }

            CIMProperty _nhrDestinationAddress = _nhrInst.getProperty(
                _nhrInst.findProperty(PROPERTY_DESTINATION_ADDRESS));

            if (!_nhrDestinationAddress.getValue().equal(
                CIMValue(_destAddress)))
            {
                continue;
            }

            // Build the CIMObjectPath from the instances matching
            CIMObjectPath _rsapObj = _rsapInst.getPath();
            CIMObjectPath _nhrObj = _nhrInst.getPath();

            CIMInstance assocInst(CLASS_PG_ASSOCIATED_NEXT_HOP);
            assocInst.addProperty(
                CIMProperty(
                    CIMName("Antecedent"),
                    _rsapObj,
                    0,
                    CLASS_CIM_REMOTE_SERVICE_ACCESS_POINT));
            assocInst.addProperty(
                CIMProperty(
                    CIMName("Dependent"),
                    _nhrObj,
                    0,
                    CLASS_CIM_NEXT_HOP_ROUTE));

            // Build CIMObjectPath from keybindings
            Array<CIMKeyBinding> keyBindings;
            CIMKeyBinding _rsapBinding(
                CIMName("Antecedent"),
                _rsapObj.toString(),
                CIMKeyBinding::REFERENCE);
            CIMKeyBinding _nhrBinding(
                CIMName("Dependent"),
                _nhrObj.toString(),
                CIMKeyBinding::REFERENCE);
            keyBindings.append (_rsapBinding);
            keyBindings.append (_nhrBinding);

            CIMObjectPath path(
                String::EMPTY,
                CIMNamespaceName(),
                CLASS_PG_ASSOCIATED_NEXT_HOP,
                keyBindings);

            assocInst.setPath(path);
            _AssociationInstances.append(assocInst);
            break;

        }  // Remote Services loop end.

    }  // Routes loop end.

    PEG_METHOD_EXIT();
}

Array<CIMInstance> ANHProvider::_NextHopRouteInstances()
{
    PEG_METHOD_ENTER(TRC_PROVIDERAGENT,
        "ANHProvider::_NextHopRouteInstances()");

    Array<CIMInstance> _retInstances;
    NextHopRouteList _nhrl;

    for (Uint16 i = 0; i<_nhrl.size(); i++)
    {
        NextHopIPRoute _nhipr = _nhrl.getRoute(i);

        if (!_nhipr.isRouteLocal())
        {
            CIMInstance instance(CLASS_CIM_NEXT_HOP_ROUTE);
            String _destAddr, _destMask, _instanceID;
            Uint16 _addrType;
            Uint8 _prefLength;

            if (!_nhipr.getInstanceID(_instanceID))
            {
                PEG_METHOD_EXIT();
                throw CIMOperationFailedException(
                    "Can't determine InstanceID in: " +
                    String("ANHProvider::_NextHopRouteInstances()"));
            }
            instance.addProperty(CIMProperty(PROPERTY_INSTANCE_ID, _destAddr));

            if (!_nhipr.getDestinationAddress(_destAddr))
            {
                PEG_METHOD_EXIT();
                throw CIMOperationFailedException(
                    "Can't determine destination address in: " +
                    String("ANHProvider::_NextHopRouteInstances()"));
            }
            instance.addProperty(CIMProperty(
                PROPERTY_DESTINATION_ADDRESS,
                _destAddr));

            if (_nhipr.getAddressType(_addrType))
            {
                instance.addProperty(CIMProperty(
                    PROPERTY_ADDRESS_TYPE,
                    _addrType));

                if (_addrType == 1)  // IPv4 address.
                {
                    if (!_nhipr.getDestinationMask(_destMask))
                    {
                        PEG_METHOD_EXIT();
                        throw CIMOperationFailedException(
                            "Can't determine destination mask in: " +
                            String("ANHProvider::_NextHopRouteInstances()"));
                    }
                    instance.addProperty(CIMProperty(
                        PROPERTY_DESTINATION_MASK,
                        _destMask));
                }
                else
                {
                    if (_addrType == 2)  // IPv6 address.
                    {
                        if (!_nhipr.getPrefixLength(_prefLength))
                        {
                            PEG_METHOD_EXIT();
                            throw CIMOperationFailedException(
                                "Can't determine prefix length for route in: " +
                               String("ANHProvider::_NextHopRouteInstances()"));
                        }
                        instance.addProperty(CIMProperty(
                            PROPERTY_PREFIX_LENGTH,
                            _prefLength));
                    }
                }
            }
            else
            {
                PEG_METHOD_EXIT();
                throw CIMOperationFailedException(
                    "Can't determine address type in: " +
                    String("ANHProvider::_NextHopRouteInstances()"));
            }

            // Build CIMObjectPath from keybindings
            Array<CIMKeyBinding> keyBindings;
            keyBindings.append(
            CIMKeyBinding(
                PROPERTY_INSTANCE_ID,
                _destAddr,
                CIMKeyBinding::STRING));

            CIMObjectPath path(
            String::EMPTY,
            CIMNamespaceName(),
            CLASS_CIM_NEXT_HOP_ROUTE,
            keyBindings);

            instance.setPath(path);
            _nhrInstances.append(instance);
            _retInstances.append(instance);
        }
    }  // Loop over next hop routes.

    PEG_METHOD_EXIT();
    return _retInstances;
}

Array<CIMInstance> ANHProvider::_RSApInstances()
{

    PEG_METHOD_ENTER(TRC_PROVIDERAGENT,
        "ANHProvider::_RSApInstances()");

    Array<CIMInstance> _retInstances;

    RSApList _rsapl;

    for (Uint16 i = 0; i<_rsapl.size(); i++)
    {
        CIMInstance instance(CLASS_CIM_REMOTE_SERVICE_ACCESS_POINT);
        RSAp _rsap = _rsapl.getService(i);

        String _accessInfo;
        Uint16 _infoFormat;

        if (_rsap.getAccessInfo(_accessInfo))
        {
            instance.addProperty(CIMProperty(
                PROPERTY_ACCESS_INFO,
                _accessInfo));
        }
        else
        {
            PEG_METHOD_EXIT();
            throw CIMOperationFailedException(
                "Can't determine property AccessInfo in: " +
                String("ANHProvider::_RSApInstances()"));
        }

        if (_rsap.getInfoFormat(_infoFormat))
        {
            instance.addProperty(CIMProperty(
                PROPERTY_INFO_FORMAT,
                _infoFormat));
        }
        else
        {
            PEG_METHOD_EXIT();
            throw CIMOperationFailedException(
                "Can't determine property InfoFormat in: " +
                String("ANHProvider::_RSApInstances()"));
        }

        // Build CIMObjectPath from keybindings
        Array<CIMKeyBinding> keyBindings;
        keyBindings.append(
            CIMKeyBinding(
                PROPERTY_ACCESS_INFO,
                _accessInfo,
                CIMKeyBinding::STRING));

        CIMObjectPath path(
            String::EMPTY,
            CIMNamespaceName(),
            CLASS_CIM_REMOTE_SERVICE_ACCESS_POINT,
            keyBindings);

         instance.setPath(path);
         _rsapInstances.append(instance);
         _retInstances.append(instance);
    }

    PEG_METHOD_EXIT();
    return _retInstances;
}

