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

   if (he = gethostbyname(hostName))
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
    if (uname(&unameInfo) < 0)
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
   if (uname(&unameInfo) < 0)
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
   // has check against standard description
   if (verbose)
      cout<<"Checking Description " << desc << endl;
   return (String::equalNoCase(desc,
     "This instance reflects the Operating System on which the "
     "CIMOM is executing (as distinguished from instances of "
     "other installed operating systems that could be run)."));
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
   return false;  // not implemented for SunOS, fail if there
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

   // for now, SunOS provider always returns Unknown
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
      cout<<"Checking OSType " << ostype << " against SunOS=30" << endl;
   return (ostype == 30);
}

Boolean OSTestClient::goodOtherTypeDescription(const String &otdesc,
                                               Boolean verbose)
{
   if (verbose)
      cout<<"Checking OtherTypeDescription " << otdesc << endl;



   struct utsname     unameInfo;
   String otdesc_expected;

   // Call uname and check for any errors.
   if (uname(&unameInfo) < 0)
   {
       return false;
   }

   // append in _cap the system name and release.
   otdesc_expected.assign(unameInfo.release);
   otdesc_expected.append(" ");
   otdesc_expected.append(unameInfo.version);

  if (verbose)
       cout<<" Should be  " << otdesc_expected << endl;

   return (String::equalNoCase(otdesc, otdesc_expected));

}

Boolean OSTestClient::goodVersion(const String &version, Boolean verbose)
{
   struct utsname  unameInfo;

   if (verbose)
     cout<<"Checking Version " << version << endl;

   // Call uname and check for any errors.
   if (uname(&unameInfo) < 0)
   {
      return false;
   }

   if (verbose)
     cout<<" Should be  " << unameInfo.release << endl;

   return (String::equalNoCase(version,unameInfo.release));
}


/**
   goodLastBootUpTime method for SunOS implementation of OS Provider

   Gets information from pstat call.  Internally in UTC (Universal
   Time Code) which must be converted to localtime for CIM
  */
Boolean OSTestClient::goodLastBootUpTime(
    const CIMDateTime &btime,
    Boolean verbose)
{
   if (verbose)
      cout<<"Checking LastBootUpTime " << btime.toString() << endl;
   cout<<"- No check written for LastBootUpTime " << endl;
   return true;

}

/**
   goodLocalDateTime method of SunOS OS Provider Test Client

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
       exit;
   }
   Uint64 delta = labs(raw_delta);

   if (verbose) {
      cout<<" Should be close to " << currentDT.toString() << endl;
      printf( " Delta should be within 360 seconds, is %lld\n",delta);
      fflush(stdout);
   }
   // arbitrary choice of expecting them to be within 360 seconds
   return (delta < 360000000);
}

/**
   goodCurrentTimeZone method of SunOS OS Provider Test Client

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
   goodNumberOfLicensedUsers method for SunOS implementation of OS provider

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
    Uint32 numberOfLicensedUsers=0;

    if (verbose)
      cout<<"Checking NumberOfLicensedUsers " << nlusers << endl;

    if (verbose)
      cout<<" Should be " << numberOfLicensedUsers << endl;

   return (nlusers == numberOfLicensedUsers);
}

/**
   goodNumberOfUsers method for SunOS implementation of OS Provider

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
   goodNumberOfProcesses method for SunOS implementation of OS Provider

   Gets number of active processes from pstat.
  */
Boolean OSTestClient::goodNumberOfProcesses(
    const Uint32 &nprocs,
    Boolean verbose)
{
   if (verbose)
      cout<<"Checking NumberOfProcesses " << nprocs << endl;
   cout<<"- No check written for NumberOfProcesses " << endl;
   return true;
}

/**
   goodMaxNumberOfProcesses method for SunOS implementation of OS Provider

   Gets maximum number of processes from pstat.
  */
Boolean OSTestClient::goodMaxNumberOfProcesses(
    const Uint32 &maxprocs,
    Boolean verbose)
{
   if (verbose)
      cout<<"Checking MaxNumberOfProcs " << maxprocs << endl;
   cout<<"- No check written for MaxNumberOfProcesses " << endl;
   return true;

}

