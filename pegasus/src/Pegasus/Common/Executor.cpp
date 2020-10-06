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

#include <cstdio>
#include <cstdlib>
#include <cstring>

#include <Pegasus/Common/Config.h>

#if defined(PEGASUS_OS_TYPE_WINDOWS)
# include <windows.h>
#else
# include <unistd.h>
# include <errno.h>
# include <sys/types.h>
# include <sys/time.h>
# include <sys/resource.h>
#endif

#if defined(PEGASUS_HAS_SIGNALS)
# include <sys/wait.h>
#endif

#include <Pegasus/Common/Constants.h>
#include <Pegasus/Common/Mutex.h>
#include <Pegasus/Common/Once.h>
#include <Pegasus/Common/FileSystem.h>
#include <Pegasus/Common/String.h>
#include <Pegasus/Common/Tracer.h>
#include <Pegasus/Common/System.h>
#include <Pegasus/Common/Executor.h>


#if defined(PEGASUS_ENABLE_PRIVILEGE_SEPARATION)
#include <Executor/Strlcpy.h>
# include <Executor/Socket.h>
# include <Executor/Messages.h>
#endif

#if defined(PEGASUS_PAM_AUTHENTICATION)
# include <Executor/PAMAuth.h>
#endif

#ifdef PEGASUS_OS_PASE
# include <as400_protos.h> // For fork400()
#endif

#ifdef PEGASUS_OS_ZOS
# include <spawn.h>
#endif

PEGASUS_NAMESPACE_BEGIN

////////////////////////////////////////////////////////////////////////////////
//
//
// class ExecutorImpl
//
//
////////////////////////////////////////////////////////////////////////////////

class ExecutorImpl
{
public:

    virtual ~ExecutorImpl()
    {
    }

    virtual int detectExecutor() = 0;

    virtual int ping() = 0;

    virtual FILE* openFile(
        const char* path,
        int mode) = 0;

    virtual int renameFile(
        const char* oldPath,
        const char* newPath) = 0;

    virtual int removeFile(
        const char* path) = 0;

    virtual int startProviderAgent(
        unsigned short bitness,
        const char* module,
        const String& pegasusHome,
        const String& userName,
        int& pid,
        AnonymousPipe*& readPipe,
        AnonymousPipe*& writePipe) = 0;

    virtual int daemonizeExecutor() = 0;

    virtual int reapProviderAgent(
        int pid) = 0;

    virtual int authenticatePassword(
        const char* username,
        const char* password) = 0;

    virtual int validateUser(
        const char* username) = 0;

    virtual int challengeLocal(
        const char* username,
        char challengeFilePath[EXECUTOR_BUFFER_SIZE]) = 0;

    virtual int authenticateLocal(
        const char* challengeFilePath,
        const char* response) = 0;

    virtual int updateLogLevel(
        const char* logLevel) = 0;
};

////////////////////////////////////////////////////////////////////////////////
//
//
// class ExecutorLoopbackImpl
//
//
////////////////////////////////////////////////////////////////////////////////

class ExecutorLoopbackImpl : public ExecutorImpl
{
public:

    virtual ~ExecutorLoopbackImpl()
    {
    }

    virtual int detectExecutor()
    {
        return -1;
    }

    virtual int ping()
    {
        return -1;
    }

    virtual FILE* openFile(
        const char* path,
        int mode)
    {
        FILE* fhandle = NULL;
        switch (mode)
        {
            case 'r':
                fhandle = fopen(path, "r");
                break;

            case 'w':
                fhandle = fopen(path, "w");
                break;

            case 'a':
                fhandle = fopen(path, "a+");
                break;

            default:
                PEGASUS_ASSERT(fhandle);
                break;
        }

        if(!fhandle)
        {
            PEG_TRACE((TRC_SERVER, Tracer::LEVEL1,
                "Open of file %s in mode %c failed: %s",path,mode,
                (const char*) PEGASUS_SYSTEM_ERRORMSG.getCString()));
        }
        return fhandle;
    }

