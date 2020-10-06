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

#ifndef _Executor_Messages_h
#define _Executor_Messages_h

/*
**==============================================================================
**
** Messages.h
**
**     This file defines messages exchanges over the socket between the
**     server and executor process.
**
**==============================================================================
*/

#include "Defines.h"

/*
**==============================================================================
**
** ExecutorMessageCode
**
**==============================================================================
*/

enum ExecutorMessageCode
{
    EXECUTOR_PING_MESSAGE = 1,
    EXECUTOR_OPEN_FILE_MESSAGE,
    EXECUTOR_START_PROVIDER_AGENT_MESSAGE,
    EXECUTOR_DAEMONIZE_EXECUTOR_MESSAGE,
    EXECUTOR_REMOVE_FILE_MESSAGE,
    EXECUTOR_RENAME_FILE_MESSAGE,
    EXECUTOR_AUTHENTICATE_PASSWORD_MESSAGE,
    EXECUTOR_VALIDATE_USER_MESSAGE,
    EXECUTOR_CHALLENGE_LOCAL_MESSAGE,
    EXECUTOR_AUTHENTICATE_LOCAL_MESSAGE,
    EXECUTOR_UPDATE_LOG_LEVEL_MESSAGE
};

/*
**==============================================================================
**
** struct ExecutorRequestHeader
**
**==============================================================================
*/

struct ExecutorRequestHeader
{
    unsigned int code;
};

/*
**==============================================================================
**
** EXECUTOR_PING_MESSAGE
**
**==============================================================================
*/

#define EXECUTOR_PING_MAGIC 0x9E5EACB6

struct ExecutorPingResponse
{
    unsigned int magic;
};

/*
**==============================================================================
**
** EXECUTOR_OPEN_FILE_MESSAGE
**
**==============================================================================
*/

struct ExecutorOpenFileRequest
{
    char path[EXECUTOR_BUFFER_SIZE];
    /* ('r' = read, 'w' = write, 'a' = append) */
    int mode;
};

struct ExecutorOpenFileResponse
{
    int status;
};

/*
**==============================================================================
**
** EXECUTOR_REMOVE_FILE_MESSAGE
**
**==============================================================================
*/

struct ExecutorRemoveFileRequest
{
    char path[EXECUTOR_BUFFER_SIZE];
};

struct ExecutorRemoveFileResponse
{
    int status;
};

/*
**==============================================================================
**
** EXECUTOR_RENAME_FILE_MESSAGE
**
**==============================================================================
*/

struct ExecutorRenameFileRequest
{
    char oldPath[EXECUTOR_BUFFER_SIZE];
    char newPath[EXECUTOR_BUFFER_SIZE];
};

struct ExecutorRenameFileResponse
{
    int status;
};

/*
**==============================================================================
**
** EXECUTOR_START_PROVIDER_AGENT_MESSAGE
**
**==============================================================================
*/

struct ExecutorStartProviderAgentRequest
{
    unsigned short moduleBitness;
    char module[EXECUTOR_BUFFER_SIZE];
    char userName[EXECUTOR_BUFFER_SIZE];
};

struct ExecutorStartProviderAgentResponse
{
    int status;
    int pid;
};

/*
**==============================================================================
**
** EXECUTOR_DAEMONIZE_EXECUTOR_MESSAGE
**
**==============================================================================
*/

struct ExecutorDaemonizeExecutorResponse
{
    int status;
};

/*
**==============================================================================
**
** EXECUTOR_AUTHENTICATE_PASSWORD_MESSAGE
**
**==============================================================================
*/

struct ExecutorAuthenticatePasswordRequest
{
    char username[EXECUTOR_BUFFER_SIZE];
    char password[EXECUTOR_BUFFER_SIZE];
};

struct ExecutorAuthenticatePasswordResponse
{
    int status;
};

/*
**==============================================================================
**
** EXECUTOR_VALIDATE_USER_MESSAGE
**
**==============================================================================
*/

struct ExecutorValidateUserRequest
{
    char username[EXECUTOR_BUFFER_SIZE];
};

struct ExecutorValidateUserResponse
{
    int status;
};

/*
**==============================================================================
**
** EXECUTOR_CHALLENGE_LOCAL_MESSAGE
**
**==============================================================================
*/

struct ExecutorChallengeLocalRequest
{
    char user[EXECUTOR_BUFFER_SIZE];
};

struct ExecutorChallengeLocalResponse
{
    int status;
    char challenge[EXECUTOR_BUFFER_SIZE];
};

/*
**==============================================================================
**
** EXECUTOR_AUTHENTICATE_LOCAL_MESSAGE
**
**==============================================================================
*/

struct ExecutorAuthenticateLocalRequest
{
    char challenge[EXECUTOR_BUFFER_SIZE];
    char response[EXECUTOR_BUFFER_SIZE];
};

struct ExecutorAuthenticateLocalResponse
{
    int status;
};

/*
**==============================================================================
**
** EXECUTOR_UPDATE_LOG_LEVEL_MESSAGE
**
**==============================================================================
*/

struct ExecutorUpdateLogLevelRequest
{
    char logLevel[EXECUTOR_BUFFER_SIZE];
};

struct ExecutorUpdateLogLevelResponse
{
    int status;
};

/*
**==============================================================================
**
** MessageCodeToString()
**
**==============================================================================
*/

EXECUTOR_LINKAGE
const char* MessageCodeToString(enum ExecutorMessageCode code);

#endif /* _Executor_Messages_h */
