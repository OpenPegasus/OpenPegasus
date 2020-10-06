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

#include <windows.h>

#if (_MSC_VER >= 1300) || defined(PEGASUS_WINDOWS_SDK_HOME)
#include <psapi.h>
#include <pdh.h>
#endif
#include <Pegasus/Provider/ProviderException.h>

#include "OperatingSystem.h"

#include <sstream>
#include <iomanip>

OperatingSystem::OperatingSystem()
{
}

OperatingSystem::~OperatingSystem()
{
}

Boolean OperatingSystem::getCSName(String& csName)
{
    unsigned long nSize = 2048;
    char value[2048];

    if(!GetComputerName(value, &nSize))
    {
        //Hostname cannot be retrieved
        csName.assign("Unknown");
        return false;
    }
    else
    {
        csName.assign(value);
    }

    return true;
}

Boolean OperatingSystem::getName(String& osName)
{
    String versionName;

    OSVERSIONINFO osvi;

    ::memset(&osvi, 0, sizeof(osvi));

    osvi.dwOSVersionInfoSize = sizeof(osvi);

    ::GetVersionEx(&osvi);

    // Get the Base Windows Platform
    switch (osvi.dwPlatformId)
    {
        case VER_PLATFORM_WIN32_NT:

            if ( osvi.dwMajorVersion <= 4 )
            {
                versionName.assign("Microsoft Windows NT");
            }
            else if ( osvi.dwMajorVersion == 5 && osvi.dwMinorVersion == 0 )
            {
                versionName.assign("Microsoft Windows 2000");
            }
            else if ( osvi.dwMajorVersion == 5 && osvi.dwMinorVersion == 1 )
            {
                versionName.assign("Microsoft Windows XP");
            }
            else if ( osvi.dwMajorVersion == 5 && osvi.dwMinorVersion == 2 )
            {
                versionName.assign("Microsoft Windows Server 2003");
            }

            break;

        case VER_PLATFORM_WIN32_WINDOWS:

            if (osvi.dwMajorVersion == 4 && osvi.dwMinorVersion == 0)
            {
                versionName.assign("Microsoft Windows 95");
                if ( osvi.szCSDVersion[1] == 'C' ||
                     osvi.szCSDVersion[1] == 'B' )
                     versionName.append("OSR2");
            }
            else if (osvi.dwMajorVersion == 4 && osvi.dwMinorVersion == 10)
            {
                versionName.assign("Microsoft Windows 98");
                if ( osvi.szCSDVersion[1] == 'A' )
                    versionName.append("SE" );
            }
            else if (osvi.dwMajorVersion == 4 && osvi.dwMinorVersion == 90)
            {
                versionName.assign("Microsoft Windows Me");
            }

            break;

        default:

            break;
    }

    osName.assign(versionName);
    return true;
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

/**
   getStatus method for Windows implementation of OS provider

   Would like to be able to return and actual status vs. just
   always Unknown, but didn't know how to differentiate between
   OK and Degraded (assuming they are the only values that make
   sense, since the CIMOM is up and running), but one could see
   an argument for including Stopping if the Shutdown or Reboot
   methods have been invoked. For now, always return "Unknown".
   */
Boolean OperatingSystem::getStatus(String& status)
{
// ATTN-SLC-P3-17-Apr-02: Get true Windows status (vs. Unknown) BZ#44

   status.assign("Unknown");

   return true;
}

Boolean OperatingSystem::getVersion(String& osVersion)
{
   OSVERSIONINFO ver;

   ::memset(&ver, 0, sizeof(ver));

   ver.dwOSVersionInfoSize = sizeof(ver);

   ::GetVersionEx(&ver);

   std::stringstream ss;

   ss << ver.dwMajorVersion << '.' << ver.dwMinorVersion << '.' <<
      ver.dwBuildNumber;

   osVersion = ss.str().c_str();

   return true;
}

Boolean OperatingSystem::getOSType(Uint16& osType)
{
   osType = Unknown;

   OSVERSIONINFO osvi;

   ::memset(&osvi, 0, sizeof(osvi));

   osvi.dwOSVersionInfoSize = sizeof(osvi);

   ::GetVersionEx(&osvi);

   switch (osvi.dwPlatformId)
   {
       case VER_PLATFORM_WIN32_NT:

           if ( osvi.dwMajorVersion <= 4 )
           {
               osType = WINNT;
           }
           else if ( osvi.dwMajorVersion == 5 && osvi.dwMinorVersion == 0 )
           {
               osType = Windows_2000;
           }
           else if ( osvi.dwMajorVersion == 5 && osvi.dwMinorVersion == 1 )
           {
               osType = Windows_XP;
           }
           else if ( osvi.dwMajorVersion == 5 && osvi.dwMinorVersion == 2 )
           {
               osType = Windows_2003;
           }

           break;

       case VER_PLATFORM_WIN32_WINDOWS:

           if (osvi.dwMajorVersion == 4 && osvi.dwMinorVersion == 0)
           {
               osType = WIN95;
           }
           else if (osvi.dwMajorVersion == 4 && osvi.dwMinorVersion == 10)
           {
               osType = WIN98;
           }
           else if (osvi.dwMajorVersion == 4 && osvi.dwMinorVersion == 90)
           {
               osType = Windows_Me;
           }

           break;

       default:

           break;
   }

   return true;
}

Boolean OperatingSystem::getOtherTypeDescription(String& otherTypeDescription)
{
    return false;
}

Boolean OperatingSystem::getLastBootUpTime(CIMDateTime& lastBootUpTime)
{
    Uint64 sysUpTime = 0;

    if(getSystemUpTime(sysUpTime))
    {
        // convert sysUpTime to microseconds
        sysUpTime *= (1000 * 1000);

        CIMDateTime currentTime = CIMDateTime::getCurrentDateTime();
        CIMDateTime bootTime =
            CIMDateTime(currentTime.toMicroSeconds() - sysUpTime, false);

        // adjust UTC offset
        String s1 = currentTime.toString();
        String s2 = bootTime.toString();

        s2[20] = s1[20];
        s2[21] = s1[21];
        s2[22] = s1[22];
        s2[23] = s1[23];

        lastBootUpTime = CIMDateTime(s2);

        return true;
    }

    return false;
}

Boolean OperatingSystem::getLocalDateTime(CIMDateTime& localDateTime)
{
   SYSTEMTIME time;
   Sint16 currentTimeZone;

   ::memset(&time, 0, sizeof(time));

   ::GetLocalTime(&time);

   std::stringstream ss;

   ss << std::setfill('0');
   ss << std::setw(4) << time.wYear;
   ss << std::setw(2) << time.wMonth;
   ss << std::setw(2) << time.wDay;
   ss << std::setw(2) << time.wHour;
   ss << std::setw(2) << time.wMinute;
   ss << std::setw(2) << time.wSecond;
   ss << ".";
   ss << std::setw(6) << time.wMilliseconds * 1000;
   if (getCurrentTimeZone(currentTimeZone))
   {
      ss << (currentTimeZone < 0 ? "-" : "+");
   }
   ss << std::setw(3) << ::abs(currentTimeZone);

   localDateTime = CIMDateTime (String (ss.str().c_str()));

   return true;
}

Boolean OperatingSystem::getCurrentTimeZone(Sint16& currentTimeZone)
{
   currentTimeZone = 0;

   TIME_ZONE_INFORMATION timezone;

   ::memset(&timezone, 0, sizeof(timezone));

   switch(::GetTimeZoneInformation(&timezone)) {
   case TIME_ZONE_ID_UNKNOWN:
      currentTimeZone = (Sint16)timezone.Bias;
      break;
   case TIME_ZONE_ID_STANDARD:
      currentTimeZone = (Sint16)timezone.Bias + (Sint16)timezone.StandardBias;
      break;
   case TIME_ZONE_ID_DAYLIGHT:
      currentTimeZone = (Sint16)timezone.Bias + (Sint16)timezone.DaylightBias;
      break;
   case TIME_ZONE_ID_INVALID:
   {
       char exceptionMsg[100] = "";
       sprintf (
           exceptionMsg,
           "Invalid time zone information : %d",
           GetLastError());
       throw CIMOperationFailedException(exceptionMsg);
   }
   default:
      break;
   }

   // the bias used to calculate the time zone is a factor that is used to
   // determine the UTC time from the local time. to get the UTC offset from
   // the local time, use the inverse.
   if(currentTimeZone != 0)
   {
      currentTimeZone *= -1;
   }

   return true;
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
    numberOfProcesses = 0;

#if (_MSC_VER >= 1300) || defined(PEGASUS_WINDOWS_SDK_HOME)
    DWORD processHandles[1024];
    DWORD size = 0;

    BOOL rc =
        ::EnumProcesses(
            processHandles,
            sizeof(processHandles),
            &size);

    if ((rc == TRUE) && (sizeof(processHandles) != size))
    {
        numberOfProcesses = size / sizeof(processHandles[0]);
    }
#endif

    return (numberOfProcesses == 0 ? false : true);
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
   total = 0;

   MEMORYSTATUS mem;

   ::memset(&mem, 0, sizeof(mem));

   mem.dwLength = sizeof(mem);

   ::GlobalMemoryStatus(&mem);

   total = mem.dwTotalVirtual / 1024;

   if (total)
      return true;
   else
      return false;
}

Boolean OperatingSystem::getFreeVirtualMemory(Uint64& freeVirtualMemory)
{
   freeVirtualMemory = 0;

   MEMORYSTATUS mem;

   ::memset(&mem, 0, sizeof(mem));

   mem.dwLength = sizeof(mem);

   ::GlobalMemoryStatus(&mem);

   freeVirtualMemory = mem.dwAvailVirtual / 1024;

   return true;
}

Boolean OperatingSystem::getFreePhysicalMemory(Uint64& total)
{
   total = 0;

   MEMORYSTATUS mem;

   ::memset(&mem, 0, sizeof(mem));

   mem.dwLength = sizeof(mem);

   ::GlobalMemoryStatus(&mem);

   total = mem.dwAvailPhys / 1024;

   return true;
}

Boolean OperatingSystem::getTotalVisibleMemorySize(Uint64& memory)
{
   memory = 0;

   MEMORYSTATUS mem;

   ::memset(&mem, 0, sizeof(mem));

   mem.dwLength = sizeof(mem);

   ::GlobalMemoryStatus(&mem);

   memory = mem.dwAvailVirtual / 1024;

   if (memory)
      return true;
   else
      return false;
}

Boolean OperatingSystem::getSizeStoredInPagingFiles(Uint64& total)
{
   total = 0;

   MEMORYSTATUS mem;

   ::memset(&mem, 0, sizeof(mem));

   mem.dwLength = sizeof(mem);

   ::GlobalMemoryStatus(&mem);

   total = mem.dwTotalPageFile / 1024;

   return true;
}

Boolean OperatingSystem::getFreeSpaceInPagingFiles(
    Uint64& freeSpaceInPagingFiles)
{
   freeSpaceInPagingFiles = 0;

   MEMORYSTATUS mem;

   ::memset(&mem, 0, sizeof(mem));

   mem.dwLength = sizeof(mem);

   ::GlobalMemoryStatus(&mem);

   freeSpaceInPagingFiles = mem.dwAvailPageFile / 1024;

   return true;
}

Boolean OperatingSystem::getMaxProcessMemorySize(Uint64& maxProcessMemorySize)
{
   maxProcessMemorySize = 0;

   SYSTEM_INFO sys;

   ::memset(&sys, 0, sizeof(sys));

   ::GetSystemInfo(&sys);

   maxProcessMemorySize =
                (reinterpret_cast<char *>(sys.lpMaximumApplicationAddress) -
                 reinterpret_cast<char *>(sys.lpMinimumApplicationAddress))
                                / 1024;

   return true;
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
    mUpTime = 0;

    #if (_MSC_VER >= 1300) || defined(PEGASUS_WINDOWS_SDK_HOME)
    HANDLE query = 0;

    LONG rc =
        ::PdhOpenQuery(
            0,
            0,
            &query);

    if(rc == ERROR_SUCCESS)
    {
        HANDLE counter = 0;

        rc = ::PdhAddCounter(
                query,
                "\\System\\System Up Time",
                0,
                &counter);

        if(rc == ERROR_SUCCESS)
        {
            ::PdhCollectQueryData(query);

            PDH_FMT_COUNTERVALUE value;
            ::memset(&value, 0, sizeof(value));

            rc = ::PdhGetFormattedCounterValue(
                    counter,
                    PDH_FMT_LARGE,
                    0,
                    &value);

            if(rc == ERROR_SUCCESS)
            {
                mUpTime = value.largeValue;
            }

            ::PdhRemoveCounter(query);
        }

        ::PdhCloseQuery(query);
    }
    #endif

    return (mUpTime == 0 ? false : true);
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
