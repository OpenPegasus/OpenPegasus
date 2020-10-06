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
//
//%/////////////////////////////////////////////////////////////////////////////


// This file has the OS-specific routines that will be called to get
// a validation of the CIM information vs. the current test system

#include "OSTestClient.h"

PEGASUS_USING_PEGASUS;
PEGASUS_USING_STD;


static const struct
{
   const char *vendor_name;
   const char *determining_filename;
   const char *optional_string;
} LINUX_VENDOR_INFO[] = {
   { "Caldera",          "coas",               "Caldera Linux" },
   { "Corel",            "environment.corel",  "Corel Linux"   },
   { "Debian GNU/Linux", "debian_version",     NULL            },
   { "Mandrake",         "mandrake-release",   NULL            },
   { "Red Hat",          "redhat-release",     NULL            },
   { "SuSE",             "SuSE-release",       NULL            },
   { "Turbolinux",       "turbolinux-release", NULL            },
   { NULL, NULL, NULL }
};


/**
   getVendorInfo method for Linux implementation of OS Provider

   Gets the system text from vendor's release file
  */
static void getVendorInfo(
    String& releaseText,
    String& nameText)
{
    static const Uint32 MAX_RELEASE_STRING_LEN = 128;
    char infoFile[MAXPATHLEN];
    char buffer[MAX_RELEASE_STRING_LEN];

    for (int ii = 0; LINUX_VENDOR_INFO[ii].vendor_name != NULL; ii++)
    {
        sprintf(infoFile, "/etc/%s",
            LINUX_VENDOR_INFO[ii].determining_filename);

        // If the file exists in /etc, we know what distro we're in
        FILE* vf = fopen(infoFile, "r");
        if (vf)
        {
            // Set the default OS name
            nameText.assign(LINUX_VENDOR_INFO[ii].vendor_name);
            nameText.append(" Distribution");

            if (LINUX_VENDOR_INFO[ii].optional_string == NULL)
            {
                // try to get a more descriptive value from the etc file
                if (fgets(buffer, MAX_RELEASE_STRING_LEN, vf) != NULL)
                {
                    String bufferString = buffer;

                    // parse the text to extract the first line
                    Uint32 bufferIndex = bufferString.find('\n');
                    if (bufferIndex != PEG_NOT_FOUND)
                    {
                        // We have found a valid index into the
                        // release string. Now get just the OS name.
                        releaseText = bufferString.subString(0, bufferIndex);
                        bufferIndex = releaseText.find(" release");
                        if (bufferIndex != PEG_NOT_FOUND)
                        {
                            nameText = releaseText.subString(0, bufferIndex);
                        }
                    }
                }
            }

            fclose(vf);
            break;
        }
    }
}

/**
   goodCSCreationClassName method for the OS Provider Test Client

   Checks the specified value against the expected value and
   returns TRUE if valid, else FALSE.
 */
Boolean OSTestClient::goodCSCreationClassName(const String &cs_ccn,
                                              Boolean verbose)
{
   // This assumes the only right answer is CIM_UnitaryComputerSystem
   // replace with approprate check for the given OS/Provider

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
   if (verbose)
      cout<<"Checking " <<csname<< " against hostname " <<endl;
   cout<<"- No check written for CSName " <<endl;

   // always returns success; replace with appropriate OS/Provider code
   return true;
}

/*
   GoodCreationClassName method for the OS Provider Test Client

   Checks the specified value against the expected value and
   returns TRUE if the same, else FALSE
 */
Boolean OSTestClient::goodCreationClassName(const String &ccn,
                                            Boolean verbose)
{
   // This assumes the only right answer is CIM_OperatingSystem
   // replace with approprate check for the given OS/Provider

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
    String releaseText, osName;

    if (verbose)
        cout<<"Checking " << name << " against OS name "<<endl;
    getVendorInfo( releaseText, osName );
    return (String::equal( name, osName ));
}
/* GoodCaption method for the OS Provider Test Client

   Checks the specified value against the expected value
   and returns TRUE if the same, else FALSE
 */
Boolean OSTestClient::goodCaption(const String &cap,
                                  Boolean verbose)
{
    String releaseText, osName;

    // has check against standard caption
    if (verbose)
        cout<<"Checking Caption " << cap << endl;
    getVendorInfo( releaseText, osName );
    return (String::equal(cap, releaseText ) );
}

/*
   GoodDescription method for the OS Provider Test Client

   Checks the specified value against the expected value and
   returns TRUE if the same, else FALSE
 */
Boolean OSTestClient::goodDescription(const String &desc,
                                      Boolean verbose)
{
    String releaseText, osName;

    // has check against standard description
    if (verbose)
        cout<<"Checking Description " << desc << endl;
    getVendorInfo( releaseText, osName );
    return (String::equal(desc, releaseText) );
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
   cout<<"- No check written for InstallDate " << endl;
   return true;
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
      cout<<"Checking Status" << stat <<  endl;
   cout<<"- No check written for Status " << endl;
   return true;
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
      cout<<"Checking OSType " << endl;
   return true;
}

Boolean OSTestClient::goodOtherTypeDescription(const String &otdesc,
                                               Boolean verbose)
{
   if (verbose)
      cout<<"Checking OtherTypeDescription " << otdesc << endl;
   return true;
}

