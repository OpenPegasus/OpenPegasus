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

/* ==========================================================================
   Includes.
   ========================================================================== */

#include "ProcessorPlatform.h"
#include <sys/utsname.h>               // for uname()
#include <unistd.h>                    // for sysconf()
#ifndef CPU_IA64_ARCHREV_0
#define CPU_IA64_ARCHREV_0 0x300
#endif

PEGASUS_USING_STD;
PEGASUS_USING_PEGASUS;

Processor::Processor()
{
}

Processor::~Processor()
{
}

/*
================================================================================
NAME              : getCaption
DESCRIPTION       : returns ATTN
ASSUMPTIONS       :
PRE-CONDITIONS    :
POST-CONDITIONS   :
NOTES             :
================================================================================
*/
Boolean Processor::getCaption(String& s) const
{
  char t[32];
  sprintf(t, "Processor %llu", Uint64(pInfo.psp_idx));
  s = String(t);
  return true;
}

/*
================================================================================
NAME              : getDescription
DESCRIPTION       : returns pst_cmd (command line) as a string (contrast
                  : this with ModulePath and Parameters below)
ASSUMPTIONS       :
PRE-CONDITIONS    :
POST-CONDITIONS   :
NOTES             :
================================================================================
*/
Boolean Processor::getDescription(String& s) const
{
  s = "This is an instance of a PG_Processor";
  return true;
}

/*
================================================================================
NAME              : getInstallDate
DESCRIPTION       :
ASSUMPTIONS       :
PRE-CONDITIONS    :
POST-CONDITIONS   :
NOTES             :
================================================================================
*/
Boolean Processor::getInstallDate(CIMDateTime& d) const
{
  // Not supported. This property is inherited from
  // CIM_ManagedSystemElement, but has no useful meaning
  // for a transient entity such as a process (there is a
  // suitable property CreationDate below
  return false;
}

/*
================================================================================
NAME              : getName
DESCRIPTION       :
ASSUMPTIONS       :
PRE-CONDITIONS    :
POST-CONDITIONS   :
NOTES             :
================================================================================
*/
Boolean Processor::getName(String& s) const
{
  // Not supported. This property is inherited from
  // CIM_ManagedSystemElement, but has no useful meaning
  // for a transient entity such as a process (there is a
  // suitable property CreationDate below
  return false;
}

/*
================================================================================
NAME              : getStatus
DESCRIPTION       :
ASSUMPTIONS       :
PRE-CONDITIONS    :
POST-CONDITIONS   :
NOTES             :
================================================================================
*/
Boolean Processor::getStatus(String& s) const
{
  // This property is inherited from CIM_ManagedSystemElement
  // We can't do very much, so if we're alive enough to execute
  // this code, we return OK
  s = "OK";
  return true;
}

/*
================================================================================
NAME              : getDeviceID
DESCRIPTION       :
ASSUMPTIONS       : None
PRE-CONDITIONS    :
POST-CONDITIONS   :
NOTES             :
================================================================================
*/
String Processor::getDeviceID(void) const
{
  char buf[22];
// ATTN for now, we are returning the index
  sprintf(buf, "%llu", Uint64(pInfo.psp_idx));
  return String(buf);
}

// =============================================================================
// NAME              : getPowerManagementSupported
// DESCRIPTION       :
// ASSUMPTIONS       : None
// PRE-CONDITIONS    :
// POST-CONDITIONS   :
// NOTES             :
// =============================================================================

Boolean Processor::getPowerManagementSupported(Boolean& b) const
{
  // not yet implemented
  return false;
}

// =============================================================================
// NAME              : getPowerManagementCapabilities
// DESCRIPTION       :
// ASSUMPTIONS       : None
// PRE-CONDITIONS    :
// POST-CONDITIONS   :
// NOTES             :
// =============================================================================

Boolean Processor::getPowerManagementCapabilities(Array<Uint16>& i) const
{
  // not yet implemented
  return false;
}

// =============================================================================
// NAME              : getAvailability
// DESCRIPTION       :
// ASSUMPTIONS       : None
// PRE-CONDITIONS    :
// POST-CONDITIONS   :
// NOTES             :
// =============================================================================

Boolean Processor::getAvailability(Uint16& i) const
{
  // not yet implemented
  return false;
}

