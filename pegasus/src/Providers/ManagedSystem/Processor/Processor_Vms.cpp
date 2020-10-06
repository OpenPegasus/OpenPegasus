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

// ==========================================================================
// Includes.
// ==========================================================================

#include "ProcessorPlatform.h"

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
DESCRIPTION       :
ASSUMPTIONS       :
PRE-CONDITIONS    :
POST-CONDITIONS   :
NOTES             :
================================================================================
*/
Boolean Processor::getCaption(String& s) const
{
  s = String::EMPTY;
  return true;
}

/*
================================================================================
NAME              : getDescription
DESCRIPTION       :
ASSUMPTIONS       :
PRE-CONDITIONS    :
POST-CONDITIONS   :
NOTES             :
================================================================================
*/
Boolean Processor::getDescription(String& s) const
{
  s = String::EMPTY;
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
  // not supported
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
  // not supported
  return false;
}


// =============================================================================
// NAME              : getDeviceID
// DESCRIPTION       : Call uname() and get the operating system name.
// ASSUMPTIONS       : None
// PRE-CONDITIONS    :
// POST-CONDITIONS   :
// NOTES             :
// =============================================================================

String Processor::getDeviceID(void) const
{
  // This routine *must* return a non-empty value if even a
  // dummy provider will be tested
  return String::EMPTY;
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
  // not yet implemented
  return false;
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
  // not yet implemented
  return false;
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
  // not yet implemented
  return false;
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
  // not yet implemented
  return false;
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
  // not yet implemented
  return false;
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
  // not yet implemented
  return false;
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
  // not yet implemented
  return false;
}


// =============================================================================
// NAME              : getSysName
// DESCRIPTION       : Platform-specific method to get CSName
// ASSUMPTIONS       : None
// PRE-CONDITIONS    :
// POST-CONDITIONS   :
// NOTES             :
// =============================================================================

String Processor::getSysName(void)
{
  // This routine must be written to return this key value
  return String::EMPTY;
}


// =============================================================================
// NAME              : loadProcessorInfo
// DESCRIPTION       : get processor info from system into internal data struct
// ASSUMPTIONS       : None
// PRE-CONDITIONS    :
// POST-CONDITIONS   :
// NOTES             :
// =============================================================================

Boolean Processor::loadProcessorInfo(int& pIndex)
{
  // not yet implemented
  return false;
}

/*
================================================================================
NAME              : findProcessor
DESCRIPTION       : find the requested processor and load its data
ASSUMPTIONS       : None
PRE-CONDITIONS    :
POST-CONDITIONS   :
NOTES             :
================================================================================
*/
Boolean Processor::findProcessor(const String& deviceID)
{
  // not yet implemented
  return false;
}
