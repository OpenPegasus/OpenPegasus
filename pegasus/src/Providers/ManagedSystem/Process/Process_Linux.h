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

#ifndef PG_PROCESS_LINUX_H
#define PG_PROCESS_LINUX_H

/* ==========================================================================
   Includes.
   ========================================================================== */

#include <Pegasus/Provider/CIMInstanceProvider.h>
#include <Pegasus/Common/String.h>
#include <dirent.h>
#include <fcntl.h>    // for O_RDONLY
#include <stdio.h>    // for sprintf
#include <unistd.h>   // for close, read and getpagesize
#include <string.h>   // for strchr
#include <pthread.h>  // for pthreads and spinlocks

PEGASUS_USING_STD;
PEGASUS_USING_PEGASUS;

// Some get_proc defines
#define GET_PROC_BY_PID true
#define GET_PROC_BY_INDEX false

/* ==========================================================================
   Type Definitions.
   ========================================================================== */
typedef struct peg_proc_status {
    char pst_ucomm[16];
    String pst_cmd;
    char   pst_stat;
    int    pst_pid;
    int   pst_ppid;
    int   pst_uid;
    int   pst_gid;
    int   pst_sid;
    int   pst_pgrp;
    int   pst_tty;
    unsigned long   pst_vdsize;
    unsigned long   pst_vssize;
    unsigned long   pst_vmmsize;
    unsigned long   pst_start;
    unsigned long   pst_dsize;
    long pst_stime;                //kernel time
    long pst_utime;
    long pst_cutime;
    long pst_cstime;
    long  pst_pri;
    long  pst_nice;
    long  pst_vshmsize;
    long  pst_tsize;
    unsigned int  pst_pctcpu;
    long size;

    struct peg_proc_status *l, *r; // linked list pointers
} peg_proc_t;

int file2str(char *directory, char *myFile, char *ret, int cap);
void parseProcStatm(char* inputFileString, peg_proc_t* P);
Boolean parseProcStat(char* inputFileString, peg_proc_t* P);
void parseProcStatus (char* inputFileString, peg_proc_t* P);
Boolean get_proc(peg_proc_t* P, int &pIndex , Boolean find_by_pid);
void doPercentCPU(char *inputFileString, peg_proc_t *P);

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

  // Loads the internal process status structure with
  // the status data for the indexed process and, if
  // necessary (on HP-UX) updates pIndex to skip unused
  // entries so that a simple increment will allow next
  // call to access next entry
  Boolean loadProcessInfo(int &pIndex);

  // Finds the requested process and loads its info into
  // the internal process status structure
  Boolean findProcess(const String& handle);

  String getHandle() const;

  String getCurrentTime() const;

  String getOSName() const;

  String getCSName() const;

private:
  peg_proc_t pInfo;
};

#endif  /* #ifndef PG_PROCESS_LINUX_H */
