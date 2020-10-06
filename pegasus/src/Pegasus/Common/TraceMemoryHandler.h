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


#ifndef Pegasus_TraceMemoryHandler_h
#define Pegasus_TraceMemoryHandler_h

#include <cstdarg>
#include <cstdio>
#include <Pegasus/Common/String.h>
#include <Pegasus/Common/Linkage.h>
#include <Pegasus/Common/Tracer.h>
#include <Pegasus/Common/TraceHandler.h>
#include <Pegasus/Common/TraceFileHandler.h>
#include <Pegasus/Common/Mutex.h>
#include <Pegasus/Common/AtomicInt.h>

PEGASUS_NAMESPACE_BEGIN

/** TraceMemoryHandler implements tracing of messages to a memory buffer
 */
#define PEGASUS_TRC_DEFAULT_BUFFER_SIZE_KB 10*1024
#define PEGASUS_TRC_BUFFER_EYE_CATCHER "PEGASUSMEMTRACE"
#define PEGASUS_TRC_BUFFER_EYE_CATCHER_LEN 16
#define PEGASUS_TRC_BUFFER_TRUNC_MARKER "*TRUNC*"
#define PEGASUS_TRC_BUFFER_TRUNC_MARKER_LEN 7
#define PEGASUS_TRC_BUFFER_EOT_MARKER "*EOTRACE*"
#define PEGASUS_TRC_BUFFER_EOT_MARKER_LEN 9

class PEGASUS_COMMON_LINKAGE TraceMemoryHandler: public TraceHandler
{
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
     */
    virtual void handleMessage(const char* message,
                               Uint32 msgLen);

    /** Flushes the trace
     */
    virtual void flushTrace();

    /** Dumps the complete content of the trace buffer to a file
        @param    filename  name of the file where to dump the trace buffer
     */
    void dumpTraceBuffer(const char* filename);

    /** Tells an instance of the traceMemoryHandler that it will be destructed
        soon and should accept no more requests for trace messages.
     */
    void die();

    /*
        Constructs a TraceMemoryHandler. No trace memory allocated.
    */
    TraceMemoryHandler();

    virtual ~TraceMemoryHandler();

private:

    /** The trace area is defined a struct to keep the following information
        together in memory, next the trace statementes:
        - eyecatcher to locate the trace buffer in a dump
        - the size of the trace buffer
        - the position after the last written trace statement
    */
    struct traceArea_t
    {
        char eyeCatcher[PEGASUS_TRC_BUFFER_EYE_CATCHER_LEN];
        Uint32 bufferSize;
        Uint32 nextPos;
        char* traceBuffer;
    };

    char* _overflowBuffer;
    Uint32 _overflowBufferSize;
    struct traceArea_t *_traceArea;
    Uint32 _leftBytesInBuffer;

    // Members used for serialization
    AtomicInt _inUseCounter;
    AtomicInt _lockCounter;
    Boolean _dying;

    // Members used for statistics only
    AtomicInt _contentionCount;
    Uint32    _numberOfLocksObtained;

    // Name of a tracefile, in case we need to flush the buffer to a file
    char* _traceFileName;

    /** Request to lock the memory buffer for writing a trace message.
        @return 1        OK, you got the lock
                0        No lock was obtained, give up!!
    */
    Boolean _lockBufferAccess();

    /** Unlock the memory buffer when no longer used for writing.
    */
    void _unlockBufferAccess();

    /** Appends a fixed length message to be buffer
    */
    void _appendSimpleMessage(const char* message, Uint32 msgLen );

    /** Appends a marker after the last trace message in the buffer
    */
    void _appendMarker();

    /** Memory buffer initialization routine
    */
    void _initializeTraceArea();

};

PEGASUS_NAMESPACE_END

#endif /* Pegasus_TraceMemoryHandler_h */
