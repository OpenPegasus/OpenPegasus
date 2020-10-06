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

#include <Pegasus/Common/Config.h>
#include <Pegasus/Common/Constants.h>
#include <Pegasus/Common/Tracer.h>
#include <Pegasus/Common/TraceFileHandler.h>
#include <Pegasus/Common/TraceLogHandler.h>
#include <Pegasus/Common/TraceMemoryHandler.h>
#include <Pegasus/Common/Thread.h>
#include <Pegasus/Common/System.h>
#include <Pegasus/Common/HTTPMessage.h>
#include <Pegasus/Common/StringConversion.h>
#include <Pegasus/Common/FileSystem.h>

PEGASUS_USING_STD;

PEGASUS_NAMESPACE_BEGIN

/**
    String constants for naming the various Trace components.
    These strings will used when turning on tracing for the respective
    components.  The component list must be kept in sync with the
    TraceComponentId enumeration.

    The tracer uses the _traceComponentMask in form of a 64bit field to mask
    the user configured components.
    Please ensure that no more than 64 components are specified in the
    TRACE_COMPONENT_LIST.

    The following example shows the usage of trace component names.
    The setTraceComponents method is used to turn on tracing for the
    components: Config and Repository. The component names are passed as a
    comma separated list.

       Tracer::setTraceComponents("Config,Repository");
*/
char const* Tracer::TRACE_COMPONENT_LIST[] =
{
    "Xml",
    "XmlIO",
    "Http",
    "Repository",
    "Dispatcher",
    "OsAbstraction",
    "Config",
    "IndicationHandler",
    "Authentication",
    "Authorization",
    "UserManager",
    "Shutdown",
    "Server",
    "IndicationService",
    "MessageQueueService",
    "ProviderManager",
    "ObjectResolution",
    "WQL",
    "CQL",
    "FQL",
    "Thread",
    "CIMExportRequestDispatcher",
    "SSL",
    "ControlProvider",
    "CIMOMHandle",
    "L10N",
    "ExportClient",
    "Listener",
    "DiscardedData",
    "ProviderAgent",
    "IndicationFormatter",
    "StatisticalData",
    "CMPIProvider",
    "IndicationGeneration",
    "IndicationReceipt",
    "CMPIProviderInterface",
    "WsmServer",
    "RsServer",
#ifdef PEGASUS_ENABLE_PROTOCOL_WEB
    "WebServer",
#endif
    "LogMessages",
    "WMIMapperConsumer",
    "InternalProvider",
    "EnumContext"
};

// Set the number of defined components
const Uint32 Tracer::_NUM_COMPONENTS =
    sizeof(TRACE_COMPONENT_LIST)/sizeof(TRACE_COMPONENT_LIST[0]);


// Defines the value values for trace facilities
// Keep the TRACE_FACILITY_LIST in sync with the TRACE_FACILITY_INDEX,
// so that the index matches the according string in the list.
char const* Tracer::TRACE_FACILITY_LIST[] =
{
    "File",
    "Log",
    "Memory",
    0
};


// Set the trace levels
// These levels will be compared against a trace level mask to determine
// if a specific trace level is enabled

const Uint32 Tracer::LEVEL0 =  0;
const Uint32 Tracer::LEVEL1 = (1 << 0);
const Uint32 Tracer::LEVEL2 = (1 << 1);
const Uint32 Tracer::LEVEL3 = (1 << 2);
const Uint32 Tracer::LEVEL4 = (1 << 3);
const Uint32 Tracer::LEVEL5 = (1 << 4);

// Set the Enter and Exit messages
const char Tracer::_METHOD_ENTER_MSG[] = "Entering method";
const char Tracer::_METHOD_EXIT_MSG[]  = "Exiting method";

// Initialize singleton instance of Tracer
Tracer* Tracer::_tracerInstance = 0;

// Set component separator
const char Tracer::_COMPONENT_SEPARATOR = ',';

// Set the line maximum
const Uint32 Tracer::_STRLEN_MAX_UNSIGNED_INT = 21;

