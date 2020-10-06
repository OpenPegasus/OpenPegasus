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
//%/////////////////////////////////////////////////////////////////////////////

/*!
    \file io.c
    \brief General I/O routines.
*/
#include "cmpir_common.h"
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>

#ifdef PEGASUS_OS_TYPE_WINDOWS
# include <winsock2.h>
# include <sys/types.h>
#else
# ifndef PEGASUS_OS_ZOS
#  include <error.h>
# endif
#endif

#include "debug.h"
#include <stdlib.h>
#include "io.h"


//! Writes fixed length buffer into a file descriptor.
/*!
    The function uses continous write(2) calls to write the entire
    buffer into the given file descriptor.

    \param fd the data sink.
    \param buf the data source.
    \param count number of bytes to write.

    \return zero on success.
*/

int io_read_fixed_length (
    int fd,
    PEGASUS_CMPIR_IO_BUFPTR_TYPE * buf,
    size_t count )
{
    ssize_t bytes;

    while (count > 0)
    {
        //invokes read on unix and recv on windows systems
        bytes = PEGASUS_CMPIR_RECV ( fd, (char *)buf, count,0 );
        if (bytes == 0)
        {
            error_at_line (
                0,
                0,
                __FILE__,
                __LINE__,
                "EOF before reading complete "
                "chunk of data from fd: %d",
                fd);
            return -1;
        }
        else if (bytes == -1)
        {
            if (errno != EINTR && errno != EAGAIN)
            {
                error_at_line (
                    0,
                    errno,
                    __FILE__,
                    __LINE__,
                    "could not read all the "
                    "requested data from fd: %d",
                    fd);
                return -1;
            }
        }
        else
        {
            count -= bytes;
#ifndef PEGASUS_OS_ZOS
            buf   += bytes;
#else
            buf   = (void*) ((long) buf + (long) bytes);
#endif
        }
    }
    return 0;
}

//! Reads fixed number of bytes from a file descriptor.
/*!
    The function uses continous read(2) calls to read the requested
    number of bytes from the given file descriptor.

    \param fd the data source.
    \param buf the data buffer.
    \param count number of bytes to read.

    \return zero on success.
*/

int io_write_fixed_length (
    int fd,
    const PEGASUS_CMPIR_IO_BUFPTR_TYPE * buf,
    size_t count )
{
    ssize_t bytes;

    while (count > 0)
    {
        //invokes write on unix and send on windows
        bytes = PEGASUS_CMPIR_SEND(fd,(char *) buf, count, 0 );
        if (bytes == 0)
        {
            error_at_line (
                0,
                0,
                __FILE__,
                __LINE__,
                "EOF before writing complete "
                "chunk of data to fd: %d",
                fd);
            return -1;
        }
        else if (bytes == -1)
        {
            if (errno != EINTR && errno != EAGAIN)
            {
                error_at_line (
                    0,
                    errno,
                    __FILE__,
                    __LINE__,
                    "could not write all the "
                    "requested data to fd: %d",
                    fd);
                return -1;
            }
        }
        else
        {
            count -= bytes;
#ifndef PEGASUS_OS_ZOS
            buf += bytes;
#else
            buf = (void*) ((long) buf + (long) bytes);
#endif
        }
    }
    return 0;
}

/****************************************************************************/

/*** Local Variables:  ***/
/*** mode: C           ***/
/*** c-basic-offset: 8 ***/
/*** End:              ***/

