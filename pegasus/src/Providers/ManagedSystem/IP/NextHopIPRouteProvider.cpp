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
//%////////////////////////////////////////////////////////////////////////////


//=============================================================================
// Includes.
//=============================================================================

#include <Providers/ManagedSystem/IP/NextHopIPRouteProvider.h>

//=============================================================================
// Class names:
//
// We use CIM_UnitaryComputerSystem as the value of the key
// SystemCreationClassName, because this class has properties that
// are important for clients of this provider.
//=============================================================================

static const CIMName CLASS_CIM_UNITARY_COMPUTER_SYSTEM = CIMName(
    "CIM_UnitaryComputerSystem");
static const CIMName CLASS_PG_IP_ROUTE = CIMName ("PG_IPRoute");
static const CIMName CLASS_PG_NEXT_HOP_IP_ROUTE = CIMName (
    "PG_NextHopIPRoute");

//=============================================================================
// The number of keys for the classes.
//=============================================================================

#define NUMKEYS_PG_NEXT_HOP_IP_ROUTE                1

//=============================================================================
// Property names.  These values are returned by the provider as
// the property names.
//=============================================================================

// Properties in CIM_ManagedElement

static const CIMName PROPERTY_CAPTION = CIMName("Caption");
static const CIMName PROPERTY_DESCRIPTION = CIMName("Description");

// Properties in CIM_ManagedSystemElement

static const CIMName PROPERTY_INSTALL_DATE = CIMName("InstallDate");
static const CIMName PROPERTY_NAME = CIMName("Name");
static const CIMName PROPERTY_STATUS = CIMName("Status");

// Properties in PG_NextHopIPRoute

static const CIMName PROPERTY_ROUTE_DERIVATION = CIMName("RouteDerivation");
static const CIMName PROPERTY_OTHER_DERIVATION = CIMName("Othererivation");
static const CIMName PROPERTY_DESTINATION_MASK = CIMName("DestinationMask");
static const CIMName PROPERTY_PREFIX_LENGTH = CIMName("PrefixLength");
static const CIMName PROPERTY_ADDRESS_TYPE = CIMName("AddressType");

// Properties in PG_NextHopRoute

static const CIMName PROPERTY_INSTANCE_ID = CIMName("InstanceID");
static const CIMName PROPERTY_DESTINATION_ADDRESS = CIMName(
    "DestinationAddress");
static const CIMName PROPERTY_ADMIN_DISTANCE = CIMName("AdminDistance");
static const CIMName PROPERTY_ROUTE_METRIC = CIMName("RouteMetric");
static const CIMName PROPERTY_IS_STATIC = CIMName("IsStatic");
static const CIMName PROPERTY_TYPE_OF_ROUTE = CIMName("TypeOfRoute");


PEGASUS_USING_STD;
PEGASUS_USING_PEGASUS;


NextHopIPRouteProvider::NextHopIPRouteProvider()
{
#ifdef IPPROVIDER_DEBUG
    cout << "NextHopIPRouteProvider::NextHopIPRouteProvider()" << endl;
#endif
}

NextHopIPRouteProvider::~NextHopIPRouteProvider()
{
}


/*
===============================================================================
NAME              : createInstance
DESCRIPTION       : Create a PG_NextHopIPRoute instance.
ASSUMPTIONS       : None
PRE-CONDITIONS    :
POST-CONDITIONS   :
NOTES             : Currently not supported.
PARAMETERS        :
===============================================================================
*/
void NextHopIPRouteProvider::createInstance(
    const OperationContext &context,
    const CIMObjectPath &instanceName,
    const CIMInstance &instanceObject,
    ObjectPathResponseHandler &handler)

{
    throw CIMNotSupportedException(String::EMPTY);
}

/*
===============================================================================
NAME              : deleteInstance
DESCRIPTION       : Delete a PG_IPRoute instance.
ASSUMPTIONS       : None
PRE-CONDITIONS    :
POST-CONDITIONS   :
NOTES             : Currently not supported.
PARAMETERS        :
===============================================================================
*/
void NextHopIPRouteProvider::deleteInstance(
    const OperationContext &context,
    const CIMObjectPath &instanceReference,
    ResponseHandler &handler)

{
    throw CIMNotSupportedException(String::EMPTY);
}

