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

#ifndef Pegasus_CIMInstanceQueryProvider_h
#define Pegasus_CIMInstanceQueryProvider_h

#include "CIMInstanceProvider.h"
#include <Pegasus/Query/QueryExpression/QueryExpression.h>

PEGASUS_NAMESPACE_BEGIN

/**
    This class extends then CIMInstanceProvider class enabling query support.

    <p>The Instance Query Provider receives operation requests from
    clients through calls to these functions by the CIM Server. Its
    purpose is to convert these to calls to system services,
    operations on system resources, or whatever platform-specific
    behavior is required to perform the operation modeled by
    the request. The specific requirements for each of the interface
    functions are discussed in their respective sections.</p>
*/

class PEGASUS_PROVIDER_LINKAGE CIMInstanceQueryProvider :
    public CIMInstanceProvider
{
public:
    /**
        Constructor.
        The constructor should not do anything.
    */
    CIMInstanceQueryProvider();

    /**
        Destructor.
        The destructor should not do anything.
    */
    virtual ~CIMInstanceQueryProvider();

    /**
        Return all instances of the specified class filtered by the query.

        @param context contains security and locale information relevant
        for the lifetime of this operation.
        @param objectPath contains namespace and classname for which the
        query is to be performed.
        @param query enables access to query details like query statement
        and language.
        @param handler asynchronusly processes the results of this operation.

        @exception CIMNotSupportedException
        @exception CIMAccessDeniedException
        @exception CIMOperationFailedException
    */
    virtual void execQuery(
       const OperationContext& context,
       const CIMObjectPath& objectPath,
       const QueryExpression& query,
       InstanceResponseHandler& handler) = 0;
};

PEGASUS_NAMESPACE_END

#endif