// Set the max PID and Thread ID Length
const Uint32 Tracer::_STRLEN_MAX_PID_TID = 21;

// Initialize public indicator of trace state
Boolean Tracer::_traceOn=false;
Uint32  Tracer::_traceLevelMask=0;
Uint64  Tracer::_traceComponentMask=(Uint64)0;

////////////////////////////////////////////////////////////////////////////////
// Tracer constructor
// Constructor is private to preclude construction of Tracer objects
// Single Instance of Tracer is maintained for each process.
////////////////////////////////////////////////////////////////////////////////
Tracer::Tracer()
    : _traceMemoryBufferSize(PEGASUS_TRC_DEFAULT_BUFFER_SIZE_KB),
      _traceFacility(TRACE_FACILITY_FILE),
      _runningOOP(false),
      _traceHandler(0)
{

    // The tracer uses a 64bit field to mask the user configured components.
    // This assert ensures that no more than 64 components are specified in the
    // TRACE_COMPONENT_LIST.
    PEGASUS_ASSERT(_NUM_COMPONENTS <= 64);

    // Instantiate trace handler according to configured facility
    _setTraceHandler(_traceFacility);
}

////////////////////////////////////////////////////////////////////////////////
//Tracer destructor
////////////////////////////////////////////////////////////////////////////////
Tracer::~Tracer()
{
    delete _traceHandler;
    delete _tracerInstance;
}


////////////////////////////////////////////////////////////////////////////////
//Factory function for the trace handler instances.
////////////////////////////////////////////////////////////////////////////////
void Tracer::_setTraceHandler( Uint32 traceFacility )
{
    TraceHandler * oldTrcHandler = _traceHandler;

    switch(traceFacility)
    {
        case TRACE_FACILITY_LOG:
            _traceFacility = TRACE_FACILITY_LOG;
            _traceHandler = new TraceLogHandler();
            break;

        case TRACE_FACILITY_MEMORY:
            _traceFacility = TRACE_FACILITY_MEMORY;
            _traceHandler = new TraceMemoryHandler();
            break;

        case TRACE_FACILITY_FILE:
        default:
            _traceFacility = TRACE_FACILITY_FILE;
            _traceHandler = new TraceFileHandler();
    }
    delete oldTrcHandler;
}

////////////////////////////////////////////////////////////////////////////////
// Validates if a given file path if it is eligible for writing traces.
////////////////////////////////////////////////////////////////////////////////
Boolean Tracer::_isValidTraceFile(String fileName)
{
    // Check if the file path is a directory
    FileSystem::translateSlashes(fileName);
    if (FileSystem::isDirectory(fileName))
    {
        return false;
    }

    // Check if the file exists and is writable
    if (FileSystem::exists(fileName))
    {
        return FileSystem::canWrite(fileName);
    }

    // Check if directory is writable
    Uint32 index = fileName.reverseFind('/');

    if (index != PEG_NOT_FOUND)
    {
        String dirName = fileName.subString(0,index);

        if (dirName.size() == 0)
        {
            dirName = "/";
        }

        if (!FileSystem::isDirectory(dirName))
        {
            return false;
        }

        return FileSystem::canWrite(dirName);
    }

    String currentDir;

    // Check if there is permission to write in the
    // current working directory
    FileSystem::getCurrentDirectory(currentDir);

    return FileSystem::canWrite(currentDir);
}

////////////////////////////////////////////////////////////////////////////////
//Traces the given message - Overloaded for including FileName and Line number
////////////////////////////////////////////////////////////////////////////////
void Tracer::_trace(
    const char* fileName,
    const Uint32 lineNum,
    const TraceComponentId traceComponent,
    const char* fmt,
    va_list argList)
{
    char* message;
    //
    // Allocate memory for the message string
    // Needs to be updated if additional info is added
    //
    message = new char[strlen(fileName) +
        _STRLEN_MAX_UNSIGNED_INT + (_STRLEN_MAX_PID_TID * 2) + 8];
    sprintf(
       message,
       "[%u:%s:%s:%u]: ",
       System::getPID(),
       Threads::id().buffer,
       fileName,
       lineNum);

    _trace(traceComponent, message, fmt, argList);
    delete [] message;
}

