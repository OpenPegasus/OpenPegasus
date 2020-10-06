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
//  PTR 73-51-112
//                 Added proc_table_mutex and proc_table_count variables.
//                 changed logic inside buildProcessTable(), loadProcessInfo(),
//                 getProcessInfo() and findProcess().
//                 This was done to make the process provider thread safe.
//  PTR 73-51-95
//                 Checking for NULL return from "exe_std$cvt_epid_to_pcb()"
//                 A process might have terminated, before calling
//                 exe_std$cvt_epid_to_pcb(). This would return a NULL value
//                 and results in a system crash.
//
//  PTR 73-51-26
//                 Changes made to incorporate review suggestions in
//                 PTR 73-51-26. Removed local definition of PCB structure
//                 and using PCB structure defined in <pcbdef.h>
//
//  PTR 73-51-25
//                 Changes made to Process::getProcessSessionID() to return
//                 False as the
//                 concept of process session id does not apply to OpenVMS
//
//  PTR 73-51-26 and PTR 73-51-15
//                 Changes made to incorporate review comments in PTR 73-51-15.
//                 Changed the getKernelModeTime(), getUserModeTime() and
//                 getCPUTime() functions such that the pid of the current
//                 process is obtained from the pInfo.pid field and not by
//                 passing the PID as a argument to the fucntions.
//                 Removed the #include <pcbdef.h> and used local PCB structure
//                 definition
//  PTR 73-51-29
//                 Changed the getCreationDate() fucntion to set the creation
//                 date to system uptime instead of 17--Nov-1858.
//
//  PTR 73-51-26
//                 Changed getCPUTicks function to take pid as first argument
//                 and use exe_std$cvt_epid_to_pcb() call to get the PCB
//                 structure pointer for each individual process. This would
//                 get the correct kernel mode time, user mode time and CPU
//                 time for each process.
//
//                 Made changes to the getKernelModeTime(), getUserModeTime()
//                 and getCPUTime() functions to make call to getCPUTicks() by
//                 passing pid as first argument.
//
///////////////////////////////////////////////////////////////////////////////

// ==========================================================================
// Includes.
// ==========================================================================
//

#include "ProcessPlatform.h"
#include <pcbdef.h>
#include <Pegasus/Common/pthread.h>
#include <Pegasus/Common/Mutex.h>
#include <Pegasus/Common/System.h>

#define MAXITMLST 16
#define MAXHOSTNAMELEN 256

PEGASUS_USING_STD;
PEGASUS_USING_PEGASUS;

// Initialize static class data


#ifdef __cplusplus
extern "C" {
#endif
PCB *exe_std$cvt_epid_to_pcb(unsigned int epid);
#ifdef __cplusplus
}
#endif

/* Array of process information table. */
static proc_info_t proc_table = (proc_info_t) 0;
/* Number of process loaded in proc_table array */
static int proc_table_count = 0;
/* Lock on proc_table */
static pthread_mutex_t  proc_table_mutex = PTHREAD_MUTEX_INITIALIZER;


Process::Process()
{
}

Process::~Process()
{
}

//
// =============================================================================
// NAME              : getCaption
// DESCRIPTION       :
// ASSUMPTIONS       :
// PRE-CONDITIONS    :
// POST-CONDITIONS   :
// NOTES             :
// =============================================================================
//

Boolean Process::getCaption (String & s)
const
{
    s = String (pInfo.command);
    return true;
}

//
// =============================================================================
// NAME              : getDescription
// DESCRIPTION       :
// ASSUMPTIONS       :
// PRE-CONDITIONS    :
// POST-CONDITIONS   :
// NOTES             :
// =============================================================================
//

Boolean Process::getDescription (String & s)
const
{
    //  s = String(pInfo.args);
    //  return true;
    // not supported

    return false;
}

//
// =============================================================================
// NAME              : getInstallDate
// DESCRIPTION       :
// ASSUMPTIONS       :
// PRE-CONDITIONS    :
// POST-CONDITIONS   :
// NOTES             :
// =============================================================================
//

Boolean Process::getInstallDate (CIMDateTime & d)
const
{
    // not supported

    return false;
}

//
// =============================================================================
// NAME              : getStatus
// DESCRIPTION       :
// ASSUMPTIONS       :
// PRE-CONDITIONS    :
// POST-CONDITIONS   :
// NOTES             :
// =============================================================================
//

Boolean Process::getStatus (String & s)
const
{
    // not supported

    return false;
}

//
// =============================================================================
// NAME              : getName
// DESCRIPTION       :
// ASSUMPTIONS       :
// PRE-CONDITIONS    :
// POST-CONDITIONS   :
// NOTES             :
// =============================================================================
//

Boolean Process::getName (String & s)
const
{
    s = String (pInfo.command);
    return true;
}

//
// =============================================================================
// NAME              : getPriority
// DESCRIPTION       :
// ASSUMPTIONS       :
// PRE-CONDITIONS    :
// POST-CONDITIONS   :
// NOTES             :
// =============================================================================
//

Boolean Process::getPriority (Uint32 & i32)
const
{
    i32 = pInfo.pri;
    return true;
}

//
// =============================================================================
// NAME              : getExecutionState
// DESCRIPTION       :
// ASSUMPTIONS       :
// PRE-CONDITIONS    :
// POST-CONDITIONS   :
// NOTES             :
// =============================================================================
//

