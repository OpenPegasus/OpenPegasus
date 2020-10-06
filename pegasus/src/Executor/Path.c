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

#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <string.h>
#include "Path.h"
#include "Strlcpy.h"
#include "Strlcat.h"
#include "Config.h"
#include "Defines.h"

/*
**==============================================================================
**
** DirName()
**
**     Remove the trailing component from the path (like the Unix dirname
**     command).
**
**         /a => /
**         /a/ => /
**         /a/b => /a
**         /a/b/foo.conf => /a/b
**
**==============================================================================
*/

void DirName(const char* path1, char path2[EXECUTOR_BUFFER_SIZE])
{
    char* p;

    /* Copy path1 to path2. */

    Strlcpy(path2, path1, EXECUTOR_BUFFER_SIZE);

    /* Find last slash. */

    p = strrchr(path2, '/');

    /* Handle "." case (empty string or no slashes). */

    if (*path2 == '\0' || p == NULL)
    {
        Strlcpy(path2, ".", EXECUTOR_BUFFER_SIZE);
        return;
    }

    /* Remove trailing slashes. */

    if (p[1] == '\0')
    {
        while (p != path2 && *p == '/')
            *p-- = '\0';
    }

    /* Remove trailing component. */

    p = strrchr(path2, '/');

    if (p)
    {
        if (p == path2)
            p[1] = '\0';

        while (p != path2 && *p == '/')
            *p-- = '\0';
    }
    else
        Strlcpy(path2, ".", EXECUTOR_BUFFER_SIZE);
}

/*
**==============================================================================
**
** GetHomedPath()
**
**     Get the absolute path of the given named file or directory. If already
**     absolute it just returns. Otherwise, it prepends the PEGASUS_HOME
**     environment variable.
**
**==============================================================================
*/

int GetHomedPath(
    const char* name,
    char path[EXECUTOR_BUFFER_SIZE])
{
    const char* home;

    /* If absolute, then use the name as is. */

    if (name && name[0] == '/')
    {
        Strlcpy(path, name, EXECUTOR_BUFFER_SIZE);
        return 0;
    }

    /* Use PEGASUS_HOME to form path. */

    /* Flawfinder: ignore */
    if ((home = getenv("PEGASUS_HOME")) == NULL)
        return -1;

    Strlcpy(path, home, EXECUTOR_BUFFER_SIZE);

    if (name)
    {
        Strlcat(path, "/", EXECUTOR_BUFFER_SIZE);
        Strlcat(path, name, EXECUTOR_BUFFER_SIZE);
    }

    return 0;
}

/*
**==============================================================================
**
** GetPegasusInternalBinDir()
**
**     Get the Pegasus "lbin" directory. This is the directory that contains
**     internal Pegasus programs. Note that administrative tools are contained
**     in the "sbin" directory.
**
**==============================================================================
*/

int GetPegasusInternalBinDir(char path[EXECUTOR_BUFFER_SIZE])
{
    char* p;
    struct stat st;

    /* Make a copy of PEGASUS_PROVIDER_AGENT_PROC_NAME: */

    char buffer[EXECUTOR_BUFFER_SIZE];
    Strlcpy(buffer, PEGASUS_PROVIDER_AGENT_PROC_NAME, sizeof(buffer));

    /* Remove CIMPROVAGT suffix. */

    p = strrchr(buffer, '/');

    if (!p)
        p = buffer;

    *p = '\0';

    /* If buffer path absolute, use this. */

    if (buffer[0] == '/')
    {
        Strlcpy(path, buffer, EXECUTOR_BUFFER_SIZE);
    }
    else
    {
        /* Prefix with PEGASUS_HOME environment variable. */

        /* Flawfinder: ignore */
        const char* home = getenv("PEGASUS_HOME");

        if (!home)
            return -1;

        Strlcpy(path, home, EXECUTOR_BUFFER_SIZE);
        Strlcat(path, "/", EXECUTOR_BUFFER_SIZE);
        Strlcat(path, buffer, EXECUTOR_BUFFER_SIZE);
    }

    /* Fail if no such directory. */

    if (stat(path, &st) != 0)
        return -1;

    if (!S_ISDIR(st.st_mode))
        return -1;

    return 0;
}
