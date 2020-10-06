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

#ifndef Pegasus_FQLQueryStatementRep_h
#define Pegasus_FQLQueryStatementRep_h

#include <Pegasus/Common/Config.h>
#include <Pegasus/FQL/Linkage.h>
#include <Pegasus/Common/ArrayInternal.h>
#include <Pegasus/Common/CIMClass.h>
#include <Pegasus/Common/CIMName.h>
#include <Pegasus/Common/CIMPropertyList.h>
#include <Pegasus/Common/CIMInstance.h>
#include <Pegasus/Common/CIMObject.h>
#include <Pegasus/FQL/FQLOperation.h>
#include <Pegasus/FQL/FQLOperand.h>
#include <Pegasus/FQL/FQLPropertySource.h>

////#include <Pegasus/Query/QueryCommon/QueryContext.h>
////#include <Pegasus/Query/QueryCommon/SelectStatementRep.h>

PEGASUS_NAMESPACE_BEGIN

class FQLQueryStatementRep
{
public:

    FQLQueryStatementRep(
        const CIMClass& queryClass,
        const String& query);

    FQLQueryStatementRep(
        const String& query);


//  FQLQueryStatementRep(
//      const String& queryLang,
//      const String& query,
//      const QueryContext& inCtx);

    /** Default constructor.
    */
    FQLQueryStatementRep();

    FQLQueryStatementRep(const FQLQueryStatementRep& rep);

    /** Destructor.
    */
    ~FQLQueryStatementRep();

    /** Clears all data members of this object.
    */
    void clear();

////  Boolean isArrayProperty(const CIMName& name) const;

    /** Returns the number of unique property names from the where clause.
    */
    Uint32 getQueryPropertyNameCount() const
    {
        return _queryPropertyNames.size();
    }

    /** Gets the i-th unique property appearing in the query.
    */
    const CIMName& getQueryPropertyName(Uint32 i) const
    {
        return _queryPropertyNames[i];
    }

    /**
        Returns a CIMPropertyList containing the unique properties used in the
        Query clause
    */
    const CIMPropertyList getQueryPropertyList(
        const CIMObjectPath& inClassName) const;

    /** Appends a property name to the  property name list. The user
    should not call this method; it should only be called by the parser.

    @param x name of the property.
    @return false if a property with that name already exists.
    */
    Boolean appendQueryPropertyName(const CIMName& x);

    /** Appends an operation to the operation array. This method should only
    be called by the parser itself.
    */
    void appendOperation(FQLOperation x)
    {
        _operations.append(x);
    }

    /** Appends an operand to the operand array. This method should
    only be called by the parser itself.
    */
    void appendOperand(const FQLOperand& x)
    {
        _operands.append(x);
    }
    /*
        Chain this operand to the last existing one in the operand
        array
    */
    void chainOperand(const FQLOperand& x)
    {
        DCOUT << "chain  to size " << _operands.size() << endl;
        PEGASUS_ASSERT(_operands.size() != 0);
        Uint32 index = _operands.size() -1;
        PEGASUS_ASSERT(_operands[index].getType()
                       == FQLOperand::PROPERTY_NAME);

        DCOUT << "chain  to " << index << endl;
        _operands[index].chain(x);
    }

    /** Evalautes the query using the symbol table to resolve
     *  symbols.
    */
    Boolean evaluateQuery(const FQLPropertySource* source) const;

    /** Prints out the members of this class.
    */
    void print() const;

    String toString() const;

    Boolean evaluate(const CIMInstance& inCI);

private:

    /**  The query string to be processed.
    */
    String _query;

    //
    // The unique list of property names appearing in the WHERE clause.
    // Although a property may occur many times in the WHERE clause, it will
    // only appear once in this list.
    //

    Array<CIMName> _queryPropertyNames;

    //
    // The list of operations encountered while parsing the WHERE clause.
    // Consider this query:
    //
    //     count > 10 OR peak < 20 AND state = "OKAY"
    //
    // This would generate the following stream of WQLOperations:
    //
    //     FQL_GT
    //     FQL_LT
    //     FQL_EQ
    //     FQL_AND
    //     FQL_OR
    //

    Array<FQLOperation> _operations;

    //
    // The list of operands encountered while parsing the query. The
    // query just above would generate the following stream of operands:
    //
    //     count, 10, peak, 20, state, "OKAY"
    //

    Array<FQLOperand> _operands;

    // KS_ TODO why separate out the array operands????
    Array<FQLOperand> _arrayOperands;

    void f() const { }
};

PEGASUS_NAMESPACE_END

#endif /* Pegasus_FQLQueryStatementRep_h */