// =============================================================================
// NAME              : getStatusInfo
// DESCRIPTION       :
// ASSUMPTIONS       : None
// PRE-CONDITIONS    :
// POST-CONDITIONS   :
// NOTES             :
// =============================================================================

Boolean Processor::getStatusInfo(Uint16& i) const
{
  // not yet implemented
  return false;
}

// =============================================================================
// NAME              : getLastErrorCode
// DESCRIPTION       :
// ASSUMPTIONS       : None
// PRE-CONDITIONS    :
// POST-CONDITIONS   :
// NOTES             :
// =============================================================================

Boolean Processor::getLastErrorCode(Uint32& i) const
{
  // not yet implemented
  return false;
}

// =============================================================================
// NAME              : getErrorDescription
// DESCRIPTION       :
// ASSUMPTIONS       : None
// PRE-CONDITIONS    :
// POST-CONDITIONS   :
// NOTES             :
// =============================================================================

Boolean Processor::getErrorDescription(String& s) const
{
  // not yet implemented
  return false;
}

// =============================================================================
// NAME              : getErrorCleared
// DESCRIPTION       :
// ASSUMPTIONS       : None
// PRE-CONDITIONS    :
// POST-CONDITIONS   :
// NOTES             :
// =============================================================================

Boolean Processor::getErrorCleared(Boolean& b) const
{
  // not yet implemented
  return false;
}

// =============================================================================
// NAME              : getOtherIdentifyingInfo
// DESCRIPTION       :
// ASSUMPTIONS       : None
// PRE-CONDITIONS    :
// POST-CONDITIONS   :
// NOTES             :
// =============================================================================

Boolean Processor::getOtherIdentifyingInfo(Array<String>& s) const
{
  // not yet implemented
  return false;
}

// =============================================================================
// NAME              : PowerOnHours
// DESCRIPTION       :
// ASSUMPTIONS       : None
// PRE-CONDITIONS    :
// POST-CONDITIONS   :
// NOTES             :
// =============================================================================

Boolean Processor::getPowerOnHours(Uint64& i) const
{
  // not yet implemented
  return false;
}

// =============================================================================
// NAME              : TotalPowerOnHours
// DESCRIPTION       :
// ASSUMPTIONS       : None
// PRE-CONDITIONS    :
// POST-CONDITIONS   :
// NOTES             :
// =============================================================================

Boolean Processor::getTotalPowerOnHours(Uint64& i) const
{
  // not yet implemented
  return false;
}

// =============================================================================
// NAME              : getIdentifyingDescriptions
// DESCRIPTION       :
// ASSUMPTIONS       : None
// PRE-CONDITIONS    :
// POST-CONDITIONS   :
// NOTES             :
// =============================================================================

Boolean Processor::getIdentifyingDescriptions(Array<String>& s) const
{
  // not yet implemented
  return false;
}

// =============================================================================
// NAME              : getAdditionalAvailability
// DESCRIPTION       :
// ASSUMPTIONS       : None
// PRE-CONDITIONS    :
// POST-CONDITIONS   :
// NOTES             :
// =============================================================================

Boolean Processor::getAdditionalAvailability(Array<Uint16>& i) const
{
  // not yet implemented
  return false;
}

// =============================================================================
// NAME              : getMaxQuiesceTime
// DESCRIPTION       :
// ASSUMPTIONS       : None
// PRE-CONDITIONS    :
// POST-CONDITIONS   :
// NOTES             :
// =============================================================================

Boolean Processor::getMaxQuiesceTime(Uint64& i) const
{
  // not yet implemented
  return false;
}

// =============================================================================
// NAME              : getRole
// DESCRIPTION       :
// ASSUMPTIONS       : None
// PRE-CONDITIONS    :
// POST-CONDITIONS   :
// NOTES             :
// =============================================================================

Boolean Processor::getRole(String& s) const
{
  // not yet implemented
  return false;
}

// =============================================================================
// NAME              : getFamily
// DESCRIPTION       :
// ASSUMPTIONS       : None
// PRE-CONDITIONS    :
// POST-CONDITIONS   :
// NOTES             :
// =============================================================================

Boolean Processor::getFamily(Uint16& i) const
{
  struct utsname u;
  if ((uname(&u) < 0) && (errno != EOVERFLOW))
  {
    return false;
  }

  // check for PA-RISC
  if ( 0 == strncmp(u.machine,"9000",4) ) i=144;  // "PA-RISC Family"
  else if ( 0 == strncmp(u.machine,"ia64",4) ) i=1; // "Other"
  else i=2; // "Unknown"

  return true;
}

