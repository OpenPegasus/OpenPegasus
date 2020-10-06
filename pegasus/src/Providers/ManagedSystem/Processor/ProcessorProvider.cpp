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

#include "ProcessorProvider.h"

// ==========================================================================
// Class names.  These values are the names of the classes that
// are common for all of the providers.
//
// We use CIM_ComputerSystem as the value of the key
// SystemCreationClassName, because this class has properties that
// are important for clients of this provider
// ==========================================================================

static const String CLASS_CIM_PROCESSOR("CIM_Processor");
static const String CLASS_PROCESSOR("PG_Processor");
static const String CLASS_CIM_COMPUTER_SYSTEM("CIM_ComputerSystem");

// ==========================================================================
// The number of keys for the classes.
// ==========================================================================

static const int NUMKEYS_PROCESSOR=4;

// ==========================================================================
// Property names.  These values are returned by the provider as
// the property names.
// ==========================================================================

// Key properties (defined in CIM_LogicalDevice)

static const String PROPERTY_SYSTEM_CREATION_CLASS_NAME(
    "SystemCreationClassName");
static const String PROPERTY_SYSTEM_NAME("SystemName");
static const String PROPERTY_CREATION_CLASS_NAME("CreationClassName");
static const String PROPERTY_DEVICE_ID("DeviceID");

// Properties in CIM_MananagedElement

static const String PROPERTY_CAPTION("Caption");
static const String PROPERTY_DESCRIPTION("Description");

// Properties in CIM_ManagedSystemElement

static const String PROPERTY_INSTALL_DATE("InstallDate");
static const String PROPERTY_NAME("Name");
static const String PROPERTY_STATUS("Status");

// Properties in CIM_LogicalElement
// ** none **

// Properties in CIM_LogicalDevice

static const String PROPERTY_POWER_MANAGEMENT_SUPPORTED(
    "PowerManagementSupported");
static const String PROPERTY_POWER_MANAGEMENT_CAPABILITIES(
    "PowerManagementCapabilities");
static const String PROPERTY_AVAILABILITY("Availability");
static const String PROPERTY_STATUS_INFO("StatusInfo");
static const String PROPERTY_LAST_ERROR_CODE("LastErrorCode");
static const String PROPERTY_ERROR_DESCRIPTION("ErrorDescription");
static const String PROPERTY_ERROR_CLEARED("ErrorCleared");
static const String PROPERTY_OTHER_IDENTIFYING_INFO("OtherIdentifyingInfo");
static const String PROPERTY_POWER_ON_HOURS("PowerOnHours");
static const String PROPERTY_TOTAL_POWER_ON_HOURS("TotalPowerHours");
static const String PROPERTY_IDENTIFYING_DESCRIPTIONS(
    "IdentifyingDescriptions");
static const String PROPERTY_ADDITIONAL_AVAILABILITY("AdditionalAvailability");
static const String PROPERTY_MAX_QUIESCE_TIME("MaxQuiesceTime");
static const String METHOD_SET_POWER_STATE("SetPowerState");
static const String METHOD_RESET("Reset");
static const String METHOD_ENABLE_DEVICE("EnableDevice");
static const String METHOD_ONLINE_DEVICE("OnlineDevice");
static const String METHOD_QUIESCE_DEVICE("QuiesceDevice");
static const String METHOD_SAVE_PROPERTIES("SaveProperties");
static const String METHOD_RESTORE_PROPERTIES("RestoreProperties");

// Properties in CIM_Processor

static const String PROPERTY_ROLE("Role");
static const String PROPERTY_FAMILY("Family");
static const String PROPERTY_OTHER_FAMILY_DESCRIPTION("OtherFamilyDescription");
static const String PROPERTY_UPGRADE_METHOD("UpgradeMethod");
static const String PROPERTY_MAX_CLOCK_SPEED("MaxClockSpeed");
static const String PROPERTY_CURRENT_CLOCK_SPEED("CurrentClockSpeed");
static const String PROPERTY_DATA_WIDTH("DataWidth");
static const String PROPERTY_ADDRESS_WIDTH("AddressWidth");
static const String PROPERTY_LOAD_PERCENTAGE("LoadPercentage");
static const String PROPERTY_STEPPING("Stepping");
static const String PROPERTY_UNIQUE_ID("UniqueID");
static const String PROPERTY_CPU_STATUS("CPUStatus");

