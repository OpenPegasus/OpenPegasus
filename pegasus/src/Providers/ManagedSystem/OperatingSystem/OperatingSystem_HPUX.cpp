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
#include <Pegasus/Common/Time.h>
#include <Pegasus/Common/Exception.h>

#include "OperatingSystem.h"

#include <iostream>
#include <set>
#include <sys/utsname.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/param.h>
#include <sys/pstat.h>
#include <unistd.h>
#include <netdb.h>
#include <time.h>
#include <utmpx.h>
#include <regex.h>
#include <dirent.h>
#include <dl.h>

/* ==========================================================================
   Type Definitions
   ========================================================================== */
typedef unsigned char boolean_t;
typedef unsigned long ErrorStatus_t;

PEGASUS_USING_STD;

OperatingSystem::OperatingSystem()
{
}

OperatingSystem::~OperatingSystem()
{
}

/**
   _getOSName method of the HP-UX implementation for the OS Provider

   Calls uname() to get the operating system name.

  */
static Boolean _getOSName(String& osName)
{
    struct utsname  unameInfo;

    // Call uname and check for any errors.
    if ((uname(&unameInfo) < 0) && (errno != EOVERFLOW))
    {
       return false;
    }

    osName.assign(unameInfo.sysname);

    return true;
}

/**
   getUtilGetHostName method for the HP-UX implementation of the OS Provider

   This supporting utility function gets the name of the host system
   from gethostname and gethostbyname.

  */

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

    if ((he = gethostbyname(hostName)) != 0)
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

/**
   getCaption method for HP-UX implementation of OS Provider

   Uses uname system call and extracts information for the Caption.
  */
Boolean OperatingSystem::getCaption(String& caption)
{
   struct utsname     unameInfo;

   // Call uname and check for any errors.
   if ((uname(&unameInfo) < 0) && (errno != EOVERFLOW))
   {
       return false;
   }

   // append in caption the system name and release.
   caption.assign(unameInfo.sysname);
   caption.append(" ");
   caption.append(unameInfo.release);

   return true;
}

/**
   getDescription method for HP-UX implementation of OS Provider
  */
Boolean OperatingSystem::getDescription(String& description)
{
// 01-jul-05: Implement getDescription for HP-UX.
   return false;
}

/**
   getInstallDate method for HP-UX implementation of OS provider

   Need to determine reliable method of knowing install date
   one possibility is date of /stand/vmunix (but not always
   truly the date the OS was installed. For now, don't return
   any date (function returns FALSE).
*/
Boolean OperatingSystem::getInstallDate(CIMDateTime& installDate)
{
// ATTN-SLC-P2-17-Apr-02:  Implement getInstallDate for HP-UX

   return false;
}

/**
   getStatus method for HP-UX implementation of OS provider

   Would like to be able to return and actual status vs. just
   always Unknown, but didn't know how to differentiate between
   OK and Degraded (assuming they are the only values that make
   sense, since the CIMOM is up and running), but one could see
   an argument for including Stopping if the Shutdown or Reboot
   methods have been invoked. For now, always return "Unknown".
*/
Boolean OperatingSystem::getStatus(String& status)
{

// ATTN-SLC-P3-17-Apr-02: Get true HP-UX status (vs. Unknown) BZ#44


   status.assign("Unknown");

   return true;
}

/**
   getVersion method for HP-UX implementation of OS provider

   Uses uname system call and extracts the release information
   (e.g., B.11.20).  Version field in uname actually contains
   user license info (thus isn't included).

   Returns FALSE if uname call results in errors.
  */

Boolean OperatingSystem::getVersion(String& osVersion)
{

    struct utsname  unameInfo;

    // Call uname and check for any errors.

    if ((uname(&unameInfo) < 0) && (errno != EOVERFLOW))
    {
       return false;
    }

    osVersion.assign(unameInfo.release);

    return true;
}