Boolean OSTestClient::goodVersion(const String &version, Boolean verbose)
{
   if (verbose)
      cout<<"Checking Version " << version << endl;
   cout<<"- No check written for Version " << endl;
   return true;
}

Boolean OSTestClient::goodLastBootUpTime(const CIMDateTime &btime,
                                         Boolean verbose)
{
   if (verbose)
      cout<<"Checking LastBootUpTime " << btime.toString() << endl;
   cout<<"- No check written for LastBootUpTime " << endl;
   return true;
}

Boolean OSTestClient::goodLocalDateTime(const CIMDateTime &ltime,
                                        Boolean verbose)
{
   if (verbose)
      cout<<"Checking LocalDateTime " << ltime.toString() << endl;
   cout<<"- No check written for LocalDateTime " << endl;
   return true;
}

Boolean OSTestClient::goodCurrentTimeZone(const Sint16 &tz, Boolean verbose)
{
   if (verbose)
      cout<<"Checking CurrentTimeZone " << tz << endl;
   cout<<"- No check written for CurrentTimeZone " << endl;
   return true;
}

Boolean OSTestClient::goodNumberOfLicensedUsers(const Uint32 &nlusers,
                                                Boolean verbose)
{
   if (verbose)
      cout<<"Checking NumberOfLicensedUsers " << nlusers << endl;
   cout<<"- No check written for NumberOfLicensedUsers " << endl;
   return true;
}

Boolean OSTestClient::goodNumberOfUsers(const Uint32 &nusers,
                                        Boolean verbose)
{
   if (verbose)
      cout<<"Checking NumberOfUsers " << nusers << endl;
   cout<<"- No check written for NumberOfUsers " << endl;
   return true;
}

Boolean OSTestClient::goodNumberOfProcesses(const Uint32 &nprocs,
                                            Boolean verbose)
{
   if (verbose)
      cout<<"Checking NumberOfProcesses " << nprocs << endl;
   cout<<"- No check written for NumberOfProcesses " << endl;
   return true;
}

Boolean OSTestClient::goodMaxNumberOfProcesses(const Uint32 &maxprocs,
                                               Boolean verbose)
{
   if (verbose)
      cout<<"Checking MaxNumberOfProcs " << maxprocs << endl;
   cout<<"- No check written for MaxNumberOfProcesses " << endl;
   return true;
}

Boolean OSTestClient::goodTotalSwapSpaceSize(const Uint64 &totalswap,
                                             Boolean verbose)
{
   if (verbose)
      cout<<"Checking TotalSwapSpaceSize "  << endl;
   cout<<"- No check written for TotalSwapSpaceSize " << endl;
   return true;
}

Boolean OSTestClient::goodTotalVirtualMemorySize(const Uint64 &totalvmem,
                                                 Boolean verbose)

{
   if (verbose)
      cout<<"Checking TotalVirtualMemorySize "  << endl;
   cout<<"- No check written for TotalVirtualMemorySize" << endl;
   return true;
}

Boolean OSTestClient::goodFreeVirtualMemory(const Uint64 &freevmem,
                                            Boolean verbose)
{
   if (verbose)
      cout<<"Checking FreeVirtualMemory " << endl;
   cout<<"- No check written for FreeVirtualMemory" << endl;
   return true;
}

Boolean OSTestClient::goodFreePhysicalMemory(const Uint64 &freepmem,
                                             Boolean verbose)
{
   if (verbose)
      cout<<"Checking FreePhysicalMemory " << endl;
   cout<<"- No check written for FreePhysicalMemory" << endl;
   return true;
}

Boolean OSTestClient::goodTotalVisibleMemorySize(const Uint64 &totalvmem,
                                                 Boolean verbose)
{
   if (verbose)
      cout<<"Checking TotalVisibleMemorySize " << endl;
   cout<<"- No check written for TotalVisibleMemorySize" << endl;
   return true;
}

Boolean OSTestClient::goodSizeStoredInPagingFiles(const Uint64 &pgsize,
                                                  Boolean verbose)
{
   if (verbose)
      cout<<"Checking SizeStoredInPagingFiles " << endl;
   cout<<"- No check written for SizeStoredInPagingFiles" << endl;
   return true;
}

Boolean OSTestClient::goodFreeSpaceInPagingFiles(const Uint64 &freepg,
                                                 Boolean verbose)
{
   if (verbose)
      cout<<"Checking FreeSpaceInPagingFiles " << endl;
   cout<<"- No check written for FreeSpaceInPagingFiles" << endl;
   return true;
}

Boolean OSTestClient::goodMaxProcessMemorySize(const Uint64 &maxpmem,
                                               Boolean verbose)
{
   if (verbose)
      cout<<"Checking MaxProcessMemSize " << endl;
   cout<<"- No check written for MaxProcessMemSize " << endl;
   return true;
}

Boolean OSTestClient::goodDistributed(const Boolean &distr,
                                      Boolean verbose)
{
   if (verbose)
      cout<<"Checking Distributed " << endl;
   cout<<"- No check written for Distributed " << endl;
   return true;
}

Boolean OSTestClient::goodMaxProcessesPerUser(const Uint32 &umaxproc,
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

Boolean OSTestClient::goodSystemUpTime(const Uint64 &uptime, Boolean verbose)
{
   if (verbose)
      cout<<"Checking SystemUpTime " << endl;
   cout<<"- No check written for SystemUpTime " << endl;
   return true;
}

