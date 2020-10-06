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

//
//  PTR 73-51-32
//  Modified getNumberOfUsers() function to return the
//  number of interactive user names for number of users
//  property
//
//  PTR 73-51-30 and PTR 73-51-2
//  Changes made to address review comments in PTR 73-51-2
//  Replacing sys$specific:[000000] with wbem_tmp: logical
//  as first argument to tempnam() in
//  in getNumberOfLicensedUsers() and getInstallDate() functions.
//
//  PTR 73-51-27
//  Changed getNumberOfLicensedUsers() to return number of licensed users
//  as 0 (unlimited licensed users) in case of I64 as the base O/S license on
//  IA64 (FOE) provides unlimited number of users
//
//  PTR 73-51-22 21-Jul-2006
//  Changed the getTotalSwapSpaceSize(),getFreeSpaceInPagingFiles(),
//  getSizeStoredInPagingFiles(), getTotalVisibleMemorySize() and
//  getFreePhysicalMemory() functions to get teh pagesize using the
//  sys$getsyiw() call using item code SYI$_PAGE_SIZE
//
//  PTR 73-51-28 12-Jul-2006
//  changed obtaining of TotalVirtualMemorySize from current process PGFLQUOTA
//  to the sum of TotalVisibleMemorySize and SizeStoredInPagingFiles.
//  changed obtaining of FreeVirtualMemory from current process PAGFILCNT
//  to the sum of FreePhysicalMemory and FreeSpaceInPagingFiles
//
//  PTR 73-51-19 and PTR 73-51-20 11-Jul-2006
//  The installDate and NumberOfLicensedUsers are not displayed in the wbemexec
//  enumerate instance output. The same was seen as appearing fine, when
//  CIMserver is running interactively.
//  The function getInstallDate() and getNumberOfLicensedUsers() is modified
//  as below:
//  The first argument of tempname() call is changed from NULL to
//  "SYS$SPECIFIC:[000000]"
//  as tempnam() would take a default of "SYS$SCRATCH:" directory, if first
//  argument is not specified.
//
//  PTR 73-51-21 10-Jul-06.
//  changed The checking for define PEGASUS_PLATFORM_VMS_IPF_DECCXX
//  to PEGASUS_PLATFORM_VMS_IA64_DECCXX, as the later was being used
//  in the /define directive during compilation.
//  The FreePhysicalMemory is retrived properly, after changing this define.
//
//  PTR 73-51-2 28-Jun-06.
//  Made changes to get the coreect OS verison.
//  Removed the display of uname.release information from getversion()
//
//  PTR 73-51-15. Removed include of <pegasus/common/system.h>
//  and <pegasus/common/logger.h> as it was giving compiler warning
//
//%////////////////////////////////////////////////////////////////////////////


#include <Pegasus/Common/Config.h>
#include "OperatingSystem.h"

#include <time.h>
#include <timers.h>
#include <dirent.h>
#include <utsname.h>
#include <netdb.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <descrip.h>
#include <syidef.h>
#include <jpidef.h>
#include <pscandef.h>
#include <lib$routines.h>
#include <starlet.h>
#include <stsdef.h>
#include <ssdef.h>
#include <libdtdef.h>
#include <lnmdef.h>
#include <pthread.h>
#include <tis.h>
#include <platforms/vms/vms_utility_routines.h>
#include <Pegasus/Common/Mutex.h>
#include <Pegasus/Common/system.h>

// Verify that /main=posix_exit is set. This changes the return status values
// of RTL calls such as system().
#ifndef _POSIX_EXIT
#error "The compile switch /main=posix_exit is required."
#endif

#define MAXHOSTNAMELEN  256
#define MAXUSERNAME 512

extern "C"
{
#if defined (PEGASUS_PLATFORM_VMS_ALPHA_DECCXX)
    extern const long SCH$GL_FREECNT;
#elif defined (PEGASUS_PLATFORM_VMS_IA64_DECCXX)
    extern const long SCH$GI_FREECNT;
#endif
}

PEGASUS_USING_STD;

OperatingSystem::OperatingSystem()
{
}

OperatingSystem::~OperatingSystem()
{
}

//
// =============================================================================
// NAME              : getUtilGetHostName
// DESCRIPTION       : Gets the name of the host system from gethostname
//                      and gethostbyname.
// ASSUMPTIONS       : None
// PRE-CONDITIONS    :
// POST-CONDITIONS   :
// NOTES             :
// =============================================================================
//

static Boolean getUtilGetHostName(String & csName)
{
    static String _csName;
    char hostName[PEGASUS_MAXHOSTNAMELEN + 1];
    struct addrinfo *info, hints;
    int rc;

    if (0 == _csName.size())
    {
        strcpy(hostName, System::getHostName().getCString());
        if (!strlen(hostName))
        {
            return false;
        }
        hostName[sizeof(hostName)-1] = 0;


        // Now get the official hostname.  If this call fails then return
        // the value from gethostname().
        // Note: gethostbyname() is not reentrant and VMS does not
        // have gethostbyname_r() so use getaddrinfo().

        info = 0;
        memset (&hints, 0, sizeof(struct addrinfo));
        hints.ai_flags = AI_CANONNAME;
        hints.ai_family = AF_INET;
        hints.ai_protocol = IPPROTO_TCP;
        hints.ai_socktype = SOCK_STREAM;

        rc = System::getAddrInfo(hostName, 0, &hints, &info);
        if ((!rc) && (info) && (info->ai_canonname))
        {
            _csName.assign(info->ai_canonname);
        }
        else {
            _csName.assign(hostName);
        }

        if (info)
        {
            freeaddrinfo(info);
        }
    }

    csName = _csName;

    return true;
}

//
// =============================================================================
// NAME              : convertToCIMDateString
// DESCRIPTION       : Converts a tm struct to a CIMDateTime formatted
//                      char *.
// ASSUMPTIONS       : None
// PRE-CONDITIONS    :
// POST-CONDITIONS   :
// NOTES             :
// =============================================================================
//

int convertToCIMDateString(struct tm *t, char *time)
{
    // Format the date.
    sprintf(time, "%04d%02d%02d%02d%02d%02d.000000%c%03d",
        t->tm_year + 1900,
        t->tm_mon + 1,
        t->tm_mday,
        t->tm_hour,
        t->tm_min,
        t->tm_sec,
        (timezone > 0) ? '-' : '+',
        abs(timezone / 60 - (t->tm_isdst ? 60 : 0)));

    return 1;
}

//
// =============================================================================
// NAME              : get_time
// DESCRIPTION       : Returns a tm structure, which contains the current time.
// ASSUMPTIONS       : None
// PRE-CONDITIONS    :
// POST-CONDITIONS   :
// NOTES             :
// =============================================================================
//

struct tm *get_time()
{
    struct tm *local;
    struct timespec since_epoch;

    if (getclock(TIMEOFDAY, &since_epoch) != 0)
    {
        return (NULL);   /*getclock error */
    }
    else
    {
        if ((local = localtime(&since_epoch.tv_sec)) == (struct tm *) NULL)
        {
            return (NULL);   /*localtime error */
        }
        else
        {
            return local;
        }
    }
}

//
// =============================================================================
// NAME              : GetFreeMem
// DESCRIPTION       : kernel routine - link/sysexe to pick up SCH$GL_FREECNT
// ASSUMPTIONS       : None
// PRE-CONDITIONS    :
// POST-CONDITIONS   :
// NOTES             :
// =============================================================================
//

