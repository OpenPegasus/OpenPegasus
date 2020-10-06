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
//%/////////////////////////////////////////////////////////////////////////////


// This file has the OS-specific routines that will be called to get
// a validation of the CIM information vs. the current test system

#include "OSTestClient.h"
#include <sys/param.h>      // for MAXHOSTNAMELEN
#include <sys/utsname.h>    // for uname
#include <unistd.h>         // for gethostname
#include <sys/socket.h>     // for gethostbyname
#include <netinet/in.h>     // for gethostbyname
#include <netdb.h>          // for gethostbyname
#include <utmpx.h>          // for utxent calls
#include <sys/pstat.h>      // for pstat
#include <dl.h>             // for shl_findsym
#include <time.h>           // for localtime_r

/**
   goodCSCreationClassName method for the OS Provider Test Client

   Checks the specified value against the expected value and
   returns TRUE if the same, else FALSE
 */
Boolean OSTestClient::goodCSCreationClassName(const String &cs_ccn,
                                              Boolean verbose)
{
   if (verbose)
      cout<<"Checking " <<cs_ccn<< " against CIM_UnitaryComputerSystem"<<endl;
   return (String::equalNoCase(cs_ccn, "CIM_UnitaryComputerSystem"));
}

/*
   GoodCSName method for the OS Provider Test Client

   Checks the specified value against the expected value and
   returns TRUE if the same, else FALSE
 */
Boolean OSTestClient::goodCSName(const String &csname, Boolean verbose)
{
   struct hostent *he;
   char hostName[PEGASUS_MAXHOSTNAMELEN + 1];

   if (verbose)
      cout<<"Checking " <<csname<< " against hostname" <<endl;

   // try and get fully qualified hostname, else just system name
   if (gethostname(hostName, sizeof(hostName)) != 0)
   {
      return false;  // if can't get data to validate, fail
   }
   hostName[sizeof(hostName)-1] = 0;

   if ((he = gethostbyname(hostName)) != 0)
   {
      strncpy(hostName, he->h_name, sizeof(hostName)-1);
   }

   if (verbose)
       cout << " Host name should be " << hostName << endl;

   return (String::equalNoCase(csname, hostName));
}

/*
   GoodCreationClassName method for the OS Provider Test Client

   Checks the specified value against the expected value and
   returns TRUE if the same, else FALSE
 */
Boolean OSTestClient::goodCreationClassName(const String &ccn,
                                            Boolean verbose)
{
   if (verbose)
      cout<<"Checking " << ccn << " against CIM_OperatingSystem"<<endl;
   return (String::equalNoCase(ccn, "CIM_OperatingSystem"));
}

/*
   GoodName method for the OS Provider Test Client

   Checks the specified value against the expected value and
   returns TRUE if the same, else FALSE
 */
Boolean OSTestClient::goodName(const String &name, Boolean verbose)
{
    struct utsname  unameInfo;

    if (verbose)
      cout<<"Checking " << name << " against OS name"<<endl;

    // Call uname and check for any errors.
    if ((uname(&unameInfo) < 0) && (errno != EOVERFLOW))
    {
       return false;  // if can't get data to validate, fail
    }

    if (verbose)
      cout<<" OS name should be " << unameInfo.sysname << endl;

    return (String::equalNoCase(name, unameInfo.sysname));
}

/* GoodCaption method for the OS Provider Test Client

   Checks the specified value against the expected value
   and returns TRUE if the same, else FALSE
 */
Boolean OSTestClient::goodCaption(const String &cap,
                                  Boolean verbose)
{

   struct utsname     unameInfo;
   String _cap;

   if (verbose)
       cout<<"Checking Caption " << cap << endl;

   // Call uname and check for any errors.
   if ((uname(&unameInfo) < 0) && (errno != EOVERFLOW))
   {
       return false;
   }

   // append in _cap the system name and release.
   _cap.assign(unameInfo.sysname);
   _cap.append(" ");
   _cap.append(unameInfo.release);

  if (verbose)
       cout<<" Should be  " << _cap << endl;

   return (String::equalNoCase(cap,_cap));
}

/*
   GoodDescription method for the OS Provider Test Client

   Checks the specified value against the expected value and
   returns TRUE if the same, else FALSE
 */