////////////////////////////////////////////////////////////////////////////////
//Traces the message in the given CIMException object.
////////////////////////////////////////////////////////////////////////////////
void Tracer::_traceCIMException(
    const TraceComponentId traceComponent,
    const CIMException& cimException)
{
    // get the CIMException trace message string
    CString traceMsg =
        TraceableCIMException(cimException).getTraceDescription().getCString();
    // trace the string
    _traceCString(traceComponent, "", (const char*) traceMsg);
}

char* Tracer::_formatHexDump(
    char* targetBuffer,
    const char * data,
    Uint32 size)
{
    unsigned char* p = (unsigned char*)data;
    unsigned char buf[16];
    size_t n = 0;
    int len;

    for (size_t i = 0, col = 0; i < size; i++)
    {
        unsigned char c = p[i];
        buf[n++] = c;

        if (col == 0)
        {
            len = sprintf(targetBuffer, "%06X ", (unsigned int)i);
            targetBuffer+=len;
        }

        len = sprintf(targetBuffer, "%02X", c);
        targetBuffer+=len;

        if ( ((col+1) & 3) == 0 )
        {
            *targetBuffer = ' ';
            targetBuffer++;
        }
        if (col + 1 == sizeof(buf) || i + 1 == size)
        {
            for (size_t j = col + 1; j < sizeof(buf); j++)
            {
                targetBuffer[0]=' ';
                targetBuffer[1]=' ';
                targetBuffer[2]=' ';
                targetBuffer += 3;
            }
            for (size_t j = 0; j < n; j++)
            {
                c = buf[j];

                if (c >= ' ' && c <= '~')
                {
                    *targetBuffer = c;
                }
                else
                {
                    *targetBuffer = '.';
                }
                targetBuffer++;
            }
            *targetBuffer = '\n';
            targetBuffer++;
            n = 0;
        }
        if (col + 1 == sizeof(buf))
        {
            col = 0;
        }
        else
        {
            col++;
        }
    }
    *targetBuffer = '\n';
    targetBuffer++;
    return targetBuffer;
}

SharedArrayPtr<char> Tracer::traceFormatChars(
    const Buffer& data,
    bool binary)
{
    static char start[]="\n### Begin of binary data\n";
    static char end[]="\n### End of binary data\n";
    static char msg[] ="\n### Parts of data omitted. Only first 768 bytes and "\
        "last 256 bytes shown. For complete information, use traceLevel 5.\n\n";

    SharedArrayPtr<char> outputBuffer(
        new char[(10*data.size()+sizeof(start)+sizeof(end)+sizeof(msg))]);

    char* target = outputBuffer.get();
    size_t size = data.size();

    if (0 == size)
    {
        target[0] = 0;
        return outputBuffer;
    }
    if (binary)
    {
        memcpy(target,&(start[0]),sizeof(start)-1);
        target+=sizeof(start)-1;
        // If there are more then 1024 bytes of binary data and the trace level
        // is not at highest level(5), we only trace part of the data and not
        // everything
        if ((_traceLevelMask & Tracer::LEVEL5) || (size <= 1024))
        {
            target=_formatHexDump(target, data.getData(), size);

        }
        else
        {
            target=_formatHexDump(target, data.getData(), 768);

            memcpy(target, &(msg[0]), sizeof(msg)-1);
            target+=sizeof(msg)-1;

            target=_formatHexDump(target, &(data.getData()[size-256]), 256);
        }
        memcpy(target,&(end[0]),sizeof(end));
    }
    else
    {
        memcpy(target, data.getData(), size);
        target[size] = 0;
    }
    return outputBuffer;
}

