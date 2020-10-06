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


#include "AnonymousPipe.h"
#include <Pegasus/Common/Signal.h>
#include "Network.h"
#include <windows.h>
#include <stdio.h>


PEGASUS_NAMESPACE_BEGIN

AnonymousPipe::AnonymousPipe()
{
    PEG_METHOD_ENTER(TRC_OS_ABSTRACTION, "AnonymousPipe::AnonymousPipe()");

    AnonymousPipeHandle thePipe[2];

    SECURITY_ATTRIBUTES saAttr;
    saAttr.nLength = sizeof(SECURITY_ATTRIBUTES);
    saAttr.bInheritHandle = TRUE;
    saAttr.lpSecurityDescriptor = NULL;

    if (!CreatePipe(&thePipe[0], &thePipe[1], &saAttr, 0))
    {
        PEG_TRACE((TRC_OS_ABSTRACTION, Tracer::LEVEL1,
            "Failed to create pipe.  Error code: %d", GetLastError()));
        PEG_METHOD_EXIT();

        MessageLoaderParms mlp("Common.AnonymousPipe.CREATE_PIPE_FAILED",
            "Failed to create pipe.");
        throw Exception(mlp);
    }

    _readHandle = thePipe[0];
    _writeHandle = thePipe[1];
    _readOpen = true;
    _writeOpen = true;

    PEG_METHOD_EXIT();
}

AnonymousPipe::AnonymousPipe(
    const char * readHandle,
    const char * writeHandle)
{
    PEG_METHOD_ENTER(TRC_OS_ABSTRACTION,
        "AnonymousPipe::AnonymousPipe(const char *, const char *)");

    _readHandle = 0;
    _writeHandle = 0;
    _readOpen = false;
    _writeOpen = false;

    if (readHandle != NULL)
    {
        if (sscanf(readHandle, "%p", &_readHandle) != 1)
        {
            PEG_TRACE((TRC_OS_ABSTRACTION, Tracer::LEVEL1,
                "Failed to create pipe: invalid read handle %s", readHandle));
            PEG_METHOD_EXIT();

            MessageLoaderParms mlp("Common.AnonymousPipe.CREATE_PIPE_FAILED",
                "Failed to create pipe.");
            throw Exception(mlp);
        }
        _readOpen = true;
    }

    if (writeHandle != NULL)
    {
        if (sscanf(writeHandle, "%p", &_writeHandle) != 1)
        {
            PEG_TRACE((TRC_OS_ABSTRACTION, Tracer::LEVEL1,
                "Failed to create pipe: invalid write handle %s", writeHandle));
            PEG_METHOD_EXIT();

            MessageLoaderParms mlp("Common.AnonymousPipe.CREATE_PIPE_FAILED",
                "Failed to create pipe.");
            throw Exception(mlp);
        }
        _writeOpen = true;
    }

    PEG_METHOD_EXIT();
}

AnonymousPipe::~AnonymousPipe()
{
    PEG_METHOD_ENTER(TRC_OS_ABSTRACTION, "AnonymousPipe::~AnonymousPipe");

    if (_readOpen)
    {
        closeReadHandle();
    }

    if (_writeOpen)
    {
        closeWriteHandle();
    }

    PEG_METHOD_EXIT();
}

AnonymousPipe::Status AnonymousPipe::writeBuffer(
    const void * buffer,
    Uint32 bytesToWrite)
{
    //
    //  Treat invalid handle as connection closed
    //
    if (!_writeOpen)
    {
        PEG_TRACE_CSTRING(TRC_OS_ABSTRACTION, Tracer::LEVEL2,
            "Attempted to write to pipe whose write handle is not open");
        return STATUS_CLOSED;
    }

    //
    //  Ignore SIGPIPE signals
    //
    SignalHandler::ignore(PEGASUS_SIGPIPE);

    const char * writeBuffer = reinterpret_cast<const char *>(buffer);
    DWORD expectedBytes = bytesToWrite;
    do
    {
        BOOL returnValue;
        DWORD bytesWritten = 0;
        returnValue = WriteFile(_writeHandle, writeBuffer, expectedBytes,
            &bytesWritten, NULL);

        if (!returnValue)
        {
            PEG_TRACE((TRC_OS_ABSTRACTION, Tracer::LEVEL2,
                "Failed to write buffer to pipe.  Error code: %d",
                GetLastError()));
            return STATUS_ERROR;
        }

        if (bytesWritten < 0)
        {
            PEG_TRACE((TRC_OS_ABSTRACTION, Tracer::LEVEL2,
                "Failed to write buffer to pipe.  Error code: %d",
                GetLastError()));

            if ((GetLastError() == ERROR_PIPE_NOT_CONNECTED) ||
                (GetLastError() == ERROR_BROKEN_PIPE))
            {
                return STATUS_CLOSED;
            }
            else
            {
                return STATUS_ERROR;
            }
        }

        expectedBytes -= bytesWritten;
        writeBuffer += bytesWritten;
    } while (expectedBytes > 0);

    return STATUS_SUCCESS;
}

