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

#include "IPRouteProvider.h"

// ==========================================================================
// Class names:
//
// We use CIM_UnitaryComputerSystem as the value of the key
// SystemCreationClassName, because this class has properties that
// are important for clients of this provider.
// ==========================================================================

static const CIMName CLASS_CIM_UNITARY_COMPUTER_SYSTEM = CIMName(
    "CIM_UnitaryComputerSystem");
static const CIMName CLASS_PG_IP_ROUTE                 = CIMName ("PG_IPRoute");

// ==========================================================================
// The number of keys for the classes.
// ==========================================================================

#define NUMKEYS_PG_IP_ROUTE                   8

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

// Properties in CIM_LogicalElement

//      none

// Properties in CIM_NextHopRouting

static const CIMName PROPERTY_DESTINATION_ADDRESS        = CIMName(
    "DestinationAddress");
static const CIMName PROPERTY_DESTINATION_MASK           = CIMName(
    "DestinationMask");
static const CIMName PROPERTY_NEXT_HOP                   = CIMName("NextHop");
static const CIMName PROPERTY_IS_STATIC                  = CIMName("IsStatic");

// Properties in CIM_IPRoute  -- note all 8 of these are the keys

static const CIMName PROPERTY_SYSTEM_CREATION_CLASS_NAME = CIMName(
    "SystemCreationClassName");
static const CIMName PROPERTY_SYSTEM_NAME                = CIMName(
    "SystemName");
static const CIMName PROPERTY_SERVICE_CREATION_CLASS_NAME= CIMName(
    "ServiceCreationClassName");
static const CIMName PROPERTY_SERVICE_NAME               = CIMName(
    "ServiceName");
static const CIMName PROPERTY_CREATION_CLASS_NAME        = CIMName(
    "CreationClassName");
static const CIMName PROPERTY_IP_DESTINATION_ADDRESS     = CIMName(
    "IPDestinationAddress");
static const CIMName PROPERTY_IP_DESTINATION_MASK        = CIMName(
    "IPDestinationMask");
static const CIMName PROPERTY_ADDRESS_TYPE               = CIMName(
    "AddressType");

//      DestinationAddress & DestinationMask overridden to insert
//      model correspondence strings.

// Properties in PG_IPRoute

//      none


PEGASUS_USING_STD;
PEGASUS_USING_PEGASUS;


IPRouteProvider::IPRouteProvider()
{
#ifdef IPPROVIDER_DEBUG
    cout << "IPRouteProvider::IPRouteProvider()" << endl;
#endif
}

IPRouteProvider::~IPRouteProvider()
{
}