Boolean OSTestClient::goodDescription(const String &desc,
                                      Boolean verbose)
{
   if (verbose)
      cout<<"Checking Description " << desc << endl;
   return false;  // HP-UX doesn't implement this
                  // thus it shouldn't be returned as a property
}

/*
   GoodInstallDate method for the OS Provider Test Client

   Checks the specified value against the expected value and
   returns TRUE if the same, else FALSE
 */
Boolean OSTestClient::goodInstallDate(const CIMDateTime &idate,
                                      Boolean verbose)
{
   if (verbose)
      cout<<"Checking InstallDate " << idate.toString() << endl;
   return false;  // not implemented for HP-UX, fail if there
}

/*
   GoodStatus method for the OS Provider Test Client

   Checks the specified value against the expected value and
   returns TRUE if the same, else FALSE
 */
Boolean OSTestClient::goodStatus(const String &stat,
                                 Boolean verbose)
{
   if (verbose)
      cout<<"Checking Status " << stat << " against Unknown" << endl;

   // for now, HP-UX provider always returns Unknown
   return (String::equalNoCase(stat, "Unknown"));
}

/*
   GoodOSType method for the OS Provider Test Client

   Checks the specified value against the expected value and
   returns TRUE if the same, else FALSE
 */
Boolean OSTestClient::goodOSType(const Uint16 &ostype,
                                 Boolean verbose)
{
   if (verbose)
      cout<<"Checking OSType " << ostype << " against 8=HP-UX" << endl;
   return (ostype == 8);  // could use OSType enum of provider
}

Boolean OSTestClient::goodOtherTypeDescription(const String &otdesc,
                                               Boolean verbose)
{
   if (verbose)
      cout<<"Checking OtherTypeDescription " << otdesc << endl;
   return false;   // HP-UX doesn't implement this
                   // thus it shouldn't be returned as a property
}

Boolean OSTestClient::goodVersion(const String &version, Boolean verbose)
{
   struct utsname  unameInfo;

   if (verbose)
     cout<<"Checking Version " << version << endl;

   // Call uname and check for any errors.
   if ((uname(&unameInfo) < 0) && (errno != EOVERFLOW))
   {
      return false;
   }

   if (verbose)
     cout<<" Should be  " << unameInfo.release << endl;

   return (String::equalNoCase(version,unameInfo.release));
}


/**
   goodLastBootUpTime method for HP-UX implementation of OS Provider

   Gets information from pstat call.  Internally in UTC (Universal
   Time Code) which must be converted to localtime for CIM
  */
Boolean OSTestClient::goodLastBootUpTime(
    const CIMDateTime &btime,
    Boolean verbose)
{
   struct tm tmval;
   struct pst_static pst;
   char dateTimeBuffer[26];
   int tzMinutesEast;

   if (verbose)
      cout<<"Checking LastBootUpTime " << btime.toString() << endl;

   if (pstat_getstatic(&pst, sizeof(pst), (size_t)1, 0) == -1)
   {
      return false;   // fail if no validation info
   }

   time_t tmp_time = pst.boot_time;
   localtime_r(&tmp_time, &tmval);
   tzMinutesEast = - (int) timezone / 60;
   if ((tmval.tm_isdst > 0) && daylight)
   {
       // ATTN: It is unclear how to determine the DST offset.  Assume 1 hour.
       tzMinutesEast += 60;
   }

   // format as CIMDateTime
   sprintf(
       dateTimeBuffer,
       "%04d%02d%02d%02d%02d%02d.%06d%+04d",
       1900 + tmval.tm_year,
       tmval.tm_mon + 1, // HP-UX stores month 0-11
       tmval.tm_mday,
       tmval.tm_hour,
       tmval.tm_min,
       tmval.tm_sec,
       0,
       tzMinutesEast);

   if (verbose)
      cout << " Should be " << dateTimeBuffer << endl;

   return (btime == CIMDateTime(dateTimeBuffer));
}

/**
   goodLocalDateTime method of HP-UX OS Provider Test Client

   Uses the CIMOM getCurrentDateTime function and checks that the
   current time from the instance is within one hour of that time.
  */
