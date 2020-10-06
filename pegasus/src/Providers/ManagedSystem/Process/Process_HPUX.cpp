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
// Author: Christopher Neufeld <neufeld@linuxcare.com>
//         David Kennedy       <dkennedy@linuxcare.com>
//
// Modified By:
//         David Kennedy       <dkennedy@linuxcare.com>
//         Christopher Neufeld <neufeld@linuxcare.com>
//         Al Stone, Hewlett-Packard Company <ahs3@fc.hp.com>
//         Jim Metcalfe, Hewlett-Packard Company
//         Carlos Bonilla, Hewlett-Packard Company
//         Mike Glantz, Hewlett-Packard Company <michael_glantz@hp.com>
//         Carol Ann Krug Graves, Hewlett-Packard Company
//             (carolann_graves@hp.com)
//         Roger Kumpf, Hewlett-Packard Company <roger_kumpf@hp.com>
//
//%////////////////////////////////////////////////////////////////////////////

/* ==========================================================================
   Includes.
   ========================================================================== */

#include "ProcessPlatform.h"
#include <errno.h>

PEGASUS_USING_STD;
PEGASUS_USING_PEGASUS;

Process::Process()
{
}

Process::~Process()
{
}

/*
================================================================================
NAME              : getCaption
DESCRIPTION       : returns pst_ucomm, the "executable basename the
                  : process is running" (so no path info)
ASSUMPTIONS       :
PRE-CONDITIONS    :
POST-CONDITIONS   :
NOTES             :
================================================================================
*/
Boolean Process::getCaption(String& s) const
{
  s = pInfo.pst_ucomm;
  return true;
}

/*
================================================================================
NAME              : getDescription
DESCRIPTION       : returns pst_cmd (command line) as a string (contrast
                  : this with ModulePath and Parameters below)
ASSUMPTIONS       :
PRE-CONDITIONS    :
POST-CONDITIONS   :
NOTES             :
================================================================================
*/
Boolean Process::getDescription(String& s) const
{
  s = pInfo.pst_cmd;
  return true;
}

/*
================================================================================
NAME              : getInstallDate
DESCRIPTION       :
ASSUMPTIONS       :
PRE-CONDITIONS    :
POST-CONDITIONS   :
NOTES             :
================================================================================
*/
Boolean Process::getInstallDate(CIMDateTime& d) const
{
  // Not supported. This property is inherited from
  // CIM_ManagedSystemElement, but has no useful meaning
  // for a transient entity such as a process (there is a
  // suitable property CreationDate below
  return false;
}

/*
================================================================================
NAME              : getStatus
DESCRIPTION       :
ASSUMPTIONS       :
PRE-CONDITIONS    :
POST-CONDITIONS   :
NOTES             :
================================================================================
*/
Boolean Process::getStatus(String& s) const
{
  // This property is inherited from CIM_ManagedSystemElement,
  // is not relevant, but the process class has defined a
  // more useful property, below: ExecutionState
  return false;
}

/*
================================================================================
NAME              : getName
DESCRIPTION       :
ASSUMPTIONS       :
PRE-CONDITIONS    :
POST-CONDITIONS   :
NOTES             :
================================================================================
*/
Boolean Process::getName(String& s) const
{
  // We will return the basename of the executable image,
  // rather than the actual command line, since this is
  // reliably the name of an executable image, unlike the
  // first token on the command line, which could be an
  // alias, or could even be something completely irrelevant
  s = pInfo.pst_ucomm;
  return true;
}

/*
================================================================================
NAME              : getPriority
DESCRIPTION       :
ASSUMPTIONS       :
PRE-CONDITIONS    :
POST-CONDITIONS   :
NOTES             :
================================================================================
*/
Boolean Process::getPriority(Uint32& i32) const
{
  i32 = pInfo.pst_pri;
  return true;
}

