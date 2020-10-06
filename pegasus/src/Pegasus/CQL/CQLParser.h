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

#ifndef Pegasus_CQLParser_h
#define Pegasus_CQLParser_h

#include <Pegasus/CQL/Linkage.h>
#include <Pegasus/Common/ArrayInternal.h>
#include <Pegasus/CQL/CQLSelectStatement.h>
#include <Pegasus/Common/Buffer.h>

#ifdef PEGASUS_USE_EXPERIMENTAL_INTERFACES

PEGASUS_NAMESPACE_BEGIN

/** This class is the main interface to the CQL parser used for parsing CQL
    compliant statements.

    Here's an example which parses a SELECT statement:

    <pre>
        const char TEXT[] = "SELECT X,Y FROM MyClass WHERE X > 10 AND Y < 3";

        // Note that this array must be null-terminated (sizeof(TEXT) includes
        // the null-terminator in the count).

        Buffer text(TEXT, sizeof(TEXT));

        CQLSelectStatement selectStatement;

        try
        {
            CQLParser::parse(text, selectStatement);
        }
        catch (QueryException&)
        {
            ...
        }
    </pre>

    The text is read and the result is left in the selectStatement output
    argument.

*/
class PEGASUS_CQL_LINKAGE CQLParser
{
public:

    /** Parse the SELECT statement given by the text parameter and initialize
        the statement parameter accordingly.

        Please note that this method is not thread safe. It must be guarded
        with mutexes by the caller.

        @param text null-terminated C-string which points to CQL statement.
        @param statement object which holds the compiled version of the SELECT
            statement upon return.
        @exception CQLSyntaxErrorException if text is not a valid SELECT
            statement.
        @exception MissingNullTerminator if text argument is not
            terminated with a null.
    */
    static void parse(
        const char* text,
        CQLSelectStatement& statement);

    /** Version of parse() taking an array of characters.
    */
    static void parse(
        const Buffer& text,
        CQLSelectStatement& statement);

    /** Version of parse() taking a string.
    */
    static void parse(
        const String& text,
        CQLSelectStatement& statement);

private:

    /** This method cleans up all the strings which were created by LEX and
        passed to YACC. These strings cannot be cleaned up by YACC actions
        since the actions that clean up certain strings are not always reached
        when errors occur.
    */
    static void cleanup();

    /** Private constructor to avoid user from creating instance of this class.
    */
    CQLParser() { }
};

PEGASUS_NAMESPACE_END
#endif
#endif /* Pegasus_CQLParser_h */