/**
    getOSType method for HP-UX implementation of OS Provider

    Always returns 8 = HP-UX
  */

Boolean OperatingSystem::getOSType(Uint16& osType)
{
    osType = HP_UX;  // from enum in .h file
    return true;
}

/**
    getOtherTypeDescription method for HP-UX implementation of OS provider

    Returns FALSE since not needed for HP-UX (don't return the empty string).
  */
Boolean OperatingSystem::getOtherTypeDescription(String& otherTypeDescription)
{
    return false;
}

/**
   getLastBootUpTime method for HP-UX implementation of OS Provider

   Gets information from pstat call.  Internally in UTC (Universal
   Time Code) which must be converted to localtime for CIM
  */
Boolean OperatingSystem::getLastBootUpTime(CIMDateTime& lastBootUpTime)
{
    struct tm tmval;
    struct pst_static pst;
    char dateTimeBuffer[26];
    int tzMinutesEast;

    // Get the static information from the pstat call to the system.

    if (pstat_getstatic(&pst, sizeof(pst), (size_t)1, 0) == -1)
    {
        return false;
    }
    // Get the boot time and convert to local time.

// ATTN-SLC-P2-17-Apr-02: use CIMOM DateTime function & consistency, BZ#45

    time_t tmp_time = pst.boot_time;
    localtime_r(&tmp_time, &tmval);
    tzMinutesEast = - (int) timezone / 60;
    if ((tmval.tm_isdst > 0) && daylight)
    {
        // ATTN: It is unclear how to determine the DST offset.  Assume 1 hour.
        tzMinutesEast += 60;
    }

    // Format the date.
    sprintf(
        dateTimeBuffer,
        "%04d%02d%02d%02d%02d%02d.%06d%+04d",
        1900 + tmval.tm_year,
        tmval.tm_mon + 1,   // HP-UX stores month 0-11
        tmval.tm_mday,
        tmval.tm_hour,
        tmval.tm_min,
        tmval.tm_sec,
        0,
        tzMinutesEast);
    lastBootUpTime.set(dateTimeBuffer);
    return true;
}

/**
   getLocalDateTime method for HP-UX implementation of OS Provider

   Gets information from localtime call, converted to CIM
   DateTime format.
  */
Boolean OperatingSystem::getLocalDateTime(CIMDateTime& localDateTime)
{
    // Get the date and time from the system.
    localDateTime = CIMDateTime::getCurrentDateTime();
    return true;
}

/**
    getCurrentTimeZone method for HP-UX implementation of OS Provider

    Gets information from Time::gettimeofday call and ensures sign follows
    CIM standard.
  */
Boolean OperatingSystem::getCurrentTimeZone(Sint16& currentTimeZone)
{
    time_t systemTime;
    struct tm tmval;

    // Get the time from the system.
    systemTime = time(0);
    localtime_r(&systemTime, &tmval);
    currentTimeZone = - (Sint16) (timezone / 60);
    if ((tmval.tm_isdst > 0) && daylight)
    {
        // ATTN: It is unclear how to determine the DST offset.  Assume 1 hour.
        currentTimeZone += 60;
    }

    return true;
}

/**
    getNumberOfLicensedUsers method for HP-UX implementation of OS provider

    Calls uname and checks the version string (to get user license
    information.  This version field doesn't currently distinguish
    between 128, 256, and unlimited user licensed (all = U).
    Need to determine how to differentiate and fix this, for now return
    0 (which is unlimited).  Don't know if uname -l has same limitation.
  */