    virtual int renameFile(
        const char* oldPath,
        const char* newPath)
    {
        return FileSystem::renameFile(oldPath, newPath) ? 0 : -1;
    }


    virtual int removeFile(
        const char* path)
    {
        return FileSystem::removeFile(path) ? 0 : -1;
    }


    virtual int startProviderAgent(
        unsigned short bitness,
        const char* module,
        const String& pegasusHome,
        const String& userName,
        int& pid,
        AnonymousPipe*& readPipe,
        AnonymousPipe*& writePipe)
    {
        PEG_METHOD_ENTER(TRC_SERVER,"ExecutorLoopbackImpl::startProviderAgent");

#if !defined(PEGASUS_ENABLE_PRIVILEGE_SEPARATION)

        // Resolve full path of "cimprovagt" program.

        String path = FileSystem::getAbsolutePath(
            pegasusHome.getCString(),
            bitness == PG_PROVMODULE_BITNESS_32 ?
                PEGASUS_PROVIDER_AGENT32_PROC_NAME :
                    PEGASUS_PROVIDER_AGENT_PROC_NAME);

        // Create CString handles for cimprovagt arguments

        CString agentProgramPath = path.getCString();
        CString userNameCString = userName.getCString();

# if defined(PEGASUS_DISABLE_PROV_USERCTXT) || defined(PEGASUS_OS_ZOS)
        const char* setUserContextFlag = "0";    // False
# else
        const char* setUserContextFlag = "1";    // True
# endif

# if defined(PEGASUS_OS_TYPE_WINDOWS)

        AutoMutex autoMutex(_mutex);

        // Set output parameters in case of failure.

        pid = 0;
        readPipe = 0;
        writePipe = 0;

        // Create pipes. Export handles to string.

        AutoPtr<AnonymousPipe> pipeFromAgent(new AnonymousPipe());
        AutoPtr<AnonymousPipe> pipeToAgent(new AnonymousPipe());

        char readHandle[32];
        char writeHandle[32];
        pipeToAgent->exportReadHandle(readHandle);
        pipeFromAgent->exportWriteHandle(writeHandle);

        // Initialize PROCESS_INFORMATION.

        PROCESS_INFORMATION piProcInfo;
        ZeroMemory(&piProcInfo, sizeof (PROCESS_INFORMATION));

        // Initialize STARTUPINFO.

        STARTUPINFO siStartInfo;
        ZeroMemory(&siStartInfo, sizeof (STARTUPINFO));
        siStartInfo.cb = sizeof (STARTUPINFO);

        // Format command line.

        char cmdLine[2048];

        sprintf(cmdLine, "\"%s\" %s %s %s \"%s\" \"%s\"",
            (const char*)agentProgramPath,
            setUserContextFlag,
            readHandle,
            writeHandle,
            (const char*)userNameCString,
            module);

        //  Create provider agent proess.

        if (!CreateProcess (
            NULL,          //
            cmdLine,       //  command line
            NULL,          //  process security attributes
            NULL,          //  primary thread security attributes
            TRUE,          //  handles are inherited
            0,             //  creation flags
            NULL,          //  use parent's environment
            NULL,          //  use parent's current directory
            &siStartInfo,  //  STARTUPINFO
            &piProcInfo))  //  PROCESS_INFORMATION
        {
            PEG_METHOD_EXIT();
            return -1;
        }

        CloseHandle(piProcInfo.hProcess);
        CloseHandle(piProcInfo.hThread);

        // Close our copies of the agent's ends of the pipes

        pipeToAgent->closeReadHandle();
        pipeFromAgent->closeWriteHandle();

        readPipe = pipeFromAgent.release();
        writePipe = pipeToAgent.release();

        PEG_METHOD_EXIT();
        return 0;

# else /* POSIX CASE FOLLOWS */

        AutoMutex autoMutex(_mutex);

        // Initialize output parameters in case of error.

        pid = -1;
        readPipe = 0;
        writePipe = 0;

        // Pipes:

        int to[2];
        int from[2];
        char toPipeArg[32];
        char fromPipeArg[32];

        do
        {
            // Create "to-agent" pipe:

            if (pipe(to) != 0)
            {
                PEG_METHOD_EXIT();
                return -1;
            }

            // Create "from-agent" pipe:

            if (pipe(from) != 0)
            {
                PEG_METHOD_EXIT();
                return -1;
            }

            // Initialize the cimprovagt pipe arguments:

            sprintf(toPipeArg, "%d", to[0]);
            sprintf(fromPipeArg, "%d", from[1]);

            // Start provider agent:

#  if defined(PEGASUS_OS_ZOS)
            // zOS uses __spawn2() instead of fork() to start provider agent

            struct __inheritance inherit;
            const char *c_argv[7];

            c_argv[0] = agentProgramPath;
            c_argv[1] = setUserContextFlag;
            c_argv[2] = toPipeArg;
            c_argv[3] = fromPipeArg;
            c_argv[4] = userNameCString;
            c_argv[5] = module;
            c_argv[6] = NULL;

            // reset the inherit structure
            memset(&inherit,0,sizeof(inherit));

            // The provider agent should get a defined JobName.
            inherit.flags=SPAWN_SETJOBNAME;
            memcpy( inherit.jobname,"CFZOOPA ",
                    sizeof(inherit.jobname));

            PEG_TRACE((TRC_SERVER, Tracer::LEVEL4,
                "Starting provider agent: %s %s %s %s %s %s %s",
                (const char*)agentProgramPath,
                c_argv[0],
                c_argv[1],
                c_argv[2],
                c_argv[3],
                c_argv[4],
                c_argv[5]));

            pid = __spawn2(agentProgramPath,0,NULL,&inherit,
                           c_argv,(const char **)environ);

            if (pid < 0)
            {
                PEG_TRACE((TRC_SERVER, Tracer::LEVEL1,
                    "Spawn of provider agent fails:%s "
                        "( errno %d , reason code %08X )",
                    strerror(errno) ,errno,__errno2()));
                PEG_METHOD_EXIT();
                return -1;
            }

#  else    // !defined(PEGASUS_OS_ZOS)

#   if defined(PEGASUS_OS_VMS)
            pid = (int)vfork();
#   elif defined(PEGASUS_OS_PASE)
            pid = (int)fork400("QUMEPRVAGT",0);
#   else
            pid = (int)fork();
#   endif

            if (pid < 0)
            {
                PEG_TRACE((TRC_SERVER, Tracer::LEVEL1,
                     "Fork for provider agent fails: errno = %d",errno));
                PEG_METHOD_EXIT();
                return -1;
            }

            if (pid == 0)
            {
                // Child process

#   if !defined(PEGASUS_OS_VMS)
                // Close unused pipe descriptors:

                close(to[1]);
                close(from[0]);

                // Close unused descriptors. Leave stdin, stdout, stderr,
                // and the child's pipe descriptors open.

                struct rlimit rlim;

                if (getrlimit(RLIMIT_NOFILE, &rlim) == 0)
                {
                    for (int i = 3; i < int(rlim.rlim_cur); i++)
                    {
                        if (i != to[0] && i != from[1])
                            close(i);
                    }
                }

#   endif /* !defined(PEGASUS_OS_VMS) */

                // Exec the cimprovagt program.

                {
                    if (execl(
                            agentProgramPath,
                            agentProgramPath,
                            setUserContextFlag,
                            toPipeArg,
                            fromPipeArg,
                            (const char*)userNameCString,
                            module,
                            (char*)0) == -1)
                    {
                        PEG_TRACE((TRC_SERVER, Tracer::LEVEL1,
                            "execl() failed.  errno = %d.", errno));
                        _exit(1);
                    }
                }
            }
#  endif /* PEGASUS_OS_ZOS */
        }
        while (0);

        PEG_TRACE((TRC_SERVER, Tracer::LEVEL4,
            "Provider agent started: pid(%d).", pid));

        // Close unused pipe descriptors.

        close(to[0]);
        close(from[1]);

        // Set output parameters.

        int readFd = from[0];
        int writeFd = to[1];

        // Create to and from AnonymousPipe instances to correspond to the pipe
        // descriptors created above.

        char readFdStr[32];
        char writeFdStr[32];
        sprintf(readFdStr, "%d", readFd);
        sprintf(writeFdStr, "%d", writeFd);

        readPipe = new AnonymousPipe(readFdStr, 0);
        writePipe = new AnonymousPipe(0, writeFdStr);

#  if defined(PEGASUS_HAS_SIGNALS) && \
      !(defined(PEGASUS_DISABLE_PROV_USERCTXT) || defined(PEGASUS_OS_ZOS))
        // The cimprovagt forks and returns right away.  Clean up the zombie
        // process now instead of in reapProviderAgent().
        int status = 0;
        while ((status = waitpid(pid, 0, 0)) == -1 && errno == EINTR)
            ;
#  endif

        PEG_METHOD_EXIT();
        return 0;

# endif /* POSIX CASE */

#else /* PEGASUS_ENABLE_PRIVILEGE_SEPARATION is defined */

        // Out-of-Process providers are never started by the cimserver process
        // when Privilege Separation is enabled.
        return -1;

#endif
    }

