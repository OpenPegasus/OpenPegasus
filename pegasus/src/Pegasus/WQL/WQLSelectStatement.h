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

#ifndef Pegasus_WQLSelectStatement_h
#define Pegasus_WQLSelectStatement_h

#include <Pegasus/Common/Config.h>
#include <Pegasus/WQL/Linkage.h>
#include <Pegasus/Common/ArrayInternal.h>
#include <Pegasus/Common/CIMName.h>
#include <Pegasus/Common/CIMPropertyList.h>
#include <Pegasus/Common/CIMInstance.h>
#include <Pegasus/Common/CIMObject.h>
#include <Pegasus/WQL/WQLOperation.h>
#include <Pegasus/WQL/WQLOperand.h>
#include <Pegasus/WQL/WQLPropertySource.h>
#include <Pegasus/Query/QueryCommon/SelectStatement.h>
#include <Pegasus/Query/QueryCommon/QueryContext.h>

PEGASUS_NAMESPACE_BEGIN


class WQLSelectStatementRep;

/** This class represents a compiled WQL1 select statement.

    An instance of WQLSelectStatement is passed to WQLParser::parse() which
    parses the WQL1 SELECT statement and initializes the WQLSelectStatement
    instance. A WQL1 SELECT statement has the following form:

    <pre>
        SELECT &lt;property&gt;...
        FROM &lt;class name&gt;
        WHERE &lt;where clause&gt;
    </pre>

    There are methods for obtaining the various elements of the select
    statement.

    The components of the where clause are stored in two arrays: one for
    operands and one for operators (these are placed in proper order by the
    YACC parser). Evaluation is performed using a Boolean stack. See the
    implementation of evaluateWhereClause() for details.
*/
class PEGASUS_WQL_LINKAGE WQLSelectStatement: public SelectStatement
{
public:

    WQLSelectStatement(
        const String& queryLang,
        const String& query);

    WQLSelectStatement(
        const String& queryLang,
        const String& query,
        const QueryContext& inCtx);

    /** Default constructor.
    */
    WQLSelectStatement();

    WQLSelectStatement(const WQLSelectStatement& statement);

    /** Destructor.
    */
    ~WQLSelectStatement();

    WQLSelectStatement& operator=(const WQLSelectStatement& rhs);

    /** Clears all data members of this object.
    */
    void clear();

    /** Accessor.
    */
    const CIMName& getClassName() const;

    /** Modifier. This method should not be called by the user (only by the
    parser).
    */
    void setClassName(const CIMName& className);

    /**
        Returns true if the query selects all properties ("*")
    */
    Boolean getAllProperties() const;

    /**
        Used by the parser to indicate the query selects all properties ("*")
        This method should not be called by the user (only by the parser).
    */
    void setAllProperties(const Boolean allProperties);

    /** Returns the number of property names which were indicated in the
    selection list.
        This function should only be used if getAllProperties() returns false.
    */
    Uint32 getSelectPropertyNameCount() const;

    /** Gets the i-th selected property name in the list.
        This function should only be used if getAllProperties() returns false.
    */
    const CIMName& getSelectPropertyName(Uint32 i) const;

    /**
        Returns the required properties from the SELECT clause for the specified
        class.

        @param  inClassName  name of the class; must be one of the classes from
                             the FROM clause

        @return  CIMPropertyList containing the required properties from the
                 SELECT clause for the specified class;
                 or a null CIMPropertyList if all properties of the specified
                 class are required
    */
    CIMPropertyList getSelectPropertyList(
        const CIMObjectPath& inClassName = CIMObjectPath());

    /** Appends a property name to the property name list. The user should
        not call this method; it should only be called by the parser.
    */
    void appendSelectPropertyName(const CIMName& x);

    /** Returns the number of unique property names from the where clause.
    */
    Uint32 getWherePropertyNameCount() const;

    /** Gets the i-th unique property appearing in the where clause.
    */
    const CIMName& getWherePropertyName(Uint32 i) const;

    /**
        Returns the required properties from the WHERE clause for the specified
        class.

        @param  inClassName  name of the class; must be one of the classes from
                             the FROM clause

        @return  CIMPropertyList containing the required properties from the
                 WHERE clause for the specified class;
                 or a null CIMPropertyList if all properties of the specified
                 class are required
    */
    CIMPropertyList getWherePropertyList(
        const CIMObjectPath& inClassName = CIMObjectPath());

    /** Appends a property name to the where property name list. The user
        should not call this method; it should only be called by the parser.

        @param x name of the property.
        @return false if a property with that name already exists.
    */
    Boolean appendWherePropertyName(const CIMName& x);

    /** Appends an operation to the operation array. This method should only
        be called by the parser itself.
    */
    void appendOperation(WQLOperation x);

    /** Appends an operand to the operation array. This method should only
        be called by the parser itself.
    */
    void appendOperand(const WQLOperand& x);

    /** Returns true if this class has a where clause.
    */
    Boolean hasWhereClause() const;

    /** Evalautes the where clause using the symbol table to resolve symbols.
    */
    Boolean evaluateWhereClause(const WQLPropertySource* source) const;

    /** Inspect an instance and remove properties not listed in Select
        projection.

        @param  allowMissing  Boolean specifying whether missing project
                              properties are allowed
        @exception Exception
    */
    void applyProjection(
        CIMInstance& inst,
        Boolean allowMissing);
    void applyProjection(
        CIMObject& inst,
        Boolean allowMissing);

    /** Prints out the members of this class.
    */
    void print() const;

    Boolean evaluate(const CIMInstance& inCI);

    void validate();

    CIMPropertyList getPropertyList(
        const CIMObjectPath& inClassName = CIMObjectPath());

    Array<CIMObjectPath> getClassPathList() const;

private:

    WQLSelectStatementRep* _rep;

    //void f() const { }

    friend class CMPI_Wql2Dnf;
};

PEGASUS_NAMESPACE_END

#endif /* Pegasus_WQLSelectStatement_h */
