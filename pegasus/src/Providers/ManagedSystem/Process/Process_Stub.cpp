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

#include "ProcessPlatform.h"

PEGASUS_USING_STD;
PEGASUS_USING_PEGASUS;


Process::Process()
{
}

Process::~Process()
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
Boolean Process::getCaption(String& s) const
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
Boolean Process::getDescription(String& s) const
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
Boolean Process::getInstallDate(CIMDateTime& d) const
{
  // not supported
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
Boolean Process::getStatus(String& s) const
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
Boolean Process::getName(String& s) const
{
  // not supported
  return false;
}

/*
================================================================================
NAME              : getPriority
DESCRIPTION       :
ASSUMPTIONS       :
PRE-CONDITIONS    :
POST-CONDITIONS   :
NOTES             :
================================================================================
*/
Boolean Process::getPriority(Uint32& i32) const
{
  // not supported
  return false;
}

/*
================================================================================
NAME              : getExecutionState
DESCRIPTION       :
ASSUMPTIONS       :
PRE-CONDITIONS    :
POST-CONDITIONS   :
NOTES             :
================================================================================
*/
Boolean Process::getExecutionState(Uint16& i16) const
{
  // not supported
  return false;
}

/*
================================================================================
NAME              : getOtherExecutionDescription
DESCRIPTION       :
ASSUMPTIONS       :
PRE-CONDITIONS    :
POST-CONDITIONS   :
NOTES             :
================================================================================
*/
Boolean Process::getOtherExecutionDescription(String& s) const
{
  // not supported
  return false;
}

/*
================================================================================
NAME              : getCreationDate
DESCRIPTION       :
ASSUMPTIONS       :
PRE-CONDITIONS    :
POST-CONDITIONS   :
NOTES             :
================================================================================
*/
Boolean Process::getCreationDate(CIMDateTime& d) const
{
  // not supported
  return false;
}

/*
================================================================================
NAME              : getTerminationDate
DESCRIPTION       :
ASSUMPTIONS       :
PRE-CONDITIONS    :
POST-CONDITIONS   :
NOTES             :
================================================================================
*/
Boolean Process::getTerminationDate(CIMDateTime& d) const
{
  // not supported
  return false;
}

/*
================================================================================
NAME              : getKernelModeTime
DESCRIPTION       :
ASSUMPTIONS       :
PRE-CONDITIONS    :
POST-CONDITIONS   :
NOTES             :
================================================================================
*/
Boolean Process::getKernelModeTime(Uint64& i64) const
{
  // not supported
  return false;
}

/*
================================================================================
NAME              : getUserModeTime
DESCRIPTION       :
ASSUMPTIONS       :
PRE-CONDITIONS    :
POST-CONDITIONS   :
NOTES             :
================================================================================
*/
Boolean Process::getUserModeTime(Uint64& i64) const
{
  // not supported
  return false;
}

/*
================================================================================
NAME              : getWorkingSetSize
DESCRIPTION       :
ASSUMPTIONS       :
PRE-CONDITIONS    :
POST-CONDITIONS   :
NOTES             :
================================================================================
*/
Boolean Process::getWorkingSetSize(Uint64& i64) const
{
  // not supported
  return false;
}

/*
================================================================================
NAME              : getRealUserID
DESCRIPTION       :
ASSUMPTIONS       :
PRE-CONDITIONS    :
POST-CONDITIONS   :
NOTES             :
================================================================================
*/
Boolean Process::getRealUserID(Uint64& i64) const
{
  // not supported
  return false;
}

/*
================================================================================
NAME              : getProcessGroupID
DESCRIPTION       :
ASSUMPTIONS       :
PRE-CONDITIONS    :
POST-CONDITIONS   :
NOTES             :
================================================================================
*/
Boolean Process::getProcessGroupID(Uint64& i64) const
{
  // not supported
  return false;
}

/*
================================================================================
NAME              : getProcessSessionID
DESCRIPTION       :
ASSUMPTIONS       :
PRE-CONDITIONS    :
POST-CONDITIONS   :
NOTES             :
================================================================================
*/
Boolean Process::getProcessSessionID(Uint64& i64) const
{
  // not supported
  return false;
}

/*
================================================================================
NAME              : getProcessTTY
DESCRIPTION       :
ASSUMPTIONS       :
PRE-CONDITIONS    :
POST-CONDITIONS   :
NOTES             :
================================================================================
*/
Boolean Process::getProcessTTY(String& s) const
{
  // not supported
  return false;
}

/*
================================================================================
NAME              : getModulePath
DESCRIPTION       :
ASSUMPTIONS       :
PRE-CONDITIONS    :
POST-CONDITIONS   :
NOTES             :
================================================================================
*/
Boolean Process::getModulePath(String& s) const
{
  // not supported
  return false;
}

/*
================================================================================
NAME              : getParameters
DESCRIPTION       :
ASSUMPTIONS       :
PRE-CONDITIONS    :
POST-CONDITIONS   :
NOTES             :
================================================================================
*/
Boolean Process::getParameters(Array<String>& as) const
{
  // not supported
  return false;
}

/*
================================================================================
NAME              : getProcessNiceValue
DESCRIPTION       :
ASSUMPTIONS       :
PRE-CONDITIONS    :
POST-CONDITIONS   :
NOTES             :
================================================================================
*/
Boolean Process::getProcessNiceValue(Uint32& i32) const
{
  // not supported
  return false;
}

/*
================================================================================
NAME              : getProcessWaitingForEvent
DESCRIPTION       :
ASSUMPTIONS       :
PRE-CONDITIONS    :
POST-CONDITIONS   :
NOTES             :
================================================================================
*/
Boolean Process::getProcessWaitingForEvent(String& s) const
{
  // not supported
  return false;
}

/*
================================================================================
NAME              : getCPUTime
DESCRIPTION       :
ASSUMPTIONS       :
PRE-CONDITIONS    :
POST-CONDITIONS   :
NOTES             :
================================================================================
*/
Boolean Process::getCPUTime(Uint32& i32) const
{
  // not supported
  return false;
}

/*
================================================================================
NAME              : getRealText
DESCRIPTION       :
ASSUMPTIONS       :
PRE-CONDITIONS    :
POST-CONDITIONS   :
NOTES             :
================================================================================
*/
Boolean Process::getRealText(Uint64& i64) const
{
  // not supported
  return false;
}

/*
================================================================================
NAME              : getRealData
DESCRIPTION       :
ASSUMPTIONS       :
PRE-CONDITIONS    :
POST-CONDITIONS   :
NOTES             :
================================================================================
*/
Boolean Process::getRealData(Uint64& i64) const
{
  // not supported
  return false;
}

/*
================================================================================
NAME              : getRealStack
DESCRIPTION       :
ASSUMPTIONS       :
PRE-CONDITIONS    :
POST-CONDITIONS   :
NOTES             :
================================================================================
*/
Boolean Process::getRealStack(Uint64& i64) const
{
  // not supported
  return false;
}

/*
================================================================================
NAME              : getVirtualText
DESCRIPTION       :
ASSUMPTIONS       :
PRE-CONDITIONS    :
POST-CONDITIONS   :
NOTES             :
================================================================================
*/
Boolean Process::getVirtualText(Uint64& i64) const
{
  // not supported
  return false;
}

/*
================================================================================
NAME              : getVirtualData
DESCRIPTION       :
ASSUMPTIONS       :
PRE-CONDITIONS    :
POST-CONDITIONS   :
NOTES             :
================================================================================
*/
Boolean Process::getVirtualData(Uint64& i64) const
{
  // not supported
  return false;
}

/*
================================================================================
NAME              : getVirtualStack
DESCRIPTION       :
ASSUMPTIONS       :
PRE-CONDITIONS    :
POST-CONDITIONS   :
NOTES             :
================================================================================
*/
Boolean Process::getVirtualStack(Uint64& i64) const
{
  // not supported
  return false;
}

/*
================================================================================
NAME              : getVirtualMemoryMappedFileSize
DESCRIPTION       :
ASSUMPTIONS       :
PRE-CONDITIONS    :
POST-CONDITIONS   :
NOTES             :
================================================================================
*/
Boolean Process::getVirtualMemoryMappedFileSize(Uint64& i64) const
{
  // not supported
  return false;
}

/*
================================================================================
NAME              : getVirtualSharedMemory
DESCRIPTION       :
ASSUMPTIONS       :
PRE-CONDITIONS    :
POST-CONDITIONS   :
NOTES             :
================================================================================
*/
Boolean Process::getVirtualSharedMemory(Uint64& i64) const
{
  // not supported
  return false;
}

/*
================================================================================
NAME              : getCpuTimeDeadChildren
DESCRIPTION       :
ASSUMPTIONS       :
PRE-CONDITIONS    :
POST-CONDITIONS   :
NOTES             :
================================================================================
*/
Boolean Process::getCpuTimeDeadChildren(Uint64& i64) const
{
  // not supported
  return false;
}

/*
================================================================================
NAME              : getSystemTimeDeadChildren
DESCRIPTION       :
ASSUMPTIONS       :
PRE-CONDITIONS    :
POST-CONDITIONS   :
NOTES             :
================================================================================
*/
Boolean Process::getSystemTimeDeadChildren(Uint64& i64) const
{
  // not supported
  return false;
}

/*
================================================================================
NAME              : getParentProcessID
DESCRIPTION       :
ASSUMPTIONS       :
PRE-CONDITIONS    :
POST-CONDITIONS   :
NOTES             :
================================================================================
*/
Boolean Process::getParentProcessID(String& s) const
{
  // not supported
  return false;
}

/*
================================================================================
NAME              : getRealSpace
DESCRIPTION       :
ASSUMPTIONS       :
PRE-CONDITIONS    :
POST-CONDITIONS   :
NOTES             : returns a modified pIndex that can be used in a
                    subsequent call to get next process structure
================================================================================
*/
Boolean Process::getRealSpace(Uint64& i64) const
{
  // not supported
  return false;
}

/*
================================================================================
NAME              : getHandle
DESCRIPTION       : Call uname() and get the operating system name.
ASSUMPTIONS       : None
PRE-CONDITIONS    :
POST-CONDITIONS   :
NOTES             :
================================================================================
*/
String Process::getHandle(void) const
{
  // This routine *must* return a non-empty value if even a
  // dummy provider will be tested
  return String::EMPTY;
}


/*
================================================================================
NAME              : getCSName
DESCRIPTION       : Platform-specific method to get CSName
ASSUMPTIONS       : None
PRE-CONDITIONS    :
POST-CONDITIONS   :
NOTES             :
================================================================================
*/
String Process::getCSName(void) const
{
  // This routine must be written to return this key value
  return String::EMPTY;
}


/*
================================================================================
NAME              : getOSName
DESCRIPTION       : Platform-specific method to get OSname.
ASSUMPTIONS       : None
PRE-CONDITIONS    :
POST-CONDITIONS   :
NOTES             :
================================================================================
*/
String Process::getOSName(void) const
{
  // This routine must be written to return this key value
  return String::EMPTY;
}


/*
================================================================================
NAME              : getCurrentTime
DESCRIPTION       : Platform-specific routine to get a timestamp stat Name key
ASSUMPTIONS       : None
PRE-CONDITIONS    :
POST-CONDITIONS   :
NOTES             :
================================================================================
*/
String Process::getCurrentTime(void) const
{
  return String::EMPTY;
}


/*
================================================================================
NAME              : loadProcessInfo
DESCRIPTION       : get process info from system into internal data struct
ASSUMPTIONS       : None
PRE-CONDITIONS    :
POST-CONDITIONS   :
NOTES             :
================================================================================
*/
Boolean Process::loadProcessInfo(int &pIndex)
{
  // not yet implemented
  return false;
}

/*
================================================================================
NAME              : findProcess
DESCRIPTION       : find the requested process and load its data
ASSUMPTIONS       : None
PRE-CONDITIONS    :
POST-CONDITIONS   :
NOTES             :
================================================================================
*/
Boolean Process::findProcess(const String& handle)
{
  // not yet implemented
  return false;
}
