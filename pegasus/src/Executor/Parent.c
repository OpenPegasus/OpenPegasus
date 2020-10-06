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

#include <sys/time.h>
#include <sys/resource.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>
#include <string.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <unistd.h>
#include <signal.h>
#include "Parent.h"
#include "Log.h"
#include "Messages.h"
#include "Socket.h"
#include "Fatal.h"
#include "Globals.h"
#include "Path.h"
#include "User.h"
#include "Exit.h"
#include "Strlcpy.h"
#include "LocalAuth.h"
#include "Strlcat.h"
#include "PasswordFile.h"
#include "Policy.h"
#include "Macro.h"
#include "FileHandle.h"

#if defined(PEGASUS_PAM_AUTHENTICATION)
# include "PAMAuth.h"
#else
/* PAM_SUCCESS is defined to 0 by PAM */
#define PAM_SUCCESS 0
#endif

/*
**==============================================================================
**
** _sigHandler()
**
**     Signal handler for SIGTERM.
**
**==============================================================================
*/

static void _sigHandler(int signum)
{
    globals.signalMask |= (1UL << signum);
}

/*
**==============================================================================
**
** ReadExecutorRequest()
**
**     Read a request of the specified size from the specified socket into the
**     buffer provided.
**
**==============================================================================
*/

static void ReadExecutorRequest(int sock, void* buffer, size_t size)
{
    if (RecvNonBlock(sock, buffer, size) != (ssize_t)size)
    {
        Fatal(FL, "Failed to read request");
    }
}

/*
**==============================================================================
**
** WriteExecutorResponse()
**
**     Write a response of the specified size from the given buffer onto the
**     specified socket.
**
**==============================================================================
*/

static void WriteExecutorResponse(int sock, const void* buffer, size_t size)
{
    if (SendNonBlock(sock, buffer, size) != (ssize_t)size)
    {
        Fatal(FL, "Failed to write response");
    }
}

/*
**==============================================================================
**
** HandlePingRequest()
**
**     Handle ping request.
**
**==============================================================================
*/

static void HandlePingRequest(int sock)
{
    struct ExecutorPingResponse response;

    memset(&response, 0, sizeof(response));
    response.magic = EXECUTOR_PING_MAGIC;

    Log(LL_TRACE, "HandlePingRequest()");

    WriteExecutorResponse(sock, &response, sizeof(response));
}

/*
**==============================================================================
**
** HandleOpenFileRequest()
**
**     Handle a request from a child to open a file.
**
**==============================================================================
*/

static void HandleOpenFileRequest(int sock)
{
    struct ExecutorOpenFileRequest request;
    struct ExecutorOpenFileResponse response;
    int fd;

    memset(&response, 0, sizeof(response));

    /* Read the request message. */

    ReadExecutorRequest(sock, &request, sizeof(request));

    /* Log the request. */

    Log(LL_TRACE, "HandleOpenFileRequest(): path=\"%s\", mode='%c'",
        request.path, request.mode);

    /* Perform the operation. */

    response.status = 0;
    fd = -1;

    do
    {
        /* Check the policy. */

        unsigned long permissions = 0;

        if (CheckOpenFilePolicy(request.path, request.mode, &permissions) != 0)
        {
            response.status = -1;
            break;
        }

        /* Open file. */

        switch (request.mode)
        {
            case 'r':
                fd = open(request.path, O_RDONLY);
                break;

            case 'w':
                fd = open(
                    request.path,
                    O_WRONLY | O_TRUNC,
                    permissions);
                break;

            case 'a':
            {
                fd = open(
                    request.path,
                    O_WRONLY | O_APPEND,
                    permissions);
                break;
            }
        }
        /* If the open call fails with ENOENT errno,then create the file.
        If the umask is set to a non default value,then the file will not
        get created with permissions specified in the open system call.
        So set the permissions for the file explicitly using fchmod.
        */
        if (fd == -1)
        {
            if (errno == ENOENT && (request.mode == 'w' || request.mode == 'a'))
            {
                fd = open(request.path,O_CREAT | O_WRONLY,permissions);
                if (fchmod(fd,permissions) != 0)
                {
                    response.status = -1;
                    close(fd);
                }
            }
            else
            {
                response.status = -1;
            }
        }
    }
    while (0);

    /* Log failure. */

    if (response.status != 0)
    {
        Log(LL_WARNING, "open(\"%s\", '%c') failed",
            request.path, request.mode);
    }

    /* Send response. */

    WriteExecutorResponse(sock, &response, sizeof(response));

    /* Send descriptor to calling process (if any to send). */

    if (response.status == 0)
    {
        int descriptors[1];
        descriptors[0] = fd;
        SendDescriptorArray(sock, descriptors, 1);
        close(fd);
    }
}

