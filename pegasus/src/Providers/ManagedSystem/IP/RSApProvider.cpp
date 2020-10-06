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

#include <Providers/ManagedSystem/IP/RSApProvider.h>

//=============================================================================
// Class names:
//
// We use CIM_UnitaryComputerSystem as the value of the key
// SystemCreationClassName, because this class has properties that
// are important for clients of this provider.
//=============================================================================

static const CIMName CLASS_CIM_SERVICE_ACCESS_POINT = CIMName (
    "CIM_ServiceAccessPoint");
static const CIMName CLASS_PG_REMOTE_SERVICE_ACCESS_POINT = CIMName (
    "PG_RemoteServiceAccessPoint");

//=============================================================================
// The number of keys for the classes.
//=============================================================================

#define NUMKEYS_CIM_SERVICE_ACCESS_POINT  1

//=============================================================================
// Property names.  These values are returned by the provider as
// the property names.
//=============================================================================

// Properties in CIM_ServiceAccessPoint

static const CIMName PROPERTY_NAME = CIMName("Name");

// Properties in PG_RemoteServiceAccessPoint

static const CIMName PROPERTY_ACCESS_INFO = CIMName("AccessInfo");
static const CIMName PROPERTY_INFO_FORMAT = CIMName("InfoFormat");
static const CIMName PROPERTY_OTHER_INFO_FORMAT_DESCRIPTION = CIMName(
    "OtherInfoFormatDescription");

PEGASUS_USING_STD;
PEGASUS_USING_PEGASUS;


RSApProvider::RSApProvider()
{
#ifdef IPPROVIDER_DEBUG
    cout << "RSApProvider::RSApProvider()" << endl;
#endif
}

