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
///////////////////////////////////////////////////////////////////////////////


#include <Pegasus/Common/Config.h>
#include <Pegasus/Common/Time.h>
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

OperatingSystem::OperatingSystem()
{
}

OperatingSystem::~OperatingSystem()
{
}


/**
   getName method of the Solaris implementation for the OS Provider

   Calls uname() to get the operating system name.

  */
Boolean OperatingSystem::getName(String& osName)
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

/**
   getUtilGetHostName method for the Solaris implementation of the OS Provider

   Gets the name of the host system from gethostname and gethostbyname.

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

     he=gethostbyname(hostName);
     if (he)
     {
         strncpy(hostName, he->h_name, sizeof(hostName)-1);
     }

     csName.assign(hostName);

     return true;
}

Boolean OperatingSystem::getCSName(String& csName)
{
    return getUtilGetHostName(csName);
}

/**
   getCaption method for Solaris implementation of OS Provider

   Uses uname system call and extracts information for the Caption.
  */
Boolean OperatingSystem::getCaption(String& caption)
{

   struct utsname     unameInfo;

   // Call uname and check for any errors.
   if (uname(&unameInfo) < 0)
   {
       return false;
   }

   // append in caption the information available from uname system call.
   //     system name, release, version, machine and nodename.
   caption.assign(unameInfo.sysname);
   caption.append(" ");
   caption.append(unameInfo.release);
   // caption.append(" ");
   // caption.append(unameInfo.version);
   // caption.append(" ");
   // caption.append(unameInfo.machine);
   // caption.append(" ");
   // caption.append(unameInfo.nodename);

   return true;
}

Boolean OperatingSystem::getDescription(String& description)
{

   description.assign("This instance reflects the Operating System"
        " on which the CIMOM is executing (as distinguished from instances"
        " of other installed operating systems that could be run).");

   return true;
}

/**
   getInstallDate method for Solaris implementation of OS provider

  */
Boolean OperatingSystem::getInstallDate(CIMDateTime& installDate)
{
// ATTN: Need to determine how to get this on SunOS

   return false;
}

/**
   getStatus method for Solaris implementation of OS provider

    */
Boolean OperatingSystem::getStatus(String& status)
{

// ATTN: Need to determine how to get this on SunOS

   status.assign("Unknown");

   return true;
}

/**
   getVersion method for Solaris implementation of OS provider

   Uses uname system call and extracts the release and version
   information (separated by a space).

   Returns FALSE if uname call results in errors.
   */
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
    // SunOS is the OS component of the packaged
    // system called Solaris so return SunOS
    // osType = Solaris;
    osType = SunOS;
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

    sprintf(version, "%s %s", unameInfo.release, unameInfo.version);
    otherTypeDescription.assign(version);

    return true;
}


/**
   getLastBootUpTime method for Solaris implementation of OS Provider

   gets information from /proc/uptime file
  */

Boolean OperatingSystem::getLastBootUpTime(CIMDateTime& lastBootUpTime)
{
// ATTN: need to determine how to get this on SunOS

return false;
}

/**
   getLocalDateTime method for Solaris implementation of OS Provider

   Currently calls time to get local time, should be changed to use
   the CIMOM date time and be consistent across all time properties
   (e.g., LastBootUpTime + SystemUpTime = LocalDateTime)
  */

Boolean OperatingSystem::getLocalDateTime(CIMDateTime& localDateTime)
{
   // Get the date and time from the system.
   localDateTime = CIMDateTime::getCurrentDateTime();
   return true;
}

/**
   getCurrentTimeZone method for Solaris implementation of OS Provider

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
   getNumberOfLicensedUsers method for Solaris implementation of OS Provider

   Always returns 0 for unlimited
  */
Boolean OperatingSystem::getNumberOfLicensedUsers(Uint32& numberOfLicensedUsers)
{
// ATTN: need todetrmine how to get this on SunOS

   // According to the MOF, if it's unlimited, use zero
   numberOfLicensedUsers = 0;
   return true;
}

/**
   getNumberOfUsers method for Solaris implementation of OS Provider

   Goes through the utents, counting the number of type USER_PROCESS
  */

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

