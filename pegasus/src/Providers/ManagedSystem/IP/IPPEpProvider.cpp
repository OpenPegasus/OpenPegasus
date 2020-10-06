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


// ==========================================================================
// Includes.
// ==========================================================================

#include "IPPEpProvider.h"

// ==========================================================================
// Class names:
//
// We use CIM_UnitaryComputerSystem as the value of the key
// SystemCreationClassName, because this class has properties that
// are important for clients of this provider.
// ==========================================================================

static const CIMName CLASS_CIM_UNITARY_COMPUTER_SYSTEM = CIMName(
    "CIM_UnitaryComputerSystem");
static const CIMName CLASS_CIM_IP_PROTOCOL_ENDPOINT    = CIMName(
    "CIM_IPProtocolEndpoint");

// ==========================================================================
// The number of keys for the classes.
// ==========================================================================

#define NUMKEYS_IP_PROTOCOL_ENDPOINT          4

// ==========================================================================
// Property names.  These values are returned by the provider as
// the property names.
// ==========================================================================

// Properties in CIM_ManagedElement

static const CIMName PROPERTY_CAPTION                    = CIMName("Caption");
static const CIMName PROPERTY_DESCRIPTION                = CIMName(
    "Description");

// Properties in CIM_ManagedSystemElement

static const CIMName PROPERTY_INSTALL_DATE               = CIMName(
    "InstallDate");
static const CIMName PROPERTY_NAME                       = CIMName("Name");
static const CIMName PROPERTY_STATUS                     = CIMName("Status");

// Properties in CIM_ServiceAccessPoint

static const CIMName PROPERTY_CREATION_CLASS_NAME        = CIMName(
    "CreationClassName");
static const CIMName PROPERTY_SYSTEM_CREATION_CLASS_NAME = CIMName(
    "SystemCreationClassName");
static const CIMName PROPERTY_SYSTEM_NAME                = CIMName(
    "SystemName");

// Properties in CIM_ProtocolEndpoint

static const CIMName PROPERTY_NAME_FORMAT                = CIMName(
    "NameFormat");
static const CIMName PROPERTY_PROTOCOL_TYPE              = CIMName(
    "ProtocolType");
static const CIMName PROPERTY_OTHER_TYPE_DESCRIPTION     = CIMName(
    "OtherTypeDescription");

// Properties in CIM_IProtocolEndpoint

static const CIMName PROPERTY_ADDRESS                    = CIMName("Address");
static const CIMName PROPERTY_IPV4_ADDRESS = CIMName("IPv4Address");
static const CIMName PROPERTY_IPV6_ADDRESS = CIMName("IPv6Address");
static const CIMName PROPERTY_PREFIX_LENGTH = CIMName("PrefixLength");
static const CIMName PROPERTY_SUBNET_MASK                = CIMName(
    "SubnetMask");
static const CIMName PROPERTY_ADDRESS_TYPE               = CIMName(
    "AddressType");
static const CIMName PROPERTY_IP_VERSION_SUPPORT         = CIMName(
    "IPVersionSupport");
static const CIMName PROPERTY_PROTOCOL_IF_TYPE = CIMName("ProtocolIFType");

PEGASUS_USING_STD;
PEGASUS_USING_PEGASUS;


IPPEpProvider::IPPEpProvider()
{
#ifdef IPPROVIDER_DEBUG
    cout << "IPPEpProvider::IPPEpProvider()" << endl;
#endif
}

IPPEpProvider::~IPPEpProvider()
{
}


/*
================================================================================
NAME              : createInstance
DESCRIPTION       : Create a IPProtocolEndpoint instance.
ASSUMPTIONS       : None
PRE-CONDITIONS    :
POST-CONDITIONS   :
NOTES             : Currently not supported.
PARAMETERS        :
================================================================================
*/
void IPPEpProvider::createInstance(
    const OperationContext &context,
    const CIMObjectPath &instanceName,
    const CIMInstance &instanceObject,
    ObjectPathResponseHandler &handler)

{
    throw CIMNotSupportedException(String::EMPTY);
}

/*
================================================================================
NAME              : deleteInstance
DESCRIPTION       : Delete a IPProtocolEndpoint instance.
ASSUMPTIONS       : None
PRE-CONDITIONS    :
POST-CONDITIONS   :
NOTES             : Currently not supported.
PARAMETERS        :
================================================================================
*/
void IPPEpProvider::deleteInstance(
    const OperationContext &context,
    const CIMObjectPath &instanceReference,
    ResponseHandler &handler)

