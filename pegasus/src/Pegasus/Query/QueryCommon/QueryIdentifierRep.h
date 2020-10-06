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
// Authors: David Rosckes (rosckes@us.ibm.com)
//          Bert Rivero (hurivero@us.ibm.com)
//          Chuck Carmack (carmack@us.ibm.com)
//          Brian Lucier (lucier@us.ibm.com)
//
// Modified By:
//
//%/////////////////////////////////////////////////////////////////////////////

#ifndef Pegasus_QueryIdentifierRep_h
#define Pegasus_QueryIdentifierRep_h

#include <Pegasus/Common/Config.h>
#include <Pegasus/Query/QueryCommon/Linkage.h>
#include <Pegasus/Common/String.h>
#include <Pegasus/Common/Array.h>
#include <Pegasus/Common/CIMName.h>
#include <Pegasus/Query/QueryCommon/SubRange.h>

PEGASUS_NAMESPACE_BEGIN

class PEGASUS_QUERYCOMMON_LINKAGE QueryIdentifierRep
{
  public:
    QueryIdentifierRep();

    QueryIdentifierRep(const QueryIdentifierRep* rep);

    QueryIdentifierRep& operator=(const QueryIdentifierRep& rhs);

    virtual ~QueryIdentifierRep();

    const CIMName& getName()const;

    void setName(const CIMName& inName);

    const String& getSymbolicConstantName()const;

    const  Array<SubRange>& getSubRanges()const;

    Boolean isArray()const;

    Boolean isSymbolicConstant()const;

    Boolean isWildcard()const;

    const String& getScope()const;

    Boolean isScoped()const;

    void applyScope(String scope);

    Boolean operator==(const CIMName &rhs)const;

    Boolean operator!=(const CIMName &rhs)const;

    Boolean operator==(const QueryIdentifierRep &rhs)const;

    Boolean operator!=(const QueryIdentifierRep &rhs)const;

    String toString()const;

  protected:

    String _symbolicConstant;
    String _scope;

    Array<SubRange> _indices;

    CIMName _name;

    Boolean _isWildcard;
    Boolean _isSymbolicConstant;
};

PEGASUS_NAMESPACE_END

#endif /* Pegasus_CQLIdentifier_h */
