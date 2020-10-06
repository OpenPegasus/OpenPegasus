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


#ifndef Pegasus_TraceFileHandler_h
#define Pegasus_TraceFileHandler_h

#include <cstdarg>
#include <cstdio>
#include <Pegasus/Common/String.h>
#include <Pegasus/Common/Linkage.h>
#include <Pegasus/Common/TraceHandler.h>

PEGASUS_NAMESPACE_BEGIN

/** TraceFileHandler implements logging of messages to file
 */

class PEGASUS_COMMON_LINKAGE TraceFileHandler: public TraceHandler
{
private:

    enum ErrLogMessageIds
    {
        TRCFH_FAILED_TO_OPEN_FILE_SYSMSG,
        TRCFH_UNEXPECTED_FILE_OWNER,
        TRCFH_FAILED_TO_SET_FILE_PERMISSIONS,
        TRCFH_UNABLE_TO_WRITE_TRACE_TO_FILE,
    };

    /** Open the specified file in append mode and ensure the file owner and
        permissions are appropriate.
        @param    fileName Full path of the file to open.
        @return   Handle to the opened file if successful, otherwise 0.
     */
    FILE* _openFile(const char* fileName);

    /** Function writes an error message to the log, but only once.
        This function is not thread safe which has to be taken care by the
        caller.
        @param    specifies the type of error message
        @param    parms MessageLoaderParms object containing the localizable
                  message to log.
     */
    void _logError(ErrLogMessageIds msgID, const MessageLoaderParms & parms);

    /* Function checking files exists or not and return boolean value
       if file not there then it will create file */
    Boolean _fileExists(char* fileName);
 
    /* File path to write messages
     */
    char* _fileName;


    /* Handle to the File
     */
    FILE* _fileHandle;

    /* Flag to track writing of message to log
     */
    Uint16 _logErrorBitField;
    Boolean _configHasChanged;

public:

    /** Writes message with format string to the tracing facility
        @param    message  message to be written
        @param    msgLen   lenght of message without terminating '\0'
        @param    fmt      printf style format string
        @param    argList  variable argument list
     */
    virtual void handleMessage(const char* message,
                               Uint32 msgLen,
                               const char* fmt,
                               va_list argList);

    /** Writes simple message to the tracing facility.
        @param    message  message to be written
        @param    msgLen   lenght of message without terminating '\0'
     */
    virtual void handleMessage(const char* message,
                               Uint32 msgLen);


    /** Informs the message handler that the configuraion
        of the trace has been updated.
     */
    virtual void configurationUpdated();

    /** Sets the Max trace file size for cimserver.trc
        @param   maxTraceFileSizeBytes maximum trace file size in bytes
     */
    void setMaxTraceFileSize (Uint32 maxTraceFileSizeBytes);

    /** Sets the Max trace file number for cimserver.trc in trace folder
        @param   maxTraceFileNumber maximum trace number
     */
    void setMaxTraceFileNumber(Uint32 maxTraceFileNumber);

    /** Create a new trace file until number of files reach
        after that it will start rolling all files 
        (means it will start replacing older files with new files). 
        @param fileName file to roll, this is the trace file name
     */
    void rollTraceFile(const char* fileName);

    TraceFileHandler();

    virtual ~TraceFileHandler();

private:
   
    void _reConfigure(void);
    Uint32 _maxTraceFileSizeBytes;
    Uint32 _maxTraceFileNumber;
};

PEGASUS_NAMESPACE_END

#endif /* Pegasus_TraceFileHandler_h */