// Properties in PG_Processor

static const String PROPERTY_BIOS_ID("BiosID");
static const String PROPERTY_FIRMWARE_ID("FirmwareID");

PEGASUS_USING_STD;
PEGASUS_USING_PEGASUS;


ProcessorProvider::ProcessorProvider()
{
  // cout << "ProcessorProvider::ProcessorProvider()" << endl;
}

ProcessorProvider::~ProcessorProvider()
{
  // cout << "ProcessorProvider::~ProcessorProvider()" << endl;
}


// =============================================================================
// NAME              : createInstance
// DESCRIPTION       : Create a CIM_Processor instance.
// ASSUMPTIONS       : None
// PRE-CONDITIONS    :
// POST-CONDITIONS   :
// NOTES             : Currently not supported.
// PARAMETERS        :
// =============================================================================

void ProcessorProvider::createInstance(
    const OperationContext &context,
    const CIMObjectPath &instanceName,
    const CIMInstance &instanceObject,
    ObjectPathResponseHandler &handler)
{
    // Current not implemented, but could be done by spawning a process
    throw CIMNotSupportedException(String::EMPTY);
}


// =============================================================================
// NAME              : deleteInstance
// DESCRIPTION       : Delete a CIM_Processor instance.
// ASSUMPTIONS       : None
// PRE-CONDITIONS    :
// POST-CONDITIONS   :
// NOTES             : Currently not supported.
// PARAMETERS        :
// =============================================================================

void ProcessorProvider::deleteInstance(const OperationContext &context,
                    const CIMObjectPath           &instanceReference,
                    ResponseHandler &handler)

{
  // Currently not implemented, but could be done with kill
  throw CIMNotSupportedException(String::EMPTY);
}


// =============================================================================
// NAME              : enumerateInstances
// DESCRIPTION       : Enumerates all of the CIM_Processor instances.
//                   : An array of instance references is returned.
// ASSUMPTIONS       : None
// PRE-CONDITIONS    :
// POST-CONDITIONS   :
// NOTES             : LocalOnly, DeepInheritance and propertyList are not
//                   : respected by this provider. Localization is not supported
// PARAMETERS        :
// =============================================================================

void ProcessorProvider::enumerateInstances(
    const OperationContext & context,
    const CIMObjectPath & classReference,
    const Boolean includeQualifiers,
    const Boolean includeClassOrigin,
    const CIMPropertyList & propertyList,
    InstanceResponseHandler & handler)
{
    // cout << "ProcessorProvider::enumerateInstances()" << endl;

    int pIndex;
    CIMName className = classReference.getClassName();
    CIMNamespaceName nameSpace = classReference.getNameSpace();
    Processor _p;

    // Validate the classname
    _checkClass(className);

    // Notify processing is starting
    handler.processing();

    // We will only return instances on enumeration requests
    // to the "leaf" class
    if (className.equal (CLASS_PROCESSOR))
    {
      // Get the processor information and construct and deliver
      // an instance for each processor
      for (pIndex = 0; _p.loadProcessorInfo(pIndex); pIndex++)
        handler.deliver(_constructInstance(CLASS_PROCESSOR,
                                           nameSpace,
                                           _p));
    }

    // Notify processing is complete
    handler.complete();

    return;
}  // enumerateInstances



// =============================================================================
// NAME              : enumerateInstanceNames
// DESCRIPTION       : Enumerates all of the CIM_Processor instance names.
//                     An array of instance references is returned.
// ASSUMPTIONS       : None
// PRE-CONDITIONS    :
// POST-CONDITIONS   :
// NOTES             : Localization is not supported by this provider.
// PARAMETERS        :
// =============================================================================