SharedArrayPtr<char> Tracer::getHTTPRequestMessage(
    const Buffer& requestMessage)
{
    const Uint32 requestSize = requestMessage.size();

    // Check if requestMessage contains "application/x-openpegasus"
    // and if true format the the requestBuf as HexDump for tracing
    //
    // Binary is only possible on localConnect and doesn't have Basic
    // authorization for that reason
    if (strstr(requestMessage.getData(),"application/x-openpegasus"))
    {
        return traceFormatChars(requestMessage,true);
    }

    // Make a copy of the request message.
    SharedArrayPtr<char>
        requestBuf(new char [requestSize + 1]);
    strncpy(requestBuf.get(), requestMessage.getData(), requestSize);
    requestBuf.get()[requestSize] = 0;

    //
    // Check if requestBuffer contains a Basic authorization header.
    // If true, suppress the user/passwd info in the request buffer.
    //
    char* sep;
    const char* line = requestBuf.get();

    while ((sep = HTTPMessage::findSeparator(line)) && (line != sep))
    {
        if (HTTPMessage::expectHeaderToken(line, "Authorization") &&
             HTTPMessage::expectHeaderToken(line, ":") &&
             HTTPMessage::expectHeaderToken(line, "Basic"))
        {
            // Suppress the user/passwd info
            HTTPMessage::skipHeaderWhitespace(line);
            for ( char* userpass = (char*)line ;
                userpass < sep;
                *userpass = 'X', userpass++)
            {
            }
            break;
        }

        line = sep + ((*sep == '\r') ? 2 : 1);
    }
    return requestBuf;
}

////////////////////////////////////////////////////////////////////////////////
//Traces method entry and exit
////////////////////////////////////////////////////////////////////////////////
void Tracer::_traceMethod(
    const char* fileName,
    const Uint32 lineNum,
    const TraceComponentId traceComponent,
    const char* methodEntryExit,
    const char* method)
{
    char* message;

    //
    // Allocate memory for the message string
    // Needs to be updated if additional info is added
    //
    // assume Method entry/exit string 15 characters long
    // +1 space character
    message = new char[ strlen(fileName) +
        _STRLEN_MAX_UNSIGNED_INT + (_STRLEN_MAX_PID_TID * 2) + 8
        + 16];

    sprintf(
       message,
       "[%u:%s:%s:%u]: %s ",
       System::getPID(),
       Threads::id().buffer,
       fileName,
       lineNum,
       methodEntryExit);

    _traceCString(traceComponent, message, method);

    delete [] message;
}

////////////////////////////////////////////////////////////////////////////////
//Called by all trace interfaces with variable arguments
//to log message to trace file
////////////////////////////////////////////////////////////////////////////////
void Tracer::_trace(
    const TraceComponentId traceComponent,
    const char* message,
    const char* fmt,
    va_list argList)
{
    char* msgHeader;
    Uint32 msgLen;
    Uint32 usec,sec;

    // Get the current system time and prepend to message
    System::getCurrentTimeUsec(sec,usec);

    //
    // Allocate messageHeader.
    // Needs to be updated if additional info is added
    //

    // Construct the message header
    // The message header is in the following format
    // timestamp: <component name> [file name:line number]
    //
    // Format string length calculation:
    //        11(sec)+ 2('s-')+11(usec)+4('us: ')+1(' ')+1(\0) = 30
    if (*message != '\0')
    {
       msgHeader = new char [strlen(message) +
           strlen(TRACE_COMPONENT_LIST[traceComponent]) + 30];

        msgLen = sprintf(msgHeader, "%us-%uus: %s %s", sec, usec,
            TRACE_COMPONENT_LIST[traceComponent], message);
    }
    else
    {
        //
        // Allocate messageHeader.
        // Needs to be updated if additional info is added
        //
        // Format string length calculation:
        //        11(sec)+2('s-')+11(usec)+4('us: ')+
        //        +2(' [')+1(':')+3(']: ')+1(\0) = 35
        msgHeader = new char[2 * _STRLEN_MAX_PID_TID +
            strlen(TRACE_COMPONENT_LIST[traceComponent]) + 35];

        msgLen = sprintf(msgHeader, "%us-%uus: %s [%u:%s]: ", sec, usec,
            TRACE_COMPONENT_LIST[traceComponent],
            System::getPID(), Threads::id().buffer);
    }

    // Call trace file handler to write message
    _getInstance()->_traceHandler->handleMessage(msgHeader,msgLen,fmt,argList);

    delete [] msgHeader;
}

