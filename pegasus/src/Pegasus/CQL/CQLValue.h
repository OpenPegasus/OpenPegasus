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

#ifndef PEGASUS_CQLVALUE_H
#define PEGASUS_CQLVALUE_H

#ifdef PEGASUS_USE_EXPERIMENTAL_INTERFACES

#include <Pegasus/CQL/Linkage.h>
#include <Pegasus/Common/CIMInstance.h>
#include <Pegasus/Common/ArrayInternal.h>
#include <Pegasus/Common/String.h>
#include <Pegasus/Query/QueryCommon/QueryContext.h>
#include <Pegasus/Common/CIMObjectPath.h>
#include <Pegasus/CQL/CQLChainedIdentifier.h>

PEGASUS_NAMESPACE_BEGIN
class CQLFactory;
class CQLValueRep;

/** The CQLValue class encapulates a value
    that is a CQL value.  The possible CQLValue
    types are the following:

    Sint64
    Uint64
    String
    CIMDateTime
    CIMReference
    CQLIdentifier


    This class can resolve an identifier to a primitive
    value such as Sint64, Uint64 or String, CIMDateTime,
    and CIMReference.

    This class overloads and performs type checking
    on the following operators:
    <, >, =, >=. <=. <>


    NOTE:  the CQLValue class assumes a symbolic constant
    is fully qualified.

*/

class PEGASUS_CQL_LINKAGE CQLValue
{
public:

  enum NumericType  { Hex, Binary,  Decimal, Real};
  enum CQLValueType { Sint64_type, Uint64_type,
                      Real_type, String_type,
              CIMDateTime_type,  CIMReference_type, CQLIdentifier_type,
              CIMObject_type, Boolean_type};

  /**
      Constructs null CQLValue object with type Boolean
      (CQLValueType::Boolean_type).

      @param  None.
      @return None.
      @throw  None.
      <I><B>Experimental Interface</B></I><BR>
  */
  CQLValue();

  /**
      Destroys CQLValue object.

      @param  None.
      @return None.
      @throw  None.
      <I><B>Experimental Interface</B></I><BR>
  */
  ~CQLValue()

  /**
      Contructs CQLValue object (Copy-Constructor).

      @param  None.
      @return None.
      @throw  None.
      <I><B>Experimental Interface</B></I><BR>
  */;
  CQLValue(const CQLValue& val);

  /**
      Contructs CQLValue object given a string and a numbericType

      @param  inString The string representation of a number
      @param  inValueType The NumericType
      @return None.
      @throw  None.
      <I><B>Experimental Interface</B></I><BR>
  */
  CQLValue(const String& inString, NumericType inValueType,
       Boolean inSign = true);

  /**
      Contructs CQLValue object via CQLChainedIdentifier.

      @param  inCQLIdent The CQLChained Identifer used to create the object
      @return None.
      @throw  None.
      <I><B>Experimental Interface</B></I><BR>
  */
  CQLValue(const CQLChainedIdentifier& inCQLIdent);

  /**
      Initializes object as a CIMObjectPath.

      @param  inObjPath CIMObjectPath used to construct object.
      @return None.
      @throw  None.
      <I><B>Experimental Interface</B></I><BR>
  */
  CQLValue(const CIMObjectPath& inObjPath);

  /**
      Initializes object as a CIMDateTime.

      @param  inDateTime Datetime used to construct object.
      @return None.
      @throw  None.
      <I><B>Experimental Interface</B></I><BR>
  */
  CQLValue(const CIMDateTime& inDateTime);

  /**
      Initializes object as a literal string (non-numeric).

      @param  inString String used to construct object.
      @return None.
      @throw  None.
      <I><B>Experimental Interface</B></I><BR>
  */
  CQLValue(const String& inString);

  /**
      Initializes object as an Sint64.

      @param  inSint Sint64 used to contruct object.
      @return None.
      @throw  None.
      <I><B>Experimental Interface</B></I><BR>
  */
  CQLValue(Sint64 inSint);

  /**
      Initializes object as a Uint64

      @param  inUint Uint64 used to construct object.
      @return None.
      @throw  None.
      <I><B>Experimental Interface</B></I><BR>
  */
  CQLValue(Uint64 inUint);

  /**
      Initializes object as a Real64

      @param  inReal Real64 used to construct object.
      @return None.
      @throw  None.
      <I><B>Experimental Interface</B></I><BR>
  */
  CQLValue(Real64 inReal);