int GetFreeMem(long *pFreeMem)
{
#if defined (PEGASUS_PLATFORM_VMS_ALPHA_DECCXX)
    *pFreeMem = SCH$GL_FREECNT;
#elif defined (PEGASUS_PLATFORM_VMS_IA64_DECCXX)
    *pFreeMem = SCH$GI_FREECNT;
#endif

    return (SS$_NORMAL);
}

//
// =============================================================================
// NAME              : getCSName
// DESCRIPTION       :
// ASSUMPTIONS       : None
// PRE-CONDITIONS    :
// POST-CONDITIONS   :
// NOTES             :
// =============================================================================
//

Boolean OperatingSystem::getCSName(String & csName)
{
    return getUtilGetHostName(csName);
}

//
// =============================================================================
// NAME              : getName
// DESCRIPTION       : Calls uname() to get the operating system name.
// ASSUMPTIONS       : None
// PRE-CONDITIONS    :
// POST-CONDITIONS   :
// NOTES             :
// =============================================================================
//

Boolean OperatingSystem::getName(String & osName)
{
    struct utsname unameInfo;

    // Call uname and check for any errors.
    if (uname(&unameInfo) < 0)
    {
        return false;
    }

    osName.assign(unameInfo.sysname);

    return true;
}

//
// =============================================================================
// NAME              : getCaption
// DESCRIPTION       : Return a string constant for the caption.
// ASSUMPTIONS       : None
// PRE-CONDITIONS    :
// POST-CONDITIONS   :
// NOTES             :
// =============================================================================
//

Boolean OperatingSystem::getCaption(String & caption)
{
    struct utsname unameInfo;
    char version[sizeof (unameInfo.version)];

    // Call uname and check for any errors.
    if (uname(&unameInfo) < 0)
    {
        return false;
    }

    sprintf(version, " %s", unameInfo.version);
    caption.assign(unameInfo.sysname);
    caption.append (version);
    return true;
}

//
// =============================================================================
// NAME              : getDescription
// DESCRIPTION       : Return a string constant for the description.
// ASSUMPTIONS       : None
// PRE-CONDITIONS    :
// POST-CONDITIONS   :
// NOTES             :
// =============================================================================
//

Boolean OperatingSystem::getDescription(String & description)
{
    description.assign("This instance reflects the Operating System"
        " on which the CIMOM is executing (as distinguished from instances"
        " of other installed operating systems that could be run).");

    return true;
}

//
// =============================================================================
// NAME              : getInstallDate
// DESCRIPTION       : Get the date that the OS was installed. Requires
//                      VMS SYSLCK priviledge.
// ASSUMPTIONS       : None
// PRE-CONDITIONS    :
// POST-CONDITIONS   :
// NOTES             :
// =============================================================================
//

Boolean OperatingSystem::getInstallDate(CIMDateTime & installDate)
{
    Boolean bStatus;
    int status;
    int istr;
    char record1[512];
    char *rptr1 = 0;
    char  HistFileBuffer[512];
    pthread_t thread_num;
    char  cmd[512];
    FILE *fptr1 = 0;
    unsigned __int64 bintime = 0;
    unsigned short int timbuf[7];
    unsigned short val = 0;
    char cimtime[80] = "";
    struct tm timetm;
    struct tm *ptimetm = &timetm;
    time_t tme = 0;
    time_t tme1 = 0;
    char t_string[24] = "";
    char libdst;
    unsigned int retlen;
    unsigned long libop;
    unsigned long libdayweek;
    unsigned long libdayear;
    long dst_desc[2];
    char log_string[] = "SYS$TIMEZONE_DAYLIGHT_SAVING";
    struct dsc$descriptor_s sysinfo;
    static $DESCRIPTOR(lnm_tbl, "LNM$SYSTEM");
    struct
    {
        unsigned short wLength;
        unsigned short wCode;
        void *pBuffer;
        unsigned int *pRetLen;
        int term;
    } item_list;

    // A temp file is used to avoid a filename collision when two users
    // call OsInfo at the same time.
    thread_num = tis_self();
    sprintf(HistFileBuffer, "%s%s%x%s", "wbem_tmp:", "oshist_",
            thread_num->sequence, ".out");

    sysinfo.dsc$b_dtype = DSC$K_DTYPE_T;
    sysinfo.dsc$b_class = DSC$K_CLASS_S;
    sysinfo.dsc$w_length = sizeof (t_string);
    sysinfo.dsc$a_pointer = t_string;


    // "pipe product show history openvms |
    //       search/nolog/nowarn/out=history.out sys$input install");
    strcpy(cmd, "pipe product show history openvms | search/nolog/nowarn/out=");
    strcat(cmd, HistFileBuffer);
    strcat(cmd, " sys$input install");

// Note: The format of this string has changed from 8.2 to 8.3. Now it has
//       just the date, no time.
// OpenVMS 8.2
// $ product sho history openvms
// -------------------------------- ----------- ----------- --------------------
// PRODUCT                          KIT TYPE    OPERATION   DATE AND TIME
// -------------------------------- ----------- ----------- --------------------
// HP I64VMS OPENVMS V8.2           Platform    Install     25-JAN-2005 10:26:24
// HP I64VMS OPENVMS X8.2-ALQ       Platform    Remove      25-JAN-2005 10:26:24
//
// VI::_TNA37:> product sho history openvms
// ------------------------------------ ----------- ----------- --- -----------
// PRODUCT                              KIT TYPE    OPERATION   VAL DATE
// ------------------------------------ ----------- ----------- --- -----------
// HP I64VMS OPENVMS X8.3-B49           Platform    Install     (U) 09-JAN-2006
// HP I64VMS OPENVMS X8.3-B3K           Platform    Remove       -  09-JAN-2006
//


    status = system(cmd);
    if (!WIFEXITED(status))
    {
        bStatus = false;
        perror("system()");
        goto done;
    }

    fptr1 = fopen(HistFileBuffer, "r");
    if (fptr1)
    {
        while (fgets(record1, sizeof (record1), fptr1))
        {
            for (istr = 0; istr <= (sizeof (record1) - 4); istr++)
            {
                if ((rptr1 = strstr(record1 + istr, "-")) &&
                    !strncmp(rptr1 + 4, "-", 1))
                {
                    break;
                }
                rptr1 = 0;
            }
            if (rptr1)
            {
                time(&tme);
                tme1 = mktime(ptimetm);        /* get timezone */
                strcpy(t_string, rptr1 - 2);
                if (t_string[11] == 10)
                {
                    // a <cr>.
                    // When the date; but not the time is provided,
                    // fill in zeros.
                    t_string[11] = ' ';
                    t_string[12] = '0';
                    t_string[13] = '0';
                    t_string[14] = ':';
                    t_string[15] = '0';
                    t_string[16] = '0';
                    t_string[17] = ':';
                    t_string[18] = '0';
                    t_string[19] = '0';
                }
                t_string[20] = '.';
                t_string[21] = '0';
                t_string[22] = '0';
                t_string[23] = '0';
                status = sys$bintim(&sysinfo, &bintime);
                if (!$VMS_STATUS_SUCCESS(status))
                {
                    bStatus = false;
                    goto done;
                }

                libop = LIB$K_DAY_OF_WEEK;
                status = lib$cvt_from_internal_time(&libop, &libdayweek,
                                                    &bintime);
                if (!$VMS_STATUS_SUCCESS(status))
                {
                    bStatus = false;
                    goto done;
                }

                libop = LIB$K_DAY_OF_YEAR;
                status = lib$cvt_from_internal_time(&libop, &libdayear,
                                                    &bintime);
                if (!$VMS_STATUS_SUCCESS(status))
                {
                    bStatus = false;
                    goto done;
                }

                dst_desc[0] = strlen(log_string);
                dst_desc[1] = (long) log_string;
                item_list.wLength = 1;
                item_list.wCode = LNM$_STRING;
                item_list.pBuffer = &libdst;
                item_list.pRetLen = &retlen;
                item_list.term = 0;

                status = sys$trnlnm(0, &lnm_tbl, &dst_desc, 0, &item_list);
                if (!$VMS_STATUS_SUCCESS(status))
                {
                    bStatus = false;
                    goto done;
                }

                status = sys$numtim(timbuf, &bintime);
                if (!$VMS_STATUS_SUCCESS(status))
                {
                    bStatus = false;
                    goto done;
                }

                timetm.tm_sec = timbuf[5];
                timetm.tm_min = timbuf[4];
                timetm.tm_hour = timbuf[3];
                timetm.tm_mday = timbuf[2];
                timetm.tm_mon = timbuf[1] - 1;
                timetm.tm_year = timbuf[0] - 1900;
                timetm.tm_wday = libdayweek - 1;
                timetm.tm_yday = libdayear - 1;
                if (libdst != 48)
                {
                    timetm.tm_isdst = 1;
                }
                status = convertToCIMDateString(ptimetm, cimtime);
                if (!$VMS_STATUS_SUCCESS(status))
                {
                    bStatus = false;
                    goto done;
                }

                installDate.clear();
                installDate.set(cimtime);

                bStatus = true;
                goto done;
            }    // end if (rptr1 = strstr(record1,"Install"))
        }
        bStatus = false;
        goto done;
    }    // end if (fptr1 = fopen(HistFileBuffer, "r"))
    else
    {
        perror("fopen oshist_");
        bStatus = false;
        goto done;
    }

done:

    if (fptr1)
    {
        fclose(fptr1);
        fptr1 = 0;
    }

    remove(HistFileBuffer);

    return bStatus;
}

