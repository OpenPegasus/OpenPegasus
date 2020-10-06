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

#ifndef Pegasus_Tracer_h
#define Pegasus_Tracer_h

#include <cstdarg>
#include <Pegasus/Common/String.h>
#include <Pegasus/Common/System.h>
#include <Pegasus/Common/Logger.h>
#include <Pegasus/Common/InternalException.h>
#include <Pegasus/Common/TraceHandler.h>
#include <Pegasus/Common/Linkage.h>
#include <Pegasus/Common/SharedPtr.h>

PEGASUS_NAMESPACE_BEGIN

/**
    Trace component identifiers.  This list must be kept in sync with the
    TRACE_COMPONENT_LIST in Tracer.cpp.
    The tracer uses the _traceComponentMask 64bit field to mask
    the user configured components.

    Please ensure that no more than 64 Trace Component ID's are specified.
*/
enum TraceComponentId
{
    TRC_XML,
    TRC_XML_IO,
    TRC_HTTP,
    TRC_REPOSITORY,
    TRC_DISPATCHER,
    TRC_OS_ABSTRACTION,
    TRC_CONFIG,
    TRC_IND_HANDLER,
    TRC_AUTHENTICATION,
    TRC_AUTHORIZATION,
    TRC_USER_MANAGER,
    TRC_SHUTDOWN,
    TRC_SERVER,
    TRC_INDICATION_SERVICE,
    TRC_MESSAGEQUEUESERVICE,
    TRC_PROVIDERMANAGER,
    TRC_OBJECTRESOLUTION,
    TRC_WQL,
    TRC_CQL,
    TRC_FQL,
    TRC_THREAD,
    TRC_EXP_REQUEST_DISP,
    TRC_SSL,
    TRC_CONTROLPROVIDER,
    TRC_CIMOM_HANDLE,
    TRC_L10N,
    TRC_EXPORT_CLIENT,
    TRC_LISTENER,
    TRC_DISCARDED_DATA,
    TRC_PROVIDERAGENT,
    TRC_IND_FORMATTER,
    TRC_STATISTICAL_DATA,
    TRC_CMPIPROVIDER,
    TRC_INDICATION_GENERATION,
    TRC_INDICATION_RECEIPT,
    TRC_CMPIPROVIDERINTERFACE,
    TRC_WSMSERVER,
    TRC_RSSERVER,
#ifdef PEGASUS_ENABLE_PROTOCOL_WEB
    TRC_WEBSERVER,
#endif /* PEGASUS_ENABLE_PROTOCOL_WEB */
    TRC_LOGMSG,
    TRC_WMI_MAPPER_CONSUMER,
    TRC_INTERNALPROVIDER,
    TRC_ENUMCONTEXT
};

/** Token used for tracing functions.
*/
struct TracerToken
{
    TraceComponentId component;
    const char* method;
};

/** Tracer implements tracing of messages to a defined file
 */
class PEGASUS_COMMON_LINKAGE Tracer
{
public:

    /** Trace Components list defines the strings repesenting each
        TraceComponentId entry. Externalized to allow display of the
        possible list of trace components.  The size of this list is
        defined in _NUM_COMPONENTS variable.
     */
    static char const* TRACE_COMPONENT_LIST[];

    /** Trace facilities
        File - tracing occurs to the trace file
        Log  - tracing occurs through the Pegasus Logger class
        Keep the TRACE_FACILITY_LIST in sync with the TRACE_FACILITY_INDEX,
        so that the index matches the according string in the list.
     */
    static char const* TRACE_FACILITY_LIST[];

    enum TRACE_FACILITY_INDEX
    {
        TRACE_FACILITY_FILE = 0,
        TRACE_FACILITY_LOG  = 1,
        TRACE_FACILITY_MEMORY = 2
    };


