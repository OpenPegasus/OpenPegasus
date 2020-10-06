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

#include "ProcessProvider.h"

// ==========================================================================
// Class names.  These values are the names of the classes that
// are common for all of the providers.
//
// We use CIM_UnitaryComputerSystem as the value of the key
// CSCreationClassName, because this class has properties that
// are important for clients of this provider
// ==========================================================================

#define CLASS_CIM_UNITARY_COMPUTER_SYSTEM  "CIM_UnitaryComputerSystem"
#define CLASS_CIM_OPERATING_SYSTEM         "CIM_OperatingSystem"
#define CLASS_CIM_PROCESS                  "CIM_Process"

// Use PG_UnixProcess until DMTF finalizes CIM_UnixProcess
#define CLASS_UNIX_PROCESS                 "PG_UnixProcess"

// ==========================================================================
// The number of keys for the classes.
// ==========================================================================

#define NUMKEYS_UNIX_PROCESS                    6

// ==========================================================================
// Property names.  These values are returned by the provider as
// the property names.
// ==========================================================================

#define PROPERTY_CS_CREATION_CLASS_NAME      "CSCreationClassName"
#define PROPERTY_CS_NAME                     "CSName"
#define PROPERTY_OS_CREATION_CLASS_NAME      "OSCreationClassName"
#define PROPERTY_OS_NAME                     "OSName"
#define PROPERTY_CREATION_CLASS_NAME         "CreationClassName"
#define PROPERTY_HANDLE                      "Handle"

#define PROPERTY_CAPTION                     "Caption"
#define PROPERTY_DESCRIPTION                 "Description"
#define PROPERTY_INSTALL_DATE                "InstallDate"
#define PROPERTY_STATUS                      "Status"

// Properties in CIM_Process

#define PROPERTY_NAME                        "Name"
#define PROPERTY_PRIORITY                    "Priority"
#define PROPERTY_EXECUTION_STATE             "ExecutionState"
#define PROPERTY_OTHER_EXECUTION_DESCRIPTION "OtherExecutionDescription"
#define PROPERTY_CREATION_DATE               "CreationDate"
#define PROPERTY_TERMINATION_DATE            "TerminationDate"
#define PROPERTY_KERNEL_MODE_TIME            "KernelModeTime"
#define PROPERTY_USER_MODE_TIME              "UserModeTime"
#define PROPERTY_WORKING_SET_SIZE            "WorkingSetSize"

// Properties in UnixProcess

#define PROPERTY_PARENT_PROCESS_ID           "ParentProcessID"
#define PROPERTY_REAL_USER_ID                "RealUserID"
#define PROPERTY_PROCESS_GROUP_ID            "ProcessGroupID"
#define PROPERTY_PROCESS_SESSION_ID          "ProcessSessionID"
#define PROPERTY_PROCESS_TTY                 "ProcessTTY"
#define PROPERTY_MODULE_PATH                 "ModulePath"
#define PROPERTY_PARAMETERS                  "Parameters"
#define PROPERTY_PROCESS_NICE_VALUE          "ProcessNiceValue"
#define PROPERTY_PROCESS_WAITING_FOR_EVENT   "ProcessWaitingForEvent"

PEGASUS_USING_STD;
PEGASUS_USING_PEGASUS;


ProcessProvider::ProcessProvider()
{
  // cout << "ProcessProvider::ProcessProvider()" << endl;
}

ProcessProvider::~ProcessProvider()
{
  // cout << "ProcessProvider::~ProcessProvider()" << endl;
}


// =============================================================================
// NAME              : createInstance
// DESCRIPTION       : Create a UnixProcess instance.
// ASSUMPTIONS       : None
// PRE-CONDITIONS    :
// POST-CONDITIONS   :
// NOTES             : Currently not supported.
// PARAMETERS        :
// =============================================================================

void ProcessProvider::createInstance(const OperationContext &context,
                    const CIMObjectPath           &instanceName,
                    const CIMInstance            &instanceObject,
                    ObjectPathResponseHandler &handler)

{
  // Current not implemented, but could be done by
  // spawning a process
  throw CIMNotSupportedException(String::EMPTY);
}