Boolean Process::getExecutionState (Uint16 & i16)
const
{
    //
    // From the MOF for this class:
    // [Description (
    // "Indicates the current operating condition of the Process. "
    // "Values include ready (2), running (3), and blocked (4), "
    // "among others."),
    // Values {"Unknown", "Other", "Ready", "Running",
    // "Blocked", "Suspended Blocked", "Suspended Ready",
    // "Terminated", "Stopped", "Growing" },
    //

    enum
    {
        Unknown,
        Other,
        Ready,
        Running,
        Blocked,
        Suspended_Blocked,
        Suspended_Ready,
        Terminated,
        Stopped,
        Growing
    };

    switch (pInfo.state)
    {
        case 1:
            i16 = Other;
            break;
        case 2:
        case 3:
        case 4:
        case 5:
            i16 = Blocked;
            break;
        case 6:
            i16 = Suspended_Blocked;
            break;
        case 7:
            i16 = Blocked;
            break;
        case 8:
            i16 = Suspended_Blocked;
            break;
        case 9:
            i16 = Suspended_Ready;
            break;
        case 10:
            i16 = Suspended_Blocked;
            break;
        case 11:
            i16 = Blocked;
            break;
        case 12:
            i16 = Running;
            break;
        case 13:
            i16 = Suspended_Ready;
            break;
        case 14:
            i16 = Ready;
            break;
        default:
            i16 = Unknown;
    }
    return true;
}

//
// =============================================================================
// NAME              : getOtherExecutionDescription
// DESCRIPTION       :
// ASSUMPTIONS       :
// PRE-CONDITIONS    :
// POST-CONDITIONS   :
// NOTES             :
// =============================================================================
//

Boolean Process::getOtherExecutionDescription (String & s)
const
{
    // not supported

    return false;
}

//
// =============================================================================
// NAME              : convertToCIMDateString
// DESCRIPTION       : Convert VMS date string to CIM format
// ASSUMPTIONS       :
// PRE-CONDITIONS    :
// POST-CONDITIONS   :
// NOTES             :
// =============================================================================
//