{
    throw CIMNotSupportedException(String::EMPTY);
}

/*
================================================================================
NAME              : enumerateInstances
DESCRIPTION       : Enumerates all of the IPProtocolEndpoint instances.
                  : An array of instance references is returned.
ASSUMPTIONS       : None
PRE-CONDITIONS    :
POST-CONDITIONS   :
NOTES             : LocalOnly, DeepInheritance and propertyList are not
                  : respected by this provider. Localization is not supported
PARAMETERS        :
================================================================================
*/
void IPPEpProvider::enumerateInstances(
    const OperationContext & context,
    const CIMObjectPath & classReference,
    const Boolean includeQualifiers,
    const Boolean includeClassOrigin,
    const CIMPropertyList & propertyList,
    InstanceResponseHandler & handler)
{
#ifdef IPPROVIDER_DEBUG
    cout << "IPPEpProvider::enumerateInstances()" << endl;
#endif

    CIMName className = classReference.getClassName();
    CIMNamespaceName nameSpace = classReference.getNameSpace();

    // Validate the classname
    _checkClass(className);

    // Notify processing is starting
    handler.processing();

    int i;
    InterfaceList _ifList;

    for (i = 0; i < _ifList.size(); i++)
    {
        handler.deliver(
            _constructInstance(CLASS_CIM_IP_PROTOCOL_ENDPOINT,
                nameSpace, _ifList.getInterface(i) ) );
    }

    // Notify processing is complete
    handler.complete();

    return;
}  // enumerateInstances

/*
================================================================================
NAME              : enumerateInstanceNames
DESCRIPTION       : Enumerates all of the IPProtocolEndpoint instance names.
                  : An array of instance references is returned.
ASSUMPTIONS       : None
PRE-CONDITIONS    :
POST-CONDITIONS   :
NOTES             : Localization is not supported by this provider.
PARAMETERS        :
================================================================================
*/
void IPPEpProvider::enumerateInstanceNames(
    const OperationContext &ctx,
    const CIMObjectPath &ref,
    ObjectPathResponseHandler &handler)
{
#ifdef IPPROVIDER_DEBUG
    cout << "IPPEpProvider::enumerateInstanceNames()" << endl;
#endif

    CIMName className = ref.getClassName();
    CIMNamespaceName nameSpace = ref.getNameSpace();

    // Validate the classname
    _checkClass(className);

    // Notify processing is starting
    handler.processing();

    int i;
    InterfaceList _ifList;

#ifdef IPPROVIDER_DEBUG
    cout << "IPPEpProvider::enumerateInstanceNames() _ifList Initialized"
         << endl;
#endif

    for (i = 0; i < _ifList.size(); i++)
    {
        // Deliver the names
        handler.deliver(
            CIMObjectPath(String::EMPTY, // hostname
                nameSpace, CLASS_CIM_IP_PROTOCOL_ENDPOINT,
                _constructKeyBindings(_ifList.getInterface(i) ) ) );
    }

    // Notify processing is complete
    handler.complete();

    return;

}  // enumerateInstanceNames