void ProcessorProvider::enumerateInstanceNames(const OperationContext &ctx,
                            const CIMObjectPath &ref,
                            ObjectPathResponseHandler &handler)
{
    // cout << "ProcessorProvider::enumerateInstanceNames()" << endl;

    int pIndex;
    Processor _p;
    CIMName className = ref.getClassName();
    CIMNamespaceName nameSpace = ref.getNameSpace();

    // Validate the classname
    _checkClass(className);

    // Notify processing is starting
    handler.processing();

    // We are only going to respond to enumeration requests on the
    // leaf class CLASS_PROCESSOR
    if (className.equal (CLASS_PROCESSOR))
    {
      // Get the process information and deliver an ObjectPath for
      // each process
      // Note that loadProcessorInfo modifies pIndex to point to the
      // next process structure before the loop increments it!
      for (pIndex = 0; _p.loadProcessorInfo(pIndex); pIndex++)
      {
        // Deliver the names
        handler.deliver(CIMObjectPath(String::EMPTY, // hostname
                                      nameSpace,
                                      CLASS_PROCESSOR,
                                      _constructKeyBindings(_p)));

      }
    }

    // Notify processing is complete
    handler.complete();

    return;

}  // enumerateInstanceNames


// =============================================================================
// NAME              : getInstance
// DESCRIPTION       : Returns a single instance.
// ASSUMPTIONS       : None
// PRE-CONDITIONS    :
// POST-CONDITIONS   :
// NOTES             : LocalOnly, DeepInheritance and propertyList are not
//                   : respected by this provider. Localization is not supported
//                   : Since this provider supports two levels of hierarchy,
//                   : this method returns different sets of properties based
//                   : on which class it was called for (handled by
//                   : constructInstance)
// PARAMETERS        :
// =============================================================================

void ProcessorProvider::getInstance(const OperationContext &ctx,
                 const CIMObjectPath           &instanceName,
                 const Boolean includeQualifiers,
                 const Boolean includeClassOrigin,
                 const CIMPropertyList        &propertyList,
                 InstanceResponseHandler &handler)
{
  // cout << "ProcessorProvider::getInstance(" << instanceName << ")" << endl;

  CIMKeyBinding kb;
  CIMName className = instanceName.getClassName();
  CIMNamespaceName nameSpace = instanceName.getNameSpace();
  String deviceID;
  int i;
  int keysFound; // this will be used as a bit array
  Processor _p;

  // Validate the classname
  _checkClass(className);

  // Extract the key values
  Array<CIMKeyBinding> kbArray = instanceName.getKeyBindings();

  // Leave immediately if wrong number of keys
  if ( (unsigned) kbArray.size() != (unsigned) NUMKEYS_PROCESSOR )
    throw CIMInvalidParameterException("Wrong number of keys");

  // Validate the keys.
  // Each loop iteration will set a bit in keysFound when a valid
  // key is found. If the expected bits aren't all set when
  // the loop finishes, it's a problem
  for(i=0, keysFound=0; i<NUMKEYS_PROCESSOR; i++)
  {
    kb = kbArray[i];

    CIMName keyName = kb.getName();
    String keyValue = kb.getValue();

    // SystemCreationClassName can be empty or must match
    if (keyName.equal (PROPERTY_SYSTEM_CREATION_CLASS_NAME))
    {
      if (String::equal(keyValue, String::EMPTY) ||
          String::equalNoCase(keyValue, CLASS_CIM_COMPUTER_SYSTEM))
        keysFound |= 1;
      else
        throw CIMInvalidParameterException(keyValue+": bad value for key "+
            keyName.getString());
    }

    // SystemName can be empty or must match
    else if (keyName.equal (PROPERTY_SYSTEM_NAME))
    {
      if (String::equal(keyValue, String::EMPTY) ||
          String::equalNoCase(keyValue, _getSysName()))
        keysFound |= 2;
      else
        throw CIMInvalidParameterException(keyValue+": bad value for key "+
            keyName.getString());
    }

    // CreationClassName can be empty or must match
    else if (keyName.equal (PROPERTY_CREATION_CLASS_NAME))
    {
      if (String::equal(keyValue, String::EMPTY) ||
          String::equalNoCase(keyValue, CLASS_PROCESSOR))
        keysFound |= 4;
      else
        throw CIMInvalidParameterException(keyValue+": bad value for key "+
            keyName.getString());
    }

    // Handle must be a valid pid, but we will know that later
    // For now, just verify that it's present
    else if (keyName.equal (PROPERTY_DEVICE_ID))
    {
      deviceID = keyValue;
      keysFound |= 8;
    }

    // Key name was not recognized by any of the above tests
    else throw CIMInvalidParameterException(keyName.getString() +
        ": Unrecognized key");

  } // for

  // We could get here if we didn't get all the keys, which
  // could happen if the right number of keys were supplied,
  // and they all had valid names and values, but there were
  // any duplicates (e.g., two DeviceIDs, no SystemName)
  if(keysFound != (1<<NUMKEYS_PROCESSOR)-1)
    throw CIMInvalidParameterException("Bad object name");

  // Find the instance

  /* Get the process information. */
  if (_p.findProcessor(deviceID))
  {
    /* Notify processing is starting. */
    handler.processing();

    /* Return the instance. */
    handler.deliver(_constructInstance(className,
                                       nameSpace,
                                       _p));

    /* Notify processing is complete. */
    handler.complete();
    return;
  }

  throw CIMObjectNotFoundException(deviceID + ": No such processor");

  return; // can never execute, but required to keep compiler happy
}

