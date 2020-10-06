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
#include <fstream>
#include <cstring>
#include <Pegasus/Common/Logger.h>
#include <Pegasus/Common/Tracer.h>
#include <Pegasus/Common/System.h>
#include <Pegasus/Common/FileSystem.h>
#include <Pegasus/Common/MessageLoader.h>
#include <Pegasus/Common/Executor.h>
#include <Pegasus/Common/Mutex.h>

#if defined(PEGASUS_USE_SYSLOGS)
# include <syslog.h>
#endif

PEGASUS_USING_STD;

PEGASUS_NAMESPACE_BEGIN


const Uint32 Logger::TRACE = (1 << 0);
const Uint32 Logger::INFORMATION = (1 << 1);
const Uint32 Logger::WARNING = (1 << 2);
const Uint32 Logger::SEVERE = (1 << 3);
const Uint32 Logger::FATAL = (1 << 4);

static char const* LOGLEVEL_LIST[] =
{
    "TRACE",
    "INFORMATION",
    "WARNING",
    "SEVERE",
    "FATAL"
};

LoggerRep* Logger::_rep = 0;
String Logger::_homeDirectory = ".";

const Uint32 Logger::_NUM_LOGLEVEL = 5;

Uint32 Logger::_severityMask;


///////////////////////////////////////////////////////////////////////////////
//
// LoggerRep
//
///////////////////////////////////////////////////////////////////////////////

#if defined(PEGASUS_USE_SYSLOGS)

class LoggerRep
{
public:

    LoggerRep(const String&)
    {
# ifdef PEGASUS_OS_ZOS
        logIdentity = strdup(System::CIMSERVER.getCString());
        // If System Log is used open it
        System::openlog(logIdentity, LOG_PID, LOG_DAEMON);
# endif
    }

    ~LoggerRep()
    {
# ifdef PEGASUS_OS_ZOS
        System::closelog();
        free(logIdentity);
# endif
    }

    // Actual logging is done in this routine
    void log(Logger::LogFileType,
        const String& systemId,
        Uint32 logLevel,
        const String localizedMsg)
    {
        // Log the message
        System::syslog(systemId, logLevel, localizedMsg.getCString());
    }

private:

# ifdef PEGASUS_OS_ZOS
    char* logIdentity;
# endif
};

#else    // !defined(PEGASUS_USE_SYSLOGS)

static const char* fileNames[] =
{
    "PegasusTrace.log",
    "PegasusStandard.log",
    "PegasusAudit.log",
    "PegasusError.log"
};
static const char* lockFileName = "PegasusLog.lock";

/*
    _constructFileName builds the absolute file name from homeDirectory
    and fileName.
*/
static CString _constructFileName(
    const String& homeDirectory,
    const char * fileName)
{
    String result;
    result.reserveCapacity(
        (Uint32)(homeDirectory.size() + 1 + strlen(fileName)));
    result.append(homeDirectory);
    result.append('/');
    result.append(fileName);
    return result.getCString();
}

class LoggerRep
{
public:

    LoggerRep(const String& homeDirectory)
    {
        // Add test for home directory set.

        // If home directory does not exist, create it.
        CString lgDir = homeDirectory.getCString();

        if (!System::isDirectory(lgDir))
            System::makeDirectory(lgDir);

        // KS: I put the second test in just in case some trys to create
        // a completly erronous directory.  At least we will get a message
        if (!System::isDirectory(lgDir))
        {
            MessageLoaderParms parms("Common.Logger.LOGGING_DISABLED",
                "Logging Disabled");

            cerr << MessageLoader::getMessage(parms);
        }

       //Filelocks are not used for VMS
# if !defined(PEGASUS_OS_VMS)
        _loggerLockFileName = _constructFileName(homeDirectory, lockFileName);

        // Open and close a file to make sure that the file exists, on which
        // file lock is requested
        FILE *fileLockFilePointer;
        fileLockFilePointer = fopen(_loggerLockFileName, "a+");
        if(fileLockFilePointer)
        {
            fclose(fileLockFilePointer);
        }
# endif

        _logFileNames[Logger::TRACE_LOG] = _constructFileName(homeDirectory,
                                               fileNames[Logger::TRACE_LOG]);

        _logFileNames[Logger::STANDARD_LOG] = _constructFileName(homeDirectory,
                                               fileNames[Logger::STANDARD_LOG]);

# ifdef PEGASUS_ENABLE_AUDIT_LOGGER
        _logFileNames[Logger::AUDIT_LOG] = _constructFileName(homeDirectory,
                                               fileNames[Logger::AUDIT_LOG]);
# endif

        _logFileNames[Logger::ERROR_LOG] = _constructFileName(homeDirectory,
                                               fileNames[Logger::ERROR_LOG]);
    }

