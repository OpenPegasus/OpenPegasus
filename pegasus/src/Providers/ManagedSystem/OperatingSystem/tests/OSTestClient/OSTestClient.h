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
// Author: Susan Campbell, Hewlett-Packard Company (scampbell@hp.com)
//
// Modified By:
//
//%/////////////////////////////////////////////////////////////////////////////

#ifndef _OSTESTCLIENT_H
#define _OSTESTCLIENT_H

#include <Pegasus/Common/Config.h>
#include <Pegasus/Client/CIMClient.h>
#include <Pegasus/Common/CIMDateTime.h>   // getCurrentDateTime

PEGASUS_USING_PEGASUS;
PEGASUS_USING_STD;

class OSTestClient
{
  public:
     OSTestClient(CIMClient &client);
     ~OSTestClient(void);

     // class is hardcoded, but flag for level of logging

     void testEnumerateInstanceNames(CIMClient &client, Boolean verbose);
     void testEnumerateInstances(CIMClient &client, Boolean verbose);
     void testGetInstance(CIMClient &client, Boolean verbose);

     // utility methods for common test functions
     void errorExit(const String &message);
     void testLog(const String &message);

     // have OS-specific checks
     Boolean goodCSCreationClassName(const String &cs_ccn,
                                     Boolean verbose);
     Boolean goodCSName(const String &cs_name,
                        Boolean verbose);
     Boolean goodCreationClassName(const String &ccn,
                                   Boolean verbose);
     Boolean goodName(const String &name, Boolean verbose);
     Boolean goodCaption(const String &cap, Boolean verbose);
     Boolean goodDescription(const String &desc, Boolean verbose);
     Boolean goodInstallDate(const CIMDateTime &idate, Boolean verbose);
     Boolean goodStatus(const String &stat, Boolean verbose);
     Boolean goodOSType(const Uint16 &ostype, Boolean verbose);
     Boolean goodOtherTypeDescription(const String &otdesc,
                                      Boolean verbose);
     Boolean goodVersion(const String &version, Boolean verbose);
     Boolean goodLastBootUpTime(const CIMDateTime &btime,
                                Boolean verbose);
     Boolean goodLocalDateTime(const CIMDateTime &ltime,
                               Boolean verbose);
     Boolean goodCurrentTimeZone(const Sint16 &tz, Boolean verbose);
     Boolean goodNumberOfLicensedUsers(const Uint32 &nlusers,
                                       Boolean verbose);
     Boolean goodNumberOfUsers(const Uint32 &nusers,
                               Boolean verbose);
     Boolean goodNumberOfProcesses(const Uint32 &nprocs,
                                   Boolean verbose);
     Boolean goodMaxNumberOfProcesses(const Uint32 &maxprocs,
                                      Boolean verbose);
     Boolean goodTotalSwapSpaceSize(const Uint64 &totalswap,
                                    Boolean verbose);
     Boolean goodVirtualMemorySize(const Uint64 &totalvmem,
                                   Boolean verbose);
     Boolean goodFreeVirtualMemory(const Uint64 &freevmem,
                                   Boolean verbose);
     Boolean goodFreePhysicalMemory(const Uint64 &freepmem,
                                    Boolean verbose);
     Boolean goodTotalVirtualMemorySize(const Uint64 &totalvmem,
                                        Boolean verbose);
     Boolean goodTotalVisibleMemorySize(const Uint64 &totalvmem,
                                        Boolean verbose);
     Boolean goodSizeStoredInPagingFiles(const Uint64 &pgsize,
                                         Boolean verbose);
     Boolean goodFreeSpaceInPagingFiles(const Uint64 &freepg,
                                        Boolean verbose);
     Boolean goodMaxProcessMemorySize(const Uint64 &maxpmem,
                                      Boolean verbose);
     Boolean goodDistributed(const Boolean &distr,
                             Boolean verbose);
     Boolean goodMaxProcessesPerUser(const Uint32 &umaxproc,
                                     Boolean verbose);
     Boolean goodOSCapability(const String &cap, Boolean verbose);
     Boolean goodSystemUpTime(const Uint64 &uptime, Boolean verbose);

  private:
     // was trying to avoid passing client to each function
     CIMClient _osclient;

     void _validateKeys(CIMObjectPath &cimRef, Boolean verboseTest);
     void _validateProperties(CIMInstance &cimInst, Boolean verboseTest);

};

#endif
