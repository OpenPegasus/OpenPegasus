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

#include <string.h>
#include <unistd.h>
#include <ctype.h>
#include <fcntl.h>
#include "Policy.h"
#include "Defines.h"
#include "Macro.h"
#include "Path.h"
#include "Fatal.h"
#include "Log.h"
#include "Match.h"
#include "Messages.h"
#include "Globals.h"
#include "Strlcat.h"
#include "Strlcpy.h"

/*
**==============================================================================
**
** ARG()
**
**     Expands function arguments to "name, value" for use in formatted
**     output statements.
**
**     For example, this,
**
**         printf("%s=\"%s\"", ARG(count));
**
**     is expanded to this:
**
**         printf("%s=\"%s\"", "count", count);
**
**==============================================================================
*/

#define ARG(X) #X, X

/*
**==============================================================================
**
** _staticPolicyTable[]
**
**     This array defines the static policy table for the executor.
**
**==============================================================================
*/

static struct Policy _staticPolicyTable[] =
{
    /* cimserver_current.conf policies */
    {
        EXECUTOR_OPEN_FILE_MESSAGE,
        "${currentConfigFilePath}",
        "w",
        (S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH) /* 0644 */
    },
    {
        EXECUTOR_RENAME_FILE_MESSAGE,
        "${currentConfigFilePath}",
        "${currentConfigFilePath}.bak",
        0, /* flags */
    },
    {
        EXECUTOR_REMOVE_FILE_MESSAGE,
        "${currentConfigFilePath}",
        NULL,
        0, /* flags */
    },
    {
        EXECUTOR_REMOVE_FILE_MESSAGE,
        "${currentConfigFilePath}.bak",
        NULL,
        0, /* flags */
    },
    /* cimserver_planned.conf policies */
    {
        EXECUTOR_OPEN_FILE_MESSAGE,
        "${plannedConfigFilePath}",
        "w",
        (S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH) /* 0644 */
    },
    {
        EXECUTOR_RENAME_FILE_MESSAGE,
        "${plannedConfigFilePath}",
        "${plannedConfigFilePath}.bak",
        0, /* flags */
    },
    {
        EXECUTOR_REMOVE_FILE_MESSAGE,
        "${plannedConfigFilePath}",
        NULL,
        0, /* flags */
    },
    {
        EXECUTOR_REMOVE_FILE_MESSAGE,
        "${plannedConfigFilePath}.bak",
        NULL,
        0, /* flags */
    },
    /* cimserver.passwd policies */
    {
        EXECUTOR_OPEN_FILE_MESSAGE,
        "${passwordFilePath}",
        "w",
        (S_IRUSR | S_IWUSR) /* 0600 */
    },
    {
        EXECUTOR_RENAME_FILE_MESSAGE,
        "${passwordFilePath}.bak",
        "${passwordFilePath}",
        0, /* flags */
    },
    {
        EXECUTOR_RENAME_FILE_MESSAGE,
        "${passwordFilePath}",
        "${passwordFilePath}.bak",
        0, /* flags */
    },
    {
        EXECUTOR_REMOVE_FILE_MESSAGE,
        "${passwordFilePath}.bak",
        NULL,
        0, /* flags */
    },
    {
        EXECUTOR_REMOVE_FILE_MESSAGE,
        "${passwordFilePath}",
        NULL,
        0, /* flags */
    },
    /* SSL key file policies. */
    {
        EXECUTOR_OPEN_FILE_MESSAGE,
        "${sslKeyFilePath}",
        "r",
        0, /* flags not used when opening a file for read access */
    },
    /* SSL trust store policies. */
    {
        EXECUTOR_OPEN_FILE_MESSAGE,
        "${sslTrustStore}/*",
        "w",
        (S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH) /* 0644 */
    },
    {
        EXECUTOR_REMOVE_FILE_MESSAGE,
        "${sslTrustStore}/*",
        NULL,
        0, /* flags */
    },
    /* CRL store policies. */
    {
        EXECUTOR_OPEN_FILE_MESSAGE,
        "${crlStore}/*",
        "w",
        (S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH) /* 0644 */
    },
    {
        EXECUTOR_REMOVE_FILE_MESSAGE,
        "${crlStore}/*",
        NULL,
        0, /* flags */
    },
    {
        EXECUTOR_REMOVE_FILE_MESSAGE,
        "${localAuthDir}/*",
        NULL,
        0, /* flags */
    },

};

static const size_t _staticPolicyTableSize =
    sizeof(_staticPolicyTable) / sizeof(_staticPolicyTable[0]);

/*
**==============================================================================
**
** CheckPolicy()
**
**==============================================================================
*/

