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

#ifndef Pegasus_Value_h
#define Pegasus_Value_h

#include <Pegasus/Common/String.h>
#include <Pegasus/Common/Config.h>
#include <Pegasus/Common/CIMType.h>
#include <Pegasus/Common/CIMObjectPath.h>
#include <Pegasus/Common/CIMObject.h>
#include <Pegasus/Common/CIMDateTime.h>
#include <Pegasus/Common/Exception.h>
#include <Pegasus/Common/Array.h>
#include <Pegasus/Common/Linkage.h>

PEGASUS_NAMESPACE_BEGIN

class CIMValueRep;
class CIMObject;
class CIMInstance;

/**
    The CIMValue class represents a value of any of the CIM data types.
    This class encapsulates a union which holds value of any CIMType.
    A type field indicates the type of the value.

    WARNING: The returned value of CIMValue get(returnValue) is never nodified
    when a CIMValue is null (i.e. isNull() == true). An isNull() test should
    be executed before executing any CIMValue get(...) if there is any
    question of a possible null value and you are not sure of the value in
    the returned variable.

*/
class PEGASUS_COMMON_LINKAGE CIMValue
{
public:
    /**
        Constructs a null CIMValue with type Boolean and isArray=false.
    */
    CIMValue();

    /**
        Constructs a null CIMValue object with the specified type and
        array indicator.
        @param type The CIMType of the value
        @param isArray A Boolean indicating whether the value is of array type
        @param arraySize An integer specifying the size of the array, if the
            CIMValue is for an array of a fixed size.  This value is ignored
            if the isArray argument is false.
    */
    CIMValue(CIMType type, Boolean isArray, Uint32 arraySize = 0);

    /**
        Constructs a CIMValue with the specified value and inferred type.
        @param x The Boolean value to assign
    */
    CIMValue(Boolean x);

    /**
        Constructs a CIMValue with the specified value and inferred type.
        @param x The Uint8 value to assign
    */
    CIMValue(Uint8 x);

    /**
        Constructs a CIMValue with the specified value and inferred type.
        @param x The Sint8 value to assign
    */
    CIMValue(Sint8 x);

    /**
        Constructs a CIMValue with the specified value and inferred type.
        @param x The Uint16 value to assign
    */
    CIMValue(Uint16 x);

    /**
        Constructs a CIMValue with the specified value and inferred type.
        @param x The Sint16 value to assign
    */
    CIMValue(Sint16 x);

    /**
        Constructs a CIMValue with the specified value and inferred type.
        @param x The Uint32 value to assign
    */
    CIMValue(Uint32 x);

    /**
        Constructs a CIMValue with the specified value and inferred type.
        @param x The Sint32 value to assign
    */
    CIMValue(Sint32 x);

    /**
        Constructs a CIMValue with the specified value and inferred type.
        @param x The Uint64 value to assign
    */
    CIMValue(Uint64 x);

    /**
        Constructs a CIMValue with the specified value and inferred type.
        @param x The Sint64 value to assign
    */
    CIMValue(Sint64 x);

    /**
        Constructs a CIMValue with the specified value and inferred type.
        @param x The Real32 value to assign
    */
    CIMValue(Real32 x);

    /**
        Constructs a CIMValue with the specified value and inferred type.
        @param x The Real64 value to assign
    */
    CIMValue(Real64 x);

    /**
        Constructs a CIMValue with the specified value and inferred type.
        @param x The Char16 value to assign
    */
    CIMValue(const Char16& x);

    /**
        Constructs a CIMValue with the specified value and inferred type.
        @param x The String value to assign
    */
    CIMValue(const String& x);

    /**
        Constructs a CIMValue with the specified value and inferred type.
        @param x The CIMDateTime value to assign
    */
    CIMValue(const CIMDateTime& x);

    /**
        Constructs a CIMValue with the specified value and inferred type.
        @param x The CIMObjectPath value to assign
    */
    CIMValue(const CIMObjectPath& x);

    /**
        Constructs a CIMValue with the specified value and inferred type.
        Note: The CIMObject argument is cloned to prevent subsequent
        modification through the shared representation model.
        @param x The CIMObject value to assign
        @exception UninitializedObjectException If the CIMObject is
            uninitialized.
    */
    CIMValue(const CIMObject& x);

