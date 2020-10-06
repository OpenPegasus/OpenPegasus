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
#include <Pegasus/Common/PegasusAssert.h>
#include <Pegasus/Common/Logger.h>
//#include <Pegasus/Common/System.h>
#include <Pegasus/Common/Tracer.h>


PEGASUS_USING_PEGASUS;
PEGASUS_USING_STD;


// Trace file for test purpose
// Will be created in the $(PEGASUS_TMP) directory, or if not set,
// in the current directory
CString FILE1;

//
// Reads the last trace message from a given trace file and compares the
// given string with the string read from file
// theft from Tracer.cpp test case
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

    /* Diagnostic to determine string differences
    if (retCode)
        cout << "Compare Error: expectedMessage= \n\"" << expectedMessage <<
            "\". actualMessage= \n\"" << actualMessage.get() << "\"" << endl;
    */

    return retCode;
}


void testLogToTraceDuplication()
{
#ifndef PEGASUS_REMOVE_TRACE
    // Setting log level to trace to check that there is no recursion
    // between Logger and Tracer happening
    Logger::setlogLevelMask("TRACE");
    // first test with log message component not set for trace
    // negative test
    Tracer::setTraceFile(FILE1);
    Tracer::setTraceComponents("");
    Tracer::setTraceLevel(Tracer::LEVEL1);
    Logger::put(
        Logger::STANDARD_LOG,
        "LoggerTest",
        Logger::WARNING,
        "Something not to be written to the trace.");
    Uint32 fileSize=0;
    System::getFileSize(FILE1,fileSize);
    PEGASUS_TEST_ASSERT(0 == fileSize);

    // activate logmessage component in tracer
    Tracer::setTraceComponents("logmessages");

    // Logger::TRACER is reserved for internal usage and should not trace
    // negative test
    Logger::put(
        Logger::TRACE_LOG,
        "LoggerTest",
        Logger::TRACE,
        "Something not to be written to the trace.");
    fileSize=0;
    System::getFileSize(FILE1,fileSize);
    PEGASUS_TEST_ASSERT(0 == fileSize);

    // test with log message component set for trace
    // play the game with all three log levels
    // no arguments given to the Logger
    // Positive test
    Logger::put(
        Logger::STANDARD_LOG,
        "LoggerTest",
        Logger::INFORMATION,
        "Now, this information line should appear in the trace.");
    PEGASUS_TEST_ASSERT(
        !compare(
            FILE1,
            "Now, this information line should appear in the trace."));
    Logger::put(
        Logger::ERROR_LOG,
        "LoggerTest",
        Logger::WARNING,
        "Now, this warning line should appear in the trace.");
    PEGASUS_TEST_ASSERT(
        !compare(
            FILE1,
            "Now, this warning line should appear in the trace."));
    Logger::put(
        Logger::ERROR_LOG,
        "LoggerTest",
        Logger::SEVERE,
        "Now, this severe error line should appear in the trace.");
    PEGASUS_TEST_ASSERT(
        !compare(
            FILE1,
            "Now, this severe error line should appear in the trace."));
    Logger::put(
        Logger::ERROR_LOG,
        "LoggerTest",
        Logger::FATAL,
        "Now, this fatal error line should appear in the trace.");
    PEGASUS_TEST_ASSERT(
        !compare(
            FILE1,
            "Now, this fatal error line should appear in the trace."));

    // test with log message component set for trace
    // use different arguments with the Logger
    // Positive test
    Logger::put(
    Logger::STANDARD_LOG,
    "LoggerTest",
    Logger::WARNING,
    "X=$0, Y=$1",
    8138,
    "Hello World");
    PEGASUS_TEST_ASSERT(!compare(FILE1,"X=8138, Y=Hello World"));
#endif
}


// ATTN-B: Complete this test by reopening the log and making sure it
// contains what we expect.

int main(int, char** argv)
{

#ifndef PEGASUS_OS_HPUX
    Logger::setHomeDirectory("./logs");
#endif

    Logger::put(
    Logger::STANDARD_LOG,
    "LoggerTest",
    Logger::WARNING,
    "X=$0, Y=$1, Z=$2",
    88,
    "Hello World",
    7.5);

    // Preparation for Log to trace duplication testing
    const char* tmpDir = getenv ("PEGASUS_TMP");
    if (tmpDir == NULL)
    {
        tmpDir = ".";
    }
    String f1 (tmpDir);
    f1.append("/log.trace");
    FILE1 = f1.getCString();

    // To ensure we start clean, just trying to remove the file
    // in case it already exists
    System::removeFile(FILE1);

    //
    testLogToTraceDuplication();

    cout << argv[0] << " +++++ passed all tests" << endl;

    System::removeFile(FILE1);

    return 0;
}
