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
// Modified By: David Dillard, VERITAS Software Corp.
//                  (david.dillard@veritas.com)
//              Aruran, IBM(ashanmug@in.ibm.com) for Bug# 3589
//
//%/////////////////////////////////////////////////////////////////////////////

#ifndef Pegasus_CQLIdentifier_h
#define Pegasus_CQLIdentifier_h

#ifdef PEGASUS_USE_EXPERIMENTAL_INTERFACES

#include <Pegasus/CQL/Linkage.h>
//#include <Pegasus/Common/Array.h>
#include <Pegasus/Common/ArrayInternal.h>
#include <Pegasus/Query/QueryCommon/QueryIdentifier.h>

PEGASUS_NAMESPACE_BEGIN

class CQLIdentifierRep;

/**
  The CQLIdentifier class encapsulates
  the different formats of the CQL property portion
  of a CQLChainedIdentifier.

  For example, a CQLChainedIdentifier can have these parts:

    Class.EmbeddedObject.Property
    Class.Property


The "Property" portion of the CQLChainedIdentifier can be
in any of these 3 formats:
   (a)  property name
   (b)  property[3]     e.g. an array index
   (c)  property#'OK'    e.g. a symbolic constant
   (d)  *   (wildcard)

In the future, there may be support added for a set of indices (ranges).
  */
class PEGASUS_CQL_LINKAGE CQLIdentifier: public QueryIdentifier
{
  public:
    /**
        Default constructor

        @param  -  None.
        @return - None.
        @throws - None.
        <I><B>Experimental Interface</B></I><BR>
    */
    CQLIdentifier();

    /**  The constructor for a CQLIdentifier object
          takes a string as input.  The string should contain the
          property portion of a CQLChainedIdentifier.

         The constructor parses the input string into the components of
         the property identifier.

         @param - identifier.  The raw string to be parsed into a CQLIdentifier
        @return - None.
         @throws - CQLIdentifierParseException
         <I><B>Experimental Interface</B></I><BR>
      */
    CQLIdentifier(const String& identifier);

    /**
        Copy constructor

        @param - id.  The CQLIdentifier to copy construct from
        @return - None.
        @throws - None.
        <I><B>Experimental Interface</B></I><BR>
    */
    CQLIdentifier(const CQLIdentifier& id);

    /**
        Constructs a CQLIdentifier from its base class

        @param - identifier.  The raw string to be parsed into a CQLIdentifier
        @return - None.
        @throws - None.
        <I><B>Experimental Interface</B></I><BR>
    */
    CQLIdentifier(const QueryIdentifier& id);

    /**
        Destructor

        @param  - None.
        @return - None.
        @throws - None.
        <I><B>Experimental Interface</B></I><BR>
    */
    ~CQLIdentifier();

    /**
        Assignment operator

        @param  - rhs.  The right hand side of the assignment operator
        @return - None.
        @throws - None.
        <I><B>Experimental Interface</B></I><BR>
    */
    CQLIdentifier& operator=(const CQLIdentifier& rhs);

  private:

};


/*
#ifndef PEGASUS_ARRAY_T
#define PEGASUS_ARRAY_T CQLIdentifier
#include <Pegasus/Common/ArrayInter.h>
#undef PEGASUS_ARRAY_T
*/

PEGASUS_NAMESPACE_END
//#endif
#endif
#endif /* Pegasus_CQLIdentifier_h */
