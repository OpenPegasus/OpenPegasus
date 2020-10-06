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

#include <fstream>
#include <cstring>
#include <Pegasus/Common/System.h>
#include <Pegasus/Common/Tracer.h>
#include <Pegasus/Common/TraceMemoryHandler.h>
#include <Pegasus/Common/AutoPtr.h>
#include <Pegasus/Common/SharedPtr.h>
#include <Pegasus/Common/CIMClass.h>
#include <Pegasus/Common/CIMMessage.h>
#include <Pegasus/Common/XmlWriter.h>
#include <Pegasus/Common/AcceptLanguageList.h>
#include <Pegasus/Common/ContentLanguageList.h>

PEGASUS_USING_STD;
PEGASUS_USING_PEGASUS;

//constants value
const String _traceFileSizeKBytes = "10240";
const String _numberOfTraceFiles = "3";

// Trace Levels 0 and 5 are defined as private constants of the tracer
// class to avoid inappropriate use in the trace calls and macros.
// Therefore these tracel levels need to be set in the tests using the
// constant values directly.
const Uint32 PEGASUS_TRACER_LEVEL0 =  0;
const Uint32 PEGASUS_TRACER_LEVEL5 = (1 << 4);


// Trace files for test purposes
// Will be created in the $(PEGASUS_TMP) directory, or if not set,
// in the current directory
CString FILE1;
CString FILE2;
CString FILE3;
CString FILE4;
CString FILE5;

// A message string for testing the tracer with variable arguments
#define VAR_TEST_MESSAGE "Variable length part of message"


//
// Reads the last trace message from a given trace file and compares the
// given string with the string read from file
//
// return 0 if the strings match
// return 1 if the strings do not match
//
Uint32 compare(const char* fileName, const char* expectedMessage)
{
    int expectedMessageLength = strlen(expectedMessage);

    // Compute the size of the message in the trace file.  Include the final
    // EOL character added by the Tracer.  This size will be used to seek
    // from the end of the file back to the beginning of the trace message.
    int seekBytes = expectedMessageLength + 1;

#if defined(PEGASUS_OS_TYPE_WINDOWS)
    // Windows converts all '\n' characters to "\r\n" sequences in the trace
    // file.  Increase the seekBytes by the number of '\r' characters added
    // when the message is written to the file.
    for (const char* newlineChar = expectedMessage;
         ((newlineChar = strchr(newlineChar, '\n')) != 0);
         newlineChar++)
    {
        seekBytes++;
    }

    // Count the '\r' character added with the final '\n' written by the Tracer
    seekBytes++;
#endif

    AutoArrayPtr<char> actualMessage(new char[expectedMessageLength + 1]);

    // Read the trace message from the file, minus the message prefix and
    // minus the trailing newline.
    fstream file;
    file.open(fileName, fstream::in);
    if (!file.good())
    {
        return 1;
    }
    file.seekg(-seekBytes, fstream::end);
    file.read(actualMessage.get(), expectedMessageLength);
    file.close();
    actualMessage[expectedMessageLength] = 0;

    // Compare the expected and actual messages
    Uint32 retCode = strcmp(expectedMessage, actualMessage.get());

    // Diagnostic to determine string differences
    if (retCode)
        cout << "Compare Error: expectedMessage= \n\"" << expectedMessage <<
            "\". actualMessage= \n\"" << actualMessage.get() << "\"" << endl;

    return retCode;
}

//
// Description:
// Trace properties file, level and component are not set
// Should not log a trace message
//
// Type:
// Negative
//
// return 0 if the test passed
// return 1 if the test failed
//
Uint32 test1()
{
    PEG_METHOD_ENTER(TRC_CONFIG,"test1");
    PEG_TRACE((TRC_CONFIG,Tracer::LEVEL2,"%s %d",
        "This message should not appear value=",123));
    PEG_METHOD_EXIT();
    return System::exists(FILE1) ? 1 : 0;
}

//
// Description:
// Trace properties level and component are not set
// Should not log a trace message
//
// Type:
// Negative
//
// return 0 if the test passed
// return 1 if the test failed
//
Uint32 test2()
{
    Tracer::setTraceFile(FILE1);
    Tracer::setMaxTraceFileSize (_traceFileSizeKBytes);
    Tracer::setMaxTraceFileNumber(_numberOfTraceFiles);
    PEG_METHOD_ENTER(TRC_CONFIG,"test2");
    PEG_TRACE((TRC_CONFIG,Tracer::LEVEL2,"%s %d",
        "This message should not appear value=",123));
    Uint32 fileSize = 0;
    System::getFileSize(FILE1, fileSize);
    return (fileSize == 0) ? 0 : 1;
}