// =============================================================================
// NAME              : getOtherFamilyDescription
// DESCRIPTION       :
// ASSUMPTIONS       : None
// PRE-CONDITIONS    :
// POST-CONDITIONS   :
// NOTES             :
// =============================================================================

Boolean Processor::getOtherFamilyDescription(String& s) const
{
  struct utsname u;
  if ((uname(&u) < 0) && (errno != EOVERFLOW))
  {
    return false;
  }

  if ( 0 == strncmp(u.machine,"ia64",4) )
  {
    s = "Itanium(TM) Processor";
    return true;
  }
  else return false;
}

// =============================================================================
// NAME              : getUpgradeMethod
// DESCRIPTION       :
// ASSUMPTIONS       : None
// PRE-CONDITIONS    :
// POST-CONDITIONS   :
// NOTES             :
// =============================================================================

Boolean Processor::getUpgradeMethod(Uint16& i) const
{
  // not yet implemented
  return false;
}

// =============================================================================
// NAME              : getMaxClockSpeed
// DESCRIPTION       :
// ASSUMPTIONS       : None
// PRE-CONDITIONS    :
// POST-CONDITIONS   :
// NOTES             :
// =============================================================================

Boolean Processor::getMaxClockSpeed(Uint32& i) const
{
  // not yet implemented
  return false;
}

// =============================================================================
// NAME              : getCurrentClockSpeed
// DESCRIPTION       :
// ASSUMPTIONS       : None
// PRE-CONDITIONS    :
// POST-CONDITIONS   :
// NOTES             :
// =============================================================================

Boolean Processor::getCurrentClockSpeed(Uint32& i) const
{
  i = pInfo.psp_iticksperclktick * sysconf(_SC_CLK_TCK) / 1000000;
  return true;
}

// =============================================================================
// NAME              : getDataWidth
// DESCRIPTION       :
// ASSUMPTIONS       : None
// PRE-CONDITIONS    :
// POST-CONDITIONS   :
// NOTES             :
// =============================================================================

Boolean Processor::getDataWidth(Uint16& i) const
{
  long bits = sysconf(_SC_HW_32_64_CAPABLE);
  if (_SYSTEM_SUPPORTS_LP64OS(bits)) i = 64;
  else if (_SYSTEM_SUPPORTS_ILP32OS(bits)) i = 32;
  else i = 0;
  return true;
}

// =============================================================================
// NAME              : getAddressWidth
// DESCRIPTION       :
// ASSUMPTIONS       : None
// PRE-CONDITIONS    :
// POST-CONDITIONS   :
// NOTES             :
// =============================================================================

Boolean Processor::getAddressWidth(Uint16& i) const
{
  // not yet implemented
  return false;
}

// =============================================================================
// NAME              : getLoadPercentage
// DESCRIPTION       :
// ASSUMPTIONS       : None
// PRE-CONDITIONS    :
// POST-CONDITIONS   :
// NOTES             :
// =============================================================================

Boolean Processor::getLoadPercentage(Uint16& i) const
{
  i = 100. * pInfo.psp_avg_1_min;
  return true;
}

// =============================================================================
// NAME              : getStepping
// DESCRIPTION       :
// ASSUMPTIONS       : None
// PRE-CONDITIONS    :
// POST-CONDITIONS   :
// NOTES             :
// =============================================================================

Boolean Processor::getStepping(String& s) const
{
  // not yet implemented
  return false;
}

// =============================================================================
// NAME              : getUniqueID
// DESCRIPTION       :
// ASSUMPTIONS       : None
// PRE-CONDITIONS    :
// POST-CONDITIONS   :
// NOTES             :
// =============================================================================

Boolean Processor::getUniqueID(String& s) const
{
  // not yet implemented
  return false;
}

// =============================================================================
// NAME              : getCPUStatus
// DESCRIPTION       :
// ASSUMPTIONS       : None
// PRE-CONDITIONS    :
// POST-CONDITIONS   :
// NOTES             :
// =============================================================================

Boolean Processor::getCPUStatus(Uint16& i) const
{
  // ATTN all we know is that if pstat_getprocessor()
  // returned an entry, the processor is active
  i = 1; // "CPU Enabled"
  return true;
}


