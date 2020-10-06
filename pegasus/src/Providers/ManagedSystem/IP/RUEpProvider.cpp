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
//    CIM_ProtocolEndpoint
//    CIM_NextHopRoute
//    PG_RouteUsesEndpoint   (association class)
//
// Association methods supported are: associators, associatorNames,
//                                    references, referenceNames
//

#include "RUEpProvider.h"

PEGASUS_USING_STD;
PEGASUS_USING_PEGASUS;

//********************************************************
//  Constants
//********************************************************

// Namespace name
static const CIMNamespaceName NAMESPACE = CIMNamespaceName("root/cimv2");

// Class names
static const CIMName CLASS_CIM_PROTOCOL_ENDPOINT = CIMName(
    "CIM_ProtocolEndpoint");
static const CIMName CLASS_CIM_NEXT_HOP_ROUTE = CIMName("CIM_NextHopRoute");
static const CIMName CLASS_PG_ROUTE_USES_ENDPOINT = CIMName(
    "PG_RouteUsesEndpoint");

// Property names.
static const CIMName PROPERTY_INSTANCE_ID = CIMName("InstanceID");
static const CIMName PROPERTY_DESTINATION_ADDRESS = CIMName(
    "DestinationAddress");
static const CIMName PROPERTY_DESTINATION_MASK = CIMName("DestinationMask");
static const CIMName PROPERTY_PREFIX_LENGTH = CIMName("PrefixLength");
static const CIMName PROPERTY_SUBNET_MASK = CIMName("SubnetMask");
static const CIMName PROPERTY_ADDRESS_TYPE = CIMName("AddressType");
static const CIMName PROPERTY_PROTOCOL_IF_TYPE = CIMName("ProtocolIFType");
static const CIMName PROPERTY_IPV4ADDRESS = CIMName("IPv4Address");
static const CIMName PROPERTY_IPV6ADDRESS = CIMName("IPv6Address");
static const CIMName PROPERTY_ADDRESS = CIMName("Address");


RUEpProvider::RUEpProvider()
{
}

RUEpProvider::~RUEpProvider()
{
}

void RUEpProvider::initialize(CIMOMHandle &ch)
{
    PEG_METHOD_ENTER(TRC_PROVIDERAGENT,
        "RUEpProvider::initialize()");

    _cimomHandle = ch;

    _createAssociationInstances(
        _NextHopRouteInstances(),
        _IPInterfaceInstances());

    PEG_METHOD_EXIT();
    return;
}

void RUEpProvider::terminate()
{
    delete this;
}

