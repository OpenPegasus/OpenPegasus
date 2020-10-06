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

#include <Pegasus/Query/QueryExpression/QueryExpression.h>
#include <Pegasus/Common/Config.h>
#include <Pegasus/Common/AutoPtr.h>
#include <Pegasus/Common/PegasusVersion.h>
#include <Pegasus/Query/QueryCommon/QueryException.h>
#include <Pegasus/CQL/CQLSelectStatement.h>
#include <Pegasus/CQL/CQLParser.h>
#include <Pegasus/WQL/WQLSelectStatement.h>
#include <Pegasus/WQL/WQLParser.h>

#include <iostream>

PEGASUS_NAMESPACE_BEGIN

QueryExpression::QueryExpression()
    : _ss(NULL)
{
}

QueryExpression::QueryExpression(
    String queryLang,
    String query,
    QueryContext& ctx)
    : _queryLang(queryLang)
{
    String cimCQL("CIM:CQL");
    String dmtfCQL("DMTF:CQL");
    String wql("WQL");

#ifdef PEGASUS_ENABLE_CQL
    if (queryLang == cimCQL ||
        queryLang == dmtfCQL)
    {
        AutoPtr<CQLSelectStatement> cqlss(
            new CQLSelectStatement(queryLang, query, ctx));

        // Compile the statement
        CQLParser::parse(query, *cqlss.get());

        // Finish checking the statement for CQL by applying the class
        // contexts to the chained identifiers.
        cqlss->applyContext();

        _ss = cqlss.release();
    }
    else
#endif
    if (queryLang == wql)
    {
        AutoPtr<WQLSelectStatement> wqlss(
            new WQLSelectStatement(queryLang, query, ctx));

        // Compile the statement
        WQLParser::parse(query, *wqlss.get());

        _ss = wqlss.release();
    }
    else
    {
        throw QueryLanguageInvalidException(MessageLoaderParms(
            "Query.QueryExpression.INVALID_QUERY_LANGUAGE",
            "The query language specified is invalid: $0.",
            queryLang));
    }
}

QueryExpression::QueryExpression(
    String queryLang,
    String query)
    : _queryLang(queryLang)
{
    String cimCQL("CIM:CQL");
    String dmtfCQL("DMTF:CQL");
    String wql("WQL");

#ifdef PEGASUS_ENABLE_CQL
    if (queryLang == cimCQL ||
        queryLang == dmtfCQL)
    {
        CQLSelectStatement* cqlss = new CQLSelectStatement(queryLang, query);

        // Note: cannot call parse the CQLSelectStatement
        // because there is no QueryContext.
        // The parse will happen when setQueryContext is called.

        _ss = cqlss;
    }
    else
#endif
    if (queryLang == wql)
    {
        WQLSelectStatement* wqlss = new WQLSelectStatement(queryLang, query);

        // Compile the statement
        WQLParser::parse(query, *wqlss);

        _ss = wqlss;
    }
    else
    {
        throw QueryLanguageInvalidException(MessageLoaderParms(
            "Query.QueryExpression.INVALID_QUERY_LANGUAGE",
            "The query language specified is invalid: $0.",
            queryLang));
    }
}

QueryExpression::QueryExpression(const QueryExpression& expr)
    : _queryLang(expr._queryLang)
{
    if (expr._ss == NULL)
    {
        _ss = NULL;
    }
    else
    {
        _ss = NULL;

        String cimCQL("CIM:CQL");
        String dmtfCQL("DMTF:CQL");
        String wql("WQL");

#ifdef PEGASUS_ENABLE_CQL
        if (expr._queryLang == cimCQL ||
            expr._queryLang == dmtfCQL)
        {
            CQLSelectStatement* tempSS =
                dynamic_cast<CQLSelectStatement*>(expr._ss);
            if (tempSS != NULL)
                _ss = new CQLSelectStatement(*tempSS);
        }
        else
#endif
        if (expr._queryLang == wql)
        {
            WQLSelectStatement* tempSS =
                dynamic_cast<WQLSelectStatement*>(expr._ss);
            if (tempSS != NULL)
                _ss = new WQLSelectStatement(*tempSS);
        }
    }
}

QueryExpression::~QueryExpression()
{
    delete _ss;
}

QueryExpression QueryExpression::operator=(const QueryExpression& rhs)
{
    if (this == &rhs)
        return *this;

    delete _ss;
    _ss = NULL;

    if (rhs._ss != NULL)
    {
        String cimCQL("CIM:CQL");
        String dmtfCQL("DMTF:CQL");
        String wql("WQL");

#ifdef PEGASUS_ENABLE_CQL
        if (rhs._queryLang == cimCQL ||
            rhs._queryLang == dmtfCQL)
        {
            CQLSelectStatement* tempSS =
                dynamic_cast<CQLSelectStatement*>(rhs._ss);
            if (tempSS != NULL)
                _ss = new CQLSelectStatement(*tempSS);
        }
        else
#endif
        if (rhs._queryLang == wql)
        {
            WQLSelectStatement* tempSS =
                dynamic_cast<WQLSelectStatement*>(rhs._ss);
            if (tempSS != NULL)
                _ss = new WQLSelectStatement(*tempSS);
          }
    }

    _queryLang = rhs._queryLang;

    return *this;
}

String QueryExpression::getQueryLanguage() const
{
    if (_ss == NULL)
    {
        MessageLoaderParms parms(
            "Query.QueryExpression.SS_IS_NULL",
            "Trying to process a query with a NULL SelectStatement.");
        throw QueryException(parms);
    }

    return _ss->getQueryLanguage();
}