////////////////////////////////////////////////////////////////////////////////
//Called by all trace interfaces using a character string without format string
//to log message to trace file
////////////////////////////////////////////////////////////////////////////////
void Tracer::_traceCString(
    const TraceComponentId traceComponent,
    const char* message,
    const char* cstring)
{
    char* completeMessage;
    Uint32 msgLen;
    Uint32 usec,sec;

    // Get the current system time and prepend to message
    System::getCurrentTimeUsec(sec,usec);

    //
    // Allocate completeMessage.
    // Needs to be updated if additional info is added
    //

    // Construct the message header
    // The message header is in the following format
    // timestamp: <component name> [file name:line number]
    //
    // Format string length calculation:
    //        11(sec)+ 2('s-')+11(usec)+4('us: ')+1(' ')+1(\0) = 30
    if (*message != '\0')
    {
       completeMessage = new char [strlen(message) +
           strlen(TRACE_COMPONENT_LIST[traceComponent]) +
           strlen(cstring) + 30];

        msgLen = sprintf(completeMessage, "%us-%uus: %s %s%s", sec, usec,
            TRACE_COMPONENT_LIST[traceComponent], message, cstring);
    }
    else
    {
        //
        // Allocate messageHeader.
        // Needs to be updated if additional info is added
        //
        // Format string length calculation:
        //        11(sec)+2('s-')+11(usec)+4('us: ')+
        //        +2(' [')+1(':')+3(']: ')+1(\0) = 35
        completeMessage = new char[2 * _STRLEN_MAX_PID_TID +
            strlen(TRACE_COMPONENT_LIST[traceComponent]) +
            strlen(cstring) +35];

        msgLen = sprintf(completeMessage, "%us-%uus: %s [%u:%s] %s", sec, usec,
            TRACE_COMPONENT_LIST[traceComponent],
            System::getPID(), Threads::id().buffer,
            cstring);
    }

    // Call trace file handler to write message
    _getInstance()->_traceHandler->handleMessage(completeMessage,msgLen);

    delete [] completeMessage;
}


////////////////////////////////////////////////////////////////////////////////
//Validate the trace file
////////////////////////////////////////////////////////////////////////////////
Boolean Tracer::isValidFileName(const char* filePath)
{
    Tracer* instance = _getInstance();
    String testTraceFile(filePath);

    if (instance->_runningOOP)
    {
        testTraceFile.append(".");
        testTraceFile.append(instance->_oopTraceFileExtension);
    }

    return _isValidTraceFile(testTraceFile);
}

////////////////////////////////////////////////////////////////////////////////
//Validate the trace components
////////////////////////////////////////////////////////////////////////////////
Boolean Tracer::isValidComponents(const String& traceComponents)
{
    String invalidComponents;
    return isValidComponents(traceComponents, invalidComponents);
}

Boolean Tracer::isValidComponents(
    const String& traceComponents,
    String& invalidComponents)
{
    // Validate the trace components and modify the traceComponents argument
    // to reflect the invalid components

    Uint32    position=0;
    Uint32    index=0;
    String    componentName;
    String    componentStr;
    Boolean   validComponent=false;
    Boolean   retCode=true;

    componentStr = traceComponents;
    invalidComponents = String::EMPTY;

    if (componentStr != String::EMPTY)
    {
        // Check if ALL is specified
        if (String::equalNoCase(componentStr,"ALL"))
        {
            return true;
        }

        // Append _COMPONENT_SEPARATOR to the end of the traceComponents
        componentStr.append(_COMPONENT_SEPARATOR);

        while (componentStr != String::EMPTY)
        {
            //
            // Get the Component name from traceComponents.
            // Components are separated by _COMPONENT_SEPARATOR
            //
            position = componentStr.find(_COMPONENT_SEPARATOR);
            componentName = componentStr.subString(0,(position));

            // Lookup the index for Component name in TRACE_COMPONENT_LIST
            index = 0;
            validComponent = false;

            while (index < _NUM_COMPONENTS)
            {
                if (String::equalNoCase(
                       componentName, TRACE_COMPONENT_LIST[index]))
                {
                    // Found component, break from the loop
                    validComponent = true;
                    break;
                }
                else
                {
                   index++;
                }
            }

            // Remove the searched componentname from the traceComponents
            componentStr.remove(0,position+1);

            if (!validComponent)
            {
                invalidComponents.append(componentName);
                invalidComponents.append(_COMPONENT_SEPARATOR);
            }
        }
    }
    else
    {
        // trace components is empty, it is a valid value so return true
        return true;
    }

    if (invalidComponents != String::EMPTY)
    {
        retCode = false;
        //
        // Remove the extra ',' at the end
        //
        invalidComponents.remove(
            invalidComponents.reverseFind(_COMPONENT_SEPARATOR));
    }
    return retCode;
}

