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
// try out all client operations on both providers
//
// The order of operations tested is:
//   enumerateInstanceNames
//   getInstance
//   getInstance with bad key name
//   createInstance
//   deleteInstance
//   enumerateInstances
//   modifyInstance
//
// getInstance() needs a set of keys from enumerateInstanceNames()
// but will not iterate through all of them. Instead, it will
// just pick the middle one. modifyInstance, createInstance, and
// deleteInstance will simply use the instance (or its keys) from
// getInstance.
//
// Some tests are expected to throw NOT_SUPPORTED or other
// exceptions, so any other behavior is a provider failure.
// ==========================================================================

// ==========================================================================
// Includes
// ==========================================================================

#include <Pegasus/Client/CIMClient.h>
#include "TestCSProvider.h"

// ==========================================================================
// Miscellaneous defines
// ==========================================================================

#define NAMESPACE "root/cimv2"
#define TIMEOUT   10000                    // timeout value in milliseconds

// ==========================================================================
// Class names.  These values are the names of the classes that
// are common for all of the providers.
// ==========================================================================

#define CLASS_CIM_COMPUTER_SYSTEM          "CIM_ComputerSystem"
#define CLASS_CIM_UNITARY_COMPUTER_SYSTEM  "CIM_UnitaryComputerSystem"
#define CLASS_EXTENDED_COMPUTER_SYSTEM     "PG_ComputerSystem"

// ==========================================================================
// The number of keys for the classes.
// ==========================================================================

#define NUMKEYS_COMPUTER_SYSTEM             2

// ==========================================================================
// Property names.  These values are returned by the provider as
// the property names.
// ==========================================================================

// Property Names

// CIM_ManagedElement
#define PROPERTY_CAPTION                     "Caption"
#define PROPERTY_DESCRIPTION                 "Description"

// CIM_ManagedSystemElement
#define PROPERTY_INSTALL_DATE                "InstallDate"
// Name is overridden in CIM_ComputerSystem
// #define PROPERTY_NAME                     "Name"
#define PROPERTY_STATUS                      "Status"

// CIM_LogicalElement
// -- none --

// CIM_System
#define PROPERTY_CREATION_CLASS_NAME         "CreationClassName"
#define PROPERTY_NAME                        "Name"
// NameFormat is overridden in CIM_ComputerSystem
// #define PROPERTY_NAME_FORMAT              "NameFormat"
#define PROPERTY_PRIMARY_OWNER_NAME          "PrimaryOwnerName"
#define PROPERTY_PRIMARY_OWNER_CONTACT       "PrimaryOwnerContact"
#define PROPERTY_ROLES                       "Roles"

// CIM_ComputerSystem
#define PROPERTY_NAME_FORMAT                 "NameFormat"
#define PROPERTY_OTHER_IDENTIFYING_INFO      "OtherIdentifyingInfo"
#define PROPERTY_IDENTIFYING_DESCRIPTIONS    "IdentifyingDescriptions"
#define PROPERTY_DEDICATED                   "Dedicated"
#define PROPERTY_RESET_CAPABILITY            "ResetCapability"
#define PROPERTY_POWER_MANAGEMENT_CAPABILITIES "PowerManagementCapabilities"
#define METHOD_SET_POWER_STATE               "SetPowerState"
#define METHOD_SET_POWER_STATE_WITH_OPTIONS  "SetPowerStateWithOptions"

// CIM_UnitaryComputerSystem
#define PROPERTY_INITIAL_LOAD_INFO           "InitialLoadInfo"
#define PROPERTY_LAST_LOAD_INFO              "LastLoadInfo"
#define PROPERTY_POWER_MANAGEMENT_SUPPORTED  "PowerManagementSupported"
#define PROPERTY_POWER_STATE                 "PowerState"
#define PROPERTY_WAKE_UP_TYPE                "WakeUpType"

// PG_ComputerSystem
#define PROPERTY_IDENTIFICATION_NUMBER       "IdentificationNumber"
#define PROPERTY_PRIMARY_OWNER_PAGER         "PrimaryOwnerPager"
#define PROPERTY_SECONDARY_OWNER_CONTACT     "SecondaryOwnerContact"
#define PROPERTY_SECONDARY_OWNER_NAME        "SecondaryOwnerName"
#define PROPERTY_SECONDARY_OWNER_PAGER       "SecondaryOwnerPager"
#define PROPERTY_SERIAL_NUMBER               "SerialNumber"


PEGASUS_USING_STD;
PEGASUS_USING_PEGASUS;

void errorExit(Exception& e)
{
  cout << "Error: Failed" << endl << e.getMessage() << endl;
  exit(1);
}

