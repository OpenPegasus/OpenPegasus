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

#ifndef _Executor_Socket_h
#define _Executor_Socket_h

#include <stdlib.h>
#include <sys/socket.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <Executor/Defines.h>

#ifdef PEGASUS_ENABLE_PRIVILEGE_SEPARATION

EXECUTOR_LINKAGE
int SetNonBlocking(int sock);

EXECUTOR_LINKAGE
int SetBlocking(int sock);

EXECUTOR_LINKAGE
int WaitForReadEnable(int sock, long timeoutMsec);

EXECUTOR_LINKAGE
ssize_t RecvNonBlock(int sock, void* buffer, size_t size);

EXECUTOR_LINKAGE
ssize_t SendNonBlock(int sock, const void* buffer, size_t size);

EXECUTOR_LINKAGE
int CloseOnExec(int fd);

EXECUTOR_LINKAGE
ssize_t SendDescriptorArray(int sock, int descriptors[], size_t count);

EXECUTOR_LINKAGE
int CreateSocketPair(int pair[2]);

static int RecvDescriptorArray(int sock, int descriptors[], size_t count)
{
    struct iovec iov[1];
    char dummy;
    struct msghdr mh;
    ssize_t n;
#if defined(HAVE_MSG_CONTROL)
    size_t size;
    char* data;
    struct cmsghdr* cmh;

    /*
     * This control data begins with a cmsghdr struct followed by the data
     * (a descriptor in this case). The union ensures that the data is
     * aligned suitably for the leading cmsghdr struct. The descriptor
     * itself is properly aligned since the cmsghdr ends on a boundary
     * that is suitably aligned for any type (including int).
     *
     *     ControlData = [ cmsghdr | int ]
     */

    size = CMSG_SPACE(sizeof(int) * count);
    data = (char*)malloc(size);

    /* Define a msghdr that refers to the control data, which is filled in
     * by calling recvmsg() below.
     */

    memset(&mh, 0, sizeof(mh));
    mh.msg_control = data;
    mh.msg_controllen = size;

#else /* !defined(HAVE_MSG_CONTROL) */

    memset(&mh, 0, sizeof(mh));
    mh.msg_accrights = (caddr_t)descriptors;
    mh.msg_accrightslen = sizeof(int) * count;

#endif /* defined(HAVE_MSG_CONTROL) */

    /*
     * The other process sends a single-byte message. This byte is not
     * used since we only need the control data (the descriptor) but we
     * must request at least one byte from recvmsg().
     */

    memset(iov, 0, sizeof(iov));
    iov[0].iov_base = &dummy;
    iov[0].iov_len = 1;
    mh.msg_iov = iov;
    mh.msg_iovlen = 1;

    /* Receive the message from the other process. */

    n = recvmsg(sock, &mh, 0);

    if (n <= 0)
    {
#if defined(HAVE_MSG_CONTROL)
        free(data);
#endif
        return -1;
    }

    /* Get a pointer to control message. Return if the header is null or
     * does not contain what we expect.
     */

#if defined(HAVE_MSG_CONTROL)

    cmh = CMSG_FIRSTHDR(&mh);

    if (!cmh ||
        cmh->cmsg_len != CMSG_LEN(sizeof(int) * count) ||
        cmh->cmsg_level != SOL_SOCKET ||
        cmh->cmsg_type != SCM_RIGHTS)
    {
        free(data);
        return -1;
    }

    /* Copy the data: */

    memcpy(descriptors, CMSG_DATA(cmh), sizeof(int) * count);

    free(data);

#else /* !defined(HAVE_MSG_CONTROL) */

    if (mh.msg_accrightslen != sizeof(int) * count)
        return -1;

    memcpy(descriptors, mh.msg_accrights, sizeof(int) * count);

#endif /* defined(HAVE_MSG_CONTROL) */

    return 0;
}

#endif

/*
    These functions are used by the PAM cimservera implementation regardless
    of whether privilege separation is enabled.
*/

static ssize_t RecvBlock(int sock, void* buffer, size_t size)
{
    size_t r = size;
    char* p = (char*)buffer;

    if (size == 0)
        return -1;

    while (r)
    {
        ssize_t n;

        EXECUTOR_RESTART(read(sock, p, r), n);

        if (n == -1)
            return -1;
        else if (n == 0)
            return size - r;

        r -= n;
        p += n;
    }

    return size - r;
}

static ssize_t SendBlock(int sock, void* buffer, size_t size)
{
    size_t r = size;
    char* p = (char*)buffer;

    while (r)
    {
        ssize_t n;
        EXECUTOR_RESTART(write(sock, p, r), n);

        if (n == -1)
            return -1;
        else if (n == 0)
            return size - r;

        r -= n;
        p += n;
    }

    return size - r;
}

#endif /* _Executor_Socket_h */