// =============================================================================
// NAME              : deleteInstance
// DESCRIPTION       : Delete a UnixProcess instance.
// ASSUMPTIONS       : None
// PRE-CONDITIONS    :
// POST-CONDITIONS   :
// NOTES             : Currently not supported.
// PARAMETERS        :
// =============================================================================

void ProcessProvider::deleteInstance(const OperationContext &context,
                    const CIMObjectPath           &instanceReference,
                    ResponseHandler &handler)

{
  // Currently not implemented, but could be done with kill
  throw CIMNotSupportedException(String::EMPTY);
}


// =============================================================================
// NAME              : enumerateInstances
// DESCRIPTION       : Enumerates all of the UnixProcess instances.
//                   : An array of instance references is returned.
// ASSUMPTIONS       : None
// PRE-CONDITIONS    :
// POST-CONDITIONS   :
// NOTES             : LocalOnly, DeepInheritance and propertyList are not
//                   : respected by this provider. Localization is not supported
// PARAMETERS        :
// =============================================================================

void ProcessProvider::enumerateInstances(
    const OperationContext & context,
    const CIMObjectPath & classReference,
    const Boolean includeQualifiers,
    const Boolean includeClassOrigin,
    const CIMPropertyList & propertyList,
    InstanceResponseHandler & handler)
{
    // cout << "ProcessProvider::enumerateInstances()" << endl;

    int pIndex;
    CIMName className = classReference.getClassName();
    CIMNamespaceName nameSpace = classReference.getNameSpace();
    Process _p;

    // Validate the classname
    _checkClass(className);

    // Notify processing is starting
    handler.processing();

    // We will only return instances on enumeration requests
    // to the "leaf" class
    if (className.equal (CLASS_UNIX_PROCESS))
    {
      // Get the process information and construct and deliver
      // an instance for each process
      // HP-UX NOTE: loadProcessInfo will change the value of pIndex
      // in order to skip over unused entries in the process table
      // Do not depend on this counter incrementing sequentially!
      for (pIndex = 0; _p.loadProcessInfo(pIndex); pIndex++)
        handler.deliver(_constructInstance(CLASS_UNIX_PROCESS,
                                           nameSpace,
                                           _p));
    }

    // Notify processing is complete
    handler.complete();

    return;
}  // enumerateInstances



// =============================================================================
// NAME              : enumerateInstanceNames
// DESCRIPTION       : Enumerates all of the UnixProcess instance names.
//                     An array of instance references is returned.
// ASSUMPTIONS       : None
// PRE-CONDITIONS    :
// POST-CONDITIONS   :
// NOTES             : Localization is not supported by this provider.
// PARAMETERS        :
// =============================================================================