/**
   getNumberOfProcesses method for Solaris implementation of OS Provider

   Counts the number of sub-directories of /proc that are of the format
   to represent processes.
  */
Boolean OperatingSystem::getNumberOfProcesses(Uint32& numberOfProcesses)
{
// ATTN: ned to determine how to get this on SunOS

   Uint32 count;
   DIR *procdir;
   regex_t process_pattern_compiled;
   const char process_pattern[] = "^[1-9][0-9]*$";

   char buffer[sizeof(dirent) + MAXNAMELEN + 1];
   struct dirent* entry = (struct dirent*)buffer;
   struct dirent* result = 0;

   count = 0;
   if ((procdir = opendir("/proc")))
   {
      if (regcomp(&process_pattern_compiled, process_pattern, 0) == 0)
      {
         while (readdir_r(procdir, entry, &result) == 0 && result != NULL)
         {
#if defined (PEGASUS_PLATFORM_LINUX_GENERIC_GNU)
            if (entry->d_type != DT_DIR)
               continue;
#endif
            if (regexec(&process_pattern_compiled, entry->d_name,
                        0, NULL, 0) == 0)
               count++;
         }
         regfree(&process_pattern_compiled);
      }
      closedir(procdir);
   }

   numberOfProcesses = count;
   return true;
}

/**
   getMaxNumberOfProcesses method for Solaris implementation of OS Provider

   gets information from /proc/sys/kernel/threads-max
  */

Boolean OperatingSystem::getMaxNumberOfProcesses(Uint32& mMaxProcesses)
{
// ATTN: Need to determine how to get this on Solaris

  return false;
}

/**
   getTotalSwapSpaceSize method for Soalris implementation of OS Provider

  */
Boolean OperatingSystem::getTotalSwapSpaceSize(Uint64& mTotalSwapSpaceSize)
{
// ATTN: Need to determine ow to get this on SunOS

  return false;
}

/** _totalVM method for Solaris implementation of OS Provider

    Calculates TotalVirtualMemory as the sum of totalSwap
    and totalMem.
*/
Uint64 OperatingSystem::_totalVM()
{
  Uint64 total;
  Uint64 tmp;

  total = 0;
  if( getTotalSwapSpaceSize(tmp) )
  {
    total += tmp;
  }
  if( getTotalVisibleMemorySize(tmp))
  {
    total += tmp;
  }
  return total;
}

/**
   getTotalVirtualMemorySize method for Soalris implementation of OS Provider

   Gets information from SwapTotal in /proc/meminfo
  */
Boolean OperatingSystem::getTotalVirtualMemorySize(Uint64& total)
{
    total = _totalVM();
    if (total) return true;
    else return false;   // possible that we had trouble with file
}

/**
   getFreeVirtualMemorySize method for Solaris implementation of OS Provider

   Gets information from SwapFree in /proc/meminfo
  */
Boolean OperatingSystem::getFreeVirtualMemory(Uint64& freeVirtualMemory)
{
// ATTN: Need to determine how to get this on SunOS

  return false;
}

/**
   getFreePhysicalMemory method for Solaris implementation of OS Provider

   Gets information from MemFree in /proc/meminfo
  */
Boolean OperatingSystem::getFreePhysicalMemory(Uint64& total)
{
// ATTN: Need to determine how to get thison SunOS

  return false;
}

/**
   getTotalVisibleMemorySize method for Solaris implementation of OS Provider

   Was returning FreePhysical - correct? diabled it.
  */
Boolean OperatingSystem::getTotalVisibleMemorySize(Uint64& memory)
{
// ATTN: Need to determin how to get this on SunOS

  return false;
}

/**
   getSizeStoredInPagingFiles method for Solaris implementation of OS Provider

   Was returning TotalSwap - correct? diabled it.
  */
Boolean OperatingSystem::getSizeStoredInPagingFiles(Uint64& total)
{
    return false;
}

/**
   getFreeSpaceInPagingFiles method for Solaris implementation of OS Provider

   Was returning TotalVirtualMemory - correct? diabled it.
  */
Boolean OperatingSystem::getFreeSpaceInPagingFiles(
                                              Uint64& freeSpaceInPagingFiles)
{
    return false;
}
/**
   getMaxProcessMemorySize method for Solaris implementation of OS Provider

   Gets information from /proc/sys/vm/overcommit_memoryt or returns
   TotalVirtualMemory
   */