//
// Description:
// Trace properties component is not set
// Should not log a trace message
//
// Type:
// Negative
//
// return 0 if the test passed
// return 1 if the test failed
//
Uint32 test3()
{
    Tracer::setTraceLevel(Tracer::LEVEL1);
    PEG_METHOD_ENTER(TRC_CONFIG,"test3");
    PEG_TRACE((TRC_CONFIG,Tracer::LEVEL2,"%s",
        "This message should not appear"));
    Uint32 fileSize = 0;
    System::getFileSize(FILE1, fileSize);
    return (fileSize == 0) ? 0 : 1;
}

//
// Description:
// Trace properties file, level and component are set
// should log a trace message
//
// Type:
// Positive
//
// return 0 if the test passed
// return 1 if the test failed
//
Uint32 test4()
{
    Tracer::setTraceLevel(Tracer::LEVEL1);
    Tracer::setTraceComponents("Config");
    PEG_TRACE_CSTRING(TRC_CONFIG,Tracer::LEVEL1,"test4");
    return(compare(FILE1,"test4"));
}

//
// Description:
// Trace component is set to an invalid component
// should not log a trace message
//
// Type:
// Negative
//
// return 0 if the test passed
// return 1 if the test failed
//
Uint32 test5()
{
    Tracer::setTraceComponents("Wrong Component Name");

    PEG_METHOD_ENTER(TRC_CONFIG,"test5");
    PEG_TRACE_CSTRING(TRC_CONFIG,Tracer::LEVEL1,"test5");
    PEG_METHOD_EXIT();
    return(compare(FILE1,"test4"));
}

//
// Description:
// Trace level is set to LEVEL 2 and logs a LEVEL 4 message
// should not log a trace message
//
// Type:
// Negative
//
// return 0 if the test passed
// return 1 if the test failed
//

Uint32 test6()
{
    Tracer::setTraceComponents("Config");
    Tracer::setTraceLevel(Tracer::LEVEL2);
    PEG_TRACE((TRC_CONFIG,Tracer::LEVEL2,"%s %s",
        "Test Message for Level2 in","test6"));
    PEG_TRACE((TRC_CONFIG,Tracer::LEVEL2,"%s %s",
        "Test Message for Level2 in","test6"));
    PEG_TRACE((TRC_CONFIG,Tracer::LEVEL4,"%s",
        "This Message should not appear"));
    return(compare(FILE1,"Test Message for Level2 in test6"));
}

//
// Description:
// Trace level is set to an invalid level
// should not log a trace message
//
// Type:
// Negative
//
// return 0 if the test passed
// return 1 if the test failed
//
Uint32 test7()
{
    Tracer::setTraceLevel(100);
    PEG_METHOD_ENTER(TRC_CONFIG,"test7");
    PEG_METHOD_EXIT();
    return(compare(FILE1,"Test Message for Level2 in test6"));
}

//
// Description:
// Changes the trace file to FILE2
//
// Type:
// Positive
//
// return 0 if the test passed
// return 1 if the test failed
//
Uint32 test9()
{
    Tracer::setTraceLevel(Tracer::LEVEL3);
    Tracer::setTraceFile(FILE2);

    PEG_METHOD_ENTER(TRC_CONFIG,"test9");
    PEG_TRACE((TRC_CONFIG,Tracer::LEVEL3,"%s %s",
        "Test Message for Level3 in","test9"));
    return(compare(FILE2,"Test Message for Level3 in test9"));
}

//
// Description:
// Passes invalid component in the trace call
// should not log a trace message
//
// Type:
// Negative
//
// return 0 if the test passed
// return 1 if the test failed
//
// This test not required with change to
// use and test macros only.

Uint32 test10()
{
    Tracer::setTraceComponents("ALL");
    PEG_METHOD_ENTER(TRC_CONFIG,"test10");
    PEG_METHOD_EXIT();
    return(compare(FILE2,"Test Message for Level3 in test9"));
}

//
// Description:
// Trace is set to level 0
// should not log a trace message
//
// Type:
// Negative
//
// return 0 if the test passed
// return 1 if the test failed
//

Uint32 test11()
{
    Tracer::setTraceComponents("ALL");
    Tracer::setTraceLevel(PEGASUS_TRACER_LEVEL0);
    PEG_TRACE((TRC_CONFIG,Tracer::LEVEL1,"%s %s",
        "Test Message for Level0 in","test11"));
    return(compare(FILE2,"Test Message for Level3 in test9"));
}

//
// Description:
// Implements trace call for Tracer::Level1
// should log a trace message
//
// Type:
// Positive
//
// return 0 if the test passed
// return 1 if the test failed
//