/*
**==============================================================================
**
** HandleStartProviderAgentRequest()
**
**==============================================================================
*/

static void HandleStartProviderAgentRequest(int sock)
{
    int status;
    int pid;
    int to[2];
    int from[2];
    struct ExecutorStartProviderAgentResponse response;
    struct ExecutorStartProviderAgentRequest request;

    memset(&response, 0, sizeof(response));

    /* Read request. */

    ReadExecutorRequest(sock, &request, sizeof(request));

    /* Log request. */

    Log(LL_TRACE, "HandleStartProviderAgentRequest(): "
        "module=%s userName=%s", request.module, request.userName);

    /* Process request. */

    status = 0;
    pid = -1;

    do
    {
        /* Resolve full path of CIMPROVAGT. */

        const char* path;

        if (request.moduleBitness == BITNESS_DEFAULT)
        {
            if ((path = FindMacro("cimprovagtPath")) == NULL)
                Fatal(FL, "Failed to locate %s program", CIMPROVAGT);
        }
        else if (request.moduleBitness == BITNESS_32)
        {
            if ((path = FindMacro("cimprovagt32Path")) == NULL)
                Fatal(FL, "Failed to locate %s program", CIMPROVAGT32);
        }
        else
        {
            status = -1;
            break;
        }
#if !defined(PEGASUS_DISABLE_PROV_USERCTXT)

        /* Look up the user ID and group ID of the specified user. */

        int uid;
        int gid;
        if (GetUserInfo(request.userName, &uid, &gid) != 0)
        {
            Log(LL_WARNING, "User %s does not exist on this system, " 
                "hence cannot start the provider agent %s", 
                request.userName, request.module);
            status = -1;
            break;
        }

        Log(LL_TRACE, "cimprovagt user context: "
            "userName=%s uid=%d gid=%d", request.userName, uid, gid);

#endif /* !defined(PEGASUS_DISABLE_PROV_USERCTXT) */

        /* Create "to-agent" pipe: */

        if (pipe(to) != 0)
        {
            status = -1;
            break;
        }

        /* Create "from-agent" pipe: */

        if (pipe(from) != 0)
        {
            close(to[0]);
            close(to[1]);
            status = -1;
            break;
        }

        /* Fork process: */

        pid = fork();

        if (pid < 0)
        {
            Log(LL_SEVERE, "fork failed");
            close(to[0]);
            close(to[1]);
            close(from[0]);
            close(from[1]);
            status = -1;
            break;
        }

        /* If child: */

        if (pid == 0)
        {
            char toPipeArg[32];
            char fromPipeArg[32];
            /* The user context is set here; no need to do it in cimprovagt. */
            const char* setUserContextFlag = "0";

            /* Close unused pipe descriptors: */

            close(to[1]);
            close(from[0]);

            /* Redirect terminal I/O if required and not yet daemonized. */

            if (globals.initCompletePipe != -1 && !globals.bindVerbose)
            {
                RedirectTerminalIO();
            }

            /*
             * Close unused descriptors. Leave stdin, stdout, stderr, and the
             * child's pipe descriptors open.
             */

            CloseUnusedDescriptors(to[0], from[1]);

#if !defined(PEGASUS_DISABLE_PROV_USERCTXT)

            SetUserContext(request.userName, uid, gid);

            Log(LL_TRACE, "starting %s on module %s as user %s",
                path, request.module, request.userName);

#endif /* !defined(PEGASUS_DISABLE_PROV_USERCTXT) */

            /* Exec the CIMPROVAGT program. */

            sprintf(toPipeArg, "%d", to[0]);
            sprintf(fromPipeArg, "%d", from[1]);

            Log(LL_TRACE, "execl(%s, %s, %s, %s, %s, %s, %s)\n",
                path,
                path,
                setUserContextFlag,
                toPipeArg,
                fromPipeArg,
                request.userName,
                request.module);

            /* Flawfinder: ignore */
            execl(
                path,
                path,
                setUserContextFlag,
                toPipeArg,
                fromPipeArg,
                request.userName,
                request.module,
                (char*)0);

            Log(LL_SEVERE, "execl(%s, %s, %s, %s, %s, %s, %s): failed\n",
                path,
                path,
                setUserContextFlag,
                toPipeArg,
                fromPipeArg,
                request.userName,
                request.module);
            _exit(1);
        }

        /* We are the parent process.  Close the child's ends of the pipes. */

        close(to[0]);
        close(from[1]);
    }
    while (0);

    /* Send response. */

    response.status = status;
    response.pid = pid;

    WriteExecutorResponse(sock, &response, sizeof(response));

    /* Send descriptors to calling process. */

    if (response.status == 0)
    {
        int descriptors[2];
        descriptors[0] = from[0];
        descriptors[1] = to[1];

        SendDescriptorArray(sock, descriptors, 2);
        close(from[0]);
        close(to[1]);
    }
}