////////////////////////////////////////////////////////////////////////////////
//Validate the trace facility
////////////////////////////////////////////////////////////////////////////////
Boolean Tracer::isValidTraceFacility(const String& traceFacility)
{
    Boolean retCode = false;

    if (traceFacility.size() != 0)
    {
        Uint32 index = 0;
        while (TRACE_FACILITY_LIST[index] != 0 )
        {
            if (String::equalNoCase(traceFacility,TRACE_FACILITY_LIST[index]))
            {
                retCode = true;
                break;
            }
            index++;
        }
    }

    return retCode;
}

////////////////////////////////////////////////////////////////////////////////
// Notify the trare running out of process and provide the trace file extension
// for the out of process trace file.
////////////////////////////////////////////////////////////////////////////////
void Tracer::setOOPTraceFileExtension(const String& oopTraceFileExtension)
{
    Tracer* instance = _getInstance();
    instance->_oopTraceFileExtension = oopTraceFileExtension;
    instance->_runningOOP=true;
    instance->_traceMemoryBufferSize /= PEGASUS_TRC_BUFFER_OOP_SIZE_DEVISOR;

}

////////////////////////////////////////////////////////////////////////////////
//Returns the Singleton instance of the Tracer
////////////////////////////////////////////////////////////////////////////////
Tracer* Tracer::_getInstance()
{
    if (_tracerInstance == 0)
    {
        _tracerInstance = new Tracer();
    }
    return _tracerInstance;
}

// PEGASUS_REMOVE_TRACE defines the compile time inclusion of the Trace
// interfaces. This section defines the trace functions IF the remove
// trace flag is NOT set.  If it is set, they are defined as empty functions
// in the header file.

#ifndef PEGASUS_REMOVE_TRACE

////////////////////////////////////////////////////////////////////////////////
//Set the trace file
////////////////////////////////////////////////////////////////////////////////
Uint32 Tracer::setTraceFile(const char* traceFile)
{
    if (*traceFile == 0)
    {
        return 1;
    }

    Tracer* instance = _getInstance();
    String newTraceFile(traceFile);

    if (instance->_runningOOP)
    {
        newTraceFile.append(".");
        newTraceFile.append(instance->_oopTraceFileExtension);
    }

    if (_isValidTraceFile(newTraceFile))
    {
        instance->_traceFile = newTraceFile;
        instance->_traceHandler->configurationUpdated();
    }
    else
    {
        return 1;
    }


    return 0;

}

////////////////////////////////////////////////////////////////////////////////
//Set the trace level
////////////////////////////////////////////////////////////////////////////////
Uint32 Tracer::setTraceLevel(const Uint32 traceLevel)
{
    Uint32 retCode = 0;

    switch (traceLevel)
    {
        case LEVEL0:
            _traceLevelMask = 0x00;
            break;

        case LEVEL1:
            _traceLevelMask = 0x01;
            break;

        case LEVEL2:
            _traceLevelMask = 0x03;
            break;

        case LEVEL3:
            _traceLevelMask = 0x07;
            break;

        case LEVEL4:
            _traceLevelMask = 0x0F;
            break;

        case LEVEL5:
            _traceLevelMask = 0x1F;
            break;

        default:
            _traceLevelMask = 0x00;
            retCode = 1;
    }

    // If one of the components was set for tracing and the traceLevel
    // is not zero, then turn on tracing.
    _traceOn=((_traceComponentMask!=(Uint64)0)&&(_traceLevelMask!=LEVEL0));

    return retCode;
}

