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

#include <iostream>
#include <Pegasus/Common/TraceFileHandler.h>
#include <Pegasus/Common/Mutex.h>

PEGASUS_USING_STD;

PEGASUS_NAMESPACE_BEGIN

static Mutex writeMutex;


////////////////////////////////////////////////////////////////////////////////
//   Writes message to file.
//   Implementation of this function is platform specific
//
//   Note: The current implementation writes the message to the defined file.
//         Will have to be enhanced to support synchronous write operations to
//         the same file.
////////////////////////////////////////////////////////////////////////////////
void TraceFileHandler::handleMessage(
    const char* message,
    Uint32,
    const char* fmt,
    va_list argList)
{
    Uint32 retCode;
    

    if (_configHasChanged)
    {
        _reConfigure();
    }

    if (!_fileHandle)
    {
        // The trace file is not open, which means an earlier fopen() was
        // unsuccessful.  Stop now to avoid logging duplicate error messages.
        return;
    }
  
    AutoMutex writeLock(writeMutex);

    if(!_fileExists(_fileName))
    {
        return;
    }       

        //Move to the End of File
        fseek(_fileHandle,0,SEEK_SET);

        // Write message to file
        fprintf(_fileHandle,"%s", message);
        vfprintf(_fileHandle,fmt,argList);
        retCode = fprintf(_fileHandle,"\n");

        if (retCode < 0)
        {
            // Unable to write message to file
            // Log message
            MessageLoaderParms parm(
                "Common.TraceFileHandlerWindows.UNABLE_TO_WRITE_TRACE_TO_FILE",
                "Unable to write trace message to File $0",
                _fileName);
            _logError(TRCFH_UNABLE_TO_WRITE_TRACE_TO_FILE,parm);
        }
        else
        {
            fflush(_fileHandle);
            // trace message successful written, reset error log messages
            // thus allow writing of errors to log again
            _logErrorBitField = 0;
        }
    
}

////////////////////////////////////////////////////////////////////////////////
//   Writes message to file.
//   Implementation of this function is platform specific
//
//   Note: The current implementation writes the message to the defined file.
//         Will have to be enhanced to support synchronous write operations to
//         the same file.
////////////////////////////////////////////////////////////////////////////////
void TraceFileHandler::handleMessage(const char* message, Uint32)
{
    Uint32 retCode;
   
    if (_configHasChanged)
    {
        _reConfigure();
    }

    if (!_fileHandle)
    {
        // The trace file is not open, which means an earlier fopen() was
        // unsuccessful.  Stop now to avoid logging duplicate error messages.
        return;
    }

    AutoMutex writeLock(writeMutex);

    if(!_fileExists(_fileName))
    {
        return;
    }

        //Move to the End of File
        fseek(_fileHandle,0,SEEK_SET);

        // Write message to file
        retCode = fprintf(_fileHandle,"%s\n", message);
        if (retCode < 0)
        {
            // Unable to write message to file
            // Log message
            MessageLoaderParms parm(
                "Common.TraceFileHandlerWindows.UNABLE_TO_WRITE_TRACE_TO_FILE",
                "Unable to write trace message to File $0",
                _fileName);
            _logError(TRCFH_UNABLE_TO_WRITE_TRACE_TO_FILE,parm);
        }
        else
        {
            fflush(_fileHandle);
            // trace message successful written, reset error log messages
            // thus allow writing of errors to log again
            _logErrorBitField = 0;
        }
    
}


PEGASUS_NAMESPACE_END