int convertToCIMDateString (struct tm *t, char *time)
{
    // Format the date.

    sprintf (time, "%04d%02d%02d%02d%02d%02d.000000%c%03d",
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
// NAME              : getCreationDate
// DESCRIPTION       :
// ASSUMPTIONS       :
// PRE-CONDITIONS    :
// POST-CONDITIONS   :
// NOTES             :
// =============================================================================
//

Boolean Process::getCreationDate (CIMDateTime & d)
const
{
    long status;
    long dst_desc[2];
    char cimtime[80] = "";
    char log_string[] = "SYS$TIMEZONE_DAYLIGHT_SAVING";
    char libdst;
    unsigned __int64 bintime = 0;
    unsigned short int timbuf[7];
    unsigned long libop;
    unsigned long libdayweek;
    unsigned long libdayear;
    unsigned int retlen;
    struct tm timetm;
    struct tm *ptimetm = &timetm;

    // Added to get system uptime for SWAPPER process - PTR 73-51-29
    long item = SYI$_BOOTTIME;
    char t_string[24] = "";
    unsigned __int64  val = 0;
    struct dsc$descriptor_s sysinfo;

    sysinfo.dsc$b_dtype = DSC$K_DTYPE_T;
    sysinfo.dsc$b_class = DSC$K_CLASS_S;
    sysinfo.dsc$w_length = sizeof (t_string);
    sysinfo.dsc$a_pointer = t_string;

    static $DESCRIPTOR (lnm_tbl, "LNM$SYSTEM");
    struct
    {
        unsigned short wLength;
        unsigned short wCode;
        void *pBuffer;
        unsigned int *pRetLen;
        int term;
    } dst_item_list;

    bintime = pInfo.p_stime;

    libop = LIB$K_DAY_OF_WEEK;
    status = lib$cvt_from_internal_time (&libop, &libdayweek, &bintime);
    if (!$VMS_STATUS_SUCCESS (status))
    {
        return false;
    }

    libop = LIB$K_DAY_OF_YEAR;
    status = lib$cvt_from_internal_time (&libop, &libdayear, &bintime);
    if (!$VMS_STATUS_SUCCESS (status))
    {
        return false;
    }

    dst_desc[0] = strlen (log_string);
    dst_desc[1] = (long) log_string;
    dst_item_list.wLength = 1;
    dst_item_list.wCode = LNM$_STRING;
    dst_item_list.pBuffer = &libdst;
    dst_item_list.pRetLen = &retlen;
    dst_item_list.term = 0;

    status = sys$trnlnm (0, &lnm_tbl, &dst_desc, 0, &dst_item_list);
    if (!$VMS_STATUS_SUCCESS (status))
    {
        return false;
    }

    //  Added to get sysuptime for SWAPPER process --- PTR 73-51-29
    if (bintime == 0)
    {
        status = lib$getsyi(&item, 0, &sysinfo, &val, 0, 0);
        status = sys$bintim(&sysinfo, &bintime);
    }

    status = sys$numtim (timbuf, &bintime);
    if (!$VMS_STATUS_SUCCESS (status))
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
    timetm.tm_gmtoff = -18000;
    timetm.tm_zone = "EST";

    if (convertToCIMDateString (ptimetm, cimtime) != -1)
    {
        d = CIMDateTime (cimtime);
        return true;
    }
    return false;
}

//
// =============================================================================
// NAME              : getTerminationDate
// DESCRIPTION       :
// ASSUMPTIONS       :
// PRE-CONDITIONS    :
// POST-CONDITIONS   :
// NOTES             :
// =============================================================================
//

Boolean Process::getTerminationDate (CIMDateTime & d)
const
{
    // not supported

    return false;
}

//
// =============================================================================
// NAME              : GetCPUTicks
// DESCRIPTION       : Get the percentage of CPU time this process is taking.
// ASSUMPTIONS       :
// PRE-CONDITIONS    :
// POST-CONDITIONS   :
// NOTES             :
// =============================================================================
//

// Kernel mode routine
// Passing extended pid of each process as an argument -PTR 73-51-26

int GetCPUTicks(
    long epid,
    long *pKernelTicks,
    long *pExecTicks,
    long *pSuperTicks,
    long *pUserTicks)
{
    PCB *other;                           // Pointer to PCB structure
    int  status = 0;


    // call to get the PCB address of each process from the process extended pid
    other = exe_std$cvt_epid_to_pcb(epid);

    // PTR 73-51-95. Checking for NULL PCB returned. in case
    // the process has terminated.
    if ((other) &&
        (other->pcb$l_kt_high <= 1))
    {
        *pKernelTicks = other->pcb$l_kernel_counter;
        *pExecTicks = other->pcb$l_exec_counter;
        *pSuperTicks = other->pcb$l_super_counter;
        *pUserTicks = other->pcb$l_user_counter;
        status = SS$_NORMAL;        // single thread only
    }

    return status;              // multithread not supported
}

//
// =============================================================================
// NAME              : getKernelModeTime
// DESCRIPTION       :
// ASSUMPTIONS       :
// PRE-CONDITIONS    :
// POST-CONDITIONS   :
// NOTES             :
// =============================================================================
//

Boolean Process::getKernelModeTime (Uint64& i64)
const
{
    int status = SS$_NORMAL;
    long lKernelTicks = 0;
    long lExecTicks = 0;
    long lSuperTicks = 0;
    long lUserTicks = 0;

    struct k1_arglist
    {    // kernel call arguments
        long lCount;           // number of arguments
        long epid;
        long *pKernelTicks;
        long *pExecTicks;
        long *pSuperTicks;
        long *pUserTicks;
    } getcputickskargs = {5};  // init to 5 arguments

    getcputickskargs.epid        = pInfo.pid;
    getcputickskargs.pKernelTicks = &lKernelTicks;
    getcputickskargs.pExecTicks = &lExecTicks;
    getcputickskargs.pSuperTicks = &lSuperTicks;
    getcputickskargs.pUserTicks = &lUserTicks;

    status = sys$cmkrnl (GetCPUTicks, &getcputickskargs);
    if (!$VMS_STATUS_SUCCESS (status))
    {
        return false;
    }

    i64 = lKernelTicks / 10;  // milliseconds
    return true;
}

//
// =============================================================================
// NAME              : getUserModeTime
// DESCRIPTION       :
// ASSUMPTIONS       :
// PRE-CONDITIONS    :
// POST-CONDITIONS   :
// KERNELNOTES             :
// =============================================================================
//

Boolean Process::getUserModeTime (Uint64& i64)
const
{
    int status = SS$_NORMAL;
    long lKernelTicks = 0;
    long lExecTicks = 0;
    long lSuperTicks = 0;
    long lUserTicks = 0;

    struct k1_arglist
    {                             // kernel call arguments
        long lCount;              // number of arguments
        long epid;
        long *pKernelTicks;
        long *pExecTicks;
        long *pSuperTicks;
        long *pUserTicks;
    } getcputickskargs = {5};     // init to 5 arguments

    getcputickskargs.epid = pInfo.pid;
    getcputickskargs.pKernelTicks = &lKernelTicks;
    getcputickskargs.pExecTicks = &lExecTicks;
    getcputickskargs.pSuperTicks = &lSuperTicks;
    getcputickskargs.pUserTicks = &lUserTicks;

    status = sys$cmkrnl (GetCPUTicks, &getcputickskargs);
    if (!$VMS_STATUS_SUCCESS (status))
    {
        return false;
    }

    i64 = lUserTicks / 10;      // milliseconds
    return true;
}

//
// =============================================================================
// NAME              : getWorkingSetSize
// DESCRIPTION       :
// ASSUMPTIONS       :
// PRE-CONDITIONS    :
// POST-CONDITIONS   :
// NOTES             :
// =============================================================================
//

Boolean Process::getWorkingSetSize (Uint64 & i64)
const
{
    i64 = pInfo.pset;
    return true;
}

//
// =============================================================================
// NAME              : getRealUserID
// DESCRIPTION       :
// ASSUMPTIONS       :
// PRE-CONDITIONS    :
// POST-CONDITIONS   :
// NOTES             :
// =============================================================================
//

Boolean Process::getRealUserID (Uint64 & i64)
const
{
    i64 = pInfo.uid;
    return true;
}

//
// =============================================================================
// NAME              : getProcessGroupID
// DESCRIPTION       :
// ASSUMPTIONS       :
// PRE-CONDITIONS    :
// POST-CONDITIONS   :
// NOTES             :
// =============================================================================
//

Boolean Process::getProcessGroupID (Uint64 & i64)
const
{
    i64 = pInfo.pgrp;
    return true;
}

//
// =============================================================================
// NAME              : getProcessSessionID
// DESCRIPTION       :
// ASSUMPTIONS       :
// PRE-CONDITIONS    :
// POST-CONDITIONS   :
// NOTES             :
// =============================================================================
//

// Changed to return false - PTR 73-51-25
Boolean Process::getProcessSessionID (Uint64 & i64) const
{
    // Not Supported
    return false;
}

//
// =============================================================================
// NAME              : getProcessTTY
// DESCRIPTION       :
// ASSUMPTIONS       :
// PRE-CONDITIONS    :
// POST-CONDITIONS   :
// NOTES             :
// =============================================================================
//

Boolean Process::getProcessTTY (String & s)
const
{
    s = String (pInfo.tty);
    return true;
}

//
// =============================================================================
// NAME              : getModulePath
// DESCRIPTION       :
// ASSUMPTIONS       :
// PRE-CONDITIONS    :
// POST-CONDITIONS   :
// NOTES             :
// =============================================================================
//

Boolean Process::getModulePath (String & s)
const
{
    // not supported
    return false;
}

//
// =============================================================================
// NAME              : getParameters
// DESCRIPTION       :
// ASSUMPTIONS       :
// PRE-CONDITIONS    :
// POST-CONDITIONS   :
// NOTES             :
// =============================================================================
//

Boolean Process::getParameters (Array < String > &as)
const
{
    // not supported
    return false;
}

//
// =============================================================================
// NAME              : getProcessNiceValue
// DESCRIPTION       :
// ASSUMPTIONS       :
// PRE-CONDITIONS    :
// POST-CONDITIONS   :
// NOTES             :
// =============================================================================
//

Boolean Process::getProcessNiceValue (Uint32 & i32)
const
{
    int nice_value;

    nice_value = pInfo.base_pri - NZERO;

    // Sanitize nice value to set below zero values equal to zero

    if (nice_value < 0)
    {
        i32 = 0;
    }
    else
    {
        i32 = nice_value;
    }
    return true;
}

//
// =============================================================================
// NAME              : getProcessWaitingForEvent
// DESCRIPTION       :
// ASSUMPTIONS       :
// PRE-CONDITIONS    :
// POST-CONDITIONS   :
// NOTES             :
// =============================================================================
//

Boolean Process::getProcessWaitingForEvent (String & s)
const
{
    char buf[100];

    switch (pInfo.state)
    {
        case 1:
            sprintf (buf, "Collided Page WAIT");
            s = buf;
            break;
      case 2:
          sprintf (buf, "Miscellaneous WAIT");
          s = buf;
          break;
      case 3:
          sprintf (buf, "Common Event Flag WAIT");
          s = buf;
          break;
      case 4:
          sprintf (buf, "Page Fault WAIT");
          s = buf;
          break;
      case 5:
          sprintf (buf, "Local Event Flag WAIT (resident)");
          s = buf;
          break;
      case 6:
          sprintf (buf, "Local Event Flag WAIT (outswapped)");
          s = buf;
          break;
      case 7:
          sprintf (buf, "Hibernate WAIT (resident)");
          s = buf;
          break;
      case 8:
          sprintf (buf, "Hibernate WAIT (outswapped)");
          s = buf;
          break;
      case 9:
          sprintf (buf, "Suspend WAIT (resident)");
          s = buf;
          break;
      case 10:
          sprintf (buf, "Suspend WAIT (outswapped)");
          s = buf;
          break;
      case 11:
          sprintf (buf, "Free Page WAIT");
          s = buf;
          break;
      default:
          sprintf (buf, "Not Waiting!");
          s = buf;
    }
    return true;
}

//
// =============================================================================
// NAME              : getCPUTime
// DESCRIPTION       :
// ASSUMPTIONS       :
// PRE-CONDITIONS    :
// POST-CONDITIONS   :
// NOTES             :
// =============================================================================
//

Boolean Process::getCPUTime (Uint32& i32)
const
{
    int status = SS$_NORMAL;
    long lKernelTicks = 0;
    long lExecTicks = 0;
    long lSuperTicks = 0;
    long lUserTicks = 0;
    long lTotalTicks = 0;
    long avcpucnt;
    __int64 pstartime;
    __int64 qcurtime;
    float fTotalTicks;
    float fpercntime;

    struct k1_arglist
    {                             // kernel call arguments
        long lCount;              // number of arguments
        long epid;
        long *pKernelTicks;
        long *pExecTicks;
        long *pSuperTicks;
        long *pUserTicks;
    } getcputickskargs = {5};     // init to 5 arguments

    item_list itmlst3[2];

    getcputickskargs.epid = pInfo.pid;
    getcputickskargs.pKernelTicks = &lKernelTicks;
    getcputickskargs.pExecTicks = &lExecTicks;
    getcputickskargs.pSuperTicks = &lSuperTicks;
    getcputickskargs.pUserTicks = &lUserTicks;

    status = sys$cmkrnl (GetCPUTicks, &getcputickskargs);
    if (!$VMS_STATUS_SUCCESS (status))
    {
        return false;
    }

    lTotalTicks = lKernelTicks + lExecTicks + lSuperTicks + lUserTicks;

    fTotalTicks = lTotalTicks;  // 10 millisec ticks
    fTotalTicks = fTotalTicks * 10000000;  // 100 nanosec ticks

    pstartime = pInfo.p_stime;  // 100 nanosec ticks

    itmlst3[0].wlength = 4;
    itmlst3[0].wcode = SYI$_AVAILCPU_CNT;
    itmlst3[0].pbuffer = &avcpucnt;
    itmlst3[0].pretlen = NULL;
    itmlst3[1].wlength = 0;
    itmlst3[1].wcode = 0;
    itmlst3[1].pbuffer = NULL;
    itmlst3[1].pretlen = NULL;

    status = sys$getsyiw (0, 0, 0, itmlst3, 0, 0, 0);
    if (!$VMS_STATUS_SUCCESS (status))
    {
        return false;
    }

    status = sys$gettim (&qcurtime);
    if (!$VMS_STATUS_SUCCESS (status))
    {
        return false;
    }

    fpercntime = avcpucnt;
    fpercntime = fpercntime * (qcurtime - pstartime);
    fpercntime = (fTotalTicks / fpercntime) * 100;
    i32 = fpercntime;
    return true;
}

//
// =============================================================================
// NAME              : getRealText
// DESCRIPTION       :
// ASSUMPTIONS       :
// PRE-CONDITIONS    :
// POST-CONDITIONS   :
// NOTES             :
// =============================================================================
//

Boolean Process::getRealText (Uint64 & i64)
const
{
    // not supported

    return false;
}

//
// =============================================================================
// NAME              : getRealData
// DESCRIPTION       :
// ASSUMPTIONS       :
// PRE-CONDITIONS    :
// POST-CONDITIONS   :
// NOTES             :
// =============================================================================
//

Boolean Process::getRealData (Uint64 & i64)
const
{
    // not supported
    return false;
}

//
// =============================================================================
// NAME              : getRealStack
// DESCRIPTION       :
// ASSUMPTIONS       :
// PRE-CONDITIONS    :
// POST-CONDITIONS   :
// NOTES             :
// =============================================================================
//

Boolean Process::getRealStack (Uint64 & i64)
const
{
    // not supported
    return false;
}

//
// =============================================================================
// NAME              : getVirtualText
// DESCRIPTION       :
// ASSUMPTIONS       :
// PRE-CONDITIONS    :
// POST-CONDITIONS   :
// NOTES             :
// =============================================================================
//

Boolean Process::getVirtualText (Uint64 & i64)
const
{
    // not supported
    return false;
}

//
// =============================================================================
// NAME              : getVirtualData
// DESCRIPTION       :
// ASSUMPTIONS       :
// PRE-CONDITIONS    :
// POST-CONDITIONS   :
// NOTES             :
// =============================================================================
//

Boolean Process::getVirtualData (Uint64 & i64)
const
{
    // not supported
    return false;
}

//
// =============================================================================
// NAME              : getVirtualStack
// DESCRIPTION       :
// ASSUMPTIONS       :
// PRE-CONDITIONS    :
// POST-CONDITIONS   :
// NOTES             :
// =============================================================================
//

Boolean Process::getVirtualStack (Uint64 & i64)
const
{
    // not supported
    return false;
}

//
// =============================================================================
// NAME              : getVirtualMemoryMappedFileSize
// DESCRIPTION       :
// ASSUMPTIONS       :
// PRE-CONDITIONS    :
// POST-CONDITIONS   :
// NOTES             :
// =============================================================================
//

Boolean Process::getVirtualMemoryMappedFileSize (Uint64 & i64)
const
{
    // not supported
    return false;
}

//
// =============================================================================
// NAME              : getVirtualSharedMemory
// DESCRIPTION       :
// ASSUMPTIONS       :
// PRE-CONDITIONS    :
// POST-CONDITIONS   :
// NOTES             :
// =============================================================================
//

Boolean Process::getVirtualSharedMemory (Uint64 & i64)
const
{
    // not supported
    return false;
}

//
// =============================================================================
// NAME              : getCpuTimeDeadChildren
// DESCRIPTION       :
// ASSUMPTIONS       :
// PRE-CONDITIONS    :
// POST-CONDITIONS   :
// NOTES             :
// =============================================================================
//

Boolean Process::getCpuTimeDeadChildren (Uint64 & i64)
const
{
    // not supported
    return false;
}

//
// =============================================================================
// NAME              : getSystemTimeDeadChildren
// DESCRIPTION       :
// ASSUMPTIONS       :
// PRE-CONDITIONS    :
// POST-CONDITIONS   :
// NOTES             :
// =============================================================================
//

Boolean Process::getSystemTimeDeadChildren (Uint64 & i64)
const
{
    // not supported
    return false;
}

//
// =============================================================================
// NAME              : getParentProcessID
// DESCRIPTION       :
// ASSUMPTIONS       :
// PRE-CONDITIONS    :
// POST-CONDITIONS   :
// NOTES             :
// =============================================================================
//

Boolean Process::getParentProcessID (String & s)
const
{
    char buf[100];

    sprintf (buf, "%x", pInfo.ppid);
    s = String (buf);

    return true;
}

//
// =============================================================================
// NAME              : getRealSpace
// DESCRIPTION       :
// ASSUMPTIONS       :
// PRE-CONDITIONS    :
// POST-CONDITIONS   :
// NOTES             : returns a modified pIndex that can be used in a
// subsequent call to get next process structure
// =============================================================================
//

Boolean Process::getRealSpace (Uint64 & i64)
const
{
    // not supported
    return false;
}

//
// =============================================================================
// NAME              : getHandle
// DESCRIPTION       : Uses the process id (PID) for the handle value.
// ASSUMPTIONS       : None
// PRE-CONDITIONS    :
// POST-CONDITIONS   :
// NOTES             :
// =============================================================================
//

String Process::getHandle()
const
{
    char buf[100];

    sprintf (buf, "%x", pInfo.pid);

    return String (buf);
}

//
// =============================================================================
// NAME              : getCSName
// DESCRIPTION       : Platform-specific method to get CSName
// ASSUMPTIONS       : None
// PRE-CONDITIONS    :
// POST-CONDITIONS   :
// NOTES             :
// =============================================================================
//

String Process::getCSName()
const
{
    char hostName[PEGASUS_MAXHOSTNAMELEN + 1];
    struct addrinfo *info, hints;
    int rc;


    if (gethostname(hostName, sizeof(hostName)) != 0)
    {
        return String("unknown");
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
    if (info)
    {
        if ((!rc) && (info->ai_canonname))
        {
            strcpy(hostName, info->ai_canonname);
        }
        freeaddrinfo(info);
    }

    return String(hostName);
}

//
// =============================================================================
// NAME              : getOSName
// DESCRIPTION       : Platform-specific method to get OSname.
// ASSUMPTIONS       : None
// PRE-CONDITIONS    :
// POST-CONDITIONS   :
// NOTES             :
// =============================================================================
//

String Process::getOSName()
const
{
    struct utsname unameInfo;

    // Call uname and check for any errors.

    if (uname (&unameInfo) < 0)
    {
        return String ("unknown");
    }
    else
    {
        return String (unameInfo.sysname);
    }
}

//
// =============================================================================
// NAME              : getCurrentTime
// DESCRIPTION       : Platform-specific routine to get a timestamp stat Name
//                     key
// ASSUMPTIONS       : None
// PRE-CONDITIONS    :
// POST-CONDITIONS   :
// NOTES             :
// =============================================================================
//

String Process::getCurrentTime() const
{
    time_t t = time (0);

    return String (ctime (&t));
}

//
// =============================================================================
// NAME              : getmaxprocount
// DESCRIPTION       : Get the maximum number of processes allowed on this
//                     system.
// ASSUMPTIONS       : None
// PRE-CONDITIONS    :
// POST-CONDITIONS   :
// NOTES             :
// =============================================================================
//

int getmaxprocount()
{
    int status;
    unsigned long maxprocount;
    item_list itmlst3[2];
    item_list *itml3 = itmlst3;

    itmlst3[0].wlength = sizeof (maxprocount);
    itmlst3[0].wcode = SYI$_MAXPROCESSCNT;
    itmlst3[0].pbuffer = &maxprocount;
    itmlst3[0].pretlen = NULL;
    itmlst3[1].wlength = 0;
    itmlst3[1].wcode = 0;
    itmlst3[1].pbuffer = NULL;
    itmlst3[1].pretlen = NULL;

    status = sys$getsyiw (0, 0, 0, itmlst3, 0, 0, 0);
    if ($VMS_STATUS_SUCCESS (status))
    {
        return maxprocount;
    }
    else
    {
      return 0;
    }
}

//
// =============================================================================
// NAME              : loadProcessInfo
// DESCRIPTION       : get process info from system into internal data struct
// ASSUMPTIONS       : None
// PRE-CONDITIONS    :
// POST-CONDITIONS   :
// NOTES             :
// =============================================================================
//

Boolean Process::loadProcessInfo (int &pIndex)
{
    int status;
    unsigned long maxprocount;
    __int64 qpid;
    char handle[100];
    unsigned long jpictx2; /* The context for JPI calls */
    item_list itmlst3[MAXITMLST];
    int procCount;
    Boolean stat;

    /* If this is the first process request. Rebuild proc_table */
    if (pIndex == 0)
    {
        /* Lock the mutex on proc_table */
        pthread_mutex_lock(&proc_table_mutex);

        /* Free the old proc_table */
        if (proc_table != NULL)
        {
            free(proc_table);
            proc_table = NULL;
            proc_table_count = 0;
        }

        /* Find the maximum process that could run on the system */
        if ((maxprocount = getmaxprocount ()) == 0)
        {
            /* Un lock the mutex on proc_table */
            pthread_mutex_unlock(&proc_table_mutex);
            /* Return false */
            return false;
        }

        /* Allocate memory to proc_table */
        proc_table =
            (proc_info_t) calloc (maxprocount + 1, sizeof (struct proc_info));

        /* Error in allocating Memory. return false */
        if (NULL == proc_table)
        {
            /* Un lock the mutex on proc_table */
            pthread_mutex_unlock(&proc_table_mutex);
            return false;
        }

        jpictx2 = 0;
        procCount = 0;
        itmlst3[0].wlength = 0;
        itmlst3[0].wcode = PSCAN$_MODE;
        itmlst3[0].pbuffer = (void *) JPI$K_OTHER;
        itmlst3[0].pretlen = NULL;
        itmlst3[1].wlength = 0;
        itmlst3[1].wcode = 0;
        itmlst3[1].pbuffer = NULL;
        itmlst3[1].pretlen = NULL;

        status = sys$process_scan (&jpictx2, itmlst3);
        if (!$VMS_STATUS_SUCCESS (status))
        {
            /* Free the proc_table */
            free (proc_table);
            proc_table = NULL;
            proc_table_count = 0;

            /* Un lock the mutex on proc_table */
            pthread_mutex_unlock(&proc_table_mutex);

            /* Return failure */
            return false;
        }

        stat = buildProcessTable(jpictx2, procCount, itmlst3, proc_table);
        proc_table_count += procCount;

        jpictx2 = 0;
        procCount = 0;
        itmlst3[0].wlength = 0;
        itmlst3[0].wcode = PSCAN$_MODE;
        itmlst3[0].pbuffer = (void *) JPI$K_BATCH;
        itmlst3[0].pretlen = NULL;
        itmlst3[1].wlength = 0;
        itmlst3[1].wcode = 0;
        itmlst3[1].pbuffer = NULL;
        itmlst3[1].pretlen = NULL;

        status = sys$process_scan (&jpictx2, itmlst3);
        if (!$VMS_STATUS_SUCCESS (status))
        {
            /* Free the proc_table */
            free (proc_table);
            proc_table = NULL;
            proc_table_count = 0;

            /* Un lock the mutex on proc_table */
            pthread_mutex_unlock(&proc_table_mutex);

            /* Return failure */
            return false;
        }

        stat = buildProcessTable(jpictx2, procCount, itmlst3,
                                 &proc_table[proc_table_count]);
        proc_table_count += procCount;

        jpictx2 = 0;
        procCount = 0;
        itmlst3[0].wlength = 0;
        itmlst3[0].wcode = PSCAN$_MODE;
        itmlst3[0].pbuffer = (void *) JPI$K_NETWORK;
        itmlst3[0].pretlen = NULL;
        itmlst3[1].wlength = 0;
        itmlst3[1].wcode = 0;
        itmlst3[1].pbuffer = NULL;
        itmlst3[1].pretlen = NULL;

        status = sys$process_scan (&jpictx2, itmlst3);
        if (!$VMS_STATUS_SUCCESS (status))
        {
            /* Free the proc_table */
            free (proc_table);
            proc_table = NULL;
            proc_table_count = 0;

            /* Un lock the mutex on proc_table */
            pthread_mutex_unlock(&proc_table_mutex);

            /* Return failure */
            return false;
        }

        stat = buildProcessTable(jpictx2, procCount, itmlst3,
                                 &proc_table[proc_table_count]);
        proc_table_count += procCount;

        jpictx2 = 0;
        itmlst3[0].wlength = 0;
        itmlst3[0].wcode = PSCAN$_MODE;
        itmlst3[0].pbuffer = (void *) JPI$K_INTERACTIVE;
        itmlst3[0].pretlen = NULL;
        itmlst3[1].wlength = 0;
        itmlst3[1].wcode = 0;
        itmlst3[1].pbuffer = NULL;
        itmlst3[1].pretlen = NULL;

        status = sys$process_scan (&jpictx2, itmlst3);
        if (!$VMS_STATUS_SUCCESS (status))
        {
            /* Free the proc_table */
            free (proc_table);
            proc_table = NULL;
            proc_table_count = 0;

            /* Un lock the mutex on proc_table */
            pthread_mutex_unlock(&proc_table_mutex);

            /* Return failure */
            return false;
        }

        stat = buildProcessTable(jpictx2, procCount, itmlst3,
                                 &proc_table[proc_table_count]);
        proc_table_count += procCount;

        /* Un lock the mutex on proc_table */
        pthread_mutex_unlock(&proc_table_mutex);
    } /* End if (pIndex == 0), rebuild of proc_table */

    return getProcessInfo(pIndex);
}

//
// =============================================================================
// NAME              : buildProcessTable
// DESCRIPTION       : process table build loop
// ASSUMPTIONS       : None
// PRE-CONDITIONS    :
// POST-CONDITIONS   :
// NOTES             :
// =============================================================================
//

Boolean Process::buildProcessTable (unsigned long& jpictx2,
                                    int& procCount,
                                    item_list* itmlst3,
                                    struct proc_info* procInfoArray)
{
    int status;
    char procimgnambuf[256];
    char proctermbuf[8];
    char usernamebuf[13];
    int procimgnamlen;
    int proctermlen;
    int usernamlen;
    unsigned __int64 proclgntim;
    long procpid;
    long proccputim;
    long procgrp;
    long procppid;
    long procuic;
    long procpgflquo;
    long procpri;
    long procbasepri;
    long procstate;
    long procwssize;
    long proclgnflgs;


    itmlst3[0].wlength = sizeof (usernamebuf);
    itmlst3[0].wcode = JPI$_USERNAME;
    itmlst3[0].pbuffer = usernamebuf;
    itmlst3[0].pretlen = &usernamlen;
    itmlst3[1].wlength = 4;
    itmlst3[1].wcode = JPI$_PID;
    itmlst3[1].pbuffer = &procpid;
    itmlst3[1].pretlen = NULL;
    itmlst3[2].wlength = 4;
    itmlst3[2].wcode = JPI$_CPUTIM;
    itmlst3[2].pbuffer = &proccputim;
    itmlst3[2].pretlen = NULL;
    itmlst3[3].wlength = 4;
    itmlst3[3].wcode = JPI$_GRP;
    itmlst3[3].pbuffer = &procgrp;
    itmlst3[3].pretlen = NULL;
    itmlst3[4].wlength = 4;
    itmlst3[4].wcode = JPI$_MASTER_PID;
    itmlst3[4].pbuffer = &procppid;
    itmlst3[4].pretlen = NULL;
    itmlst3[5].wlength = 4;
    itmlst3[5].wcode = JPI$_UIC;
    itmlst3[5].pbuffer = &procuic;
    itmlst3[5].pretlen = NULL;
    itmlst3[6].wlength = sizeof (procimgnambuf);
    itmlst3[6].wcode = JPI$_IMAGNAME;
    itmlst3[6].pbuffer = procimgnambuf;
    itmlst3[6].pretlen = &procimgnamlen;
    itmlst3[7].wlength = 4;
    itmlst3[7].wcode = JPI$_PGFLQUOTA;
    itmlst3[7].pbuffer = &procpgflquo;
    itmlst3[7].pretlen = NULL;
    itmlst3[8].wlength = 4;
    itmlst3[8].wcode = JPI$_PRI;
    itmlst3[8].pbuffer = &procpri;
    itmlst3[8].pretlen = NULL;
    itmlst3[9].wlength = 4;
    itmlst3[9].wcode = JPI$_PRIB;
    itmlst3[9].pbuffer = &procbasepri;
    itmlst3[9].pretlen = NULL;
    itmlst3[10].wlength = 4;
    itmlst3[10].wcode = JPI$_STATE;
    itmlst3[10].pbuffer = &procstate;
    itmlst3[10].pretlen = NULL;
    itmlst3[11].wlength = 4;
    itmlst3[11].wcode = JPI$_WSSIZE;
    itmlst3[11].pbuffer = &procwssize;
    itmlst3[11].pretlen = NULL;
    itmlst3[12].wlength = 4;
    itmlst3[12].wcode = JPI$_LOGIN_FLAGS;
    itmlst3[12].pbuffer = &proclgnflgs;
    itmlst3[12].pretlen = NULL;
    itmlst3[13].wlength = 8;
    itmlst3[13].wcode = JPI$_LOGINTIM;
    itmlst3[13].pbuffer = &proclgntim;
    itmlst3[13].pretlen = NULL;
    itmlst3[14].wlength = sizeof (proctermbuf);
    itmlst3[14].wcode = JPI$_TERMINAL;
    itmlst3[14].pbuffer = proctermbuf;
    itmlst3[14].pretlen = &proctermlen;
    itmlst3[MAXITMLST - 1].wlength = 0;
    itmlst3[MAXITMLST - 1].wcode = 0;
    itmlst3[MAXITMLST - 1].pbuffer = NULL;
    itmlst3[MAXITMLST - 1].pretlen = NULL;

    procCount = 0;
    while (1)
    {
        status = sys$getjpiw (0, &jpictx2, NULL, itmlst3, 0, NULL, 0);
        if (status == SS$_NOMOREPROC)
        {
            break;
        }
        if (!$VMS_STATUS_SUCCESS (status))
        {
            return status;
        }

        usernamebuf[12] = '\0';

        procInfoArray[procCount].ppid = procppid;
        procInfoArray[procCount].pid = procpid;
        procInfoArray[procCount].uid = procuic & 0xFFFF;
        procInfoArray[procCount].pgrp = (procuic >> 16) & 0xFFFF;
        procInfoArray[procCount].rgid = procgrp;
        procInfoArray[procCount].cpu = proccputim;
        procInfoArray[procCount].virtual_size = procpgflquo;
        procInfoArray[procCount].pri = procpri;
        procInfoArray[procCount].base_pri = procbasepri;
        procInfoArray[procCount].state = procstate;
        procInfoArray[procCount].pset = procwssize;
        procInfoArray[procCount].p_stime = proclgntim;
        strncpy (procInfoArray[procCount].uname, usernamebuf, 12);
        strncpy (procInfoArray[procCount].command, procimgnambuf, 256);
        strncpy (procInfoArray[procCount].tty, proctermbuf, 8);
        procCount++;
    }

    return true;
}
//
// =============================================================================
// NAME              : getProcessInfo
// DESCRIPTION       : reset process table pointer based on the index
// ASSUMPTIONS       : None
// PRE-CONDITIONS    :
// POST-CONDITIONS   :
// NOTES             :
// =============================================================================
//

Boolean Process::getProcessInfo (int Index)
{
    Boolean retVal;

    /* Lock the mutex on proc_table */
    pthread_mutex_lock(&proc_table_mutex);

    if (Index < proc_table_count)
    {
        memcpy(&pInfo, &proc_table[Index], sizeof (struct proc_info));
        retVal = true;
    }
    else
    {
        retVal = false;
    }

    /* Un lock the mutex on proc_table */
    pthread_mutex_unlock(&proc_table_mutex);

    return retVal;
}

//
// =============================================================================
// NAME              : findProcess
// DESCRIPTION       : find the requested process and load its data
// ASSUMPTIONS       : None
// PRE-CONDITIONS    :
// POST-CONDITIONS   :
// NOTES             :
// =============================================================================
//

Boolean Process::findProcess (const String & handle)
{
    long i;
    Boolean retVal = false;
    unsigned __int64 pid;

    /* Convert handle to an integer */
    pid = strtol(handle.getCString (), (char **)NULL, 16);

    /* Lock the mutex on proc_table */
    pthread_mutex_lock(&proc_table_mutex);

    for (i = 0; i < proc_table_count; i++)
    {
        if (proc_table[i].pid == pid)
        {
            memcpy(&pInfo, &proc_table[i], sizeof (struct proc_info));
            retVal = true;
        }
    }

    /* Un lock the mutex on proc_table */
    pthread_mutex_unlock(&proc_table_mutex);

    return retVal;
}