Uint32 test12()
{
    Tracer::setTraceComponents("ALL");
    Tracer::setTraceLevel(Tracer::LEVEL4);
    PEG_TRACE((TRC_CONFIG,Tracer::LEVEL1,"%s %s",
        "Test Message for Level1 in","test12"));
    return(compare(FILE2,"Test Message for Level1 in test12"));
}

//
// Description:
// Implements trace call for Tracer::Level2
// should log a trace message
//
// Type:
// Positive
//
// return 0 if the test passed
// return 1 if the test failed
//

Uint32 test13()
{
    Tracer::setTraceComponents("ALL");
    Tracer::setTraceLevel(Tracer::LEVEL4);
    PEG_TRACE((TRC_CONFIG,Tracer::LEVEL2,"%s %s",
        "Test Message for Level2 in","test13"));
    return(compare(FILE2,"Test Message for Level2 in test13"));
}

//
// Description:
// Implements trace call for Tracer::Level3
// should log a trace message
//
// Type:
// Positive
//
// return 0 if the test passed
// return 1 if the test failed
//

Uint32 test14()
{
    Tracer::setTraceComponents("ALL");
    Tracer::setTraceLevel(Tracer::LEVEL4);
    Tracer::setTraceFile(FILE3);
    PEG_TRACE((TRC_CONFIG,Tracer::LEVEL3,"%s %s",
        "Test Message for Level3 in","test14"));
    return(compare(FILE3,"Test Message for Level3 in test14"));
}

//
// Description:
// Implements trace call for Tracer::Level4
// should log a trace message
//
// Type:
// Positive
//
// return 0 if the test passed
// return 1 if the test failed
//

Uint32 test15()
{
    Tracer::setTraceComponents("ALL");
    Tracer::setTraceLevel(Tracer::LEVEL4);
    PEG_TRACE((TRC_CONFIG,Tracer::LEVEL4,"%s %s",
        "Test Message for Level4 in", "test15"));
    return(compare(FILE3,"Test Message for Level4 in test15"));
}

//
// Description:
// Implements trace call for Tracer::Level5 (trace enter/exit)
// should log a trace message
//
// Type:
// Positive
//
// return 0 if the test passed
// return 1 if the test failed
//

Uint32 test15a()
{
    Tracer::setTraceComponents("ALL");
    Tracer::setTraceLevel(PEGASUS_TRACER_LEVEL5);
    PEG_METHOD_ENTER(TRC_CONFIG,"test15a");
    return(compare(FILE3,"Entering method test15a"));
}

//
// Description:
// Implements trace call for Tracer::Level5 (trace enter/exit)
// should log a trace message
//
// Type:
// Positive
//
// return 0 if the test passed
// return 1 if the test failed
//

Uint32 test15b()
{
    Tracer::setTraceComponents("ALL");
    Tracer::setTraceLevel(PEGASUS_TRACER_LEVEL5);
    PEG_METHOD_ENTER(TRC_CONFIG,"test15b");
    PEG_METHOD_EXIT();
    return(compare(FILE3,"Exiting method test15b"));
}

//
// Description:
// calls the setTraceComponents with null string
// should not log a trace message
//
// Type:
// Negative
//
// return 0 if the test passed
// return 1 if the test failed
//

Uint32 test16()
{
    Tracer::setTraceComponents("ALL");
    Tracer::setTraceLevel(PEGASUS_TRACER_LEVEL5);
    PEG_TRACE_CSTRING(TRC_CONFIG,Tracer::LEVEL4,"test16 - check value");
    Tracer::setTraceComponents("");
    Tracer::setTraceLevel(Tracer::LEVEL4);
    PEG_TRACE((TRC_CONFIG,Tracer::LEVEL4,"%s %s",
    "This Message should not appear in","test16"));
    return(compare(FILE3,"test16 - check value"));
}

//
// Description:
// calls the setTraceComponents with one valid and another invalid component
// should log a trace message
//
// Type:
// Negative
//
// return 0 if the test passed
// return 1 if the test failed
//

Uint32 test17()
{
    Tracer::setTraceComponents("ALL");
    Tracer::setTraceLevel(PEGASUS_TRACER_LEVEL5);
    PEG_TRACE_CSTRING(TRC_CONFIG,Tracer::LEVEL4,"test17 - check value");
    Tracer::setTraceComponents("InvalidComp");
    Tracer::setTraceLevel(Tracer::LEVEL4);
    PEG_TRACE((TRC_CONFIG,Tracer::LEVEL4,"%s %s",
    "This Message should not appear in", "test17"));
    return(compare(FILE3,"test17 - check value"));
}
//
// Description:
// calls the _traceBuffer call
// should log a trace message
//
// Type:
// Positive
//
// return 0 if the test passed
// return 1 if the test failed
//