//
// =============================================================================
// NAME              : getStatus
// DESCRIPTION       : Since the OS is up and running, we'll return unknown
//                     here.
// ASSUMPTIONS       : None
// PRE-CONDITIONS    :
// POST-CONDITIONS   :
// NOTES             :
// =============================================================================
//

Boolean OperatingSystem::getStatus(String & status)
{
    status.assign("Unknown");
    return true;
}

//
// =============================================================================
// NAME              : getVersion
// DESCRIPTION       : Uses uname system call to extract the release and
//                     version.
// ASSUMPTIONS       : None
// PRE-CONDITIONS    :
// POST-CONDITIONS   :
// NOTES             :
// =============================================================================
//
Boolean OperatingSystem::getVersion(String & osVersion)
{
    struct utsname unameInfo;
    // Changed to fix PTR -73-51-2
    char version[sizeof (unameInfo.version)];

    // Call uname and check for any errors.
    if (uname(&unameInfo) < 0)
    {
        return false;
    }

    // Changed to fix PTR -73-51-2
    sprintf(version, "%s", unameInfo.version);
    osVersion.assign(version);

    return true;
}

//
// =============================================================================
// NAME              : getOSType
// DESCRIPTION       : Return OSType value for OpenVms.
// ASSUMPTIONS       : None
// PRE-CONDITIONS    :
// POST-CONDITIONS   :
// NOTES             :
// =============================================================================
//

Boolean OperatingSystem::getOSType(Uint16 & osType)
{
    osType = OpenVMS;
    return true;
}

//
// =============================================================================
// NAME              : getOtherTypeDescription
// DESCRIPTION       : Return NULL since we have an OSType.
// ASSUMPTIONS       : None
// PRE-CONDITIONS    :
// POST-CONDITIONS   :
// NOTES             :
// =============================================================================
//

Boolean OperatingSystem::getOtherTypeDescription(String & otherTypeDescription)
{
    otherTypeDescription = String::EMPTY;
    return true;
}

//
// =============================================================================
// NAME              : getLastBootUpTime
// DESCRIPTION       : Get the time the OS was last booted.
// ASSUMPTIONS       : None
// PRE-CONDITIONS    :
// POST-CONDITIONS   :
// NOTES             :
// =============================================================================
//

Boolean OperatingSystem::getLastBootUpTime(CIMDateTime & lastBootUpTime)
{
    long status;
    long item = SYI$_BOOTTIME;
    long dst_desc[2];
    char t_string[24] = "";
    char cimtime[80] = "";
    char log_string[] = "SYS$TIMEZONE_DAYLIGHT_SAVING";
    char libdst;
    time_t tme = 0;
    time_t tme1 = 0;
    unsigned __int64 bintime = 0;
    unsigned short int timbuf[7];
    unsigned short val = 0;
    unsigned long libop;
    unsigned long libdayweek;
    unsigned long libdayear;
    unsigned int retlen;
    struct tm timetm;
    struct tm *ptimetm = &timetm;
    struct dsc$descriptor_s sysinfo;
    static $DESCRIPTOR(lnm_tbl, "LNM$SYSTEM");
    struct
    {
        unsigned short wLength;
        unsigned short wCode;
        void *pBuffer;
        unsigned int *pRetLen;
        int term;
    } item_list;

    sysinfo.dsc$b_dtype = DSC$K_DTYPE_T;
    sysinfo.dsc$b_class = DSC$K_CLASS_S;
    sysinfo.dsc$w_length = sizeof (t_string);
    sysinfo.dsc$a_pointer = t_string;

    status = lib$getsyi(&item, 0, &sysinfo, &val, 0, 0);
    if (!$VMS_STATUS_SUCCESS(status))
    {
        return false;
    }

    status = sys$bintim(&sysinfo, &bintime);
    if (!$VMS_STATUS_SUCCESS(status))
    {
        return false;
    }

    libop = LIB$K_DAY_OF_WEEK;
    status = lib$cvt_from_internal_time(&libop, &libdayweek, &bintime);
    if (!$VMS_STATUS_SUCCESS(status))
    {
        return false;
    }

    libop = LIB$K_DAY_OF_YEAR;
    status = lib$cvt_from_internal_time(&libop, &libdayear, &bintime);
    if (!$VMS_STATUS_SUCCESS(status))
    {
        return false;
    }

    dst_desc[0] = strlen(log_string);
    dst_desc[1] = (long) log_string;
    item_list.wLength = 1;
    item_list.wCode = LNM$_STRING;
    item_list.pBuffer = &libdst;
    item_list.pRetLen = &retlen;
    item_list.term = 0;

    status = sys$trnlnm(0, &lnm_tbl, &dst_desc, 0, &item_list);
    if (!$VMS_STATUS_SUCCESS(status))
    {
        return false;
    }

    status = sys$numtim(timbuf, &bintime);
    if (!$VMS_STATUS_SUCCESS(status))
    {
        return false;
    }

    timetm.tm_sec = timbuf[5];
    timetm.tm_min = timbuf[4];
    timetm.tm_hour = timbuf[3];
    timetm.tm_mday = timbuf[2];
    timetm.tm_mon = timbuf[1] - 1;
    timetm.tm_year = timbuf[0] - 1900;
    timetm.tm_wday = libdayweek - 1;
    timetm.tm_yday = libdayear - 1;
    timetm.tm_isdst = 0;
    if (libdst != 48)
    {
        timetm.tm_isdst = 1;
    }

    if (convertToCIMDateString(ptimetm, cimtime) != -1)
    {
        lastBootUpTime.clear();
        lastBootUpTime.set(cimtime);
        return true;
    }
    else
    {
        return false;
    }
}

//
// =============================================================================
// NAME              : getLocalDateTime
// DESCRIPTION       : Get the local date and time.
// ASSUMPTIONS       : None
// PRE-CONDITIONS    :
// POST-CONDITIONS   :
// NOTES             :
// =============================================================================
//