    ~LoggerRep()
    {
    }

    // Actual logging is done in this routine
    void log(Logger::LogFileType logFileType,
        const String& systemId,
        Uint32 logLevel,
        const String &localizedMsg)
    {
        // Prepend the systemId to the incoming message
        String messageString(systemId);
        messageString.append(": ");
        messageString.append(localizedMsg);  // l10n

        // Get the logLevel String
        // This converts bitmap to string based on highest order
        // bit set
        // ATTN: KS Fix this more efficiently.
        const char* tmp = "";
        if (logLevel & Logger::TRACE) tmp =       "TRACE   ";
        if (logLevel & Logger::INFORMATION) tmp = "INFO    ";
        if (logLevel & Logger::WARNING) tmp =     "WARNING ";
        if (logLevel & Logger::SEVERE) tmp =      "SEVERE  ";
        if (logLevel & Logger::FATAL) tmp =       "FATAL   ";

# ifndef PEGASUS_OS_VMS
        // Acquire AutoMutex (for thread sync)
        // and AutoFileLock (for Process Sync).
        AutoMutex am(_mutex);
        AutoFileLock fileLock(_loggerLockFileName);

        Uint32  logFileSize = 0;

        // Read logFileSize to check if the logfile needs to be pruned.
        FileSystem::getFileSize(String(_logFileNames[logFileType]),
                                       logFileSize);

        // Check if the size of the logfile is exceeding _maxLogFileSizeBytes.
        if ( logFileSize > _maxLogFileSizeBytes)
        {
            // Prepare appropriate file name based on the logFileType.
            // Eg: if Logfile name is PegasusStandard.log, pruned logfile name
            // will be PegasusStandard-062607-122302.log,where 062607-122302
            // is the time stamp.
            String prunedLogfile(_logFileNames[logFileType],
                                (Uint32)strlen(_logFileNames[logFileType]) - 4);
            prunedLogfile.append('-');

            // Get timestamp,remove illegal chars in file name'/' and ':'
            // (: is illegal Open VMS) from the time stamp. Append the time
            // info to the file name.

            String timeStamp = System::getCurrentASCIITime();
            for ( unsigned int i = 0; i < timeStamp.size(); i++ )
            {
                if( timeStamp[i] == '/' || timeStamp[i] == ':')
                {
                    timeStamp.remove(i, 1);
                }
            }
            prunedLogfile.append(timeStamp);

            // Append '.log' to the file
            prunedLogfile.append( ".log");

            // Rename the logfile
            FileSystem::renameFile(String(_logFileNames[logFileType]),
                                   prunedLogfile);

        } // Check if the logfile needs to be pruned.
# endif  // ifndef PEGASUS_OS_VMS

        // Open Logfile. Based on the value of logFileType, one of the five
        // Logfiles will be opened.
        ofstream logFileStream;
        logFileStream.open(_logFileNames[logFileType], ios::app);
        logFileStream << System::getCurrentASCIITime()
           << " " << tmp << (const char *)messageString.getCString() << endl;
        logFileStream.close();
    }