/*
**==============================================================================
**
** HandleDaemonizeExecutorRequest()
**
**==============================================================================
*/

static void HandleDaemonizeExecutorRequest(int sock)
{
    struct ExecutorDaemonizeExecutorResponse response;

    memset(&response, 0, sizeof(response));

    Log(LL_TRACE, "HandleDaemonizeExecutorRequest()");

    if (!globals.bindVerbose)
    {
        RedirectTerminalIO();
    }

    if (globals.initCompletePipe != -1)
    {
        ssize_t result;

        EXECUTOR_RESTART(write(globals.initCompletePipe, "\0", 1), result);
        close(globals.initCompletePipe);
        globals.initCompletePipe = -1;
    }

    response.status = 0;

    WriteExecutorResponse(sock, &response, sizeof(response));
}

/*
**==============================================================================
**
** HandleRenameFileRequest()
**
**==============================================================================
*/

static void HandleRenameFileRequest(int sock)
{
    struct ExecutorRenameFileResponse response;
    struct ExecutorRenameFileRequest request;

    memset(&response, 0, sizeof(response));

    /* Read the request message. */

    ReadExecutorRequest(sock, &request, sizeof(request));

    /* Log request. */

    Log(LL_TRACE, "HandleRenameFileRequest(): oldPath=%s newPath=%s",
        request.oldPath, request.newPath);

    /* Perform the operation: */

    response.status = 0;

    do
    {
        /* Check the policy. */

        if (CheckRenameFilePolicy(request.oldPath, request.newPath) != 0)
        {
            response.status = -1;
            break;
        }

        /* Rename the file. */

        if (rename(request.oldPath, request.newPath) != 0)
        {
            response.status = -1;
            break;
        }
    }
    while (0);

    /* Send response message. */

    WriteExecutorResponse(sock, &response, sizeof(response));
}

/*
**==============================================================================
**
** HandleRemoveFileRequest()
**
**==============================================================================
*/

static void HandleRemoveFileRequest(int sock)
{
    struct ExecutorRemoveFileRequest request;
    struct ExecutorRemoveFileResponse response;

    memset(&response, 0, sizeof(response));

    /* Read the request message. */

    ReadExecutorRequest(sock, &request, sizeof(request));

    /* Log request. */

    Log(LL_TRACE, "HandleRemoveFileRequest(): path=%s", request.path);

    response.status = 0;

    do
    {
        /* Check the policy. */

        if (CheckRemoveFilePolicy(request.path) != 0)
        {
            response.status = -1;
            break;
        }

        /* Remove the file. */

        if (unlink(request.path) != 0)
        {
            response.status = -1;
            Log(LL_WARNING, "unlink(%s) failed", request.path);
            break;
        }
    }
    while (0);

    /* Send response message. */

    WriteExecutorResponse(sock, &response, sizeof(response));
}

/*
**==============================================================================
**
** HandleAuthenticatePasswordRequest()
**
**==============================================================================
*/

