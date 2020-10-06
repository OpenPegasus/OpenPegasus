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
///////////////////////////////////////////////////////////////////////////////

#include <Pegasus/Server/CIMServerState.h>

PEGASUS_NAMESPACE_BEGIN

static const char* STATE_STRINGS[] =
{
    "running",
    "terminating"
};

/**
    Constructor
*/
CIMServerState::CIMServerState()
{
    _currentState = RUNNING;
}

/**
    Destructor
*/
CIMServerState::~CIMServerState()
{
}

/**
    Set the state of the CIM Server.
*/
void CIMServerState::setState(Uint32 code)
{
    //
    // only set server state if the state code is valid
    //
    if (code >= RUNNING && code <= TERMINATING)
    {
        _currentState = code;
    }
}

/**
    Return the state of the CIM Server.
*/
Uint32 CIMServerState::getState()
{
    return _currentState;
}

/**
    Return a string representation of the state of the CIM Server.
*/
const char* CIMServerState::toString(Uint32 code)
{
    if (code >= RUNNING && code <= TERMINATING)
    {
        return STATE_STRINGS[code-1];
    }
    else
    {
        return "unknown";
    }
}

PEGASUS_NAMESPACE_END
