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
#include <fcntl.h>
#include <unistd.h>
#include <sys/time.h>
#include <sys/resource.h>
#include "FileHandle.h"

/*
**==============================================================================
**
** RedirectTerminalIO()
**
**     Redirect stdin, stdout, and stderr to /dev/null.
**
**==============================================================================
*/

void RedirectTerminalIO(void)
{
    /* Close these file descriptors (stdin, stdout, stderr). */

    close(0);
    close(1);
    close(2);

    /* Direct standard input, output, and error to /dev/null: */

    open("/dev/null", O_RDONLY);
    open("/dev/null", O_RDWR);
    open("/dev/null", O_RDWR);
}

/*
**==============================================================================
**
** CloseUnusedDescriptors()
**
**     Close all file descriptors except stdin, stdout, stderr, and the two
**     specified file descriptors.
**
**==============================================================================
*/

void CloseUnusedDescriptors(int fd1, int fd2)
{
    struct rlimit rlim;

    if (getrlimit(RLIMIT_NOFILE, &rlim) == 0)
    {
        int i;

        for (i = 3; i < (int)rlim.rlim_cur; i++)
        {
            if (i != fd1 && i != fd2)
            {
                close(i);
            }
        }
    }
}
