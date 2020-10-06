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

#ifndef Pegasus_QueryExpression_h
#define Pegasus_QueryExpression_h

#include <Pegasus/Query/QueryExpression/Linkage.h>
#include <Pegasus/Common/Config.h>
#include <Pegasus/Common/String.h>
#include <Pegasus/Common/CIMInstance.h>
#include <Pegasus/Common/CIMPropertyList.h>
#include <Pegasus/Query/QueryCommon/QueryContext.h>
#include <Pegasus/Query/QueryCommon/SelectStatement.h>

PEGASUS_NAMESPACE_BEGIN

class SelectStatement;

/** This class gives access to the particulars of a query expression.*/
class PEGASUS_QUERYEXPRESSION_LINKAGE QueryExpression
{
public:

    /**
        Constructor
        Note: this object will throw exceptions when it is used after
        default construction.
    */
    QueryExpression();

    /**
        Constructor.
        At the moment the constructor only accepts WQL style Select statements.
        The constructor is normally called by the Provider Manager.

        @param language contains the query language type, "WQL" is supported.
        @param query contains a WQL style select statement
    */
    QueryExpression(
        String queryLang,
        String query,
        QueryContext& ctx);

    QueryExpression(
        String queryLang,
        String query);

    QueryExpression(const QueryExpression& expr);

    /**
        Destructor.
        The destructor normally only called by the Provider Manager.
    */
    ~QueryExpression();

    /**
        Operator =
    */
    QueryExpression operator=(const QueryExpression& rhs);

   /**
       \Label{getQueryLanguage}
       Returns the query language identifier.
       @return A String instance containing the query language identifier.
    */
    String getQueryLanguage() const;

    /**
        \Label{getQuery}
        Returns the query select statement.
        @return A String instance containing the query select statement.
    */
    String getQuery() const;

    /**
        \Label{evaluate}
        Returns true if the instance passes where clause evaluation
        successfully.

        @param inst the instance to be evaluated.
        @return A Boolean indicating result of evaluation.
    */
    Boolean evaluate(const CIMInstance& inst) const;

    /**
        \Label{getPropertyList}
        Returns the select clause projection specification as CIMPropertyList
        A NULL CIMPropertyList is returned when all properties are to be
        returned.
        @return A CIMPropertylist instance containing the select clause
        projection specification.
    */
    CIMPropertyList getPropertyList(
        const CIMObjectPath& objectPath = CIMObjectPath()) const;

    /**
        Returns the required properties from the SELECT clause for the specified
        class.

        @param  objectPath  the class; must be one of the classes from the FROM
                            clause

        @return  CIMPropertyList containing the required properties from the
                 SELECT clause for the specified class;
                 or a null CIMPropertyList if all properties of the specified
                 class are required
    */
    CIMPropertyList getSelectPropertyList(
        const CIMObjectPath& objectPath = CIMObjectPath()) const;

    /**
        Returns the required properties from the WHERE clause for the specified
        class.

        @param  objectPath  the class; must be one of the classes from the FROM
                            clause

        @return  CIMPropertyList containing the required properties from the
                 WHERE clause for the specified class;
                 or a null CIMPropertyList if all properties of the specified
                 class are required
    */
    CIMPropertyList getWherePropertyList(
        const CIMObjectPath& objectPath = CIMObjectPath()) const;

    void applyProjection(
        CIMInstance instance,
        Boolean allowMissing);

    void validate();

    Array<CIMObjectPath> getClassPathList() const;

    SelectStatement* getSelectStatement();

    void setQueryContext(QueryContext& inCtx);

private:
    SelectStatement *_ss;
    String _queryLang;
};

PEGASUS_NAMESPACE_END

#endif
