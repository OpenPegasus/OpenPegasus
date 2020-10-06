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

#ifndef PEGASUS_CQLVALUEREP_H
#define PEGASUS_CQLVALUEREP_H


#include <Pegasus/Common/Config.h>
#include <Pegasus/Common/CIMInstance.h>
#include <Pegasus/Query/QueryCommon/QueryContext.h>
#include <Pegasus/Common/CIMObjectPath.h>
#include <Pegasus/CQL/CQLChainedIdentifier.h>
#include <Pegasus/CQL/CQLValue.h>
#include <Pegasus/Common/ArrayInternal.h>
#include <Pegasus/CQL/Linkage.h>

PEGASUS_NAMESPACE_BEGIN
class CQLFactory;

class CQLValueRep
{
  public:

    CQLValueRep();
    ~CQLValueRep();
    CQLValueRep(const CQLValueRep& val);
    CQLValueRep(const CQLValueRep* val);
    CQLValueRep(const String & inString, CQLValue::NumericType inValueType,
        Boolean inSign = true);

    /**  Initializes object as a CQLIdentifier.
       */
    CQLValueRep(const CQLChainedIdentifier& inCQLIdent);

   /**  Initializes object as a CIMObjectPath.
       */
    CQLValueRep(const CIMObjectPath& inObjPath);

   /**  Initializes object as a CIMDateTime.
       */
    CQLValueRep(const CIMDateTime& inDateTime);

    /**  Initializes object as a literal string (non-numeric).
       */
    CQLValueRep(const String& inString);

   CQLValueRep(Sint64 inSint);
   CQLValueRep(Uint64 inUint);
   CQLValueRep(Real64 inReal);
   CQLValueRep(const CIMInstance& inInstance);
   CQLValueRep(Boolean inBool);
   CQLValueRep(const CIMClass& inClass);
   CQLValueRep(const CIMObject& inObject);
   CQLValueRep(const CIMValue& inVal);

    /**  This method is used to ask an identifier to resolve itself to
           a number or string (primitive value).

          Resolves:
           - symbolic constants
           - indexing an array
           - decoding identifier
                * URI reference
                * embedded object (e.g. myclass.embeddedobject.prop1)
                    and this is recursive
           - class aliasing
     */

    void resolve(const CIMInstance& CI, const QueryContext& inQueryCtx);

    CQLValueRep& operator=(const CQLValueRep& rhs);

    /** operator == compares two CQLValue objects for equality and
                            performs type checking.
        @param x - First CQLValue to compare
        @param y - Second CQLValue to compare
        @return true if they are identical in type, attribute and value,
        false otherwise.
    */

    Boolean operator==(const CQLValueRep& x);

    /** operator == compares two CQLValue objects for inequality and
                            performs type checking.
        @param x - First CQLValue to compare
        @param y - Second CQLValue to compare
        @return true if they are NOT identical in type, attribute and value,
        false otherwise.
    */

    Boolean operator!=(const CQLValueRep& x);

    /** operator <= compares two CQLValue objects to see if x is less than or
                            equal to y and performs type checking.
        @param x - First CQLValue to compare
        @param y - Second CQLValue to compare
        @return true if x is less than or equal to y,
        false otherwise.
    */

    Boolean operator<=(const CQLValueRep& x);

    /** operator >= compares two CQLValue objects to see if x is greater than
                            or equal to y and performs type checking.
        @param x - First CQLValue to compare
        @param y - Second CQLValue to compare
        @return true if x is greater than or equal to y,
        false otherwise.
    */

    Boolean operator>=(const CQLValueRep& x);

    /** The overload operator (<) compares CQLValue obects and
          performs type checking.
        <pre>
            CQLValue   val1 = "def";
            CQLValue   val2 = "a";
            PEGASUS_ASSERT (val2 < val1);
        </pre>
    */

    Boolean operator<(const CQLValueRep& val1);

    /** The overload operator (>) compares CQLValue obects and
          performs type checking.
        <pre>
            CQLValue   val1 = "def";
            CQLValue   val2 = "a";
            PEGASUS_ASSERT (val2 < val1);
        </pre>
    */

    Boolean operator>(const CQLValueRep& val1);