Boolean OperatingSystem::getNumberOfLicensedUsers(Uint32& numberOfLicensedUsers)
{
    struct utsname  unameInfo;

    // Call uname and check for any errors.
    if ((uname(&unameInfo) < 0) && (errno != EOVERFLOW))
    {
       return false;
    }
    // For HP-UX, the number of licensed users is returned in the version
    // field of uname result.
    switch (unameInfo.version[0]) {
        case 'A' : { numberOfLicensedUsers = 2; break; }
        case 'B' : { numberOfLicensedUsers = 16; break; }
        case 'C' : { numberOfLicensedUsers = 32; break; }
        case 'D' : { numberOfLicensedUsers = 64; break; }
        case 'E' : { numberOfLicensedUsers = 8; break; }
        case 'U' : {
            // U could be 128, 256, or unlimited
            // need to find test system with 128 or 256 user license
            // to determine if uname -l has correct value
            // for now, return 0 = unlimited
//ATTN-SLC-P2-18-Apr-02: Distinguish HP-UX 128,256,unliminted licenses BZ#43
            numberOfLicensedUsers = 0;
            break;
        }
        default : return false;
     }
    return true;
}

/**
    getNumberOfUsers method for HP-UX implementation of OS Provider

    Goes through the utents, counting the number of type USER_PROCESS
  */
