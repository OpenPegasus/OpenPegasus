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
//   createInstance
//   deleteInstance
//   enumerateInstances
//   modifyInstance
//
// getInstance() needs some names from enumerateInstanceNames()
// but will not iterate through all of them. Instead, it will
// just pick the middle one. modifyInstance, createInstance, and
// deleteInstance will simply use the instance (or its keys) from
// getInstance.
//
// Some operations are expected to fail with NOT_SUPPORTED, so
// any other behavior is a provider failure.
// ==========================================================================

// ==========================================================================
// Includes
// ==========================================================================

#include <Pegasus/Client/CIMClient.h>
#include "TestProcessorProvider.h"
#include "../ProcessorPlatform.h"

// ==========================================================================
// Miscellaneous defines
// ==========================================================================

const String NAMESPACE("root/cimv2");

// ==========================================================================
// Class names.  These values are the names of the classes that
// are common for all of the providers.
//
// We use CIM_UnitaryComputerSystem as the value of the key
// SystemCreationClassName, because this class has properties that
// are important for clients of this provider
// ==========================================================================

static const String CLASS_CIM_PROCESSOR("CIM_Processor");
static const String CLASS_PROCESSOR("PG_Processor");
static const String CLASS_CIM_UNITARY_COMPUTER_SYSTEM(
    "CIM_UnitaryComputerSystem");

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
static const String PROPERTY_STATUS("Status");

// Properties in CIM_LogicalElement
// ** none **

// Properties in CIM_LogicalDevice

static const String PROPERTY_POWER_MANAGEMENT_SUPPORTED(
    "PowerManagementSupported");
static const String PROPERTY_POWER_MANAGEMENT_CAPABILITYES(
    "PowerManagementCapabilities");
static const String PROPERTY_AVAILABILITY("Availability");
static const String PROPERTY_STATUS_INFO("StatusInfo");
static const String PROPERTY_LAST_ERROR_CODE("LastErrorCode");
static const String PROPERTY_ERROR_DESCRIPTION("ErrorDescription");
static const String PROPERTY_ERROR_CLEARED("ErrorCleared");
static const String PROPERTY_OTHER_IDENTIFYING_INFO("OtherIdentifyingInfo");
static const String PROPERTY_POWER_ON_HOURS("PowerOnHours");
static const String PROPERTY_TOTAL_POWER_HOURS("TotalPowerHours");
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

static CIMClient c;
static Boolean processorTestVerbose = false;

void errorExit(Exception& e)
{
  cout << "Error: Failed" << endl << e.getMessage() << endl;
  exit(1);
}