/*
================================================================================
NAME              : getInstance
DESCRIPTION       : Returns a single instance.
ASSUMPTIONS       : None
PRE-CONDITIONS    :
POST-CONDITIONS   :
NOTES             : LocalOnly, DeepInheritance and propertyList are not
                  : respected by this provider. Localization is not supported
PARAMETERS        :
================================================================================
*/
void IPPEpProvider::getInstance(
    const OperationContext &ctx,
    const CIMObjectPath &instanceName,
    const Boolean includeQualifiers,
    const Boolean includeClassOrigin,
    const CIMPropertyList &propertyList,
    InstanceResponseHandler &handler)
{
#ifdef IPPROVIDER_DEBUG
    cout << "IPPEpProvider::getInstance(" << instanceName.toString() << ")"
         << endl;
#endif

    CIMKeyBinding kb;
    CIMName className = instanceName.getClassName();
    CIMNamespaceName nameSpace = instanceName.getNameSpace();
    int i;
    int keysFound; // this will be used as a bit array
    String sn;     // system name
    String ifName;

    // Grab the system name
    if (IPInterface::getSystemName(sn) == false)
        sn = String::EMPTY;

    // Validate the classname
    _checkClass(className);

    // Extract the key values
    Array<CIMKeyBinding> kbArray = instanceName.getKeyBindings();

    // Leave immediately if wrong number of keys
    if ( kbArray.size() != NUMKEYS_IP_PROTOCOL_ENDPOINT )
        throw CIMInvalidParameterException("Wrong number of keys");

    // Validate the keys.
    // Each loop iteration will set a bit in keysFound when a valid
    // key is found. If the expected bits aren't all set when
    // the loop finishes, it's a problem
    for (i=0, keysFound=0; i < NUMKEYS_IP_PROTOCOL_ENDPOINT; i++)
    {
        kb = kbArray[i];

        CIMName keyName = kb.getName();
        String keyValue = kb.getValue();

        // SystemCreationClassName can be empty or must match
        if (keyName.equal (PROPERTY_SYSTEM_CREATION_CLASS_NAME))
        {
            if (String::equal(keyValue, String::EMPTY) ||
                String::equalNoCase(keyValue,
                    CLASS_CIM_UNITARY_COMPUTER_SYSTEM.getString()))
            {
                keysFound |= 1;
            }
            else
            {
                throw CIMInvalidParameterException(
                    keyValue + ": bad value for key " + keyName.getString());
            }
        }

        // SystemName can be empty or must match
        else if (keyName.equal (PROPERTY_SYSTEM_NAME))
        {
            if (String::equal(keyValue, String::EMPTY) ||
                String::equalNoCase(keyValue, sn))
            {
                keysFound |= 2;
            }
            else
            {
                throw CIMInvalidParameterException(
                    keyValue + ": bad value for key " + keyName.getString());
            }
        }

        // CreationClassName can be empty or must match
        else if (keyName.equal (PROPERTY_CREATION_CLASS_NAME))
        {
            if (String::equal(keyValue, String::EMPTY) ||
                String::equalNoCase(keyValue,
                    CLASS_CIM_IP_PROTOCOL_ENDPOINT.getString()))
            {
                keysFound |= 4;
            }
            else
            {
                throw CIMInvalidParameterException(
                    keyValue + ": bad value for key " + keyName.getString());
            }
        }

        // Name must be a valid IP interface, but we will know that later
        // For now, just verify that it's present
        else if (keyName.equal (PROPERTY_NAME))
        {
            ifName = keyValue;
            keysFound |= 8;
        }

        // Key name was not recognized by any of the above tests
        else
            throw CIMInvalidParameterException(
                keyName.getString() + ": Unrecognized key");
    } // for

    // We could get here if we didn't get all the keys, which
    // could happen if the right number of keys were supplied,
    // and they all had valid names and values, but there were
    // any duplicates (e.g., two Names, no SystemName)
    if (keysFound != (1<<NUMKEYS_IP_PROTOCOL_ENDPOINT)-1)
        throw CIMInvalidParameterException("Bad object name");

    /* Find the instance.  First convert the instance id which is the */
    /* process handle to an integer.  This is necessary because the   */
    /* handle is the process id on HP-UX which must be passed to      */
    /* pstat_getproc() as an integer.                                 */

    /* Get the Interface List. */
    InterfaceList _ifList;
    IPInterface _ipif;

    if (_ifList.findInterface(ifName, _ipif))
    {
        /* Notify processing is starting. */
        handler.processing();

        /* Return the instance. */
        handler.deliver(_constructInstance(className, nameSpace, _ipif));

        /* Notify processing is complete. */
        handler.complete();
        return;
    }

    throw CIMObjectNotFoundException(ifName+": No such IP Interface");

    return; // can never execute, but required to keep compiler happy
}

/*
================================================================================
NAME              : modifyInstance
DESCRIPTION       : Modify a IPProtocolEndpoint instance.
ASSUMPTIONS       : None
PRE-CONDITIONS    :
POST-CONDITIONS   :
NOTES             : Currently not supported.
PARAMETERS        :
================================================================================
*/
void IPPEpProvider::modifyInstance(
    const OperationContext &context,
    const CIMObjectPath &instanceName,
    const CIMInstance &instanceObject,
    const Boolean includeQualifiers,
    const CIMPropertyList &propertyList,
    ResponseHandler &handler)
{
    // Could be supported in the future for certain properties
    throw CIMNotSupportedException(String::EMPTY);
}

