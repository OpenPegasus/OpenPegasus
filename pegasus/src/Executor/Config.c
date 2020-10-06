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
#include <ctype.h>
#include <stdio.h>
#include "Config.h"
#include "Strlcpy.h"
#include "Strlcat.h"
#include "Path.h"
#include "Defines.h"
#include "Globals.h"
#include "Assert.h"

/*
**==============================================================================
**
** ConfigParameter
**
**     The _config[] array below defines default values for the
**     various configuration items.
**
**==============================================================================
*/

struct ConfigParameter
{
    const char* name;
    const char* value;
};

static struct ConfigParameter _config[] =
{
#include <Pegasus/Config/FixedPropertyTable.h>
};

static size_t _configSize =
    sizeof(_config) / sizeof(_config[0]);

/*
**==============================================================================
**
** GetConfigParamFromCommandLine()
**
**     Attempt to find a command line configuratin parameter of the form
**     name=value. For example:
**         cimservermain option=value
**     Return 0 if found.
**
**==============================================================================
*/

int GetConfigParamFromCommandLine(
    const char* name,
    char value[EXECUTOR_BUFFER_SIZE])
{
    size_t n = strlen(name);
    int i;

    EXECUTOR_ASSERT(globals.argv != NULL);

    for (i = 1; i < globals.argc; i++)
    {
        if (strncmp(globals.argv[i], name, n) == 0 && globals.argv[i][n] == '=')
        {
            Strlcpy(value, globals.argv[i] + n + 1, EXECUTOR_BUFFER_SIZE);
            return 0;
        }
    }

    return -1;
}

/*
**==============================================================================
**
** GetConfigParamFromFile()
**
**     Attempt to find the named option in the configuration file. If found,
**     set value and return 0.
**
**==============================================================================
*/

int GetConfigParamFromFile(
    const char* path,
    const char* name,
    char value[EXECUTOR_BUFFER_SIZE])
{
    char buffer[EXECUTOR_BUFFER_SIZE];
    FILE* is;
    size_t n;

    if ((is = fopen(path, "r")) == NULL)
        return -1;

    n = strlen(name);

    while (fgets(buffer, sizeof(buffer), is) != 0)
    {
        size_t r;

        /* Skip comments. */

        if (buffer[0] == '#')
            continue;

        /* Remove trailing whitespace. */

        r = strlen(buffer);

        while (r--)
        {
            if (isspace(buffer[r]))
                buffer[r] = '\0';
        }

        /* Skip blank lines. */

        if (buffer[0] == '\0')
            continue;

        /* Check option. */

        if (strncmp(buffer, name, n) == 0 &&  buffer[n] == '=')
        {
            Strlcpy(value, buffer + n + 1, EXECUTOR_BUFFER_SIZE);
            fclose(is);
            return 0;
        }
    }

    /* Not found! */
    fclose(is);
    return -1;
}

/*
**==============================================================================
**
** GetConfigParam()
**
**     Attempt to find a configuration setting for the given name. First,
**     search the command line and then the config file.
**
**==============================================================================
*/

int GetConfigParam(
    const char* name,
    char value[EXECUTOR_BUFFER_SIZE])
{
    const char* configFileName = 0;
    char path[EXECUTOR_BUFFER_SIZE];
    size_t i;

    /* (1) First check command line. */

    if (GetConfigParamFromCommandLine(name, value) == 0)
        return 0;

    /* (2) Next check config file. */

    if (strcmp(name, "shutdownTimeout") == 0)
    {
        configFileName = PEGASUS_CURRENT_CONFIG_FILE_PATH;
    }
    else
    {
        configFileName = PEGASUS_PLANNED_CONFIG_FILE_PATH;
    }

    if (GetHomedPath(configFileName, path) == 0 &&
        GetConfigParamFromFile(path, name, value) == 0)
        return 0;

    /* (3) Finally check the default configuration table. */

    for (i = 0; i < _configSize; i++)
    {
        if (strcmp(_config[i].name, name) == 0)
        {
            Strlcpy(value, _config[i].value, EXECUTOR_BUFFER_SIZE);
            return 0;
        }
    }

    /* Not found! */
    return -1;
}