  /**
      Initializes object as a CIMInstance

      @param  inInstance CIMInstance used to construct object.
      @return None.
      @throw  None.
      <I><B>Experimental Interface</B></I><BR>
  */
  CQLValue(const CIMInstance& inInstance);

  /**
      Initializes object as a Boolean

      @param  inBool inBool used to construct object.
      @return None.
      @throw  None.
      <I><B>Experimental Interface</B></I><BR>
  */
  CQLValue(Boolean inBool);

  /**
      Initializes object as a inClass

      @param  inClass CIMClass used to construct object.
      @return None.
      @throw  None.
      <I><B>Experimental Interface</B></I><BR>
  */
  CQLValue(const CIMClass& inClass);

  /**
      Initializes object as a CQLValueRep

      @param  rhs CQLValueRep used to construct object.
      @return None.
      @throw  None.
      <I><B>Experimental Interface</B></I><BR>
  */
  CQLValue(const CQLValueRep& rhs);

  /**
      Initializes object as a CIMObject

      @param  inObject CIMObject used to construct object.
      @return None.
      @throw  None.
      <I><B>Experimental Interface</B></I><BR>
  */
  CQLValue(const CIMObject& inObject);

  /**
      Initializes object as a CIMValue

      @param  inVal CIMValue used to construct object.
      @return None.
      @throw  None.
      <I><B>Experimental Interface</B></I><BR>
  */
  CQLValue(const CIMValue& inVal);

  /**  This method is used to ask an identifier to resolve itself to
       a primitive value.

       Resolves:
       - symbolic constants
       - indexing an array
       - decoding identifier
       * embedded object (e.g. myclass.embeddedobject.prop1)
       and this is recursive
       - class aliasing

      @param  CI CIMInstance where information is retrieved.
      @param  inQueryCtx QueryContext contains query data.
      @return None.
      @throw  CQLRuntimeException.
      <I><B>Experimental Interface</B></I><BR>
  */
  void resolve(const CIMInstance& CI,const  QueryContext& inQueryCtx);

  /**
      Assignment operator.

      @param  rhs right hand side of operator.
      @return Reference to this object.
      @throw  None.
      <I><B>Experimental Interface</B></I><BR>
  */
  CQLValue& operator=(const CQLValue& rhs);

  /**
      operator == compares two CQLValue objects for equality and
      performs type checking.

      @param  rhs right hand side of operator.
      @return Boolean.
      @throw  None.
      <I><B>Experimental Interface</B></I><BR>
  */
  Boolean operator==(const CQLValue& x) const;

  /**
      operator != compares two CQLValue objects for inequality and
      performs type checking.

      @param  x Right hand side of operator.
      @return Boolean.
      @throw  None.
      <I><B>Experimental Interface</B></I><BR>
  */
  Boolean operator!=(const CQLValue& x) const;

  /**
      operator <= compares two CQLValue objects to see if leftside is
      less than or equal to right side and performs type checking.

      @param  x Right hand side of operator.
      @return Boolean.
      @throw  None.
      <I><B>Experimental Interface</B></I><BR>
  */
  Boolean operator<=(const CQLValue& x) const;

  /**
      operator >= compares two CQLValue objects to see if leftside
      is greater than or equal to rightside and performs type checking.

      @param  x Right hand side of operator.
      @return Boolean.
      @throw  None.
      <I><B>Experimental Interface</B></I><BR>
  */
  Boolean operator>=(const CQLValue& x) const;

  /**
      The overload operator < compares CQLValue obects and
      performs type checking.

      @param  val1 Right hand side of operator.
      @return Boolean.
      @throw  None.
      <I><B>Experimental Interface</B></I><BR>
  */
  Boolean operator<(const CQLValue& val1) const;

  /**
      The overload operator > compares CQLValue obects and
      performs type checking.

      @param  val1 Right hand side of operator.
      @return Boolean.
      @throw  None.
      <I><B>Experimental Interface</B></I><BR>
  */
  Boolean operator>(const CQLValue& val1) const;

  /** This overload operator (+) concatenates the value of
      CQLValue objects and performs type checking.
      (Currently supports only CQLValue of String type.)

      @param  x Right hand side of operator.
      @return Boolean.
      @throw  None.
      <I><B>Experimental Interface</B></I><BR>
  */
  CQLValue operator+(const CQLValue& x);

  /**
      Accessor for getting the type of the CQLValue.

      @param  None.
      @return CQLValueType.
      @throw  None.
      <I><B>Experimental Interface</B></I><BR>
  */
  CQLValueType getValueType() const;

  /**
      Returns whether the value has been resolved to a primitive.

      @param  None.
      @return True when value is a primitive.
      @throw  None.
      <I><B>Experimental Interface</B></I><BR>
  */
  Boolean isResolved() const;

