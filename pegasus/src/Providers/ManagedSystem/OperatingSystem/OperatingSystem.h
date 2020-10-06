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
//%/////////////////////////////////////////////////////////////////////////
#ifndef _OPERATINGSYSTEM_H
#define _OPERATINGSYSTEM_H

#include <Pegasus/Common/Config.h>
#include <Pegasus/Common/String.h>
#include <Pegasus/Common/CIMDateTime.h>

class OperatingSystem
{
public:

    enum OSTYPE {
        Unknown, Other, MACOS, ATTUNIX, DGUX, DECNT, Digital_Unix,
        OpenVMS, HP_UX, AIX, MVS, OS400, OS2, JavaVM, MSDOS, WIN3x,
        WIN95, WIN98, WINNT, WINCE, NCR3000, NetWare, OSF,
        DCOS, Reliant_UNIX, SCO_UnixWare, SCO_OpenServer, Sequent,
        IRIX, Solaris, SunOS, U6000, ASERIES, TandemNSK, TandemNT,
        BS2000, LINUX, Lynx, XENIX, VM_ESA, Interactive_UNIX,
        BSDUNIX, FreeBSD, NetBSD, GNU_Hurd, OS9, MACH_Kernel,
        Inferno, QNX, EPOC, IxWorks, VxWorks, MiNT, BeOS, HP_MPE,
        NextStep, PalmPilot, Rhapsody, Windows_2000, Dedicated,
        OS_390, VSE, TPF, Windows_Me, Open_UNIX, OpenBDS, NotApplicable,
        Windows_XP, zOS, Windows_2003, Windows_2003_64
    };

public:
    OperatingSystem();
    ~OperatingSystem();
    Boolean getCSName(String& csName);
    Boolean getName(String& name);
    Boolean getCaption(String& caption);
    Boolean getDescription(String& description);
    Boolean getInstallDate(CIMDateTime& installDate);
    Boolean getStatus(String& status);
    Boolean getOSType(Uint16& osType);
    Boolean getOtherTypeDescription(String& otherTypeDescription);
    Boolean getVersion(String& osVersion);
    Boolean getLastBootUpTime(CIMDateTime& lastBootUpTime);
    Boolean getLocalDateTime(CIMDateTime& localDateTime);
    Boolean getCurrentTimeZone(Sint16& currentTimeZone);
    Boolean getNumberOfLicensedUsers(Uint32& numberOfLicensedUsers);
    Boolean getNumberOfUsers(Uint32& numberOfUsers);
    Boolean getNumberOfProcesses(Uint32& numberOfProcesses);
    Boolean getMaxNumberOfProcesses(Uint32& maxNumberOfProcesses);
    Boolean getTotalSwapSpaceSize(Uint64& totalSwapSpaceSize);
    Boolean getTotalVirtualMemorySize(Uint64& totalVirtualMemorySize);
    Boolean getFreeVirtualMemory(Uint64& freeVirtualMemory);
    Boolean getFreePhysicalMemory(Uint64& freePhysicalMemory);
    Boolean getTotalVisibleMemorySize(Uint64& totalVisibleMemorySize);
    Boolean getSizeStoredInPagingFiles(Uint64& sizeStoredInPagingFiles);
    Boolean getFreeSpaceInPagingFiles(Uint64& freeSpaceInPagingFiles);
    Boolean getMaxProcessMemorySize(Uint64& maxProcessMemorySize);
    Boolean getDistributed(Boolean& distributed);
    Boolean getMaxProcsPerUser(Uint32& maxProcsPerUser);
    Boolean getSystemUpTime(Uint64& systemUpTime);
    Boolean getOperatingSystemCapability(String& operatingSystemCapability);

protected:
    Uint64 _totalVM();
    Uint32 _reboot();
    Uint32 _shutdown();
};

#endif
