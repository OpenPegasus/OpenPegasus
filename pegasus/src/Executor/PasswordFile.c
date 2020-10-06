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

#define _XOPEN_SOURCE
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include "Defines.h"
#include "Strlcpy.h"
#include "PasswordFile.h"

/*
**==============================================================================
**
** CheckPasswordFile()
**
**     Checks whether the *password* is correct for the given *username*,
**     according to the password file referred to by *path*. The file has
**     the following format.
**
**         <usrname>:<encrypted-password>
**
**     For example (smith's password is "changeme"):
**
**         smith:AB5bZ.JX9fQzA
**         jones:XMllrzJ80fd.A
**         williams:lM80ffj.jiOiO
**
**     Returns zero if the password matches or if password is null and the
**     user exists.
**
**==============================================================================
*/

int CheckPasswordFile(
    const char* path,
    const char* username,
    const char* password)
{
    FILE* is;
    char line[EXECUTOR_BUFFER_SIZE];

    /* Open file. */

    if ((is = fopen(path, "r")) == NULL)
        return -1;

    /* Lookup encrypted password for this user. */

    while (fgets(line, sizeof(line), is) != NULL)
    {
        char* p;
        char encryptedPassword[14];
        char buffer[EXECUTOR_BUFFER_SIZE];
        char salt[3];

        /* Skip lines starting with '#'. */

        if (line[0] == '#')
            continue;

        /* Replace colon with null-terminator. */

        if ((p = strchr(line, ':')) == NULL)
            continue;

        *p++ = '\0';

        /* Skip this line, if username does not match. */

        if (strcmp(line, username) != 0)
            continue;

        /* If password is null, we are done. */

        if (password == NULL)
        {
            fclose(is);
            return -1;
        }

        /* Get encrypted password. */

        Strlcpy(encryptedPassword, p, sizeof(encryptedPassword));

        /* Get salt from encrypted password. */

        salt[0] = encryptedPassword[0];
        salt[1] = encryptedPassword[1];
        salt[2] = '\0';

        /* Check password. */

        /* Flawfinder: ignore */
        Strlcpy(buffer, crypt(password, salt), sizeof(buffer));

        if (strcmp(buffer, encryptedPassword) == 0)
        {
            fclose(is);
            return 0;
        }
        else
        {
            fclose(is);
            return -1;
        }
    }

    /* User entry not found. */

    fclose(is);
    return -1;
}