Boolean OSTestClient::goodLocalDateTime(
    const CIMDateTime &ltime,
    Boolean verbose)
{
   if (verbose)
      cout<<"Checking LocalDateTime " << ltime.toString() << endl;

   CIMDateTime currentDT = CIMDateTime::getCurrentDateTime();

   Sint64 raw_delta;
   try
   {
       raw_delta = CIMDateTime::getDifference(ltime, currentDT);
   }
   catch(DateTimeOutOfRangeException &e)
   {
       cout << "Error : " << e.getMessage() << endl;
       return false;
   }
   Uint64 delta = labs(raw_delta);

   if (verbose) {
      cout<<" Should be close to " << currentDT.toString() << endl;
      printf( " Delta should be within 360 seconds, is %llu\n",delta);
      fflush(stdout);
   }
   // arbitrary choice of expecting them to be within 360 seconds
   return (delta < 360000000);
}

/**
   goodCurrentTimeZone method of HP-UX OS Provider Test Client

   Expect the timezone now to be identical to that returned.
  */
Boolean OSTestClient::goodCurrentTimeZone(const Sint16 &tz, Boolean verbose)
{
   if (verbose)
      cout<<"Checking CurrentTimeZone " << tz << endl;

   CIMDateTime currentDT = CIMDateTime::getCurrentDateTime();
   String ds = currentDT.toString();  // want timezone

   // cheat here since we know the position of the timezone info
   // subtracting '0' gets us the number from the ASCII, while
   // the multiplies do our shifts and we use the sign appropriately
   Sint32 calctz = ((ds[22]-'0') * 100 +
                    (ds[23]-'0') * 10 +
                    (ds[24]-'0')) *
                    (ds[21]=='-'?-1:1);

   if (verbose)
      cout << " Should be " << calctz << endl;

   return (tz == calctz);
}

/**
   goodNumberOfLicensedUsers method for HP-UX implementation of OS provider

   Calls uname and checks the version string (to get user license
   information.  This version field doesn't currently distinguish
   between 128, 256, and unlimited user licensed (all = U).
   Need to determine how to differentiate and fix this, for now return
   0 (which is unlimited).  Don't know if uname -l has same limitation.
  */
Boolean OSTestClient::goodNumberOfLicensedUsers(
    const Uint32 &nlusers,
    Boolean verbose)
{
    struct utsname  unameInfo;
    Uint32 numberOfLicensedUsers;

    if (verbose)
      cout<<"Checking NumberOfLicensedUsers " << nlusers << endl;

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
    if (verbose)
      cout<<" Should be " << numberOfLicensedUsers << endl;

   return (nlusers == numberOfLicensedUsers);
}

/**
   goodNumberOfUsers method for HP-UX implementation of OS Provider

   Goes through the utents, counting the number of type USER_PROCESS
   Works in isolated test env without new users logging in
  */
Boolean OSTestClient::goodNumberOfUsers(
    const Uint32 &nusers,
    Boolean verbose)
{
   struct utmpx * utmpp;
   Uint32 numberOfUsers;

   if (verbose)
      cout<<"Checking NumberOfUsers " << nusers << endl;

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
   if (verbose)
      cout << " Should be " << numberOfUsers << endl;

// works in isolated test env without new users logging in
   return (nusers == numberOfUsers);
}

/**
   goodNumberOfProcesses method for HP-UX implementation of OS Provider

   Gets number of active processes from pstat.
  */
Boolean OSTestClient::goodNumberOfProcesses(
    const Uint32 &nprocs,
    Boolean verbose)
{
   struct pst_dynamic psd;
   Uint32 numberOfProcesses;

   if (verbose)
      cout<<"Checking NumberOfProcesses " << nprocs << endl;

   if (pstat_getdynamic(&psd, sizeof(psd), (size_t)1, 0) == -1)
   {
       return false;
   }

   numberOfProcesses = psd.psd_activeprocs;

   Sint32 raw_delta = nprocs - numberOfProcesses;
   Uint32 delta = abs(raw_delta);

   if (verbose)
   {
      printf (" Delta should be within 10, is %u\n", delta);
      fflush(stdout);
   }

   return (delta <= 10);
}

/**
   goodMaxNumberOfProcesses method for HP-UX implementation of OS Provider

   Gets maximum number of processes from pstat.
  */
