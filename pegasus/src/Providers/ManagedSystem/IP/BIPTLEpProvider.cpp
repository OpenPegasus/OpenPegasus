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

#include "BIPTLEpProvider.h"

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
static const CIMName CLASS_CIM_LAN_ENDPOINT            = CIMName(
    "CIM_LANEndpoint");
static const CIMName CLASS_PG_BINDS_IP_TO_LAN_ENDPOINT = CIMName(
    "PG_BindsIPToLANEndpoint");

// ==========================================================================
// The number of keys for the classes.
// ==========================================================================

#define NUMKEYS_PG_BINDS_IP_TO_LAN_ENDPOINT   2
#define NUMKEYS_CIM_PROTOCOL_ENDPOINT         4

// ==========================================================================
// Property names.  These values are returned by the provider as
// the property names.
// ==========================================================================

// Properties in PG_BindsIPToLANEndpoint

// References
//      CIM_LANEndpoint REF Antecedent
static const CIMName PROPERTY_ANTECEDENT         = CIMName ("Antecedent");
//      CIM_IPProtocolEndpoint REF Dependent
static const CIMName PROPERTY_DEPENDENT          = CIMName ("Dependent");

// Regular Properties
static const CIMName PROPERTY_FRAME_TYPE         = CIMName ("FrameType");

// Properties that make up the References

static const CIMName PROPERTY_NAME                       = CIMName ("Name");
static const CIMName PROPERTY_CREATION_CLASS_NAME        = CIMName(
    "CreationClassName");
static const CIMName PROPERTY_SYSTEM_CREATION_CLASS_NAME = CIMName(
    "SystemCreationClassName");
static const CIMName PROPERTY_SYSTEM_NAME                = CIMName(
    "SystemName");


PEGASUS_USING_STD;
PEGASUS_USING_PEGASUS;


BIPTLEpProvider::BIPTLEpProvider()
{
#ifdef IPPROVIDER_DEBUG
  cout << "BIPTLEpProvider::BIPTLEpProvider()" << endl;
#endif
}

