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

#ifndef Pegasus_WQLSelectStatementRep_h
#define Pegasus_WQLSelectStatementRep_h

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
#include <Pegasus/Query/QueryCommon/QueryContext.h>
#include <Pegasus/Query/QueryCommon/SelectStatementRep.h>

PEGASUS_NAMESPACE_BEGIN

class WQLSelectStatementRep: public SelectStatementRep
{
public:

    WQLSelectStatementRep(
        const String& queryLang,
        const String& query);

    WQLSelectStatementRep(
        const String& queryLang,
        const String& query,
        const QueryContext& inCtx);

    /** Default constructor.
    */
    WQLSelectStatementRep();

    WQLSelectStatementRep(const WQLSelectStatementRep& rep);

    /** Destructor.
    */
    ~WQLSelectStatementRep();

    /** Clears all data members of this object.
    */
    void clear();

    /** Accessor.
    */
    const CIMName& getClassName() const
    {
        return _className;
    }

    /** Modifier. This method should not be called by the user (only by the
    parser).
    */
    void setClassName(const CIMName& className)
    {
        _className = className;
    }

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
    Uint32 getSelectPropertyNameCount() const
    {
    return _selectPropertyNames.size();
    }

    /** Gets the i-th selected property name in the list.
        This function should only be used if getAllProperties() returns false.
    */
    const CIMName& getSelectPropertyName(Uint32 i) const
    {
    return _selectPropertyNames[i];
    }

    /**
        Returns a CIMPropertyList containing the selected properties.
        The list is NULL if the query selects all properties (SELECT * FROM...).
    */
    const CIMPropertyList getSelectPropertyList(
        const CIMObjectPath& inClassName) const;

    /** Appends a property name to the property name list. The user should
        not call this method; it should only be called by the parser.
    */
    void appendSelectPropertyName(const CIMName& x)
    {
        _selectPropertyNames.append(x);
    }

    /** Returns the number of unique property names from the where clause.
    */
    Uint32 getWherePropertyNameCount() const
    {
        return _wherePropertyNames.size();
    }

    /** Gets the i-th unique property appearing in the where clause.
    */
    const CIMName& getWherePropertyName(Uint32 i) const
    {
        return _wherePropertyNames[i];
    }

    /**
        Returns a CIMPropertyList containing the unique properties used in the
        WHERE clause
    */
    const CIMPropertyList getWherePropertyList(
        const CIMObjectPath& inClassName) const;

    /** Appends a property name to the where property name list. The user
    should not call this method; it should only be called by the parser.

    @param x name of the property.
    @return false if a property with that name already exists.
    */
    Boolean appendWherePropertyName(const CIMName& x);

    /** Appends an operation to the operation array. This method should only
    be called by the parser itself.
    */
    void appendOperation(WQLOperation x)
    {
        _operations.append(x);
    }

    /** Appends an operand to the operation array. This method should only
    be called by the parser itself.
    */
    void appendOperand(const WQLOperand& x)
    {
        _operands.append(x);
    }

    /** Returns true if this class has a where clause.
    */
    Boolean hasWhereClause() const
    {
        return _operations.size() != 0;
    }

    /** Evalautes the where clause using the symbol table to resolve symbols.
    */
    Boolean evaluateWhereClause(const WQLPropertySource* source) const;

    /** Inspect an instance and remove properties not listed in Select
        projection.
    */
    void applyProjection(CIMInstance& inst,
        Boolean allowMissing);
    void applyProjection(CIMObject& inst,
        Boolean allowMissing);

    /** Prints out the members of this class.
    */
    void print() const;

    Boolean evaluate(const CIMInstance& inCI);

    void validate();

    CIMPropertyList getPropertyList(const CIMObjectPath& inClassName);

    Array<CIMObjectPath> getClassPathList() const;

private:

    //
    // The name of the target class. For example:
    //
    //     SELECT *
    //     FROM TargetClass
    //     WHERE ...
    //

    CIMName _className;

    //
    // Indicates that all properties are selected (i.e. SELECT * FROM ...)
    //
    Boolean _allProperties;

    //
    // The list of property names being selected. For example, see "firstName",
    // and "lastName" below.
    //
    //     SELECT firstName, lastName
    //     FROM TargetClass
    //     WHERE ...
    //
    // NOTE: if the query selects all properties, this list is empty, and
    // _allProperties is true
    //
    // NOTE: duplicate property names are not removed from the select list
    // (e.g. SELECT firstName, firstName FROM...) results in a list of
    // two properties
    //

    Array<CIMName> _selectPropertyNames;

    //
    // The unique list of property names appearing in the WHERE clause.
    // Although a property may occur many times in the WHERE clause, it will
    // only appear once in this list.
    //

    Array<CIMName> _wherePropertyNames;

    //
    // The list of operations encountered while parsing the WHERE clause.
    // Consider this query:
    //
    //     SELECT *
    //     FROM TargetClass
    //     WHERE count > 10 OR peak < 20 AND state = "OKAY"
    //
    // This would generate the following stream of WQLOperations:
    //
    //     WQL_GT
    //     WQL_LT
    //     WQL_EQ
    //     WQL_AND
    //     WQL_OR
    //

    Array<WQLOperation> _operations;

    //
    // The list of operands encountered while parsing the WHERE clause. The
    // query just above would generate the following stream of operands:
    //
    //     count, 10, peak, 20, state, "OKAY"
    //

    Array<WQLOperand> _operands;

    void f() const { }

    friend class CMPI_Wql2Dnf;
};

PEGASUS_NAMESPACE_END

#endif /* Pegasus_WQLSelectStatementRep_h */
