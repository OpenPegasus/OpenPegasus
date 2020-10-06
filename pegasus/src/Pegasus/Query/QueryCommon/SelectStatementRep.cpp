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

#include "SelectStatement.h"
#include "SelectStatementRep.h"

PEGASUS_NAMESPACE_BEGIN

SelectStatementRep::SelectStatementRep()
    : _ctx(NULL)
{
}

SelectStatementRep::SelectStatementRep(const SelectStatementRep& ssr)
    : _qlang(ssr._qlang),
      _query(ssr._query)
{
    _ctx = NULL;
    if (ssr._ctx != NULL)
    {
        _ctx = ssr._ctx->clone();
    }
}

SelectStatementRep::SelectStatementRep(
    const String& inQlang,
    const String& inQuery,
    const QueryContext& inCtx)
    : _qlang(inQlang),
      _query(inQuery)
{
    _ctx = inCtx.clone();
}

SelectStatementRep::SelectStatementRep(
    const String& inQlang,
    const String& inQuery)
    : _qlang(inQlang),
      _query(inQuery),
      _ctx(NULL)
{
}

SelectStatementRep::~SelectStatementRep()
{
    delete _ctx;
}

SelectStatementRep& SelectStatementRep::operator=(const SelectStatementRep& rhs)
{
    if (this == &rhs)
        return *this;

    _qlang = rhs._qlang;
    _query = rhs._query;

    delete _ctx;

    if (rhs._ctx != NULL)
    {
        _ctx = rhs._ctx->clone();
    }
    else
    {
        _ctx = NULL;
    }

    return *this;
}

String SelectStatementRep::getQueryLanguage() const
{
    return _qlang;
}

String SelectStatementRep::getQuery() const
{
    return _query;
}

void SelectStatementRep::setQueryContext(const QueryContext& inCtx)
{
    if (_ctx == NULL)
    {
        _ctx = inCtx.clone();
    }
    else
    {
        throw QueryException(MessageLoaderParms(
            "QueryCommon.SelectStatementRep.QUERY_CTX_ALREADY_SET",
            "The QueryContext can only be set once on a SelectStatement."));
    }
}

PEGASUS_NAMESPACE_END