Boolean OperatingSystem::getNumberOfUsers(Uint32& numberOfUsers)
{
    struct utmpx * utmpp;

    numberOfUsers = 0;

// ATTN-SLC-P3-17-Apr-02: optimization? parse uptime instead?

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

/**
    getNumberOfProcesses method for HP-UX implementation of OS Provider

    Gets number of active processes from pstat.
  */
Boolean OperatingSystem::getNumberOfProcesses(Uint32& numberOfProcesses)
{
    struct pst_dynamic psd;

    if (pstat_getdynamic(&psd, sizeof(psd), (size_t)1, 0) == -1)
    {
        return false;
    }

    numberOfProcesses = psd.psd_activeprocs;

    return true;
}

/**
    getMaxNumberOfProcesses method for HP-UX implementation of OS Provider

    Gets maximum number of processes from pstat.
  */
Boolean OperatingSystem::getMaxNumberOfProcesses(Uint32& mMaxProcesses)
{
    struct pst_static pst;

    if (pstat_getstatic(&pst, sizeof(pst), (size_t)1, 0) == -1)
    {
        return false;
    }

    mMaxProcesses = pst.max_proc;

    return true;
}

/**
    _totalVM method for HP-UX implementation of HP-UX

    Gets information from swapinfo -q command (already in KB).
    Invoked for TotalVirtualMemory as well as TotalSwapSpaceSize.
    Would be more efficient to get this only once.
  */
Uint64 OperatingSystem::_totalVM()
{
    char               mline[80];
    FILE             * mswapInfo;
    Uint32             swapSize;

    // Initialize the return parameter in case swapinfo is not available.
    swapSize = 0;

    // Use a pipe to invoke swapinfo.
    if ((mswapInfo = popen("/usr/sbin/swapinfo -q 2>/dev/null", "r")) != NULL)
    {
        // Now extract the total swap space size from the swapinfo output.
        while (fgets(mline, 80, mswapInfo))
        {
            sscanf(mline, "%u", &swapSize);
        }  // end while

        (void)pclose (mswapInfo);
    }
    return swapSize;
}
/**
   getTotalSwapSpaceSize method for HP-UX implementation of HP-UX

   Gets information from swapinfo -q command (techically not swap
   space, it's paging).  No formal paging files, report as swap.

  */
Boolean OperatingSystem::getTotalSwapSpaceSize(Uint64& mTotalSwapSpaceSize)
{
    mTotalSwapSpaceSize = _totalVM();
    if (mTotalSwapSpaceSize != 0)
        return true;
    else
        return false;
}

/**
    getTotalVirutalMemorySize method for HP-UX implementation of HP-UX

    Gets information from swapinfo -q command (techically not swap
    space, it's paging).  Same as the information returned for
    TotalSwapSpace.

  */
Boolean OperatingSystem::getTotalVirtualMemorySize(Uint64& total)
{

// Returns the same information as TotalSwapSpace (since the same)

    total = _totalVM();
    if (total != 0)
        return true;
    else
        return false;
}

/**
   getFreeVirutalMemorySize method for HP-UX implementation of HP-UX

   Gets information from swapinfo -at command (the Free column)

  */
Boolean OperatingSystem::getFreeVirtualMemory(Uint64& freeVirtualMemory)
{
    char               mline[80];
    FILE             * mswapInfo;
    Uint32             swapAvailable = 0;
    Uint32             swapUsed = 0;
    Uint32             swapFree = 0;

    // Initialize the return parameter in case swapinfo is not available.
    freeVirtualMemory = 0;

    // Use a pipe to invoke swapinfo.
    if ((mswapInfo = popen("/usr/sbin/swapinfo -at 2>/dev/null", "r")) != NULL)
    {
        // Now extract the total swap space size from the swapinfo output.
        while (fgets(mline, 80, mswapInfo))
        {
           sscanf(mline, "total %u %u %u", &swapAvailable,
                  &swapUsed, &swapFree);
        }  // end while

        (void)pclose (mswapInfo);
    }
    freeVirtualMemory = swapFree;
    if (freeVirtualMemory != 0)
        return true;
    else
        return false;
}

/**
   getFreePhysicalMemory method for HP-UX implementation of HP-UX

   Gets information from the pstat system call (psd_free field)

  */
Boolean OperatingSystem::getFreePhysicalMemory(Uint64& total)
{
    struct pst_dynamic psd;
    struct pst_static pst;
    float  psize;
    float  subtotal;

    // Feb-25-2005:  Correct value to reflect Kbytes instead of pages

    if (pstat_getstatic(&pst, sizeof(pst), (size_t)1, 0) == -1)
    {
        return false;
    }
    psize = pst.page_size / 1024;

    if (pstat_getdynamic(&psd, sizeof(psd), (size_t)1, 0) == -1)
    {
        return false;
    }
    subtotal = ((float)psd.psd_free * psize);
    total = subtotal;
    return true;
}

/**
   getTotalVisibleMemorySize method for HP-UX implementation of OS Provider

   Gets information from pstat (pst.physcial_memory adjusted for page size)
   */
Boolean OperatingSystem::getTotalVisibleMemorySize(Uint64& memory)
{
    float         psize;
    float         total;
    struct        pst_static pst;

    if (pstat_getstatic(&pst, sizeof(pst), (size_t)1, 0) == -1)
    {
        return false;
    }

    // Feb-25-2005:  Correct value to reflect Kbytes instead of Mbytes
    psize = pst.page_size / 1024;
    total = ((float)pst.physical_memory * psize);
    memory = total;
    return true;
}

/**
   getSizeStoredInPagingFiles method for HP-UX implementation of OS Provider

   HP-UX doesn't have Paging Files, thus return 0 (as specified in the MOF)
   */
Boolean OperatingSystem::getSizeStoredInPagingFiles(Uint64& total)
{
    total = 0;
    return true;
}

/**
    getFreeSpaceInPagingFiles method for HP-UX implementation of OS Provider

    HP-UX doesn't have Paging Files, thus return 0 (as specified in the MOF)
   */
Boolean OperatingSystem::getFreeSpaceInPagingFiles(
    Uint64& freeSpaceInPagingFiles)
{
    freeSpaceInPagingFiles = 0;
    return true;
}

static Boolean getMaxProcMemViaKmtune(Boolean are32bit,
                                      Uint64& maxProcMemSize)
{
    char               mline[80];
    FILE             * mtuneInfo;
    Uint32             maxdsiz = 0;
    Uint32             maxssiz = 0;
    Uint32             maxtsiz = 0;
    Uint64             maxdsiz_64bit = 0;
    Uint64             maxssiz_64bit = 0;
    Uint64             maxtsiz_64bit = 0;

    if (are32bit)
    {
        // Use a pipe to invoke kmtune (since don't have gettune on 11.0)
        if ((mtuneInfo = popen("/usr/sbin/kmtune -q maxdsiz -q maxssiz "
                               "-q maxtsiz 2>/dev/null", "r")) != NULL)
        {
            // Now extract the three values and sum them
            while (fgets(mline, 80, mtuneInfo))
            {
                sscanf(mline, "maxdsiz %x", &maxdsiz);
                sscanf(mline, "maxssiz %x", &maxssiz);
                sscanf(mline, "maxtsiz %x", &maxtsiz);
            }    // end while

            (void)pclose (mtuneInfo);
            maxProcMemSize = (maxdsiz + maxssiz + maxtsiz);
            return true;
        } // end if popen worked
        return false;
    }  // end if are32bit
    else   // are 64bit, different parameter names must be used
    {
        // Use a pipe to invoke kmtune (since don't have gettune on all OSs)
        if ((mtuneInfo = popen("/usr/sbin/kmtune -q maxdsiz_64bit "
                               "-q maxssiz_64bit -q maxtsiz_64bit "
                               "2> /dev/null","r")) != NULL)
        {
            // Now extract the three values and sum them
            while (fgets(mline, 80, mtuneInfo))
            {
                sscanf(mline, "maxdsiz_64bit %llx", &maxdsiz_64bit);
                sscanf(mline, "maxssiz_64bit %llx", &maxssiz_64bit);
                sscanf(mline, "maxtsiz_64bit %llx", &maxtsiz_64bit);
            }  // end while

            (void)pclose (mtuneInfo);
            maxProcMemSize = (maxdsiz_64bit + maxssiz_64bit + maxtsiz_64bit);
            return true;
        } // end if popen worked
        return false;
    }
}

static Boolean getMaxProcMemViaGettune(
    Boolean are32bit,
    Uint64& maxProcMemSize)
{
    uint64_t         maxdsiz = 0;
    uint64_t         maxssiz = 0;
    uint64_t         maxtsiz = 0;
    uint64_t         maxdsiz_64bit = 0;
    uint64_t         maxssiz_64bit = 0;
    uint64_t         maxtsiz_64bit = 0;
    uint64_t         total = 0;

    // we may be compiling on a system without gettune, but
    // run-time would have checked version and only be here
    // if we expect to have the gettune system call in libc

    // if handle is NULL, findsym is supposed to check currently
    // loaded libraries (and we know libc should be loaded)

    // get the procedure pointer for gettune
    int (*gettune_sym) (const char *, uint64_t *) = NULL;
    shl_t handle = NULL;

    if (shl_findsym(&handle,
                    "gettune",
                    TYPE_PROCEDURE,
                    (void *)&gettune_sym) != 0)
    {
        return false;
    }
    if (gettune_sym == NULL)
    {
        return false;
    }

    if (are32bit)
    {
        if (gettune_sym("maxdsiz", &maxdsiz) != 0)
            return false;  // fail if can't get info
        if (gettune_sym("maxssiz", &maxssiz) != 0)
            return false;  // fail if can't get info
        if (gettune_sym("maxtsiz", &maxtsiz) != 0)
            return false;  // fail if can't get info
        total  = maxdsiz + maxtsiz + maxssiz;
        maxProcMemSize = total;
        return true;
    }  // end if are32bit
    else
    {  // are 64bit
        if (gettune_sym("maxdsiz_64bit", &maxdsiz_64bit) != 0)
            return false;  // fail if can't get info
        if (gettune_sym("maxssiz_64bit", &maxssiz_64bit) != 0)
            return false;  // fail if can't get info
        if (gettune_sym("maxtsiz_64bit", &maxtsiz_64bit) != 0)
            return false;  // fail if can't get info
        total  = maxdsiz_64bit + maxtsiz_64bit + maxssiz_64bit;
        maxProcMemSize = total;
        return true;
    }
}

/**
    getMaxProcessMemorySize method for HP-UX implementation of OS Provider

    Calculate values by summing kernel tunable values for max data size, max
    stack size, and max text size.  Different names if 32-bit vs. 64-bit.
    NOT the pstat() pst_maxmem value; that is the amount of physical
    memory available to all user processes when the system first boots.

    Could use the gettune(2) system call on some systems, but it isn't
    available for 11.0.  kmtune format changes in release 11.20, so will
    have separate paths anyway (vs. same kmtune parsing for all releases).
    Thus, chose to parse for 11.0, and use gettune for other releases.
   */
Boolean OperatingSystem::getMaxProcessMemorySize(Uint64& maxProcessMemorySize)
{
    long ret;

    // Initialize the return parameter in case kmtune is not available.
    maxProcessMemorySize = 0;

    ret = sysconf (_SC_KERNEL_BITS);
    if (ret == -1)
    {
        return false;
    }

    // First, check if we're an 11.0 system, if so, use kmtune parsing
    // If have many such checks, can store off Release/Version versus
    // getting as needed.

    struct utsname  unameInfo;
    // Call uname and check for any errors.
    if ((uname(&unameInfo) < 0) && (errno != EOVERFLOW))
    {
        return false;
    }

    // Need to check if 32-bit or 64-bit to use the suitable name
    if (ret == 32)
    {  // we're 32 bit
        if (strcmp(unameInfo.release,"B.11.00")==0)
        {
            // Use kmtune on 11.0 (since don't have gettune)
            return getMaxProcMemViaKmtune(true, maxProcessMemorySize);
        }
        else
        {
            // can use gettune call 11.11 and onwards (won't be WBEM pre-11.0)
            return getMaxProcMemViaGettune(true, maxProcessMemorySize);
        }
    } // end if (ret == 32)

    else   // so ret was 64 (only returns -1, 32, or 64)
    {
        if (strcmp(unameInfo.release,"B.11.00")==0)
        {
            // Use kmtune on 11.0 (since don't have gettune)
            return getMaxProcMemViaKmtune(false, maxProcessMemorySize);
        }
        else
        {
            // can use gettune call 11.11 and onwards (won't be WBEM pre-11.0)
            return getMaxProcMemViaGettune(false, maxProcessMemorySize);
        }
    }  // end else
}

/**
    getDistributed method for HP-UX implementation of OS Provider

    Always sets the distributed boolean to FALSE
  */
Boolean OperatingSystem::getDistributed(Boolean& distributed)
{
    distributed = false;

    return true;
}

/**
    getMaxProcsPerUser method for HP-UX implementation of OS Provider

    Gets the information using kmtune.
  */
Boolean OperatingSystem::getMaxProcsPerUser (Uint32& maxProcsPerUser)
{
    FILE             * mtuneInfo;
    char               mline[80];
    struct utsname     unameInfo;
    uint64_t           maxuprc = 0;

    // Call uname and check for any errors.
    if ((uname(&unameInfo) < 0) && (errno != EOVERFLOW))
    {
        return false;
    }

    if (strcmp(unameInfo.release,"B.11.00")==0)
    {
        // Use a pipe to invoke kmtune (since don't have gettune on 11.0)
        if ((mtuneInfo =
                 popen("/usr/sbin/kmtune -q maxuprc 2>/dev/null", "r")) != NULL)
        {
            // Now extract the value
            while (fgets(mline, 80, mtuneInfo))
            {
                sscanf(mline, "maxuprc %u", &maxProcsPerUser);
            }
            (void)pclose (mtuneInfo);
        }
        else
        {
            return false;
        }
    }
    else
    {
        // we may be compiling on a system without gettune, but
        // run-time would have checked version and only be here
        // if we expect to have the gettune system call in libc

        // if handle is NULL, findsym is supposed to check currently
        // loaded libraries (and we know libc should be loaded)

        // get the procedure pointer for gettune
        int (*gettune_sym) (const char *, uint64_t *) = NULL;
        shl_t handle = NULL;

        if (shl_findsym(&handle,
               "gettune",
               TYPE_PROCEDURE,
               (void *)&gettune_sym) != 0)
        {
            return false;
        }
        if (gettune_sym == NULL)
        {
            return false;
        }
        if (gettune_sym("maxuprc", &maxuprc) != 0)
        {
            return false;  // fail if can't get info
        }
        maxProcsPerUser = maxuprc;
    }
    return true;
}

/**
    getSystemUpTime method for HP-UX implementation of OS Provider

    Calculates the information from the local time and boot time.
    Could also consider use of uptime information.  Ideally, would
    like to have consistency across the time-related properties
    (e.g., uptime = local time - Boot time).
  */
Boolean OperatingSystem::getSystemUpTime(Uint64& mUpTime)
{
    time_t timeval;
    struct pst_static pst;

    if (pstat_getstatic(&pst, sizeof(pst), (size_t)1, 0) == -1)
    {
        return false;
    }
    else
    {
// ATTN-SLC-P2-17-Apr-02: use CIMOM DateTime function & consistency, BZ#45
        timeval= time((time_t *)NULL);
        timeval= (time_t)((long)timeval - (long)pst.boot_time);
        mUpTime = (long)timeval;
    }

    return true;
}

/**
    getOperatingSystemCapability method for HP-UX implementation of OS Provider

    Gets information from sysconf call (using _SC_KERNEL_BITS).
   */
Boolean OperatingSystem::getOperatingSystemCapability(String& scapability)
{
    char               capability[80];
    long               ret;

    ret = sysconf (_SC_KERNEL_BITS);
    if (ret != -1)
    {
        sprintf (capability, "%ld bit", ret);
    }
    else
    {
       return false;
    }

    scapability.assign(capability);
    return true;
}

/**
    _reboot method for HP-UX implementation of OS Provider

    Finds executable in /sbin, /usr/bin, or /usr/local/sbin and invokes.
    Currently disabled (as we don't want folks rebooting systems yet)

    Invokes as via system system call, so have full checking of user's
    authorization (already authenticated by CIMOM)
   */
Uint32 OperatingSystem::_reboot()
{
    const char *reboot[] = { "reboot", NULL };
    const char *paths[] = { "/sbin", "/usr/sbin", "/usr/local/sbin", NULL };
    struct stat sbuf;
    String fname;
    CString p;
    Uint32 result;

    // ATTN-SLC-P2-17-Apr-02: this method not invoked for HP-UX (run as root)
    result = 1;
    for (int ii = 0; paths[ii] != NULL; ii++)
    {
        for (int jj = 0; reboot[jj]; jj++)
        {
            fname = paths[ii];
            fname.append("/");
            fname.append(reboot[jj]);
            p = fname.getCString();
            if (stat(p, &sbuf) == 0 && (sbuf.st_mode & S_IXUSR))
            {
                result = 2;
                if (system(p) == 0)
                    result = 0;

                return result;
            }
        }
    }
    return result;
}

Uint32 OperatingSystem::_shutdown()
{
    const char *poweroff[] = { "poweroff", NULL };
    const char *paths[] = { "/sbin", "/usr/sbin", "/usr/local/sbin", NULL };
    struct stat sbuf;
    String fname;
    CString p;
    Uint32 result;

    // ATTN-SLC-P2-17-Apr-02: this method not invoked for HP-UX (run as root)

    result = 1;
    for (int ii = 0; paths[ii] != NULL; ii++)
    {
        for (int jj = 0; poweroff[jj]; jj++)
        {
            fname = paths[ii];
            fname.append("/");
            fname.append(poweroff[jj]);
            p = fname.getCString();
            if (stat(p, &sbuf) == 0 && (sbuf.st_mode & S_IXUSR))
            {
                result = 2;
                if (system(p) == 0)
                    result = 0;

                return result;
            }
        }
    }
    return result;
}