void ProcessProvider::enumerateInstanceNames(const OperationContext &ctx,
                            const CIMObjectPath &ref,
                            ObjectPathResponseHandler &handler)
{
    // cout << "ProcessProvider::enumerateInstanceNames()" << endl;

    int pIndex;
    Process _p;
    CIMName className = ref.getClassName();
    CIMNamespaceName nameSpace = ref.getNameSpace();

    // Validate the classname
    _checkClass(className);

    // Notify processing is starting
    handler.processing();

    // We are only going to respond to enumeration requests on
    // CLASS_UNIX_PROCESS
    if (className.equal (CLASS_UNIX_PROCESS))
    {
      // Get the process information and deliver an ObjectPath for
      // each process
      // Note that loadProcessInfo modifies pIndex to point to the
      // next process structure before the loop increments it!
      for (pIndex = 0; _p.loadProcessInfo(pIndex); pIndex++)
      {
        // Deliver the names
        handler.deliver(CIMObjectPath(String::EMPTY, // hostname
                                      nameSpace,
                                      CLASS_UNIX_PROCESS,
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

void ProcessProvider::getInstance(const OperationContext &ctx,
                 const CIMObjectPath           &instanceName,
                 const Boolean                 includeQualifiers,
                 const Boolean                 includeClassOrigin,
                 const CIMPropertyList        &propertyList,
                 InstanceResponseHandler &handler)
{
  // cout << "ProcessProvider::getInstance(" << instanceName << ")" << endl;

  CIMKeyBinding kb;
  CIMName className = instanceName.getClassName();
  CIMNamespaceName nameSpace = instanceName.getNameSpace();
  String handle;
  int i;
  int keysFound; // this will be used as a bit array
  Process _p;

  // Validate the classname
  _checkClass(className);

  // Extract the key values
  Array<CIMKeyBinding> kbArray = instanceName.getKeyBindings();

  // Leave immediately if wrong number of keys
  if ( kbArray.size() != NUMKEYS_UNIX_PROCESS )
    throw CIMInvalidParameterException("Wrong number of keys");

  // Validate the keys.
  // Each loop iteration will set a bit in keysFound when a valid
  // key is found. If the expected bits aren't all set when
  // the loop finishes, it's a problem
  for(i=0, keysFound=0; i<NUMKEYS_UNIX_PROCESS; i++)
  {
    kb = kbArray[i];

    CIMName keyName = kb.getName();
    String keyValue = kb.getValue();

    // CSCreationClassName can be empty or must match
    if (keyName.equal (PROPERTY_CS_CREATION_CLASS_NAME))
    {
      if (String::equal(keyValue, String::EMPTY) ||
          String::equalNoCase(keyValue, CLASS_CIM_UNITARY_COMPUTER_SYSTEM))
        keysFound |= 1;
      else
        throw CIMInvalidParameterException(keyValue+": bad value for key "+
            keyName.getString());
    }

    // CSName can be empty or must match
    else if (keyName.equal (PROPERTY_CS_NAME))
    {
      if (String::equal(keyValue, String::EMPTY) ||
          String::equalNoCase(keyValue, _getCSName()))
        keysFound |= 2;
      else
        throw CIMInvalidParameterException(keyValue+": bad value for key "+
            keyName.getString());
    }

    // OSCreationClassName can be empty or must match
    else if (keyName.equal (PROPERTY_OS_CREATION_CLASS_NAME))
    {
      if (String::equal(keyValue, String::EMPTY) ||
          String::equalNoCase(keyValue, CLASS_CIM_OPERATING_SYSTEM))
        keysFound |= 4;
      else
        throw CIMInvalidParameterException(keyValue+": bad value for key "+
            keyName.getString());
    }

    // OSName can be empty or must match
    else if (keyName.equal (PROPERTY_OS_NAME))
    {
      if (String::equal(keyValue, String::EMPTY) ||
          String::equalNoCase(keyValue, _getOSName()))
        keysFound |= 8;
      else
        throw CIMInvalidParameterException(keyValue+": bad value for key "+
            keyName.getString());
    }

    // CreationClassName can be empty or must match
    else if (keyName.equal (PROPERTY_CREATION_CLASS_NAME))
    {
      if (String::equal(keyValue, String::EMPTY) ||
          String::equalNoCase(keyValue, CLASS_UNIX_PROCESS))
        keysFound |= 16;
      else
        throw CIMInvalidParameterException(keyValue+": bad value for key "+
            keyName.getString());
    }

    // Handle must be a valid pid, but we will know that later
    // For now, just verify that it's present
    else if (keyName.equal (PROPERTY_HANDLE))
    {
      handle = keyValue;
      keysFound |= 32;
    }

    // Key name was not recognized by any of the above tests
    else throw CIMInvalidParameterException(keyName.getString() +
        ": Unrecognized key");

  } // for

  // We could get here if we didn't get all the keys, which
  // could happen if the right number of keys were supplied,
  // and they all had valid names and values, but there were
  // any duplicates (e.g., two Handles, no OSName)
  if(keysFound != (1<<NUMKEYS_UNIX_PROCESS)-1)
    throw CIMInvalidParameterException("Bad object name");

  /* Find the instance.  First convert the instance id which is the */
  /* process handle to an integer.  This is necessary because the   */
  /* handle is the process id on HP-UX which must be passed to      */
  /* pstat_getproc() as an integer.                                 */

  /* Get the process information. */
  if (_p.findProcess(handle))
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

  throw CIMObjectNotFoundException(handle+": No such process");

  return; // can never execute, but required to keep compiler happy
}

/*
================================================================================
NAME              : modifyInstance
DESCRIPTION       : Modify a UnixProcess instance.
ASSUMPTIONS       : None
PRE-CONDITIONS    :
POST-CONDITIONS   :
NOTES             : Currently not supported.
PARAMETERS        :
================================================================================
*/
void ProcessProvider::modifyInstance(
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
void ProcessProvider::initialize(CIMOMHandle &ch)
{
   // cout << "ProcessProvider::initialize()" << endl;
  _cimomHandle = ch;

  // call platform-specific routine to get values
  Process _p;
  _hostName = _p.getCSName();
  _osName = _p.getOSName();
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
void ProcessProvider::terminate()
{
  // cout << "ProcessProvider::terminate()" << endl;
  delete this;
}

// =============================================================================
// NAME              : _constructKeyBindings
// DESCRIPTION       : Constructs an array of keybindings for process
// ASSUMPTIONS       : None
// PRE-CONDITIONS    :
// POST-CONDITIONS   :
// NOTES             :
// PARAMETERS        : className, Process
// =============================================================================

Array<CIMKeyBinding> ProcessProvider::_constructKeyBindings(const Process& _p)
{
    Array<CIMKeyBinding> keyBindings;

    // Construct the key bindings
    keyBindings.append(CIMKeyBinding(
        PROPERTY_CS_CREATION_CLASS_NAME,
        CLASS_CIM_UNITARY_COMPUTER_SYSTEM,
        CIMKeyBinding::STRING));

    keyBindings.append(CIMKeyBinding(
        PROPERTY_CS_NAME,
        _getCSName(),
        CIMKeyBinding::STRING));

    keyBindings.append(CIMKeyBinding(
        PROPERTY_OS_CREATION_CLASS_NAME,
        CLASS_CIM_OPERATING_SYSTEM,
        CIMKeyBinding::STRING));

    keyBindings.append(CIMKeyBinding(
        PROPERTY_OS_NAME,
        _getOSName(),
        CIMKeyBinding::STRING));

    keyBindings.append(CIMKeyBinding(
        PROPERTY_CREATION_CLASS_NAME,
        CLASS_UNIX_PROCESS,
        CIMKeyBinding::STRING));

    keyBindings.append(CIMKeyBinding(
        PROPERTY_HANDLE,
        _p.getHandle(),
        CIMKeyBinding::STRING));

    return keyBindings;
}

// =============================================================================
// NAME              : _constructInstance
// DESCRIPTION       : Constructs instance by adding its properties. The
//                   : Process instance argument has already been filled in
//                   : with data from an existing process
// ASSUMPTIONS       : None
// PRE-CONDITIONS    :
// POST-CONDITIONS   :
// NOTES             :
// PARAMETERS        : className, Process
// =============================================================================

CIMInstance ProcessProvider::_constructInstance(
    const CIMName &className,
    const CIMNamespaceName &nameSpace,
    const Process &_p)
{
  String s;
  Array<String> as;
  Uint16 i16;
  Uint32 i32;
  Uint64 i64;
  CIMDateTime d;

  CIMInstance inst(className);

  // Set path

  inst.setPath(CIMObjectPath(String::EMPTY, // hostname
                             nameSpace,
                             CLASS_UNIX_PROCESS,
                             _constructKeyBindings(_p)));

// CIM_ManagedElement

//   string Caption
  if (_p.getCaption(s))
    inst.addProperty(CIMProperty(PROPERTY_CAPTION,s));

//   string Description
  if (_p.getDescription(s))
    inst.addProperty(CIMProperty(PROPERTY_DESCRIPTION,s));

// CIM_ManagedSystemElement

//   datetime InstallDate
  if (_p.getInstallDate(d))
    inst.addProperty(CIMProperty(PROPERTY_INSTALL_DATE,d));

//   string Name    // Populated in CIM_Process

//   string Status
  if (_p.getStatus(s))
    inst.addProperty(CIMProperty(PROPERTY_STATUS,s));

// CIM_LogicalElement
//   ** No local properties added in this class **

// ===========================================
// The following properties are in CIM_Process
// ===========================================

  // The keys for this class are:
  // [ key ] string CSCreationClassName
  // [ key ] string CSName
  // [ key ] string OSCreationClassName
  // [ key ] string OSName
  // [ key ] string CreationClassName
  // [ key ] string Handle

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

//   string Name
  if (_p.getName(s))
    inst.addProperty(CIMProperty(PROPERTY_NAME,s));

//   uint32 Priority
  if (_p.getPriority(i32))
    inst.addProperty(CIMProperty(PROPERTY_PRIORITY,i32));

//   uint16 ExecutionState
  if (_p.getExecutionState(i16))
    inst.addProperty(CIMProperty(PROPERTY_EXECUTION_STATE,i16));

//   string OtherExecutionDescription
  if (_p.getOtherExecutionDescription(s))
  {
    // if an empty string was returned, the value must be set to NULL
    // with type string, not an array
    if (String::equal(s,String::EMPTY))
    {
      inst.addProperty(CIMProperty(PROPERTY_OTHER_EXECUTION_DESCRIPTION,
                                   CIMValue(CIMTYPE_STRING, false)));
    }
    else
    {
      inst.addProperty(CIMProperty(PROPERTY_OTHER_EXECUTION_DESCRIPTION,s));
    }
  }

//   datetime CreationDate
  if (_p.getCreationDate(d))
    inst.addProperty(CIMProperty(PROPERTY_CREATION_DATE,d));

//   datetime TerminationDate
  if (_p.getTerminationDate(d))
    inst.addProperty(CIMProperty(PROPERTY_TERMINATION_DATE,d));

//   uint64 KernelModeTime
  if (_p.getKernelModeTime(i64))
    inst.addProperty(CIMProperty(PROPERTY_KERNEL_MODE_TIME,i64));

//   uint64 UserModeTime
  if (_p.getUserModeTime(i64))
    inst.addProperty(CIMProperty(PROPERTY_USER_MODE_TIME,i64));

//   uint64 WorkingSetSize
  if (_p.getWorkingSetSize(i64))
    inst.addProperty(CIMProperty(PROPERTY_WORKING_SET_SIZE,i64));

// =============================================
// return if the requested class was CIM_Process
// =============================================

  if (className.equal (CLASS_CIM_PROCESS)) return inst;

// ===========================================
// The following properties are in UnixProcess
// ===========================================

//    string ParentProcessID
  if (_p.getParentProcessID(s))
    inst.addProperty(CIMProperty(PROPERTY_PARENT_PROCESS_ID,s));

//    uint64 RealUserID
  if (_p.getRealUserID(i64))
    inst.addProperty(CIMProperty(PROPERTY_REAL_USER_ID,i64));

//    uint64 ProcessGroupID
  if (_p.getProcessGroupID(i64))
    inst.addProperty(CIMProperty(PROPERTY_PROCESS_GROUP_ID,i64));

//    uint64 ProcessSessionID
  if (_p.getProcessSessionID(i64))
    inst.addProperty(CIMProperty(PROPERTY_PROCESS_SESSION_ID,i64));

//    string ProcessTTY
  if (_p.getProcessTTY(s))
    inst.addProperty(CIMProperty(PROPERTY_PROCESS_TTY,s));

//    string ModulePath
  if (_p.getModulePath(s))
    inst.addProperty(CIMProperty(PROPERTY_MODULE_PATH,s));

//    string Parameters[]
  if (_p.getParameters(as))
    inst.addProperty(CIMProperty(PROPERTY_PARAMETERS,as));

//    uint32 ProcessNiceValue
  if (_p.getProcessNiceValue(i32))
    inst.addProperty(CIMProperty(PROPERTY_PROCESS_NICE_VALUE,i32));

//    string ProcessWaitingForEvent
  if (_p.getProcessWaitingForEvent(s))
    inst.addProperty(CIMProperty(PROPERTY_PROCESS_WAITING_FOR_EVENT,s));

  return inst;
}


/*
================================================================================
NAME              : _getOSName
DESCRIPTION       : Call uname() and get the operating system name.
ASSUMPTIONS       : None
PRE-CONDITIONS    :
POST-CONDITIONS   :
NOTES             :
================================================================================
*/
String &ProcessProvider::_getOSName()
{
  // This routine could do something dynamic; for now
  // it returns a string that was captured when the
  // initialize() method was invoked
  return _osName;
}


/*
================================================================================
NAME              : _getCSName
DESCRIPTION       : return the hostname.
ASSUMPTIONS       : None
PRE-CONDITIONS    :
POST-CONDITIONS   :
NOTES             :
================================================================================
*/
String &ProcessProvider::_getCSName()
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
void ProcessProvider::_checkClass(CIMName& className)
{
  if (!className.equal (CLASS_CIM_PROCESS) &&
      !className.equal (CLASS_UNIX_PROCESS))
    throw CIMNotSupportedException(className.getString() +
        ": Class not supported");
}