static void HandleAuthenticatePasswordRequest(int sock)
{
    int status;
    struct ExecutorAuthenticatePasswordRequest request;
    struct ExecutorAuthenticatePasswordResponse response;

    memset(&response, 0, sizeof(response));

    /* Read the request message. */

    ReadExecutorRequest(sock, &request, sizeof(request));

    /* Log request. */

    Log(LL_TRACE, "HandleAuthenticatePasswordRequest(): username=%s",
        request.username);

    /* Perform the operation: */

    status = 0;

    do
    {

#if defined(PEGASUS_PAM_AUTHENTICATION)

        status = PAMAuthenticate(request.username, request.password);

        if (status == PAM_SUCCESS)
        {
            Log(LL_TRACE,
                "Basic authentication through PAM: "
                    "username = %s, successful.",
                request.username);
        }
        else
        {
            Log(LL_TRACE,
                "Basic authentication through PAM: "
                    "username = %s, failed with PAM return code= %d.",
                request.username,
                status);
        }
        
#else /* !PEGASUS_PAM_AUTHENTICATION */

        {
            int gid;
            int uid;
            if (GetUserInfo(request.username, &uid, &gid) != 0)
            {
                status = -1;
                break;
            }

            const char* path = FindMacro("passwordFilePath");

            if (!path)
            {
                status = -1;
                break;
            }

            if (CheckPasswordFile(
                path, request.username, request.password) != 0)
            {
                status = -1;
                break;
            }
        }

        Log(LL_TRACE, "Basic authentication attempt: username = %s, "
            "successful = %s",
            request.username, status == PAM_SUCCESS ? "TRUE" : "FALSE" );

#endif /* !PEGASUS_PAM_AUTHENTICATION */
    }
    while (0);

    /* Send response message. */

    response.status = status;

    WriteExecutorResponse(sock, &response, sizeof(response));
}

/*
**==============================================================================
**
** HandleValidateUserRequest()
**
**==============================================================================
*/

static void HandleValidateUserRequest(int sock)
{
    int status;
    struct ExecutorValidateUserResponse response;
    struct ExecutorValidateUserRequest request;

    memset(&response, 0, sizeof(response));

    /* Read the request message. */

    ReadExecutorRequest(sock, &request, sizeof(request));

    /* Validate the user. */

    Log(LL_TRACE,
        "HandleValidateUserRequest(): username=%s", request.username);

    /* Get the uid for the username. */

    status = 0;

#if defined(PEGASUS_PAM_AUTHENTICATION)

    status = PAMValidateUser(request.username);

#else /* !PEGASUS_PAM_AUTHENTICATION */

    do
    {
        const char* path = FindMacro("passwordFilePath");

        if (!path)
        {
            status = -1;
            break;
        }

        if (CheckPasswordFile(path, request.username, NULL) != 0)
        {
            status = -1;
            break;
        }
    }
    while (0);

#endif /* !PEGASUS_PAM_AUTHENTICATION */

    /* Send response message. */

    response.status = status;

    WriteExecutorResponse(sock, &response, sizeof(response));
}

/*
**==============================================================================
**
** HandleChallengeLocalRequest()
**
**==============================================================================
*/

static void HandleChallengeLocalRequest(int sock)
{
    char challenge[EXECUTOR_BUFFER_SIZE];
    struct ExecutorChallengeLocalRequest request;
    struct ExecutorChallengeLocalResponse response;

    memset(&response, 0, sizeof(response));

    /* Read the request message. */

    ReadExecutorRequest(sock, &request, sizeof(request));

    Log(LL_TRACE, "HandleChallengeLocalRequest(): user=%s", request.user);

    /* Perform operation. */

    response.status = StartLocalAuthentication(request.user, challenge);

    /* Send response message. */

    if (response.status == 0)
    {
        Strlcpy(response.challenge, challenge, sizeof(response.challenge));
    }

    WriteExecutorResponse(sock, &response, sizeof(response));
}

/*
**==============================================================================
**
** HandleAuthenticateLocalRequest()
**
**==============================================================================
*/

static void HandleAuthenticateLocalRequest(int sock)
{
    int status;
    struct ExecutorAuthenticateLocalRequest request;
    struct ExecutorAuthenticateLocalResponse response;

    memset(&response, 0, sizeof(response));

    /* Read the request message. */

    ReadExecutorRequest(sock, &request, sizeof(request));

    Log(LL_TRACE, "HandleAuthenticateLocalRequest()");

    /* Perform operation. */

    status = FinishLocalAuthentication(request.challenge, request.response);

    /* Send response. */

    response.status = status;

    WriteExecutorResponse(sock, &response, sizeof(response));
}