int CheckPolicy(
    const struct Policy* policyTable,
    size_t policyTableSize,
    enum ExecutorMessageCode messageCode,
    const char* arg1,
    const char* arg2,
    unsigned long* flags)
{
    size_t i;

    /* Clear the flags. */

    if (flags)
        *flags = 0;

    for (i = 0; i < policyTableSize; i++)
    {
        const struct Policy* p;

        p = &policyTable[i];

        /* Check message code */

        if (p->messageCode != messageCode)
            continue;

        /* Check arg1. */

        if (p->arg1)
        {
            char pat[EXECUTOR_BUFFER_SIZE];

            if (ExpandMacros(p->arg1, pat) != 0 || Match(pat, arg1) != 0)
                continue;
        }

        /* Check arg2. */

        if (p->arg2)
        {
            char pat[EXECUTOR_BUFFER_SIZE];

            if (ExpandMacros(p->arg2, pat) != 0 || Match(pat, arg2) != 0)
                continue;
        }

        /* Set the output flags argument. */

        if (flags)
            *flags = p->flags;

        /* Found a matching policy! */

        return 0;
    }

    /* Failed to find any matching policy. */

    return -1;
}

/*
**==============================================================================
**
** CheckOpenFilePolicy()
**
**==============================================================================
*/

int CheckOpenFilePolicy(const char* path, int mode, unsigned long* flags)
{
    char arg2[2];

    arg2[0] = mode;
    arg2[1] = '\0';

    if (CheckPolicy(_staticPolicyTable, _staticPolicyTableSize,
        EXECUTOR_OPEN_FILE_MESSAGE, path, arg2, flags) == 0)
    {
        Log(LL_TRACE, "CheckOpenFilePolicy(%s=\"%s\", %s='%c') passed",
            ARG(path), ARG(mode));
        return 0;
    }

    Log(LL_SEVERE, "CheckOpenFilePolicy(%s=\"%s\", %s='%c') failed",
        ARG(path), ARG(mode));

#if defined(EXIT_ON_POLICY_FAILURE)
    Fatal(FL, "exited due to policy failure");
#endif

    return -1;
}

/*
**==============================================================================
**
** CheckRemoveFilePolicy()
**
**==============================================================================
*/

int CheckRemoveFilePolicy(const char* path)
{
    if (CheckPolicy(_staticPolicyTable, _staticPolicyTableSize,
        EXECUTOR_REMOVE_FILE_MESSAGE, path, NULL, NULL) == 0)
    {
        Log(LL_TRACE, "CheckRemoveFilePolicy(%s=\"%s\") passed", ARG(path));
        return 0;
    }

    Log(LL_SEVERE, "CheckRemoveFilePolicy(%s=\"%s\") failed", ARG(path));

#if defined(EXIT_ON_POLICY_FAILURE)
    Fatal(FL, "exited due to policy failure");
#endif

    return -1;
}

/*
**==============================================================================
**
** CheckRenameFilePolicy()
**
**==============================================================================
*/

int CheckRenameFilePolicy(const char* oldPath, const char* newPath)
{
    if (CheckPolicy(_staticPolicyTable, _staticPolicyTableSize,
        EXECUTOR_RENAME_FILE_MESSAGE, oldPath, newPath, NULL) == 0)
    {
        Log(LL_TRACE, "CheckRenameFilePolicy(%s=\"%s\", %s=\"%s\") passed",
            ARG(oldPath), ARG(newPath));
        return 0;
    }

    Log(LL_SEVERE, "CheckRenameFilePolicy(%s=\"%s\", %s=\"%s\") failed",
        ARG(oldPath), ARG(newPath));

#if defined(EXIT_ON_POLICY_FAILURE)
    Fatal(FL, "exited due to policy failure");
#endif

    return -1;
}

/*
**==============================================================================
**
** DumpPolicyHelper()
**
**     Dump the policy table given by *policyTable* and *policyTableSize*.
**     Expand any macros in the entries, if requested.
**
**==============================================================================
*/

void DumpPolicyHelper(
    FILE* outputStream,
    const struct Policy* policyTable,
    size_t policyTableSize,
    int expandMacros)
{
    size_t i;

    for (i = 0; i < policyTableSize; i++)
    {
        const struct Policy* p = &policyTable[i];
        const char* codeStr = MessageCodeToString(p->messageCode);
        char arg1[EXECUTOR_BUFFER_SIZE];
        char arg2[EXECUTOR_BUFFER_SIZE];

        if (expandMacros)
        {
            if (p->arg1)
                ExpandMacros(p->arg1, arg1);

            if (p->arg2)
                ExpandMacros(p->arg2, arg2);
        }
        else
        {
            if (p->arg1)
                Strlcpy(arg1, p->arg1, sizeof(arg1));

            if (p->arg2)
                Strlcpy(arg2, p->arg2, sizeof(arg2));
        }

        fprintf(outputStream, "%s(", codeStr);
        if (p->arg1)
            fprintf(outputStream, "\"%s\"", arg1);
        if (p->arg2)
            fprintf(outputStream, ", \"%s\"", arg2);
        fprintf(outputStream, ")\n");
    }
}

/*
**==============================================================================
**
** DumpPolicy()
**
**     Dump the static policy table.
**
**==============================================================================
*/

void DumpPolicy(FILE* outputStream, int expandMacros)
{
    fprintf(outputStream, "===== Policy:\n");

    DumpPolicyHelper(
        outputStream, _staticPolicyTable, _staticPolicyTableSize, expandMacros);

    putc('\n', outputStream);
}
