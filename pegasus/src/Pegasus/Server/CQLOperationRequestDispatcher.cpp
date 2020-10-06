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
//%////////////////////////////////////////////////////////////////////////////


#include "CQLOperationRequestDispatcher.h"

#include <Pegasus/Common/AutoPtr.h>
#include <Pegasus/Common/QueryExpressionRep.h>
#include <Pegasus/Common/StatisticalData.h>
#include <Pegasus/Provider/CIMOMHandleQueryContext.h>
#include <Pegasus/Server/EnumerationContext.h>

PEGASUS_NAMESPACE_BEGIN

PEGASUS_USING_STD;

void CQLOperationRequestDispatcher::applyQueryToEnumeration(
    CIMResponseMessage* msg,
    QueryExpressionRep* query)
{
    PEG_METHOD_ENTER(TRC_DISPATCHER,
        "CQLOperationRequestDispatcher::applyQueryToEnumeration");

    CIMEnumerateInstancesResponseMessage* enr =
        (CIMEnumerateInstancesResponseMessage*) msg;
    CQLSelectStatement* qs = ((CQLQueryExpressionRep*)query)->_stmt;

    // Get instances in the response data, converting all of them
    // to C++ Instance format for the evaluator
    Array<CIMInstance>& a = enr->getResponseData().getInstances();

    // Remove any instances from this array that do not match
    for (int i = a.size() - 1; i >= 0; i--)
    {
        try
        {
            if (qs->evaluate(a[i]))
            {
                //
                // Specify that missing requested project properties are
                // allowed to be consistent with clarification from DMTF
                qs->applyProjection(a[i], true);
            }
            else
            {
                a.remove(i);
            }
        }
        catch (...)
        {
            a.remove(i);
        }
    }

    // Reset the CIMResponseData size since we may have modified it.
    enr->getResponseData().setSize();
    PEG_METHOD_EXIT();
}

bool CQLOperationRequestDispatcher::handleQueryRequest(
    CIMExecQueryRequestMessage* request,
    CIMException& cimException,
    EnumerationContext* enumerationContext)
{
    PEG_METHOD_ENTER(TRC_DISPATCHER,
        "CQLOperationRequestDispatcher::handleQueryRequest");
    Boolean exception=false;

    CIMOMHandle _ch;
    CIMOMHandleQueryContext _queryOrig(request->nameSpace,_ch);

    AutoPtr<CQLSelectStatement> selectStatement(
        new CQLSelectStatement(request->queryLanguage,
            request->query,
            _queryOrig));

    AutoPtr<CQLQueryExpressionRep> qx;
    CIMName className;

    if (request->queryLanguage != "DMTF:CQL")
    {
        cimException = PEGASUS_CIM_EXCEPTION(
            CIM_ERR_QUERY_LANGUAGE_NOT_SUPPORTED, request->queryLanguage);
        exception=true;
    }
    else
    {
        try
        {
            CQLParser::parse(request->query, *selectStatement.get());

            Array<CIMObjectPath> classPath =
                selectStatement->getClassPathList();
            className = classPath[0].getClassName();
            qx.reset(
                new CQLQueryExpressionRep("DMTF:CQL", selectStatement.get()));
            selectStatement.release();
        }
        catch (ParseError&)
        {
            cimException =
                PEGASUS_CIM_EXCEPTION(CIM_ERR_INVALID_QUERY, request->query);
            exception=true;
        }
        catch (MissingNullTerminator&)
        {
            cimException =
                PEGASUS_CIM_EXCEPTION(CIM_ERR_INVALID_QUERY, request->query);
            exception = true;
        }

        if (exception == false)
        {
            if (!_checkExistenceOfClass(request->nameSpace, className))
            {
                cimException = PEGASUS_CIM_EXCEPTION(
                    CIM_ERR_INVALID_CLASS, className.getString());
                exception = true;
            }
        }
    }

    if (exception)
    {
        PEG_METHOD_EXIT();
        return false;
    }

    bool rtn = handleQueryRequestCommon(request,
        cimException,
        enumerationContext,
        "DMTF:CQL",
        className,
        qx.release());

    PEG_METHOD_EXIT();
    return rtn;
}

PEGASUS_NAMESPACE_END