/*
================================================================================
NAME              : modifyInstance
DESCRIPTION       : Modify a CIM_Processor instance.
ASSUMPTIONS       : None
PRE-CONDITIONS    :
POST-CONDITIONS   :
NOTES             : Currently not supported.
PARAMETERS        :
================================================================================
*/
void ProcessorProvider::modifyInstance(
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
void ProcessorProvider::initialize(CIMOMHandle &ch)
{
   // cout << "ProcessorProvider::initialize()" << endl;
  _cimomHandle = ch;

  // call platform-specific routine to get values
  _hostName = Processor::getSysName();
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
void ProcessorProvider::terminate()
{
  // cout << "ProcessorProvider::terminate()" << endl;
  delete this;
}

// =============================================================================
// NAME              : _constructKeyBindings
// DESCRIPTION       : Constructs an array of keybindings for process
// ASSUMPTIONS       : None
// PRE-CONDITIONS    :
// POST-CONDITIONS   :
// NOTES             :
// PARAMETERS        : className
// =============================================================================

Array<CIMKeyBinding> ProcessorProvider::_constructKeyBindings(
    const Processor& _p)
{
  Array<CIMKeyBinding> keyBindings;

  // Construct the key bindings
  keyBindings.append(CIMKeyBinding(
      PROPERTY_SYSTEM_CREATION_CLASS_NAME,
      CLASS_CIM_COMPUTER_SYSTEM,
      CIMKeyBinding::STRING));

  keyBindings.append(CIMKeyBinding(
      PROPERTY_SYSTEM_NAME,
      _getSysName(),
      CIMKeyBinding::STRING));

  keyBindings.append(CIMKeyBinding(
      PROPERTY_CREATION_CLASS_NAME,
      CLASS_PROCESSOR,
      CIMKeyBinding::STRING));

  keyBindings.append(CIMKeyBinding(
      PROPERTY_DEVICE_ID,
      _p.getDeviceID(),
      CIMKeyBinding::STRING));

  return keyBindings;
}

// =============================================================================
// NAME              : _constructInstance
// DESCRIPTION       : Constructs instance by adding its properties. The
//                   : Processor instance argument has already been filled in
//                   : with data from an existing process
// ASSUMPTIONS       : None
// PRE-CONDITIONS    :
// POST-CONDITIONS   :
// NOTES             :
// PARAMETERS        : className, nameSpace, Processor
// =============================================================================

CIMInstance ProcessorProvider::_constructInstance(
    const CIMName &className,
    const CIMNamespaceName &nameSpace,
    const Processor &_p)
{
  String s;
  Array<String> as;
  Uint16 i16;
  Boolean b;
  Array<Uint16> ai16;
  Uint32 i32;
  Uint64 i64;
  CIMDateTime d;

  CIMInstance inst(className);

  // Set path

  inst.setPath(CIMObjectPath(String::EMPTY, // hostname
                             nameSpace,
                             CLASS_PROCESSOR,
                             _constructKeyBindings(_p)));

  // =================================================
  // CIM_ManagedElement
  // =================================================

  //   string Caption
  if (_p.getCaption(s))
    inst.addProperty(CIMProperty(PROPERTY_CAPTION,s));

  //   string Description
  if (_p.getDescription(s))
    inst.addProperty(CIMProperty(PROPERTY_DESCRIPTION,s));

  // =================================================
  // CIM_ManagedSystemElement
  // =================================================

  //   datetime InstallDate
  if (_p.getInstallDate(d))
    inst.addProperty(CIMProperty(PROPERTY_INSTALL_DATE,d));

  // string Name
  if (_p.getName(s))
    inst.addProperty(CIMProperty(PROPERTY_NAME,s));

  //   string Status
  if (_p.getStatus(s))
    inst.addProperty(CIMProperty(PROPERTY_STATUS,s));

  // =================================================
  // CIM_LogicalElement
  // =================================================
  //   ** No local properties added in this class **

  // =================================================
  // CIM_LogicalDevice
  // =================================================

  // The keys for this class are:
  // [ key ] string SystemCreationClassName
  // [ key ] string SystemName
  // [ key ] string CreationClassName
  // [ key ] string DeviceID

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

  // boolean PowerManagementSupported;
  if (_p.getPowerManagementSupported(b))
    inst.addProperty(CIMProperty(PROPERTY_POWER_MANAGEMENT_SUPPORTED,b));

  // uint16 PowerManagementCapabilities[];
  if (_p.getPowerManagementCapabilities(ai16))
    inst.addProperty(CIMProperty(PROPERTY_POWER_MANAGEMENT_CAPABILITIES,ai16));

  // uint16 Availability;
  if (_p.getAvailability(i16))
    inst.addProperty(CIMProperty(PROPERTY_AVAILABILITY,i16));

  // uint16 StatusInfo;
  if (_p.getStatusInfo(i16))
    inst.addProperty(CIMProperty(PROPERTY_STATUS_INFO,i16));

  // uint32 LastErrorCode;
  if (_p.getLastErrorCode(i32))
    inst.addProperty(CIMProperty(PROPERTY_LAST_ERROR_CODE,i32));

  // string ErrorDescription;
  if (_p.getErrorDescription(s))
    inst.addProperty(CIMProperty(PROPERTY_ERROR_DESCRIPTION,s));

  // boolean ErrorCleared;
  if (_p.getErrorCleared(b))
    inst.addProperty(CIMProperty(PROPERTY_ERROR_CLEARED,b));

  // string OtherIdentifyingInfo[];
  if (_p.getOtherIdentifyingInfo(as))
    inst.addProperty(CIMProperty(PROPERTY_OTHER_IDENTIFYING_INFO,as));

  // uint64 PowerOnHours;
  if (_p.getPowerOnHours(i64))
    inst.addProperty(CIMProperty(PROPERTY_POWER_ON_HOURS,i64));

  // uint64 TotalPowerOnHours;
  if (_p.getTotalPowerOnHours(i64))
    inst.addProperty(CIMProperty(PROPERTY_TOTAL_POWER_ON_HOURS,i64));

  // string IdentifyingDescriptions[];
  if (_p.getIdentifyingDescriptions(as))
    inst.addProperty(CIMProperty(PROPERTY_IDENTIFYING_DESCRIPTIONS,as));

  // uint16 AdditionalAvailability[];
  if (_p.getAdditionalAvailability(ai16))
    inst.addProperty(CIMProperty(PROPERTY_ADDITIONAL_AVAILABILITY,ai16));

  // uint64 MaxQuiesceTime;
  if (_p.getMaxQuiesceTime(i64))
    inst.addProperty(CIMProperty(PROPERTY_MAX_QUIESCE_TIME,i64));


  // =================================================
  // CIM_Processor
  // =================================================

  // string Role;
  if (_p.getRole(s))
    inst.addProperty(CIMProperty(PROPERTY_ROLE,s));

  // uint16 Family;
  if (_p.getFamily(i16))
    inst.addProperty(CIMProperty(PROPERTY_FAMILY,i16));

  // string OtherFamilyDescription;
  if (_p.getOtherFamilyDescription(s))
    inst.addProperty(CIMProperty(PROPERTY_OTHER_FAMILY_DESCRIPTION,s));

  // uint16 UpgradeMethod;
  if (_p.getUpgradeMethod(i16))
    inst.addProperty(CIMProperty(PROPERTY_UPGRADE_METHOD,i16));

  // uint32 MaxClockSpeed;
  if (_p.getMaxClockSpeed(i32))
    inst.addProperty(CIMProperty(PROPERTY_MAX_CLOCK_SPEED,i32));

  // uint32 CurrentClockSpeed;
  if (_p.getCurrentClockSpeed(i32))
    inst.addProperty(CIMProperty(PROPERTY_CURRENT_CLOCK_SPEED,i32));

  // uint16 DataWidth;
  if (_p.getDataWidth(i16))
    inst.addProperty(CIMProperty(PROPERTY_DATA_WIDTH,i16));

  // uint16 AddressWidth;
  if (_p.getAddressWidth(i16))
    inst.addProperty(CIMProperty(PROPERTY_ADDRESS_WIDTH,i16));

  // uint16 LoadPercentage;
  if (_p.getLoadPercentage(i16))
    inst.addProperty(CIMProperty(PROPERTY_LOAD_PERCENTAGE,i16));

  // string Stepping;
  if (_p.getStepping(s))
    inst.addProperty(CIMProperty(PROPERTY_STEPPING,s));

  // string UniqueID;
  if (_p.getUniqueID(s))
    inst.addProperty(CIMProperty(PROPERTY_UNIQUE_ID,s));

  // uint16 CPUStatus;
  if (_p.getCPUStatus(i16))
    inst.addProperty(CIMProperty(PROPERTY_CPU_STATUS,i16));

  if (className.equal (CLASS_CIM_PROCESSOR)) return inst;

  // =================================================
  // PG_Processor
  // =================================================

  // string BiosID;
  if (_p.getBiosID(s))
    inst.addProperty(CIMProperty(PROPERTY_BIOS_ID,s));

  // string FirmwareID;
  if (_p.getFirmwareID(s))
    inst.addProperty(CIMProperty(PROPERTY_FIRMWARE_ID,s));

  return inst;
}


/*
================================================================================
NAME              : _getSysName
DESCRIPTION       : return the hostname.
ASSUMPTIONS       : None
PRE-CONDITIONS    :
POST-CONDITIONS   :
NOTES             :
================================================================================
*/
String &ProcessorProvider::_getSysName()
{
  // This routine could do something dynamic; for nooow
  // it returns a string that was captured when the
  // initialize() method was invoked
  return _hostName;
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
void ProcessorProvider::_checkClass(CIMName& className)
{
  if (!className.equal (CLASS_CIM_PROCESSOR) &&
      !className.equal (CLASS_PROCESSOR))
    throw CIMNotSupportedException(className.getString() +
        ": Class not supported");
}