    /** Levels of trace
        Trace messages are written to the trace file only if they are at or
        above a given trace level
        LEVEL1 - Severe and log messages
        LEVEL2 - Basic flow trace messages, low data detail
        LEVEL3 - Inter-function logic flow, medium data detail
        LEVEL4 - High data detail
     */
    static const Uint32 LEVEL1;
    static const Uint32 LEVEL2;
    static const Uint32 LEVEL3;
    static const Uint32 LEVEL4;

    /** Traces the given character string.
        Overloaded to include the filename
        and line number of trace origin.
        @param fileName        filename of the trace originator
        @param lineNum         line number of the trace originator
        @param traceComponent  component being traced
        @param cstring         the character string to be traced
     */
    static void traceCString(
        const char* fileName,
        const Uint32 lineNum,
        const TraceComponentId traceComponent,
        const char* cstring);

    /** Traces the message in the given CIMException object.  The message
        written to the trace file will include the source filename and
        line number of the CIMException originator.
        @param traceComponent  component being traced
        @param level           trace level of the trace message
        @param cimException    the CIMException to be traced.
     */
    static void traceCIMException(
        const TraceComponentId traceComponent,
        const Uint32 level,
        const CIMException& cimException);

    /** Formats the message given in data as hex dump if binary is true
        @param data      Message to be formatted
        @param binary    flag indicating if message is binary or not
    */
    static SharedArrayPtr<char> traceFormatChars(
        const Buffer& data,
        bool binary);

    /** Gets an HTTP request message.

        Given a binary HTTP request message(application/x-openpegasus
        this method returns the request message formatted in hex dump format
        and returns.

        Given an HTTP request message, this method checks if the
        message contains a "Basic" authorization header.

        If true, the username/passwd is suppressed and returned.
        Otherwise the request message is returned without any changes.

        @param requestMessage  requestMessage to be checked

        @return request message

    */
    static SharedArrayPtr<char> getHTTPRequestMessage(
        const Buffer& requestMessage);

    /** Set the trace file to the given file
        @param  traceFile  full path of the trace file
        @return 0          if the filepath is valid
                1          if the traceFile is an empty string or
                           if an error occurs while opening the file
                           in append mode
    */
    static Uint32 setTraceFile(const char* traceFile);

    /** Set the trace level to the given level
        @param  level  trace level to be set
        @return 0      if trace level is valid
                1      if trace level is invalid
    */
    static Uint32 setTraceLevel(const Uint32 level);

    /** Set components to be traced
        @param traceComponents list of components to be traced,
               components should be separated by ','
    */
    static void setTraceComponents(
       const String& traceComponents);

    /** Set trace facility to be used
        @param traceFacility facility to be used for tracing,
               for example Log or File.
        @return 0      if trace facility is valid
                1      if trace facility is invalid
    */
    static Uint32 setTraceFacility(const String& traceFacility);

    /** Get trace facility currently in use
        @return TRACE_FACILITY_FILE - if trace facility is file
                TRACE_FACILITY_LOG - if trace facility is the log
                TRACE_FACILITY_MEMORY - if trace facility is memory tracing
    */
    static Uint32 getTraceFacility();

    /** Set buffer size to be used for the memory tracing facility
        @param bufferSize buffer size in Kbyte to be used for memory tracing
        @return true   if function was successfully.
    */
    static Boolean setTraceMemoryBufferSize(Uint32 bufferSize);

    /** Set the Max trace File Size and used for the File tracing
        @param maxLogFileSizeBytes size of cimserver.trc
    */
    static void setMaxTraceFileSize (const String &size);

    /** Set the Max trace File number
        @param maxLogFileNumber number of cimserver.trc in trace folder
    */
    static void setMaxTraceFileNumber(const String &numberOfFiles);

    /** Flushes the trace buffer to traceFilePath. This method will only
        have an effect when traceFacility=Memory.
    */
    static void flushTrace();

    /** Traces method entry.
        @param token           TracerToken
        @param fileName        filename of the trace originator
        @param lineNum         line number of the trace originator
        @param traceComponent  component being traced
        @param methodName      method being traced
     */
    static void traceEnter(
        TracerToken& token,
        const char* file,
        size_t line,
        TraceComponentId traceComponent,
        const char* method);

