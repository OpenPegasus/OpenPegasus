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

#ifndef Pegasus_FQLParser_h
#define Pegasus_FQLParser_h

#include <Pegasus/Common/Config.h>
#include <Pegasus/Common/ArrayInternal.h>
#include <Pegasus/FQL/Linkage.h>
#include <Pegasus/FQL/FQLQueryStatement.h>
#include <Pegasus/Common/Buffer.h>

PEGASUS_NAMESPACE_BEGIN

class PEGASUS_FQL_LINKAGE FQLParser
{
public:

    /** Parse the FQL Query statement given by the text parameter
    and initialize the statement parameter accordingly.

    This method is not thread safe. It must be guarded
    with mutexes by the caller.

    @param text null-terminated C-string which points to FQL
                statement.
    @param statement FQLQueryStatement object which holds the
        compiled version of the SELECT statement upon return.
    @exception ParseError if text is not a valid SELECT statement.
    @exception MissingNullTerminator if text argument is not
        terminated with a null.
    */
    static void parse(
        const char* text,
        FQLQueryStatement& statement);

    /** Version of parse() taking an array of characters.
    */
    static void parse(
        const Buffer& text,
        FQLQueryStatement& statement);

    /** Version of parse() taking a string.
    */
    static void parse(
        const String& text,
        FQLQueryStatement& statement);

private:

    /** This method cleans up all the strings which were created by LEX and
        passed to YACC. These strings cannot be cleaned up by YACC actions
        since the actions that clean up certain strings are not always reached
        when errors occur.
    */
    static void cleanup();

    /** Private constructor to avoid user from creating instance of this class.
    */
    FQLParser() { }
};

PEGASUS_NAMESPACE_END

#endif /* Pegasus_FQLParser_h */
