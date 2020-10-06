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
//
//%/////////////////////////////////////////////////////////////////////////////

#ifndef Pegasus_FQLQueryStatement_h
#define Pegasus_FQLQueryStatement_h

#include <Pegasus/Common/Config.h>
#include <Pegasus/FQL/Linkage.h>
#include <Pegasus/Common/ArrayInternal.h>
#include <Pegasus/Common/CIMName.h>
#include <Pegasus/Common/CIMPropertyList.h>
#include <Pegasus/Common/CIMInstance.h>
#include <Pegasus/Common/CIMObject.h>
#include <Pegasus/Common/CIMClass.h>
#include <Pegasus/FQL/FQLOperation.h>
#include <Pegasus/FQL/FQLOperand.h>
#include <Pegasus/FQL/FQLPropertySource.h>
////#include <Pegasus/FQL/FQLQueryStatement.h>
//// #include <Pegasus/Query/QueryCommon/SelectStatement.h>
//// #include <Pegasus/Query/QueryCommon/QueryContext.h>

PEGASUS_NAMESPACE_BEGIN


class FQLQueryStatementRep;

/** This class represents a compiled FQL query statement.

    An instance of FQLQueryStatement is passed to FQLParser::parse() which
    parses the FQL query statement and initializes the
    FQLQueryStatement instance. An FQL query statement has the
    following form:

    <pre>
        TODO Fix this up
        SELECT &lt;property&gt;...
        FROM &lt;class name&gt;
        WHERE &lt;where clause&gt;
    </pre>

    The components of the statement are stored in two arrays:
    one for operands and one for operators (these are placed in
    proper order by the YACC parser). Evaluation is performed
    using a Boolean stack. See the implementation of
    evaluate() for details.
*/
class PEGASUS_FQL_LINKAGE FQLQueryStatement
{
public:

    FQLQueryStatement(
        const CIMClass& queryClass,
        const String& query);

    FQLQueryStatement(
        const String& query);

    /** Default constructor.
    */
    FQLQueryStatement();

    FQLQueryStatement(const FQLQueryStatement& statement);

    /** Destructor.
    */
    ~FQLQueryStatement();

    FQLQueryStatement& operator=(const FQLQueryStatement& rhs);

    /** Clears all data members of this object.
    */
    void clear();

////  Boolean isArrayProperty(const CIMName& name) const;

    /** Returns the number of unique property names from the  statement.
    */
    Uint32 getQueryPropertyNameCount() const;

    /** Gets the i-th unique property appearing in the statement.
    */
    const CIMName& getQueryPropertyName(Uint32 i) const;

    /**
        Returns the required properties from the query for the
        specified class.

        @param  inClassName  name of the class; must be one of the classes from
                             the FROM clause

        @return  CIMPropertyList containing the required properties from the
                 WHERE clause for the specified class;
                 or a null CIMPropertyList if all properties of the specified
                 class are required
    */
    CIMPropertyList getQueryPropertyList(
        const CIMObjectPath& inClassName = CIMObjectPath());

    /** Appends a property name to the query property name list. The
        user should not call this method; it should only be called
        by the parser.

        @param x name of the property.
        @return false if a property with that name already exists.
    */
    Boolean appendQueryPropertyName(const CIMName& x);


    /** Appends an operation to the operation array. This method should only
        be called by the parser itself.
    */
    void appendOperation(FQLOperation x);

    /** Appends an operand to the operation array. This method should only
        be called by the parser itself.
    */
    void appendOperand(const FQLOperand& x);

    /*
        chain a new operand to an existing operand
    */
    void chainOperand(const FQLOperand& x);

    /** Evalautes the query using the symbol table to resolve
     *  symbols.
    */
    Boolean evaluateQuery(const FQLPropertySource* source) const;


    /** Prints out the members of this class.
    */
    void print() const;

    // Generate a String display output of the statement.
    String toString() const;

    Boolean evaluate(const CIMInstance& inCI);

private:

    FQLQueryStatementRep* _rep;
};

PEGASUS_NAMESPACE_END

#endif /* Pegasus_FQLQueryStatement_h */