Uint32 test18()
{
    Tracer::setTraceComponents("Config,InvalidComp");
    Tracer::setTraceLevel(Tracer::LEVEL4);
    PEG_TRACE_CSTRING(TRC_CONFIG,Tracer::LEVEL4,
        "This Message should appear in");
    return(compare(FILE3,"This Message should appear in"));
}

//
// Description:
// Trace a string.
// Calls the PEG_TRACE macro
// should log a trace message
//
// Type:
// Positive
//
// return 0 if the test passed
// return 1 if the test failed
//

Uint32 test20()
{
    Tracer::setTraceFile(FILE4);
    Tracer::setTraceComponents("ALL");
    Tracer::setTraceLevel(Tracer::LEVEL4);

    PEG_METHOD_ENTER(TRC_CONFIG, "test20");
    PEG_TRACE((TRC_CONFIG,Tracer::LEVEL4,
    "Test Message for Level4 in test20"));
    return(compare(FILE4,"Test Message for Level4 in test20"));
}

//
// Description:
// Trace a CIMException.
// Calls the traceCIMException() method
// should log a trace message
//
// Type:
// Positive
//
// return 0 if the test passed
// return 1 if the test failed
//

Uint32 test21()
{
    Tracer::setTraceFile(FILE4);
    Tracer::setTraceComponents("ALL");
    Tracer::setTraceLevel(Tracer::LEVEL4);

    PEG_METHOD_ENTER(TRC_CONFIG, "test21");

    // test tracing CIMException
    try
    {
        throw PEGASUS_CIM_EXCEPTION(
            CIM_ERR_NOT_SUPPORTED,
            "CIM Exception Message for Level4 in test21.");
    }
    catch (CIMException& e)
    {
        Tracer::traceCIMException(TRC_CONFIG,Tracer::LEVEL4, e);
    }

    return 0;
}

//
// Description:
// Trace a string using macro.
// should log a trace message
//
// Type:
// Positive
//
// return 0 if the test passed
// return 1 if the test failed
//
Uint32 test22()
{
    Tracer::setTraceFile(FILE4);
    Tracer::setTraceComponents("ALL");
    Tracer::setTraceLevel(Tracer::LEVEL4);

    PEG_METHOD_ENTER(TRC_CONFIG, "test22");

    PEG_TRACE_CSTRING(TRC_CONFIG,Tracer::LEVEL4,
        "Test message for Level4 in test22.");

    return(compare(FILE4,"Test message for Level4 in test22."));
}

//
// Description:
// Trace a character string using macro.
// should log a trace message
//
// Type:
// Positive
//
// return 0 if the test passed
// return 1 if the test failed
//
Uint32 test23()
{
    Tracer::setTraceFile(FILE4);
    Tracer::setTraceComponents("ALL");
    Tracer::setTraceLevel(Tracer::LEVEL4);

    PEG_METHOD_ENTER(TRC_CONFIG, "test23");

    PEG_TRACE_CSTRING(TRC_CONFIG,Tracer::LEVEL4,
                      "Test message for Level4 in test23.");

    return(compare(FILE4,"Test message for Level4 in test23."));
}

//
// Description:
// Change traceFacility to Logger
// No more trace messages should be written
//
// Type:
// Negative
//
// return 0 if the test passed
// return 1 if the test failed
//
Uint32 test24()
{
    Tracer::setTraceFacility("Log");
    Tracer::setTraceFile(FILE4);
    Tracer::setTraceComponents("ALL");
    Tracer::setTraceLevel(Tracer::LEVEL4);

    PEG_TRACE_CSTRING(TRC_CONFIG,Tracer::LEVEL4,
                      "Test message for traceFacility=Log in test24.");

    return(compare(FILE4,"Test message for Level4 in test23."));
}

//
// Description:
// Change traceFacility back to File
// Trace messages should be written again
//
// Type:
// Positive
//
// return 0 if the test passed
// return 1 if the test failed
//
Uint32 test25()
{
    Tracer::setTraceFacility("File");
    Tracer::setTraceFile(FILE4);
    Tracer::setTraceComponents("ALL");
    Tracer::setTraceLevel(Tracer::LEVEL4);

    PEG_TRACE_CSTRING(TRC_CONFIG,Tracer::LEVEL4,
                      "Test message for traceFacility=File in test25.");

    return(compare(FILE4,"Test message for traceFacility=File in test25."));
}