Boolean OSTestClient::goodMaxNumberOfProcesses(
    const Uint32 &maxprocs,
    Boolean verbose)
{
   struct pst_static pst;
   Uint32 maxNumberOfProcesses;

   if (verbose)
      cout<<"Checking MaxNumberOfProcs " << maxprocs << endl;

   if (pstat_getstatic(&pst, sizeof(pst), (size_t)1, 0) == -1)
   {
      return false;
   }

   maxNumberOfProcesses = pst.max_proc;
   if (verbose)
      cout<<" Should be " << maxNumberOfProcesses << endl;

   return (maxprocs == maxNumberOfProcesses);
}

/**
   _totalVM method for HP-UX implementation of HP-UX

   Gets information from swapinfo -q command (already in KB).
   Invoked for TotalVirtualMemory as well as TotalSwapSpaceSize.
   Would be more efficient to get this only once.
  */
static Uint64 _totalVM()
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
    return Uint64(swapSize);
}
/**
   goodTotalSwapSpaceSize method for HP-UX implementation of HP-UX

   Gets information from swapinfo -q command (techically not swap
   space, it's paging).   No formal paging files, report as swap.

  */
Boolean OSTestClient::goodTotalSwapSpaceSize(
    const Uint64 &totalswap,
    Boolean verbose)
{
   Uint64 mTotalSwapSpaceSize = 0;

   if (verbose)
   {
      printf("Checking TotalSwapSpaceSize %llu\n", totalswap);
      fflush(stdout);
   }

   mTotalSwapSpaceSize = _totalVM();
   if (mTotalSwapSpaceSize == 0)
      return false;

   if (verbose)
   {
      printf(" Should be %llu\n", mTotalSwapSpaceSize);
      fflush(stdout);
   }
   return (totalswap == mTotalSwapSpaceSize);
}

/**
   getTotalVirtualMemorySize method for HP-UX implementation of HP-UX

   Gets information from swapinfo -q command (techically not swap
   space, it's paging).  Same as the information returned for
   TotalSwapSpace.

  */
Boolean OSTestClient::goodTotalVirtualMemorySize(
    const Uint64 &totalvmem,
    Boolean verbose)
{
   if (verbose)
      cout<<"Checking TotalVirtualMemorySize "<<Uint32(totalvmem)<<endl;

   Uint64 totalVMem = _totalVM();

   if (verbose)
      cout<<" Should be " << Uint32(totalVMem) << endl;

   return (totalvmem == totalVMem);  // will return false if totalVMem=0
}

/**
   goodFreeVirtualMemorySize method for HP-UX implementation of
   OS Provider Test Client.

   Gets information from swapinfo -at command (the Free column)
  */
Boolean OSTestClient::goodFreeVirtualMemory(
    const Uint64 &freevmem,
    Boolean verbose)
{
   char               mline[80];
   FILE             * mswapInfo;
   Uint32             swapAvailable;
   Uint32             swapUsed;
   Uint32             swapFree;

   if (verbose)
      cout<<"Checking FreeVirtualMemory "<< Uint32(freevmem) << endl;

   swapFree = 0;

   // Use a pipe to invoke swapinfo.
   if ((mswapInfo = popen("/usr/sbin/swapinfo -at 2>/dev/null", "r")) != NULL)
   {
      // Now extract the total swap space size from the swapinfo output
      while (fgets(mline, 80, mswapInfo))
      {
          sscanf(mline, "total %u %u %u", &swapAvailable,
                &swapUsed, &swapFree);
      }  // end while

      (void)pclose (mswapInfo);
   }
   if (verbose)
      cout<<" Should be close to " << swapFree << endl;

   Sint64 raw_delta = freevmem - Uint64(swapFree);
   Uint64 delta = labs(raw_delta);

   if (verbose)
   {
      printf (" Delta should be within 131072, is %llu\n", delta);
      fflush(stdout);
   }

   // Empirical results have shown a delta as high as 104788, so use a
   // maximum delta of 2^17 = 131072.
   Boolean isInRange = (delta < 131072);

   if (!isInRange)
   {
      cout << "FreeVirtualMemory received = " << Uint32(freevmem) <<
          ", expected a value near " << Uint32(swapFree) << endl;
   }

   return isInRange;
}

/**
   goodFreePhysicalMemory method for HP-UX implementation of
   OS Provider Test Client.

   Gets information from the pstat system call (psd_free field)
  */