    /** Traces method exit.
        @param token    TracerToken containing component and method
        @param fileName filename of the trace originator
        @param lineNum  line number of the trace originator
     */
    static void traceExit(
        TracerToken& token,
        const char* file,
        size_t line);

    /** Validates the File Path for the trace File
        @param  filePath full path of the file
        @return 1        if the file path is valid
                0        if the file path is invalid
     */
    static Boolean isValidFileName(const char* filePath);

    /** Validates the trace components
        @param  traceComponents   comma separated list of trace components
        @return 1        if the components are valid
                0        if one or more components are invalid
     */
    static Boolean isValidComponents(const String& traceComponents);

    /** Validates the trace components
        @param  traceComponents   comma separated list of trace components
        @param  invalidComponents comma separated list of invalid components
        @return 1        if the components are valid
                0        if one or more components are invalid
     */
    static Boolean isValidComponents(
        const String& traceComponents,
        String& invalidComponents);

    /** Validates the trace facility string value
        @param  traceFacility   The trace facility as string
        @return 1        if the trace facility is valid
                0        if the trace facility is invalid
     */
    static Boolean isValidTraceFacility( const String& traceFacility );

    /** Signals the trace to be running OOP and provides the file name
        extension of  the trace file.  If non-empty, this
        value is used as an extension to the name of the trace file.
        @param  oopTraceFileExtension Trace file extension.
     */
    static void setOOPTraceFileExtension(const String& oopTraceFileExtension);

    /**
    */
    static Boolean isTraceOn() { return _traceOn; }

    // Checks if trace is enabled for the given component and trace level
    // @param    traceComponent  component being traced
    // @param    level      level of the trace message
    // @return   0               if the component and level are not enabled
    //           1               if the component and level are enabled
    static Boolean isTraceEnabled(
        const TraceComponentId traceComponent,
        const Uint32 traceLevel)
    {
        return ((traceLevel & _traceLevelMask) &&
                (_traceComponentMask & ((Uint64)1 << traceComponent)));
    }

private:

    /** A static single indicator if tracing is turned on allowing to
        determine the state of trace quickly without many instructions.
        Used to wrap the public static trace interface methods to avoid
        obsolete calls when tracing is turned off.
     */
    static Boolean _traceOn;

    /** Internal only Levels of trace
        These cannot be used in any of the trace calls directly, but are set
        by methods of the Tracer class for specific purposes, such as trace
        Enter and traceExit.
        LEVEL0 - Trace is switched off
        LEVEL5 - used for method enter & exit
     */
    static const Uint32 LEVEL0;
    static const Uint32 LEVEL5;

    static const char   _COMPONENT_SEPARATOR;
    static const Uint32 _NUM_COMPONENTS;
    static const Uint32 _STRLEN_MAX_UNSIGNED_INT;
    static const Uint32 _STRLEN_MAX_PID_TID;
    static Uint64                _traceComponentMask;
    static Uint32                _traceLevelMask;
    static Tracer*               _tracerInstance;
    Uint32                _traceMemoryBufferSize;
    Uint32                _traceFacility;
    Boolean               _runningOOP;
    TraceHandler*         _traceHandler;
    String                _traceFile;
    String                _oopTraceFileExtension;


    // Message Strings for function Entry and Exit
    static const char _METHOD_ENTER_MSG[];
    static const char _METHOD_EXIT_MSG[];

    // Function formats size bytes of binary data given in data in a nicely
    // readable hex format and writes the output to targetBuffer
    // Return value: Pointer to one byte behind last position that was written
    static char* _formatHexDump(
        char* targetBuffer,
        const char* data,Uint32 size);

    // Factory function to create an instance of the matching trace handler
    // for the given type of traceFacility.
    // @param    traceFacility  type of trace handler to create
    void _setTraceHandler( Uint32 traceFacility );

