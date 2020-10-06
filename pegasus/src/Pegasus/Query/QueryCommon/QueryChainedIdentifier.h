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

#ifndef Pegasus_QueryChainedIdentifier_h
#define Pegasus_QueryChainedIdentifier_h

#include <Pegasus/Query/QueryCommon/Linkage.h>
#include <Pegasus/Common/Config.h>
#include <Pegasus/Common/ArrayInternal.h>
#include <Pegasus/Common/String.h>
#include <Pegasus/Query/QueryCommon/QueryIdentifier.h>

#ifdef PEGASUS_USE_EXPERIMENTAL_INTERFACES

PEGASUS_NAMESPACE_BEGIN

class QueryChainedIdentifierRep;
class QueryContext;

class PEGASUS_QUERYCOMMON_LINKAGE QueryChainedIdentifier
{
public:

    QueryChainedIdentifier();

    QueryChainedIdentifier(const QueryIdentifier & id);

    QueryChainedIdentifier(const QueryChainedIdentifier& cid);

    virtual ~QueryChainedIdentifier();

    const Array<QueryIdentifier>& getSubIdentifiers() const;

    String toString() const;

    void append(const QueryIdentifier & id);

    Uint32 size() const;

    Boolean prepend(const QueryIdentifier & id);

    QueryIdentifier operator[](Uint32 index) const;

    QueryChainedIdentifier& operator=(const QueryChainedIdentifier& rhs);

    Boolean isSubChain(const QueryChainedIdentifier & chain) const;

    QueryIdentifier getLastIdentifier() const;

    void applyContext(const QueryContext& inContext);

  protected:

    QueryChainedIdentifierRep* _rep;

};

PEGASUS_NAMESPACE_END

#endif

#endif /* Pegasus_QueryChainedIdentifier_h */
