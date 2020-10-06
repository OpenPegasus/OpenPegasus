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
#include <Pegasus/Common/InternalException.h>
#include <Pegasus/Common/Tracer.h>
#include <Pegasus/Common/PegasusVersion.h>
#include <Pegasus/Common/Mutex.h>
#include <iostream>
#include "WQLParser.h"
#include "WQLParserState.h"

PEGASUS_USING_STD;

extern int WQL_parse();
extern void WQL_restart (FILE *input_file);

PEGASUS_NAMESPACE_BEGIN

WQLParserState* globalParserState = 0;
static Mutex WQL_mutex;

void WQLParser::parse(
    const char* text,
    WQLSelectStatement& statement)
{
    PEG_METHOD_ENTER(TRC_WQL,"WQLParser::parse");

     AutoMutex mtx(WQL_mutex);

    if (!text)
    {
        PEG_METHOD_EXIT();
        throw NullPointer();
    }

    statement.clear();

    globalParserState = new WQLParserState;
    globalParserState->error = false;
    globalParserState->text = text;
    globalParserState->textSize = (Uint32)(strlen(text) + 1);
    globalParserState->offset = 0;
    globalParserState->statement = &statement;

    WQL_parse();

    if (globalParserState->error)
    {
        String errorMessage = globalParserState->errorMessage;
        cleanup();
        delete globalParserState;
        PEG_METHOD_EXIT();
        throw ParseError(errorMessage);
    }

    cleanup();
    delete globalParserState;
    PEG_METHOD_EXIT();
}

void WQLParser::parse(
    const Buffer& text,
    WQLSelectStatement& statement)
{
    PEG_METHOD_ENTER(TRC_WQL,"WQLParser::parse");

    parse(text.getData(), statement);

    PEG_METHOD_EXIT();
}

void WQLParser::parse(
    const String& text,
    WQLSelectStatement& statement)
{
    PEG_METHOD_ENTER(TRC_WQL,"WQLParser::parse");

    parse(text.getCString(), statement);

    PEG_METHOD_EXIT();
}

void WQLParser::cleanup()
{
    PEG_METHOD_ENTER(TRC_WQL,"WQLParser::cleanup");

    Array<char*>& arr = globalParserState->outstandingStrings;

    for (Uint32 i = 0, n = arr.size(); i < n; i++)
        delete [] arr[i];

    arr.clear();

    PEG_METHOD_EXIT();
}

PEGASUS_NAMESPACE_END

PEGASUS_USING_PEGASUS;

int WQL_error(const char* errorMessage)
{
    PEG_METHOD_ENTER(TRC_WQL,"WQL_error");

    globalParserState->error = true;
    globalParserState->errorMessage = errorMessage;

    //
    //  flex does not automatically flush the input buffer in case of error
    //
    WQL_restart (0);

    PEG_METHOD_EXIT();
    return -1;
}

int WQLInput(char* buffer, int& numRead, int numRequested)
{
    PEG_METHOD_ENTER(TRC_WQL,"WQLInput");
    //
    // Be sure to account for the null terminator (the size of the text will
    // be one or more; this is fixed checked beforehand by WQLParser::parse()).
    //

    int remaining =
        globalParserState->textSize - globalParserState->offset - 1;

    if (remaining == 0)
    {
        numRead = 0;
        PEG_METHOD_EXIT();
        return 0;
    }

    if (remaining < numRequested)
        numRequested = remaining;

    memcpy(buffer,
    globalParserState->text + globalParserState->offset,
    numRequested);

    globalParserState->offset += numRequested;
    numRead = numRequested;

    PEG_METHOD_EXIT();
    return numRead;
}