void RUEpProvider::associators(
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

    if (associationClass == CLASS_PG_ROUTE_USES_ENDPOINT)
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

void RUEpProvider::associatorNames(
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

    if (associationClass == CLASS_PG_ROUTE_USES_ENDPOINT)
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

void RUEpProvider::references(
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
    if (resultClass == CLASS_PG_ROUTE_USES_ENDPOINT)
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

void RUEpProvider::referenceNames(
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
    if (resultClass == CLASS_PG_ROUTE_USES_ENDPOINT)
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


void RUEpProvider::_associators(
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
            else if (className == CLASS_CIM_PROTOCOL_ENDPOINT)
            {
                // find instance that corresponds to the reference
                for (Uint32 k = 0, s = _ipifInstances.size(); k < s; k++)
                {
                    CIMObjectPath path = _ipifInstances[k].getPath();

                    // Build a host and namespace independent object path
                    CIMObjectPath localPath = CIMObjectPath(
                        String(),
                        CIMNamespaceName(),
                        path.getClassName(),
                        path.getKeyBindings());

                    if (resultPaths[j].identical(localPath))
                    {
                        // deliver instance
                        handler.deliver(_ipifInstances[k]);
                    }
                }
            }
        }
    }
}

void RUEpProvider::_associatorNames(
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
Array<CIMInstance> RUEpProvider::_filterAssociationInstancesByRole(
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
Array<CIMObjectPath> RUEpProvider::_filterAssociationInstances(
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

void RUEpProvider::_createAssociationInstances(
    Array<CIMInstance> nhrInst,
    Array<CIMInstance> ipifInst)
{
    PEG_METHOD_ENTER(TRC_PROVIDERAGENT,
        "RUEpProvider::_createAssociationInstances()");

    Uint16 nhrInstSize = nhrInst.size();
    Uint16 ipifInstSize = ipifInst.size();

    for (Uint16 i = 0; i < nhrInstSize; i++)  // Routes loop.
    {
        CIMInstance _nhrInstRet, _ipifInstRet;
        CIMInstance _nhrInst = nhrInst[i];

        CIMProperty _ipifAddress;
        CIMProperty _ipifSubnetMask_PrefixLength;
        CIMProperty _ipifProtocolIFType;
        CIMProperty _nhrInstanceID = _nhrInst.getProperty(
            _nhrInst.findProperty(PROPERTY_INSTANCE_ID));
        CIMProperty _nhrAddrType;
        CIMProperty _nhrDestMask_PrefixLength;


        for (Uint16 j = 0; j<ipifInstSize; j++)  // Interfaces loop.
        {
            CIMInstance _ipifInst = ipifInst[j];

            _ipifProtocolIFType = _ipifInst.getProperty(
                _ipifInst.findProperty(PROPERTY_PROTOCOL_IF_TYPE));

            Uint16 _pift;
            CIMValue _piftCIMValue = _ipifProtocolIFType.getValue();
            if ((_piftCIMValue.getType() == CIMTYPE_UINT16) &&
                (!_piftCIMValue.isNull ()))
            {
                _piftCIMValue.get(_pift);
            }
            else
            {
                throw CIMOperationFailedException(
                    "Can't determine CIMValue::TYPE of ProtocolIFType:" +
                    String(cimTypeToString(_piftCIMValue.getType())));
            }

            if (_pift == 4096)  // IPv4 address.
            {
                _ipifAddress = _ipifInst.getProperty(
                    _ipifInst.findProperty(
                        PROPERTY_IPV4ADDRESS));
                _ipifSubnetMask_PrefixLength = _ipifInst.getProperty(
                    _ipifInst.findProperty(
                    PROPERTY_SUBNET_MASK));
            }
            else
            {
                if (_pift == 4097)  // IPv6 address.
                {
                    _ipifAddress = _ipifInst.getProperty(
                        _ipifInst.findProperty(
                            PROPERTY_IPV6ADDRESS));
                    _ipifSubnetMask_PrefixLength = _ipifInst.getProperty(
                        _ipifInst.findProperty(
                            PROPERTY_IPV6ADDRESS));
                }
                else
                {
                    char buffer[22];
                    Uint32 sz;
                    String _piftStr = Uint16ToString(buffer, _pift, sz);
                    throw CIMOperationFailedException(
                        "ProtocolIFType == " +  _piftStr );
                }
            }

            // In this implementation, we choose InstanceID to be
            // equal DestinationAddress. So, if Address of interface
            // and InstanceID of route doesn't match, check if SubnetMask
            // of interface and DestinationMask route does.
            if (!_nhrInstanceID.getValue().equal(_ipifAddress.getValue()))
            {
                _nhrAddrType = _nhrInst.getProperty(
                    _nhrInst.findProperty(
                        PROPERTY_ADDRESS_TYPE));
                Uint16 _addrt;
                CIMValue _nhratCIMValue = _nhrAddrType.getValue();
                if ((_nhratCIMValue.getType() == CIMTYPE_UINT16) &&
                    (!_nhratCIMValue.isNull()))
                {
                    _nhratCIMValue.get(_addrt);
                }
                else
                {
                    throw CIMOperationFailedException(
                        "Can't determine CIMValue::TYPE of AddressType: " +
                        String(cimTypeToString(_piftCIMValue.getType())));
                }

                if (_addrt == 1)  // IPv4 address.
                {
                    _nhrDestMask_PrefixLength = _nhrInst.getProperty(
                       _nhrInst.findProperty(
                           PROPERTY_DESTINATION_MASK));
                }
                else
                {
                    if (_addrt == 2)  // IPv6 address.
                    {
                         _nhrDestMask_PrefixLength = _nhrInst.getProperty(
                             _nhrInst.findProperty(
                                 PROPERTY_PREFIX_LENGTH));
                    }
                    else
                    {
                        char buffer[22];
                        Uint32 sz;
                        String _addrtStr = Uint16ToString(buffer, _addrt, sz);
                        throw CIMOperationFailedException(
                            "Unknown AddressType = " + _addrtStr);
                    }
                }

                // If SubnetMask of interface and DestinationMask route
                // doesn't match, these instances are unrelated. So,
                // proceed to the next pair.
                if (!_nhrDestMask_PrefixLength.getValue().equal(
                    _ipifSubnetMask_PrefixLength.getValue()))
                {
                    continue;
                }
            }

            // Build the CIMObjectPath from the instances matching
            CIMObjectPath _ipifObj = _ipifInst.getPath();
            CIMObjectPath _nhrObj = _nhrInst.getPath();

            CIMInstance assocInst(CLASS_PG_ROUTE_USES_ENDPOINT);
            assocInst.addProperty(
                CIMProperty(
                    CIMName("Antecedent"),
                    _ipifObj,
                    0,
                    CLASS_CIM_PROTOCOL_ENDPOINT));
            assocInst.addProperty(
                CIMProperty(
                    CIMName("Dependent"),
                    _nhrObj,
                    0,
                    CLASS_CIM_NEXT_HOP_ROUTE));

            // Build CIMObjectPath from keybindings
            Array<CIMKeyBinding> keyBindings;
            CIMKeyBinding _ipifBinding(
                CIMName("Antecedent"),
                _ipifObj.toString(),
                CIMKeyBinding::REFERENCE);
            CIMKeyBinding _nhrBinding(
                CIMName("Dependent"),
                _nhrObj.toString(),
                CIMKeyBinding::REFERENCE);
            keyBindings.append (_ipifBinding);
            keyBindings.append (_nhrBinding);

            CIMObjectPath path(
                String::EMPTY,
                CIMNamespaceName(),
                CLASS_PG_ROUTE_USES_ENDPOINT,
                keyBindings);

            assocInst.setPath(path);
            _AssociationInstances.append(assocInst);
            break;
        }  // Interfaces loop end.

    }  // Routes loop end.

    PEG_METHOD_EXIT();
}

Array<CIMInstance> RUEpProvider::_NextHopRouteInstances()
{
    PEG_METHOD_ENTER(TRC_PROVIDERAGENT,
        "RUEpProvider::_NextHopRouteInstances()");

    Array<CIMInstance> _retInstances;
    NextHopRouteList _nhrl;
    InterfaceList _ifl;

    for (Uint16 i = 0; i<_nhrl.size(); i++)
    {
        NextHopIPRoute _nhipr = _nhrl.getRoute(i);

        if (_nhipr.isRouteLocal())
        {
            CIMInstance instance(CLASS_CIM_NEXT_HOP_ROUTE);
            String _destAddr, _destMask, _instanceID;
            Uint16 _addrType;
            Uint8 _prefLength;

            if (!_nhipr.getInstanceID(_instanceID))
            {
                throw CIMOperationFailedException(
                    "Can't determine InstanceID in: " +
                    String("RUEpProvider::_NextHopRouteInstances()"));
            }
            instance.addProperty(CIMProperty(
                PROPERTY_INSTANCE_ID,
                _instanceID));

            if (!_nhipr.getDestinationAddress(_destAddr))
            {
                throw CIMOperationFailedException(
                    "Can't determine destination address in: " +
                    String("RUEpProvider::_NextHopRouteInstances()"));
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
                        throw CIMOperationFailedException(
                            "Can't determine destination mask in: " +
                            String("RUEpProvider::_NextHopRouteInstances()"));
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
                            throw CIMOperationFailedException(
                                "Can't determine prefix length for route in: " +
                              String("RUEpProvider::_NextHopRouteInstances()"));
                        }
                        instance.addProperty(CIMProperty(
                            PROPERTY_PREFIX_LENGTH,
                            _prefLength));
                    }
                }
            }
            else
            {
                throw CIMOperationFailedException(
                    "Can't determine address type in: " +
                    String("RUEpProvider::_NextHopRouteInstances()"));
            }

            // Build CIMObjectPath from keybindings
            Array<CIMKeyBinding> keyBindings;
            keyBindings.append(
            CIMKeyBinding(
                PROPERTY_INSTANCE_ID,
                _instanceID,
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

Array<CIMInstance> RUEpProvider::_IPInterfaceInstances()
{
    PEG_METHOD_ENTER(TRC_PROVIDERAGENT,
        "RUEpProvider::_IPInterfaceInstances()");

    Array<CIMInstance> _retInstances;
    InterfaceList _ifl;

    for (Uint16 i = 0; i<_ifl.size(); i++)
    {
        CIMInstance instance(CLASS_CIM_PROTOCOL_ENDPOINT);
        IPInterface _ipif = _ifl.getInterface(i);

        String _addr, _subnetMask;
        Uint16 _protocolIFType;
        Uint8 _prefLength;

        if (_ipif.getProtocolIFType(_protocolIFType))
        {
            instance.addProperty(CIMProperty(
                PROPERTY_PROTOCOL_IF_TYPE,
                _protocolIFType));

            if (_protocolIFType == 4096)  // IPv4 address.
            {
                if (!_ipif.getIPv4Address(_addr))
                {
                    throw CIMOperationFailedException(
                        "Can't determine IPv4 address in: " +
                        String("RUEpProvider::_IPInterfaceInstances()"));
                }
                instance.addProperty(CIMProperty(PROPERTY_IPV4ADDRESS, _addr));

                if (!_ipif.getSubnetMask(_subnetMask))
                {
                    throw CIMOperationFailedException(
                        "Can't determine subnet mask in: " +
                        String("RUEpProvider::_IPInterfaceInstances()"));
                }
                instance.addProperty(CIMProperty(
                    PROPERTY_SUBNET_MASK,
                    _subnetMask));
            }
            else
            {
                if (_protocolIFType == 4097)  // IPv6 address.
                {
                    if (!_ipif.getIPv6Address(_addr))
                    {
                        throw CIMOperationFailedException(
                            "Can't determine IPv6 address in: " +
                            String("RUEpProvider::_IPInterfaceInstances()"));
                    }
                    instance.addProperty(CIMProperty(
                        PROPERTY_IPV6ADDRESS,
                        _addr));

                    if (!_ipif.getPrefixLength(_prefLength))
                    {
                        throw CIMOperationFailedException(
                            "Can't determine prefix length for interface in: " +
                            String("RUEpProvider::_IPInterfaceInstances()"));
                    }
                    instance.addProperty(CIMProperty(
                        PROPERTY_PREFIX_LENGTH,
                        _prefLength));
                }
            }
        }
        else
        {
            throw CIMOperationFailedException(
                "Can't determine instance protocol type in: " +
                String("RUEpProvider::_IPInterfaceInstances()"));
        }

        // Build CIMObjectPath from keybindings
        Array<CIMKeyBinding> keyBindings;
        keyBindings.append(
            CIMKeyBinding(
                PROPERTY_ADDRESS,
                _addr,
                CIMKeyBinding::STRING));

        CIMObjectPath path(
            String::EMPTY,
            CIMNamespaceName(),
            CLASS_CIM_PROTOCOL_ENDPOINT,
            keyBindings);

         instance.setPath(path);
         _ipifInstances.append(instance);
         _retInstances.append(instance);
    }

    PEG_METHOD_EXIT();
    return _retInstances;
}

