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
// Author: Humberto Rivero (hurivero@us.ibm.com)
//
// Modified By: David Dillard, VERITAS Software Corp.
//                  (david.dillard@veritas.com)
//              Josephine Eskaline Joyce (jojustin@in.ibm.com) for PEP#101
//
//%/////////////////////////////////////////////////////////////////////////////

#include "CQLParser.h"
#include "CQLParserState.h"
#include <Pegasus/Common/InternalException.h>
#include <Pegasus/Common/Mutex.h>
#include <Pegasus/Query/QueryCommon/QueryException.h>
#include <Pegasus/Common/Tracer.h>
#include <Pegasus/Common/PegasusVersion.h>

PEGASUS_USING_STD;

extern int CQL_parse();
extern void CQL_restart (FILE *input_file);
extern void CQL_Bison_Cleanup();
extern void CQL_Arglist_Cleanup();

PEGASUS_NAMESPACE_BEGIN

CQLParserState* CQL_globalParserState = 0;
static Mutex CQL_mutex;

void CQLParser::parse(
    const char* text,
    CQLSelectStatement& statement)
{
    PEG_METHOD_ENTER(TRC_CQL,"CQLParser::parse");

    AutoMutex mtx(CQL_mutex);

    if (!text)
    {
        PEG_METHOD_EXIT();
        throw NullPointer();
    }

    statement.clear();
    CQL_restart (0);

    CQL_globalParserState = new CQLParserState;
    CQL_globalParserState->error = false;
    CQL_globalParserState->text = text;
    CQL_globalParserState->textSize = (Uint32)(strlen(text) + 1);
    CQL_globalParserState->offset = 0;
    CQL_globalParserState->currentTokenPos = 0;
    CQL_globalParserState->tokenCount = 0;
    CQL_globalParserState->currentRule = String::EMPTY;
    CQL_globalParserState->statement = &statement;

    try
    {
        CQL_parse();
    }
    catch(...)
    {
        CQL_Bison_Cleanup();
        delete CQL_globalParserState;
        PEG_METHOD_EXIT();
        throw;
    }

    if (CQL_globalParserState->error)
    {
        String errorMessage = CQL_globalParserState->errorMessage;
        cleanup();
        Uint32 position = CQL_globalParserState->currentTokenPos;
        Uint32 token = CQL_globalParserState->tokenCount;
        String rule = CQL_globalParserState->currentRule;
        delete CQL_globalParserState;
        PEG_METHOD_EXIT();
        throw CQLSyntaxErrorException(errorMessage,token,position,rule);
    }

    cleanup();
    delete CQL_globalParserState;
    PEG_METHOD_EXIT();
}

void CQLParser::parse(
    const Buffer& text,
    CQLSelectStatement& statement)
{
    PEG_METHOD_ENTER(TRC_CQL,"CQLParser::parse");

    if (text.size() == 0 || text[text.size() - 1] != '\0')
    {
        PEG_METHOD_EXIT();
        throw MissingNullTerminator();
    }

    parse(text.getData(), statement);
    PEG_METHOD_EXIT();
}

void CQLParser::parse(
    const String& text,
    CQLSelectStatement& statement)
{
    PEG_METHOD_ENTER(TRC_CQL,"CQLParser::parse");

    parse(text.getCString(), statement);

    PEG_METHOD_EXIT();
}

void CQLParser::cleanup()
{
    PEG_METHOD_ENTER(TRC_CQL,"CQLParser::cleanup");


    Array<char*>& arr = CQL_globalParserState->outstandingStrings;

    for (Uint32 i = 0, n = arr.size(); i < n; i++)
        delete [] arr[i];

    arr.clear();

    PEG_METHOD_EXIT();
}

PEGASUS_NAMESPACE_END

PEGASUS_USING_PEGASUS;

int CQL_error(const char* errorMessage)
{
    PEG_METHOD_ENTER(TRC_CQL,"CQL_error");
    CQL_globalParserState->error = true;
    CQL_globalParserState->errorMessage = errorMessage;

    //
    //  flex does not automatically flush the input buffer in case of error
    //
    CQL_restart (0);

    // flush ArgList
    CQL_Arglist_Cleanup();

    PEG_METHOD_EXIT();
    return -1;
}

int CQLInput(char* buffer, int& numRead, int numRequested)
{
    PEG_METHOD_ENTER(TRC_CQL,"CQLInput");
    //
    // Be sure to account for the null terminator (the size of the text will
    // be one or more; this is fixed checked beforehand by CQLParser::parse()).
    //
    int remaining =
        CQL_globalParserState->textSize - CQL_globalParserState->offset - 1;

    if (remaining == 0)
    {
        numRead = 0;
        PEG_METHOD_EXIT();
        return 0;
    }

    if (remaining < numRequested)
        numRequested = remaining;

    memcpy(buffer,
        CQL_globalParserState->text + CQL_globalParserState->offset,
        numRequested);

    CQL_globalParserState->offset += numRequested;
    numRead = numRequested;


    PEG_METHOD_EXIT();
    return numRead;
}