    // Validates if the given file path if it is eligible for writing traces.
    // @param    fileName      a file intended to be used to write traces
    static Boolean _isValidTraceFile(String fileName);

    // Traces the given message. Overloaded to include the file name and the
    // line number as one of the parameters.
    // @param    traceComponent  component being traced
    // @param    message         message header (file name:line number)
    // @param    *fmt            printf style format string
    // @param    argList         variable argument list
    static void _trace(
        const char* fileName,
        const Uint32 lineNum,
        const TraceComponentId traceComponent,
        const char* fmt,
        va_list argList);

    //  Traces the message in the given CIMException object.  The message
    //  to be written to the trace file will include the source filename and
    //  line number of the CIMException originator.
    //  @param    traceComponent  component being traced
    //  @param    CIMException    the CIMException to be traced.
    //
    static void _traceCIMException(
        const TraceComponentId traceComponent,
        const CIMException& cimException);

    // Called by all the trace interfaces to log message
    // consisting of a single character string to the trace file
    // @param    traceComponent  component being traced
    // @param    cstring         the string to be traced
    static void _traceCString(
        const TraceComponentId traceComponent,
        const char* message,
        const char* cstring);

    // Called by all the trace interfaces to log message
    // with variable number of arguments to the trace file
    // @param    traceComponent  component being traced
    // @param    *fmt            printf style format string
    // @param    argList         variable argument list
    static void _trace(
        const TraceComponentId traceComponent,
        const char* message,
        const char* fmt,
        va_list argList);

    // Traces method enter/exit
    // @param    fileName        filename of the trace originator
    // @param    lineNum         line number of the trace originator
    // @param    traceComponent  component being traced
    // @param    method          name of the method
    static void _traceMethod(
        const char* fileName,
        const Uint32 lineNum,
        const TraceComponentId traceComponent,
        const char* methodEntryExit,
        const char* method);

    // Tracer constructor
    // Constructor is private to prevent construction of Tracer objects
    // Single Instance of Tracer is maintained for each process.
    Tracer();

    //   Tracer destructor
    ~Tracer();

    // Returns the Singleton instance of the Tracer
    // @return   Tracer*  Instance of Tracer
    static Tracer* _getInstance();

    friend class TraceCallFrame;
    friend class TracePropertyOwner;
    friend class TraceMemoryHandler;
    friend class TraceFileHandler;
};

//==============================================================================
//
// PEGASUS_REMOVE_TRACE defines the compile time inclusion of the Trace
// interfaces. If defined the interfaces map to empty functions.
//
//==============================================================================

#ifdef PEGASUS_REMOVE_TRACE

inline void Tracer::traceCString(
    const char* fileName,
    const Uint32 lineNum,
    const TraceComponentId traceComponent,
    const char* cstring)
{
    // empty function
}

inline void Tracer::traceCIMException(
    const TraceComponentId traceComponent,
    const Uint32 level,
    const CIMException& cimException)
{
    // empty function
}

static SharedArrayPtr<char> getHTTPRequestMessage(
        const Buffer& requestMessage)
{
    //empty function
    return SharedArrayPtr<char>();
}

inline Uint32 Tracer::setTraceFile(const char* traceFile)
{
    // empty function
    return 0;
}

inline Uint32 Tracer::setTraceLevel(const Uint32 level)
{
    // empty function
    return 0;
}

inline void Tracer::setTraceComponents(const String& traceComponents)
{
    // empty function
}

inline Uint32 Tracer::setTraceFacility(const String& traceFacility)
{
    // empty function
    return 0;
}

inline Uint32 Tracer::getTraceFacility()
{
    // empty function
    return 0;
}

inline Boolean Tracer::setTraceMemoryBufferSize(Uint32 bufferSize)
{
    // empty function
    return true;
}

inline void Tracer::flushTrace()
{
    // empty function
    return;
}



#endif /* PEGASUS_REMOVE_TRACE */