/*
================================================================================
NAME              : createInstance
DESCRIPTION       : Create a PG_IPRoute instance.
ASSUMPTIONS       : None
PRE-CONDITIONS    :
POST-CONDITIONS   :
NOTES             : Currently not supported.
PARAMETERS        :
================================================================================
*/
void IPRouteProvider::createInstance(
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
DESCRIPTION       : Delete a PG_IPRoute instance.
ASSUMPTIONS       : None
PRE-CONDITIONS    :
POST-CONDITIONS   :
NOTES             : Currently not supported.
PARAMETERS        :
================================================================================
*/
void IPRouteProvider::deleteInstance(
    const OperationContext &context,
    const CIMObjectPath &instanceReference,
    ResponseHandler &handler)

{
    throw CIMNotSupportedException(String::EMPTY);
}

/*
================================================================================
NAME              : enumerateInstances
DESCRIPTION       : Enumerates all of the PG_IPRoute instances.
                  : An array of instance references is returned.
ASSUMPTIONS       : None
PRE-CONDITIONS    :
POST-CONDITIONS   :
NOTES             : LocalOnly, DeepInheritance and propertyList are not
                  : respected by this provider. Localization is not supported
PARAMETERS        :
================================================================================
*/
void IPRouteProvider::enumerateInstances(
    const OperationContext & context,
    const CIMObjectPath & classReference,
    const Boolean includeQualifiers,
    const Boolean includeClassOrigin,
    const CIMPropertyList & propertyList,
    InstanceResponseHandler & handler)
{
#ifdef IPPROVIDER_DEBUG
    cout << "IPRouteProvider::enumerateInstances()" << endl;
#endif

    CIMName className = classReference.getClassName();
    CIMNamespaceName nameSpace = classReference.getNameSpace();

    // Validate the classname
    _checkClass(className);

    // Notify processing is starting
    handler.processing();

    int i;
    RouteList _rList;

    for (i = 0; i < _rList.size(); i++)
    {
        handler.deliver(_constructInstance(
            CLASS_PG_IP_ROUTE, nameSpace, _rList.getRoute(i)));
    }

    // Notify processing is complete
    handler.complete();

    return;
}  // enumerateInstances

/*
================================================================================
NAME              : enumerateInstanceNames
DESCRIPTION       : Enumerates all of the PG_IPRoute instance names.
                  : An array of instance references is returned.
ASSUMPTIONS       : None
PRE-CONDITIONS    :
POST-CONDITIONS   :
NOTES             : Localization is not supported by this provider.
PARAMETERS        :
================================================================================
*/
void IPRouteProvider::enumerateInstanceNames(
    const OperationContext &ctx,
    const CIMObjectPath &ref,
    ObjectPathResponseHandler &handler)
{
#ifdef IPPROVIDER_DEBUG
    cout << "IPRouteProvider::enumerateInstanceNames()" << endl;
#endif

    CIMName className = ref.getClassName();
    CIMNamespaceName nameSpace = ref.getNameSpace();

    // Validate the classname
    _checkClass(className);

    // Notify processing is starting
    handler.processing();

    int i;
    RouteList _rList;

#ifdef IPPROVIDER_DEBUG
    cout << "IPRouteProvider::enumerateInstanceNames() _rList Initialized"
         << endl;
#endif

    for (i = 0; i < _rList.size(); i++)
    {
        // Deliver the names
        handler.deliver(CIMObjectPath(
            String::EMPTY, // hostname
            nameSpace,
            CLASS_PG_IP_ROUTE,
            _constructKeyBindings(_rList.getRoute(i))));
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
void IPRouteProvider::getInstance(
    const OperationContext &ctx,
    const CIMObjectPath &instanceName,
    const Boolean includeQualifiers,
    const Boolean includeClassOrigin,
    const CIMPropertyList &propertyList,
    InstanceResponseHandler &handler)
{
#ifdef IPPROVIDER_DEBUG
  cout << "IPRouteProvider::getInstance(" << instanceName.toString() << ")"
       << endl;
#endif

  CIMKeyBinding kb;
  CIMName className = instanceName.getClassName();
  CIMNamespaceName nameSpace = instanceName.getNameSpace();
  int i;
  int keysFound; // this will be used as a bit array
  String sn;     // system name
  String ipda,   // IP Destination Address
         ipdm;   // IP Destination Mask
  Uint16 ipat;   // IP Address Type

  // Grab the system name
  if (IPInterface::getSystemName(sn) == false)
     sn = String::EMPTY;

  // Validate the classname
  _checkClass(className);

  // Extract the key values
  Array<CIMKeyBinding> kbArray = instanceName.getKeyBindings();

  // Leave immediately if wrong number of keys
  if ( kbArray.size() != NUMKEYS_PG_IP_ROUTE )
    throw CIMInvalidParameterException("Wrong number of keys");

  // Validate the keys.
  // Each loop iteration will set a bit in keysFound when a valid
  // key is found. If the expected bits aren't all set when
  // the loop finishes, it's a problem
  for (i=0, keysFound=0; i < NUMKEYS_PG_IP_ROUTE; i++)
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
        keysFound |= 1;
      else
        throw CIMInvalidParameterException(keyValue+": bad value for key "+
            keyName.getString());
    }

    // SystemName can be empty or must match
    else if (keyName.equal (PROPERTY_SYSTEM_NAME))
    {
      if (String::equal(keyValue, String::EMPTY) ||
          String::equalNoCase(keyValue, sn) )
        keysFound |= 2;
      else
        throw CIMInvalidParameterException(keyValue+": bad value for key "+
            keyName.getString());
    }

    // ServiceCreationClassName must exist, but can be ignored
    else if (keyName.equal (PROPERTY_SERVICE_CREATION_CLASS_NAME))
    {
        keysFound |= 4;
    }

    // ServiceName must exist, but can be ignored
    else if (keyName.equal (PROPERTY_SERVICE_NAME))
    {
        keysFound |= 8;
    }

    // CreationClassName can be empty or must match
    else if (keyName.equal (PROPERTY_CREATION_CLASS_NAME))
    {
      if (String::equal(keyValue, String::EMPTY) ||
          String::equalNoCase(keyValue, CLASS_PG_IP_ROUTE.getString()))
        keysFound |= 16;
      else
        throw CIMInvalidParameterException(keyValue+": bad value for key "+
            keyName.getString());
    }

    // IP Destination Address, IP Destination Mask, and Address Type
    // must be valid, but we will know that later
    // For now, just verify that they're present, and record them.

    // IPDestinationAddress must be present
    else if (keyName.equal (PROPERTY_IP_DESTINATION_ADDRESS))
    {
      ipda = keyValue;
      keysFound |= 32;
    }

    // IPDestinationMask must be present
    else if (keyName.equal (PROPERTY_IP_DESTINATION_MASK))
    {
      ipdm = keyValue;
      keysFound |= 64;
    }

    // AddressType must be present
    else if (keyName.equal (PROPERTY_ADDRESS_TYPE))
    {
      unsigned short ti;

      sscanf(keyValue.getCString(), "%hu", &ti);
      ipat = Uint16(ti);
      keysFound |= 128;
    }

    // Key name was not recognized by any of the above tests
    else throw CIMInvalidParameterException(keyName.getString() +
        ": Unrecognized key");
  } // for

  // We could get here if we didn't get all the keys, which
  // could happen if the right number of keys were supplied,
  // and they all had valid names and values, but there were
  // any duplicates (e.g., two Names, no SystemName)
  if (keysFound != (1<<NUMKEYS_PG_IP_ROUTE)-1)
    throw CIMInvalidParameterException("Bad object name");

  /* Find the instance.  First convert the instance id which is the */
  /* process handle to an integer.  This is necessary because the   */
  /* handle is the process id on HP-UX which must be passed to      */
  /* pstat_getproc() as an integer.                                 */

  /* Get the Route List. */
  RouteList _rList;
  IPRoute _ipr;

  if (_rList.findRoute(ipda, ipdm, ipat, _ipr))
  {
    /* Notify processing is starting. */
    handler.processing();

    /* Return the instance. */
    handler.deliver(_constructInstance(className, nameSpace, _ipr));

    /* Notify processing is complete. */
    handler.complete();
    return;
  }

  char c[100];
  sprintf (c, "%hu", ipat);
  throw CIMObjectNotFoundException(
      "DestAddr=" + String(ipda) +
      ", DestMask=" + String(ipdm) +
      ", AddrType=" + String(c) +
      ": No such IP Route");

  return; // can never execute, but required to keep compiler happy
}

/*
================================================================================
NAME              : modifyInstance
DESCRIPTION       : Modify a PG_IPRoute instance.
ASSUMPTIONS       : None
PRE-CONDITIONS    :
POST-CONDITIONS   :
NOTES             : Currently not supported.
PARAMETERS        :
================================================================================
*/
void IPRouteProvider::modifyInstance(
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
void IPRouteProvider::initialize(CIMOMHandle &ch)
{
#ifdef IPPROVIDER_DEBUG
    cout << "IPRouteProvider::initialize()" << endl;
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
void IPRouteProvider::terminate()
{
#ifdef IPPROVIDER_DEBUG
    cout << "IPRouteProvider::terminate()" << endl;
#endif

    delete this;
}

/*
================================================================================
NAME              : _constructKeyBindings
DESCRIPTION       : Constructs an array of keybindings for an IP Route
ASSUMPTIONS       : None
PRE-CONDITIONS    :
POST-CONDITIONS   :
NOTES             :
PARAMETERS        : IP Route
================================================================================
*/
Array<CIMKeyBinding> IPRouteProvider::_constructKeyBindings(
    const IPRoute& _ipr)
{
#ifdef IPPROVIDER_DEBUG
    cout << "IPRouteProvider::_constructKeyBindings()" << endl;
#endif

    Array<CIMKeyBinding> keyBindings;
    String s;
    Uint16 i16;


    // Construct the key bindings

    keyBindings.append(CIMKeyBinding(
        PROPERTY_SYSTEM_CREATION_CLASS_NAME,
        CLASS_CIM_UNITARY_COMPUTER_SYSTEM.getString(),
        CIMKeyBinding::STRING));

    if (IPInterface::getSystemName(s))
        keyBindings.append(CIMKeyBinding(
            PROPERTY_SYSTEM_NAME,
            s,
            CIMKeyBinding::STRING));
    else
        throw CIMNotSupportedException(
            String("Host-specific module doesn't support Key `") +
                PROPERTY_SYSTEM_NAME.getString() + String("'"));

    keyBindings.append(CIMKeyBinding(
        PROPERTY_SERVICE_CREATION_CLASS_NAME,
        String::EMPTY,
        CIMKeyBinding::STRING));

    keyBindings.append(CIMKeyBinding(
        PROPERTY_SERVICE_NAME,
        String::EMPTY,
        CIMKeyBinding::STRING));

    keyBindings.append(CIMKeyBinding(
        PROPERTY_CREATION_CLASS_NAME,
        CLASS_PG_IP_ROUTE.getString(),
        CIMKeyBinding::STRING));

    if (_ipr.getDestinationAddress(s))
        keyBindings.append(CIMKeyBinding(
            PROPERTY_IP_DESTINATION_ADDRESS,
            s,
            CIMKeyBinding::STRING));
    else
        throw CIMNotSupportedException(
            String("Host-specific module doesn't support Key `") +
                PROPERTY_IP_DESTINATION_ADDRESS.getString() + String("'"));

    if (_ipr.getDestinationMask(s))
        keyBindings.append(CIMKeyBinding(PROPERTY_IP_DESTINATION_MASK,
                                s, CIMKeyBinding::STRING));
    else
        throw CIMNotSupportedException(
            String("Host-specific module doesn't support Key `") +
                PROPERTY_IP_DESTINATION_MASK.getString() + String("'"));

    if (_ipr.getAddressType(i16))
    {
        char c[100];
        sprintf(c,"%hu",i16);
        s = String(c);
        keyBindings.append(CIMKeyBinding(
            PROPERTY_ADDRESS_TYPE,
            s,
            CIMKeyBinding::NUMERIC));
    }
    else
        throw CIMNotSupportedException(
            String("Host-specific module doesn't support Key `") +
                PROPERTY_ADDRESS_TYPE.getString() + String("'"));

#ifdef IPPROVIDER_DEBUG
    cout << "IPRouteProvider::_constructKeyBindings() -- done" << endl;
#endif

    return keyBindings;
}

/*
================================================================================
NAME              : _constructInstance
DESCRIPTION       : Constructs instance by adding its properties. The
                  : IP Route argument has already been filled in
                  : with data from an existing IP Route
ASSUMPTIONS       : None
PRE-CONDITIONS    :
POST-CONDITIONS   :
NOTES             :
PARAMETERS        : className, nameSpace, IP Route
================================================================================
*/
CIMInstance IPRouteProvider::_constructInstance(
    const CIMName &className,
    const CIMNamespaceName &nameSpace,
    const IPRoute &_ipr)
{
#ifdef IPPROVIDER_DEBUG
  cout << "IPRouteProvider::_constructInstance()" << endl;
#endif

  String s;
  CIMDateTime d;
  Boolean b;
  Uint16 ui;

  CIMInstance inst(className);

  // Set path

  inst.setPath(CIMObjectPath(String::EMPTY, // hostname
                             nameSpace,
                             CLASS_PG_IP_ROUTE,
                             _constructKeyBindings(_ipr)));

// CIM_ManagedElement

//   string Caption
  if (_ipr.getCaption(s))
    inst.addProperty(CIMProperty(PROPERTY_CAPTION,s));

//   string Description
  if (_ipr.getDescription(s))
    inst.addProperty(CIMProperty(PROPERTY_DESCRIPTION,s));

// CIM_ManagedSystemElement

//   datetime InstallDate
  if (_ipr.getInstallDate(d))
    inst.addProperty(CIMProperty(PROPERTY_INSTALL_DATE,d));

//   string Name
  if (_ipr.getName(s))
    inst.addProperty(CIMProperty(PROPERTY_NAME,s));

//   string Status
  if (_ipr.getStatus(s))
    inst.addProperty(CIMProperty(PROPERTY_STATUS,s));

// CIM_LogicalElement
//   ** No local properties added in this class **

// CIM_NextHopRouting

//   String DestinationAddress
  if (_ipr.getDestinationAddress(s))
    inst.addProperty(CIMProperty(PROPERTY_DESTINATION_ADDRESS,s));

//   String DestinationMask
  if (_ipr.getDestinationMask(s))
    inst.addProperty(CIMProperty(PROPERTY_DESTINATION_MASK,s));

//   String NextHop
  if (_ipr.getNextHop(s))
    inst.addProperty(CIMProperty(PROPERTY_NEXT_HOP,s));

//   String IsStatic
  if (_ipr.getIsStatic(b))
    inst.addProperty(CIMProperty(PROPERTY_IS_STATIC,b));

//   String SystemCreationClassName
  inst.addProperty(CIMProperty(PROPERTY_SYSTEM_CREATION_CLASS_NAME,
                               CLASS_CIM_UNITARY_COMPUTER_SYSTEM.getString()));

//   String SystemName
  if (IPInterface::getSystemName(s))
        inst.addProperty(CIMProperty(PROPERTY_SYSTEM_NAME, s));
  else
        throw CIMNotSupportedException(
                String("Host-specific module doesn't support Key `") +
                PROPERTY_SYSTEM_NAME.getString() + String("'"));

//   String ServiceCreationClassName
  inst.addProperty(CIMProperty(PROPERTY_SERVICE_CREATION_CLASS_NAME,
                               String::EMPTY));
//   String ServiceName
  inst.addProperty(CIMProperty(PROPERTY_SERVICE_NAME,
                               String::EMPTY));

//   String CreationClassName
  inst.addProperty(CIMProperty(PROPERTY_CREATION_CLASS_NAME,
                               CLASS_PG_IP_ROUTE.getString()));

//   String IPDestinationAddress
  if (_ipr.getDestinationAddress(s))
    inst.addProperty(CIMProperty(PROPERTY_IP_DESTINATION_ADDRESS,s));

//   String IPDestinationMask
  if (_ipr.getDestinationMask(s))
    inst.addProperty(CIMProperty(PROPERTY_IP_DESTINATION_MASK,s));

//   Uint16 AddressType
  if (_ipr.getAddressType(ui))
    inst.addProperty(CIMProperty(PROPERTY_ADDRESS_TYPE,ui));

#ifdef IPPROVIDER_DEBUG
  cout << "IPRouteProvider::_constructInstance() -- done" << endl;
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
void IPRouteProvider::_checkClass(CIMName& className)
{
    if (!className.equal (CLASS_PG_IP_ROUTE))
        throw CIMNotSupportedException(
            className.getString() + ": Class not supported");
}