/**
   _totalVM method for SunOS implementation of OS Provider

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
           sscanf(mline, "%d", &swapSize);
        }  // end while

        (void)pclose (mswapInfo);
    }
    return Uint64(swapSize);
}
/**
   goodTotalSwapSpaceSize method for SunOS implementation of OS Provider

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
      printf("Checking TotalSwapSpaceSize %lld\n", totalswap);
      fflush(stdout);
   }

   mTotalSwapSpaceSize = _totalVM();
   if (mTotalSwapSpaceSize == 0)
      return false;

   if (verbose)
   {
      printf(" Should be %lld\n", mTotalSwapSpaceSize);
      fflush(stdout);
   }
   return (totalswap == mTotalSwapSpaceSize);
}

/**
   getTotalVirtualMemorySize method for SunOS implementation of OS Provider

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
   goodFreeVirtualMemorySize method for SunOS implementation of OS Provider

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
      printf (" Delta should be within 65536, is %lld\n", delta);
      fflush(stdout);
   }

   // arbitrary choice of valid delta - typically ran within
   // 2048, but with many client connections, went as high as
   // 36,000+.  Thus chose 2^16 = 65536 (still helps weed out
   // garbage values).
   return (delta < 65536);
}

/**
   goodFreePhysicalMemory method for SunOS implementation of
   OS Provider Test Client.

  */
Boolean OSTestClient::goodFreePhysicalMemory(
    const Uint64 &freepmem,
    Boolean verbose)
{
   if (verbose)
      cout<<"Checking FreePhysicalMemory " << endl;
   cout<<"- No check written for FreePhysicalMemory" << endl;
   return true;
}

/**
   goodTotalVisibleMemorySize method for SunOS implementation of
   OS Provider Test Client.

   Gets information from pstat (pst.physical_memory adjusted for
   the page size.
   */
Boolean OSTestClient::goodTotalVisibleMemorySize(
    const Uint64 &totalvmem,
    Boolean verbose)
{
   if (verbose)
      cout<<"Checking TotalVisibleMemorySize " << endl;
   cout<<"- No check written for TotalVisibleMemorySize" << endl;
   return true;
}

Boolean OSTestClient::goodSizeStoredInPagingFiles(
    const Uint64 &pgsize,
    Boolean verbose)
{
   if (verbose)
      cout<<"Checking SizeStoredInPagingFiles " << endl;
   cout<<"- No check written for SizeStoredInPagingFiles" << endl;
   return true;
}

Boolean OSTestClient::goodFreeSpaceInPagingFiles(
    const Uint64 &freepg,
    Boolean verbose)
{
   if (verbose)
      cout<<"Checking FreeSpaceInPagingFiles " << endl;
   cout<<"- No check written for FreeSpaceInPagingFiles" << endl;
   return true;
}


/**
   goodMaxProcessMemorySize method for Solaris implementation of OS Provider

   */
Boolean OSTestClient::goodMaxProcessMemorySize(
    const Uint64 &maxpmem,
    Boolean verbose)
{
   if (verbose)
      cout<<"Checking MaxProcessMemSize " << endl;
   cout<<"- No check written for MaxProcessMemSize " << endl;
   return true;
}

Boolean OSTestClient::goodDistributed(
    const Boolean &distr,
    Boolean verbose)
{
   if (verbose)
      cout<<"Checking Distributed against FALSE" << endl;
   return (distr == false);  // Solaris always false
}

/**
   goodMaxProcessesPerUser method for Solaris implementation of OS Provider

  */
Boolean OSTestClient::goodMaxProcessesPerUser(
    const Uint32& umaxproc,
    Boolean verbose)
{
   if (verbose)
      cout<<"Checking MaxProcsPerUser " << umaxproc << endl;
   cout<<"- No check written for MaxProcsPerUser " << endl;
   return true;
}

Boolean OSTestClient::goodOSCapability(const String &cap, Boolean verbose)
{
   if (verbose)
      cout<<"Checking OSCapability " << cap << endl;
   cout<<"- No check written for OSCapability " << endl;
   return true;
}

/**
   goodSystemUpTime method of Solaris OS Provider Test Client

  */
Boolean OSTestClient::goodSystemUpTime(const Uint64 &uptime, Boolean verbose)
{
   if (verbose)
      cout<<"Checking SystemUpTime " << endl;
   cout<<"- No check written for SystemUpTime " << endl;
   return true;
}