RSApProvider::~RSApProvider()
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
void RSApProvider::createInstance(
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
void RSApProvider::deleteInstance(
    const OperationContext &context,
    const CIMObjectPath &instanceReference,
    ResponseHandler &handler)

{
    throw CIMNotSupportedException(String::EMPTY);
}

/*
===============================================================================
NAME              : enumerateInstances.
DESCRIPTION       : Enumerates all the PG_RemoteServiceAccessPoint instances.
                  : An array of instance references is returned.
ASSUMPTIONS       : None.
PRE-CONDITIONS    :
POST-CONDITIONS   :
NOTES             : LocalOnly, DeepInheritance and propertyList are not
                  : respected by this provider. Localization is not supported.
PARAMETERS        :
===============================================================================
*/
void RSApProvider::enumerateInstances(
    const OperationContext & context,
    const CIMObjectPath & classReference,
    const Boolean includeQualifiers,
    const Boolean includeClassOrigin,
    const CIMPropertyList & propertyList,
    InstanceResponseHandler & handler)
{
    // Could be supported in the future for certain properties.
    //throw CIMNotSupportedException(String::EMPTY);


#ifdef IPPROVIDER_DEBUG
    cout << "RSApProvider::enumerateInstances()" << endl;
#endif

    CIMName className = classReference.getClassName();
    CIMNamespaceName nameSpace = classReference.getNameSpace();

    // Validate the classname.
    _checkClass(className);

    // Notify processing is starting.
    handler.processing();

    RSApList _rsapList;

    for (Uint16 i = 0; i < _rsapList.size(); i++)
    {
        handler.deliver(
            _constructInstance(
                CLASS_PG_REMOTE_SERVICE_ACCESS_POINT,
                nameSpace,
                _rsapList.getService(i)));
    }

    // Notify processing is complete.
    handler.complete();

    return;
}

/*
===============================================================================
NAME              : enumerateInstanceNames.
DESCRIPTION       : Enumerates all of the PG_RemoteServiceAccessPoint instance
                  : names. An array of instance references is returned.
ASSUMPTIONS       : None.
PRE-CONDITIONS    :
POST-CONDITIONS   :
NOTES             : Localization is not supported by this provider.
PARAMETERS        :
===============================================================================
*/
void RSApProvider::enumerateInstanceNames(
    const OperationContext &ctx,
    const CIMObjectPath &ref,
    ObjectPathResponseHandler &handler)
{

#ifdef IPPROVIDER_DEBUG
    cout << "RSApProvider::enumerateInstanceNames()" << endl;
#endif

    CIMName className = ref.getClassName();
    CIMNamespaceName nameSpace = ref.getNameSpace();

    // Validate the classname.
    _checkClass(className);

    // Notify processing is starting.
    handler.processing();

    RSApList _rList;

#ifdef IPPROVIDER_DEBUG
    cout << "RSApProvider::enumerateInstanceNames() "
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
                CLASS_PG_REMOTE_SERVICE_ACCESS_POINT,
                _constructKeyBindings(_rList.getService(i))));
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
void RSApProvider::getInstance(
    const OperationContext &ctx,
    const CIMObjectPath &instanceName,
    const Boolean includeQualifiers,
    const Boolean includeClassOrigin,
    const CIMPropertyList &propertyList,
    InstanceResponseHandler &handler)
{

#ifdef IPPROVIDER_DEBUG
    cout << "RSApProvider::getInstance(" << instanceName.toString()
         << ")" << endl;
#endif

    CIMKeyBinding kb;
    Uint16 keysFound=0;
    CIMName className = instanceName.getClassName();
    CIMNamespaceName nameSpace = instanceName.getNameSpace();
    String Name;    // Property Name

    // Validate the classname.
    _checkClass(className);

    // Extract the key values.
    Array<CIMKeyBinding> kbArray = instanceName.getKeyBindings();

    // Leave immediately if wrong number of keys.
    if (kbArray.size() != NUMKEYS_CIM_SERVICE_ACCESS_POINT)
    {
        throw CIMInvalidParameterException("Wrong number of keys");
    }

    // Validate the keys.
    // Each loop iteration will set a bit in keysFound when a valid
    // key is found. If the expected bits aren't all set when
    // the loop finishes, it's a problem.
    for (Uint16 i=0; i < NUMKEYS_CIM_SERVICE_ACCESS_POINT; i++)
    {
        keysFound=0;
        kb = kbArray[i];

        CIMName keyName = kb.getName();
        String keyValue = kb.getValue();

        // IP Destination Address, IP Destination Mask, and Address Type
        // must be valid, but we will know that later.
        // For now, just verify that they're present, and record them.

        // Name property must be present.
        if (keyName.equal (PROPERTY_NAME))
        {
            Name = keyValue;
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
    if (keysFound != (1 << NUMKEYS_CIM_SERVICE_ACCESS_POINT) - 1)
    {
        throw CIMInvalidParameterException("Bad object name");
    }

    // Find the instance.  First convert the instance id which is the
    // process handle to an integer.  This is necessary because the
    // handle is the process id on HP-UX which must be passed to
    // pstat_getproc() as an integer.

    // Get the Route List.
    RSApList _rsapl;
    RSAp _rsap;

    if (_rsapl.findService(Name, _rsap))
    {
        // Notify processing is starting.
        handler.processing();

        // Return the instance.
        handler.deliver(_constructInstance(className, nameSpace, _rsap));

        // Notify processing is complete.
        handler.complete();
        return;
    }

    throw CIMObjectNotFoundException(
        "Name=" + Name + ": No such Remote Service Access Point");

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
void RSApProvider::modifyInstance(
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
void RSApProvider::initialize(CIMOMHandle &ch)
{
#ifdef IPPROVIDER_DEBUG
    cout << "RSApProvider::initialize()" << endl;
#endif

    _cimomHandle = ch;

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
void RSApProvider::terminate()
{
#ifdef IPPROVIDER_DEBUG
    cout << "RSApProvider::terminate()" << endl;
#endif

    delete this;
}

/*
===============================================================================
NAME              : _constructKeyBindings.
DESCRIPTION       : Constructs an array of keybindings for an Remote Service
                  : Access Point.
ASSUMPTIONS       : None.
PRE-CONDITIONS    :
POST-CONDITIONS   :
NOTES             :
PARAMETERS        : Remote Service Access Point.
===============================================================================
*/
Array<CIMKeyBinding> RSApProvider::_constructKeyBindings(
    const RSAp& _rsap)
{
#ifdef IPPROVIDER_DEBUG
    cout << "RSApProvider::_constructKeyBindings()" << endl;
#endif

    Array<CIMKeyBinding> keyBindings;
    String s;

    // Construct the key bindings

    if (_rsap.getName(s))
    {
        keyBindings.append(CIMKeyBinding(
            PROPERTY_NAME,
            s,
            CIMKeyBinding::STRING));
    }
    else
    {
        throw CIMNotSupportedException(
            String(
                "Host-specific module doesn't support Key `") +
                PROPERTY_NAME.getString() +
                String("'"));
    }

#ifdef IPPROVIDER_DEBUG
    cout << "RSApProvider::_constructKeyBindings() -- done" << endl;
#endif

    return keyBindings;
}

/*
===============================================================================
NAME              : _constructInstance.
DESCRIPTION       : Constructs instance by adding its properties.
ASSUMPTIONS       : None.
PRE-CONDITIONS    :
POST-CONDITIONS   :
NOTES             :
PARAMETERS        : className, nameSpace, Remote Service Access Point.
===============================================================================
*/
CIMInstance RSApProvider::_constructInstance(
    const CIMName &className,
    const CIMNamespaceName &nameSpace,
    const RSAp &_rsap)
{
#ifdef IPPROVIDER_DEBUG
    cout << "RSApProvider::_constructInstance()" << endl;
#endif

    String s;
    Uint16 ui;

    CIMInstance inst(className);

    // Set path.
    inst.setPath(CIMObjectPath(
        String::EMPTY, // hostname
        nameSpace,
        CLASS_PG_REMOTE_SERVICE_ACCESS_POINT,
        _constructKeyBindings(_rsap)));

    // PG_RemoteServiceAccessPoint class.

    // string AccessInfo property.
    if (_rsap.getAccessInfo(s))
    {
        inst.addProperty(CIMProperty(PROPERTY_ACCESS_INFO,s));
    }

    // string InfoFormat property.
    if (_rsap.getInfoFormat(ui))
    {
        inst.addProperty(CIMProperty(PROPERTY_INFO_FORMAT,ui));
    }

    // String OtherInfoFormatDescription property.
    if (_rsap.getOtherInfoFormatDescription(s))
    {
        inst.addProperty(CIMProperty(PROPERTY_OTHER_INFO_FORMAT_DESCRIPTION,s));
    }

#ifdef IPPROVIDER_DEBUG
    cout << "RSApProvider::_constructInstance() -- done" << endl;
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
void RSApProvider::_checkClass(CIMName& className)
{
    if (!className.equal (CLASS_PG_REMOTE_SERVICE_ACCESS_POINT))
    {
        throw CIMNotSupportedException(
            className.getString() + ": Class not supported");
    }
}