////////////////////////////////////////////////////////////////////////////////
// Set components to be traced.
////////////////////////////////////////////////////////////////////////////////
void Tracer::setTraceComponents(const String& traceComponents)
{
    // Check if ALL is specified
    if (String::equalNoCase(traceComponents,"ALL"))
    {
        // initialize ComponentMask bit array to true
        _traceComponentMask = (Uint64)-1;

        // If tracing isn't turned off by a traceLevel of zero, let's
        // turn on the flag that activates tracing.
        _traceOn = (_traceLevelMask != LEVEL0);

        return;
    }

    // initialize ComponentMask bit array to false
    _traceComponentMask = (Uint64)0;
    _traceOn = false;

    if (traceComponents != String::EMPTY)
    {
        Uint32 index = 0;
        Uint32 position = 0;
        String componentName;
        String componentStr = traceComponents;


        // Append _COMPONENT_SEPARATOR to the end of the traceComponents
        componentStr.append(_COMPONENT_SEPARATOR);

        while (componentStr != String::EMPTY)
        {
            // Get the Component name from traceComponents.
            // Components are separated by _COMPONENT_SEPARATOR
            position = componentStr.find(_COMPONENT_SEPARATOR);
            componentName = componentStr.subString(0,(position));

            // Lookup the index for Component name in TRACE_COMPONENT_LIST
            index = 0;
            while (index < _NUM_COMPONENTS)
            {
                if (String::equalNoCase(
                    componentName,TRACE_COMPONENT_LIST[index]))
                {
                    _traceComponentMask=_traceComponentMask|((Uint64)1<<index);
                    // Found component, break from the loop
                    break;
                }
                else
                {
                    index++;
                }
            }
            // Remove the searched componentname from the traceComponents
            componentStr.remove(0,position+1);
        }
        // If one of the components was set for tracing and the traceLevel
        // is not zero, then turn on tracing.
        _traceOn=((_traceComponentMask!=(Uint64)0)&&(_traceLevelMask!=LEVEL0));
    }

    return ;
}

////////////////////////////////////////////////////////////////////////////////
// Set the trace facility to be used
////////////////////////////////////////////////////////////////////////////////
Uint32 Tracer::setTraceFacility(const String& traceFacility)
{
    Uint32 retCode = 0;
    Tracer* instance = _getInstance();

    if (traceFacility.size() != 0)
    {
        Uint32 index = 0;
        while (TRACE_FACILITY_LIST[index] != 0 )
        {
            if (String::equalNoCase( traceFacility,TRACE_FACILITY_LIST[index]))
            {
                if (index != instance->_traceFacility)
                {
                    instance->_setTraceHandler(index);
                }
                retCode = 1;
                break;
            }
            index++;
        }
    }

    return retCode;
}

////////////////////////////////////////////////////////////////////////////////
// Get the trace facility in use
////////////////////////////////////////////////////////////////////////////////
Uint32 Tracer::getTraceFacility()
{
    return _getInstance()->_traceFacility;
}

////////////////////////////////////////////////////////////////////////////////
// Set the size of the memory trace buffer
////////////////////////////////////////////////////////////////////////////////
Boolean Tracer::setTraceMemoryBufferSize(Uint32 bufferSize)
{
    Tracer* instance = _getInstance();
    if (instance->_runningOOP)
    {
        // in OOP we reduce the trace memory buffer by factor
        // PEGASUS_TRC_BUFFER_OOP_SIZE_DEVISOR
        instance->_traceMemoryBufferSize =
            bufferSize / PEGASUS_TRC_BUFFER_OOP_SIZE_DEVISOR;
    }
    else
    {
        instance->_traceMemoryBufferSize = bufferSize;
    }

    // If we decide to dynamically change the trace buffer size,
    // this is where it needs to be implemented.
    return true;
}