    static void setMaxLogFileSize(Uint32 maxLogFileSizeBytes)
    {
        _maxLogFileSizeBytes = maxLogFileSizeBytes;
    }
private:

    CString _logFileNames[int(Logger::NUM_LOGS)];

    static Uint32 _maxLogFileSizeBytes;
# ifndef PEGASUS_OS_VMS
    CString _loggerLockFileName;
    Mutex _mutex;
# endif
};

Uint32 LoggerRep::_maxLogFileSizeBytes;

#endif    // !defined(PEGASUS_USE_SYSLOGS)


///////////////////////////////////////////////////////////////////////////////
//
// Logger
//
///////////////////////////////////////////////////////////////////////////////

void Logger::_putInternal(
    LogFileType logFileType,
    const String& systemId,
    Uint32 logLevel,
    const String& message)
{
    if (!_rep)
       _rep = new LoggerRep(_homeDirectory);

    // Call the actual logging routine is in LoggerRep.
    _rep->log(logFileType, systemId, logLevel, message);

    // PEP 315
    // The trace can be routed into the log. The logged trace messages are
    // logged with logFileType of Logger::TRACE_LOG.
    // To avoid a cirular writing of these messages, log messages with
    // logFileType of Logger::TRACE_LOG are never send to the trace.
    if (Logger::TRACE_LOG != logFileType)
    {
        // For all other logFileType's send the log messages to the trace.
        // But do not write log messages to trace when the trace facility is
        // set to log. This avoids double messages.
        if (Tracer::TRACE_FACILITY_LOG != Tracer::getTraceFacility())
        {
            PEG_TRACE_CSTRING(
                TRC_LOGMSG,
                Tracer::LEVEL1,
                (const char*) message.getCString());
        }
    }
}

////////////////////////////////////////////////////////////////////////////////
//
// Public methods start here:
//
////////////////////////////////////////////////////////////////////////////////

void Logger::put(
    LogFileType logFileType,
    const String& systemId,
    Uint32 logLevel,
    const String& formatString,
    const Formatter::Arg& arg0,
    const Formatter::Arg& arg1,
    const Formatter::Arg& arg2,
    const Formatter::Arg& arg3,
    const Formatter::Arg& arg4,
    const Formatter::Arg& arg5,
    const Formatter::Arg& arg6,
    const Formatter::Arg& arg7,
    const Formatter::Arg& arg8,
    const Formatter::Arg& arg9)
{
    if (wouldLog(logLevel))
    {
        Logger::_putInternal(logFileType, systemId, logLevel,
            Formatter::format(formatString, arg0, arg1, arg2, arg3,
                arg4, arg5, arg6, arg7, arg8, arg9));
    }
}

void Logger::put(
    LogFileType logFileType,
    const String& systemId,
    Uint32 logLevel,
    const String& formatString)
{
    if (wouldLog(logLevel))
    {
        Logger::_putInternal(logFileType, systemId, logLevel, formatString);
    }
}

void Logger::put(
    LogFileType logFileType,
    const String& systemId,
    Uint32 logLevel,
    const String& formatString,
    const Formatter::Arg& arg0)
{
    if (wouldLog(logLevel))
    {
        Logger::_putInternal(logFileType, systemId, logLevel,
            Formatter::format(formatString, arg0));
    }
}

void Logger::put(
    LogFileType logFileType,
    const String& systemId,
    Uint32 logLevel,
    const String& formatString,
    const Formatter::Arg& arg0,
    const Formatter::Arg& arg1)
{
    if (wouldLog(logLevel))
    {
        Logger::_putInternal(logFileType, systemId, logLevel,
            Formatter::format(formatString, arg0, arg1));
    }
}

void Logger::put(
    LogFileType logFileType,
    const String& systemId,
    Uint32 logLevel,
    const String& formatString,
    const Formatter::Arg& arg0,
    const Formatter::Arg& arg1,
    const Formatter::Arg& arg2)
{
    if (wouldLog(logLevel))
    {
        Logger::_putInternal(logFileType, systemId, logLevel,
            Formatter::format(formatString, arg0, arg1, arg2));
    }
}