//
// Description:
// Test the getHTTPRequestMessage method.
//
// Type:
// Positive
// Tests with a HTTP Request without a basic authorization header.
// Message is written to trace file without any changes.
//
// return 0 if the test passed
// return 1 if the test failed
//
Uint32 test26()
{
    Tracer::setTraceFile(FILE4);
    Tracer::setTraceComponents("xmlio");
    Tracer::setTraceLevel(Tracer::LEVEL2);

    CIMPropertyList propertyList;
    Buffer params;
    AcceptLanguageList al;
    ContentLanguageList cl;

    XmlWriter::appendClassNameIParameter(
        params, "ClassName", CIMName("testclass"));
    Buffer buffer = XmlWriter::formatSimpleIMethodReqMessage(
        "localhost",
        CIMNamespaceName("test/cimv2"),
        CIMName ("EnumerateInstanceNames"),
        "12345",
        HTTP_METHOD__POST,
        "Basic: Authorization AAAAA",
        al,
        cl,
        params,
        false);

    SharedArrayPtr<char> reqMsg(Tracer::getHTTPRequestMessage(
            buffer));

    PEG_TRACE((
        TRC_XML_IO,
        Tracer::LEVEL2,
        "<!-- Request: queue id: %u -->\n%s",
        18,
        reqMsg.get()));

    return(compare(FILE4, buffer.getData()));
}

//
// Description:
// Test the getHTTPRequestMessage method.
//
// Type:
// Positive
// Tests with a HTTP Request that contains a Basic authorization header.
// The user/password info in the message is suppressed before writing it to
// the trace file.
//
// return 0 if the test passed
// return 1 if the test failed
//
Uint32 test27()
{
    Tracer::setTraceFile(FILE4);
    Tracer::setTraceComponents("xmlio");
    Tracer::setTraceLevel(Tracer::LEVEL2);

    CIMPropertyList propertyList;
    Buffer params;
    AcceptLanguageList al;
    ContentLanguageList cl;
    String authHeader = "Authorization: Basic ABCDEABCDE==";
    String MSGID = "32423424";

    XmlWriter::appendClassNameIParameter(
        params,
        "ClassName",
        CIMName("testclass"));
    Buffer buffer = XmlWriter::formatSimpleIMethodReqMessage(
        "localhost",
        CIMNamespaceName("test/cimv2"),
        CIMName ("EnumerateInstanceNames"),
        MSGID,
        HTTP_METHOD__POST,
        authHeader,
        al,
        cl,
        params,
        false);

    PEG_TRACE((
        TRC_XML_IO,
        Tracer::LEVEL2,
        "<!-- Request: queue id: %u -->\n%s",
        18,
        Tracer::getHTTPRequestMessage(
            buffer).get()));

    String testStr(buffer.getData());
    Uint32 pos = testStr.find("ABCDEABCDE==");

    for ( Uint32 i = pos; i < pos+strlen("ABCDEABCDE=="); i++)
        testStr[i] = 'X';

    return(compare(FILE4, testStr.getCString()));
}


//----------------------------------------------------------
// Tests for the traceMemoryHandler
//----------------------------------------------------------
typedef struct TTTParmType
{
    Thread *trd;
    TraceMemoryHandler *trcHandler;
    const char* trcMessage;
    Uint32 msgLen;
    Uint32 number;
    Boolean isVariableMsg;
} TTTParm;


void traceVariableArgs( TraceMemoryHandler *trcHdler,
                        const char* msg, Uint32 msgLen, const char* fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    trcHdler->handleMessage(msg, msgLen, fmt, ap);
    va_end(ap);
}

ThreadReturnType PEGASUS_THREAD_CDECL tracerThread( void* parm )
{
    Thread *my_handle = (Thread *)parm;
    TTTParm * my_parm = (TTTParm *)my_handle->get_parm();

    Threads::yield();
    for (Uint32 x=0; x < my_parm->number; x++)
    {
        if (x % 911 == 0 )
        {
            // Give other threads time to run.
            Threads::yield();
        }
        if (my_parm->isVariableMsg)
        {
            traceVariableArgs(my_parm->trcHandler,
                              my_parm->trcMessage,
                              my_parm->msgLen,
                              my_parm->trcMessage,
                              VAR_TEST_MESSAGE);
        }
        else
        {
            my_parm->trcHandler->handleMessage(my_parm->trcMessage,
                                               my_parm->msgLen);
        }
    }

    return ThreadReturnType(0);
}