Boolean OperatingSystem::getLocalDateTime(CIMDateTime & localDateTime)
{
    struct tm *local;
    char date[80];

    if ((local = get_time()) == NULL)
    {
        return false;
    }

    if (convertToCIMDateString(local, date) != -1)
    {
        localDateTime.clear();
        localDateTime.set(date);
        return true;
    }

    return false;
}

//
// =============================================================================
// NAME              : getCurrentTimeZone
// DESCRIPTION       : Get the number of minutes that OS is offset from GMT.
// ASSUMPTIONS       : None
// PRE-CONDITIONS    :
// POST-CONDITIONS   :
// NOTES             :
// =============================================================================
//

Boolean OperatingSystem::getCurrentTimeZone(Sint16 & currentTimeZone)
{
    struct tm *tz;

    if ((tz = get_time()) == NULL)
    {
        return false;
    }

    if (timezone > 0)
    {
        currentTimeZone = -(timezone / 60 - (tz->tm_isdst ? 60 : 0));
    }
    else
    {
        currentTimeZone = timezone / 60 - (tz->tm_isdst ? 60 : 0);
    }

    return true;
}

//
// =============================================================================
// NAME              : getNumberOfLicensedUsers
// DESCRIPTION       : Get the number of user licenses for the OS.
// ASSUMPTIONS       : None
// PRE-CONDITIONS    :
// POST-CONDITIONS   :
// NOTES             :
// =============================================================================
//
//PTR 73-51-27 - Changed to return 0 for number of licensed users on I64.
Boolean OperatingSystem::getNumberOfLicensedUsers(
    Uint32 & numberOfLicensedUsers)
{
    Boolean bStatus = false;
    int status;
    int loaded_units;
    int req_units;
    char record1[512];
    char record2[512];
    char *rptr1 = 0;
    char *rptr2 = 0;
    char *rptr3 = 0;
    pthread_t thread_num;
    char UsageFileBuffer[512];
    char UnitsFileBuffer[512];
    char usage_cmd[512];
    char units_cmd[512];
    FILE *fptr1 = 0;
    FILE *fptr2 = 0;


    // A temp file is used to avoid a filename collision when two users
    // call OsInfo at the same time.
    thread_num = tis_self();
    // wbem_tmp:usage_6.out
    sprintf(UsageFileBuffer, "%s%s%x%s", "wbem_tmp:", "usage_",
            thread_num->sequence, ".out");

#ifdef  PEGASUS_PLATFORM_VMS_ALPHA_DECCXX
    // wbem_tmp:units_6.out
    sprintf(UnitsFileBuffer, "%s%s%x%s", "wbem_tmp:", "units_",
          thread_num->sequence, ".out");
#endif

// Note: This code may not work on non-English versions of OpenVMS...
//
// Alpha
// $ sho lic/usage
//
// View of loaded licenses from node WALNUT             20-JAN-2006 11:57:02.12
//
// ------- Product ID --------   ---- Unit usage information ----
// Product            Producer       Loaded  Allocated  Available
// ACMS               DEC        Unlimited license, no usage information
// ADA                DEC        Unlimited license, no usage information
// ...
// OPENVMS-ALPHA      DEC        Unlimited license, no usage information
// OPENVMS-ALPHA-USER DEC        Unlimited license, no usage information
// OPENVMS-GALAXY     DEC        Unlimited license, no usage information
//
// I64
// $ sho lic/usage
//
// View of loaded licenses from node RED           20-JAN-2006 11:54:54.12
//
// ------- Product ID --------   ---- Unit usage information ----------------
// Product            Producer       Loaded  Allocated  Available  Compliance
// ACMS               HP                 50         14         36  Yes
// BASIC              HP                 50         14         36  Yes
// ...
// OPENVMS-I64-EOE    HP                 50         14         36  Yes
// OPENVMS-I64-FOE    HP                 50         14         36  Yes
// OPENVMS-I64-MCOE   HP                 50         14         36  Yes
//
    strcpy(usage_cmd, "show license/usage/nowarning_interval/out=");
    strcat(usage_cmd, UsageFileBuffer);
#ifdef PEGASUS_PLATFORM_VMS_ALPHA_DECCXX
    strcat(usage_cmd, " openvms-alpha");
#endif
#ifdef PEGASUS_PLATFORM_VMS_IA64_DECCXX
// Which of these to use for IA64?
// When a machine has more than one of these installed, this will
// find the first one.
    strcat(usage_cmd, " openvms-i64-*");
//    strcat(usage_cmd, " openvms-i64-EOE");
//    strcat(usage_cmd, " openvms-i64-FOE");
//    strcat(usage_cmd, " openvms-i64-MCOE");
#endif

    status = system(usage_cmd);
    if (!WIFEXITED(status))
    {
        bStatus = false;
        perror("system()");
        goto done;
    }

#ifdef  PEGASUS_PLATFORM_VMS_ALPHA_DECCXX
// Alpha
// $ sho lic/units/nowarn
//   VMS/LMF Charge Information for node WALNUT
//   This is a COMPAQ AlphaServer DS20E 666 MHz, hardware model type 1940
//   Type: A, Units Required: 75     (VAX/VMS Capacity or OpenVMS Unlimited or
//                                    Base)
//   Type: B, * Not Permitted *      (VAX/VMS F&A Server)
//   Type: C, * Not Permitted *      (VAX/VMS Concurrent User)
//   Type: D, * Not Permitted *      (VAX/VMS Workstation)
//   Type: E, * Not Permitted *      (VAX/VMS System Integrated Products)
//   Type: F, * Not Permitted *      (VAX Layered Products)
//   Type: G, * Not Permitted *      (Reserved)
//   Type: H, Units Required: 1050   (Alpha Layered Products)
//   Type: I, Units Required: 1050   (Layered Products)
//
// I64
// $ sho lic/units/nowarn
//   OpenVMS I64/LMF Charge Information for node RED
//   This is an HP rx4640  (1.50GHz/6.0MB), with 4 CPUs active, 4 socket(s)
//   Type: PPL,   Units Required: 4  (I64 Per Processor)
//
    strcpy(units_cmd, "show license/units/nowarning_interval/out=");
    strcat(units_cmd, UnitsFileBuffer);

    status = system(units_cmd);
    if (!WIFEXITED(status))
    {
        bStatus = false;
        perror("system()");
        goto done;
    }

#endif

    fptr1 = fopen(UsageFileBuffer, "r");
    if (fptr1)
    {
        while (fgets(record1, sizeof (record1), fptr1))
        {
#ifdef PEGASUS_PLATFORM_VMS_IA64_DECCXX
// The base O/S license on IA64 (FOE) provides unlimited number
// of users - PTR 73-51-27

            if (rptr1 = strstr(record1, "HP "))
            {
                // either  FOE,EOE or MCOE license is existing on the system
                // Return Unlimited number of users
                numberOfLicensedUsers = 0;
                bStatus = true;
                goto done;
            }
#endif

#ifdef PEGASUS_PLATFORM_VMS_ALPHA_DECCXX
            if (rptr1 = strstr(record1, "DEC "))
            {
                rptr2 = strstr(rptr1 + 3, "Unlimited license");
                if (rptr2)
                {
                    numberOfLicensedUsers = 0;
                    bStatus = true;
                    goto done;
                }
                else
                {
                    rptr2 = strtok(rptr1 + 3, " ");
                    loaded_units = strtol(rptr2, NULL, 10);

                    if (fptr2 = fopen(UnitsFileBuffer, "r"))
                    {
                        while (fgets(record2, sizeof (record2), fptr2))
                        {
                            if (rptr1 =
                                strstr(record2, "Type: A, Units Required:"))
                            {
                                rptr3 = strtok(rptr1 + 25, " ");
                                req_units = strtol(rptr3, NULL, 10);
                                if (req_units != 0)
                                {
                                    numberOfLicensedUsers =
                                        loaded_units / req_units;
                                    bStatus = true;
                                    goto done;
                                }
                                else
                                {
                                    bStatus = false;
                                    goto done;
                                }
                            }
                        }  // end while (fgets(record2, sizeof(record2), fptr2))
                        if (!fptr2 == 0)
                        {
                            fclose(fptr2);
                            fptr2 = 0;
                        }
                    }    // end if (fptr2 = fopen(UnitsFileBuffer, "r"))

                    else
                    {
                        perror("fopen units_");
                        bStatus = false;
                        goto done;
                    }
                }    // end if (rptr2)
            }    // end if (rptr1 = strstr(record1,"DEC "))

#endif  //#ifdef  PEGASUS_PLATFORM_VMS_ALPHA_DECCXX

        }    // end while (fgets(record1, sizeof(record1), fptr1))

#ifdef PEGASUS_PLATFORM_VMS_IA64_DECCXX
        // no FOE,EOE or MCOE license is existing on the system
        // Returning False
        bStatus = false;
        goto done;
#endif

    }    // end if (fptr1 = fopen(UsageFileBuffer, "r"))

done:

    if (!fptr1 == 0)
    {
        fclose(fptr1);
        fptr1 = 0;
    }
    if (!fptr2 == 0)
    {
        fclose(fptr2);
        fptr2 = 0;
    }

    remove(UsageFileBuffer);

#ifdef  PEGASUS_PLATFORM_VMS_ALPHA_DECCXX
    remove(UnitsFileBuffer);
#endif

    return bStatus;
}