    virtual int daemonizeExecutor()
    {
        return -1;
    }

#if defined(PEGASUS_ENABLE_PRIVILEGE_SEPARATION)
    virtual int reapProviderAgent(int)
    {
        // Out-of-Process providers are never started by the cimserver process
        // when Privilege Separation is enabled.
        return -1;
    }
#else  /* PEGASUS_ENABLE_PRIVILEGE_SEPARATION is NOT defined */
# if defined(PEGASUS_HAS_SIGNALS) && \
     (defined(PEGASUS_DISABLE_PROV_USERCTXT) || defined(PEGASUS_OS_ZOS))
    virtual int reapProviderAgent(int pid)
    {
        int status = 0;
        // When provider user context is enabled, this is done in
        // startProviderAgent().
        while ((status = waitpid(pid, 0, 0)) == -1 && errno == EINTR)
        {
        };
        return status;
    }
# else
    virtual int reapProviderAgent(int)
    {
        return 0;
    }
# endif
#endif


#if defined(PEGASUS_PAM_AUTHENTICATION)
    virtual int authenticatePassword(
        const char* username,
        const char* password)
    {
        return PAMAuthenticate(username, password);
    }
    
    virtual int validateUser(
        const char* username)
    {
        return PAMValidateUser(username);
    }
#else    
    virtual int authenticatePassword(
        const char*,
        const char*)
    {
        // ATTN: not handled so don't call in this case.
        return -1;
    }
    