    /**
        Constructs a CIMValue with the specified value and inferred type.
        Note: The CIMInstance argument is cloned to prevent subsequent
        modification through the shared representation model.
        @param x The CIMInstance value to assign
        @exception UninitializedObjectException If the CIMInstance is
            uninitialized.
    */
    CIMValue(const CIMInstance& x);

    /**
        Constructs a CIMValue with the specified value and inferred type.
        @param x The Boolean Array value to assign
    */
    CIMValue(const Array<Boolean>& x);

    /**
        Constructs a CIMValue with the specified value and inferred type.
        @param x The Uint8 Array value to assign
    */
    CIMValue(const Array<Uint8>& x);

    /**
        Constructs a CIMValue with the specified value and inferred type.
        @param x The Sint8 Array value to assign
    */
    CIMValue(const Array<Sint8>& x);

    /**
        Constructs a CIMValue with the specified value and inferred type.
        @param x The Uint16 Array value to assign
    */
    CIMValue(const Array<Uint16>& x);

    /**
        Constructs a CIMValue with the specified value and inferred type.
        @param x The Sint16 Array value to assign
    */
    CIMValue(const Array<Sint16>& x);

    /**
        Constructs a CIMValue with the specified value and inferred type.
        @param x The Uint32 Array value to assign
    */
    CIMValue(const Array<Uint32>& x);

    /**
        Constructs a CIMValue with the specified value and inferred type.
        @param x The Sint32 Array value to assign
    */
    CIMValue(const Array<Sint32>& x);

    /**
        Constructs a CIMValue with the specified value and inferred type.
        @param x The Uint64 Array value to assign
    */
    CIMValue(const Array<Uint64>& x);

    /**
        Constructs a CIMValue with the specified value and inferred type.
        @param x The Sint64 Array value to assign
    */
    CIMValue(const Array<Sint64>& x);

    /**
        Constructs a CIMValue with the specified value and inferred type.
        @param x The Real32 Array value to assign
    */
    CIMValue(const Array<Real32>& x);

    /**
        Constructs a CIMValue with the specified value and inferred type.
        @param x The Real64 Array value to assign
    */
    CIMValue(const Array<Real64>& x);

    /**
        Constructs a CIMValue with the specified value and inferred type.
        @param x The Char16 Array value to assign
    */
    CIMValue(const Array<Char16>& x);

    /**
        Constructs a CIMValue with the specified value and inferred type.
        @param x The String Array value to assign
    */
    CIMValue(const Array<String>& x);

    /**
        Constructs a CIMValue with the specified value and inferred type.
        @param x The CIMDateTime Array value to assign
    */
    CIMValue(const Array<CIMDateTime>& x);

    /**
        Constructs a CIMValue with the specified value and inferred type.
        @param x The CIMObjectPath Array value to assign
    */
    CIMValue(const Array<CIMObjectPath>& x);

    /**
        Constructs a CIMValue with the specified value and inferred type.
        Note: The CIMObjects in the Array argument are cloned to prevent
        subsequent modification through the shared representation model.
        @param x The CIMObject Array value to assign
        @exception UninitializedObjectException If any of the CIMObjects in the
            Array are uninitialized.
    */
    CIMValue(const Array<CIMObject>& x);

    /**
        Constructs a CIMValue with the specified value and inferred type.
        Note: The CIMInstances in the Array argument are cloned to prevent
        subsequent modification through the shared representation model.
        @param x The CIMInstance Array value to assign
        @exception UninitializedObjectException If any of the CIMInstances in
            the Array are uninitialized.
    */
    CIMValue(const Array<CIMInstance>& x);

    /**
        Constructs a CIMValue by copying another CIMValue object.
        Note: If the specified CIMValue contains CIMObject or CIMInstance
        objects, they are cloned to prevent subsequent modification through
        the shared representation model.
        @param x The CIMValue object to copy
    */
    CIMValue(const CIMValue& x);

    /**
        Destructs a CIMValue object.
    */
    ~CIMValue();

    /**
        Assigns the value from a specified CIMValue object.
        Note: If the specified CIMValue contains CIMObject or CIMInstance
        objects, they are cloned to prevent subsequent modification through
        the shared representation model.
        @param x The CIMValue object to copy
        @return A reference to this CIMValue object with the new assignment
    */
    CIMValue& operator=(const CIMValue& x);

    /**
        Assigns the value from a specified CIMValue object.
        Note: If the specified CIMValue contains CIMObject or CIMInstance
        objects, they are cloned to prevent subsequent modification through
        the shared representation model.
        @param x The CIMValue object to copy
    */
    void assign(const CIMValue& x);