// =============================================================================
// NAME              : getBiosID
// DESCRIPTION       :
// ASSUMPTIONS       : None
// PRE-CONDITIONS    :
// POST-CONDITIONS   :
// NOTES             :
// =============================================================================

Boolean Processor::getBiosID(String& s) const
{
  // not yet implemented
  return false;
}


// =============================================================================
// NAME              : getFirmwareID
// DESCRIPTION       :
// ASSUMPTIONS       : None
// PRE-CONDITIONS    :
// POST-CONDITIONS   :
// NOTES             :
// =============================================================================

Boolean Processor::getFirmwareID(String& s) const
{
  long cpu = sysconf(_SC_CPU_VERSION);

  if (cpu == CPU_PA_RISC1_0) s = "HP PA_RISC1.0";
  else if (cpu == CPU_PA_RISC1_1) s = "HP PA_RISC1.1";
  else if (cpu == CPU_PA_RISC1_2) s = "HP PA_RISC1.2";
  else if (cpu == CPU_PA_RISC2_0) s = "HP PA_RISC2.0";
  else if (cpu == CPU_IA64_ARCHREV_0) s = "IA64_0";
  else s = "Unknown";

  return true;
}


/*
================================================================================
NAME              : getSysName
DESCRIPTION       : Platform-specific routine to get CSName
ASSUMPTIONS       : None
PRE-CONDITIONS    :
POST-CONDITIONS   :
NOTES             :
================================================================================
*/
String Processor::getSysName(void)
{
  struct hostent *he;
  char hn[PEGASUS_MAXHOSTNAMELEN + 1];

  // fill in hn with what this system thinks is its name
  gethostname(hn, sizeof(hn));
  hn[sizeof(hn)-1] = 0;

  // find out what the nameservices think is its full name
  if ((he = gethostbyname(hn)) != 0)
  {
      return String(he->h_name);
  }
  // but if that failed, return what gethostname said
  else
  {
      return String(hn);
  }
}

/*
================================================================================
NAME              : loadProcessorInfo
DESCRIPTION       : Use pstat_getprocessor() to fill in a struct pst_processor
ASSUMPTIONS       : None
PRE-CONDITIONS    :
POST-CONDITIONS   :
NOTES             :
================================================================================
*/
Boolean Processor::loadProcessorInfo(int &pIndex)
{
  // This routine fills in the protected member pInfo by calling
  // pstat_getprocessor. Because HP-UX processor entries are not contiguous
  // this routine modified pIndex so that the caller, after
  // incrementing pIndex, will be able to fetch the next processor in
  // a subsequent call. The routine is functionally equivalent to
  // pstat_getprocessor(), except that it hides the platform-specific
  // structure pst_processor.
  // It returns true if it succeeded in fetching a processor, otherwise
  // false, incidating that there are no more processors to be fetched.

  // pstat_getprocessor() takes an empty struct to fill in, each element's size,
  // the number of elements (0 if last arg is pid instead of index),
  // and an index to start at

  int stat = pstat_getprocessor(&pInfo, sizeof(pInfo), 1, pIndex);

  // pstat_getprocessor returns number of returned structures
  // if this was not 1, it means that we are at the end
  // of the process entry table, and the caller should not
  // use data from this call
  if (stat != 1) return false;
  pIndex = pInfo.psp_idx;
  return true;
}


/*
================================================================================
NAME              : findProcessor
DESCRIPTION       : find the requested process and load its data
ASSUMPTIONS       : None
PRE-CONDITIONS    :
POST-CONDITIONS   :
NOTES             :
================================================================================
*/
Boolean Processor::findProcessor(const String& deviceID)
{
  int pIndex;
  int stat;

  // Convert id to an integer
  int idx = atoi((const char*)deviceID.getCString());

  // loop ends with stat==0, meaning number of entries
  // returned was zero
  // if this loop finishes, we haven't found the process
  for (pIndex=0, stat=1; stat!=0; pIndex++)
  {
    // pstat_getprocessor() fills in ps with a process entry's
    // data, and returns the number of entries filled in,
    // so that anything other than 1 is a problem
    stat = pstat_getprocessor(&pInfo, sizeof(pInfo), 1, pIndex);

    // we can return right now if we found it
    if (idx == pInfo.psp_idx) return true;
    pIndex = pInfo.psp_idx;
  }

  // we finished the loop without finding the process
  // signal this to the caller
  return false;
}
