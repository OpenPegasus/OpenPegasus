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

#include "OperatingSystem.h"

#include <sys/utsname.h>
//#include <__ftp.h>
#include <netdb.h>
#include <unistd.h>

PEGASUS_USING_STD;

OperatingSystem::OperatingSystem()
{
}

OperatingSystem::~OperatingSystem()
{
}

Boolean OperatingSystem::getCSName(String& csName)
{
    char    hostName[PEGASUS_MAXHOSTNAMELEN];
    struct  hostent *he;

    if (gethostname(hostName, PEGASUS_MAXHOSTNAMELEN) != 0)
    {
        return false;
    }

// Now get the official hostname.  If this call fails then return
// the value from gethostname().

    if (he=gethostbyname(hostName))
    {
        strcpy(hostName, he->h_name);
    }

    csName.assign(hostName);

    return true;
}

Boolean OperatingSystem::getName(String& osName)
{
    struct utsname osname;

    if (uname(&osname) != -1)
    {
        osName.assign(osname.nodename);
        cerr << "OperatingSystem_zOS::getName(osName)=" << osName << endl;
        return true;
    } else return false;
}

Boolean OperatingSystem::getCaption(String& caption)
{
    caption.assign("The current Operating System");
    return true;
}

Boolean OperatingSystem::getDescription(String& description)
{
    description.assign(
     "This instance reflects the Operating System on which the "
     "CIMOM is executing (as distinguished from instances of "
     "other installed operating systems that could be run).");
    return true;
}

Boolean OperatingSystem::getInstallDate(CIMDateTime& installDate)
{
    return false;
}

Boolean OperatingSystem::getStatus(String& status)
{
    return false;
}

Boolean OperatingSystem::getVersion(String& osVersion)
{
    return false;
}

Boolean OperatingSystem::getOSType(Uint16& osType)
{
    osType = OS_390;
    return true;
}

Boolean OperatingSystem::getOtherTypeDescription(String& otherTypeDescription)
{
    return false;
}

Boolean OperatingSystem::getLastBootUpTime(CIMDateTime& lastBootUpTime)
{
    return false;
}

Boolean OperatingSystem::getLocalDateTime(CIMDateTime& localDateTime)
{
    return false;
}

Boolean OperatingSystem::getCurrentTimeZone(Sint16& currentTimeZone)
{
    return false;
}

Boolean OperatingSystem::getNumberOfLicensedUsers(Uint32& numberOfLicensedUsers)
{
    return false;
}

Boolean OperatingSystem::getNumberOfUsers(Uint32& numberOfUsers)
{
    return false;
}

Boolean OperatingSystem::getNumberOfProcesses(Uint32& numberOfProcesses)
{
    return false;
}

Boolean OperatingSystem::getMaxNumberOfProcesses(Uint32& mMaxProcesses)
{
    return false;
}

Boolean OperatingSystem::getTotalSwapSpaceSize(Uint64& mTotalSwapSpaceSize)
{
    return false;
}

Boolean OperatingSystem::getTotalVirtualMemorySize(Uint64& total)
{
    return false;
}

Boolean OperatingSystem::getFreeVirtualMemory(Uint64& freeVirtualMemory)
{
    return false;
}

Boolean OperatingSystem::getFreePhysicalMemory(Uint64& total)
{
    return false;
}

Boolean OperatingSystem::getTotalVisibleMemorySize(Uint64& memory)
{
    return false;
}

Boolean OperatingSystem::getSizeStoredInPagingFiles(Uint64& total)
{
    return false;
}

Boolean OperatingSystem::getFreeSpaceInPagingFiles(
    Uint64& freeSpaceInPagingFiles)
{
    return false;
}

Boolean OperatingSystem::getMaxProcessMemorySize(Uint64& maxProcessMemorySize)
{
    return false;
}

Boolean OperatingSystem::getDistributed(Boolean& distributed)
{
    return false;
}

Boolean OperatingSystem::getMaxProcsPerUser(Uint32& maxProcsPerUser)
{
    return false;
}

Boolean OperatingSystem::getSystemUpTime(Uint64& mUpTime)
{
    return false;
}

Boolean OperatingSystem::getOperatingSystemCapability(String& scapability)
{
    return false;
}

Uint32 OperatingSystem::_reboot()
{
    return false;
}

Uint32 OperatingSystem::_shutdown()
{
    return false;
}

