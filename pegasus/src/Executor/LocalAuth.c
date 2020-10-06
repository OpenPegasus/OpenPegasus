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

#include "LocalAuth.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <pthread.h>
#include "Defines.h"
#include "Strlcpy.h"
#include "Strlcat.h"
#include "User.h"
#include "Random.h"
#include "Log.h"
#include "User.h"

#define TOKEN_LENGTH 40

/*
**==============================================================================
**
** BuildLocalAuthFilePath()
**
**     This function generates an appropriate name for a local authentication
**     file for the given *user*.  The file path has the following format:
**
**         PEGASUS_LOCAL_AUTH_DIR/cimclient_<user>_<timestamp>_<seq>
**
**==============================================================================
*/

static void BuildLocalAuthFilePath(
    const char* user,
    char path[EXECUTOR_BUFFER_SIZE])
{
    static unsigned int _nextSeq = 1;
    static pthread_mutex_t _nextSeqMutex = PTHREAD_MUTEX_INITIALIZER;
    unsigned int seq;
    struct timeval tv;
    char buffer[EXECUTOR_BUFFER_SIZE];

    /* Assign next sequence number. */

    pthread_mutex_lock(&_nextSeqMutex);
    seq = _nextSeq++;
    pthread_mutex_unlock(&_nextSeqMutex);

    /* Get microseconds elapsed since epoch. */

    gettimeofday(&tv, NULL);

    /* Build path */

    Strlcpy(path, PEGASUS_LOCAL_AUTH_DIR, EXECUTOR_BUFFER_SIZE);
    Strlcat(path, "/cimclient_", EXECUTOR_BUFFER_SIZE);
    Strlcat(path, user, EXECUTOR_BUFFER_SIZE);
    sprintf(buffer, "_%u_%u", seq, (unsigned int)(tv.tv_usec / 1000));
    Strlcat(path, buffer, EXECUTOR_BUFFER_SIZE);
}

/*
**==============================================================================
**
** CreateLocalAuthFile()
**
**     This function creates a local authentication file with the given *path*
**     and returns 0 on success.
**
**     The algorithm:
**
**         1. Generate a random token
**            (e.g., 8F85CB1129B2B93F77F5CCA16850D659CCD16FE0).
**
**         2. Create the file (owner=root, permissions=0400).
**
**         3. Write random token to file.
**
**         4. Change file owner to *uid* and group to *gid*.
**
**==============================================================================
*/

int CreateLocalAuthFile(
    const char* path,
    int uid,
    int gid)
{
    char token[TOKEN_LENGTH+1];
    int fd;

    /* Generate random token. */

    {
        unsigned char data[TOKEN_LENGTH/2];
        FillRandomBytes(data, sizeof(data));
        RandBytesToHexASCII(data, sizeof(data), token);
    }

    /* If file already exists, remove it. */

    /* Flawfinder: ignore */
    if (access(path, F_OK) == 0 && unlink(path) != 0)
        return -1;

    /* Create the file as read-only by user. */

    fd = open(path, O_WRONLY | O_EXCL | O_CREAT | O_TRUNC, S_IRUSR);

    if (fd < 0)
        return -1;

    /* Write the random token. */

    if (write(fd, token, TOKEN_LENGTH) != TOKEN_LENGTH)
    {
        close(fd);
        unlink(path);
        return -1;
    }

    /* Change owner of file. */

    if (fchown(fd, uid, gid) != 0)
    {
        close(fd);
        unlink(path);
        return -1;
    }

    close(fd);
    return 0;
}

/*
**==============================================================================
**
** CheckLocalAuthToken()
**
**     Compare the *token* with the token in the given file. Return 0 if they
**     are identical.
**
**==============================================================================
*/

int CheckLocalAuthToken(
    const char* path,
    const char* token)
{
    char buffer[TOKEN_LENGTH+1];
    int fd;

    /* Open the file. */

    if ((fd = open(path, O_RDONLY)) < 0)
        return -1;

    /* Read the token. */

    if (read(fd, buffer, TOKEN_LENGTH) != TOKEN_LENGTH)
    {
        close(fd);
        return -1;
    }

    buffer[TOKEN_LENGTH] = '\0';

    /* Compare the token. */

    if (strcmp(token, buffer) != 0)
    {
        close(fd);
        return -1;
    }

    /* Okay! */
    close(fd);
    return 0;
}

/*
**==============================================================================
**
** StartLocalAuthentication()
**
**     Initiate first phase of local authentication.
**
**==============================================================================
*/

int StartLocalAuthentication(
    const char* user,
    char challengeFilePath[EXECUTOR_BUFFER_SIZE])
{
    /* Get uid: */

    int uid;
    int gid;

    if (GetUserInfo(user, &uid, &gid) != 0)
        return -1;

    /* Build an appropriate local authentication file path. */

    BuildLocalAuthFilePath(user, challengeFilePath);

    /* Create the local authentication file. */

    return CreateLocalAuthFile(challengeFilePath, uid, gid);
}

/*
**==============================================================================
**
** FinishLocalAuthentication()
**
**     Initiates second and final phase of local authentication.  Returns 0
**     if authentication is successful, -1 otherwise.
**
**==============================================================================
*/

int FinishLocalAuthentication(
    const char* challengeFilePath,
    const char* response)
{
    /* Check token against the one in the file. */

    int rc = CheckLocalAuthToken(challengeFilePath, response);

    /* Clean up the file now that the authentication is complete. */

    unlink(challengeFilePath);

    return rc;
}