    /**
        Resets to a null value with type Boolean and isArray=false.
    */
    void clear();

    /**
        Compares the type and isArray attributes with a specified CIMValue.
        @param x The CIMValue object with which to compare
        @return A Boolean indicating whether the CIMValue objects have the same
            type and isArray attribute value.
    */
    Boolean typeCompatible(const CIMValue& x) const;

    /**
        Indicates whether the value is an array.
        @return A Boolean indicating whether the value is an array.
    */
    Boolean isArray() const;

    /**
        Indicates whether the value is null.  A null CIMValue has a type, but
        no value can be retrieved from it.
        @return A Boolean indicating whether the value is null.
    */
    Boolean isNull() const;

    /**
        Gets the fixed array size of a CIMValue.  This value is undefined for
        non-array values.  A value of 0 is given for variable size arrays.
        Result is undetermined if CIMValue is null.
        @return An integer indicating the array size.
    */
    Uint32 getArraySize() const;

    /**
        Gets the value type.
        @return A CIMType indicating the type of the value.
    */
    CIMType getType() const;

    /**
        Sets the value to null with the specified type and array indicator.
        @param type The CIMType of the value
        @param isArray A Boolean indicating whether the value is of array type
        @param arraySize An integer specifying the size of the array, if the
            CIMValue is for an array of a fixed size.  This value is ignored
            if the isArray argument is false.
    */
    void setNullValue(CIMType type, Boolean isArray, Uint32 arraySize = 0);

    /**
        Sets the CIMValue to the specified value and inferred type.
        @param x The Boolean value to assign
    */
    void set(Boolean x);

    /**
        Sets the CIMValue to the specified value and inferred type.
        @param x The Uint8 value to assign
    */
    void set(Uint8 x);

    /**
        Sets the CIMValue to the specified value and inferred type.
        @param x The Sint8 value to assign
    */
    void set(Sint8 x);

    /**
        Sets the CIMValue to the specified value and inferred type.
        @param x The Uint16 value to assign
    */
    void set(Uint16 x);

    /**
        Sets the CIMValue to the specified value and inferred type.
        @param x The Sint16 value to assign
    */
    void set(Sint16 x);

    /**
        Sets the CIMValue to the specified value and inferred type.
        @param x The Uint32 value to assign
    */
    void set(Uint32 x);

    /**
        Sets the CIMValue to the specified value and inferred type.
        @param x The Sint32 value to assign
    */
    void set(Sint32 x);

    /**
        Sets the CIMValue to the specified value and inferred type.
        @param x The Uint64 value to assign
    */
    void set(Uint64 x);

    /**
        Sets the CIMValue to the specified value and inferred type.
        @param x The Sint64 value to assign
    */
    void set(Sint64 x);

    /**
        Sets the CIMValue to the specified value and inferred type.
        @param x The Real32 value to assign
    */
    void set(Real32 x);

    /**
        Sets the CIMValue to the specified value and inferred type.
        @param x The Real64 value to assign
    */
    void set(Real64 x);

    /**
        Sets the CIMValue to the specified value and inferred type.
        @param x The Char16 value to assign
    */
    void set(const Char16& x);

    /**
        Sets the CIMValue to the specified value and inferred type.
        @param x The String value to assign
    */
    void set(const String& x);

    /**
        Sets the CIMValue to the specified value and inferred type.
        @param x The CIMDateTime value to assign
    */
    void set(const CIMDateTime& x);

    /**
        Sets the CIMValue to the specified value and inferred type.
        @param x The CIMObjectPath value to assign
    */
    void set(const CIMObjectPath& x);

    /**
        Sets the CIMValue to the specified value and inferred type.
        Note: The CIMObject argument is cloned to prevent subsequent
        modification through the shared representation model.
        @param x The CIMObject value to assign
        @exception UninitializedObjectException If the CIMObject is
            uninitialized.
    */
    void set(const CIMObject& x);

    /**
        Sets the CIMValue to the specified value and inferred type.
        Note: The CIMInstance argument is cloned to prevent subsequent
        modification through the shared representation model.
        @param x The CIMInstance value to assign
        @exception UninitializedObjectException If the CIMInstance is
            uninitialized.
    */
    void set(const CIMInstance& x);

    /**
        Sets the CIMValue to the specified value and inferred type.
        @param x The Boolean Array value to assign
    */
    void set(const Array<Boolean>& x);