//
// =============================================================================
// NAME              : getNumberOfUsers
// DESCRIPTION       : Number of user sessions.
// ASSUMPTIONS       : None
// PRE-CONDITIONS    :
// POST-CONDITIONS   :
// NOTES             :
// =============================================================================
//

//Made changes to return only the numbers of interactive users - PTR-73-51-32
Boolean OperatingSystem::getNumberOfUsers(Uint32 & numberOfUsers)
{
    int i;
    int status;
    int count = 0;
    int usernamlen;
    unsigned long int jpictx;
    typedef struct
    {
        unsigned short wlength;
        unsigned short wcode;
        void *pbuffer;
        void *pretlen;
    } item_list;

    item_list itmlst3[2];
    char usernamebuf[13];
    char *ptr1 = 0;
    char username[MAXUSERNAME * 13] = "";

    itmlst3[0].wlength = 0;
    itmlst3[0].wcode = PSCAN$_MODE;
    itmlst3[0].pbuffer = (void *) JPI$K_INTERACTIVE;
    itmlst3[0].pretlen = NULL;
    itmlst3[1].wlength = 0;
    itmlst3[1].wcode = 0;
    itmlst3[1].pbuffer = NULL;
    itmlst3[1].pretlen = NULL;

    status = sys$process_scan(&jpictx, itmlst3);
    if (!$VMS_STATUS_SUCCESS(status))
    {
        return false;
    }

    itmlst3[0].wlength = 12;
    itmlst3[0].wcode = JPI$_USERNAME;
    itmlst3[0].pbuffer = usernamebuf;
    itmlst3[0].pretlen = &usernamlen;
    itmlst3[1].wlength = 0;
    itmlst3[1].wcode = 0;
    itmlst3[1].pbuffer = NULL;
    itmlst3[1].pretlen = NULL;

    while (1)
    {
        status = sys$getjpiw(0, &jpictx, NULL, itmlst3, 0, NULL, 0);
        if (status == SS$_NOMOREPROC)
        {
            break;
        }
        if (!$VMS_STATUS_SUCCESS(status))
        {
          return false;
        }
        usernamebuf[12] = '\0';
        for (i = 0; i < MAXUSERNAME; i++)
        {
            ptr1 = &username[i * 13];
            if (!strncmp(usernamebuf, ptr1, 13))
            {
                break;
            }
            else
            {
                if (!strcmp(ptr1, ""))
                {
                    strncpy(ptr1, usernamebuf, 13);
                    count++;
                    break;
                }
            }
        }
    }
    numberOfUsers = count;
    return true;
}

//
// =============================================================================
// NAME              : getNumberOfProcesses
// DESCRIPTION       : Number of process contexts currently loaded or running.
// ASSUMPTIONS       : None
// PRE-CONDITIONS    :
// POST-CONDITIONS   :
// NOTES             : VMS WORLD priviledge is needed for this routine.
// =============================================================================
//

Boolean OperatingSystem::getNumberOfProcesses(Uint32 & numberOfProcesses)
{
    int status;
    int count = 0;
    int usernamlen;
    unsigned long int jpictx;
    typedef struct
    {
        unsigned short wlength;
        unsigned short wcode;
        void *pbuffer;
        void *pretlen;
    } item_list;

    item_list itmlst3[2];
    char usernamebuf[13];

    itmlst3[0].wlength = 0;
    itmlst3[0].wcode = PSCAN$_MODE;
    itmlst3[0].pbuffer = (void *) JPI$K_INTERACTIVE;
    itmlst3[0].pretlen = NULL;
    itmlst3[1].wlength = 0;
    itmlst3[1].wcode = 0;
    itmlst3[1].pbuffer = NULL;
    itmlst3[1].pretlen = NULL;

    status = sys$process_scan(&jpictx, itmlst3);
    if (!$VMS_STATUS_SUCCESS(status))
    {
        return false;
    }

    itmlst3[0].wlength = 12;
    itmlst3[0].wcode = JPI$_USERNAME;
    itmlst3[0].pbuffer = usernamebuf;
    itmlst3[0].pretlen = &usernamlen;
    itmlst3[1].wlength = 0;
    itmlst3[1].wcode = 0;
    itmlst3[1].pbuffer = NULL;
    itmlst3[1].pretlen = NULL;

    while (1)
    {
        status = sys$getjpiw(0, &jpictx, NULL, itmlst3, 0, NULL, 0);
        if (status == SS$_NOMOREPROC)
        {
            break;
        }
        if (!$VMS_STATUS_SUCCESS(status))
        {
            return false;
        }
        count++;
    }
    itmlst3[0].wlength = 0;
    itmlst3[0].wcode = PSCAN$_MODE;
    itmlst3[0].pbuffer = (void *) JPI$K_OTHER;
    itmlst3[0].pretlen = NULL;
    itmlst3[1].wlength = 0;
    itmlst3[1].wcode = 0;
    itmlst3[1].pbuffer = NULL;
    itmlst3[1].pretlen = NULL;

    status = sys$process_scan(&jpictx, itmlst3);
    if (!$VMS_STATUS_SUCCESS(status))
    {
        return false;
    }

    itmlst3[0].wlength = 12;
    itmlst3[0].wcode = JPI$_USERNAME;
    itmlst3[0].pbuffer = usernamebuf;
    itmlst3[0].pretlen = &usernamlen;
    itmlst3[1].wlength = 0;
    itmlst3[1].wcode = 0;
    itmlst3[1].pbuffer = NULL;
    itmlst3[1].pretlen = NULL;

    while (1)
    {
        status = sys$getjpiw(0, &jpictx, NULL, itmlst3, 0, NULL, 0);
        if (status == SS$_NOMOREPROC)
        {
            break;
        }
        if (!$VMS_STATUS_SUCCESS(status))
        {
            return false;
        }
        count++;
    }
    itmlst3[0].wlength = 0;
    itmlst3[0].wcode = PSCAN$_MODE;
    itmlst3[0].pbuffer = (void *) JPI$K_BATCH;
    itmlst3[0].pretlen = NULL;
    itmlst3[1].wlength = 0;
    itmlst3[1].wcode = 0;
    itmlst3[1].pbuffer = NULL;
    itmlst3[1].pretlen = NULL;

    status = sys$process_scan(&jpictx, itmlst3);
    if (!$VMS_STATUS_SUCCESS(status))
    {
        return false;
    }

    itmlst3[0].wlength = 12;
    itmlst3[0].wcode = JPI$_USERNAME;
    itmlst3[0].pbuffer = usernamebuf;
    itmlst3[0].pretlen = &usernamlen;
    itmlst3[1].wlength = 0;
    itmlst3[1].wcode = 0;
    itmlst3[1].pbuffer = NULL;
    itmlst3[1].pretlen = NULL;

    while (1)
    {
        status = sys$getjpiw(0, &jpictx, NULL, itmlst3, 0, NULL, 0);
        if (status == SS$_NOMOREPROC)
        {
            break;
        }
        if (!$VMS_STATUS_SUCCESS(status))
        {
            return false;
        }
        count++;
    }
    itmlst3[0].wlength = 0;
    itmlst3[0].wcode = PSCAN$_MODE;
    itmlst3[0].pbuffer = (void *) JPI$K_NETWORK;
    itmlst3[0].pretlen = NULL;
    itmlst3[1].wlength = 0;
    itmlst3[1].wcode = 0;
    itmlst3[1].pbuffer = NULL;
    itmlst3[1].pretlen = NULL;

    status = sys$process_scan(&jpictx, itmlst3);
    if (!$VMS_STATUS_SUCCESS(status))
    {
        return false;
    }

    itmlst3[0].wlength = 12;
    itmlst3[0].wcode = JPI$_USERNAME;
    itmlst3[0].pbuffer = usernamebuf;
    itmlst3[0].pretlen = &usernamlen;
    itmlst3[1].wlength = 0;
    itmlst3[1].wcode = 0;
    itmlst3[1].pbuffer = NULL;
    itmlst3[1].pretlen = NULL;

    while (1)
    {
        status = sys$getjpiw(0, &jpictx, NULL, itmlst3, 0, NULL, 0);
        if (status == SS$_NOMOREPROC)
        {
            break;
        }
        if (!$VMS_STATUS_SUCCESS(status))
        {
            return false;
        }
        count++;
    }
    numberOfProcesses = count;

    return true;
}