    virtual int validateUser(
        const char*)
    {
        // ATTN: not handled so don't call in this case.
        return -1;
    }
#endif

    virtual int challengeLocal(
        const char*,
        char[EXECUTOR_BUFFER_SIZE])
    {
        // ATTN: not handled so don't call in this case.
        return -1;
    }

    virtual int authenticateLocal(
        const char*,
        const char*)
    {
        // ATTN: not handled so don't call in this case.
        return -1;
    }

    virtual int updateLogLevel(
        const char*)
    {
        // If Privilege Separation is not enabled, we don't need to update
        // the log level in the Executor.
        return 0;
    }

private:

    Mutex _mutex;
};

////////////////////////////////////////////////////////////////////////////////
//
//
// class ExecutorSocketImpl : public ExecutorImpl
//
//
////////////////////////////////////////////////////////////////////////////////

#if defined(PEGASUS_ENABLE_PRIVILEGE_SEPARATION)

class ExecutorSocketImpl : public ExecutorImpl
{
public:

    ExecutorSocketImpl(int sock) : _sock(sock)
    {
    }

    virtual ~ExecutorSocketImpl()
    {
    }

    virtual int detectExecutor()
    {
        return 0;
    }

    virtual int ping()
    {
        AutoMutex autoMutex(_mutex);

        // Send request header:

        ExecutorRequestHeader header;
        header.code = EXECUTOR_PING_MESSAGE;

        if (SendBlock(_sock, &header, sizeof(header)) != sizeof(header))
            return -1;

        ExecutorPingResponse response;

        if (RecvBlock(_sock, &response, sizeof(response)) != sizeof(response))
            return -1;

        if (response.magic == EXECUTOR_PING_MAGIC)
            return 0;

        return -1;
    }