Uint32 testMemoryHandler(const char* filename)
{
    #define NUM_TEST_THREADS 99
    Uint32 rc = 0;

    TraceMemoryHandler *trcHdler = new TraceMemoryHandler();

    const char* trcMsg1 = "1START A short trace message. END1";
    const char* trcMsg2 = "2START A trace message which is a little longer "
                          "than the previous trace message. END2";
    const char* trcMsg3 = "3START A trace message which is much longer than "
                          "the previous trace messages, which were small and a "
                          "little larger, but this one is at least double the "
                          "size as both others together. END3";
    const char* trcMsg4 = "4START <%s> END4";
    const char* trcMsgs[] = { trcMsg1, trcMsg2, trcMsg3, trcMsg4 };
    Uint32 numMsgs = sizeof(trcMsgs) / sizeof(const char*);

    TTTParm *tttParms[NUM_TEST_THREADS];

    for (int x=0; x < NUM_TEST_THREADS; x++)
    {
        Uint32 msgNumber = x%numMsgs;
        tttParms[x] = new TTTParm();
        tttParms[x]->trd = 0;
        tttParms[x]->trcHandler = trcHdler;
        tttParms[x]->trcMessage = trcMsgs[msgNumber];
        tttParms[x]->msgLen = strlen(trcMsgs[msgNumber]);
        tttParms[x]->number = 100000;
        // Is the message a variable one (= the last in the list)?
        tttParms[x]->isVariableMsg = (msgNumber == (numMsgs-1));
    }

    for (int x=0; x < NUM_TEST_THREADS; x++)
    {
        tttParms[x]->trd = new Thread(tracerThread, tttParms[x], false);
        tttParms[x]->trd->run();
    }

    for (int x=0; x < NUM_TEST_THREADS; x++)
    {
        tttParms[x]->trd->join();
    }

    // Setting an empty trace file name should fail.
    PEGASUS_TEST_ASSERT(Tracer::setTraceFile(""));

    if (Tracer::setTraceFile(filename))
    {
        cout << "Failure in call to setTraceFile for file \""
                << filename << "\"\n" << endl;
        PEGASUS_TEST_ASSERT(0);
    }

    trcHdler->flushTrace();

    // To test the variable messages, we replace the variable message
    // in the list with a resolved copy.
    Uint32 lastMsg = numMsgs-1;
    char resolvedMsg[1024];
    memcpy( resolvedMsg, trcMsgs[lastMsg], strlen(trcMsgs[lastMsg]) );
    sprintf( resolvedMsg+strlen(trcMsgs[lastMsg]),
             trcMsgs[lastMsg],
             VAR_TEST_MESSAGE );
    trcMsgs[lastMsg] = resolvedMsg;


    // Now analyze the dumped buffer content, to ensure no messages
    // were damaged.
    // For this we read the buffer content line by line and check if it
    // matches one of the messages from the list above.
    {
        fstream file;
        file.open(filename, fstream::in);
        if (!file.good())
        {
            cout << "Failed to open file \"" << filename << "\"\n" << endl;
            PEGASUS_TEST_ASSERT(0);
        }

        // Keep the first line on the side, since this is probably the
        // wrapped remainder of the very last messsage in the buffer.
        char firstLine[256];
        file.getline( firstLine, 256 );

        char currentLine[256];
        file.getline( currentLine, 256 );

        while( !file.eof() )
        {
            Boolean found = false;
            for (Uint32 x=0; x < numMsgs; x++)
            {
                if ( strcmp(trcMsgs[x], currentLine) == 0 )
                {
                    found = true;
                    continue;
                }
            }
            if ( !found )
            {
                if ( strncmp(currentLine, "*EOTRACE*", strlen("*EOTRACE*")) )
                {
                    // if we got here, this is either an error, or we reached
                    // the end of the trace buffer, where it had wrapped.
                    // To check this we paste together the message in the
                    // first line we read, and which is supposed to be the
                    // remainder of the wrapped message.
                    strcat(currentLine, firstLine);
                    for (Uint32 x=0; x < numMsgs; x++)
                    {
                        if ( strcmp(trcMsgs[x], currentLine) == 0 )
                        {
                            found = true;
                            continue;
                        }
                    }
                    if ( !found )
                    {
                        // Diagnostics about the error
                        cout << "Compare Error: unexpected message= \n\""
                             << currentLine << "\"\n" << endl;
                        PEGASUS_TEST_ASSERT(0);
                    }
                }
            }
            file.getline( currentLine, 256 );
        }
        file.close();
    }


    for (int x=0; x < NUM_TEST_THREADS; x++)
    {
        delete( tttParms[x]->trd );
        delete( tttParms[x] );
    }

    // test the wrapping of a too long message:
    {
        #define FIX_PART_OF_MESSAGE "LongMsg:"

        // The trace buffer is not at that size used for tracing
        // then it is defined with PEGASUS_TRC_DEFAULT_BUFFER_SIZE_KB*1024
        // The resulting message is shriked by:
        // * the controll structure traceArea_t ( privat of TraceMemoryHandler)
        // * the fixed message part
        // * the markers,  '\n' and '\0'
        Uint32 bufferDelta = 2* sizeof(Uint32)+ sizeof(char*)
                           + PEGASUS_TRC_BUFFER_EYE_CATCHER_LEN
                           + strlen(FIX_PART_OF_MESSAGE)
                           + strlen(PEGASUS_TRC_BUFFER_TRUNC_MARKER) + 1
                           + strlen(PEGASUS_TRC_BUFFER_EOT_MARKER)+ 1
                           // this adds the '2' before the truncation marker
                           // should appear in the message
                           + 1;

        Uint32 sizeOfVeryLongMSG=PEGASUS_TRC_DEFAULT_BUFFER_SIZE_KB*1024;

        // Construct the big message:
        //  LongMsg:111...11122222222222222
        char* veryLongMSG = (char *)malloc(sizeOfVeryLongMSG);
        memset((void*)veryLongMSG,'1',sizeOfVeryLongMSG-bufferDelta);
        memset((void*)&(veryLongMSG[sizeOfVeryLongMSG-bufferDelta]),
               '2',bufferDelta);
        veryLongMSG[sizeOfVeryLongMSG-1] = 0 ;

        traceVariableArgs(trcHdler,
                          FIX_PART_OF_MESSAGE,
                          strlen(FIX_PART_OF_MESSAGE),
                         "%s",
                          veryLongMSG);

        Tracer::setTraceFile(filename);
        trcHdler->flushTrace();

        // resuse the buffer for reading the result file.
        memset((void*)veryLongMSG,0,sizeOfVeryLongMSG);

        fstream file;
        file.open(filename, fstream::in);
        if (!file.good())
        {
            cout << "Failed to open file \"" << filename << "\"\n" << endl;
            PEGASUS_TEST_ASSERT(0);
        }

        // The first line must be the trucated big message.
        file.getline( veryLongMSG, sizeOfVeryLongMSG );

        // The second line must be the end of trace marker
        char lastLine[256];
        file.getline( lastLine, 256 );

        // The result must be in the two lines. Close the file
        file.close();

        // the last line must be only the end of trace marker.
        if ( strncmp(lastLine, "*EOTRACE*", strlen("*EOTRACE*")) )
        {
                cout << "Compare Error: unexpected message= \n\""
                     << lastLine << "\"\n" << endl;
                PEGASUS_TEST_ASSERT(0);
        }

        char* cursor = veryLongMSG;

        // The big messsage has the format:
        //  LongMsg:111...1112*TRUNC*
        // To validate that the message is truncated at the right position,
        // only one '2' has to show up before the truncation marker.

        if ( strncmp(cursor, FIX_PART_OF_MESSAGE
                     , strlen(FIX_PART_OF_MESSAGE)) )
        {
                cout << "Compare Error: unexpected message= \n\""
                     << cursor << "\"\n" << endl;
                PEGASUS_TEST_ASSERT(0);
        }

        cursor = cursor + strlen(FIX_PART_OF_MESSAGE);
        int noErrors = 0;
        for (int i = 0 ; i < (int)(sizeOfVeryLongMSG-bufferDelta); i++)
        {
            if (cursor[i] != '1')
            {
                noErrors++;
                cout << "Compare Error: unexpected char '"
                     <<  cursor[i] << "' at position "
                     << strlen(FIX_PART_OF_MESSAGE) + i
                     << " expecting '1'." << endl;
                // limmit the number of printed errors to 20
                if (noErrors > 20 )
                {
                    cout << "Too manny missmatches. Abort long message test"
                         << endl;
                    PEGASUS_TEST_ASSERT(0);
                }

            }
        }

        // If any error occures in the above test, exit the test.
        if (noErrors != 0)
        {
          PEGASUS_TEST_ASSERT(0);
        }

        if (cursor[(sizeOfVeryLongMSG-bufferDelta)] != '2')
        {
            cout << "Compare Error: unexpected char '"
                 << cursor[(sizeOfVeryLongMSG-bufferDelta)] << "' at position "
                 << strlen(FIX_PART_OF_MESSAGE) +
                       (sizeOfVeryLongMSG-bufferDelta)
                 << " expecting '2'." << endl;
        }

        cursor = cursor + (sizeOfVeryLongMSG-bufferDelta) + 1;
        if ( strcmp(cursor, "*TRUNC*") )
        {
                cout << "Compare Error: unexpected message= \n\""
                     << cursor << "\"\n" << endl;
                PEGASUS_TEST_ASSERT(0);
        }

        free(veryLongMSG);
    }

    delete( trcHdler );
    return rc;
}



