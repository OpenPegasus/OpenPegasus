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

#ifndef Pegasus_CQLChainedIdentifierRep_h
#define Pegasus_CQLChainedIdentifierRep_h

#include <Pegasus/CQL/Linkage.h>
#include <Pegasus/Common/Array.h>
#include <Pegasus/Common/String.h>
#include <Pegasus/CQL/CQLIdentifier.h>
#include <Pegasus/Query/QueryCommon/QueryChainedIdentifierRep.h>

PEGASUS_NAMESPACE_BEGIN
class CQLFactory;
/**
CQLChainedIdentifier class is used for parsing (not resolving)
the identifier portion of a CQL select statement.
A couple examples of a CQLIdentifier:

  Class.EmbeddedObject.Property[3]
  Class.Property#'OK'


The CQLChainedIdentifier class can parse and return each
 section of a CQL identifier.  The possible sections are:

     class (optional)
     zero or more embedded objects
     property

Note:  this must handle wildcard("*"), embedded objects, arrays, symbolic
constants, etc.
   */
class CQLChainedIdentifierRep : public QueryChainedIdentifierRep
{
  public:

    CQLChainedIdentifierRep();
    /**  constructor
          Parses the string into the various components of a CQL identifier.
          Throws parsing errors.
      */
    CQLChainedIdentifierRep(const String& inString);

    CQLChainedIdentifierRep(const CQLIdentifier &id);

    CQLChainedIdentifierRep(const CQLChainedIdentifierRep* rep);

    ~CQLChainedIdentifierRep();

    CQLIdentifier operator[](Uint32 index)const;

    Array<CQLIdentifier> getSubIdentifiers()const;

    CQLIdentifier getLastIdentifier()const;

    friend class CQLFactory;

  private:

    CQLChainedIdentifierRep(const CQLChainedIdentifierRep& rhs);
    CQLChainedIdentifierRep& operator=(const CQLChainedIdentifierRep& rhs);

    void parse(const String& string);
};

PEGASUS_NAMESPACE_END

#endif /* Pegasus_CQLChainedIdentifierRep_h */