    virtual FILE* openFile(
        const char* path,
        int mode)
    {
        AutoMutex autoMutex(_mutex);

        if (mode != 'r' && mode != 'w' && mode != 'a')
            return NULL;

        // Send request header:

        ExecutorRequestHeader header;
        header.code = EXECUTOR_OPEN_FILE_MESSAGE;

        if (SendBlock(_sock, &header, sizeof(header)) != sizeof(header))
            return NULL;

        // Send request body.

        ExecutorOpenFileRequest request;
        memset(&request, 0, sizeof(request));
        Strlcpy(request.path, path, EXECUTOR_BUFFER_SIZE);
        request.mode = mode;

        if (SendBlock(_sock, &request, sizeof(request)) != sizeof(request))
            return NULL;

        // Receive the response

        ExecutorOpenFileResponse response;

        if (RecvBlock(_sock, &response, sizeof(response)) != sizeof(response))
            return NULL;

        // Receive descriptor (if response successful).

        if (response.status == 0)
        {
            int fds[1];

            if (RecvDescriptorArray(_sock, fds, 1) != 0)
                return NULL;

            if (fds[0] == -1)
                return NULL;
            else
            {
                if (mode == 'r')
                    return fdopen(fds[0], "rb");
                else
                    return fdopen(fds[0], "wb");
            }
        }

        return NULL;
    }

    virtual int renameFile(
        const char* oldPath,
        const char* newPath)
    {
        AutoMutex autoMutex(_mutex);

        // Send request header:

        ExecutorRequestHeader header;
        header.code = EXECUTOR_RENAME_FILE_MESSAGE;

        if (SendBlock(_sock, &header, sizeof(header)) != sizeof(header))
            return -1;

        // Send request body.

        ExecutorRenameFileRequest request;
        memset(&request, 0, sizeof(request));
        Strlcpy(request.oldPath, oldPath, EXECUTOR_BUFFER_SIZE);
        Strlcpy(request.newPath, newPath, EXECUTOR_BUFFER_SIZE);

        if (SendBlock(_sock, &request, sizeof(request)) != sizeof(request))
            return -1;

        // Receive the response

        ExecutorRenameFileResponse response;

        if (RecvBlock(_sock, &response, sizeof(response)) != sizeof(response))
            return -1;

        return response.status;
    }

    virtual int removeFile(
        const char* path)
    {
        AutoMutex autoMutex(_mutex);

        // Send request header:

        ExecutorRequestHeader header;
        header.code = EXECUTOR_REMOVE_FILE_MESSAGE;

        if (SendBlock(_sock, &header, sizeof(header)) != sizeof(header))
            return -1;

        // Send request body.

        ExecutorRemoveFileRequest request;
        memset(&request, 0, sizeof(request));
        Strlcpy(request.path, path, EXECUTOR_BUFFER_SIZE);

        if (SendBlock(_sock, &request, sizeof(request)) != sizeof(request))
            return -1;

        // Receive the response

        ExecutorRemoveFileResponse response;

        if (RecvBlock(_sock, &response, sizeof(response)) != sizeof(response))
            return -1;

        return response.status;
    }