/*
===============================================================================
NAME              : enumerateInstances.
DESCRIPTION       : Enumerates all of the PG_NextHopIPRoute instances.
                  : An array of instance references is returned.
ASSUMPTIONS       : None.
PRE-CONDITIONS    :
POST-CONDITIONS   :
NOTES             : LocalOnly, DeepInheritance and propertyList are not
                  : respected by this provider. Localization is not supported.
PARAMETERS        :
===============================================================================
*/
void NextHopIPRouteProvider::enumerateInstances(
    const OperationContext & context,
    const CIMObjectPath & classReference,
    const Boolean includeQualifiers,
    const Boolean includeClassOrigin,
    const CIMPropertyList & propertyList,
    InstanceResponseHandler & handler)
{
#ifdef IPPROVIDER_DEBUG
    cout << "NextHopIPRouteProvider::enumerateInstances()" << endl;
#endif

    CIMName className = classReference.getClassName();
    CIMNamespaceName nameSpace = classReference.getNameSpace();

    // Validate the classname.
    _checkClass(className);

    // Notify processing is starting.
    handler.processing();

    NextHopRouteList _nhrList;

    for (Uint16 i = 0; i < _nhrList.size(); i++)
    {
        handler.deliver(
            _constructInstance(
                CLASS_PG_NEXT_HOP_IP_ROUTE,
                nameSpace,
                _nhrList.getRoute(i)));
    }

    // Notify processing is complete.
    handler.complete();

    return;
}

/*
===============================================================================
NAME              : enumerateInstanceNames.
DESCRIPTION       : Enumerates all of the PG_IPRoute instance names.
                  : An array of instance references is returned.
ASSUMPTIONS       : None.
PRE-CONDITIONS    :
POST-CONDITIONS   :
NOTES             : Localization is not supported by this provider.
PARAMETERS        :
===============================================================================
*/
void NextHopIPRouteProvider::enumerateInstanceNames(
    const OperationContext &ctx,
    const CIMObjectPath &ref,
    ObjectPathResponseHandler &handler)
{
#ifdef IPPROVIDER_DEBUG
    cout << "NextHopIPRouteProvider::enumerateInstanceNames()" << endl;
#endif

    CIMName className = ref.getClassName();
    CIMNamespaceName nameSpace = ref.getNameSpace();

    // Validate the classname.
    _checkClass(className);

    // Notify processing is starting.
    handler.processing();

    NextHopRouteList _rList;

#ifdef IPPROVIDER_DEBUG
    cout << "NextHopIPRouteProvider::enumerateInstanceNames() "
         << "_rList Initialized"
         << endl;
#endif

    for (Uint16 i = 0; i < _rList.size(); i++)
    {
        // Deliver the names.
        handler.deliver(
            CIMObjectPath(
                String::EMPTY, // hostname.
                nameSpace,
                CLASS_PG_NEXT_HOP_IP_ROUTE,
                _constructKeyBindings(_rList.getRoute(i))));
    }

    // Notify processing is complete.
    handler.complete();

    return;

}

