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


#ifndef Pegasus_WbemConsumer_Tracer_h
#define Pegasus_WbemConsumer_Tracer_h

#include <Pegasus/Common/Config.h>
#include <Pegasus/Common/Tracer.h>

// Defines a variable that bypasses inclusion of line and filename in output.
// #define PEGASUS_NO_FILE_LINE_TRACE=1 to exclude file names and line numbers
#ifdef PEGASUS_NO_FILE_LINE_TRACE
# define PEGASUS_COMMA_FILE_LINE /* empty */
# define PEGASUS_FILE_LINE_COMMA /* empty */
#else
# define PEGASUS_COMMA_FILE_LINE ,__FILE__,__LINE__
# define PEGASUS_FILE_LINE_COMMA __FILE__,__LINE__,
#endif

// Define the macros for method entry/exit, and tracing a given string

#ifdef PEGASUS_REMOVE_TRACE
    #define CNS_METHOD_ENTER(traceComponent,methodName) 
    #define CNS_METHOD_EXIT() 
    #define CNS_TRACE_STRING(traceComponent,traceLevel,traceString) 
#else
    /** Macro for tracing method entry
        @param    traceComponent  component being traced
        @param    methodName      name of the method
    */
    #define CNS_METHOD_ENTER(traceComponent,methodName) \
        Pegasus::TracerToken __tracerToken; \
        __tracerToken.method = 0; \
        Pegasus::Tracer::traceEnter(__tracerToken PEGASUS_COMMA_FILE_LINE, \
            traceComponent, methodName)

    /** Macro for tracing method exit
     */
    #define CNS_METHOD_EXIT() \
        Pegasus::Tracer::traceExit(__tracerToken PEGASUS_COMMA_FILE_LINE)

    /** Macro for tracing a string
        @param    traceComponent  component being traced
        @param    traceLevel      trace level of the trace message
        @param    traceString     the string to be traced
     */
    #define CNS_TRACE_STRING(traceComponent,traceLevel,traceString) \
        Pegasus::Tracer::trace(PEGASUS_FILE_LINE_COMMA traceComponent, \
            traceLevel,traceString)

//
// This macro is a wrapper for calling the printf-style form of the 
// Tracer::trace() function. Since macros cannot have a varying number of 
// arguments, CNS_TRACE() must be invoked with double parentheses. For
// example:
//
//     CNS_TRACE((TRC_HTTP, Tracer::LEVEL1, "Oops: %d", 999));
//
// This macro offers two advantages over the calling trace() directly.
//
//     1. It eliminates the call to trace() if isTraceOn() returns false.
//        This has proven to reduce the expense of servicing a request
//        (when tracing is off) by as much as 3%.
//
//     2. It implicitly injects the __FILE__ and __LINE__ macros, relieving
//        the caller of this burden.
//
    # define CNS_TRACE(VAR_ARGS) \
    do \
    { \
        if (Pegasus::Tracer::isTraceOn()) \
        { \
            Pegasus::TraceCallFrame frame(__FILE__, __LINE__); \
                frame.invoke VAR_ARGS; \
        } \
    } \
    while (0)  


#endif
#endif /* Pegasus_WbemConsumer_Tracer_h */