    virtual int startProviderAgent(
        unsigned short bitness,
        const char* module,
        const String& pegasusHome,
        const String& userName,
        int& pid,
        AnonymousPipe*& readPipe,
        AnonymousPipe*& writePipe)
    {
        AutoMutex autoMutex(_mutex);

        readPipe = 0;
        writePipe = 0;

        // Reject strings longer than EXECUTOR_BUFFER_SIZE.

        size_t moduleNameLength = strlen(module);

        if (moduleNameLength >= EXECUTOR_BUFFER_SIZE)
            return -1;

        CString userNameCString = userName.getCString();
        size_t userNameLength = strlen(userNameCString);

        if (userNameLength >= EXECUTOR_BUFFER_SIZE)
            return -1;

        // Send request header:

        ExecutorRequestHeader header;
        header.code = EXECUTOR_START_PROVIDER_AGENT_MESSAGE;

        if (SendBlock(_sock, &header, sizeof(header)) != sizeof(header))
            return -1;

        // Send request body.

        ExecutorStartProviderAgentRequest request;
        memset(&request, 0, sizeof(request));
        memcpy(request.module, module, moduleNameLength);
        memcpy(request.userName, userNameCString, userNameLength);
        request.moduleBitness = bitness;

        if (SendBlock(_sock, &request, sizeof(request)) != sizeof(request))
            return -1;

        // Receive the response

        ExecutorStartProviderAgentResponse response;

        if (RecvBlock(_sock, &response, sizeof(response)) != sizeof(response))
            return -1;

        // Check response status and pid.

        if (response.status != 0)
            return -1;

        // Get pid:

        pid = response.pid;

        // Receive descriptors.

        int descriptors[2];
        int result = RecvDescriptorArray(_sock, descriptors, 2);

        if (result == 0)
        {
            int readFd = descriptors[0];
            int writeFd = descriptors[1];

            // Create to and from AnonymousPipe instances to correspond to
            // the pipe descriptors created above.

            char readFdStr[32];
            char writeFdStr[32];
            sprintf(readFdStr, "%d", readFd);
            sprintf(writeFdStr, "%d", writeFd);

            readPipe = new AnonymousPipe(readFdStr, 0);
            writePipe = new AnonymousPipe(0, writeFdStr);
        }

        return result;
    }

    virtual int daemonizeExecutor()
    {
        AutoMutex autoMutex(_mutex);

        // Send request header:

        ExecutorRequestHeader header;
        header.code = EXECUTOR_DAEMONIZE_EXECUTOR_MESSAGE;

        if (SendBlock(_sock, &header, sizeof(header)) != sizeof(header))
            return -1;

        // Receive the response

        ExecutorDaemonizeExecutorResponse response;

        if (RecvBlock(_sock, &response, sizeof(response)) != sizeof(response))
            return -1;

        return response.status;
    }

    virtual int reapProviderAgent(
        int pid)
    {
        // The Executor process automatically cleans up all its child
        // processes, so it does not need to explicitly harvest the
        // exit status of the cimprovagt processes it starts.

        return 0;
    }

    virtual int authenticatePassword(
        const char* username,
        const char* password)
    {
        AutoMutex autoMutex(_mutex);

        // Send request header:

        ExecutorRequestHeader header;
        header.code = EXECUTOR_AUTHENTICATE_PASSWORD_MESSAGE;

        if (SendBlock(_sock, &header, sizeof(header)) != sizeof(header))
            return -1;

        // Send request body.

        ExecutorAuthenticatePasswordRequest request;
        memset(&request, 0, sizeof(request));
        Strlcpy(request.username, username, EXECUTOR_BUFFER_SIZE);
        Strlcpy(request.password, password, EXECUTOR_BUFFER_SIZE);

        if (SendBlock(_sock, &request, sizeof(request)) != sizeof(request))
            return -1;

        // Receive the response

        ExecutorAuthenticatePasswordResponse response;

        if (RecvBlock(_sock, &response, sizeof(response)) != sizeof(response))
            return -1;

        return response.status;
    }