Boolean OSTestClient::goodFreePhysicalMemory(
    const Uint64 &freepmem,
    Boolean verbose)
{
   struct pst_dynamic psd;
   struct pst_static pst;
   float psize;

   if (verbose)
      cout<<"Checking FreePhysicalMemory "<<Uint32(freepmem) << endl;

   if (pstat_getstatic(&pst, sizeof(pst), (size_t)1, 0) == -1)
   {
       return false;
   }
   psize = pst.page_size / 1024;

   if (pstat_getdynamic(&psd, sizeof(psd), (size_t)1, 0) == -1)
   {
       return false;
   }

   if (verbose)
      cout<<" Should be close to "  << Sint32(psd.psd_free * psize) << endl;

   Sint32 raw_delta = (freepmem - Sint32(psd.psd_free * psize));
   Uint32 delta = abs(raw_delta);

   if (verbose)
      cout<<" Delta should be within 32768, is " << delta << endl;

   // Empirical results have shown a delta as high as 18988, so use a
   // maximum delta of 32768.
   Boolean isInRange = (delta <= 32768);

   if (!isInRange)
   {
      cout << "FreePhysicalMemory received = " << Uint32(freepmem) <<
          ", expected a value near " << Uint32(psd.psd_free * psize) << endl;
   }

   return isInRange;
}

/**
   goodTotalVisibleMemorySize method for HP-UX implementation of
   OS Provider Test Client.

   Gets information from pstat (pst.physical_memory adjusted for
   the page size.
   */
Boolean OSTestClient::goodTotalVisibleMemorySize(
    const Uint64 &totalvmem,
    Boolean verbose)
{
   float         psize;
   float         total;
   struct        pst_static pst;

   if (verbose)
      cout<<"Checking TotalVisibleMemorySize "<<Uint32(totalvmem)<<endl;

   if (pstat_getstatic(&pst, sizeof(pst), (size_t)1, 0) == -1)
   {
       return false;
   }

   psize = pst.page_size / 1024;
   total = ((float)pst.physical_memory * psize);
   Uint64 totalVMem = total;

   if (verbose)
      cout<<" Should be " << Uint32(totalVMem) << endl;

   return (totalvmem == totalVMem);
}

Boolean OSTestClient::goodSizeStoredInPagingFiles(
    const Uint64 &pgsize,
    Boolean verbose)
{
   if (verbose)
   {
      cout<<"Checking SizeStoredInPagingFiles " << endl;
      cout<<" Should be 0 on HP-UX" << endl;
   }
   return (pgsize == 0);
}

Boolean OSTestClient::goodFreeSpaceInPagingFiles(
    const Uint64 &freepg,
    Boolean verbose)
{
   if (verbose)
   {
      cout<<"Checking FreeSpaceInPagingFiles " <<endl; //<< freepg << endl;
      cout<<" Should be 0 on HP-UX" << endl;
   }
   return (freepg == 0);
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
           maxProcMemSize = (maxdsiz_64bit + maxssiz_64bit
                                  + maxtsiz_64bit);
           return true;
       } // end if popen worked
       return false;
    }
}


static Boolean getMaxProcMemViaGettune(Boolean are32bit,
                                       Uint64& maxProcMemSize)
{
    uint64_t             maxdsiz = 0;
    uint64_t             maxssiz = 0;
    uint64_t             maxtsiz = 0;
    uint64_t             maxdsiz_64bit = 0;
    uint64_t             maxssiz_64bit = 0;
    uint64_t             maxtsiz_64bit = 0;
    uint64_t             total = 0;


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
   goodMaxProcessMemorySize method for HP-UX implementation of OS Provider

   Calculate values by summing kernel tunable values for max data size, max
   stack size, and max text size.  Different names if 32-bit vs. 64-bit.
   NOT the pstat() pst_maxmem value; that is the amount of physical
   memory available to all user processes when the system first boots.

   Could use the gettune(2) system call on some systems, but it isn't
   available for 11.0, so used kmtune for all releases.
   */
Boolean OSTestClient::goodMaxProcessMemorySize(
    const Uint64 &maxpmem,
    Boolean verbose)
{
   long ret;

   if (verbose)
   {
      printf("Checking maxProcessMemorySize = 0x%llx = %llu\n",
             maxpmem, maxpmem);
      fflush(stdout);   // flush, especially since mix of cout and printf
   }

   Uint64 maxProcessMemorySize = 0;

   ret = sysconf (_SC_KERNEL_BITS);
   if (ret == -1)
   {
      return false;  // fail if no validation info
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
         if (getMaxProcMemViaKmtune(true, maxProcessMemorySize) == false)
            return false;  //fail if can't get info to check
      }
      else
      {
         // can use gettune call 11.11 and onwards (won't be WBEM pre-11.0)
         if (getMaxProcMemViaGettune(true, maxProcessMemorySize) == false)
            return false;  //fail if can't get info to check
      }
   } // end if (ret == 32)

   else   // so ret was 64 (only returns -1, 32, or 64)
   {
      if (strcmp(unameInfo.release,"B.11.00")==0)
      {
         // Use kmtune on 11.0 (since don't have gettune)
         if (getMaxProcMemViaKmtune(false, maxProcessMemorySize) == false)
            return false;  //fail if can't get info to check
      }
      else
      {
         // can use gettune call 11.11 and onwards (won't be WBEM pre-11.0)
         if (getMaxProcMemViaGettune(false, maxProcessMemorySize) == false)
            return false;  //fail if can't get info to check
      }
   }  // end else

   if (verbose)
   {
      printf(" Should be 0x%llx = %llu\n", maxProcessMemorySize,
             maxProcessMemorySize);
      fflush(stdout);
   }

   return (maxpmem == maxProcessMemorySize);
}