Boolean OperatingSystem::getMaxProcessMemorySize(Uint64& maxProcessMemorySize)
{
// ATTN: Need to determine how to get this for SunOS

  return false;
}

 /**
   getDistributed method for Solaris implementation of OS Provider

   Always sets the distributed boolean to FALSE
  */
Boolean OperatingSystem::getDistributed(Boolean& distributed)
{
    distributed = false;
    return true;
}

/**
   getMaxProcsPerUser method for Solaris implementation of OS Provider

   Retrieves the _SC_CHILD_MAX value from sysconf.
  */
Boolean OperatingSystem::getMaxProcsPerUser (Uint32& maxProcsPerUser)
{
// ATTN: Need to determine how to get this on SunOS

    return false;
}

/**
   getSystemUpTime method for Solaris implementation of OS Provider

   Gets information from /proc/uptime (already in seconds).
  */
Boolean OperatingSystem::getSystemUpTime(Uint64& mUpTime)
{
// ATTN: Need to determine how to get this on SunOS

   return false;
}

/**
   getOperatingSystemCapability handles a Pegasus extension of the DMTF defined
   CIM_Operating System. This attribute is defined as a string either "64 bit"
   or "32 bit". On the Solaris side we will determine that by measuring
   the number
   of bytes allocated for pointers because this implementation will change
   based on the underlying processor architecture. 32-bit 64-bit... 128-bit
  */

Boolean OperatingSystem::getOperatingSystemCapability(String& scapability)
{
    char capability[80];
    void *ptr;
    int ptr_bits;

    ptr_bits = 8*sizeof(ptr);

    sprintf (capability, "%d bit", ptr_bits);

    scapability.assign(capability);
    return true;
}

/**
   _reboot method for Solaris implementation of OS Provider

   Finds executable in /sbin, /usr/bin, or /usr/local/sbin and invokes.

   Invokes as via system system call, so have full checking of user's
   authorization (already authenticated by CIMOM)

   Don't we want to do some additional cleanup before actually
   invoking the reboot command?  For example, we know the CIMOM is up
   and running.  Perhaps set the OS state to 'Stopping' and do a
   graceful shutdown of the CIMOM (at least)?

   */
Uint32 OperatingSystem::_reboot()
{
/*
******************************************************
ATTN: At the time of implementation the Provider Manager will not call
this function. The code is defined out becasue it is not clear that all
users would want this supportted. If it were to be supported then at a
minimum the user must be athenticated or the code should run switch and
run on behalf of the caller rather than as the CIMOM and then let the
system validate the users credentials and permissions to determine if
the calling user has the authority to invoke a shutdown or a reboot.
******************************************************
*/

   return false;

#ifdef NOTDEF
   const char *reboot[] = { "reboot", NULL };
   const char *paths[] = { "/sbin", "/usr/sbin", "/usr/local/sbin", NULL };
   struct stat sbuf;
   String fname;
   CString p;
   Uint32 result;

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
#endif // NOTDEF
}
/**
   _shutdown method for Solaris implementation of OS Provider

   Finds executable in /sbin, /usr/bin, or /usr/local/sbin and invokes.

   Don't we want to add some more cleanup - especially since we know
   the CIMOM is running - this could cause things to be set into a
   'Stopping' state while the OS cleans up before actually invoking
   the poweroff command.
   */
Uint32 OperatingSystem::_shutdown()
{
/*
******************************************************
ATTN: At the time of implementation the Provider Manager will not call
this function. The code is defined out becasue it is not clear that all
users would want this supportted. If it were to be supported then at a
minimum the user must be athenticated or the code should run switch and
run on behalf of the caller rather than as the CIMOM and then let the
system validate the users credentials and permissions to determine if
the calling user has the authority to invoke a shutdown or a reboot.
******************************************************
*/

   return false;

#ifdef NOTDEF
   const char *poweroff[] = { "poweroff", NULL };
   const char *paths[] = { "/sbin", "/usr/sbin", "/usr/local/sbin", NULL };
   struct stat sbuf;
   String fname;
   CString p;
   Uint32 result;

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
#endif //NOTDEF
}

