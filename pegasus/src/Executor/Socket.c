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

#include "Socket.h"
#include <unistd.h>
#include <fcntl.h>
#include <signal.h>
#include <string.h>
#include <errno.h>
#include <sys/socket.h>
#include "Exit.h"
#include "Globals.h"
#include "Defines.h"
#include "Socket.h"

/*
**==============================================================================
**
** CloseOnExec()
**
**     Direct the kernel not to keep the given file descriptor open across
**     exec() system call.
**
**==============================================================================
*/

int CloseOnExec(int fd)
{
    return fcntl(fd, F_SETFD, fcntl(fd, F_GETFD) | FD_CLOEXEC);
}

/*
**==============================================================================
**
** SetNonBlocking()
**
**     Set the given socket into non-blocking mode.
**
**==============================================================================
*/

int SetNonBlocking(int sock)
{
    return fcntl(sock, F_SETFL, fcntl(sock, F_GETFL, 0) | O_NONBLOCK);
}

/*
**==============================================================================
**
** SetBlocking()
**
**     Set the given socket into blocking mode.
**
**==============================================================================
*/

int SetBlocking(int sock)
{
    return fcntl(sock, F_SETFL, fcntl(sock, F_GETFL, 0) & ~O_NONBLOCK);
}

/*
**==============================================================================
**
** WaitForReadEnable()
**
**     Wait until the given socket is read-enabled. Returns 1 if read enabled
**     and 0 on timed out.
**
**==============================================================================
*/

int WaitForReadEnable(int sock, long timeoutMsec)
{
    struct timeval timeout;

    fd_set readSet;
    FD_ZERO(&readSet);
    FD_SET(sock, &readSet);

    timeout.tv_sec = timeoutMsec / 1000;
    timeout.tv_usec = (timeoutMsec % 1000) * 1000;

    return select(sock + 1, &readSet, 0, 0, &timeout);
}

/*
**==============================================================================
**
** _waitForWriteEnable()
**
**     Wait until the given socket is write-enabled. Returns 1 if write enabled
**     and 0 on timed out.
**
**==============================================================================
*/

static int _waitForWriteEnable(int sock, long timeoutMsec)
{
    fd_set writeSet;
    struct timeval timeout;

    FD_ZERO(&writeSet);
    FD_SET(sock, &writeSet);

    timeout.tv_sec = timeoutMsec / 1000;
    timeout.tv_usec = (timeoutMsec % 1000) * 1000;

    return select(sock + 1, 0, &writeSet, 0, &timeout);
}

/*
**==============================================================================
**
** RecvNonBlock()
**
**     Receive at least size bytes from the given non-blocking socket.
**
**==============================================================================
*/

ssize_t RecvNonBlock(
    int sock,
    void* buffer,
    size_t size)
{
    const long TIMEOUT_MSEC = 250;
    size_t r = size;
    char* p = (char*)buffer;

    if (size == 0)
        return -1;

    while (r)
    {
        int status = WaitForReadEnable(sock, TIMEOUT_MSEC);
        ssize_t n;

        if ((globals.signalMask & (1 << SIGTERM)) ||
            (globals.signalMask & (1 << SIGINT)))
        {
            /* Exit on either of these signals. */
            Exit(0);
        }

        if (status == 0)
            continue;

        EXECUTOR_RESTART(read(sock, p, r), n);

        if (n == -1 && errno == EINTR)
            continue;

        if (n == -1)
        {
            if (errno == EWOULDBLOCK)
            {
                size_t total = size - r;

                if (total)
                    return total;

                return -1;
            }
            else
                return -1;
        }
        else if (n == 0)
            return size - r;

        r -= n;
        p += n;
    }

    return size - r;
}

/*
**==============================================================================
**
** SendNonBlock()
**
**     Sends at least size bytes on the given non-blocking socket.
**
**==============================================================================
*/

ssize_t SendNonBlock(
    int sock,
    const void* buffer,
    size_t size)
{
    const long TIMEOUT_MSEC = 250;
    size_t r = size;
    char* p = (char*)buffer;

    while (r)
    {
        int status = _waitForWriteEnable(sock, TIMEOUT_MSEC);
        ssize_t n;

        if ((globals.signalMask & (1 << SIGTERM)) ||
            (globals.signalMask & (1 << SIGINT)))
        {
            /* Exit on either of these signals. */
            Exit(0);
        }

        if (status == 0)
            continue;

        EXECUTOR_RESTART(write(sock, p, r), n);

        if (n == -1)
        {
            if (errno == EWOULDBLOCK)
                return size - r;
            else
                return -1;
        }
        else if (n == 0)
            return size - r;

        r -= n;
        p += n;
    }

    return size - r;
}

/*
**==============================================================================
**
** SendDescriptorArray()
**
**     Send an array of descriptors (file, socket, pipe) to the child process.
**
**==============================================================================
*/

ssize_t SendDescriptorArray(int sock, int descriptors[], size_t count)
{
    struct iovec iov[1];
    char dummy;
    struct msghdr mh;
    int result;
#if defined(HAVE_MSG_CONTROL)
    size_t size;
    char* data;
    struct cmsghdr* cmh;

    /* Allocate space for control header plus descriptors. */

    size = CMSG_SPACE(sizeof(int) * count);
    data = (char*)malloc(size);
    memset(data, 0, size);

    /* Initialize msghdr struct to refer to control data. */

    memset(&mh, 0, sizeof(mh));
    mh.msg_control = data;
    mh.msg_controllen = size;

    /* Fill in the control data struct with the descriptor and other fields. */

    cmh = CMSG_FIRSTHDR(&mh);
    cmh->cmsg_len = CMSG_LEN(sizeof(int) * count);
    cmh->cmsg_level = SOL_SOCKET;
    cmh->cmsg_type = SCM_RIGHTS;
    memcpy((int*)CMSG_DATA(cmh), descriptors, sizeof(int) * count);

#else /* defined(HAVE_MSG_CONTROL) */

    memset(&mh, 0, sizeof(mh));
    mh.msg_accrights = (caddr_t)descriptors;
    mh.msg_accrightslen = count * sizeof(int);

#endif /* defined(HAVE_MSG_CONTROL) */

    /*
     * Prepare to send single dummy byte. It will not be used but we must send
     * at least one byte otherwise the call will fail on some platforms.
     */

    memset(iov, 0, sizeof(iov));
    dummy = '\0';
    iov[0].iov_base = &dummy;
    iov[0].iov_len = 1;
    mh.msg_iov = iov;
    mh.msg_iovlen = 1;

    /* Send message to child. */

    result = sendmsg(sock, &mh, 0);

#if defined(HAVE_MSG_CONTROL)
    free(data);
#endif

    return result == -1 ? -1 : 0;
}

/*
**==============================================================================
**
** CreateSocketPair()
**
**     Send an array of descriptors (file, socket, pipe) to the child process.
**
**==============================================================================
*/

int CreateSocketPair(int pair[2])
{
    return socketpair(AF_UNIX, SOCK_STREAM, 0, pair);
}