    /**
        Sets the CIMValue to the specified value and inferred type.
        @param x The Uint8 Array value to assign
    */
    void set(const Array<Uint8>& x);

    /**
        Sets the CIMValue to the specified value and inferred type.
        @param x The Sint8 Array value to assign
    */
    void set(const Array<Sint8>& x);

    /**
        Sets the CIMValue to the specified value and inferred type.
        @param x The Uint16 Array value to assign
    */
    void set(const Array<Uint16>& x);

    /**
        Sets the CIMValue to the specified value and inferred type.
        @param x The Sint16 Array value to assign
    */
    void set(const Array<Sint16>& x);

    /**
        Sets the CIMValue to the specified value and inferred type.
        @param x The Uint32 Array value to assign
    */
    void set(const Array<Uint32>& x);

    /**
        Sets the CIMValue to the specified value and inferred type.
        @param x The Sint32 Array value to assign
    */
    void set(const Array<Sint32>& x);

    /**
        Sets the CIMValue to the specified value and inferred type.
        @param x The Uint64 Array value to assign
    */
    void set(const Array<Uint64>& x);

    /**
        Sets the CIMValue to the specified value and inferred type.
        @param x The Sint64 Array value to assign
    */
    void set(const Array<Sint64>& x);

    /**
        Sets the CIMValue to the specified value and inferred type.
        @param x The Real32 Array value to assign
    */
    void set(const Array<Real32>& x);

    /**
        Sets the CIMValue to the specified value and inferred type.
        @param x The Real64 Array value to assign
    */
    void set(const Array<Real64>& x);

    /**
        Sets the CIMValue to the specified value and inferred type.
        @param x The Char16 Array value to assign
    */
    void set(const Array<Char16>& x);

    /**
        Sets the CIMValue to the specified value and inferred type.
        @param x The String Array value to assign
    */
    void set(const Array<String>& x);

    /**
        Sets the CIMValue to the specified value and inferred type.
        @param x The CIMDateTime Array value to assign
    */
    void set(const Array<CIMDateTime>& x);

    /**
        Sets the CIMValue to the specified value and inferred type.
        @param x The CIMObjectPath Array value to assign
    */
    void set(const Array<CIMObjectPath>& x);

    /**
        Sets the CIMValue to the specified value and inferred type.
        Note: The CIMObjects in the Array argument are cloned to prevent
        subsequent modification through the shared representation model.
        @param x The CIMObject Array value to assign
        @exception UninitializedObjectException If any of the CIMObjects in the
            Array are uninitialized.
    */
    void set(const Array<CIMObject>& x);

    /**
        Sets the CIMValue to the specified value and inferred type.
        Note: The CIMInstances in the Array argument are cloned to prevent
        subsequent modification through the shared representation model.
        @param x The CIMInstance Array value to assign
        @exception UninitializedObjectException If any of the CIMInstances in
            the Array are uninitialized.
    */
    void set(const Array<CIMInstance>& x);

    /**
        Gets the value of the CIMValue.  The caller should first verify that
        the value is not null and may verify type to avoid the possibility of
        the TypeMismatchException.  The output parameter x is NOT
        updated if the value is null (isNull() == true).
        <pre>
            // get with complete checking on type and nullness
            Boolean v;
            CIMValue value = property.getValue();
            if ((value.getType() == CIMTYPE_BOOLEAN) && (!value.isNull()))
                value.get(v);
            or
            // Set the value from property into v if the value is
            // of Boolean type and !isNull(). If type incorrect, generates
            // an exception.  If CIMValue is null, leaves v false. Else
            // sets Boolean value from property into v.
            Boolean v = false;
            property.getValue().get(v);
        </pre>
        @param x Output Boolean into which the value is stored if CIMValue
            state is not null.
        @exception TypeMismatchException If the value type does not match the
            output parameter.
    */
    void get(Boolean& x) const;

    /**
        Gets the value of the CIMValue.  The caller should first verify that
        the value is not null and may verify type to avoid the possibility of
        the TypeMismatchException.  The output parameter x is NOT
        updated if the value is null (isNull() == true).

        @param x Output Uint8 into which the value is stored if CIMValue
            state is not null.
        @exception TypeMismatchException If the value type does not match the
            output parameter.
    */
    void get(Uint8& x) const;