AnonymousPipe::Status AnonymousPipe::readBuffer(
    void * buffer,
    Uint32 bytesToRead)
{
    //
    //  Treat invalid handle as connection closed
    //
    if (!_readOpen)
    {
        PEG_TRACE_CSTRING(TRC_OS_ABSTRACTION, Tracer::LEVEL2,
            "Attempted to read from pipe whose read handle is not open");
        return STATUS_CLOSED;
    }

    DWORD expectedBytes = bytesToRead;

    do
    {
        BOOL returnValue;
        DWORD bytesRead;
        returnValue = ReadFile(
            _readHandle, buffer, bytesToRead, &bytesRead, NULL);

        if (!returnValue)
        {
            PEG_TRACE((TRC_OS_ABSTRACTION, Tracer::LEVEL2,
                "Failed to read buffer from pipe.  Error code: %d",
                GetLastError()));
            if ((GetLastError() == ERROR_PIPE_NOT_CONNECTED) ||
                (GetLastError() == ERROR_BROKEN_PIPE))
            {
                return STATUS_CLOSED;
            }

            return STATUS_ERROR;
        }

        if (bytesRead == 0)
        {
            //
            //  Connection closed
            //
            PEG_TRACE_CSTRING(TRC_OS_ABSTRACTION, Tracer::LEVEL2,
                "Failed to read buffer from pipe: connection closed");
            return STATUS_CLOSED;
        }

        if (bytesRead < 0)
        {
            PEG_TRACE((TRC_OS_ABSTRACTION, Tracer::LEVEL2,
                "Failed to read buffer from pipe.  Error code: %d",
                GetLastError()));

            //
            //  Error reading from pipe
            //
            return STATUS_ERROR;
        }

        buffer = reinterpret_cast<char *>(buffer) + bytesRead;
        bytesToRead -= bytesRead;
    } while (bytesToRead > 0);

    return STATUS_SUCCESS;
}

void AnonymousPipe::exportReadHandle(char * buffer) const
{
    PEG_METHOD_ENTER(TRC_OS_ABSTRACTION, "AnonymousPipe::exportReadHandle");

    sprintf(buffer, "%p", _readHandle);

    PEG_METHOD_EXIT();
}

void AnonymousPipe::exportWriteHandle(char * buffer) const
{
    PEG_METHOD_ENTER(TRC_OS_ABSTRACTION, "AnonymousPipe::exportWriteHandle");

    sprintf(buffer, "%p", _writeHandle);

    PEG_METHOD_EXIT();
}

void AnonymousPipe::closeReadHandle()
{
    PEG_METHOD_ENTER(TRC_OS_ABSTRACTION, "AnonymousPipe::closeReadHandle");

    if (_readOpen)
    {
        if (!CloseHandle(_readHandle))
        {
            PEG_TRACE((TRC_OS_ABSTRACTION, Tracer::LEVEL2,
                "Failed to close read handle.  Error code: %d",
                GetLastError()));
        }
        else
        {
            _readOpen = false;
        }
    }
    else
    {
        PEG_TRACE_CSTRING(TRC_OS_ABSTRACTION, Tracer::LEVEL2,
            "Attempted to close read handle that was not open");
    }

    PEG_METHOD_EXIT();
}

void AnonymousPipe::closeWriteHandle()
{
    PEG_METHOD_ENTER(TRC_OS_ABSTRACTION, "AnonymousPipe::closeWriteHandle");

    if (_writeOpen)
    {
        if (!CloseHandle(_writeHandle))
        {
            PEG_TRACE((TRC_OS_ABSTRACTION, Tracer::LEVEL2,
                "Failed to close write handle.  Error code: %d",
                GetLastError()));
        }
        else
        {
            _writeOpen = false;
        }
    }
    else
    {
        PEG_TRACE_CSTRING(TRC_OS_ABSTRACTION, Tracer::LEVEL2,
            "Attempted to close write handle that was not open");
    }

    PEG_METHOD_EXIT();
}

PEGASUS_NAMESPACE_END
