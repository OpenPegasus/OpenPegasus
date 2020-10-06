/*
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
*/

#include <stdio.h>
#include <string.h>
#include "Process.h"
#include "Strlcpy.h"

#if defined(PEGASUS_OS_HPUX)
# include <sys/pstat.h>
#endif

/*
**==============================================================================
**
** GetProcessName()
**
**==============================================================================
*/

#if defined(PEGASUS_OS_HPUX)

int GetProcessName(int pid, char name[EXECUTOR_BUFFER_SIZE])
{
    struct pst_status psts;

    if (pstat_getproc(&psts, sizeof(psts), 0, pid) == -1)
        return -1;

    Strlcpy(name, psts.pst_ucomm, EXECUTOR_BUFFER_SIZE);

    return 0;
}

#elif defined(PEGASUS_PLATFORM_LINUX_GENERIC_GNU)

int GetProcessName(int pid, char name[EXECUTOR_BUFFER_SIZE])
{
    FILE* is;
    char* start;
    char* end;

    /* Read the process name from the file. */

    static char buffer[1024];
    sprintf(buffer, "/proc/%d/stat", pid);

    if ((is = fopen(buffer, "r")) == NULL)
        return -1;

    /* Read the first line of the file. */

    if (fgets(buffer, sizeof(buffer), is) == NULL)
    {
        fclose(is);
        return -1;
    }

    fclose(is);

    /* Extract the PID enclosed in parentheses. */

    start = strchr(buffer, '(');

    if (!start)
        return -1;

    start++;

    end = strchr(start, ')');

    if (!end)
        return -1;

    if (start == end)
        return -1;

    *end = '\0';

    Strlcpy(name, start, EXECUTOR_BUFFER_SIZE);

    return 0;
}

#else
# error "not implemented on this platform."
#endif /* PEGASUS_PLATFORM_LINUX_GENERIC_GNU */

/*
**==============================================================================
**
** ReadPidFile()
**
**==============================================================================
*/

int ReadPidFile(const char* pidFilePath, int* pid)
{
    FILE* is = fopen(pidFilePath, "r");

    if (!is)
        return -1;

    if (fscanf(is, "%d\n", pid) != 1)
    {
        *pid = 0;
    }

    fclose(is);

    if (*pid == 0)
        return -1;

    return 0;
}

/*
**==============================================================================
**
** TestProcessRunning()
**
**     Returns 0 if a process is running with the ID in the specified PID file
**     and with the specified process name.
**
**==============================================================================
*/

int TestProcessRunning(
    const char* pidFilePath,
    const char* processName)
{
    int pid;
    char name[EXECUTOR_BUFFER_SIZE];

    if (ReadPidFile(pidFilePath, &pid) != 0)
        return -1;

    if (GetProcessName(pid, name) != 0 || strcmp(name, processName) != 0)
        return -1;

    return 0;
}