  /**
      Returns whether the CQLValue is null.

      @param  None.
      @return True if CQLValue primitive value is NULL.
      @throw  None.
      <I><B>Experimental Interface</B></I><BR>
  */
  Boolean isNull() const;

  /** This will test to determine if the CQLValue isa String contained
      in the cid, that identifies a class. The CQLValue must be of a
      type Instance or Class; Also the cid must contain a String that
      is a valid class name.

      @param  cid Contains the class name.
      @param  QueryCtx Contains query related data.
      @return true CQLValue isa class identified by cid.
      @throw  None.
      <I><B>Experimental Interface</B></I><BR>
  */
  Boolean isa(const CQLChainedIdentifier& cid, QueryContext& QueryCtx) const;

  /** Tests to see if this "like" the input string.
      Both sides of the LIKE comparison must have a String type:
      the result is a Boolean type.

      The LIKE comparison allows a string to be tested by pattern-matching,
      using special characters n the right-hand-side string.
      See the DMTF CQL Specification for details.

      For Basic Query, the left-hand expression (this) may be restricted to
      chain, and
      the right-hand expression may be restricted to be a literal-string.

      @param  inVal CQLValue to be compared.
      @return true CQLValue is like the passed in CQLValue.
      @throw  None.
      <I><B>Experimental Interface</B></I><BR>
  */
  Boolean like(const CQLValue& inVal) const;

  /**
      Get the ChainedIdentifier. Primitive from CQLValue

      @param  None.
      @return ChainedIdentifier.
      @throw  CQLRuntimeException.
      <I><B>Experimental Interface</B></I><BR>
  */
  CQLChainedIdentifier getChainedIdentifier() const;

  /**
      Get the Uint64.  Primitive from CQLValue

      @param  None.
      @return Uint64.
      @throw  CQLRuntimeException.
      <I><B>Experimental Interface</B></I><BR>
  */
  Uint64 getUint() const;

  /**
      Get the Sint64. Primitive from CQLValue.

      @param  None.
      @return Sint64.
      @throw  CQLRuntimeException.
      <I><B>Experimental Interface</B></I><BR>
  */
  Sint64 getSint() const;

  /**
      Get the Real64. Primitive from CQLValue.

      @param  None.
      @return Real64.
      @throw  CQLRuntimeException.
      <I><B>Experimental Interface</B></I><BR>
  */
  Real64 getReal() const;

  /**
      Get the String. Primitive from CQLValue

      @param  None.
      @return String.
      @throw  CQLRuntimeException.
      <I><B>Experimental Interface</B></I><BR>
  */
  String getString() const;

  /**
      Get the Boolean. Primitive from CQLValue

      @param  None.
      @return Boolean
      @throw  CQLRuntimeException.
      <I><B>Experimental Interface</B></I><BR>
  */
  Boolean getBool() const;

  /**
      Get the CIMDateTime. Primitive from CQLValue.

      @param  None.
      @return CIMDateTime
      @throw  CQLRuntimeException.
      <I><B>Experimental Interface</B></I><BR>
  */
  CIMDateTime getDateTime() const;

  /**
      Get the CIMObjectPath. Primitive from CQLValue.

      @param  None.
      @return Reference.
      @throw  CQLRuntimeException.
      <I><B>Experimental Interface</B></I><BR>
  */
  CIMObjectPath getReference() const;

  /**
      Get the CIMObject. Primitive from CQLValue.

      @param  None.
      @return CIMObject.
      @throw  CQLRuntimeException.
      <I><B>Experimental Interface</B></I><BR>
  */
  CIMObject getObject() const;

  /**
      Return String representation of object.

      @param  None.
      @return String
      @throw  None.
      <I><B>Experimental Interface</B></I><BR>
  */
  String toString() const;

  /**
      Apply scope and class to CQLValue's chainedIdentifier.

      @param  None.
      @return None.
      @throw  None.
      <I><B>Experimental Interface</B></I><BR>
  */
  void applyContext(const QueryContext& _ctx,
            const CQLChainedIdentifier& inCid);

  friend class CQLFactory;

private:

  CQLValueRep *_rep;
};

/*
#ifndef PEGASUS_ARRAY_T
#define PEGASUS_ARRAY_T CQLValue
#include <Pegasus/Common/ArrayInter.h>
#undef PEGASUS_ARRAY_T
#endif
*/

PEGASUS_NAMESPACE_END
#endif
#endif /* CQLVALUE_H */
