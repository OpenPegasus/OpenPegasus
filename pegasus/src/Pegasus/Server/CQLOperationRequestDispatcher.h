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

#ifndef CQLOPERATIONREQUESTDISPATCHER_H_
#define CQLOPERATIONREQUESTDISPATCHER_H_

#include <Pegasus/Server/CIMOperationRequestDispatcher.h>
#include <Pegasus/CQL/CQLParser.h>
#include <Pegasus/CQL/CQLSelectStatement.h>
#include <Pegasus/CQL/CQLQueryExpressionRep.h>

PEGASUS_NAMESPACE_BEGIN

class PEGASUS_SERVER_LINKAGE CQLOperationRequestDispatcher :
    public CIMOperationRequestDispatcher
{
private:
    CQLOperationRequestDispatcher(
        CIMRepository* repository,
        ProviderRegistrationManager* providerRegistrationManager)
        : CIMOperationRequestDispatcher(
              repository,providerRegistrationManager)
    {
    }

public:

    virtual ~CQLOperationRequestDispatcher() {}

    /** Procces the query request. If there is an error returns the
       cimException and return = true
       @param request Query message being processed
       @param cimException CIMException that will contain the
       CIMException to be returned if there was an error in processing.
       Only set when the return is true
       parm enumerationContext EnumerationContext object if this is
       a pull request. Otherwise it is NULL
       @return true if no errors. False if there were errors in processing
       and the cimException parameter must contain the error.
    */
    bool handleQueryRequest(
        CIMExecQueryRequestMessage* request,
        CIMException& cimException,
        EnumerationContext* enumerationContext = NULL);

    static void applyQueryToEnumeration(
        CIMResponseMessage* msg,
        QueryExpressionRep* query);
};

PEGASUS_NAMESPACE_END

#endif /* CQLOPERATIONREQUESTDISPATCHER_H_ */