    virtual int validateUser(
        const char* username)
    {
        AutoMutex autoMutex(_mutex);

        // Send request header:

        ExecutorRequestHeader header;
        header.code = EXECUTOR_VALIDATE_USER_MESSAGE;

        if (SendBlock(_sock, &header, sizeof(header)) != sizeof(header))
            return -1;

        // Send request body.

        ExecutorValidateUserRequest request;
        memset(&request, 0, sizeof(request));
        Strlcpy(request.username, username, EXECUTOR_BUFFER_SIZE);

        if (SendBlock(_sock, &request, sizeof(request)) != sizeof(request))
            return -1;

        // Receive the response

        ExecutorValidateUserResponse response;

        if (RecvBlock(_sock, &response, sizeof(response)) != sizeof(response))
            return -1;

        return response.status;
    }

    virtual int challengeLocal(
        const char* username,
        char challengeFilePath[EXECUTOR_BUFFER_SIZE])
    {
        AutoMutex autoMutex(_mutex);

        // Send request header:

        ExecutorRequestHeader header;
        header.code = EXECUTOR_CHALLENGE_LOCAL_MESSAGE;

        if (SendBlock(_sock, &header, sizeof(header)) != sizeof(header))
            return -1;

        // Send request body.

        ExecutorChallengeLocalRequest request;
        memset(&request, 0, sizeof(request));
        Strlcpy(request.user, username, EXECUTOR_BUFFER_SIZE);

        if (SendBlock(_sock, &request, sizeof(request)) != sizeof(request))
            return -1;

        // Receive the response

        ExecutorChallengeLocalResponse response;

        if (RecvBlock(_sock, &response, sizeof(response)) != sizeof(response))
            return -1;

        Strlcpy(challengeFilePath, response.challenge, EXECUTOR_BUFFER_SIZE);

        return response.status;
    }

    virtual int authenticateLocal(
        const char* challengeFilePath,
        const char* response)
    {
        AutoMutex autoMutex(_mutex);

        // Send request header:

        ExecutorRequestHeader header;
        header.code = EXECUTOR_AUTHENTICATE_LOCAL_MESSAGE;

        if (SendBlock(_sock, &header, sizeof(header)) != sizeof(header))
            return -1;

        // Send request body.

        ExecutorAuthenticateLocalRequest request;
        memset(&request, 0, sizeof(request));
        Strlcpy(request.challenge, challengeFilePath, EXECUTOR_BUFFER_SIZE);
        Strlcpy(request.response, response, EXECUTOR_BUFFER_SIZE);

        if (SendBlock(_sock, &request, sizeof(request)) != sizeof(request))
            return -1;

        // Receive the response

        ExecutorAuthenticateLocalResponse response_;

        if (RecvBlock(_sock, &response_, sizeof(response_)) !=
                sizeof(response_))
        {
            return -1;
        }

        return response_.status;
    }

    virtual int updateLogLevel(
        const char* logLevel)
    {
        AutoMutex autoMutex(_mutex);

        // Send request header:

        ExecutorRequestHeader header;
        header.code = EXECUTOR_UPDATE_LOG_LEVEL_MESSAGE;

        if (SendBlock(_sock, &header, sizeof(header)) != sizeof(header))
            return -1;

        // Send request body:

        ExecutorUpdateLogLevelRequest request;
        memset(&request, 0, sizeof(request));
        Strlcpy(request.logLevel, logLevel, EXECUTOR_BUFFER_SIZE);

        if (SendBlock(_sock, &request, sizeof(request)) != sizeof(request))
            return -1;

        // Receive the response

        ExecutorUpdateLogLevelResponse response;

        if (RecvBlock(_sock, &response, sizeof(response)) != sizeof(response))
            return -1;

        return response.status;
    }

private:

    int _sock;
    Mutex _mutex;
};

#endif /* defined(PEGASUS_ENABLE_PRIVILEGE_SEPARATION) */

////////////////////////////////////////////////////////////////////////////////
//
//
// class Executor
//
//
////////////////////////////////////////////////////////////////////////////////

static int _executorSock = -1;
static AutoPtr<ExecutorImpl> _executorImpl;
static Once _executorImplOnce = PEGASUS_ONCE_INITIALIZER;

static void _initExecutorImpl()
{
#if defined(PEGASUS_ENABLE_PRIVILEGE_SEPARATION)
    if (_executorSock == -1)
        _executorImpl.reset(new ExecutorLoopbackImpl());
    else
        _executorImpl.reset(new ExecutorSocketImpl(_executorSock));
#else
    _executorImpl.reset(new ExecutorLoopbackImpl());
#endif
}

void Executor::setSock(int sock)
{
    _executorSock = sock;
}

int Executor::detectExecutor()
{
    once(&_executorImplOnce, _initExecutorImpl);
    return _executorImpl->detectExecutor();
}

int Executor::ping()
{
    once(&_executorImplOnce, _initExecutorImpl);
    return _executorImpl->ping();
}

FILE* Executor::openFile(
    const char* path,
    int mode)
{
    once(&_executorImplOnce, _initExecutorImpl);
    return _executorImpl->openFile(path, mode);
}

int Executor::renameFile(
    const char* oldPath,
    const char* newPath)
{
    once(&_executorImplOnce, _initExecutorImpl);
    return _executorImpl->renameFile(oldPath, newPath);
}

int Executor::removeFile(
    const char* path)
{
    once(&_executorImplOnce, _initExecutorImpl);
    return _executorImpl->removeFile(path);
}

int Executor::startProviderAgent(
    unsigned short bitness,
    const char* module,
    const String& pegasusHome,
    const String& userName,
    int& pid,
    AnonymousPipe*& readPipe,
    AnonymousPipe*& writePipe)
{
    once(&_executorImplOnce, _initExecutorImpl);
    return _executorImpl->startProviderAgent(bitness,
        module, pegasusHome, userName, pid, readPipe, writePipe);
}

int Executor::daemonizeExecutor()
{
    once(&_executorImplOnce, _initExecutorImpl);
    return _executorImpl->daemonizeExecutor();
}

int Executor::reapProviderAgent(
    int pid)
{
    once(&_executorImplOnce, _initExecutorImpl);
    return _executorImpl->reapProviderAgent(pid);
}

int Executor::authenticatePassword(
    const char* username,
    const char* password)
{
    once(&_executorImplOnce, _initExecutorImpl);
    return _executorImpl->authenticatePassword(username, password);
}

int Executor::validateUser(
    const char* username)
{
    once(&_executorImplOnce, _initExecutorImpl);
    return _executorImpl->validateUser(username);
}

int Executor::challengeLocal(
    const char* user,
    char challengeFilePath[EXECUTOR_BUFFER_SIZE])
{
    once(&_executorImplOnce, _initExecutorImpl);
    return _executorImpl->challengeLocal(user, challengeFilePath);
}

int Executor::authenticateLocal(
    const char* challengeFilePath,
    const char* response)
{
    once(&_executorImplOnce, _initExecutorImpl);
    return _executorImpl->authenticateLocal(challengeFilePath, response);
}

int Executor::updateLogLevel(
    const char* logLevel)
{
    once(&_executorImplOnce, _initExecutorImpl);
    return _executorImpl->updateLogLevel(logLevel);
}

PEGASUS_NAMESPACE_END