/*
===============================================================================
NAME              : getInstance.
DESCRIPTION       : Returns a single instance.
ASSUMPTIONS       : None.
PRE-CONDITIONS    :
POST-CONDITIONS   :
NOTES             : LocalOnly, DeepInheritance and propertyList are not
                  : respected by this provider. Localization is not supported.
PARAMETERS        :
===============================================================================
*/
void NextHopIPRouteProvider::getInstance(
    const OperationContext &ctx,
    const CIMObjectPath &instanceName,
    const Boolean includeQualifiers,
    const Boolean includeClassOrigin,
    const CIMPropertyList &propertyList,
    InstanceResponseHandler &handler)
{
#ifdef IPPROVIDER_DEBUG
    cout << "NextHopIPRouteProvider::getInstance(" << instanceName.toString()
         << ")" << endl;
#endif

    CIMKeyBinding kb;
    Uint16 keysFound=0;
    CIMName className = instanceName.getClassName();
    CIMNamespaceName nameSpace = instanceName.getNameSpace();
    String instanceID;    // Instance ID in the format "<OrigID>:<LocalID>"

    // Validate the classname.
    _checkClass(className);

    // Extract the key values.
    Array<CIMKeyBinding> kbArray = instanceName.getKeyBindings();

    // Leave immediately if wrong number of keys.
    if ( kbArray.size() != NUMKEYS_PG_NEXT_HOP_IP_ROUTE )
    {
        throw CIMInvalidParameterException("Wrong number of keys");
    }

    // Validate the keys.
    // Each loop iteration will set a bit in keysFound when a valid
    // key is found. If the expected bits aren't all set when
    // the loop finishes, it's a problem.
    for (Uint16 i=0; i < NUMKEYS_PG_NEXT_HOP_IP_ROUTE; i++)
    {
        keysFound=0;
        kb = kbArray[i];

        CIMName keyName = kb.getName();
        String keyValue = kb.getValue();

        // IP Destination Address, IP Destination Mask, and Address Type
        // must be valid, but we will know that later.
        // For now, just verify that they're present, and record them.

        // IPDestinationAddress must be present.
        if (keyName.equal (PROPERTY_INSTANCE_ID))
        {
            instanceID = keyValue;
            keysFound |= 1;
        }
        else  // Key name was not recognized by any of the above tests.
        {
            throw CIMInvalidParameterException(
                keyName.getString() + ": Unrecognized key");
        }

    } // for

    // We could get here if we didn't get all the keys, which
    // could happen if the right number of keys were supplied,
    // and they all had valid names and values, but there were
    // any duplicates (e.g., two Names, no SystemName)
    if (keysFound != (1 << NUMKEYS_PG_NEXT_HOP_IP_ROUTE) - 1)
    {
        throw CIMInvalidParameterException("Bad object name");
    }

    // Find the instance.  First convert the instance id which is the
    // process handle to an integer.  This is necessary because the
    // handle is the process id on HP-UX which must be passed to
    // pstat_getproc() as an integer.

    // Get the Route List.
    NextHopRouteList _nhrList;
    NextHopIPRoute _nhipr;

    if (_nhrList.findRoute(instanceID, _nhipr))
    {
        // Notify processing is starting.
        handler.processing();

        // Return the instance.
        handler.deliver(_constructInstance(className, nameSpace, _nhipr));

        // Notify processing is complete.
        handler.complete();
        return;
    }

    throw CIMObjectNotFoundException(
        "InstanceID=" + instanceID + ": No such IP Route");

    return; // should never execute, but required to keep compiler happy.
}

/*
===============================================================================
NAME              : modifyInstance.
DESCRIPTION       : Modify a PG_NextHopIPRoute instance.
ASSUMPTIONS       : None.
PRE-CONDITIONS    :
POST-CONDITIONS   :
NOTES             : Currently not supported.
PARAMETERS        :
===============================================================================
*/
void NextHopIPRouteProvider::modifyInstance(
    const OperationContext &context,
    const CIMObjectPath &instanceName,
    const CIMInstance &instanceObject,
    const Boolean includeQualifiers,
    const CIMPropertyList &propertyList,
    ResponseHandler &handler)
{
    // Could be supported in the future for certain properties.
    throw CIMNotSupportedException(String::EMPTY);
}

/*
===============================================================================
NAME              : initialize.
DESCRIPTION       : Initializes the provider.
ASSUMPTIONS       : None.
PRE-CONDITIONS    :
POST-CONDITIONS   :
NOTES             :
PARAMETERS        :
===============================================================================
*/
void NextHopIPRouteProvider::initialize(CIMOMHandle &ch)
{
#ifdef IPPROVIDER_DEBUG
    cout << "NextHopIPRouteProvider::initialize()" << endl;
#endif

    _cimomHandle = ch;

    // call platform-specific routine to initialize System Name.
    // This hides whether or not the platform-specific code wants
    // to set up a static System Name or dynamically obtain it
    // each time.
    IPInterface::initSystemName();

    return;
}

/*
===============================================================================
NAME              : terminate.
DESCRIPTION       : Terminates the provider.
ASSUMPTIONS       : None.
PRE-CONDITIONS    :
POST-CONDITIONS   :
NOTES             :
PARAMETERS        :
===============================================================================
*/
void NextHopIPRouteProvider::terminate()
{
#ifdef IPPROVIDER_DEBUG
    cout << "NextHopIPRouteProvider::terminate()" << endl;
#endif
    delete this;
}

/*
===============================================================================
NAME              : _constructKeyBindings.
DESCRIPTION       : Constructs an array of keybindings for an IP Route.
ASSUMPTIONS       : None.
PRE-CONDITIONS    :
POST-CONDITIONS   :
NOTES             :
PARAMETERS        : Next Hop IP Route.
===============================================================================
*/
Array<CIMKeyBinding> NextHopIPRouteProvider::_constructKeyBindings(
    const NextHopIPRoute& _nhipr)
{
#ifdef IPPROVIDER_DEBUG
    cout << "NextHopIPRouteProvider::_constructKeyBindings()" << endl;
#endif

    Array<CIMKeyBinding> keyBindings;
    String s;

    // Construct the key bindings

    if (_nhipr.getInstanceID(s))
    {
        keyBindings.append(CIMKeyBinding(
            PROPERTY_INSTANCE_ID,
            s,
            CIMKeyBinding::STRING));
    }
    else
    {
        throw CIMNotSupportedException(
            String(
                "Host-specific module doesn't support Key `") +
                PROPERTY_INSTANCE_ID.getString() +
                String("'"));
    }

#ifdef IPPROVIDER_DEBUG
    cout << "NextHopIPRouteProvide::_constructKeyBindings() -- done" << endl;
#endif

    return keyBindings;
}

