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


#ifndef Pegasus_TraceLogHandler_h
#define Pegasus_TraceLogHandler_h

#include <cstdarg>
#include <cstdio>
#include <Pegasus/Common/String.h>
#include <Pegasus/Common/Linkage.h>
#include <Pegasus/Common/TraceHandler.h>

PEGASUS_NAMESPACE_BEGIN

/** TraceLogHandler implements tracing of messages to the logger facility
 */

class PEGASUS_COMMON_LINKAGE TraceLogHandler: public TraceHandler
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
        @param    msgLen   lenght of message without terminating '\0'
     */
    virtual void handleMessage(const char* message,
                               Uint32 msgLen);


    TraceLogHandler();

    virtual ~TraceLogHandler();

};

PEGASUS_NAMESPACE_END

#endif /* Pegasus_TraceLogHandler_h */