int testClass(const String& className)
{
  Array<CIMObjectPath> refs;

  cout << endl << "+++++ Testing Class " << className << " +++++" << endl;

  // =======================================================================
  // enumerateInstanceNames
  // =======================================================================

  cout << "+++++ enumerateInstanceNames(" << className << "): ";
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

  // There must be at least 1 processor or something's wrong
  if (refs.size() < 1)
  {
    cout << "+++++ Error: too few instances returned" << endl;
    return 1;
  }

  // =======================================================================
  // getInstance
  // =======================================================================

  // -------------------- First do normal getInstance() --------------------

  // pick the middle instance of the bunch
  Uint32 i = (refs.size()-1) >> 1;  // This is a shift right, not streamio!
  CIMObjectPath ref = refs[i];
  CIMInstance inst;
  cout << "+++++ getInstance " << i << endl;
  try
  {
    inst = c.getInstance(NAMESPACE,ref);
  }
  catch (Exception& e)
  {
    errorExit(e);
  }

  if (processorTestVerbose)
  {
    // Display keys
    Array<CIMKeyBinding> keys = ref.getKeyBindings();
    cout << "  Keys:" << endl;
    for (i=0; i<keys.size(); i++)
        cout << "    " << keys[i].getName().getString() << " = "
             << keys[i].getValue() << endl;
  }

  // check returned property values

  // first get the PID and load a processor object
  String devID;
  inst.getProperty(inst.findProperty("DeviceID")).getValue().get(devID);
  Processor p;
  // error if can't get the process
  if (!p.findProcessor(devID))
  {
    cout << "+++++ Error: can't find process corresponding to instance" << endl;
    return 1;
  }

  if (processorTestVerbose) cout << "  Properties:" << endl;

  Boolean ba, bb;
  String sa, sb;
  Array<String> asa, asb;
  Uint16 i16a, i16b;
  Array<Uint16> ai16a, ai16b;
  Uint32 i32a, i32b;
  Uint64 i64a, i64b;
  CIMDateTime da, db;

  // For each property, get it from the just-loaded process
  // object and compare with what was returned by getInstance()

  // ===================== CIM_ManagedElement =========================

  // Caption and Description are common to all classes
  if (p.getCaption(sa))
  {
    if (processorTestVerbose) cout << "    Caption" << endl;
    inst.getProperty(inst.findProperty("Caption")).getValue().get(sb);
    if (sa != sb)
    {
      cout << "+++++ Error: property mismatch: Caption" << endl;
      return 1;
    }
  }

  if (p.getDescription(sa))
  {
    if (processorTestVerbose) cout << "    Description" << endl;
    inst.getProperty(inst.findProperty("Description")).getValue().get(sb);
    if (sa != sb)
    {
      cout << "+++++ Error: property mismatch: Description" << endl;
      return 1;
    }
  }

  // The rest of the properties to check depend on the
  // class we are testing

  // ===================== CIM_ManagedSystemElement =========================

  if (String::equalNoCase(className,"CIM_Processor") ||
      String::equalNoCase(className,"PG_Processor"))
  {
    if (p.getInstallDate(da))
    {
      if (processorTestVerbose) cout << "    InstallDate" << endl;
      inst.getProperty(inst.findProperty("InstallDate")).getValue().get(db);
      if (da != db)
      {
        cout << "+++++ Error: property mismatch: InstallDate" << endl;
        return 1;
      }
    }

    if (p.getStatus(sa))
    {
      if (processorTestVerbose) cout << "    Status" << endl;
      inst.getProperty(inst.findProperty("Status")).getValue().get(sb);
      if (sa != sb)
      {
        cout << "+++++ Error: property mismatch: Status" << endl;
        return 1;
      }
    }

  // ===================== CIM_LogicalDevice =========================

    if (p.getPowerManagementSupported(ba))
    {
      if (processorTestVerbose) cout << "    PowerManagementSupported" << endl;
      inst.getProperty(
          inst.findProperty("PowerManagementSupported")).getValue().get(bb);
      if (ba != bb)
      {
        cout << "+++++ Error: property mismatch: PowerManagementSupported" <<
            endl;
        return 1;
      }
    }

    if (p.getPowerManagementCapabilities(ai16a))
    {
      if (processorTestVerbose)
          cout << "    PowerManagementCapabilities" << endl;
      inst.getProperty(inst.findProperty(
          "PowerManagementCapabilities")).getValue().get(ai16b);
      if (ai16a != ai16b)
      {
        cout << "+++++ Error: property mismatch: PowerManagementCapabilities"
             << endl;
        return 1;
      }
    }

    if (p.getAvailability(i16a))
    {
      if (processorTestVerbose) cout << "    Availability" << endl;
      inst.getProperty(inst.findProperty("Availability")).getValue().get(i16b);
      if (i16a != i16b)
      {
        cout << "+++++ Error: property mismatch: Availability" << endl;
        return 1;
      }
    }

    if (p.getStatusInfo(i16a))
    {
      if (processorTestVerbose) cout << "    StatusInfo" << endl;
      inst.getProperty(inst.findProperty("StatusInfo")).getValue().get(i16b);
      if (i16a != i16b)
      {
        cout << "+++++ Error: property mismatch: StatusInfo" << endl;
        return 1;
      }
    }

    if (p.getLastErrorCode(i32a))
    {
      if (processorTestVerbose) cout << "    LastErrorCode" << endl;
      inst.getProperty(inst.findProperty("LastErrorCode")).getValue().get(i32b);
      if (i32a != i32b)
      {
        cout << "+++++ Error: property mismatch: LastErrorCode" << endl;
        return 1;
      }
    }

    if (p.getErrorDescription(sa))
    {
      if (processorTestVerbose) cout << "    ErrorDescription" << endl;
      inst.getProperty(
          inst.findProperty("ErrorDescription")).getValue().get(sb);
      if (sa != sb)
      {
        cout << "+++++ Error: property mismatch: ErrorDescription" << endl;
        return 1;
      }
    }

    if (p.getErrorCleared(ba))
    {
      if (processorTestVerbose) cout << "    ErrorCleared" << endl;
      inst.getProperty(inst.findProperty("ErrorCleared")).getValue().get(bb);
      if (ba != bb)
      {
        cout << "+++++ Error: property mismatch: ErrorCleared" << endl;
        return 1;
      }
    }

    if (p.getOtherIdentifyingInfo(asa))
    {
      if (processorTestVerbose) cout << "    OtherIdentifyingInfo" << endl;
      inst.getProperty(
          inst.findProperty("OtherIdentifyingInfo")).getValue().get(asb);
      if (asa != asb)
      {
        cout << "+++++ Error: property mismatch: OtherIdentifyingInfo" << endl;
        return 1;
      }
    }

    if (p.getPowerOnHours(i64a))
    {
      if (processorTestVerbose) cout << "    PowerOnHours" << endl;
      inst.getProperty(inst.findProperty("PowerOnHours")).getValue().get(i64b);
      if (i64a != i64b)
      {
        cout << "+++++ Error: property mismatch: PowerOnHours" << endl;
        return 1;
      }
    }

    if (p.getTotalPowerOnHours(i64a))
    {
      if (processorTestVerbose) cout << "    TotalPowerOnHours" << endl;
      inst.getProperty(
          inst.findProperty("TotalPowerOnHours")).getValue().get(i64b);
      if (i64a != i64b)
      {
        cout << "+++++ Error: property mismatch: TotalPowerOnHours" << endl;
        return 1;
      }
    }

    if (p.getIdentifyingDescriptions(asa))
    {
      if (processorTestVerbose) cout << "    IdentifyingDescriptions" << endl;
      inst.getProperty(
          inst.findProperty("IdentifyingDescriptions")).getValue().get(asb);
      if (asa != asb)
      {
        cout << "+++++ Error: property mismatch: IdentifyingDescriptions" <<
            endl;
        return 1;
      }
    }

    if (p.getAdditionalAvailability(ai16a))
    {
      if (processorTestVerbose) cout << "    AdditionalAvailability" << endl;
      inst.getProperty(
          inst.findProperty("AdditionalAvailability")).getValue().get(ai16b);
      if (ai16a != ai16b)
      {
        cout << "+++++ Error: property mismatch: AdditionalAvailability" <<
            endl;
        return 1;
      }
    }

    if (p.getMaxQuiesceTime(i64a))
    {
      if (processorTestVerbose) cout << "    MaxQuiesceTime" << endl;
      inst.getProperty(
          inst.findProperty("MaxQuiesceTime")).getValue().get(i64b);
      if (i64a != i64b)
      {
        cout << "+++++ Error: property mismatch: MaxQuiesceTime" << endl;
        return 1;
      }
    }

  // ===================== CIM_Processor =========================

    if (p.getRole(sa))
    {
      if (processorTestVerbose) cout << "    Role" << endl;
      inst.getProperty(inst.findProperty("Role")).getValue().get(sb);
      if (sa != sb)
      {
        cout << "+++++ Error: property mismatch: Role" << endl;
        return 1;
      }
    }

    if (p.getFamily(i16a))
    {
      if (processorTestVerbose) cout << "    Family" << endl;
      inst.getProperty(inst.findProperty("Family")).getValue().get(i16b);
      if (i16a != i16b)
      {
        cout << "+++++ Error: property mismatch: Family" << endl;
        return 1;
      }
    }

    if (p.getOtherFamilyDescription(sa))
    {
      if (processorTestVerbose) cout << "    OtherFamilyDescription" << endl;
      inst.getProperty(
          inst.findProperty("OtherFamilyDescription")).getValue().get(sb);
      if (sa != sb)
      {
        cout << "+++++ Error: property mismatch: OtherFamilyDescription" <<
            endl;
        return 1;
      }
    }

    if (p.getUpgradeMethod(i16a))
    {
      if (processorTestVerbose) cout << "    UpgradeMethod" << endl;
      inst.getProperty(inst.findProperty("UpgradeMethod")).getValue().get(i16b);
      if (i16a != i16b)
      {
        cout << "+++++ Error: property mismatch: UpgradeMethod" << endl;
        return 1;
      }
    }

    if (p.getMaxClockSpeed(i32a))
    {
      if (processorTestVerbose) cout << "    MaxClockSpeed" << endl;
      inst.getProperty(inst.findProperty("MaxClockSpeed")).getValue().get(i32b);
      if (i32a != i32b)
      {
        cout << "+++++ Error: property mismatch: MaxClockSpeed" << endl;
        return 1;
      }
    }

    if (p.getCurrentClockSpeed(i32a))
    {
      if (processorTestVerbose) cout << "    CurrentClockSpeed" << endl;
      inst.getProperty(
          inst.findProperty("CurrentClockSpeed")).getValue().get(i32b);
      if (i32a != i32b)
      {
        cout << "+++++ Error: property mismatch: CurrentClockSpeed" << endl;
        return 1;
      }
    }

    if (p.getDataWidth(i16a))
    {
      if (processorTestVerbose) cout << "    DataWidth" << endl;
      inst.getProperty(inst.findProperty("DataWidth")).getValue().get(i16b);
      if (i16a != i16b)
      {
        cout << "+++++ Error: property mismatch: DataWidth" << endl;
        return 1;
      }
    }

    if (p.getAddressWidth(i16a))
    {
      if (processorTestVerbose) cout << "    AddressWidth" << endl;
      inst.getProperty(inst.findProperty("AddressWidth")).getValue().get(i16b);
      if (i16a != i16b)
      {
        cout << "+++++ Error: property mismatch: AddressWidth" << endl;
        return 1;
      }
    }

    if (p.getLoadPercentage(i16a))
    {
      if (processorTestVerbose) cout << "    LoadPercentage" << endl;
      inst.getProperty(
          inst.findProperty("LoadPercentage")).getValue().get(i16b);
      if (i16a != i16b)
      {
        cout << "+++++ Error: property mismatch: LoadPercentage" << endl;
        cout << "LoadPercentage expected = " << i16a << ", received = " <<
            i16b << endl;
        return 1;
      }
    }

    if (p.getStepping(sa))
    {
      if (processorTestVerbose) cout << "    Stepping" << endl;
      inst.getProperty(inst.findProperty("Stepping")).getValue().get(sb);
      if (sa != sb)
      {
        cout << "+++++ Error: property mismatch: Stepping" << endl;
        return 1;
      }
    }

    if (p.getUniqueID(sa))
    {
      if (processorTestVerbose) cout << "    UniqueID" << endl;
      inst.getProperty(inst.findProperty("UniqueID")).getValue().get(sb);
      if (sa != sb)
      {
        cout << "+++++ Error: property mismatch: UniqueID" << endl;
        return 1;
      }
    }

    if (p.getCPUStatus(i16a))
    {
      if (processorTestVerbose) cout << "    CPUStatus" << endl;
      inst.getProperty(inst.findProperty("CPUStatus")).getValue().get(i16b);
      if (i16a != i16b)
      {
        cout << "+++++ Error: property mismatch: CPUStatus" << endl;
        return 1;
      }
    }

  // ===================== PG_Processor =========================

    if (p.getBiosID(sa))
    {
      if (processorTestVerbose) cout << "    BiosID" << endl;
      inst.getProperty(inst.findProperty("BiosID")).getValue().get(sb);
      if (sa != sb)
      {
        cout << "+++++ Error: property mismatch: BiosID" << endl;
        return 1;
      }
    }

    if (p.getFirmwareID(sa))
    {
      if (processorTestVerbose) cout << "    FirmwareID" << endl;
      inst.getProperty(inst.findProperty("FirmwareID")).getValue().get(sb);
      if (sa != sb)
      {
        cout << "+++++ Error: property mismatch: FirmwareID" << endl;
        return 1;
      }
    }

    cout << "+++++ property values ok" << endl;
  }

  else
  {
    cout << "+++++ Error: class " << className << " not recognized" << endl;
    return 1;
  }

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

int main(int argc, char *argv[])
{
  cout << "+++++ Testing Processor Provider" << endl;

  processorTestVerbose = getenv("PEGASUS_TEST_VERBOSE");

  if ((argc == 2) && String::equalNoCase(argv[1], "verbose"))
  {
    processorTestVerbose = true;
  }

  // Connect
  try
  {
    c.connectLocal();
  }
  catch (Exception& e)
  {
    errorExit(e);
  }

  int rc;
  if ((rc = testClass(CLASS_CIM_PROCESSOR)) != 0) return rc;
  if ((rc = testClass(CLASS_PROCESSOR)) != 0) return rc;
  cout << "+++++ passed all tests" << endl;
  return 0;
}