//
// =============================================================================
// NAME              : getMaxNumberOfProcesses
// DESCRIPTION       : Maximum number of process contexts supported.
// ASSUMPTIONS       : None
// PRE-CONDITIONS    :
// POST-CONDITIONS   :
// NOTES             :
// =============================================================================
//

Boolean OperatingSystem::getMaxNumberOfProcesses(Uint32 & mMaxProcesses)
{
    int status;
    int maxprocount;
    typedef struct
    {
        unsigned short wlength;
        unsigned short wcode;
        void *pbuffer;
        void *pretlen;
    } item_list;
    item_list itmlst3[2];

    itmlst3[0].wlength = sizeof (maxprocount);
    itmlst3[0].wcode = SYI$_MAXPROCESSCNT;
    itmlst3[0].pbuffer = &maxprocount;
    itmlst3[0].pretlen = NULL;
    itmlst3[1].wlength = 0;
    itmlst3[1].wcode = 0;
    itmlst3[1].pbuffer = NULL;
    itmlst3[1].pretlen = NULL;

    status = sys$getsyiw(0, 0, 0, itmlst3, 0, 0, 0);
    if ($VMS_STATUS_SUCCESS(status))
    {
        mMaxProcesses = maxprocount;
        return true;
    }
    else
    {
        return false;
    }
}

//
// =============================================================================
// NAME              : getTotalSwapSpaceSize
// DESCRIPTION       : Total system swap space in Kbytes.
// ASSUMPTIONS       : None
// PRE-CONDITIONS    :
// POST-CONDITIONS   :
// NOTES             :
// =============================================================================
//


// Added calculation of pagesize using sys$getsyi() -PTR 73-51-22
Boolean OperatingSystem::getTotalSwapSpaceSize(Uint64 & mTotalSwapSpaceSize)
{
    int status;
    int swapsize;
    int pagesize;
    typedef struct
    {
        unsigned short wlength;
        unsigned short wcode;
        void *pbuffer;
        void *pretlen;
    } item_list;
    item_list itmlst3[3];

    itmlst3[0].wlength = 4;
    itmlst3[0].wcode = SYI$_SWAPFILE_PAGE;
    itmlst3[0].pbuffer = &swapsize;
    itmlst3[0].pretlen = NULL;
    itmlst3[1].wlength = 4;
    itmlst3[1].wcode = SYI$_PAGE_SIZE;
    itmlst3[1].pbuffer = &pagesize;
    itmlst3[1].pretlen = NULL;
    itmlst3[2].wlength = 0;
    itmlst3[2].wcode = 0;
    itmlst3[2].pbuffer = NULL;
    itmlst3[2].pretlen = NULL;

    status = sys$getsyiw(0, 0, 0, itmlst3, 0, 0, 0);

    // Pagesize in KB
    pagesize = pagesize/1024;

    if ($VMS_STATUS_SUCCESS(status))
    {
        mTotalSwapSpaceSize = swapsize * pagesize;
        return true;
    }
    else
    {
        return false;
    }
}

//
// =============================================================================
// NAME              : getTotalVirtualMemorySize
// DESCRIPTION       : Number of Kbytes of virtual memory for this process.
// ASSUMPTIONS       : None
// PRE-CONDITIONS    :
// POST-CONDITIONS   :
// NOTES             : returns the sum of TotalVisibleMemorySize and
//                     SizeStoredInPagingFiles, which would be the total
//                     physical memory and total size in pagefile.sys
// =============================================================================
//

Boolean OperatingSystem::getTotalVirtualMemorySize(Uint64 & total)
{
    Uint64 visibleMemory;
    Uint64 sizeInPageFile;

    if (getTotalVisibleMemorySize(visibleMemory) == false)
    {
        return false;
    }

    if (getSizeStoredInPagingFiles(sizeInPageFile)  == false)
    {
        return false;
    }

    total = visibleMemory + sizeInPageFile;

    return true;
}

//
// =============================================================================
// NAME              : getFreeVirtualMemory
// DESCRIPTION       : Number of Kbytes of unused virtual memory for this
//                     process.
// ASSUMPTIONS       : None
// PRE-CONDITIONS    :
// POST-CONDITIONS   :
// NOTES             : returns the sum of FreePhysicalMemory and
//                     FreeSpaceInPagingFiles which would be the free physical
//                     memory and free size in pagefile.sys
// =============================================================================
//

Boolean OperatingSystem::getFreeVirtualMemory(Uint64 & freeVirtualMemory)
{
    Uint64 freePhysicalMem;
    Uint64 freePageFile;

    if (getFreePhysicalMemory(freePhysicalMem) == false)
    {
        return false;
    }

    if (getFreeSpaceInPagingFiles(freePageFile) == false)
    {
        return false;
    }

    freeVirtualMemory = freePhysicalMem + freePageFile;

    return true;
}

//
// =============================================================================
// NAME              : getFreePhysicalMemory
// DESCRIPTION       : Number of Kbytes of physical memory unused.
// ASSUMPTIONS       : None
// PRE-CONDITIONS    :
// POST-CONDITIONS   :
// NOTES             :
// =============================================================================
//