BIPTLEpProvider::~BIPTLEpProvider()
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
void BIPTLEpProvider::createInstance(const OperationContext &context,
                    const CIMObjectPath           &instanceName,
                    const CIMInstance            &instanceObject,
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
void BIPTLEpProvider::deleteInstance(const OperationContext &context,
                    const CIMObjectPath           &instanceReference,
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
void BIPTLEpProvider::enumerateInstances(
    const OperationContext & context,
    const CIMObjectPath & classReference,
    const Boolean includeQualifiers,
    const Boolean includeClassOrigin,
    const CIMPropertyList & propertyList,
    InstanceResponseHandler & handler)
{
#ifdef IPPROVIDER_DEBUG
    cout << "BIPTLEpProvider::enumerateInstances()" << endl;
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
        IPInterface _ipif = _ifList.getInterface(i);

        if (_ipif.bindsToLANInterface())
        {
            handler.deliver(
                _constructInstance(CLASS_PG_BINDS_IP_TO_LAN_ENDPOINT,
                     nameSpace, _ipif ) );
        }
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
void BIPTLEpProvider::enumerateInstanceNames(const OperationContext &ctx,
                            const CIMObjectPath &ref,
                            ObjectPathResponseHandler &handler)
{
#ifdef IPPROVIDER_DEBUG
    cout << "BIPTLEpProvider::enumerateInstanceNames()" << endl;
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
    cout << "BIPTLEpProvider::enumerateInstanceNames() _ifList Initialized"
        << endl;
#endif

    for (i = 0; i < _ifList.size(); i++)
    {
        IPInterface _ipif = _ifList.getInterface(i);

        if (_ipif.bindsToLANInterface())
        {
            // Deliver the names
            handler.deliver(
                CIMObjectPath(String::EMPTY, // hostname
                    nameSpace, CLASS_PG_BINDS_IP_TO_LAN_ENDPOINT,
                    _constructKeyBindings(nameSpace, _ipif)));
        }
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
void BIPTLEpProvider::getInstance(const OperationContext &ctx,
                 const CIMObjectPath           &instanceName,
                 const Boolean includeQualifiers,
                 const Boolean includeClassOrigin,
                 const CIMPropertyList        &propertyList,
                 InstanceResponseHandler &handler)
{
#ifdef IPPROVIDER_DEBUG
  cout << "BIPTLEpProvider::getInstance(" << instanceName.toString() << ")"
       << endl;
#endif

  CIMKeyBinding kb;
  CIMName className = instanceName.getClassName();
  CIMNamespaceName nameSpace = instanceName.getNameSpace();
  int i;
  int keysFound;  // this will be used as a bit array
  String sn;      // system name
  String lepName, // LAN Endpoint Name
         ipeName, // IP Protocol Endpoint Name
         refCCN,  // Reference's Creation Class Name
         refName; // Reference's Name

  // Grab the system name
  if (IPInterface::getSystemName(sn) == false)
     sn = String::EMPTY;

  // Validate the classname
  _checkClass(className);

  // Extract the key values
  Array<CIMKeyBinding> kbArray = instanceName.getKeyBindings();

  // Leave immediately if wrong number of keys
  if ( kbArray.size() != NUMKEYS_PG_BINDS_IP_TO_LAN_ENDPOINT )
    throw CIMInvalidParameterException("Wrong number of keys");

  // Validate the keys.
  // Each loop iteration will set a bit in keysFound when a valid
  // key is found. If the expected bits aren't all set when
  // the loop finishes, it's a problem
  for (i=0, keysFound=0; i < NUMKEYS_PG_BINDS_IP_TO_LAN_ENDPOINT; i++)
  {
    kb = kbArray[i];

    CIMName keyName = kb.getName();
    String keyValue = kb.getValue();

    // Antecedent must match
    if (keyName.equal (PROPERTY_ANTECEDENT))
    {
      if (_goodPERefKeys(keyValue, refCCN, refName) &&
          String::equalNoCase(refCCN, CLASS_CIM_LAN_ENDPOINT.getString()))
      {
          keysFound |= 1;
          lepName = refName;
      }
      else
          throw CIMInvalidParameterException(keyValue+": bad value for key " +
              keyName.getString());
    }

    // Dependent must match
    else if (keyName.equal (PROPERTY_DEPENDENT))
    {
      if (_goodPERefKeys(keyValue, refCCN, refName) &&
          String::equalNoCase(refCCN,
              CLASS_CIM_IP_PROTOCOL_ENDPOINT.getString()))
      {
        keysFound |= 2;
        ipeName = refName;
      }
      else
         throw CIMInvalidParameterException(keyValue+": bad value for key "+
             keyName.getString());
    }

    // Key name was not recognized by any of the above tests
    else
        throw CIMInvalidParameterException(keyName.getString() +
            ": Unrecognized key");
  } // for

  // We could get here if we didn't get all the keys, which
  // could happen if the right number of keys were supplied,
  // and they all had valid names and values, but there were
  // any duplicates (e.g., two Names, no SystemName)
  if (keysFound != (1<<NUMKEYS_PG_BINDS_IP_TO_LAN_ENDPOINT)-1)
    throw CIMInvalidParameterException("Bad object name");

  // Get the Interface List
  InterfaceList _ifList;
  IPInterface _ipif;

  // Make sure the LAN Endpoint name is embedded in the IP Protocol Endpoint
  // name.  If we can find the request interface and if it's one that
  // binds to a LAN Interface, then we've found the right one, so
  // return it to the client.
  if (ipeName.find(lepName) != PEG_NOT_FOUND &&
      _ifList.findInterface(ipeName, _ipif) &&
      _ipif.bindsToLANInterface())
  {
    /* Notify processing is starting. */
    handler.processing();

    /* Return the instance. */
    handler.deliver(_constructInstance(className, nameSpace, _ipif));

    /* Notify processing is complete. */
    handler.complete();
    return;
  }

  throw CIMObjectNotFoundException(ipeName+": No such IP Interface");

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
void BIPTLEpProvider::modifyInstance(
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
void BIPTLEpProvider::initialize(CIMOMHandle &ch)
{
#ifdef IPPROVIDER_DEBUG
  cout << "BIPTLEpProvider::initialize()" << endl;
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
void BIPTLEpProvider::terminate()
{
#ifdef IPPROVIDER_DEBUG
  cout << "BIPTLEpProvider::terminate()" << endl;
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
PARAMETERS        : className, Process
================================================================================
*/
Array<CIMKeyBinding> BIPTLEpProvider::_constructKeyBindings(
    const CIMNamespaceName &nameSpace,
    const IPInterface &_ipif)
{
#ifdef IPPROVIDER_DEBUG
    cout << "BIPTLEpProvider::_constructKeyBindings()" << endl;
#endif

    Array<CIMKeyBinding> keyBindings;
    String s, sn;
    CIMObjectPath op;

    if (_ipif.getSystemName(sn) == false)
    {
        throw CIMNotSupportedException(
            String("Host-specific module doesn't support Key `") +
                PROPERTY_SYSTEM_NAME.getString() + String("'"));
    }

    // Construct the key bindings
    op = CIMObjectPath(
        sn, //hostname
        nameSpace,
        CLASS_CIM_LAN_ENDPOINT,
        _constructReference(
            CLASS_CIM_LAN_ENDPOINT,
            sn,
            _ipif.get_LANInterfaceName()));

    keyBindings.append(CIMKeyBinding(PROPERTY_ANTECEDENT,
                                   op.toString(),
                                   CIMKeyBinding::REFERENCE));

    if (_ipif.getName(s))
    {
        op = CIMObjectPath(
            sn, //hostname
            nameSpace,
            CLASS_CIM_IP_PROTOCOL_ENDPOINT,
            _constructReference(
                CLASS_CIM_IP_PROTOCOL_ENDPOINT,
                sn,
                s));

        keyBindings.append(CIMKeyBinding(
            PROPERTY_DEPENDENT,
            op.toString(),
            CIMKeyBinding::REFERENCE));
    }
    else
    {
        throw CIMNotSupportedException(
            String("Host-specific module doesn't support Key `") +
                PROPERTY_NAME.getString() + String("'"));
    }

#ifdef IPPROVIDER_DEBUG
    cout << "BIPTLEpProvider::_constructKeyBindings() -- done" << endl;
#endif

    return keyBindings;
}


/*
================================================================================
NAME              : _constructReference
DESCRIPTION       : Constructs a reference pointing to the appropriate class
ASSUMPTIONS       : None
PRE-CONDITIONS    :
POST-CONDITIONS   :
NOTES             :
PARAMETERS        : className, Process
================================================================================
*/
Array<CIMKeyBinding> BIPTLEpProvider::_constructReference(
    const CIMName& className,
    const String& sysName,
    const String& instName)
{
#ifdef IPPROVIDER_DEBUG
    cout << "BIPTLEpProvider::_constructReference()" << endl;
#endif

    Array<CIMKeyBinding> kba;

    // Construct the key bindings
    kba.append(CIMKeyBinding(
        PROPERTY_SYSTEM_CREATION_CLASS_NAME,
        CLASS_CIM_UNITARY_COMPUTER_SYSTEM.getString(),
        CIMKeyBinding::STRING));

    kba.append(CIMKeyBinding(
        PROPERTY_SYSTEM_NAME,
        sysName,
        CIMKeyBinding::STRING));

    kba.append(CIMKeyBinding(
        PROPERTY_CREATION_CLASS_NAME,
        className.getString(),
        CIMKeyBinding::STRING));

    kba.append(CIMKeyBinding(
        PROPERTY_NAME,
        instName,
        CIMKeyBinding::STRING));

#ifdef IPPROVIDER_DEBUG
    cout << "BIPTLEpProvider::_constructReference() -- done" << endl;
#endif

    return kba;
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
PARAMETERS        : className, Process
================================================================================
*/
CIMInstance BIPTLEpProvider::_constructInstance(
    const CIMName &className,
    const CIMNamespaceName &nameSpace,
    const IPInterface &_ipif)
{
#ifdef IPPROVIDER_DEBUG
  cout << "BIPTLEpProvider::_constructInstance()" << endl;
#endif

  String s;
  Uint16 i16;
  CIMDateTime d;

  CIMInstance inst(className);

  // Set path

  inst.setPath(CIMObjectPath(String::EMPTY, // hostname
                             nameSpace,
                             CLASS_PG_BINDS_IP_TO_LAN_ENDPOINT,
                             _constructKeyBindings(nameSpace, _ipif)));

// ======================================================
// The following properties are in CIM_ServiceAccessPoint
// ======================================================

  // The keys for this class are:
  // CIM_LANEndpoint REF Antecedent
  // CIM_IPProtocolEndpoint REF Dependent

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

// CIM_BindsToLANEndpoint

//   uint16 FrameType
  if (_ipif.getFrameType(i16))
    inst.addProperty(CIMProperty(PROPERTY_FRAME_TYPE,i16));

#ifdef IPPROVIDER_DEBUG
  cout << "BIPTLEpProvider::_constructInstance() -- done" << endl;
#endif

  return inst;
}

/*
================================================================================
NAME              : _goodPERefKeys
DESCRIPTION       : Checks to see if this is a value reference to a
                  : Protocol Endpoint with good values for SCCN, SN,
                  : and CCN.  If it is, it returns the Name;
                  : otherwise, it returns false.
ASSUMPTIONS       : None
PRE-CONDITIONS    :
POST-CONDITIONS   :
NOTES             :
================================================================================
*/
Boolean BIPTLEpProvider::_goodPERefKeys(
    const CIMObjectPath &instName,
    String &rccn,    // CreationClassName
    String &rname)   // Name
{
#ifdef IPPROVIDER_DEBUG
   cout << "BIPTLEpProvider::_goodPERefKeys(" << instName.toString() << ")"
        << endl;
#endif

   int keysFound,  // this will be used as a bit array
       i;

   Array<CIMKeyBinding> kbArray = instName.getKeyBindings();

   if (kbArray.size() != NUMKEYS_CIM_PROTOCOL_ENDPOINT)
       throw CIMInvalidParameterException("Wrong number of keys in reference");

   for (i=0, keysFound=0; i < NUMKEYS_CIM_PROTOCOL_ENDPOINT; i++)
   {
      CIMKeyBinding kb = kbArray[i];

      CIMName keyName = kb.getName();
      String keyValue = kb.getValue();

      // SystemCreationClassName
      if (keyName.equal (PROPERTY_SYSTEM_CREATION_CLASS_NAME))
      {
          if (String::equalNoCase(keyValue,
                  CLASS_CIM_UNITARY_COMPUTER_SYSTEM.getString()) ||
                  String::equal(keyValue, String::EMPTY) )
             keysFound |= 1;
          else
             return false;  // invalid value for SystemCreationClassName
      }

      // SystemName
      else if (keyName.equal (PROPERTY_SYSTEM_NAME))
      {
          String sn; // System Name
          if (IPInterface::getSystemName(sn) == false)
              sn = String::EMPTY;

          if (String::equalNoCase(keyValue,sn) ||
                  String::equal(keyValue, String::EMPTY) )
             keysFound |= 2;
          else
             return false;  // invalid value for SystemName
      }

      // CreationClassName
      else if (keyName.equal (PROPERTY_CREATION_CLASS_NAME))
      {
          rccn = keyValue;
            keysFound |= 4;
      }

      // Name must be a valid IP interface, but we will know that later
      // For now, just verify that it's present
      else if (keyName.equal (PROPERTY_NAME))
      {
            rname = keyValue;
            keysFound |= 8;
      }

      // Key name was not recognized by any of the above tests
      else return false;

   } // for

   // We could get here if we didn't get all the keys, which
   // could happen if the right number of keys were supplied,
   // and they all had valid names and values, but there were
   // any duplicates (e.g., two Names, no SystemName)
   if (keysFound != (1<<NUMKEYS_CIM_PROTOCOL_ENDPOINT)-1)
       return false;

#ifdef IPPROVIDER_DEBUG
   cout << "BIPTLEpProvider::_goodPERefKeys() - success!" << endl;
#endif

   return true;
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
================================================================================
*/
void BIPTLEpProvider::_checkClass(CIMName& className)
{
    if (!className.equal(CLASS_PG_BINDS_IP_TO_LAN_ENDPOINT))
        throw CIMNotSupportedException(className.getString() +
            ": Class not supported");
}