String QueryExpression::getQuery() const
{
    if (_ss == NULL)
    {
        MessageLoaderParms parms(
            "Query.QueryExpression.SS_IS_NULL",
            "Trying to process a query with a NULL SelectStatement.");
        throw QueryException(parms);
    }

    return _ss->getQuery();
}

Boolean QueryExpression::evaluate(const CIMInstance & inst) const
{
    if(_ss == NULL)
    {
        MessageLoaderParms parms(
            "Query.QueryExpression.SS_IS_NULL",
            "Trying to process a query with a NULL SelectStatement.");
        throw QueryException(parms);
    }

    try
    {
        return _ss->evaluate(inst);
    }
    catch (QueryException&)
    {
        throw;
    }
    catch (Exception& e)
    {
        throw PEGASUS_QUERY_EXCEPTION(e.getContentLanguages(), e.getMessage());
    }
}

CIMPropertyList QueryExpression::getPropertyList(
    const CIMObjectPath& objectPath) const
{
    if (_ss == NULL)
    {
        MessageLoaderParms parms(
            "Query.QueryExpression.SS_IS_NULL",
            "Trying to process a query with a NULL SelectStatement.");
        throw QueryException(parms);
    }

    try
    {
        return _ss->getPropertyList(objectPath);
    }
    catch (QueryException&)
    {
        throw;
    }
    catch (Exception& e)
    {
        throw PEGASUS_QUERY_EXCEPTION(e.getContentLanguages(), e.getMessage());
    }
}

CIMPropertyList QueryExpression::getSelectPropertyList
    (const CIMObjectPath& objectPath) const
{
    if (_ss == NULL)
    {
        MessageLoaderParms parms ("Query.QueryExpression.SS_IS_NULL",
            "Trying to process a query with a NULL SelectStatement.");
        throw QueryException (parms);
    }

    try
    {
        return _ss->getSelectPropertyList (objectPath);
    }
    catch (QueryException&)
    {
        throw;
    }
    catch (Exception& e)
    {
        throw PEGASUS_QUERY_EXCEPTION(e.getContentLanguages(), e.getMessage());
    }
}

CIMPropertyList QueryExpression::getWherePropertyList
    (const CIMObjectPath& objectPath) const
{
    if (_ss == NULL)
    {
        MessageLoaderParms parms ("Query.QueryExpression.SS_IS_NULL",
            "Trying to process a query with a NULL SelectStatement.");
        throw QueryException (parms);
    }

    try
    {
        return _ss->getWherePropertyList (objectPath);
    }
    catch (QueryException&)
    {
        throw;
    }
    catch (Exception& e)
    {
        throw PEGASUS_QUERY_EXCEPTION(e.getContentLanguages(), e.getMessage());
    }
}

void QueryExpression::applyProjection(CIMInstance instance,
    Boolean allowMissing)
{
    if (_ss == NULL)
    {
        MessageLoaderParms parms(
            "Query.QueryExpression.SS_IS_NULL",
            "Trying to process a query with a NULL SelectStatement.");
        throw QueryException(parms);
    }

    try
    {
        _ss->applyProjection(instance, allowMissing);
    }
    catch (QueryException&)
    {
        throw;
    }
    catch (Exception& e)
    {
        throw PEGASUS_QUERY_EXCEPTION(e.getContentLanguages(), e.getMessage());
    }
}

void QueryExpression::validate()
{
    if (_ss == NULL)
    {
        MessageLoaderParms parms(
            "Query.QueryExpression.SS_IS_NULL",
            "Trying to process a query with a NULL SelectStatement.");
        throw QueryException(parms);
    }

    try
    {
        _ss->validate();
    }
    catch (QueryException&)
    {
        throw;
    }
    catch (Exception& e)
    {
        throw PEGASUS_QUERY_EXCEPTION(e.getContentLanguages(), e.getMessage());
    }
}

Array<CIMObjectPath> QueryExpression::getClassPathList() const
{
    if (_ss == NULL)
    {
        MessageLoaderParms parms(
            "Query.QueryExpression.SS_IS_NULL",
            "Trying to process a query with a NULL SelectStatement.");
        throw QueryException(parms);
    }

    try
    {
        return _ss->getClassPathList();
    }
    catch (QueryException&)
    {
        throw;
    }
    catch (Exception& e)
    {
        throw PEGASUS_QUERY_EXCEPTION(e.getContentLanguages(), e.getMessage());
    }
}

SelectStatement* QueryExpression::getSelectStatement()
{
    return _ss;
}

void QueryExpression::setQueryContext(QueryContext& inCtx)
{
    if (_ss == NULL)
    {
        MessageLoaderParms parms(
            "Query.QueryExpression.SS_IS_NULL",
            "Trying to process a query with a NULL SelectStatement.");
        throw QueryException(parms);
    }

    // SelectStatement only allows this to be called once.
    _ss->setQueryContext(inCtx);

#ifdef PEGASUS_ENABLE_CQL
    String cimCQL("CIM:CQL");
    String dmtfCQL("DMTF:CQL");

    if (_queryLang == cimCQL ||
        _queryLang == dmtfCQL)
    {
        // Now that we have a QueryContext, we can finish compiling
        // the CQL statement.
        CQLSelectStatement* tempSS = dynamic_cast<CQLSelectStatement*>(_ss);
        if (tempSS != NULL)
        {
            CQLParser::parse(getQuery(), *tempSS);
            tempSS->applyContext();
        }
    }
#endif
}

PEGASUS_NAMESPACE_END