// Added calculation of pagesize using sys$getsyi() -PTR 73-51-22
Boolean OperatingSystem::getFreePhysicalMemory(Uint64 & total)
{
    long status = SS$_NORMAL;
    long lFreeMem;
    int pagesize;
    typedef struct
    {
        unsigned short wlength;
        unsigned short wcode;
        void *pbuffer;
        void *pretlen;
    } item_list;
    item_list itmlst3[2];

    itmlst3[0].wlength = 4;
    itmlst3[0].wcode = SYI$_PAGE_SIZE;
    itmlst3[0].pbuffer = &pagesize;
    itmlst3[0].pretlen = NULL;
    itmlst3[1].wlength = 0;
    itmlst3[1].wcode = 0;
    itmlst3[1].pbuffer = NULL;
    itmlst3[1].pretlen = NULL;

    status = sys$getsyiw(0, 0, 0, itmlst3, 0, 0, 0);

    // Pagesize in KB
    pagesize = pagesize/1024;

    struct k1_arglist
    {   // kernel call arguments
        long lCount;    // number of arguments
        long *pFreeMem;
    }
    getfreememkargs = {1};    // init 1 argument

    getfreememkargs.pFreeMem = &lFreeMem;

    status = sys$cmkrnl(GetFreeMem, &getfreememkargs);
    if ($VMS_STATUS_SUCCESS(status))
    {
        total = lFreeMem;
        total = total * pagesize;
        return true;
    }
    else
    {
        return false;
    }
}

//
// =============================================================================
// NAME              : getTotalVisibleMemorySize
// DESCRIPTION       : Number of Kbytes of physical memory.
// ASSUMPTIONS       : None
// PRE-CONDITIONS    :
// POST-CONDITIONS   :
// NOTES             :
// =============================================================================
//

// Added calculation of pagesize using sys$getsyi() -PTR 73-51-22
Boolean OperatingSystem::getTotalVisibleMemorySize(Uint64 & memory)
{
    long status;
    long physmem;
    __int64 membytes;
    int pagesize;
    typedef struct
    {
        unsigned short wlength;
        unsigned short wcode;
        void *pbuffer;
        void *pretlen;
    } item_list;
    item_list itmlst3[3];

    itmlst3[0].wlength = 4;
    itmlst3[0].wcode = SYI$_MEMSIZE;
    itmlst3[0].pbuffer = &physmem;
    itmlst3[0].pretlen = NULL;
    itmlst3[1].wlength = 4;
    itmlst3[1].wcode = SYI$_PAGE_SIZE;
    itmlst3[1].pbuffer = &pagesize;
    itmlst3[1].pretlen = NULL;
    itmlst3[2].wlength = 0;
    itmlst3[2].wcode = 0;
    itmlst3[2].pbuffer = NULL;
    itmlst3[2].pretlen = NULL;

    status = sys$getsyiw(0, 0, 0, itmlst3, 0, 0, 0);

    // Pagesize in KB
    pagesize = pagesize/1024;

    if ($VMS_STATUS_SUCCESS(status))
    {
        membytes = physmem;
        memory = membytes * pagesize;
        return true;
    }
    else
    {
        return false;
    }
}

//
// =============================================================================
// NAME              : getSizeStoredInPagingFiles
// DESCRIPTION       : Number of Kbytes that can be stored in the OS's paging
//                     files.
// ASSUMPTIONS       : None
// PRE-CONDITIONS    :
// POST-CONDITIONS   :
// NOTES             :
// =============================================================================
//

// Added calculation of pagesize using sys$getsyi() -PTR 73-51-22
Boolean OperatingSystem::getSizeStoredInPagingFiles(Uint64 & total)
{
    int status;
    int pagesize;
    int pgsize;
    typedef struct
    {
        unsigned short wlength;
        unsigned short wcode;
        void *pbuffer;
        void *pretlen;
    }item_list;
    item_list itmlst3[3];

    itmlst3[0].wlength = 4;
    itmlst3[0].wcode = SYI$_PAGEFILE_PAGE;
    itmlst3[0].pbuffer = &pagesize;
    itmlst3[0].pretlen = NULL;
    itmlst3[1].wlength = 4;
    itmlst3[1].wcode = SYI$_PAGE_SIZE;
    itmlst3[1].pbuffer = &pgsize;
    itmlst3[1].pretlen = NULL;
    itmlst3[2].wlength = 0;
    itmlst3[2].wcode = 0;
    itmlst3[2].pbuffer = NULL;
    itmlst3[2].pretlen = NULL;

    status = sys$getsyiw(0, 0, 0, itmlst3, 0, 0, 0);

    // Pagesize in KB
    pgsize = pgsize/1024;

    if ($VMS_STATUS_SUCCESS(status))
    {
        total = pagesize * pgsize;
        return true;
    }
    else
    {
        return false;
    }
}

//
// =============================================================================
// NAME              : getFreeSpaceInPagingFiles
// DESCRIPTION       : Number of Kbytes that can be mapped into the OS's paging
//                     files without causing other pages to be swapped out.
// ASSUMPTIONS       : None
// PRE-CONDITIONS    :
// POST-CONDITIONS   :
// NOTES             :
// =============================================================================
//

// Added calculation of pagesize using sys$getsyi() -PTR 73-51-22
Boolean OperatingSystem::getFreeSpaceInPagingFiles(
    Uint64 & freeSpaceInPagingFiles)
{
    int status;
    int pagefree;
    int pagesize;
    typedef struct
    {
        unsigned short wlength;
        unsigned short wcode;
        void *pbuffer;
        void *pretlen;
    } item_list;
    item_list itmlst3[3];

    itmlst3[0].wlength = 4;
    itmlst3[0].wcode = SYI$_PAGEFILE_FREE;
    itmlst3[0].pbuffer = &pagefree;
    itmlst3[0].pretlen = NULL;
    itmlst3[1].wlength = 4;
    itmlst3[1].wcode = SYI$_PAGE_SIZE;
    itmlst3[1].pbuffer = &pagesize;
    itmlst3[1].pretlen = NULL;
    itmlst3[2].wlength = 0;
    itmlst3[2].wcode = 0;
    itmlst3[2].pbuffer = NULL;
    itmlst3[2].pretlen = NULL;

    status = sys$getsyiw(0, 0, 0, itmlst3, 0, 0, 0);

    // Pagesize in KB
    pagesize = pagesize/1024;

    if ($VMS_STATUS_SUCCESS(status))
    {
        freeSpaceInPagingFiles = pagefree * pagesize;
        return true;
    }
    else
    {
        return false;
    }
}

//
// =============================================================================
// NAME              : getMaxProcessMemorySize
// DESCRIPTION       : Maximum amount of Kbytes of memory that can be
//                     allocated to this process.
// ASSUMPTIONS       : None
// PRE-CONDITIONS    :
// POST-CONDITIONS   :
// NOTES             :
// =============================================================================
//

Boolean OperatingSystem::getMaxProcessMemorySize(Uint64 & maxProcessMemorySize)
{
    int status;
    __int64 maxprocmem;
    typedef struct
    {
        unsigned short wlength;
        unsigned short wcode;
        void *pbuffer;
        void *pretlen;
    } item_list;
    item_list itmlst3[2];

    itmlst3[0].wlength = sizeof (maxprocmem);
    itmlst3[0].wcode = SYI$_WSMAX;
    itmlst3[0].pbuffer = &maxprocmem;
    itmlst3[0].pretlen = NULL;
    itmlst3[1].wlength = 0;
    itmlst3[1].wcode = 0;
    itmlst3[1].pbuffer = NULL;
    itmlst3[1].pretlen = NULL;

    status = sys$getsyiw(0, 0, 0, itmlst3, 0, 0, 0);
    if ($VMS_STATUS_SUCCESS(status))
    {
        maxProcessMemorySize = (maxprocmem * 512) / 1024;
        return true;
    }
    else
    {
        return false;
    }
}

