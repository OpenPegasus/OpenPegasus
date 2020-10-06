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
//%////////////////////////////////////////////////////////////////////////////

#ifndef CQLQUERYEXPRESSIONREP_H_
#define CQLQUERYEXPRESSIONREP_H_

#include <Pegasus/Common/Config.h>
#include <Pegasus/Common/String.h>
#include <Pegasus/Common/CIMName.h>
#include <Pegasus/Common/CIMInstance.h>
#include <Pegasus/Common/CIMPropertyList.h>

#include <Pegasus/Common/QueryExpressionRep.h>
#include "CQLSelectStatement.h"

PEGASUS_NAMESPACE_BEGIN

class CQLSelectStatement;

class PEGASUS_CQL_LINKAGE CQLQueryExpressionRep : public QueryExpressionRep
{
    friend class QueryExpression;
public:
    CQLQueryExpressionRep(const String & queryLanguage)
      : QueryExpressionRep(queryLanguage), _stmt(NULL) {}
    CQLQueryExpressionRep(const String & queryLanguage, CQLSelectStatement *s)
      : QueryExpressionRep(queryLanguage), _stmt(s){}
    CQLQueryExpressionRep(const String & queryLanguage, String & query)
      : QueryExpressionRep(queryLanguage,query), _stmt(NULL) {}

    PEGASUS_HIDDEN_LINKAGE
    virtual ~CQLQueryExpressionRep();

    PEGASUS_HIDDEN_LINKAGE
    virtual const CIMPropertyList getPropertyList() const;

    PEGASUS_HIDDEN_LINKAGE
    virtual Boolean evaluate(const CIMInstance &) const;

    PEGASUS_HIDDEN_LINKAGE
    virtual void applyProjection(CIMInstance &, Boolean allowMissing);

    PEGASUS_HIDDEN_LINKAGE
    void _parse();

    CQLSelectStatement *_stmt;
};

PEGASUS_NAMESPACE_END



#endif /* CQLQUERYEXPRESSIONREP_H_ */