    /** This overload operator (+) concatenates or adds the value of
    CQLValue objects and performs type checking and Uint64 handling.
          For example,
        <pre>
            CQLValue  t1 = "abc";  // it's a string type
            CQLValue  t2 = "def";   // it's another string type
            CQLValue   t3 = t1 + t2;
            PEGASUS_ASSERT(t3 == "abcdef");
        </pre>
    */

     CQLValueRep operator+(const CQLValueRep &x);

    /** This overload operator (-) subtracts the value of CQLValue
          objects and performs type checking and Uint64 handling.
          For example,
        <pre>
            CQLValue  t1 = 5;
            CQLValue  t2 = 2;
            CQLValue   t3 = t1 - t2;
            PEGASUS_ASSERT(t3 == 3);
        </pre>
    */

    //CQLValueRep operator-(const CQLValueRep& x);

    /** This overload operator (*) multiplies the values of CQLValue
          objects and performs type checking and Uint64 handling.
          For example,
        <pre>
            CQLValue  t1 = 3;
            CQLValue  t2 = 2;
            CQLValue   t3 = t1 *  t2;
            PEGASUS_ASSERT(t3 == 6);
        </pre>
    */

    //CQLValueRep operator*(const CQLValueRep& x);

    /** This overload operator (*) divides the values of CQLValue
          objects and performs type checking and Uint64 handling.
          For example,
        <pre>
            CQLValue  t1 = 6;
            CQLValue  t2 = 2;
            CQLValue   t3 = t1 /  t2;
            PEGASUS_ASSERT(t3 == 2);
        </pre>
    */

    //CQLValueRep operator/(const CQLValueRep& x);

    /** Accessor for getting the type of the CQLValue.

    TODO:  do we need this method?
      */

    CQLValue::CQLValueType getValueType();

    /** Returns whether the value has been resolved to a primitive.
      */

    Boolean isResolved();

    /** Returns whether the CQLValue is null.
      */

    Boolean isNull() const;

    /** Tests to see if this "isa" the input string.
    */

    Boolean isa(const CQLChainedIdentifier& inID, QueryContext& QueryCtx);

    /** Tests to see if this "like" the input string.
          Both sides of the LIKE comparison must have a String type:
          the result is a Boolean type.

         The LIKE comparison allows a string to be tested by pattern-matching,
          using special characters n the right-hand-side string.
          See the DMTF CQL Specification for details.

          For Basic Query, the left-hand expression (this) may be restricted to
    chain, and
          the right-hand expression may be restricted to be a literal-string.
    */

    Boolean like(const CQLValueRep& inVal);
    /*
   void invert();
    */
   CQLChainedIdentifier getChainedIdentifier()const;
   Uint64 getUint()const;
   Sint64 getSint()const;
   Real64 getReal()const;
   String getString()const;
   Boolean getBool()const;
   CIMDateTime getDateTime()const;
   CIMObjectPath getReference()const;
   CIMObject getObject()const;
   String toString()const;
   void applyContext(const QueryContext& _ctx,
                     const CQLChainedIdentifier& inCid);

   static String valueTypeToString(const CQLValue::CQLValueType parmType);
   friend class CQLFactory;
   friend class CQLValue;
  private:
   Boolean _areClassesInline(const CIMClass& c1,const CIMClass& c2,
                             QueryContext& in);
   void _validate(const CQLValueRep& x);
   void _resolveSymbolicConstant(const QueryContext& inQueryCtx);
   void _setValue(CIMValue cv, Sint64 Index = -1);
   void _process_value(CIMProperty& propObj,CQLIdentifier& _id,
                       const QueryContext& inQueryContext);
   Boolean _compareObjects(CIMObject& _in1, CIMObject& _in2);
   Boolean _compareArray(const CQLValueRep& _in);
   CQLValue::CQLValueType _getCQLType(const CIMType &type) const;
   CIMValue _theValue;

   CQLChainedIdentifier _CQLChainId;

   Boolean _isResolved;

   CQLValue::CQLValueType _valueType;

   String _ArrayType;
};

/*
#ifndef PEGASUS_ARRAY_T
#define PEGASUS_ARRAY_T CQLValueRep
#include <Pegasus/Common/ArrayInter.h>
#undef PEGASUS_ARRAY_T
#endif
*/

PEGASUS_NAMESPACE_END

#endif /* CQLVALUEREP_H */
