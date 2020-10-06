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

#ifndef PG_PROCESS_VMS_H
#define PG_PROCESS_VMS_H

/* ==========================================================================
   Includes.
   ========================================================================== */

#include <Pegasus/Provider/CIMInstanceProvider.h>

#include <utsname.h>
#include <netdb.h>
#include <time.h>
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

/*
 * Item List
 */
typedef struct
{
  unsigned short wlength;
  unsigned short wcode;
  void *pbuffer;
  void *pretlen;
} item_list;

/*
 * collected proc info
 */
typedef struct proc_info *proc_info_t;
struct proc_info
{
    uid_t             uid;          // user ID-
    gid_t             rgid;         // real group ID
    pid_t             pid;          // process ID-
    pid_t             ppid;         // parent PID-
    pid_t             pgrp;         // parent group-
    pid_t             session;      // session ID-
    pid_t             tpgrp;        // session associated with parent group
    pid_t             tsession;     // session associated with terminal
    int               jobc;         // current process count qualifying for
                                    // job control
    int               status;       // process status (i.e., ZOMBIE)-
    int               flag;         // process flags
    int               state;        // process execution state
    int               pri;          // process priority-
    int               base_pri;     // base scheduling priority-
    char              p_cursig;     // current signal pointer
    sigset_t          p_sig;        // pointer to signals pending to this
                                    // process
    sigset_t          p_sigmask;    // pointer to signal mask (blocked)
    sigset_t          p_sigignore;  // pointer to signals being ignored
    sigset_t          p_sigcatch;   // pointer to signals being caught
    int               all_swapped;  // process swapped out
    long              p_utime;      // time spent in user space
    unsigned __int64  p_stime;      // process start time pointer
    long              virtual_size; // process virtual address size-
    long              resident_size;// real memory size of process-
    float             percent_mem;  // percent real memory usage
    int               cpu_usage;    // percent CPU usage
    char              tty[8];       // controlling terminal device name
    char              command[256]; // process command string-
    char              *args;        // process command arguments string pointer-
    long              slptime;      // process sleep time
    long              wait_chan;    // address of wait event
    char              uname[13];    // user name string-
    int               suspend_count;// process suspend count
    int               num_threads;  // process thread count
    int               cpu;          // current CPU time used
    int               bound;        //
    int               policy;       // process scheduling policy
    int               pset;         // current process set
    proc_info_t       threads;      // (array) pointer to next proc_info struct
};

#undef NZERO
#define NZERO 0

extern "C" {
extern long CTL$GL_PCB;
};

PEGASUS_USING_STD;
PEGASUS_USING_PEGASUS;


/* ==========================================================================
   Type Definitions.
   ========================================================================== */

class Process
{
public:

    Process();
    ~Process();

    Boolean getCaption(String&) const;

    Boolean getDescription(String&) const;

    Boolean getInstallDate(CIMDateTime&) const;

    Boolean getStatus(String&) const;

    Boolean getName(String&) const;

    Boolean getPriority(Uint32&) const;

    Boolean getExecutionState(Uint16&) const;

    Boolean getOtherExecutionDescription(String&) const;

    Boolean getCreationDate(CIMDateTime&) const;

    Boolean getTerminationDate(CIMDateTime&) const;

    Boolean getKernelModeTime(Uint64&) const;

    Boolean getUserModeTime(Uint64&) const;

    Boolean getWorkingSetSize(Uint64&) const;

    Boolean getRealUserID(Uint64&) const;

    Boolean getProcessGroupID(Uint64&) const;

    Boolean getProcessSessionID(Uint64&) const;

    Boolean getProcessTTY(String&) const;

    Boolean getModulePath(String&) const;

    Boolean getParameters(Array<String>&) const;

    Boolean getProcessNiceValue(Uint32&) const;

    Boolean getProcessWaitingForEvent(String&) const;

    Boolean getCPUTime(Uint32&) const;

    Boolean getRealText(Uint64&) const;

    Boolean getRealData(Uint64&) const;

    Boolean getRealStack(Uint64&) const;

    Boolean getVirtualText(Uint64&) const;

    Boolean getVirtualData(Uint64&) const;

    Boolean getVirtualStack(Uint64&) const;

    Boolean getVirtualMemoryMappedFileSize(Uint64&) const;

    Boolean getVirtualSharedMemory(Uint64&) const;

    Boolean getCpuTimeDeadChildren(Uint64&) const;

    Boolean getSystemTimeDeadChildren(Uint64&) const;

    Boolean getParentProcessID(String&) const;

    Boolean getRealSpace(Uint64&) const;

    Boolean loadProcessInfo(int &pIndex);

    Boolean getProcessInfo(int pIndex);

    Boolean buildProcessTable (unsigned long& jpictx2, int& procCount,
       item_list* itmlst3, struct proc_info* procInfoArray);

    Boolean findProcess(const String& handle);

    String getHandle(void) const;

    String getCurrentTime(void) const;

    String getOSName(void) const;

    String getCSName(void) const;

protected:

private:

    struct proc_info pInfo;

};

#endif  /* #ifndef PG_PROCESS_VMS_H */
