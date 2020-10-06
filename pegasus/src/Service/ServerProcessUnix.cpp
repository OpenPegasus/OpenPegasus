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

#include <fcntl.h>
#include <unistd.h>

#include <Pegasus/Common/Signal.h>
#include <Pegasus/Common/Executor.h>

#define MAX_WAIT_TIME 240

PEGASUS_USING_PEGASUS;
PEGASUS_USING_STD;

Boolean handleSigUsr1 = false;
Boolean graveError = false;

void sigUsr1Handler(int s_n, PEGASUS_SIGINFO_T * s_info, void * sig)
{
    handleSigUsr1 = true;
}

void sigTermHandler(int s_n, PEGASUS_SIGINFO_T * s_info, void * sig)
{
    graveError= handleSigUsr1=true;
}


//constructor
ServerProcess::ServerProcess() {}

//destructor
ServerProcess::~ServerProcess() {}

// no-ops
void ServerProcess::cimserver_set_process(void* p) {}
void ServerProcess::cimserver_exitRC(int rc) {}
int ServerProcess::cimserver_initialize() { return 1; }

// for all OSes supporting signals provide a cimserver_wait function
// that waits to be awakened by signal PEGASUS_SIGTERM or PEGASUS_SIGHUP
#ifdef PEGASUS_HAS_SIGNALS
int ServerProcess::cimserver_wait()
{
    int sig = -1;
    sigset_t set;
    sigemptyset(&set);
    sigaddset(&set, PEGASUS_SIGTERM);
    sigaddset(&set, PEGASUS_SIGHUP);
    errno = 0;
    do
    {
#if defined(PEGASUS_OS_ZOS)
        sig = sigwait(&set);
#else // else for platforms = LINUX, HPUX, AIX
        sigwait(&set, &sig);
#endif
    } while (errno == EINTR);
    return sig;
}
#else
int ServerProcess::cimserver_wait() { return 1; }
#endif

String ServerProcess::getHome() { return String::EMPTY; }

// daemon_init , RW Stevens, "Advance UNIX Programming"

int ServerProcess::cimserver_fork()
{
    umask(S_IRWXG | S_IRWXO);

    if (Executor::detectExecutor() == 0)
    {
        // We don't need to fork if we're running with Privilege Separation
        return 0;
    }

    getSigHandle()->registerHandler(SIGTERM, sigTermHandler);
    getSigHandle()->activate(SIGTERM);
    getSigHandle()->registerHandler(PEGASUS_SIGUSR1, sigUsr1Handler);
    getSigHandle()->activate(PEGASUS_SIGUSR1);

    pid_t pid;
    if( (pid = fork() ) < 0)
    {
        getSigHandle()->deactivate(PEGASUS_SIGUSR1);
        getSigHandle()->deactivate(SIGTERM);
        return -1;
    }
    else if (pid != 0)
    {
        //
        // parent wait for child
        // if there is a problem with signal, parent process terminates
        // when waitTime expires
        //
        Uint32 waitTime = MAX_WAIT_TIME;

        while (!handleSigUsr1 && waitTime > 0)
        {
            sleep(1);
            waitTime--;
        }

        if (!handleSigUsr1)
        {
            MessageLoaderParms parms(
                "src.Service.ServerProcessUnix.CIMSERVER_START_TIMEOUT",
                "The cimserver command timed out waiting for the CIM server "
                    "to start.");
            PEGASUS_STD(cerr) << MessageLoader::getMessage(parms) <<
                PEGASUS_STD(endl);
        }
        exit(graveError);
    }

    setsid();
    umask(S_IRWXG | S_IRWXO);

    // spawned daemon process doesn't need the old signal handlers of its parent
    getSigHandle()->deactivate(PEGASUS_SIGUSR1);
    getSigHandle()->deactivate(SIGTERM);

    return 0;
}


// notify parent process to terminate so user knows that cimserver
// is ready to serve CIM requests.
void ServerProcess::notify_parent(int id)
{
    pid_t ppid = getppid();
    if (id)
    {
        kill(ppid, SIGTERM);
    }
    else
    {
        if (Executor::detectExecutor() == 0)
        {
            Executor::daemonizeExecutor();
        }
        else
        {
            kill(ppid, PEGASUS_SIGUSR1);
        }
    }
}


// Platform specific run
int ServerProcess::platform_run(
    int argc,
    char** argv,
    Boolean shutdownOption,
    Boolean debugOutputOption)
{
    return cimserver_run(argc, argv, shutdownOption, debugOutputOption);
}