int testClass(CIMClient &c, const String& className)
{
  Array<CIMObjectPath> refs;

  // =======================================================================
  // enumerateInstanceNames
  // =======================================================================

  cout << "+++++ enumerateInstanceNames(" << className << ") ";
  try
  {
    refs = c.enumerateInstanceNames(NAMESPACE,className);
  }
  catch (Exception& e)
  {
    cout << endl;
    errorExit(e);
  }

  cout << refs.size() << " instances" << endl;
  // if zero instances, not an error, but can't proceed
  if (refs.size() == 0)
  {
    cout << "+++++ test completed early" << endl;
    return 0;
  }

  // =======================================================================
  // getInstance
  // =======================================================================

  // -------------------- First do normal getInstance() --------------------

  // pick the middle instance of the bunch
  int i = (refs.size()-1) >> 1;  // This is a shift right, not streamio!
  CIMObjectPath ref = refs[i];
  CIMInstance inst;
  cout << "+++++ getInstance #" << i << endl;
  try
  {
    inst = c.getInstance(NAMESPACE,ref);
  }
  catch (Exception& e)
  {
    errorExit(e);
  }

  // ATTN-MG-20020501: Can add some property value checks here

  // ------------------ do getInstance() with bad key ----------------------

  Array<CIMKeyBinding> kb = ref.getKeyBindings();
  // mess up first key name
  kb[0].setName("foobar");
  ref.setKeyBindings(kb);

  int status = 0;

  cout << "+++++ getInstance with bad key" << endl;
  try
  {
    inst = c.getInstance(NAMESPACE,ref);
  }
  catch (CIMException& e)
  {
    if (e.getCode() == CIM_ERR_INVALID_PARAMETER) status = 1;
  }
  catch (Exception&)
  {
    // any other exception is a failure; leave status alone
  }
  if (status == 0)
  {
    cout << "+++++ Error: bad instance name not rejected" << endl;
    return 1;
  }

  // =======================================================================
  // createInstance
  // =======================================================================

  CIMObjectPath ref2;
  cout << "+++++ createInstance" << endl;
  status = 0;
  try
  {
    ref2 = c.createInstance(NAMESPACE,inst);
  }
  catch (CIMException& e)
  {
    if (e.getCode() == CIM_ERR_NOT_SUPPORTED) status = 1;
  }
  catch (Exception&)
  {
    // any other Exception is a problem; leave status alone
  }
  if (status == 0)
  {
    cout << "+++++ Error: createInstance didn't throw exception" << endl;
    return 1;
  }

  // =======================================================================
  // deleteInstance
  // =======================================================================

  cout << "+++++ deleteInstance" << endl;
  status = 0;
  try
  {
    c.deleteInstance(NAMESPACE,ref);
  }
  catch (CIMException& e)
  {
    if (e.getCode() == CIM_ERR_NOT_SUPPORTED) status = 1;
  }
  catch (Exception&)
  {
    // any other Exception is a problem; leave status alone
  }
  if (status == 0)
  {
    cout << "+++++ Error: deleteInstance didn't throw exception" << endl;
    return 1;
  }

  // =======================================================================
  // enumerateInstances
  // =======================================================================

  cout << "+++++ enumerateInstances(" << className << ")" << endl;

  Array<CIMInstance> ia;
  try
  {
    ia = c.enumerateInstances(NAMESPACE,className);
  }
  catch (Exception& e)
  {
    errorExit(e);
  }

  // There should be several instances
  if (ia.size() == 0)
  {
    cout << "+++++ Error: enumerateInstances on " << className <<
        " returned too few instances" << endl;
    cout << "+++++ Test failed" << endl;
    return 1;
  }

  // =======================================================================
  // modifyInstance
  // =======================================================================

  // We do modifyInstance after enumerateInstances, because
  // modifyInstance requires a CIMInstance argument, which
  // is conveniently what was returned by enumerateInstances

  CIMInstance ni = ia[(ia.size()-1) >> 1]; // pick the middle one

  cout << "+++++ modifyInstance" << endl;
  status = 0;
  try
  {
    c.modifyInstance(NAMESPACE,ni);
  }
  catch (CIMException& e)
  {
    if (e.getCode() == CIM_ERR_NOT_SUPPORTED) status = 1;
  }
  catch (Exception&)
  {
    // any other Exception is a problem; leave status alone
  }
  if (status == 0)
  {
    cout << "+++++ Error: modifyInstance didn't throw exception" << endl;
    return 1;
  }

  // =======================================================================
  // Tests completed
  // =======================================================================

  return 0;
}

int main()
{
  cout << "+++++ Testing ComputerSystem Provider" << endl;

  //
  // This MUST be inside of main for OpenVMS because the OS has a problem
  //  with initializing global variables 'in a timely manner' during
  //  executable initialization.
  //

  CIMClient c;

  // Connect
  try
  {
    c.connectLocal ();
  }
  catch (Exception& e)
  {
    errorExit(e);
  }

  int rc;
  if ((rc = testClass(c, CLASS_CIM_COMPUTER_SYSTEM)) != 0) return rc;
  if ((rc = testClass(c, CLASS_CIM_UNITARY_COMPUTER_SYSTEM)) != 0) return rc;
  if ((rc = testClass(c, CLASS_EXTENDED_COMPUTER_SYSTEM)) != 0) return rc;
  cout << "+++++ passed all tests" << endl;
  return 0;
}