    /**
        Gets the value of the CIMValue.  The caller should first verify that
        the value is not null and may verify type to avoid the possibility of
        the TypeMismatchException.  The output parameter x is NOT
        updated if the value is null (isNull() == true).
        @param x Output Sint8 into which the value is stored if CIMValue
            state is not null.
        @exception TypeMismatchException If the value type does not match the
            output parameter.
    */
    void get(Sint8& x) const;

    /**
        Gets the value of the CIMValue.  The caller should first verify that
        the value is not null and may verify type to avoid the possibility of
        the TypeMismatchException.  The output parameter x is NOT
        updated if the value is null (isNull() == true).
        @param x Output Uint16 into which the value is stored if CIMValue
            state is not null.
        @exception TypeMismatchException If the value type does not match the
            output parameter.
    */
    void get(Uint16& x) const;

    /**
        Gets the value of the CIMValue.  The caller should first verify that
        the value is not null and may verify type to avoid the possibility of
        the TypeMismatchException.  The output parameter x is NOT
        updated if the value is null (isNull() == true).
        @param x Output Sint16 into which the value is stored if CIMValue
            state is not null.
        @exception TypeMismatchException If the value type does not match the
            output parameter.
    */
    void get(Sint16& x) const;

    /**
        Gets the value of the CIMValue.  The caller should first verify that
        the value is not null and may verify type to avoid the possibility of
        the TypeMismatchException.  The output parameter x is NOT
        updated if the value is null (isNull() == true).
        <pre>
            Uint32 v;
            CIMValue value = property.getValue();
            if ((value.getType() == CIMTYPE_SINT32) && (!value.isNull()))
                value.get(v);
        </pre>
        @param x Output Uint32 into which the value is stored if CIMValue
            state is not null.
        @exception TypeMismatchException If the value type does not match the
            output parameter.
    */
    void get(Uint32& x) const;

    /**
        Gets the value of the CIMValue.  The caller should first verify that
        the value is not null and may verify type to avoid the possibility of
        the TypeMismatchException.  The output parameter x is NOT
        updated if the value is null (isNull() == true).
        @param x Output Sint32 into which the value is stored if CIMValue
            state is not null.
        @exception TypeMismatchException If the value type does not match the
            output parameter.
    */
    void get(Sint32& x) const;

    /**
        Gets the value of the CIMValue.  The caller should first verify that
        the value is not null and may verify type to avoid the possibility of
        the TypeMismatchException.  The output parameter x is NOT
        updated if the value is null (isNull() == true).
        @param x Output Uint64 into which the value is stored if CIMValue
            state is not null.
        @exception TypeMismatchException If the value type does not match the
            output parameter.
    */
    void get(Uint64& x) const;

    /**
        Gets the value of the CIMValue.  The caller should first verify that
        the value is not null and may verify type to avoid the possibility of
        the TypeMismatchException.  The output parameter x is NOT
        updated if the value is null (isNull() == true).
        @param x Output Sint64 into which the value is stored if CIMValue
            state is not null.
        @exception TypeMismatchException If the value type does not match the
            output parameter.
    */
    void get(Sint64& x) const;

    /**
        Gets the value of the CIMValue.  The caller should first verify that
        the value is not null and may verify type to avoid the possibility of
        the TypeMismatchException.  The output parameter x is NOT
        updated if the value is null (isNull() == true).
        @param x Output Real32 into which the value is stored if CIMValue
            state is not null.
        @exception TypeMismatchException If the value type does not match the
            output parameter.
    */
    void get(Real32& x) const;

    /**
        Gets the value of the CIMValue.  The caller should first verify that
        the value is not null and may verify type to avoid the possibility of
        the TypeMismatchException.  The output parameter x is NOT
        updated if the value is null (isNull() == true).
        @param x Output Real64 into which the value is stored if CIMValue
            state is not null.
        @exception TypeMismatchException If the value type does not match the
            output parameter.
    */
    void get(Real64& x) const;

    /**
        Gets the value of the CIMValue.  The caller should first verify that
        the value is not null and may verify type to avoid the possibility of
        the TypeMismatchException.  The output parameter x is NOT
        updated if the value is null (isNull() == true).
        @param x Output Char16 into which the value is stored if CIMValue
            state is not null.
        @exception TypeMismatchException If the value type does not match the
            output parameter.
    */
    void get(Char16& x) const;

    /**
        Gets the value of the CIMValue.  The caller should first verify that
        the value is not null and may verify type to avoid the possibility of
        the TypeMismatchException.  The output parameter x is NOT
        updated if the value is null (isNull() == true).
        @param x Output String into which the value is stored if CIMValue
            state is not null.
        @exception TypeMismatchException If the value type does not match the
            output parameter.
    */
    void get(String& x) const;