/*
================================================================================
NAME              : initialize
DESCRIPTION       : Initializes the provider.
ASSUMPTIONS       : None
PRE-CONDITIONS    :
POST-CONDITIONS   :
NOTES             :
PARAMETERS        :
================================================================================
*/
void IPPEpProvider::initialize(CIMOMHandle &ch)
{
#ifdef IPPROVIDER_DEBUG
    cout << "IPPEpProvider::initialize()" << endl;
#endif

    _cimomHandle = ch;

    // call platform-specific routine to initialize System Name
    // This hides whether or not the platform-specific code wants
    // to set up a static System Name or dynamically obtain it
    // each time.
    IPInterface::initSystemName();

    return;

}  /* initialize */

/*
================================================================================
NAME              : terminate
DESCRIPTION       : Terminates the provider.
ASSUMPTIONS       : None
PRE-CONDITIONS    :
POST-CONDITIONS   :
NOTES             :
PARAMETERS        :
================================================================================
*/
void IPPEpProvider::terminate()
{
#ifdef IPPROVIDER_DEBUG
    cout << "IPPEpProvider::terminate()" << endl;
#endif

    delete this;
}

/*
================================================================================
NAME              : _constructKeyBindings
DESCRIPTION       : Constructs an array of keybindings for an IP Interface
ASSUMPTIONS       : None
PRE-CONDITIONS    :
POST-CONDITIONS   :
NOTES             :
PARAMETERS        : IP Interface
================================================================================
*/
Array<CIMKeyBinding> IPPEpProvider::_constructKeyBindings(
    const IPInterface& _ipif)
{
#ifdef IPPROVIDER_DEBUG
    cout << "IPPEpProvider::_constructKeyBindings()" << endl;
#endif

    Array<CIMKeyBinding> keyBindings;
    String s;


    // Construct the key bindings
    keyBindings.append(CIMKeyBinding(
        PROPERTY_SYSTEM_CREATION_CLASS_NAME,
        CLASS_CIM_UNITARY_COMPUTER_SYSTEM.getString(),
        CIMKeyBinding::STRING));

    keyBindings.append(CIMKeyBinding(
        PROPERTY_CREATION_CLASS_NAME,
        CLASS_CIM_IP_PROTOCOL_ENDPOINT.getString(),
        CIMKeyBinding::STRING));

    if (_ipif.getSystemName(s))
    {
        keyBindings.append(CIMKeyBinding(
            PROPERTY_SYSTEM_NAME, s, CIMKeyBinding::STRING));
    }
    else
    {
        throw CIMNotSupportedException(
            String("Host-specific module doesn't support Key `") +
                PROPERTY_SYSTEM_NAME.getString() + String("'"));
    }

    if (_ipif.getName(s))
        keyBindings.append(CIMKeyBinding(
            PROPERTY_NAME, s, CIMKeyBinding::STRING));
    else
        throw CIMNotSupportedException(
            String("Host-specific module doesn't support Key `") +
                PROPERTY_NAME.getString() + String("'"));

#ifdef IPPROVIDER_DEBUG
    cout << "IPPEpProvider::_constructKeyBindings() -- done" << endl;
#endif

    return keyBindings;
}

