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

#ifndef Pegasus_CQLIdentifierRep_h
#define Pegasus_CQLIdentifierRep_h

#include <Pegasus/Common/Config.h>
#include <Pegasus/CQL/Linkage.h>
#include <Pegasus/Common/Array.h>
#include <Pegasus/Common/CIMName.h>
#include <Pegasus/Query/QueryCommon/QueryIdentifierRep.h>

PEGASUS_NAMESPACE_BEGIN


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
class CQLIdentifierRep: public QueryIdentifierRep
{
public:
    CQLIdentifierRep();
    /**  The constructor for a CQLIdentifier object
          takes a const string reference as input.
          The string should contain the property portion of a
          CQLChainedIdentifier.

         The constructor parses the input string into the components of
         the property identifier.

         Throws parsing errors.
      */
    CQLIdentifierRep(const String& identifier);

    CQLIdentifierRep(const CQLIdentifierRep* rep);

    ~CQLIdentifierRep();

    CQLIdentifierRep& operator=(const CQLIdentifierRep& rhs);

private:

    /**   This method needs to continue to take a String object, not a
        reference nor a const reference.  The reason being it changes
        the value of the parameter and that changed value should not
        be returned to the caller.
    */

    void parse(String indentifier);

    static Char16 STAR;
    static Char16 HASH;
    static Char16 RBRKT;
    static Char16 LBRKT;
    static const char SCOPE[];
};

PEGASUS_NAMESPACE_END

#endif /* Pegasus_CQLIdentifier_h */
