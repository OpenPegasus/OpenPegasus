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

#include <Pegasus/Common/Config.h>
#include <Pegasus/Common/System.h>
#include <Pegasus/Common/Logger.h>
#include "OperatingSystem.h"

#include <iostream>
#include <set>
#include <sys/utsname.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/param.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <unistd.h>
#include <time.h>
#include <utmpx.h>
#include <regex.h>
#include <dirent.h>


PEGASUS_USING_STD;

OperatingSystem::OperatingSystem(void)
{
}

OperatingSystem::~OperatingSystem(void)
{
}

static Boolean _getOSName(String& osName)
{
    struct utsname  unameInfo;

    // Call uname and check for any errors.
    if (uname(&unameInfo) < 0)
    {
        return false;
    }

    osName.assign(unameInfo.sysname);

    return true;
}

static Boolean getUtilGetHostName(String& csName)
{
    char    hostName[PEGASUS_MAXHOSTNAMELEN + 1];
    struct  hostent *he;

    if (gethostname(hostName, sizeof(hostName)) != 0)
    {
        return false;
    }
    hostName[sizeof(hostName)-1] = 0;

    // Now get the official hostname.  If this call fails then return
    // the value from gethostname().

    if (he=gethostbyname(hostName))
    {
        csName.assign(he->h_name);
    }
    else
    {
        csName.assign(hostName);
    }

    return true;
}

Boolean OperatingSystem::getCSName(String& csName)
{
    return getUtilGetHostName(csName);
}

Boolean OperatingSystem::getName(String& osName)
{
    return _getOSName(osName);
}

Boolean OperatingSystem::getCaption(String& caption)
{
    caption.assign("The current Operating System");

    return true;
}

Boolean OperatingSystem::getDescription(String& description)
{
    description.assign("This instance reflects the Operating System"
            " on which the CIMOM is executing (as distinguished from instances"
            " of other installed operating systems that could be run).");

    return true;
}

Boolean OperatingSystem::getInstallDate(CIMDateTime& installDate)
{

    return false;
}

Boolean OperatingSystem::getStatus(String& status)
{

    status.assign("Unknown");

    return true;
}

Boolean OperatingSystem::getVersion(String& osVersion)
{
    struct utsname  unameInfo;

    // Call uname and check for any errors.

    if (uname(&unameInfo) < 0)
    {
        return false;
    }

    osVersion.assign(unameInfo.release);

    return true;
}

Boolean OperatingSystem::getOSType(Uint16& osType)
{
    osType = OS400;
    return true;

}

Boolean OperatingSystem::getOtherTypeDescription(String& otherTypeDescription)
{
    struct utsname  unameInfo;
    char version[sizeof(unameInfo.release) + sizeof(unameInfo.version)];

    // Call uname and check for any errors.

    if (uname(&unameInfo) < 0)
    {
        return false;
    }

    sprintf(version, "%s %s", unameInfo.version, unameInfo.release);
    otherTypeDescription.assign(version);

    return true;
}

static CIMDateTime time_t_to_CIMDateTime(time_t *time_to_represent)
{
    const int CIM_DATE_TIME_ASCII_LEN = 256;
    const CIMDateTime NULLTIME;

    CIMDateTime dt;
    char date_ascii_rep[CIM_DATE_TIME_ASCII_LEN];
    char utc_offset[20];
    struct tm broken_time;
    struct timeval   tv;
    struct timezone  tz;

    dt = NULLTIME;
    localtime_r(time_to_represent, &broken_time);
    gettimeofday(&tv,&tz);
    if (strftime(date_ascii_rep, CIM_DATE_TIME_ASCII_LEN,
                "%Y%m%d%H%M%S.000000", &broken_time))
    {
        snprintf(utc_offset, 20, "%+04ld", -tz.tz_minuteswest);
        strncat(date_ascii_rep, utc_offset, CIM_DATE_TIME_ASCII_LEN);
        dt = String(date_ascii_rep);
    }

    return dt;
}

Boolean OperatingSystem::getLastBootUpTime(CIMDateTime& lastBootUpTime)
{
    return false;
}

Boolean OperatingSystem::getLocalDateTime(CIMDateTime& localDateTime)
{
    time_t now;

    now = time(NULL);
    localDateTime = time_t_to_CIMDateTime(&now);
    return true;
}

Boolean OperatingSystem::getCurrentTimeZone(Sint16& currentTimeZone)
{
    struct timeval   tv;
    struct timezone  tz;

    // Get the time from the system.
    gettimeofday(&tv,&tz);
    currentTimeZone = -tz.tz_minuteswest;
    return true;
}

Boolean OperatingSystem::getNumberOfLicensedUsers(Uint32& numberOfLicensedUsers)
{
    return false;
}

Boolean OperatingSystem::getNumberOfUsers(Uint32& numberOfUsers)
{
    struct utmpx * utmpp;

    numberOfUsers = 0;

    while ((utmpp = getutxent()) != NULL)
    {
        if (utmpp->ut_type == USER_PROCESS)
        {
            numberOfUsers++;
        }
    }

    endutxent();

    return true;
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
    distributed = false;
    return true;
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

#ifdef _SC_AIX_KERNEL_BITMODE
    char               capability[80];
    long               ret;

    ret = sysconf (_SC_AIX_KERNEL_BITMODE);
    if (ret != -1)
    {
        sprintf (capability, "%d bit", ret);
        scapability.assign(capability);
        return true;
    }
#endif

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