/*
================================================================================
NAME              : getExecutionState
DESCRIPTION       :
ASSUMPTIONS       :
PRE-CONDITIONS    :
POST-CONDITIONS   :
NOTES             :
================================================================================
*/
Boolean Process::getExecutionState(Uint16& i16) const
{
  /*
     From the MOF for this class:
      [Description (
        "Indicates the current operating condition of the Process. "
        "Values include ready (2), running (3), and blocked (4), "
        "among others."),
       Values {"Unknown", "Other", "Ready", "Running",
               "Blocked", "Suspended Blocked", "Suspended Ready",
               "Terminated", "Stopped", "Growing" },
   */

  enum
  { Unknown,
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

  switch (pInfo.pst_stat)
  {
  case PS_SLEEP:
    i16 = Suspended_Ready;
    break;

  case PS_RUN:
    i16 = Running;
    break;

  case PS_STOP:
    i16 = Stopped;
    break;

  case PS_ZOMBIE:
    i16 = Other; // this is coordinated with OtherExecutionDescription
    break;

  case PS_IDLE:
    i16 = Ready;
    break;

  case PS_OTHER:
    i16 = Other; // This is coordinated with OtherExecutionDescription
    break;

  default:
    i16 = Unknown;
  }
  return true;
}

/*
================================================================================
NAME              : getOtherExecutionDescription
DESCRIPTION       :
ASSUMPTIONS       :
PRE-CONDITIONS    :
POST-CONDITIONS   :
NOTES             :
================================================================================
*/
Boolean Process::getOtherExecutionDescription(String& s) const
{
  switch (pInfo.pst_stat)
  {
  case PS_ZOMBIE:
    s = "Zombie";
    break;

  case PS_OTHER:
    s = "Other";
    break;

  default:
    s = String::EMPTY; // ExecutionState is not Other
    // In this case, the caller must know to set the
    // property value to NULL (XML: no <VALUE> element)
  }

  return true;
}

/*
================================================================================
NAME              : getCreationDate
DESCRIPTION       :
ASSUMPTIONS       :
PRE-CONDITIONS    :
POST-CONDITIONS   :
NOTES             :
================================================================================
*/
Boolean Process::getCreationDate(CIMDateTime& d) const
{
  // convert 64-bit pInfo data to 32-bit needed by localtime_r()
  time_t start = pInfo.pst_start;
  struct tm tmBuffer;
  struct tm *t = localtime_r(&start, &tmBuffer);

  // If localtime_r() failed, we will not return this property
  // There's really no way it can fail for a process start time
  // since the only failures occur when its argument is not a
  // valid time, but if this happened, the system must have had
  // some serious problem to return an invalid time for a
  // process creation time
  if (t == 0) return false;

  // convert to CIMDateTime format
  char timstr[26];
  sprintf(timstr,"%04d%02d%02d%02d%02d%02d.000000%c%03ld",t->tm_year+1900,
                       t->tm_mon+1,
                       t->tm_mday,
                       t->tm_hour,
                       t->tm_min,
                       t->tm_sec,
                       (timezone>0)?'-':'+',
                       labs (timezone/60 - (t->tm_isdst? 60:0)));
  d = CIMDateTime(timstr);
  return true;
}

/*
================================================================================
NAME              : getTerminationDate
DESCRIPTION       :
ASSUMPTIONS       :
PRE-CONDITIONS    :
POST-CONDITIONS   :
NOTES             :
================================================================================
*/
Boolean Process::getTerminationDate(CIMDateTime& d) const
{
  // Not supported: this is a dynamic provider that can only
  // return information on processes while they exist. A
  // different provider that saved information about previously
  // existing processes might fill in this property (if it could)
  return false;
}

/*
================================================================================
NAME              : getKernelModeTime
DESCRIPTION       :
ASSUMPTIONS       :
PRE-CONDITIONS    :
POST-CONDITIONS   :
NOTES             :
================================================================================
*/
Boolean Process::getKernelModeTime(Uint64& i64) const
{
  // time obtained in seconds from pstat needs to be
  // returned in milliseconds
  i64 = 1000 * pInfo.pst_stime;
  return true;
}

/*
================================================================================
NAME              : getUserModeTime
DESCRIPTION       :
ASSUMPTIONS       :
PRE-CONDITIONS    :
POST-CONDITIONS   :
NOTES             :
================================================================================
*/
Boolean Process::getUserModeTime(Uint64& i64) const
{
  // time obtained in seconds from pstat needs to be
  // returned in milliseconds
  i64 = 1000 * pInfo.pst_utime;
  return true;
}

/*
================================================================================
NAME              : getWorkingSetSize
DESCRIPTION       :
ASSUMPTIONS       :
PRE-CONDITIONS    :
POST-CONDITIONS   :
NOTES             :
================================================================================
*/
Boolean Process::getWorkingSetSize(Uint64& i64) const
{
  /*
     From MOF:
      [Gauge, Description (
        "The amount of memory in bytes that a Process needs to "
        "execute efficiently, for an OperatingSystem that uses "
        "page-based memory management.  If an insufficient amount "
        "of memory is available (< working set size), thrashing "
        "will occur.  If this information is not known, NULL or 0 "
        "should be entered.  If this data is provided, it could be "
        "monitored to understand a Process' changing memory "
        "requirements as execution proceeds."),
       Units ("Bytes") ]
   uint64 WorkingSetSize;
   */

  i64 = 0;
  return true;
}

/*
================================================================================
NAME              : getRealUserID
DESCRIPTION       :
ASSUMPTIONS       :
PRE-CONDITIONS    :
POST-CONDITIONS   :
NOTES             :
================================================================================
*/
Boolean Process::getRealUserID(Uint64& i64) const
{
  i64 = pInfo.pst_uid;
  return true;
}

/*
================================================================================
NAME              : getProcessGroupID
DESCRIPTION       :
ASSUMPTIONS       :
PRE-CONDITIONS    :
POST-CONDITIONS   :
NOTES             :
================================================================================
*/
Boolean Process::getProcessGroupID(Uint64& i64) const
{
  i64 = pInfo.pst_gid;
  return true;
}

/*
================================================================================
NAME              : getProcessSessionID
DESCRIPTION       :
ASSUMPTIONS       :
PRE-CONDITIONS    :
POST-CONDITIONS   :
NOTES             :
================================================================================
*/
Boolean Process::getProcessSessionID(Uint64& i64) const
{
  i64 = pInfo.pst_sid;
  return true;
}

/*
================================================================================
NAME              : getProcessTTY
DESCRIPTION       :
ASSUMPTIONS       :
PRE-CONDITIONS    :
POST-CONDITIONS   :
NOTES             :
================================================================================
*/
Boolean Process::getProcessTTY(String& s) const
{
  // ATTN-MG-P3-20020503: The following conversion from major/minor returned
  // by pstat_getproc() to deviceID used by devnm() may
  // not be supported. Also, while the use of major/minor for device
  // identification will continue to be supported, future enhancements
  // to various operating systems might add other important info which
  // is not represented in major/minor, and this code may have to be
  // modified to obtain additional information

  if (pInfo.pst_major == -1 && pInfo.pst_minor == -1)
  {
    s = "?";
    return true;
  }

  dev_t devID = (pInfo.pst_major << 24) + pInfo.pst_minor;
  char result[132];
  int rc = devnm(S_IFCHR, devID, result, sizeof(result)-1, 1);
  s = result;
  return true;
}

/*
================================================================================
NAME              : getModulePath
DESCRIPTION       :
ASSUMPTIONS       :
PRE-CONDITIONS    :
POST-CONDITIONS   :
NOTES             :
================================================================================
*/
Boolean Process::getModulePath(String& s) const
{
  // We don't support this property, because there's no
  // reliable way to get the module path for a command

  // ATTN-MG-P5-20020503: it may be possible to build this
  // from the PATH environment variable and the command
  // basename returned in pst_status.pst_ucomm
  return false;
}

/*
================================================================================
NAME              : getParameters
DESCRIPTION       :
ASSUMPTIONS       :
PRE-CONDITIONS    :
POST-CONDITIONS   :
NOTES             :
================================================================================
*/
Boolean Process::getParameters(Array<String>& as) const
{
  // ATTN-MG-P5-20020503: we return the full command line
  // including the first "argument", which is the command.
  // This could be considered incorrect.

  // start with p at beginning of string
  // q is position of first blank after p
  // loop doesn't execute if q==0
  // otherwise, assigns as[i] to string starting at p and
  // going for q-p characters
  // p then set to point past blank for next iteration

  const char *p = pInfo.pst_cmd;
  const char *q;
  for ( ; (q = strchr(p, ' ')) != 0; p = q + 1)
  {
      as.append(String(p, q-p));
  }

  // when no more blanks found, stick what's left into last
  // element of array
  as.append(String(p));
  return true;
}

/*
================================================================================
NAME              : getProcessNiceValue
DESCRIPTION       :
ASSUMPTIONS       :
PRE-CONDITIONS    :
POST-CONDITIONS   :
NOTES             :
================================================================================
*/
Boolean Process::getProcessNiceValue(Uint32& i32) const
{
  i32 = pInfo.pst_nice;
  return true;
}

/*
================================================================================
NAME              : getProcessWaitingForEvent
DESCRIPTION       :
ASSUMPTIONS       :
PRE-CONDITIONS    :
POST-CONDITIONS   :
NOTES             :
================================================================================
*/
Boolean Process::getProcessWaitingForEvent(String& s) const
{
  // Not supported. It is not feasible to determine what event
  // a process might be waiting for
  return false;
}

/*
================================================================================
NAME              : getCPUTime
DESCRIPTION       :
ASSUMPTIONS       :
PRE-CONDITIONS    :
POST-CONDITIONS   :
NOTES             :
================================================================================
*/
Boolean Process::getCPUTime(Uint32& i32) const
{
  // pst_pctcpu is a float between 0 and 1, 1=100%
  // CPUTime property is the percent
  i32 = pInfo.pst_pctcpu * 100;
  return true;
}

/*
================================================================================
NAME              : getRealText
DESCRIPTION       :
ASSUMPTIONS       :
PRE-CONDITIONS    :
POST-CONDITIONS   :
NOTES             :
================================================================================
*/
Boolean Process::getRealText(Uint64& i64) const
{
  // ATTN-MG-P4-20020503:
  // For this and other functions that call getpagesize(),
  // this need not be done if the page size can't change; the
  // value could be acquired once. But on systems where this
  // can change and be different from different processes, or
  // even different segments of the address space of a process,
  // this code will not work, because it is getting the page
  // size of the cimom!
  i64 = pInfo.pst_tsize * getpagesize() / 1024;
  return true;
}

/*
================================================================================
NAME              : getRealData
DESCRIPTION       :
ASSUMPTIONS       :
PRE-CONDITIONS    :
POST-CONDITIONS   :
NOTES             :
================================================================================
*/
Boolean Process::getRealData(Uint64& i64) const
{
  i64 = pInfo.pst_dsize * getpagesize() / 1024;
  return true;
}

/*
================================================================================
NAME              : getRealStack
DESCRIPTION       :
ASSUMPTIONS       :
PRE-CONDITIONS    :
POST-CONDITIONS   :
NOTES             :
================================================================================
*/
Boolean Process::getRealStack(Uint64& i64) const
{
  i64 = pInfo.pst_ssize * getpagesize() / 1024;
  return true;
}

/*
================================================================================
NAME              : getVirtualText
DESCRIPTION       :
ASSUMPTIONS       :
PRE-CONDITIONS    :
POST-CONDITIONS   :
NOTES             :
================================================================================
*/
Boolean Process::getVirtualText(Uint64& i64) const
{
  i64 = pInfo.pst_vtsize * getpagesize() / 1024;
  return true;
}

/*
================================================================================
NAME              : getVirtualData
DESCRIPTION       :
ASSUMPTIONS       :
PRE-CONDITIONS    :
POST-CONDITIONS   :
NOTES             :
================================================================================
*/
Boolean Process::getVirtualData(Uint64& i64) const
{
  i64 = pInfo.pst_vdsize * getpagesize() / 1024;
  return true;
}

/*
================================================================================
NAME              : getVirtualStack
DESCRIPTION       :
ASSUMPTIONS       :
PRE-CONDITIONS    :
POST-CONDITIONS   :
NOTES             :
================================================================================
*/
Boolean Process::getVirtualStack(Uint64& i64) const
{
  i64 = pInfo.pst_vssize * getpagesize() / 1024;
  return true;
}

/*
================================================================================
NAME              : getVirtualMemoryMappedFileSize
DESCRIPTION       :
ASSUMPTIONS       :
PRE-CONDITIONS    :
POST-CONDITIONS   :
NOTES             :
================================================================================
*/
Boolean Process::getVirtualMemoryMappedFileSize(Uint64& i64) const
{
  i64 = pInfo.pst_vmmsize * getpagesize() / 1024;
  return true;
}

/*
================================================================================
NAME              : getVirtualSharedMemory
DESCRIPTION       :
ASSUMPTIONS       :
PRE-CONDITIONS    :
POST-CONDITIONS   :
NOTES             :
================================================================================
*/
Boolean Process::getVirtualSharedMemory(Uint64& i64) const
{
  i64 = pInfo.pst_vshmsize * getpagesize() / 1024;
  return true;
}

/*
================================================================================
NAME              : getCpuTimeDeadChildren
DESCRIPTION       :
ASSUMPTIONS       :
PRE-CONDITIONS    :
POST-CONDITIONS   :
NOTES             :
================================================================================
*/
Boolean Process::getCpuTimeDeadChildren(Uint64& i64) const
{

// ATTN-MG-P4-20020503: this field seems not to be available
// on system we are using. It may be possible to obtain this
// field on a system on which the necessary defines are present

#ifdef _RUSAGE_EXTENDED
  // value to be returned in clock ticks, not time
  i64 = ((Uint64)pInfo.pst_child_usercycles.psc_hi << 32) +
          (Uint64)pInfo.pst_child_usercycles.psc_lo +
        ((Uint64)pInfo.pst_child_systemcycles.psc_hi << 32) +
          (Uint64)pInfo.pst_child_systemcycles.psc_lo +
        ((Uint64)pInfo.pst_child_interruptcycles.psc_hi << 32) +
          (Uint64)pInfo.pst_child_interruptcycles.psc_lo;
  return true;
#else
  // not supported
  return false;
#endif
}

/*
================================================================================
NAME              : getSystemTimeDeadChildren
DESCRIPTION       :
ASSUMPTIONS       :
PRE-CONDITIONS    :
POST-CONDITIONS   :
NOTES             :
================================================================================
*/
Boolean Process::getSystemTimeDeadChildren(Uint64& i64) const
{

// ATTN-MG-P4-20020503: this field seems not to be available
// on system we are using. It may be possible to obtain this
// field on a system on which the necessary defines are present

#ifdef _RUSAGE_EXTENDED
  // these values are to be returned in clock ticks, not
  // time
  i64 = ((Uint64)pInfo.pst_child_systemcycles.psc_hi << 32) +
          (Uint64)pInfo.pst_child_systemcycles.psc_lo;
  return true;
#else
  // not supported
  return false;
#endif
}

/*
================================================================================
NAME              : getParentProcessID
DESCRIPTION       :
ASSUMPTIONS       :
PRE-CONDITIONS    :
POST-CONDITIONS   :
NOTES             :
================================================================================
*/
Boolean Process::getParentProcessID(String& s) const
{
  char buf[22];
  sprintf(buf, "%lld", Sint64(pInfo.pst_ppid));
  s = buf;
  return true;
}

/*
================================================================================
NAME              : getRealSpace
DESCRIPTION       :
ASSUMPTIONS       :
PRE-CONDITIONS    :
POST-CONDITIONS   :
NOTES             : returns a modified pIndex that can be used in a
                    subsequent call to get next process structure
================================================================================
*/
Boolean Process::getRealSpace(Uint64& i64) const
{
  // total up all pages used, compute Kbyte (1024 = 1K)
  i64 = (pInfo.pst_dsize +  // real data
         pInfo.pst_tsize +  // real text
         pInfo.pst_ssize)   // real stack
        * getpagesize() / 1024;
  return true;
}

/*
================================================================================
NAME              : getHandle
DESCRIPTION       :
ASSUMPTIONS       : None
PRE-CONDITIONS    :
POST-CONDITIONS   :
NOTES             :
================================================================================
*/
String Process::getHandle(void) const
{
  char buf[22];
  sprintf(buf, "%lld", Sint64(pInfo.pst_pid));
  return String(buf);
}

/*
================================================================================
NAME              : getCSName
DESCRIPTION       : Platform-specific routine to get CSName
ASSUMPTIONS       : None
PRE-CONDITIONS    :
POST-CONDITIONS   :
NOTES             :
================================================================================
*/
String Process::getCSName(void) const
{
  struct hostent *he;
  char hn[PEGASUS_MAXHOSTNAMELEN + 1];

  // fill in hn with what this system thinks is its name
  gethostname(hn, sizeof(hn));
  hn[sizeof(hn)-1] = 0;

  // find out what the nameservices think is its full name
  if ((he = gethostbyname(hn)) != 0)
  {
      return String(he->h_name);
  }
  // but if that failed, return what gethostname said
  else
  {
      return String(hn);
  }
}

/*
================================================================================
NAME              : getOSName
DESCRIPTION       : Platform-specific routine to get OSName
ASSUMPTIONS       : None
PRE-CONDITIONS    :
POST-CONDITIONS   :
NOTES             :
================================================================================
*/
String Process::getOSName(void) const
{
  struct utsname unameInfo;

  /* Call uname, handle errors */
  if ((uname(&unameInfo) < 0) && (errno != EOVERFLOW))
  {
    throw CIMOperationFailedException(strerror(errno));
  }
  return String(unameInfo.sysname);
}


/*
================================================================================
NAME              : getCurrentTime
DESCRIPTION       : Platform-specific routine to get a timestamp stat Name key
ASSUMPTIONS       : None
PRE-CONDITIONS    :
POST-CONDITIONS   :
NOTES             :
================================================================================
*/
String Process::getCurrentTime(void) const
{
  time_t t = time(0);
  char buffer[40];
  return String(ctime_r(&t, buffer));
}


/*
================================================================================
NAME              : loadProcessInfo
DESCRIPTION       : Use pstat_getproc() to fill in a struct pst_status
ASSUMPTIONS       : None
PRE-CONDITIONS    :
POST-CONDITIONS   :
NOTES             :
================================================================================
*/
Boolean Process::loadProcessInfo(int &pIndex)
{
  // This routine fills in the protected member pInfo by calling
  // pstat_getproc. Because HP-UX process entries are not contiguous
  // this routine modified pIndex so that the caller, after
  // incrementing pIndex, will be able to fetch the next process in
  // a subsequent call. The routine is functionally equivalent to
  // pstat_getproc(), except that it hides the platform-specific
  // structure pst_status.
  // It returns true if it succeeded in fetching a process, otherwise
  // false, incidating that there are no more processes to be fetched.

  // pstat_getproc() takes an empty struct to fill in, each element's size,
  // the number of elements (0 if last arg is pid instead of index),
  // and an index to start at

  int stat = pstat_getproc(&pInfo, sizeof(pInfo), 1, pIndex);

  // pstat_getproc returns number of returned structures
  // if this was not 1, it means that we are at the end
  // of the process entry table, and the caller should not
  // use data from this call
  if (stat != 1) return false;

  pIndex = pInfo.pst_idx;
  return true;
}


/*
================================================================================
NAME              : findProcess
DESCRIPTION       : find the requested process and load its data
ASSUMPTIONS       : None
PRE-CONDITIONS    :
POST-CONDITIONS   :
NOTES             :
================================================================================
*/
Boolean Process::findProcess(const String& handle)
{
  int pIndex;
  int stat;

  // Convert handle to an integer
  int pid = atoi(handle.getCString());

  // loop ends with stat==0, meaning number of entries
  // returned was zero
  // if this loop finishes, we haven't found the process
  for (pIndex=0, stat=1; stat!=0; pIndex++)
  {
    // pstat_getproc() fills in ps with a process entry's
    // data, and returns the number of entries filled in,
    // so that anything other than 1 is a problem
    stat = pstat_getproc(&pInfo, sizeof(pInfo), 1, pIndex);

    // we can return right now if we found it
    if (pid == pInfo.pst_pid) return true;

    // Need to set pIndex to skip empty entries, because
    // HP-UX doesn't necessarily store entries contiguously
    pIndex = pInfo.pst_idx;
  }

  // we finished the loop without finding the process
  // signal this to the caller
  return false;
}