/*
**==============================================================================
**
** HandleUpdateLogLevelRequest()
**
**==============================================================================
*/

static void HandleUpdateLogLevelRequest(int sock)
{
    int status;
    struct ExecutorUpdateLogLevelRequest request;
    struct ExecutorUpdateLogLevelResponse response;

    memset(&response, 0, sizeof(response));

    /* Read the request message. */

    ReadExecutorRequest(sock, &request, sizeof(request));

    /* Log request. */

    Log(LL_TRACE, "HandleUpdateLogLevelRequest(): logLevel=%s",
        request.logLevel);

    /* Perform operation. */

    status = SetLogLevel(request.logLevel);

    if (status == -1)
        Log(LL_WARNING, "SetLogLevel(%d) failed", request.logLevel);

    /* Send response message. */

    response.status = status;

    WriteExecutorResponse(sock, &response, sizeof(response));
}

/*
**==============================================================================
**
** Parent()
**
**     The parent process (cimserver).
**
**==============================================================================
*/

void Parent(int sock, int initCompletePipe, int childPid, int bindVerbose)
{
    /* Handle Ctrl-C. */

    signal(SIGINT, _sigHandler);

    /* Catch SIGTERM, sent by kill program. */

    signal(SIGTERM, _sigHandler);

    /*
     * Ignore SIGPIPE, which occurs if a child with whom the executor shares
     * a local domain socket unexpectedly dies. In such a case, the socket
     * read/write functions will return an error. There are two child processes
     * the executor talks to over sockets: CIMSERVERA and CIMSERVERMAIN.
     */

    signal(SIGPIPE, SIG_IGN);

    /* Save child PID globally; it is used by Exit() function. */

    globals.childPid = childPid;

    /* Save initCompletePipe; it is used at daemonization. */

    globals.initCompletePipe = initCompletePipe;

    /* Save bindVerbose; it is used at daemonization and cimprovagt start. */

    globals.bindVerbose = bindVerbose;

    /* Prepares socket into non-blocking I/O. */

    SetNonBlocking(sock);

    /* Process client requests until client exists. */

    for (;;)
    {
        size_t n;
        struct ExecutorRequestHeader header;

        /* Receive request header. */

        n = RecvNonBlock(sock, &header, sizeof(header));

        if (n == 0)
        {
            /*
             * Either client closed its end of the pipe (possibly by exiting)
             * or we caught a SIGTERM.
             */
            break;
        }

        if (n != sizeof(header))
            Fatal(FL, "Failed to read header");

        /* Dispatch request. */

        switch ((enum ExecutorMessageCode)header.code)
        {
            case EXECUTOR_PING_MESSAGE:
                HandlePingRequest(sock);
                break;

            case EXECUTOR_OPEN_FILE_MESSAGE:
                HandleOpenFileRequest(sock);
                break;

            case EXECUTOR_START_PROVIDER_AGENT_MESSAGE:
                HandleStartProviderAgentRequest(sock);
                break;

            case EXECUTOR_DAEMONIZE_EXECUTOR_MESSAGE:
                HandleDaemonizeExecutorRequest(sock);
                break;

            case EXECUTOR_RENAME_FILE_MESSAGE:
                HandleRenameFileRequest(sock);
                break;

            case EXECUTOR_REMOVE_FILE_MESSAGE:
                HandleRemoveFileRequest(sock);
                break;

            case EXECUTOR_AUTHENTICATE_PASSWORD_MESSAGE:
                HandleAuthenticatePasswordRequest(sock);
                break;

            case EXECUTOR_VALIDATE_USER_MESSAGE:
                HandleValidateUserRequest(sock);
                break;

            case EXECUTOR_CHALLENGE_LOCAL_MESSAGE:
                HandleChallengeLocalRequest(sock);
                break;

            case EXECUTOR_AUTHENTICATE_LOCAL_MESSAGE:
                HandleAuthenticateLocalRequest(sock);
                break;

            case EXECUTOR_UPDATE_LOG_LEVEL_MESSAGE:
                HandleUpdateLogLevelRequest(sock);
                break;

            default:
                Fatal(FL, "Invalid request code: %d", header.code);
                break;
        }
    }

    /* Reached due to socket EOF, SIGTERM, or SIGINT. */

    Exit(0);
}