////////////////////////////////////////////////////////////////////////////////
// Flushes the trace buffer to traceFilePath. This method will only
// have an effect when traceFacility=Memory.
////////////////////////////////////////////////////////////////////////////////
void Tracer::flushTrace()
{
    _getInstance()->_traceHandler->flushTrace();
    return;
}


void Tracer::traceEnter(
    TracerToken& token,
    const char* file,
    size_t line,
    TraceComponentId traceComponent,
    const char* method)
{
    token.component = traceComponent;
    token.method = method;

    if (isTraceEnabled(traceComponent, LEVEL5))
    {
        _traceMethod(
            file, (Uint32)line, traceComponent,
            _METHOD_ENTER_MSG, method);
    }
}

void Tracer::traceExit(
    TracerToken& token,
    const char* file,
    size_t line)
{
    if (isTraceEnabled(token.component, LEVEL5) && token.method)
        _traceMethod(
            file, (Uint32)line, token.component,
            _METHOD_EXIT_MSG, token.method);
}

////////////////////////////////////////////////////////////////////////////////
//Traces the given string - Overloaded to include the fileName and line number
//of trace origin.
////////////////////////////////////////////////////////////////////////////////
void Tracer::traceCString(
    const char* fileName,
    const Uint32 lineNum,
    const TraceComponentId traceComponent,
    const char* cstring)
{
    char* message;

    Uint32 msgLen;
    Uint32 usec,sec;

    // Get the current system time
    System::getCurrentTimeUsec(sec,usec);

    //
    // Allocate memory for the message string
    // Needs to be updated if additional info is added
    //
    message = new char [strlen(fileName) +
        _STRLEN_MAX_UNSIGNED_INT + (_STRLEN_MAX_PID_TID * 2) + 8 +
        strlen(TRACE_COMPONENT_LIST[traceComponent]) +
        strlen(cstring) + 30];

    msgLen = sprintf(message, "%us-%uus: %s [%u:%s:%s:%u]: %s",
        sec,
        usec,
        TRACE_COMPONENT_LIST[traceComponent],
        System::getPID(),
        Threads::id().buffer,
        fileName,
        lineNum,
        cstring);

    // Call trace file handler to write message
    _getInstance()->_traceHandler->handleMessage(message,msgLen);

    delete [] message;
}

void Tracer::traceCIMException(
    const TraceComponentId traceComponent,
    const Uint32 traceLevel,
    const CIMException& cimException)
{
    if (isTraceEnabled(traceComponent, traceLevel))
    {
        _traceCIMException(traceComponent, cimException);
    }
}

#endif /* !PEGASUS_REMOVE_TRACE */

//set the trace file size only when the tracing is on a file
void Tracer::setMaxTraceFileSize(const String &size)
{
    Tracer *inst = _getInstance();
    if ( inst->getTraceFacility() == TRACE_FACILITY_FILE )
    {
        Uint32 traceFileSizeKBytes = 0;
        StringConversion::decimalStringToUint32(size, traceFileSizeKBytes);

        //Safe to typecast here as we know that handler is of type file
        TraceFileHandler *hdlr = (TraceFileHandler*) (inst->_traceHandler);

        hdlr->setMaxTraceFileSize(traceFileSizeKBytes*1024);

    }
}

//set the trace file number for rolling only when the tracing is on a file
void Tracer::setMaxTraceFileNumber(const String &maxTraceFileNumber)
{
    Tracer *inst = _getInstance();

    if ( inst->getTraceFacility() == TRACE_FACILITY_FILE )
    {
        Uint32 numberOfTraceFiles = 0;
        StringConversion::decimalStringToUint32(maxTraceFileNumber,
                                                numberOfTraceFiles);

        //Safe to typecast here as we know that handler is of type file
        TraceFileHandler *hdlr = (TraceFileHandler*) (inst->_traceHandler);

        hdlr->setMaxTraceFileNumber(numberOfTraceFiles);
     }
}


PEGASUS_NAMESPACE_END