/*
===============================================================================
NAME              : _constructInstance.
DESCRIPTION       : Constructs instance by adding its properties. The
                  : Next Hop IP Route argument has already been filled in
                  : with data from an existing IP Route.
ASSUMPTIONS       : None.
PRE-CONDITIONS    :
POST-CONDITIONS   :
NOTES             :
PARAMETERS        : className, nameSpace, Next Hop IP Route.
===============================================================================
*/
CIMInstance NextHopIPRouteProvider::_constructInstance(
    const CIMName &className,
    const CIMNamespaceName &nameSpace,
    const NextHopIPRoute &_nhipr)
{
#ifdef IPPROVIDER_DEBUG
    cout << "NextHopIPRouteProvider::_constructInstance()" << endl;
#endif

    String s;
    CIMDateTime d;
    Boolean b;
    Uint16 ui;
    Uint8 i;

    CIMInstance inst(className);

    // Set path.
    inst.setPath(CIMObjectPath(
        String::EMPTY, // hostname
        nameSpace,
        CLASS_PG_NEXT_HOP_IP_ROUTE,
        _constructKeyBindings(_nhipr)));

    // CIM_ManagedElement.

    // string Caption.
    if (_nhipr.getCaption(s))
    {
        inst.addProperty(CIMProperty(PROPERTY_CAPTION,s));
    }

    // string Description.
    if (_nhipr.getDescription(s))
    {
        inst.addProperty(CIMProperty(PROPERTY_DESCRIPTION,s));
    }

    // CIM_ManagedSystemElement.

    // datetime InstallDate.
    if (_nhipr.getInstallDate(d))
    {
        inst.addProperty(CIMProperty(PROPERTY_INSTALL_DATE,d));
    }

    // string Name.
    if (_nhipr.getName(s))
    {
        inst.addProperty(CIMProperty(PROPERTY_NAME,s));
    }

    // string Status.
    if (_nhipr.getStatus(s))
    {
        inst.addProperty(CIMProperty(PROPERTY_STATUS,s));
    }

    // CIM_LogicalElement.

    // No local properties added in this class.

    // CIM_NextHopRoute.

    // String DestinationAddress.
    if (_nhipr.getDestinationAddress(s))
    {
        inst.addProperty(CIMProperty(PROPERTY_DESTINATION_ADDRESS,s));
    }

    // String DestinationMask.
    if (_nhipr.getDestinationMask(s))
    {
        inst.addProperty(CIMProperty(PROPERTY_DESTINATION_MASK,s));
    }

    // Uint8 PrefixLength.
    if (_nhipr.getPrefixLength(i))
    {
        inst.addProperty(CIMProperty(PROPERTY_PREFIX_LENGTH,i));
    }

    // String IsStatic.
    if (_nhipr.getIsStatic(b))
    {
        inst.addProperty(CIMProperty(PROPERTY_IS_STATIC,b));
    }

    // Uint16 AddressType.
    if (_nhipr.getAddressType(ui))
    {
        inst.addProperty(CIMProperty(PROPERTY_ADDRESS_TYPE,ui));
    }

#ifdef IPPROVIDER_DEBUG
    cout << "NextHopIPRouteProvider::_constructInstance() -- done" << endl;
#endif

    return inst;
}

/*
===============================================================================
NAME              : _checkClass.
DESCRIPTION       : Tests the argument for valid classname.
                  : Throws exception if not.
ASSUMPTIONS       : None.
PRE-CONDITIONS    :
POST-CONDITIONS   :
NOTES             :
PARAMETERS        : className.
===============================================================================
*/
void NextHopIPRouteProvider::_checkClass(CIMName& className)
{
    if (!className.equal (CLASS_PG_NEXT_HOP_IP_ROUTE))
    {
        throw CIMNotSupportedException(
            className.getString() + ": Class not supported");
    }
}