int main(int argc, char** argv)
{

// Execute the tests only if trace calls are included

#ifdef PEGASUS_REMOVE_TRACE
    cout << argv[0] << " +++++ passed all tests" << endl;
    return 0;
#else

    const char* tmpDir = getenv ("PEGASUS_TMP");
    if (tmpDir == NULL)
    {
        tmpDir = ".";
    }
    String f1 (tmpDir);
    f1.append("/testtracer1.trace");
    FILE1 = f1.getCString();
    String f2 (tmpDir);
    f2.append("/testtracer2.trace");
    FILE2 = f2.getCString();
    String f3 (tmpDir);
    f3.append("/testtracer3.trace");
    FILE3 = f3.getCString();
    String f4 (tmpDir);
    f4.append("/testtracer4.trace");
    FILE4 = f4.getCString();
    String f5 (tmpDir);
    f5.append("/testtracer5.trace");
    FILE5 = f5.getCString();

    System::removeFile(FILE1);
    System::removeFile(FILE2);
    System::removeFile(FILE3);
    System::removeFile(FILE4);
    System::removeFile(FILE5);
    if (test1() != 0)
    {
       cout << "Tracer test (test1) failed" << endl;
       exit(1);
    }
    if (test2() != 0)
    {
       cout << "Tracer test (test2) failed" << endl;
       exit(1);
    }
    if (test3() != 0)
    {
       cout << "Tracer test (test3) failed" << endl;
       exit(1);
    }
    if (test4() != 0)
    {
       cout << "Tracer test (test4) failed" << endl;
       exit(1);
    }
    if (test5() != 0)
    {
       cout << "Tracer test (test5) failed" << endl;
       exit(1);
    }
    if (test6() != 0)
    {
       cout << "Tracer test (test6) failed" << endl;
       exit(1);
    }
    if (test7() != 0)
    {
       cout << "Tracer test (test7) failed" << endl;
       exit(1);
    }
    if (test9() != 0)
    {
       cout << "Tracer test (test9) failed" << endl;
       exit(1);
    }
    /***************************
       Test 10 bypassed when tests changed to
       use macros.  It did an invalid call which is
       not possible with macros

    if (test10() != 0)
    {
       cout << "Tracer test (test10) failed" << endl;
       exit(1);
    }
    ******************************/
    if (test11() != 0)
    {
       cout << "Tracer test (test11) failed" << endl;
       exit(1);
    }
    if (test12() != 0)
    {
       cout << "Tracer test (test12) failed" << endl;
       exit(1);
    }
    if (test13() != 0)
    {
       cout << "Tracer test (test13) failed" << endl;
       exit(1);
    }
    if (test14() != 0)
    {
       cout << "Tracer test (test14) failed" << endl;
       exit(1);
    }
    if (test15() != 0)
    {
       cout << "Tracer test (test15) failed" << endl;
       exit(1);
    }
# ifndef PEGASUS_REMOVE_METHODTRACE
    if (test15a() != 0)
    {
       cout << "Tracer test (test15a) failed" << endl;
       exit(1);
    }
    if (test15b() != 0)
    {
       cout << "Tracer test (test15b) failed" << endl;
       exit(1);
    }
# endif
    if (test16() != 0)
    {
       cout << "Tracer test (test16) failed" << endl;
       exit(1);
    }
    if (test17() != 0)
    {
       cout << "Tracer test (test17) failed" << endl;
       exit(1);
    }
    if (test18() != 0)
    {
       cout << "Tracer test (test18) failed" << endl;
       exit(1);
    }
    if (test20() != 0)
    {
       cout << "Tracer test (test20) failed" << endl;
       exit(1);
    }
    if (test21() != 0)
    {
       cout << "Tracer test (test21) failed" << endl;
       exit(1);
    }
    if (test22() != 0)
    {
       cout << "Tracer test (test22) failed" << endl;
       exit(1);
    }
    if (test23() != 0)
    {
       cout << "Tracer test (test23) failed" << endl;
       exit(1);
    }
    if (test24() != 0)
    {
       cout << "Tracer test (test24) failed" << endl;
       exit(1);
    }

    if (test25() != 0)
    {
       cout << "Tracer test (test25) failed" << endl;
       exit(1);
    }
    if (test26() != 0)
    {
       cout << "Tracer test (test26) failed" << endl;
       exit(1);
    }

    if (test27() != 0)
    {
       cout << "Tracer test (test27) failed" << endl;
       exit(1);
    }

    if (testMemoryHandler(FILE5) != 0)
    {
       cout << "Tracer test (testMemoryHandler) failed" << endl;
       exit(1);
    }


    cout << argv[0] << " +++++ passed all tests" << endl;
    System::removeFile(FILE1);
    System::removeFile(FILE2);
    System::removeFile(FILE3);
    System::removeFile(FILE4);
    System::removeFile(FILE5);
    return 0;
#endif
}