    /**
        Gets the value of the CIMValue.  The caller should first verify that
        the value is not null and may verify type to avoid the possibility of
        the TypeMismatchException.  The output parameter x is NOT
        updated if the value is null (isNull() == true).
        @param x Output CIMDateTime into which the value is stored if CIMValue
            state is not null.
        @exception TypeMismatchException If the value type does not match the
            output parameter.
    */
    void get(CIMDateTime& x) const;

    /**
        Gets the value of the CIMValue.  The caller should first verify that
        the value is not null and may verify type to avoid the possibility of
        the TypeMismatchException.  The output parameter x is NOT
        updated if the value is null (isNull() == true).).
        @param x Output CIMObjectPath into which the value is stored if
            CIMValue state is not null.
        @exception TypeMismatchException If the value type does not match the
            output parameter.
    */
    void get(CIMObjectPath& x) const;

    /**
        Gets the value of the CIMValue.  The caller should first verify that
        the value is not null and may verify type to avoid the possibility of
        the TypeMismatchException.  The output parameter x is NOT
        updated if the value is null (isNull() == true).
        @param x Output CIMObject into which the value is stored if CIMValue
            state is not null.
        @exception TypeMismatchException If the value type does not match the
            output parameter.
    */
    void get(CIMObject& x) const;

    /**
        Gets the value of the CIMValue.  The caller should first verify that
        the value is not null and may verify type to avoid the possibility of
        the TypeMismatchException.  The output parameter x is NOT
        updated if the value is null (isNull() == true).
        @param x Output CIMInstance into which the value is stored if CIMValue
            state is not null.
        @exception TypeMismatchException If the value type does not match the
            output parameter.
    */
    void get(CIMInstance& x) const;

    /**
        Gets the value of the CIMValue.  The caller should first verify that
        the value is not null and may verify type to avoid the possibility of
        the TypeMismatchException.  The output parameter x is NOT
        updated if the value is null (isNull() == true).
        @param x Output Boolean Array into which the value is stored if
            CIMValue state is not null.
        @exception TypeMismatchException If the value type does not match the
            output parameter.
    */
    void get(Array<Boolean>& x) const;

    /**
        Gets the value of the CIMValue.  The caller should first verify that
        the value is not null and may verify type to avoid the possibility of
        the TypeMismatchException.  The output parameter x is NOT
        updated if the value is null (isNull() == true).
        @param x Output Uint8 Array into which the value is stored if CIMValue
            state is not null.
        @exception TypeMismatchException If the value type does not match the
            output parameter.
    */
    void get(Array<Uint8>& x) const;

    /**
        Gets the value of the CIMValue.  The caller should first verify that
        the value is not null and may verify type to avoid the possibility of
        the TypeMismatchException.  The output parameter x is NOT
        updated if the value is null (isNull() == true).
        @param x Output Sint8 Array into which the value is stored if CIMValue
            state is not null.
        @exception TypeMismatchException If the value type does not match the
            output parameter.
    */
    void get(Array<Sint8>& x) const;

    /**
        Gets the value of the CIMValue.  The caller should first verify that
        the value is not null and may verify type to avoid the possibility of
        the TypeMismatchException.  The output parameter x is NOT
        updated if the value is null (isNull() == true).).
        @param x Output Uint16 Array into which the value is stored if CIMValue
            state is not null.
        @exception TypeMismatchException If the value type does not match the
            output parameter.
    */
    void get(Array<Uint16>& x) const;

    /**
        Gets the value of the CIMValue.  The caller should first verify that
        the value is not null and may verify type to avoid the possibility of
        the TypeMismatchException.  The output parameter x is NOT
        updated if the value is null (isNull() == true).
        @param x Output Sint16 Array into which the value is stored if CIMValue
            state is not null.
        @exception TypeMismatchException If the value type does not match the
            output parameter.
    */
    void get(Array<Sint16>& x) const;

    /**
        Gets the value of the CIMValue.  The caller should first verify that
        the value is not null and may verify type to avoid the possibility of
        the TypeMismatchException.  The output parameter x is NOT
        updated if the value is null (isNull() == true).
        @param x Output Uint32 Array into which the value is stored if CIMValue
            state is not null.
        @exception TypeMismatchException If the value type does not match the
            output parameter.
    */
    void get(Array<Uint32>& x) const;

