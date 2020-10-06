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
//%/////////////////////////////////////////////////////////////////////////////

#ifndef Pegasus_CQLChainedIdentifier_h
#define Pegasus_CQLChainedIdentifier_h

#include <Pegasus/CQL/Linkage.h>
//#include <Pegasus/Common/Array.h>
#include <Pegasus/Common/ArrayInternal.h>
#include <Pegasus/Query/QueryCommon/QueryChainedIdentifier.h>
#include <Pegasus/CQL/CQLIdentifier.h>

#ifdef PEGASUS_USE_EXPERIMENTAL_INTERFACES

PEGASUS_NAMESPACE_BEGIN
class CQLFactory;
class CQLChainedIdentifierRep;

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
class PEGASUS_CQL_LINKAGE CQLChainedIdentifier : public QueryChainedIdentifier
{
  public:

    CQLChainedIdentifier();
    /**
    Constructor
        Parses the string into the various components of a CQL identifier.
        @param  - inString.  The raw string to be parsed into a
                  CQLChainedIdentifier
        @return - None.
        @throws - None.
    */
    CQLChainedIdentifier(const String& inString);

    /**
    The constructor for a CQLChainedIdentifier object.

        @param  - id.  The CQLIdentifier to construct this object with.
        @return - None.
        @throws - None.
    */
    CQLChainedIdentifier(const CQLIdentifier & id);

    /**
        Copy Constructor

        @param  - cid. The CQLChainedIdentifier to construct this object with.
        @return - None.
        @throws - None.
    */
    CQLChainedIdentifier(const CQLChainedIdentifier& cid);

    /**
        Destructor

        @param  - None.
        @return - None.
        @throws - None.
    */
    ~CQLChainedIdentifier();

    /**
        Opertor=

        @param  - rhs. Right hand side of assignement operator.
        @return - *this.
        @throws - None.
    */
    CQLChainedIdentifier& operator=(const CQLChainedIdentifier& rhs);

    /**
        Operator[]. Allows indexed access into the CQLChainedIdentifier

        @param  - index. Location of data within the CQLChainedIdentifier
        @return - CQLIdentifier.  returns CQLIdentifer() if index is
                  out of bounds.
        @throws - None.
    */
    CQLIdentifier operator[](Uint32 index)const;

    /**
        Gets all of the CQLIdentifiers within the CQLChainedIdentifier

        @param  - None.
        @return - Array<CQLIdentifier>.
        @throws - None.
    */
    Array<CQLIdentifier> getSubIdentifiers()const;

    /**
        Gets the last CQLIdentifier in this objects Array of CQLIdentifiers

        @param  - None.
        @return - CQLIdentifier.
        @throws - None.
    */
    CQLIdentifier getLastIdentifier()const;

    friend class CQLFactory;

  private:

};

/*
#ifndef PEGASUS_ARRAY_T
#define PEGASUS_ARRAY_T CQLChainedIdentifier
#include <Pegasus/Common/ArrayInter.h>
#undef PEGASUS_ARRAY_T
#endif
*/

PEGASUS_NAMESPACE_END
#endif
#endif /* Pegasus_CQLChainedIdentifier_h */