Boolean OSTestClient::goodDistributed(
    const Boolean &distr,
    Boolean verbose)
{
   if (verbose)
      cout<<"Checking Distributed against FALSE" << endl;
   return (distr == false);  // HP-UX always false
}

/**
   goodMaxProcessesPerUser method for HP-UX implementation of OS Provider

   Gets the information from kmtune
  */
Boolean OSTestClient::goodMaxProcessesPerUser(
    const Uint32& umaxproc,
    Boolean verbose)
{
    Uint32             maxProcsPerUser;
    FILE             * mtuneInfo;
    char               mline[80];
    struct utsname     unameInfo;
    uint64_t           ret = (uint64_t) -1;

    if  (verbose)
      cout<<"Checking MaxProcsPerUser " << umaxproc << endl;

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
                sscanf(mline, "maxuprc %llu", &ret);
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

        if (shl_findsym(
                &handle,
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
        if (gettune_sym("maxuprc", &ret) != 0)
        {
            return false;  // fail if can't get info
        }
    }

    if (ret != (uint64_t) -1)
    {
        maxProcsPerUser = ret;
    }
    else
    {
       return false;
    }
    if  (verbose)
      cout<<" Should be " << maxProcsPerUser << endl;
    return (umaxproc == maxProcsPerUser);
}

Boolean OSTestClient::goodOSCapability(const String &cap, Boolean verbose)
{
   if (verbose)
      cout<<"Checking OSCapability " << cap << endl;

   // determine what the OS capability should be
   long ret = sysconf(_SC_KERNEL_BITS);

   if (ret == 32)
   {
      if (verbose)
         cout << " Should be 32 bit " << endl;
      return (String::equalNoCase(cap,"32 bit"));
   }
   else if (ret == 64)
   {
      if (verbose)
         cout << " Should be 64 bit " << endl;
      return (String::equalNoCase(cap,"64 bit"));
   }
   return false;
}

/**
   goodSystemUpTime method of HP-UX OS Provider Test Client

   checks the value of uptime versus the value presently.  Expect the
   current value to be greater (by no more than one hour).  Return
   TRUE if within this allowable delta, else FALSE
  */
Boolean OSTestClient::goodSystemUpTime(const Uint64 &uptime, Boolean verbose)
{
   time_t  timeval;
   struct pst_static pst;

   if (verbose)
      // want to print out the Uint64, for now cheat with cast
      cout<<"Checking SystemUpTime " << Uint32(uptime) << endl;

   if (pstat_getstatic(&pst, sizeof(pst), (size_t)1, 0) == -1)
   {
      return false;  // fail if can't get info to validate
   }

   timeval = time((time_t *)NULL);
   timeval = (time_t)((long)timeval - (long)pst.boot_time);
   Uint64 calcUpTime = Uint64(timeval);

   if (verbose)
   {
      printf (" Should be slightly > %llu\n", calcUpTime);
      fflush(stdout);
   }

   Uint32 delta = calcUpTime - uptime;

   if (verbose)
      cout << " Delta should be within 360 seconds, is " << delta << endl;

   return (delta <= 360);
}

