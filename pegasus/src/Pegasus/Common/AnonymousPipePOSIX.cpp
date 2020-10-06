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

#if defined (PEGASUS_OS_VMS)
# include <climsgdef.h>
# include <stdio.h>
# include <stdlib.h>
# include <perror.h>
# include <processes.h>
#endif /* PEGASUS_OS_VMS */

#include <unistd.h>
#include <errno.h>

PEGASUS_NAMESPACE_BEGIN

AnonymousPipe::AnonymousPipe ()
{
    PEG_METHOD_ENTER (TRC_OS_ABSTRACTION, "AnonymousPipe::AnonymousPipe ()");

    AnonymousPipeHandle thePipe [2];
    if (pipe (thePipe) < 0)
    {
        PEG_TRACE((TRC_OS_ABSTRACTION, Tracer::LEVEL1,
            "Failed to create pipe: %s", strerror (errno)));
        PEG_METHOD_EXIT ();

        MessageLoaderParms mlp ("Common.AnonymousPipe.CREATE_PIPE_FAILED",
            "Failed to create pipe.");
        throw Exception (mlp);
    }

    _readHandle = thePipe [0];
    _writeHandle = thePipe [1];
    _readOpen = true;
    _writeOpen = true;

    PEG_METHOD_EXIT ();
}

AnonymousPipe::AnonymousPipe (
    const char * readHandle,
    const char * writeHandle)
{
    PEG_METHOD_ENTER (TRC_OS_ABSTRACTION,
        "AnonymousPipe::AnonymousPipe (const char *, const char *)");

    _readHandle = 0;
    _writeHandle = 0;
    _readOpen = false;
    _writeOpen = false;

    if (readHandle != NULL)
    {
        if (sscanf (readHandle, "%d", &_readHandle) != 1)
        {
            PEG_TRACE((TRC_OS_ABSTRACTION, Tracer::LEVEL1,
                "Failed to create pipe: invalid read handle %s", readHandle));
            PEG_METHOD_EXIT ();

            MessageLoaderParms mlp ("Common.AnonymousPipe.CREATE_PIPE_FAILED",
                "Failed to create pipe.");
            throw Exception (mlp);
        }
        _readOpen = true;
    }

    if (writeHandle != NULL)
    {
        if (sscanf (writeHandle, "%d", &_writeHandle) != 1)
        {
            PEG_TRACE((TRC_OS_ABSTRACTION, Tracer::LEVEL1,
                "Failed to create pipe: invalid write handle %s", writeHandle));
            PEG_METHOD_EXIT ();

            MessageLoaderParms mlp ("Common.AnonymousPipe.CREATE_PIPE_FAILED",
                "Failed to create pipe.");
            throw Exception (mlp);
        }
        _writeOpen = true;
    }

    PEG_METHOD_EXIT ();
}

AnonymousPipe::~AnonymousPipe ()
{
    PEG_METHOD_ENTER (TRC_OS_ABSTRACTION, "AnonymousPipe::~AnonymousPipe");

    if (_readOpen)
    {
        closeReadHandle ();
    }
    if (_writeOpen)
    {
        closeWriteHandle ();
    }

    PEG_METHOD_EXIT ();
}

AnonymousPipe::Status AnonymousPipe::writeBuffer (
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
    SignalHandler::ignore (PEGASUS_SIGPIPE);

    const char * writeBuffer = reinterpret_cast<const char*>(buffer);
    int expectedBytes = bytesToWrite;
    do
    {
        int bytesWritten = write (_writeHandle, writeBuffer, expectedBytes);

        if (bytesWritten < 0)
        {
            PEG_TRACE((TRC_OS_ABSTRACTION, Tracer::LEVEL2,
                "Failed to write buffer to pipe: %s", strerror (errno)));

            if (errno == EPIPE)
            {
                //
                //  Other end of pipe is closed
                //
                return STATUS_CLOSED;
            }
            else if (errno == EINTR)
            {
                //
                //  Keep trying to write
                //
                bytesWritten = 0;
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

AnonymousPipe::Status AnonymousPipe::readBuffer (
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

    Uint32 expectedBytes = bytesToRead;

    do
    {
        int bytesRead = read (_readHandle, buffer, bytesToRead);

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
                "Failed to read buffer from pipe: %s", strerror (errno)));

            //
            //  If read was interrupted, keep trying
            //  Otherwise, return error
            //
            if (errno == EINTR)
            {
                if (bytesToRead == expectedBytes)
                {
                    //
                    //  Got a signal and haven't read any bytes yet
                    //
                    return STATUS_INTERRUPT;
                }
                bytesRead = 0;
            }
            else
            {
                //
                //  Error reading from pipe
                //
                return STATUS_ERROR;
            }
        }

        buffer = reinterpret_cast<char *>(buffer) + bytesRead;
        bytesToRead -= bytesRead;
    } while (bytesToRead > 0);

    return STATUS_SUCCESS;
}

void AnonymousPipe::exportReadHandle (
    char * buffer) const
{
    PEG_METHOD_ENTER (TRC_OS_ABSTRACTION, "AnonymousPipe::exportReadHandle");

    sprintf (buffer, "%d", _readHandle);

    PEG_METHOD_EXIT ();
}

void AnonymousPipe::exportWriteHandle (
    char * buffer) const
{
    PEG_METHOD_ENTER (TRC_OS_ABSTRACTION, "AnonymousPipe::exportWriteHandle");

    sprintf (buffer, "%d", _writeHandle);

    PEG_METHOD_EXIT ();
}

void AnonymousPipe::closeReadHandle ()
{
    PEG_METHOD_ENTER (TRC_OS_ABSTRACTION, "AnonymousPipe::closeReadHandle");

    if (_readOpen)
    {
        if (close (_readHandle) != 0)
        {
            PEG_TRACE((TRC_OS_ABSTRACTION, Tracer::LEVEL2,
                "Failed to close read handle: %s", strerror (errno)));
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

    PEG_METHOD_EXIT ();
}

void AnonymousPipe::closeWriteHandle ()
{
    PEG_METHOD_ENTER (TRC_OS_ABSTRACTION, "AnonymousPipe::closeWriteHandle");

    if (_writeOpen)
    {
        if (close (_writeHandle) != 0)
        {
            PEG_TRACE((TRC_OS_ABSTRACTION, Tracer::LEVEL2,
                "Failed to close write handle: %s", strerror (errno)));
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

    PEG_METHOD_EXIT ();
}

PEGASUS_NAMESPACE_END
