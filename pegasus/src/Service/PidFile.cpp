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

#include <Pegasus/Common/System.h>
#include <Pegasus/Common/Logger.h>
#include <Pegasus/Common/FileSystem.h>
#include <Service/PidFile.h>

PEGASUS_NAMESPACE_BEGIN

PidFile::PidFile(const char* pidFilePath)
    : _pidFilePath(pidFilePath)
{
}

PidFile::~PidFile()
{
}

unsigned long PidFile::getPid()
{
    FILE* pidFile;
    unsigned long pid = 0;

    // open the PID file
    pidFile = fopen(_pidFilePath, "r");
    if (!pidFile)
    {
        return 0;
    }

    // get the pid from the file
    if (fscanf(pidFile, "%lu\n", &pid) != 1)
    {
        fclose(pidFile);
        return 0;
    }

    fclose(pidFile);

    return pid;
}

void PidFile::setPid(unsigned long pid)
{
    // Remove the old file if it exists
    remove();

    // Open the file
    FILE *pidFile = fopen(_pidFilePath, "w");

    if (pidFile)
    {
        // Save the pid in the file
        fprintf(pidFile, "%lu\n", (unsigned long)pid);
        fclose(pidFile);
    }
}

void PidFile::remove()
{
    System::removeFile(_pidFilePath);
}

PEGASUS_NAMESPACE_END