//==============================================================================
//
// Tracing macros
//
//==============================================================================

// Defines a variable that bypasses inclusion of line and filename in output.
// #define PEGASUS_NO_FILE_LINE_TRACE=1 to exclude file names and line numbers
#ifdef PEGASUS_NO_FILE_LINE_TRACE
# define PEGASUS_COMMA_FILE_LINE /* empty */
# define PEGASUS_FILE_LINE_COMMA /* empty */
#else
# define PEGASUS_COMMA_FILE_LINE ,__FILE__,__LINE__
# define PEGASUS_FILE_LINE_COMMA __FILE__,__LINE__,
#endif

#ifdef PEGASUS_REMOVE_TRACE

# define PEG_METHOD_ENTER(comp,meth)
# define PEG_METHOD_EXIT()
# define PEG_TRACE(VAR_ARGS)
# define PEG_TRACE_CSTRING(comp,level,chars)

#else /* PEGASUS_REMOVE_TRACE */

// remover trace code for method enter/exit
# ifdef  PEGASUS_REMOVE_METHODTRACE
#  define PEG_METHOD_ENTER(comp,meth)
#  define PEG_METHOD_EXIT()
# else
#  define PEG_METHOD_ENTER(comp, meth) \
    TracerToken __tracerToken; \
    __tracerToken.method = 0; \
    do \
    { \
        if (Tracer::isTraceOn()) \
            Tracer::traceEnter( \
                __tracerToken PEGASUS_COMMA_FILE_LINE, comp, meth); \
    } \
    while (0)

#  define PEG_METHOD_EXIT() \
    do \
    { \
        if (Tracer::isTraceOn()) \
            Tracer::traceExit(__tracerToken PEGASUS_COMMA_FILE_LINE); \
    } \
    while (0)
# endif

// Macro to trace character lists.  the do construct allows this to appear
// as a single statement.
# define PEG_TRACE_CSTRING(comp, level, chars) \
    do \
    { \
        if (Tracer::isTraceOn()) \
        { \
            if (Tracer::isTraceEnabled(comp, level)) \
            { \
                Tracer::traceCString(PEGASUS_FILE_LINE_COMMA comp, chars); \
            } \
        } \
    } \
    while (0)

//
// This class is constructed with the same arguments passed to PEG_TRACE().
// The constructor saves all the fixed arguments and calls va_start() on
// the varying arguments (wherein the va_list argument is the ap member of
// this class). The PEG_TRACE() macro eventually calls invoke() with the
// file and line macros in order to write the trace entry. For more details,
// see the comments below on the PEG_TRACE() macro.
//
class TraceCallFrame
{
public:

    const char* file;
    Uint32 line;

    TraceCallFrame(const char* file_, Uint32 line_) : file(file_), line(line_)
    {
    }

    PEGASUS_FORMAT(4, 5)
    inline void invoke(
        const TraceComponentId component,
        const Uint32 level,
        const char* format,
        ...)
    {
        if (Tracer::isTraceEnabled(component, level))
        {
            va_list ap;
            va_start(ap, format);
            Tracer::_trace(file, line, component, format, ap);
            va_end(ap);
        }
    }

    ~TraceCallFrame()
    {
    }
};
//
// This macro is a wrapper for calling the printf-style form of the
// Tracer::trace() function. Since macros cannot have a varying number of
// arguments, PEG_TRACE() must be invoked with double parentheses. For
// example:
//
//     PEG_TRACE((TRC_HTTP, Tracer::LEVEL1, "Oops: %d", 999));
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
# define PEG_TRACE(VAR_ARGS) \
    do \
    { \
        if (Tracer::isTraceOn()) \
        { \
                TraceCallFrame frame(__FILE__, __LINE__); \
                frame.invoke VAR_ARGS; \
        } \
    } \
    while (0)

#endif /* !PEGASUS_REMOVE_TRACE */

PEGASUS_NAMESPACE_END

#endif /* Pegasus_Tracer_h */
