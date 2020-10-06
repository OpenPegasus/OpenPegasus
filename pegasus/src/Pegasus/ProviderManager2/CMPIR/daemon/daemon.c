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
    \file daemon.c
    \brief Remote daemon launcher.

    This program is to be run on remote host to enable them for remote
    providers. It loads predefined communication libraries and initializes
    them. Afterwards the remote daemon thread enters the cleanup procedure
    that checks for unused remote providers to be unloaded.

    \sa remote_broker.c
*/


#include "../cmpir_common.h"
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>

#if defined PEGASUS_OS_TYPE_WINDOWS
# include <winsock2.h>
# include <winbase.h>
# include <process.h>
# include <windows.h>
#else
# include <dlfcn.h>
# include <string.h>
# include <stdlib.h>
# include <fcntl.h>
# include <errno.h>
# include <unistd.h>
# include <signal.h>
#endif

#include "mm.h"
#include "../remote.h"
#include "../native/native.h"
#include "../ip.h"
#include "../io.h"
#include "../tcpcomm.h"
#include "../serialization.h"
#include "../debug.h"
#include <Pegasus/Common/Config.h>

typedef int (* START_DAEMON) ();

typedef struct
{
    const char * libname;
    void * hLibrary;
    CMPI_THREAD_TYPE thread;
} comm_lib;


//! List of communication libraries to be initialized.
static comm_lib __libs[] =
{
    { "CMPIRTCPCommRemote",0 ,0},
};

int nativeSide=1;

PEGASUS_IMPORT extern const struct BinarySerializerFT *binarySerializerFTptr;

const struct BinarySerializerFT *__sft = NULL;


/***************************************************************************/


//! Initializes a remote communication library.
/*!
    Opens the communication library, searches the entry points and executes
    it. The communication layer may then spawn additional listener threads,
    if necessary.

    \param comm pointer to the communication library to be loaded.
*/

#ifdef PEGASUS_OS_TYPE_WINDOWS
void winStartNetwork()
{
    WSADATA winData;
    WSAStartup ( 0x0002, &winData );
}
#endif

static void __init_remote_comm_lib ( comm_lib * comm )
{
    void * hdl = comm->hLibrary = tool_mm_load_lib ( comm->libname );

    if (hdl)
    {
#ifdef PEGASUS_OS_TYPE_WINDOWS
        START_DAEMON fp = (START_DAEMON) GetProcAddress (
            hdl,
            "start_remote_daemon" );
#else
        START_DAEMON fp = (START_DAEMON) dlsym ( hdl, "start_remote_daemon" );
#endif


        if (fp)
        {
            if (fp ())
            {
                fprintf ( stderr,
                        "Failed to initialize library." );
            }
            return;
        }
    }

#if defined (PEGASUS_OS_TYPE_WINDOWS)
    fprintf( stderr, "%s\n", strerror(errno) );
#else
    fprintf ( stderr, "%s\n", dlerror () );
#endif

}

void _usage()
{
    printf ( "Usage: CMPIRDaemon [--foreground | --stop | --help]\n" );
    printf ( "--foreground : Runs the daemon in the foreground.\n");
    printf ( "--stop       : Stops the daemon.\n" );
    printf ( "--help       : Prints this message.\n" );
}


//! Loads all communication libraries.
/*!
    The function initializes the de-/activation context for remote providers,
    then loads all the communication layers. Finally it enters the cleanup
    loop.

    \sa init_activation_context()
    \sa cleanup_remote_brokers()
*/
int main (int argc, char *argv[])
{
    unsigned int i;
    int foreground = 0;
    int unix_platform = 0;
    int daemon_stop = 0;
    char* message_code;
    CMPIContext * ctx;
    int socket;

    __sft = binarySerializerFTptr;

    if (argc > 2)
    {
        _usage();
        return 0;
    }

    if (argc == 2)
    {
        if (!strcmp (argv[1], "--foreground" ))
        {
            foreground = 1;
        }
        else if (!strcmp (argv[1], "--stop" ))
        {
            daemon_stop = 1;
        }
        else
        {
            _usage();
            return 0;
        }
    }
    // "tickle" the connection.

#ifdef PEGASUS_OS_TYPE_WINDOWS
    winStartNetwork();
#endif

    socket = open_connection (
        "127.0.0.1",
        REMOTE_LISTEN_PORT,
        PEGASUS_SUPPRESS_ERROR_MESSAGE);
    if (socket >= 0)
    {
        if (daemon_stop)
        {
            message_code = PEGASUS_CMPIR_DAEMON_STOP;
            printf ("CMPIRDaemon stopped.\n");
        }
        else
        {
            message_code = PEGASUS_CMPIR_DAEMON_IS_RUNNING;
            printf ("CMPIRDaemon is already started.\n");
        }


        __sft->serialize_string (socket, message_code );

        //invokes close(socket) on unix & closesocket(socket) on windows
        PEGASUS_CMPIR_CLOSESOCKET (socket);

        return 0;
    }
    else if (daemon_stop)
    {
        printf ("CMPIRDaemon is not running.\n");
        return 0;
    }

    // Start daemon on unix platforms
    if (!foreground)
    {
#if defined(PEGASUS_OS_TYPE_UNIX)
        pid_t pid;
        unix_platform = 1;
        pid = fork ();
        if (pid > 0)
        {
            exit(0); // Parent, die now...
        }
        if (pid < 0 || setsid() < 0 || chdir("/") < 0)
        {
            printf ("CMPIRDaemon: Can't run as daemon.\n");
            return -1;
        }
        printf ("CMPIRDaemon started.\n");
        /* Close out the standard file descriptors */
        close (STDIN_FILENO);
        close (STDOUT_FILENO);
        close (STDERR_FILENO);
#else
        foreground=1;
#endif
    }
    if (!unix_platform && !foreground)
    {
        printf ("Daemon is supported only on UNIX platforms,"
            " running in foreground.\n" );
    }

    ctx = native_new_CMPIContext ( TOOL_MM_NO_ADD );

    init_activation_context ( ctx );

    for (i = 0;
        i < sizeof ( __libs ) / sizeof ( comm_lib );
        i++)
    {
        __init_remote_comm_lib ( __libs + i );
    }

    if (foreground || !unix_platform)
    {
        TRACE_NORMAL(("All remote daemons started.\n"));
        TRACE_NORMAL(("Entering provider cleanup loop ...\n"));
    }

    cleanup_remote_brokers ((long) 100, 30 );
    return 0;
}

/*** Local Variables:  ***/
/*** mode: C           ***/
/*** c-basic-offset: 8 ***/
/*** End:              ***/


