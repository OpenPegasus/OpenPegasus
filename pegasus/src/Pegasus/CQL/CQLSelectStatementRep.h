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

#ifndef Pegasus_CQLSelectStatementRep_h
#define Pegasus_CQLSelectStatementRep_h

#include <Pegasus/Query/QueryCommon/SelectStatementRep.h>
#include <Pegasus/Query/QueryCommon/QueryChainedIdentifier.h>
#include <Pegasus/CQL/CQLChainedIdentifier.h>
#include <Pegasus/CQL/Linkage.h>

PEGASUS_NAMESPACE_BEGIN

struct PropertyNode;

class CQLSelectStatementRep : public SelectStatementRep
{
  public:

    CQLSelectStatementRep();

    CQLSelectStatementRep(
        const String& inQlang,
        const String& inQuery,
        const QueryContext& inCtx);

    CQLSelectStatementRep(
        const String& inQlang,
        const String& inQuery);

    CQLSelectStatementRep(const CQLSelectStatementRep& rep);

    ~CQLSelectStatementRep();

    CQLSelectStatementRep& operator=(const CQLSelectStatementRep& cqlss);

    Boolean evaluate(const CIMInstance& inCI);

    void applyProjection(
        CIMInstance& inCI,
        Boolean allowMissing);

    void validate();

    Array<CIMObjectPath> getClassPathList() const;

    CIMPropertyList getPropertyList(const CIMObjectPath& inClassName);

    CIMPropertyList getSelectPropertyList(const CIMObjectPath& inClassName);

    CIMPropertyList getWherePropertyList(const CIMObjectPath& inClassName);

    Array<CQLChainedIdentifier> getSelectChainedIdentifiers();

    Array<CQLChainedIdentifier> getWhereChainedIdentifiers();

    void appendClassPath(const CQLIdentifier& inIdentifier);

    void setPredicate(const CQLPredicate& inPredicate);

    CQLPredicate getPredicate() const;

    void insertClassPathAlias(
        const CQLIdentifier& inIdentifier,
        const String& inAlias);

    void appendSelectIdentifier(const CQLChainedIdentifier& x);

    void applyContext();

    void normalizeToDOC();

    String toString() const;

    void setHasWhereClause();

    Boolean hasWhereClause() const;

    void clear();

  protected:

    Array<CQLChainedIdentifier> _selectIdentifiers;

    Boolean _hasWhereClause;

  private:

    Boolean applyProjection(
        PropertyNode* node,
        CIMProperty& nodeProp,
        Boolean& preservePropsForParent,
        Boolean allowMissing) const;

    void validateProperty(const QueryChainedIdentifier& chainId) const;

    CIMName lookupFromClass(const String& lookup) const;

    CIMPropertyList getPropertyListInternal(
        const CIMObjectPath& inClassName,
        Boolean includeSelect,
        Boolean includeWhere);

    Boolean addRequiredProperty(
        Array<CIMName>& reqProps,
        const CIMName& className,
        const QueryChainedIdentifier& chainId,
        Array<CIMName>& matchedScopes,
        Array<CIMName>& unmatchedScopes) const;

    Boolean isFilterable(
        const CIMInstance& inst,
        PropertyNode* node) const;

    void filterInstance(
        CIMInstance& inst,
        Boolean& allPropsRequired,
        const CIMName& allPropsClass,
        Array<CIMName>& requiredProps,
        Boolean& preserveProps,
        Boolean allowMissing) const;

    static Boolean containsProperty(
        const CIMName& name,
        const Array<CIMName>& props);

    Boolean isFromChild(const CIMName& className) const;

    void checkWellFormedIdentifier(const QueryChainedIdentifier& chainId,
                                   Boolean isSelectListId);

    void reportNullContext() const;

    void CheckQueryContext() const
    {
        if (0 == _ctx)
        {
            reportNullContext();
        }
    }

    CQLPredicate _predicate;

    Boolean _contextApplied;
};

PEGASUS_NAMESPACE_END

#endif