/*
================================================================================
NAME              : _constructInstance
DESCRIPTION       : Constructs instance by adding its properties. The
                  : IP Interface argument has already been filled in
                  : with data from an existing IP Interface
ASSUMPTIONS       : None
PRE-CONDITIONS    :
POST-CONDITIONS   :
NOTES             :
PARAMETERS        : className, nameSpace, IP Interface
================================================================================
*/
CIMInstance IPPEpProvider::_constructInstance(
    const CIMName &className,
    const CIMNamespaceName &nameSpace,
    const IPInterface &_ipif)
{
#ifdef IPPROVIDER_DEBUG
  cout << "IPPEpProvider::_constructInstance()" << endl;
#endif

  String s;
  Uint16 i16;
  Uint8 i8;
  CIMDateTime d;

  CIMInstance inst(className);

  // Set path

  inst.setPath(CIMObjectPath(String::EMPTY, // hostname
                             nameSpace,
                             CLASS_CIM_IP_PROTOCOL_ENDPOINT,
                             _constructKeyBindings(_ipif)));

// CIM_ManagedElement

//   string Caption
  if (_ipif.getCaption(s))
    inst.addProperty(CIMProperty(PROPERTY_CAPTION,s));

//   string Description
  if (_ipif.getDescription(s))
    inst.addProperty(CIMProperty(PROPERTY_DESCRIPTION,s));

// CIM_ManagedSystemElement

//   datetime InstallDate
  if (_ipif.getInstallDate(d))
    inst.addProperty(CIMProperty(PROPERTY_INSTALL_DATE,d));

//   string Name    // Overridden in CIM_ServiceAccessPoint

//   string Status
  if (_ipif.getStatus(s))
    inst.addProperty(CIMProperty(PROPERTY_STATUS,s));

// CIM_LogicalElement
//   ** No local properties added in this class **

// ======================================================
// The following properties are in CIM_ServiceAccessPoint
// ======================================================

  // The keys for this class are:
  // [ key ] string SystemCreationClassName
  // [ key ] string SystemName
  // [ key ] string CreationClassName
  // [ key ] string Name

  // Rather than rebuilding the key properties, we will reuse
  // the values that were inserted for us in the ObjectPath,
  // trusting that this was done correctly

  // Get the keys
  Array<CIMKeyBinding> key = inst.getPath().getKeyBindings();
  // loop through keys, inserting them as properties
  // luckily, all keys for this class are strings, so no
  // need to check key type
  for (Uint32 i=0; i<key.size(); i++)
  {
    // add a property created from the name and value
    inst.addProperty(CIMProperty(key[i].getName(),key[i].getValue()));
  }

// CIM_ProtocolEndpoint

//   string NameFormat
  if (_ipif.getNameFormat(s))
    inst.addProperty(CIMProperty(PROPERTY_NAME_FORMAT,s));

//   uint16 ProtocolType
  if (_ipif.getProtocolType(i16))
    inst.addProperty(CIMProperty(PROPERTY_PROTOCOL_TYPE,i16));

//   string OtherTypeDescription
  if (_ipif.getOtherTypeDescription(s))
  {
    // if an empty string was returned, the value must be set to NULL
    // with type string, not an array
    if (String::equal(s,String::EMPTY))
    {
      inst.addProperty(CIMProperty(PROPERTY_OTHER_TYPE_DESCRIPTION,
                                   CIMValue(CIMTYPE_STRING, false)));
    }
    else
    {
      inst.addProperty(CIMProperty(PROPERTY_OTHER_TYPE_DESCRIPTION,s));
    }
  }

// CIM_IPProtocolEndpoint

//   string Address
  if (_ipif.getAddress(s))
    inst.addProperty(CIMProperty(PROPERTY_ADDRESS,s));

//   string IPv4Address
    if (_ipif.getIPv4Address(s))
    {
        inst.addProperty(CIMProperty(PROPERTY_IPV4_ADDRESS,s));
    }

//   string IPv6Address
    if (_ipif.getIPv6Address(s))
    {
        inst.addProperty(CIMProperty(PROPERTY_IPV6_ADDRESS,s));
    }

//   string SubnetMask
  if (_ipif.getSubnetMask(s))
    inst.addProperty(CIMProperty(PROPERTY_SUBNET_MASK,s));

//   uint8 PrefixLength
    if (_ipif.getPrefixLength(i8))
    {
        inst.addProperty(CIMProperty(PROPERTY_PREFIX_LENGTH,i8));
    }

//   uint16 AddressType
  if (_ipif.getAddressType(i16))
    inst.addProperty(CIMProperty(PROPERTY_ADDRESS_TYPE,i16));

//   uint16 IPVersionSupport
  if (_ipif.getIPVersionSupport(i16))
    inst.addProperty(CIMProperty(PROPERTY_IP_VERSION_SUPPORT,i16));

//   uint16 ProtocolIFType
    if (_ipif.getProtocolIFType(i16))
    {
        inst.addProperty(CIMProperty(PROPERTY_PROTOCOL_IF_TYPE,i16));
    }

#ifdef IPPROVIDER_DEBUG
  cout << "IPPEpProvider::_constructInstance() -- done" << endl;
#endif

  return inst;
}

/*
================================================================================
NAME              : _checkClass
DESCRIPTION       : tests the argument for valid classname,
                  : throws exception if not
ASSUMPTIONS       : None
PRE-CONDITIONS    :
POST-CONDITIONS   :
NOTES             :
PARAMETERS        : className
================================================================================
*/
void IPPEpProvider::_checkClass(CIMName& className)
{
    if (!className.equal (CLASS_CIM_IP_PROTOCOL_ENDPOINT))
        throw CIMNotSupportedException(className.getString() +
            ": Class not supported");
}
