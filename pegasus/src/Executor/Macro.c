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
#include "Defines.h"
#include "Macro.h"
#include "Strlcpy.h"
#include "Strlcat.h"
#include "Log.h"
#include "Config.h"
#include "Globals.h"
#include "Path.h"
#include <assert.h>

/*
**==============================================================================
**
** Macro
**
**     This structure defines a macro (a name-value pair).
**
**==============================================================================
*/

struct Macro
{
    char* name;
    char* value;
    struct Macro* next;
};

/*
**==============================================================================
**
** _macros
**
**     The list of process macros.
**
**==============================================================================
*/

static struct Macro* _macros = 0;

/*
**==============================================================================
**
** FindMacro()
**
**     Find the value of the named macro. Return pointer to value or NULL if
**     not found.
**
**==============================================================================
*/

const char* FindMacro(const char* name)
{
    const struct Macro* p;

    for (p = _macros; p; p = p->next)
    {
        if (strcmp(p->name, name) == 0)
        {
            return p->value;
        }
    }

    /* Not found. */
    return NULL;
}

/*
**==============================================================================
**
** DefineMacro()
**
**     Add a new macro to the macro list.
**
**==============================================================================
*/

int DefineMacro(const char* name, const char* value)
{
    struct Macro* macro;

    /* Reject if the macro is already defined. */

    if (FindMacro(name) != NULL)
    {
        return -1;
    }

    /* Create new macro. */

    if ((macro = (struct Macro*)malloc(sizeof(struct Macro))) == NULL)
    {
        return -1;
    }
    macro->name = strdup(name);
    macro->value = strdup(value);

    /* Add to end of list. */

    {
        struct Macro* p;
        struct Macro* prev = NULL;

        for (p = _macros; p; p = p->next)
        {
            prev = p;
        }

        if (prev)
        {
            prev->next = macro;
        }
        else
        {
            _macros = macro;
        }

        macro->next = NULL;
    }

    return 0;
}

/*
**==============================================================================
**
** UndefineMacro()
**
**     Remove the given macro from the macro table.
**
**==============================================================================
*/

int UndefineMacro(const char* name)
{
    struct Macro* p;
    struct Macro* prev;

    for (p = _macros, prev = 0; p; p = p->next)
    {
        if (strcmp(p->name, name) == 0)
        {
            if (prev)
            {
                prev->next = p->next;
            }
            else
            {
                _macros = p->next;
            }

            free(p->name);
            free(p->value);
            free(p);
            return 0;
        }

        prev = p;
    }

    /* Not found. */
    return -1;
}

/*
**==============================================================================
**
** ExpandMacros()
**
**     Expand all macros in *input*. Leave result in *output*.
**
**==============================================================================
*/

int ExpandMacros(const char* input, char output[EXECUTOR_BUFFER_SIZE])
{
    char buffer[EXECUTOR_BUFFER_SIZE];
    char* p;

    /* Make copy of input since we'll need to destroy it. */

    Strlcpy(buffer, input, sizeof(buffer));
    output[0] = '\0';

    /* Traverse buffer, copying characters and expanding macros as we go. */

    for (p = buffer; *p; )
    {
        /* Look for start of macro. */

        if (p[0] == '$' && p[1] == '{')
        {
            char* q;

            /* Set p and q as follows.
             *
             *       p       q
             *       |       |
             *       v       v
             * "...${MY_MACRO}..."
             */

            p += 2;

            /* Find end of macro. */

            for (q = p; *q && *q != '}'; q++)
                ;

            if (*q != '}')
            {
                Log(LL_SEVERE, "ExpandMacros(): corrupt input: %s", input);
                return -1;
            }

            /* Replace '}' with null-terminator. */

            *q = '\0';

            /* Lookup macro and append value. */

            {
                const char* value;
                value = FindMacro(p);

                if (!value)
                {
                    Log(LL_SEVERE, "ExpandMacros(): Undefined macro: %s", p);
                    return -1;
                }

                Strlcat(output, value, EXECUTOR_BUFFER_SIZE);
            }

            p = q + 1;
        }
        else
        {
            /* Append current character to output. */

            char two[2];
            two[0] = *p++;
            two[1] = '\0';

            Strlcat(output, two, EXECUTOR_BUFFER_SIZE);
        }
    }

    return 0;
}

/*
**==============================================================================
**
** DefineConfigPathMacro()
**
**     Define a new path macro whose value is taken from the given
**     configuration parameter. If no such configuration parameter is defined,
**     use the defaultPath.
**
**==============================================================================
*/

int DefineConfigPathMacro(const char* configParam, const char* defaultPath)
{
    char path[EXECUTOR_BUFFER_SIZE];
    int status;

    status = 0;

    do
    {
        char buffer[EXECUTOR_BUFFER_SIZE];

        /* First try to get value from configuration. */

        if (GetConfigParam(configParam, buffer) == 0)
        {
            if (buffer[0] == '/')
            {
                Strlcpy(path, buffer, sizeof(path));
                break;
            }
            else if (GetHomedPath(buffer, buffer) == 0)
            {
                Strlcpy(path, buffer, sizeof(path));
                break;
            }
        }

        /* Just use the default value. */

        if (GetHomedPath(defaultPath, buffer) == 0)
        {
            Strlcpy(path, buffer, sizeof(path));
            break;
        }

        /* Failed. */

        status = -1;
    }
    while (0);

    if (status == 0)
    {
        DefineMacro(configParam, path);
    }

    return status;
}

/*
**==============================================================================
**
** DumpMacros()
**
**     Dump all macros to standard output.
**
**==============================================================================
*/

void DumpMacros(FILE* outputStream)
{
    const struct Macro* p;

    fprintf(outputStream, "===== Macros:\n");

    for (p = _macros; p; p = p->next)
    {
        fprintf(outputStream, "%s=%s\n", p->name, p->value);
    }

    putc('\n', outputStream);
}
