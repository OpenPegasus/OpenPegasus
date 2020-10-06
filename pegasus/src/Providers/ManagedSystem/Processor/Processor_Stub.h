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
// Author: Christopher Neufeld <neufeld@linuxcare.com>
//         David Kennedy       <dkennedy@linuxcare.com>
//
// Modified By:
//         David Kennedy       <dkennedy@linuxcare.com>
//         Christopher Neufeld <neufeld@linuxcare.com>
//         Al Stone, Hewlett-Packard Company <ahs3@fc.hp.com>
//         Jim Metcalfe, Hewlett-Packard Company
//         Carlos Bonilla, Hewlett-Packard Company
//         Mike Glantz, Hewlett-Packard Company <michael_glantz@hp.com>
//
//%////////////////////////////////////////////////////////////////////////////

#ifndef PG_PROCESSOR_STUB_H
#define PG_PROCESSOR_STUB_H

/* ==========================================================================
   Includes.
   ========================================================================== */
#include <Pegasus/Common/Config.h>
#include <Pegasus/Common/String.h>
#include <Pegasus/Common/CIMDateTime.h>

PEGASUS_USING_STD;
PEGASUS_USING_PEGASUS;


/* ==========================================================================
   Type Definitions.
   ========================================================================== */

class Processor
{

protected:

public:

  Processor();
  ~Processor();

  // CIM_ManagedElement
  Boolean getCaption(String&) const;
  Boolean getDescription(String&) const;

  // CIM_ManagedSystemElement
  Boolean getInstallDate(CIMDateTime&) const;
  Boolean getName(String&) const;
  Boolean getStatus(String&) const;

  // CIM_LogicalElement

  // CIM_LogicalDevice
  String getDeviceID(void) const;
  Boolean getPowerManagementSupported(Boolean&) const;
  Boolean getPowerManagementCapabilities(Array<Uint16>& ui16a) const;
  Boolean getAvailability(Uint16&) const;
  Boolean getStatusInfo(Uint16&) const;
  Boolean getLastErrorCode(Uint32&) const;
  Boolean getErrorDescription(String&) const;
  Boolean getErrorCleared(Boolean&) const;
  Boolean getOtherIdentifyingInfo(Array<String>&) const;
  Boolean getPowerOnHours(Uint64&) const;
  Boolean getTotalPowerOnHours(Uint64&) const;
  Boolean getIdentifyingDescriptions(Array<String>&) const;
  Boolean getAdditionalAvailability(Array<Uint16>&) const;
  Boolean getMaxQuiesceTime(Uint64&) const;
  // Methods
#if 0
  Boolean execSetPowerState(&) const;
  Boolean execReset(&) const;
  Boolean execEnableDevice(&) const;
  Boolean execOnlineDevice(&) const;
  Boolean execQuiesceDevice(&) const;
  Boolean execSaveProperties(&) const;
  Boolean execRestoreProperties(&) const;
#endif

  // CIM_Processor
  Boolean getRole(String&) const;
  Boolean getFamily(Uint16&) const;
  Boolean getOtherFamilyDescription(String&) const;
  Boolean getUpgradeMethod(Uint16&) const;
  Boolean getMaxClockSpeed(Uint32&) const;
  Boolean getCurrentClockSpeed(Uint32&) const;
  Boolean getDataWidth(Uint16&) const;
  Boolean getAddressWidth(Uint16&) const;
  Boolean getLoadPercentage(Uint16&) const;
  Boolean getStepping(String&) const;
  Boolean getUniqueID(String&) const;
  Boolean getCPUStatus(Uint16&) const;

  // PG_Processor
  Boolean getBiosID(String&) const;
  Boolean getFirmwareID(String&) const;

  // Auxiliary routines

  // Loads the internal processor structure with
  // the status data for the indexed processor and, if
  // necessary (on HP-UX) updates pIndex to skip unused
  // entries so that a simple increment will allow next
  // call to access next entry
  Boolean loadProcessorInfo(int &pIndex);

  // Finds the requested process and loads its info into
  // the internal process status structure
  Boolean findProcessor(const String& deviceID);

  static String getSysName(void);
};


#endif  /* #ifndef PG_PROCESSOR_STUB_H */
