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

#ifndef Pegasus_WQLParser_h
#define Pegasus_WQLParser_h

#include <Pegasus/Common/Config.h>
#include <Pegasus/Common/ArrayInternal.h>
#include <Pegasus/WQL/Linkage.h>
#include <Pegasus/WQL/WQLSelectStatement.h>
#include <Pegasus/Common/Buffer.h>

PEGASUS_NAMESPACE_BEGIN

/** This class is the main interface to the WQL parser used for parsing WQL1
    compliant SQL statements.

    Here's an example which parses a SELECT statement:

    <pre>
    const char TEXT[] = "SELECT X,Y FROM MyClass WHERE X > 10 AND Y < 3";

    // Note that this array must be null-terminated (sizeof(TEXT) includes
    // the null-terminator in the count).

    Buffer text(TEXT, sizeof(TEXT));

    WQLSelectStatement selectStatement;

    WQLParser parser;

    try
    {
        parser.parse(text, selectStatement);
    }
    catch (ParseError&)
    {
        ...
    }
    catch (MissingNullTerminator&)
    {
        ...
    }
    </pre>

    Note that the text must be NULL terminated or else the MissingNullTerminator
    exception is thrown.

    The text is read and the result is left in the selectStatement output
    argument.

    At this point you might wish to peek at the contents of the selectStatement.
    This may be done by calling WQLSelectStatement::print() like this:

    <pre>
    WQLSelectStatement selectStatement;
    ...
    selectStatement.print();
    </pre>

    For the above query text, the following is printed:

    <pre>
    WQLSelectStatement
    {
        _className: "MyClass"

        _propertyNames[0]: "X"
        _propertyNames[1]: "Y"

        _operations[0]: "WQL_GT"
        _operations[1]: "WQL_LT"
        _operations[2]: "WQL_AND"

        _operands[0]: "PROPERTY_NAME: X"
        _operands[1]: "INTEGER_VALUE: 10"
        _operands[2]: "PROPERTY_NAME: Y"
        _operands[3]: "INTEGER_VALUE: 3"
    }
    </pre>

    The WQLSelectStatement::evaluateWhereClause() method may be called to
    determine whether a particular instance (whose properties are made
    available to he evaluateWhereClause() by a user implementation of the
    WQLPropertySource class). This method returns true, if the where clause
    matches this instance. Here is an example:

    <pre>
    WQLSelectStatement selectStatement;
    ...
    WQLPropertySource* propertySource = new MyPropertySource(...);

    if (selectStatement.evaluateWhereClause(propertySource))
    {
        // It's a match!
    }
    </pre>

    The evaluateWhereClause() method calls propertySource->getValue() to
    obtain values for each of the properties referred to in where clause (X
    and Y in the above query example).

    The implementer of the WQLPropertySource interface must provide the
    implementation of getValue() to produce values for the target data
    types. The WQL library makes no assumptions about the nature of the
    target data representation. This was done so that this libary could be
    adapted to multiple data representations.

    For use with Pegasus CIMInstance objects, a CIMInstancePropertySource
    class could be developed whose getValue() method fetches values from
    a CIMInstance. Here is an example of how it might be used.

    <pre>
    CIMInstancePropertySource* propertySource
        = new CIMInstancePropertySource(...);

    CIMInstance instance;

    while (instance = GetNextInstance(...))
    {
        propertySource->setInstance(currentInstance);

        if (selectStatement.evaluateWhereClause(propertySource))
        {
        // It's a match!
        }
    }
    </pre>

    Of course the numeration of instances is left to the user of WQL.
*/
class PEGASUS_WQL_LINKAGE WQLParser
{
public:

    /** Parse the SELECT statement given by the text parameter and initialize
    the statement parameter accordingly.

    Please note that this method is not thread safe. It must be guarded
    with mutexes by the caller.

    @param text null-terminated C-string which points to SQL statement.
    @param statement object which holds the compiled version of the SELECT
        statement upon return.
    @exception ParseError if text is not a valid SELECT statement.
    @exception MissingNullTerminator if text argument is not
        terminated with a null.
    */
    static void parse(
    const char* text,
    WQLSelectStatement& statement);

    /** Version of parse() taking an array of characters.
    */
    static void parse(
    const Buffer& text,
    WQLSelectStatement& statement);

    /** Version of parse() taking a string.
    */
    static void parse(
    const String& text,
    WQLSelectStatement& statement);

private:

    /** This method cleans up all the strings which were created by LEX and
        passed to YACC. These strings cannot be cleaned up by YACC actions
        since the actions that clean up certain strings are not always reached
        when errors occur.
    */
    static void cleanup();

    /** Private constructor to avoid user from creating instance of this class.
    */
    WQLParser() { }
};

PEGASUS_NAMESPACE_END

#endif /* Pegasus_WQLParser_h */