void Logger::put_l(
    LogFileType logFileType,
    const String& systemId,
    Uint32 logLevel,
    const MessageLoaderParms& msgParms)
{
    if (wouldLog(logLevel))
    {
        MessageLoaderParms parms = msgParms;
        parms.useProcessLocale = true;
        Logger::_putInternal(logFileType, systemId, logLevel,
            MessageLoader::getMessage(parms));
    }
}

void Logger::trace(
    LogFileType logFileType,
    const String& systemId,
    const String& message)
{
    if (wouldLog(Logger::TRACE))
    {
        Logger::_putInternal(logFileType, systemId, Logger::TRACE,
            message);
    }
}

void Logger::setHomeDirectory(const String& homeDirectory)
{
    _homeDirectory = homeDirectory;
}

void Logger::setlogLevelMask( const String &logLevelList )
{
    Uint32 logLevelType = 0;
    String logLevelName      = logLevelList;

    // Check if logLevel has been specified
    if (logLevelName != String::EMPTY)
    {
        // initialise _severityMask
        _severityMask = 0;

        // Set logLevelType to indicate the level of logging
        // required by the user.
        if (String::equalNoCase(logLevelName,"TRACE"))
        {
            logLevelType =  Logger::TRACE;
        }
        else if (String::equalNoCase(logLevelName,"INFORMATION"))
        {
            logLevelType =  Logger::INFORMATION;
        }
        else if (String::equalNoCase(logLevelName,"WARNING"))
        {
            logLevelType = Logger::WARNING;
        }
        else if (String::equalNoCase(logLevelName,"SEVERE"))
        {
            logLevelType = Logger::SEVERE;
        }
        else if (String::equalNoCase(logLevelName,"FATAL"))
        {
            logLevelType = Logger::FATAL;
        }
        // Setting _severityMask.  NOTE:  When adding new logLevels
        // it is essential that they are adding in ascending order
        // based on priority.  Once a case statement is true we will
        // continue to set all following log levels with a higher
        // priority.
        switch(logLevelType)
        {
            case Logger::TRACE:
                  _severityMask |= Logger::TRACE;
            case Logger::INFORMATION:
                  _severityMask |= Logger::INFORMATION;
            case Logger::WARNING:
                  _severityMask |= Logger::WARNING;
            case Logger::SEVERE:
                  _severityMask |= Logger::SEVERE;
            case Logger::FATAL:
                  _severityMask |= Logger::FATAL;
        }

        Executor::updateLogLevel(logLevelName.getCString());
    }
    else
    {
        // Property logLevel not specified, set default value.
        _severityMask = ~Logger::TRACE;
        Executor::updateLogLevel("INFORMATION");
    }
}

Boolean Logger::isValidlogLevel(const String &logLevel)
{
    // Validate the logLevel and modify the logLevel argument
    // to reflect the invalid logLevel

    Uint32    index=0;
    String    logLevelName = String::EMPTY;
    Boolean   validlogLevel=false;

    logLevelName = logLevel;

    if (logLevelName != String::EMPTY)
    {
        // Lookup the index for logLevel name in _logLevel_LIST
        index = 0;
        validlogLevel = false;

        while (index < _NUM_LOGLEVEL)
        {
            if (String::equalNoCase(logLevelName, LOGLEVEL_LIST[index]))
            {
                // Found logLevel, break from the loop
                validlogLevel = true;
                break;
            }
            else
            {
                index++;
            }
        }
    }
    else
    {
        // logLevels is empty, it is a valid value so return true
        return true;
    }

    return validlogLevel;
}

#if !defined (PEGASUS_USE_SYSLOGS)
void Logger::setMaxLogFileSize(Uint32 maxLogFileSizeBytes)
{
     LoggerRep::setMaxLogFileSize(maxLogFileSizeBytes);
}
#endif

PEGASUS_NAMESPACE_END