    /**
        Gets the value of the CIMValue.  The caller should first verify that
        the value is not null and may verify type to avoid the possibility of
        the TypeMismatchException.  The output parameter x is NOT
        updated if the value is null (isNull() == true).
        @param x Output Sint32 Array into which the value is stored if CIMValue
            state is not null.
        @exception TypeMismatchException If the value type does not match the
            output parameter.
    */
    void get(Array<Sint32>& x) const;

    /**
        Gets the value of the CIMValue.  The caller should first verify that
        the value is not null and may verify type to avoid the possibility of
        the TypeMismatchException.  The output parameter x is NOT
        updated if the value is null (isNull() == true).
        @param x Output Uint64 Array into which the value is stored if CIMValue
            state is not null.
        @exception TypeMismatchException If the value type does not match the
            output parameter.
    */
    void get(Array<Uint64>& x) const;

    /**
        Gets the value of the CIMValue.  The caller should first verify that
        the value is not null and may verify type to avoid the possibility of
        the TypeMismatchException.  The output parameter x is NOT
        updated if the value is null (isNull() == true).
        @param x Output Sint64 Array into which the value is stored if CIMValue
            state is not null.
        @exception TypeMismatchException If the value type does not match the
            output parameter.
    */
    void get(Array<Sint64>& x) const;

    /**
        Gets the value of the CIMValue.  The caller should first verify that
        the value is not null and may verify type to avoid the possibility of
        the TypeMismatchException.  The output parameter x is NOT
        updated if the value is null (isNull() == true).
        @param x Output Real32 Array into which the value is stored if CIMValue
            state is not null.
        @exception TypeMismatchException If the value type does not match the
            output parameter.
    */
    void get(Array<Real32>& x) const;

    /**
        Gets the value of the CIMValue.  The caller should first verify that
        the value is not null and may verify type to avoid the possibility of
        the TypeMismatchException.  The output parameter x is NOT
        updated if the value is null (isNull() == true).
        @param x Output Real64 Array into which the value is stored if CIMValue
            state is not null.
        @exception TypeMismatchException If the value type does not match the
            output parameter.
    */
    void get(Array<Real64>& x) const;

    /**
        Gets the value of the CIMValue.  The caller should first verify that
        the value is not null and may verify type to avoid the possibility of
        the TypeMismatchException.  The output parameter x is NOT
        updated if the value is null (isNull() == true).
        @param x Output Char16 Array into which the value is stored if CIMValue
            state is not null.
        @exception TypeMismatchException If the value type does not match the
            output parameter.
    */
    void get(Array<Char16>& x) const;

    /**
        Gets the value of the CIMValue.  The caller should first verify that
        the value is not null and may verify type to avoid the possibility of
        the TypeMismatchException.  The output parameter x is NOT
        updated if the value is null (isNull() == true).
        @param x Output String Array into which the value is stored if CIMValue
            state is not null.
        @exception TypeMismatchException If the value type does not match the
            output parameter.
    */
    void get(Array<String>& x) const;

    /**
        Gets the value of the CIMValue.  The caller should first verify that
        the value is not null and may verify type to avoid the possibility of
        the TypeMismatchException.  The output parameter x is NOT
        updated if the value is null (isNull() == true).
        @param x Output CIMDateTime Array into which the value is stored if
            CIMValue state is not null.
        @exception TypeMismatchException If the value type does not match the
            output parameter.
    */
    void get(Array<CIMDateTime>& x) const;

    /**
        Gets the value of the CIMValue.  The caller should first verify that
        the value is not null and may verify type to avoid the possibility of
        the TypeMismatchException.  The output parameter x is NOT
        updated if the value is null (isNull() == true).
        @param x Output CIMObjectPath Array into which the value is stored if
            CIMValue state is not null.
        @exception TypeMismatchException If the value type does not match the
            output parameter.
    */
    void get(Array<CIMObjectPath>& x) const;

    /**
        Gets the value of the CIMValue.  The caller should first verify that
        the value is not null and may verify type to avoid the possibility of
        the TypeMismatchException.  The output parameter x is NOT
        updated if the value is null (isNull() == true).
        @param x Output CIMObject Array into which the value is stored if
            CIMValue state is not null.
        @exception TypeMismatchException If the value type does not match the
            output parameter.
    */
    void get(Array<CIMObject>& x) const;

