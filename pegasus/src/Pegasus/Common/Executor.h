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

#ifndef _Pegasus_Common_Executor_h
#define _Pegasus_Common_Executor_h

#include <Pegasus/Common/Config.h>
#include <Pegasus/Common/MessageLoader.h>
#include <Pegasus/Common/AnonymousPipe.h>
#include <Pegasus/Common/Linkage.h>
#include <Executor/Defines.h>
#include <cstdio>

PEGASUS_NAMESPACE_BEGIN

/** The Executor class is used to perform various privileged operations. When
    Pegasus is built with privilege separation, the methods of this class are
    used to submit requests to a privileged process called and "executor". The
    current process communicates with the executor over an anonymous local
    domain socket. But, when Pegasus is built without privilege separation,
    the methods are implemented in the same process (within Executor.cpp).

    <br>
    When configured for privilege separation, the Pegasus server runs as two
    processes.

    <ul>
        <li>the executor (the cimserver program).
        <li>the server (the cimservermain program).
    </ul>

    The "executor" is the parent process. When it starts the server it passes
    the -x option with a socket number. The server checks for this option. It
    if finds it, is assumes it is running in privilege separation mode, in
    which case is calls Executor::setSock() with this socket number.

    <br>
    The Executor::detectExecutor() method is used in various places to see if
    the executor is present. For example.

        <pre>
        if (Executor::detectExecutor() == 0)
        {
            // Executor is present.
        }
        </pre>

    The remaining methods provide an interface for submitting requests to the
    executor over the given socket, if present. Otherwise, the methods are
    handled directly by this class (see Executor.cpp). Here is a typical
    exampe of its usage.

        <pre>
        if (Executor::removeFile(path) == 0)
        {
            // File successfully removed.
        }
        </pre>

    This example removes the given file. But be aware that the executor defines
    a policy that identifies which files it may removed (or manipulated by the
    other methods). In order to remove a file, the file must be added to the
    executor policy (located in pegasus/src/Executor/Policy.c).
*/
class PEGASUS_COMMON_LINKAGE Executor
{
public:

    /** Sets the local socket used to communicate with the executor.
        Warning: this method must be called before any other method or
        not at all.
        @sock the socket
    */
    static void setSock(int sock);

    /** Return zero if the executor is the parent of the current process.
        If so, the methods below are handled by the executor. Otherwise,
        they are handled by alternative in-process "loopback" methods.
        @return 0=success, -1=failure
    */
    static int detectExecutor();

    /** Ping the executor to see if it is responsive.
        @return 0=success, -1=failure
    */
    static int ping();

    /** Open the given file with the given mode.  The file permissions are
        governed by Executor policy (if the Executor is called) or by the
        process umask setting.
        @param path the path of the file.
        @param mode 'r'=read, 'w'=write, and 'a'=append.
        @return the file stream or NULL on failure.
    */
    static FILE* openFile(
        const char* path,
        int mode);

    /** Rename the given file.
        @param oldPath the path of the old file.
        @param newPath the path of the new file.
        @return 0=success, -1=failure
    */
    static int renameFile(
        const char* oldPath,
        const char* newPath);

    /** Remove the given file.
        @path the path of the file that will be reoved.
        @return 0=success, -1=failure
    */
    static int removeFile(
        const char* path);

    /** Start a provider agent as the given user. The provider agent will
        load the given provider module.
        @param type of provider module (32 or 64 bit)
        @param module name of provider module to be loaded.
        @param pegasusHome the Pegasus home directory to use to find the
            cimprovagt executable.
        @param userName the user name to run the provider agent as.
        @param pid the PID of the new process (to be eventually passed to
            reapProviderAgent()).
        @param readPipe pipe used to read data from provider agent.
        @param writePipe pipe used to write data from provider agent.
        @return 0=success, -1=failure
    */
    static int startProviderAgent(
        unsigned short bitness,
        const char* module,
        const String& pegasusHome,
        const String& userName,
        int& pid,
        AnonymousPipe*& readPipe,
        AnonymousPipe*& writePipe);

    /** Cause the executor to complete its daemonization and the cimserver
        command to exit with success status.
        @return 0=success, -1=failure
    */
    static int daemonizeExecutor();

    /** Wait for the provider agent to exit.
        @param pid the process id obtained with startProviderAgent().
        @return 0=success, -1=failure
    */
    static int reapProviderAgent(
        int pid);

    /** Check whether the password is correct for the given user, using an
        underyling authentication mechanism (either PAM or cimserver.passwd
        file).
        @param username the name of a valid system user.
        @param password the clear text password for the given user.
        @return 0=success, -1=failure, >0 = PAM return code
    */
    static int authenticatePassword(
        const char* username,
        const char* password);

    /** Check whether the given user is valid for the underlying authentcation
        mechanism.
        @param username the name of the user.
        @return 0=success, -1=failure, >0 = PAM return code
    */
    static int validateUser(
        const char* username);

    /** Begin authenticating the given *user* using the "local authentication"
        algorithm. A file containing a secret token is created on the local
        file system. The file is only readable by the given user. The caller
        should pass the path of this file to the client, who will attempt to
        read the secret token from the file and return it to the server. This
        token and the file path generated by this function should then be
        passed to authenticateLocal().
        @param username name of user to be challenged.
        @param challenge The challenge file path to be forwared by the caller
            to the client.
        @return 0=success, -1=failure
    */
    static int challengeLocal(
        const char* username,
        char challengeFilePath[EXECUTOR_BUFFER_SIZE]);

    /** Authenticate the given *user* using the "local authentication"
        algorithm. The secret token is read from the file created by
        challengeLocal(). If it matches the *response* argument,
        then the authentication is successful (returns zero).
        @param challengeFilePath The file path that was sent to the client
            to challenge for the secret token, generated by challengeLocal().
        @param response The challenge response obtained from the
            authenticating user. This is the response to the challenge
            obtained from challengeLocal().
        @return 0=success, -1=failure
    */
    static int authenticateLocal(
        const char* challengeFilePath,
        const char* response);

    /** Update the log level used by the Executor process.
        @param logLevel the new log level to use in the Executor.
        @return 0=success, -1=failure
    */
    static int updateLogLevel(
        const char* logLevel);

private:
    // Private to prevent instantiation.
    Executor();
};

PEGASUS_NAMESPACE_END

#endif /* _Pegasus_Common_Executor_h */