//
// =============================================================================
// NAME              : getDistributed
// DESCRIPTION       : Determine whether the OS is distributed across several
//                      nodes. If the system is a cluster member, it is
//                      distributed.
// ASSUMPTIONS       : None
// PRE-CONDITIONS    :
// POST-CONDITIONS   :
// NOTES             :
// =============================================================================
//

Boolean OperatingSystem::getDistributed(Boolean & distributed)
{
    int status;
    int clumem;
    typedef struct
    {
        unsigned short wlength;
        unsigned short wcode;
        void *pbuffer;
        void *pretlen;
    }item_list;
    item_list itmlst3[2];

    itmlst3[0].wlength = 4;
    itmlst3[0].wcode = SYI$_CLUSTER_MEMBER;
    itmlst3[0].pbuffer = &clumem;
    itmlst3[0].pretlen = NULL;
    itmlst3[1].wlength = 0;
    itmlst3[1].wcode = 0;
    itmlst3[1].pbuffer = NULL;
    itmlst3[1].pretlen = NULL;

    status = sys$getsyiw(0, 0, 0, itmlst3, 0, 0, 0);
    if ($VMS_STATUS_SUCCESS(status))
    {
        distributed = clumem;
        return true;
    }
    else
    {
        return false;
    }
}

//
// =============================================================================
// NAME              : getMaxProcsPerUser
// DESCRIPTION       : Maximum number of procs this user can have.
// ASSUMPTIONS       : None
// PRE-CONDITIONS    :
// POST-CONDITIONS   :
// NOTES             :
// =============================================================================
//

Boolean OperatingSystem::getMaxProcsPerUser(Uint32 & maxProcsPerUser)
{
    int status;
    int maxprocount;
    typedef struct
    {
        unsigned short wlength;
        unsigned short wcode;
        void *pbuffer;
        void *pretlen;
    } item_list;
    item_list itmlst3[2];

    itmlst3[0].wlength = sizeof (maxprocount);
    itmlst3[0].wcode = SYI$_MAXPROCESSCNT;
    itmlst3[0].pbuffer = &maxprocount;
    itmlst3[0].pretlen = NULL;
    itmlst3[1].wlength = 0;
    itmlst3[1].wcode = 0;
    itmlst3[1].pbuffer = NULL;
    itmlst3[1].pretlen = NULL;

    status = sys$getsyiw(0, 0, 0, itmlst3, 0, 0, 0);
    if ($VMS_STATUS_SUCCESS(status))
    {
        maxProcsPerUser = maxprocount;
        return true;
    }
    else
    {
        return false;
    }
}

//
// =============================================================================
// NAME              : getSystemUpTime
// DESCRIPTION       : Elapsed time since the OS was booted, in seconds.
// ASSUMPTIONS       : None
// PRE-CONDITIONS    :
// POST-CONDITIONS   :
// NOTES             :
// =============================================================================
//

Boolean OperatingSystem::getSystemUpTime(Uint64 & mUpTime)
{
    long status;
    long item = SYI$_BOOTTIME;
    long dst_desc[2];
    char t_string[24] = "";
    char cimtime[80] = "";
    char log_string[] = "SYS$TIMEZONE_DAYLIGHT_SAVING";
    char libdst;
    time_t tme = 0;
    time_t tme1 = 0;
    unsigned __int64 bintime = 0;
    unsigned short int timbuf[7];
    unsigned short int val = 0;
    unsigned long libop;
    unsigned long libdayweek;
    unsigned long libdayear;
    unsigned int retlen;
    struct tm timetm;
    struct tm *ptimetm = &timetm;
    struct dsc$descriptor_s sysinfo;
    static $DESCRIPTOR(lnm_tbl, "LNM$SYSTEM");
    struct
    {
        unsigned short wLength;
        unsigned short wCode;
        void *pBuffer;
        unsigned int *pRetLen;
        int term;
    } item_list;

    sysinfo.dsc$b_dtype = DSC$K_DTYPE_T;
    sysinfo.dsc$b_class = DSC$K_CLASS_S;
    sysinfo.dsc$w_length = sizeof (t_string);
    sysinfo.dsc$a_pointer = t_string;

    status = lib$getsyi(&item, 0, &sysinfo, &val, 0, 0);
    if (!$VMS_STATUS_SUCCESS(status))
    {
        return false;
    }

    status = sys$bintim(&sysinfo, &bintime);
    if (!$VMS_STATUS_SUCCESS(status))
    {
        return false;
    }

    libop = LIB$K_DAY_OF_WEEK;
    status = lib$cvt_from_internal_time(&libop, &libdayweek, &bintime);
    if (!$VMS_STATUS_SUCCESS(status))
    {
        return false;
    }

    libop = LIB$K_DAY_OF_YEAR;
    status = lib$cvt_from_internal_time(&libop, &libdayear, &bintime);
    if (!$VMS_STATUS_SUCCESS(status))
    {
        return false;
    }

    dst_desc[0] = strlen(log_string);
    dst_desc[1] = (long) log_string;
    item_list.wLength = 1;
    item_list.wCode = LNM$_STRING;
    item_list.pBuffer = &libdst;
    item_list.pRetLen = &retlen;
    item_list.term = 0;

    status = sys$trnlnm(0, &lnm_tbl, &dst_desc, 0, &item_list);
    if (!$VMS_STATUS_SUCCESS(status))
    {
        return false;
    }

    status = sys$numtim(timbuf, &bintime);
    if (!$VMS_STATUS_SUCCESS(status))
    {
        return false;
    }

    timetm.tm_sec = timbuf[5];
    timetm.tm_min = timbuf[4];
    timetm.tm_hour = timbuf[3];
    timetm.tm_mday = timbuf[2];
    timetm.tm_mon = timbuf[1] - 1;
    timetm.tm_year = timbuf[0] - 1900;
    timetm.tm_wday = libdayweek - 1;
    timetm.tm_yday = libdayear - 1;
    timetm.tm_isdst = 0;
    if (libdst != 48)
    {
        timetm.tm_isdst = 1;
    }

    time(&tme);
    tme1 = mktime(ptimetm);
    mUpTime = tme - tme1;

    return true;
}

//
// =============================================================================
// NAME              : getOperatingSystemCapability
// DESCRIPTION       : The OS capability.
// ASSUMPTIONS       : None
// PRE-CONDITIONS    :
// POST-CONDITIONS   :
// NOTES             :
// =============================================================================
//

Boolean OperatingSystem::getOperatingSystemCapability(String & scapability)
{
    scapability.assign("64 bit");
    return true;
}

//
// =============================================================================
// NAME              : _reboot
// DESCRIPTION       : Method to reboot the system.
// ASSUMPTIONS       : None
// PRE-CONDITIONS    :
// POST-CONDITIONS   :
// NOTES             :
// =============================================================================
//

Uint32 OperatingSystem::_reboot()
{
    return false;
}

//
// =============================================================================
// NAME              : _shutdown
// DESCRIPTION       : Method to shutdown system.
// ASSUMPTIONS       : None
// PRE-CONDITIONS    :
// POST-CONDITIONS   :
// NOTES             :
// =============================================================================
//

Uint32 OperatingSystem::_shutdown()
{
    return false;
}
