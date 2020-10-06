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
#include <unistd.h>
#include <sys/types.h>
#include <pwd.h>
#include <grp.h>
#include "User.h"
#include "Log.h"
#include "Strlcpy.h"

#define PWD_BUFF_SIZE 4096

/*
**==============================================================================
**
** GetUserInfo()
**
**     Lookup the given user's uid and gid.
**
**==============================================================================
*/

int GetUserInfo(const char* user, int* uid, int* gid)
{
    struct passwd pwd;
    char buffer[PWD_BUFF_SIZE];
    struct passwd* ptr = 0;

    if (getpwnam_r(user, &pwd, buffer, PWD_BUFF_SIZE, &ptr) != 0 || !ptr)
    {
        Log(LL_TRACE, "getpwnam_r(%s, ...) failed", user);
        return -1;
    }

    *uid = ptr->pw_uid;
    *gid = ptr->pw_gid;

    return 0;
}

/*
**==============================================================================
**
** GetUserName()
**
**     Lookup the user name for the specified user ID.
**
**==============================================================================
*/

int GetUserName(int uid, char username[EXECUTOR_BUFFER_SIZE])
{
    struct passwd pwd;
    char buffer[PWD_BUFF_SIZE];
    struct passwd* ptr = 0;

    if (getpwuid_r(uid, &pwd, buffer, PWD_BUFF_SIZE, &ptr) != 0 || !ptr)
    {
        Log(LL_TRACE, "getpwuid_r(%d, ...) failed", uid);
        username[0] = '\0';
        return -1;
    }

    Strlcpy(username, ptr->pw_name, EXECUTOR_BUFFER_SIZE);
    return 0;
}

/*
**==============================================================================
**
** SetUserContext()
**
**     Set the process user ID, group ID, and supplemental groups
**
**==============================================================================
*/

void SetUserContext(const char* username, int uid, int gid)
{
    if ((int)getgid() != gid)
    {
        if (setgid((gid_t)gid) != 0)
        {
            Log(LL_SEVERE, "setgid(%d) failed\n", gid);
            _exit(1);
        }
    }

    if (initgroups(username, gid) != 0)
    {
        Log(LL_SEVERE, "initgroups(%s, %d) failed\n", username, gid);
        _exit(1);
    }

    if ((int)getuid() != uid)
    {
        if (setuid((uid_t)uid) != 0)
        {
            Log(LL_SEVERE, "setuid(%d) failed\n", uid);
            _exit(1);
        }
    }
}
