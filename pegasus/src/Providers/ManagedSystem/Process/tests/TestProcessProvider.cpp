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
#include "TestProcessProvider.h"
#include "../ProcessPlatform.h"

// ==========================================================================
// Miscellaneous defines
// ==========================================================================

#define NAMESPACE "root/cimv2"
#define TIMEOUT   60000                    // timeout value in milliseconds

// ==========================================================================
// Class names.  These values are the names of the classes that
// are common for all of the providers.
// ==========================================================================

#define CLASS_CIM_UNITARY_COMPUTER_SYSTEM  "CIM_UnitaryComputerSystem"
#define CLASS_CIM_OPERATING_SYSTEM         "CIM_OperatingSystem"
#define CLASS_CIM_PROCESS                  "CIM_Process"

// Use PG_UnixProcess until DMTF finalizes CIM_UnixProcess
#define CLASS_UNIX_PROCESS                 "PG_UnixProcess"
#define CLASS_UNIX_PROCESS_STAT          "PG_UnixProcessStatisticalInformation"

// ==========================================================================
// The number of keys for the classes.
// ==========================================================================

#define NUMKEYS_UNIX_PROCESS                 6
#define NUMKEYS_UNIX_PROCESS_STAT            7

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

// Properties in UnixProcessStatisticalInformation

#define PROPERTY_CPU_TIME                    "CPUTime"
#define PROPERTY_REAL_TEXT                   "RealText"
#define PROPERTY_REAL_DATA                   "RealData"
#define PROPERTY_REAL_STACK                  "RealStack"
#define PROPERTY_VIRTUAL_TEXT                "VirtualText"
#define PROPERTY_VIRTUAL_DATA                "VirtualData"
#define PROPERTY_VIRTUAL_STACK               "VirtualStack"
#define PROPERTY_VIRTUAL_MEMORY_MAPPED_FILESIZE  "VirtualMemoryMappedFileSize"
#define PROPERTY_VIRTUAL_SHARED_MEMORY       "VirtualSharedMemory"
#define PROPERTY_CPU_TIME_DEAD_CHILDREN      "CpuTimeDeadChildren"
#define PROPERTY_SYSTEM_TIME_DEAD_CHILDREN   "SystemTimeDeadChildren"
#define PROPERTY_REAL_SPACE                  "RealSpace"


PEGASUS_USING_STD;
PEGASUS_USING_PEGASUS;

static CIMClient c;
static Boolean processTestVerbose = false;

void errorExit(Exception& e)
{
  cout << "Error: Failed" << endl << e.getMessage() << endl;
  exit(1);
}

