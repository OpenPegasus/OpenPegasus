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

#ifndef Pegasus_Logger_h
#define Pegasus_Logger_h

#include <Pegasus/Common/Config.h>
#include <Pegasus/Common/Formatter.h>
#include <Pegasus/Common/Linkage.h>
#include <Pegasus/Common/System.h>
#include <Pegasus/Common/MessageLoader.h>

PEGASUS_NAMESPACE_BEGIN

class LoggerRep;

/** This class provides the interface for writing log records to the log.
*/
class PEGASUS_COMMON_LINKAGE Logger
{
public:

    enum LogFileType
    {
        TRACE_LOG,
        STANDARD_LOG,
        AUDIT_LOG, // Use only if PEGASUS_ENABLE_AUDIT_LOGGER is defined
        ERROR_LOG
    };

    enum { NUM_LOGS = 4 };

    /** Log file Level - Defines the loglevel of the log entry irrespective of
        which log file it goes into. This is actually a bit mask as defined in
        logger.cpp. Thus, it serves both as a level of indication of the
        seriousness and possibly as a mask to select what is logged.
        ATTN: The selection test has not been done.
    */
    static const Uint32 TRACE;
    static const Uint32 INFORMATION;
    static const Uint32 WARNING;
    static const Uint32 SEVERE;
    static const Uint32 FATAL;

    /** Puts a message to the defined log file
        @param logFileType - Type of log file (Trace, etc.)
        @param systemId  - ID of the system generating the log entry within
        Pegasus. This is user defined but generally breaks down into major
        Pegasus components.
        @param level logLevel of the log entry. To be used both t0
        mark the log entry and tested against a mask to determine if log
        entry should be written.
        @param formatString     Format definition string for the Log. See the
        Formatter for details.
        @param Arg0 - Arg 9 - Up to 9 arguments representing the variables
        that go into the log entry.
        <pre>
        Logger::put(Logger::TRACE_LOG, System::CIMSERVER, Logger::WARNING,
            "X=$0, Y=$1, Z=$2", 88,  "Hello World", 7.5);
        </pre>
    */
    static void put(
        LogFileType logFileType,
        const String& systemId,
        Uint32 logLevel,
        const String& formatString,
        const Formatter::Arg& arg0,
        const Formatter::Arg& arg1,
        const Formatter::Arg& arg2,
        const Formatter::Arg& arg3,
        const Formatter::Arg& arg4 = Formatter::DEFAULT_ARG,
        const Formatter::Arg& arg5 = Formatter::DEFAULT_ARG,
        const Formatter::Arg& arg6 = Formatter::DEFAULT_ARG,
        const Formatter::Arg& arg7 = Formatter::DEFAULT_ARG,
        const Formatter::Arg& arg8 = Formatter::DEFAULT_ARG,
        const Formatter::Arg& arg9 = Formatter::DEFAULT_ARG);

    /** Optimized zero-argument form of put().
    */
    static void put(
        LogFileType logFileType,
        const String& systemId,
        Uint32 logLevel,
        const String& formatString);

    /** Optimized one-argument form of put().
    */
    static void put(
        LogFileType logFileType,
        const String& systemId,
        Uint32 logLevel,
        const String& formatString,
        const Formatter::Arg& arg0);

    /** Optimized two-argument form of put().
    */
    static void put(
        LogFileType logFileType,
        const String& systemId,
        Uint32 logLevel,
        const String& formatString,
        const Formatter::Arg& arg0,
        const Formatter::Arg& arg1);

    /** Optimized three-argument form of put().
    */
    static void put(
        LogFileType logFileType,
        const String& systemId,
        Uint32 logLevel,
        const String& formatString,
        const Formatter::Arg& arg0,
        const Formatter::Arg& arg1,
        const Formatter::Arg& arg2);

    /** put_l - Puts a localized message to the defined log file
        @param logFileType - Type of log file (Trace, etc.)
        @param systemId  - ID of the system generating the log entry within
        Pegasus. This is user defined but generally breaks down into major
        Pegasus components.
        @param level logLevel of the log entry. To be used both to
        mark the log entry and tested against a mask to determine if log
        entry should be written.
        @param msgParms MessageLoaderParms object containing the localizable
        message to log.
    */
    static void put_l(
        LogFileType logFileType,
        const String& systemId,
        Uint32 logLevel,
        const MessageLoaderParms& msgParms);

    /** Puts a trace message into the specified log.
    */
    static void trace(
        LogFileType logFileType,
        const String& systemId,
        const String& message);

    /** setHomeDirectory
    */
    static void setHomeDirectory(const String& homeDirectory);

    /** setlogLevelMask
    */
    static void setlogLevelMask(const String &logLevelList);

    /** Returns true if the given string is one of the legal log levels.
    */
    static Boolean isValidlogLevel(const String &logLevel);

    /** Tests if a log entry would be created by this call before
        the logger is called.  This function is intended to be used
        within the server for high usage log entries to avoid the
        overhead of doing the call when no log is created.
        @param logLevel Uint32 defining the level of the log to be
        executed.
        <p><b>Example:</b>
        <pre>
        if (Logger::wouldLog(Logger::TRACE))
        {
            Logger::put(Logger::STANDARD_LOG, System::CIMSERVER,
                Logger::TRACE, "HTTPMessage - HTTP header name: $0  "
                "HTTP header value: $1" ,name,value);
        }
        </pre>
    */
    static Boolean wouldLog(Uint32 logLevel)
    {
        return (_severityMask & logLevel) != 0;
    }

#if !defined(PEGASUS_USE_SYSLOGS)
    static void setMaxLogFileSize (Uint32 maxLogFileSizeBytes);
#endif

private:

    static LoggerRep* _rep;
    static String _homeDirectory;
    static Uint32 _severityMask;
    static const Uint32 _NUM_LOGLEVEL;

    static void _putInternal(
        LogFileType logFileType,
        const String& systemId,
        Uint32 logLevel,
        const String& message);
};

PEGASUS_NAMESPACE_END

#endif /* Pegasus_Logger_h */