    /**
        Gets the value of the CIMValue.  The caller should first verify that
        the value is not null and may verify type to avoid the possibility of
        the TypeMismatchException.  The output parameter x is NOT
        updated if the value is null (isNull() == true).
        @param x Output CIMInstance Array into which the value is stored if
            CIMValue state is not null.
        @exception TypeMismatchException If the value type does not match the
            output parameter.
    */
    void get(Array<CIMInstance>& x) const;

    /**
        Compares with another CIMValue object for equality.
        @param x The CIMValue with which to compare
        @return A Boolean indicating whether they are identical in type,
            array attributes, and value.
    */
    Boolean equal(const CIMValue& x) const;

    /**
        Converts a CIMValue to a String.  This method should only be
        used for diagnostic output purposes.  To get an actual String value,
        use get(String &).
        @return A String representation of the value.
    */
    String toString() const;

#ifdef PEGASUS_USE_DEPRECATED_INTERFACES
    /**
        <I><B>Deprecated Interface</B></I><BR>
        Constructs a CIMValue with the specified value and type Sint8.
        (Note: This constructor exists solely to support binary compatibility
        with a previous definition of the Sint8 type.)
        @param x The Sint8 value to assign
    */
    CIMValue(char x);

    /**
        <I><B>Deprecated Interface</B></I><BR>
        Constructs a CIMValue with the specified value and type Sint8 Array.
        (Note: This constructor exists solely to support binary compatibility
        with a previous definition of the Sint8 type.)
        @param x The Sint8 Array value to assign
    */
    CIMValue(const Array<char>& x);

    /**
        <I><B>Deprecated Interface</B></I><BR>
        Sets the CIMValue to the specified value and type Sint8.
        (Note: This method exists solely to support binary compatibility with
        a previous definition of the Sint8 type.)
        @param x The value to assign
    */
    void set(char x);

    /**
        <I><B>Deprecated Interface</B></I><BR>
        Sets the CIMValue to the specified value and type Sint8 Array.
        (Note: This method exists solely to support binary compatibility with
        a previous definition of the Sint8 type.)
        @param x The Array value to assign
    */
    void set(const Array<char>& x);

    /**
        <I><B>Deprecated Interface</B></I><BR>
        Gets the Sint8 value of the CIMValue.  The caller should first verify
        that the value is of the expected type and is not null.  The output
        parameter x is not updated if the value is null (isNull() == true).
        (Note: This method exists solely to support binary compatibility with
        a previous definition of the Sint8 type.)
        @param x Output variable into which the value is stored.
    */
    void get(char& x) const;

    /**
        <I><B>Deprecated Interface</B></I><BR>
        Gets the Sint8 Array value of the CIMValue.  The caller should first
        verify that the value is of the expected type and is not null.  The
        output parameter x is not updated if the value is null
        (isNull() == true).
        (Note: This method exists solely to support binary compatibility with
        a previous definition of the Sint8 type.)
        @param x Output Array variable into which the value is stored.
    */
    void get(Array<char>& x) const;
#endif

private:

    void _get(const String*& data, Uint32& size) const;

    CIMValueRep* _rep;

    friend class CIMMethodRep;
    friend class CIMParameterRep;
    friend class CIMPropertyRep;
    friend class CIMQualifierRep;
    friend class CIMQualifierDeclRep;
    friend class BinaryStreamer;
    friend class XmlWriter;
    friend class SCMOClass;
    friend class SCMOInstance;
};

/**
    Compares two CIMValue objects for equality.
    @param x First CIMValue to compare
    @param y Second CIMValue to compare
    @return A Boolean indicating whether they are identical in type, array
        attributes, and value.
*/
PEGASUS_COMMON_LINKAGE Boolean operator==(const CIMValue& x, const CIMValue& y);

/**
    Compares two CIMValue objects for inequality.
    @param x First CIMValue to compare
    @param y Second CIMValue to compare
    @return A Boolean indicating whether they are NOT identical in type, array
        attributes, and value.
*/
PEGASUS_COMMON_LINKAGE Boolean operator!=(const CIMValue& x, const CIMValue& y);

#define PEGASUS_ARRAY_T CIMValue
# include <Pegasus/Common/ArrayInter.h>
#undef PEGASUS_ARRAY_T

PEGASUS_NAMESPACE_END

#ifdef PEGASUS_INTERNALONLY
#include <Pegasus/Common/CIMValueInline.h>
#endif

#endif /* Pegasus_Value_h */