int testClass(const String& className, const int& attempt)
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

  // There must be at least 5 processes or something's wrong
  if (refs.size() < 5)
  {
    cout << "+++++ Error: too few instances returned" << endl;
    return 1;
  }

  // =======================================================================
  // getInstance
  // =======================================================================

  // -------------------- First do normal getInstance() --------------------

  // pick the middle instance of the bunch in the first attempt, or 
  // 1/4th or 1/8th of the list in second and third attempts respectively.
  
  Uint32 i = (refs.size()-1) 
      >> (attempt + 1);  // This is a shift right, not streamio!
  
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

  if (processTestVerbose)
  {
    // Display keys
    Array<CIMKeyBinding> keys = ref.getKeyBindings();
    cout << "  Keys:" << endl;
    for (i=0; i<keys.size(); i++)
      cout << "    " << keys[i].getName().getString() << " = " <<
          keys[i].getValue() << endl;
  }

  // check returned property values

  // first get the PID and load a process object
  String handle;
  inst.getProperty(inst.findProperty("handle")).getValue().get(handle);
  Process p;
  // error if can't get the process
  if (!p.findProcess(handle))
  {
    cout << "+++++ Error: can't find process corresponding to instance" << endl;
    return 1;
  }

  if (processTestVerbose) cout << "  Properties:" << endl;

  String sa, sb;
  Array<String> asa, asb;
  Uint16 i16a, i16b;
  Uint32 i32a, i32b;
  Uint64 i64a, i64b;
  CIMDateTime da, db;

  // For each property, get it from the just-loaded process
  // object and compare with what was returned by getInstance()

  // Caption and Description are common to all classes
  if (p.getCaption(sa))
  {
    if (processTestVerbose) cout << "    Caption" << endl;
    inst.getProperty(inst.findProperty("Caption")).getValue().get(sb);
    if (sa != sb)
    {
      cout << "+++++ Error: property mismatch: Caption" << endl;
      return 1;
    }
  }

  if (p.getDescription(sa))
  {
    if (processTestVerbose) cout << "    Description" << endl;
    inst.getProperty(inst.findProperty("Description")).getValue().get(sb);
    if (sa != sb)
    {
      cout << "+++++ Error: property mismatch: Description" << endl;
      return 1;
    }
  }

  // The rest of the properties to check depend on the
  // class we are testing

  // ===================== UnixProcess instances =========================

  if (String::equalNoCase(className,"CIM_Process") ||
      String::equalNoCase(className,"PG_UnixProcess"))
  {
    if (p.getInstallDate(da))
    {
      if (processTestVerbose) cout << "    InstallDate" << endl;
      inst.getProperty(inst.findProperty("InstallDate")).getValue().get(db);
      if (da != db)
      {
        cout << "+++++ Error: property mismatch: InstallDate" << endl;
        return 1;
      }
    }

    if (p.getStatus(sa))
    {
      if (processTestVerbose) cout << "    Status" << endl;
      inst.getProperty(inst.findProperty("Status")).getValue().get(sb);
      if (sa != sb)
      {
        cout << "+++++ Error: property mismatch: Status" << endl;
        return 1;
      }
    }

    if (p.getName(sa))
    {
      if (processTestVerbose) cout << "    Name" << endl;
      inst.getProperty(inst.findProperty("Name")).getValue().get(sb);
      if (sa != sb)
      {
        cout << "+++++ Error: property mismatch: Name" << endl;
        return 1;
      }
    }

    if (p.getPriority(i32a))
    {
      if (processTestVerbose) cout << "    Priority" << endl;
      inst.getProperty(inst.findProperty("Priority")).getValue().get(i32b);
#ifdef PEGASUS_OS_HPUX
      // Empirical evidence has shown that the priority of a process may
      // change by as much as 24 between the times it is retrieved by the
      // provider and by the test client.
      if (abs((int)i32a - (int)i32b) > 24)
#elif defined (PEGASUS_OS_LINUX)
      // On RHEL4 U2 systems, priority may change by 1.
      if (abs((int)i32a - (int)i32b) > 1)
#else
      if (i32a != i32b)
#endif
      {
        cout << "+++++ Error: property mismatch: Priority" << endl;
        cout << "Process handle = " << handle << endl;
        if (p.getDescription(sa))
        {
            cout << "Process description = " << sa << endl;
        }
        cout << "Priority expected = " << i32a << ", received = " << i32b <<
            endl;
        return 1;
      }
    }

    if (p.getExecutionState(i16a))
    {
      if (processTestVerbose) cout << "    ExecutionState" << endl;
      inst.getProperty(
          inst.findProperty("ExecutionState")).getValue().get(i16b);
      if (i16a != i16b)
      {
        cout << "+++++ Error: property mismatch: ExecutionState" << endl;
        return 1;
      }
    }

    if (p.getOtherExecutionDescription(sa))
    {
      if (processTestVerbose) cout << "    OtherExecutionDescription" << endl;
      CIMValue oedVal = inst.getProperty
          (inst.findProperty("OtherExecutionDescription")).getValue();
      if (!oedVal.isNull())
          oedVal.get(sb);
      else
         sb = String::EMPTY;
      if (sa != sb)
      {
        cout << "+++++ Error: property mismatch: OtherExecutionDescription" <<
            endl;
        return 1;
      }
    }

    if (p.getCreationDate(da))
    {
      if (processTestVerbose) cout << "    CreationDate" << endl;
      inst.getProperty(inst.findProperty("CreationDate")).getValue().get(db);
      if (da != db)
      {
        cout << "+++++ Error: property mismatch: CreationDate" << endl;
        return 1;
      }
    }

    if (p.getTerminationDate(da))
    {
      if (processTestVerbose) cout << "    TerminationDate" << endl;
      inst.getProperty(inst.findProperty("TerminationDate")).getValue().get(db);
      if (da != db)
      {
        cout << "+++++ Error: property mismatch: TerminationDate" << endl;
        return 1;
      }
    }

    if (p.getKernelModeTime(i64a))
    {
      if (processTestVerbose) cout << "    KernelModeTime" << endl;
      inst.getProperty(
          inst.findProperty("KernelModeTime")).getValue().get(i64b);
      if (i64a != i64b)
      {
        cout << "+++++ Error: property mismatch: KernelModeTime" << endl;
        return 1;
      }
    }

    if (p.getUserModeTime(i64a))
    {
      if (processTestVerbose) cout << "    UserModeTime" << endl;
      inst.getProperty(inst.findProperty("UserModeTime")).getValue().get(i64b);
      if (i64a != i64b)
      {
        cout << "+++++ Error: property mismatch: UserModeTime" << endl;
        return 1;
      }
    }

    if (p.getWorkingSetSize(i64a))
    {
      if (processTestVerbose) cout << "    WorkingSetSize" << endl;
      inst.getProperty(
          inst.findProperty("WorkingSetSize")).getValue().get(i64b);
      if (i64a != i64b)
      {
        cout << "+++++ Error: property mismatch: WorkingSetSize" << endl;
        return 1;
      }
    }

    if (p.getParentProcessID(sa))
    {
      if (processTestVerbose) cout << "    ParentProcessID" << endl;
      inst.getProperty(inst.findProperty("ParentProcessID")).getValue().get(sb);
      if (sa != sb)
      {
        cout << "+++++ Error: property mismatch: ParentProcessID" << endl;
        return 1;
      }
    }

    if (p.getRealUserID(i64a))
    {
      if (processTestVerbose) cout << "    RealUserID" << endl;
      inst.getProperty(inst.findProperty("RealUserID")).getValue().get(i64b);
      if (i64a != i64b)
      {
        cout << "+++++ Error: property mismatch: RealUserID" << endl;
        return 1;
      }
    }

    if (p.getProcessGroupID(i64a))
    {
      if (processTestVerbose) cout << "    ProcessGroupID" << endl;
      inst.getProperty(
          inst.findProperty("ProcessGroupID")).getValue().get(i64b);
      if (i64a != i64b)
      {
        cout << "+++++ Error: property mismatch: ProcessGroupID" << endl;
        return 1;
      }
    }

    if (p.getProcessSessionID(i64a))
    {
      if (processTestVerbose) cout << "    ProcessSessionID" << endl;
      inst.getProperty(
          inst.findProperty("ProcessSessionID")).getValue().get(i64b);
      if (i64a != i64b)
      {
        cout << "+++++ Error: property mismatch: ProcessSessionID" << endl;
        return 1;
      }
    }

    if (p.getProcessTTY(sa))
    {
      if (processTestVerbose) cout << "    ProcessTTY" << endl;
      inst.getProperty(inst.findProperty("ProcessTTY")).getValue().get(sb);
      if (sa != sb)
      {
        cout << "+++++ Error: property mismatch: ProcessTTY" << endl;
        return 1;
      }
    }

    if (p.getModulePath(sa))
    {
      if (processTestVerbose) cout << "    ModulePath" << endl;
      inst.getProperty(inst.findProperty("ModulePath")).getValue().get(sb);
      if (sa != sb)
      {
        cout << "+++++ Error: property mismatch: ModulePath" << endl;
        return 1;
      }
    }

    if (p.getParameters(asa))
    {
      if (processTestVerbose) cout << "    Parameters" << endl;
      inst.getProperty(inst.findProperty("Parameters")).getValue().get(asb);
      if (asa != asb)
      {
        cout << "+++++ Error: property mismatch: Parameters" << endl;
        return 1;
      }
    }

    if (p.getProcessNiceValue(i32a))
    {
      if (processTestVerbose) cout << "    ProcessNiceValue" << endl;
      inst.getProperty(
          inst.findProperty("ProcessNiceValue")).getValue().get(i32b);
      if (i32a != i32b)
      {
        cout << "+++++ Error: property mismatch: ProcessNiceValue" << endl;
        return 1;
      }
    }

    if (p.getProcessWaitingForEvent(sa))
    {
      if (processTestVerbose) cout << "    ProcessWaitingForEvent" << endl;
      inst.getProperty(
          inst.findProperty("ProcessWaitingForEvent")).getValue().get(sb);
      if (sa != sb)
      {
        cout << "+++++ Error: property mismatch: ProcessWaitingForEvent" <<
            endl;
        return 1;
      }
    }
    cout << "+++++ property values ok" << endl;
  }

  // ========== UnixProcessStatisticalInformation instances ===============

  else if (String::equalNoCase(
               className,"PG_UnixProcessStatisticalInformation"))
  {
    if (p.getCPUTime(i32a))
    {
      if (processTestVerbose) cout << "    CPUTime" << endl;
      inst.getProperty(inst.findProperty("CPUTime")).getValue().get(i32b);
      if (i32a != i32b)
      {
        cout << "+++++ Error: property mismatch: CPUTime" << endl;
        cout << "Process handle = " << handle << endl;
        if (p.getDescription(sa))
        {
            cout << "Process description = " << sa << endl;
        }
        cout << "CPUTime expected = " << i32a << ", received = " << i32b <<
            endl;
        return 1;
      }
    }

    if (p.getRealText(i64a))
    {
      if (processTestVerbose) cout << "    RealText" << endl;
      inst.getProperty(inst.findProperty("RealText")).getValue().get(i64b);
      if (i64a != i64b)
      {
        cout << "+++++ Error: property mismatch: RealText" << endl;
        return 1;
      }
    }

    if (p.getRealData(i64a))
    {
      if (processTestVerbose) cout << "    RealData" << endl;
      inst.getProperty(inst.findProperty("RealData")).getValue().get(i64b);
      if (i64a != i64b)
      {
        cout << "+++++ Error: property mismatch: RealData" << endl;
        return 1;
      }
    }

    if (p.getRealStack(i64a))
    {
      if (processTestVerbose) cout << "    RealStack" << endl;
      inst.getProperty(inst.findProperty("RealStack")).getValue().get(i64b);
      if (i64a != i64b)
      {
        cout << "+++++ Error: property mismatch: RealStack" << endl;
        return 1;
      }
    }

    if (p.getVirtualText(i64a))
    {
      if (processTestVerbose) cout << "    VirtualText" << endl;
      inst.getProperty(inst.findProperty("VirtualText")).getValue().get(i64b);
      if (i64a != i64b)
      {
        cout << "+++++ Error: property mismatch: VirtualText" << endl;
        return 1;
      }
    }

    if (p.getVirtualData(i64a))
    {
      if (processTestVerbose) cout << "    VirtualData" << endl;
      inst.getProperty(inst.findProperty("VirtualData")).getValue().get(i64b);
      if (i64a != i64b)
      {
        cout << "+++++ Error: property mismatch: VirtualData" << endl;
        return 1;
      }
    }

    if (p.getVirtualStack(i64a))
    {
      if (processTestVerbose) cout << "    VirtualStack" << endl;
      inst.getProperty(inst.findProperty("VirtualStack")).getValue().get(i64b);
      if (i64a != i64b)
      {
        cout << "+++++ Error: property mismatch: VirtualStack" << endl;
        return 1;
      }
    }

    if (p.getVirtualMemoryMappedFileSize(i64a))
    {
      if (processTestVerbose) cout << "    VirtualMemoryMappedFileSize" << endl;
      inst.getProperty(inst.findProperty(
          "VirtualMemoryMappedFileSize")).getValue().get(i64b);
      if (i64a != i64b)
      {
        cout << "+++++ Error: property mismatch: VirtualMemoryMappedFileSize"
             << endl;
        return 1;
      }
    }

    if (p.getVirtualSharedMemory(i64a))
    {
      if (processTestVerbose) cout << "    VirtualSharedMemory" << endl;
      inst.getProperty(
          inst.findProperty("VirtualSharedMemory")).getValue().get(i64b);
      if (i64a != i64b)
      {
        cout << "+++++ Error: property mismatch: VirtualSharedMemory" << endl;
        return 1;
      }
    }

    if (p.getCpuTimeDeadChildren(i64a))
    {
      if (processTestVerbose) cout << "    CpuTimeDeadChildren" << endl;
      inst.getProperty(
          inst.findProperty("CpuTimeDeadChildren")).getValue().get(i64b);
      if (i64a != i64b)
      {
        cout << "+++++ Error: property mismatch: CpuTimeDeadChildren" << endl;
        return 1;
      }
    }

    if (p.getSystemTimeDeadChildren(i64a))
    {
      if (processTestVerbose) cout << "    SystemTimeDeadChildren" << endl;
      inst.getProperty(
          inst.findProperty("SystemTimeDeadChildren")).getValue().get(i64b);
      if (i64a != i64b)
      {
        cout << "+++++ Error: property mismatch: SystemTimeDeadChildren" <<
            endl;
        return 1;
      }
    }

    if (p.getRealSpace(i64a))
    {
      if (processTestVerbose) cout << "    RealSpace" << endl;
      inst.getProperty(inst.findProperty("RealSpace")).getValue().get(i64b);
      if (i64a != i64b)
      {
        cout << "+++++ Error: property mismatch: RealSpace" << endl;
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
  if (ia.size() < 5)
  {
    cout << "+++++ Error: enumerateInstances on " << className <<
        " returned too few instances" << endl;
    return 1;
  }

#ifndef PEGASUS_TEST_VALGRIND
  // For UnixProcess, we should be able to find this test process
  // and the cimserver
  if (String::equalNoCase(className,"CIM_Process") ||
      String::equalNoCase(className,"PG_UnixProcess"))
  {
    status = 0;
    for (i=0; i<ia.size(); i++)
    {
      Array<String> cmd;
      ia[i].getProperty(ia[i].findProperty("Parameters")).getValue().get(cmd);
      if (cmd[0] == "cimserver")
      {
        status++;
        if (processTestVerbose) cout << "    cimserver" << endl;
      }
      if (cmd[0] == "TestClientProcessProvider")
      {
        status++;
        if (processTestVerbose) cout << "    ProcessProviderTestClient" << endl;
      }
    }
    if (status < 2)
    {
      cout << "Error: couldn't find cimserver or test client process!" << endl;
      return 1;
    }
  }
#endif

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

int testClass(const String& className) 
{

    // The approach of this test assumes that the process attributes 
    // gathered by the provider remain the same when the test client gathers
    // the same data a little later. 
    // It's possible that the data could be different depending on the process
    // that get's picked. 
    // So, repeat this with a different process if the checks for a process
    // failed. This change isn't going to eliminate this possibility,
    // but makes it less likely. This is probably better than not 
    // verifying those properties at all.

    int attempt = 0;
    while (attempt < 3)
    {
        if (testClass(className, attempt) == 0)
        {
            return 0;
        }
        cout << "+++++ Process attributes changed, trying again" << endl;
        ++attempt;
    }
    return 1;
}


int main()
{
  cout << "+++++ Testing ProcessProviders" << endl;

  // Connect
  try
  {
    c.connectLocal ();
  }
  catch (Exception& e)
  {
    errorExit(e);
  }

  //
  //  Set a timeout longer than the default to avoid operations such as
  //  enumerateInstances timing out on slower systems
  //
  c.setTimeout (TIMEOUT);

  int rc;
  if ((rc = testClass(CLASS_CIM_PROCESS)) != 0) return rc;
  if ((rc = testClass(CLASS_UNIX_PROCESS)) != 0) return rc;
  if ((rc = testClass(CLASS_UNIX_PROCESS_STAT)) != 0) return rc;
  cout << "+++++ passed all tests" << endl;
  return 0;
}
