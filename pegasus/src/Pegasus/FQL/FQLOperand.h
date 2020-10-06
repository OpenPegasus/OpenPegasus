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

#ifndef Pegasus_FQLOperand_h
#define Pegasus_FQLOperand_h

#include <new>
#include <Pegasus/Common/Config.h>
#include <Pegasus/Common/String.h>
#include <Pegasus/Common/InternalException.h>
#include <Pegasus/FQL/Linkage.h>
#include <Pegasus/Common/CIMDateTime.h>
#include <Pegasus/Common/CIMObjectPath.h>
#include <Pegasus/Common/Pegasus_inl.h>
#include <Pegasus/Common/ArrayInternal.h>
#include <Pegasus/Common/ArrayInter.h>
#include <Pegasus/Common/Exception.h>

PEGASUS_NAMESPACE_BEGIN

PEGASUS_USING_STD;
#ifdef FQL_DOTRACE
#define DCOUT if (true) cout << __FILE__ << ":" << __LINE__ << " "
#define CCOUT if (true) cout
#else
#define DCOUT if (false) cout << __FILE__ << ":" << __LINE__ << " "
#define CCOUT if (false) cout
#endif

/** Tags used to force invocation of the xxx value form of
    the FQLOperand Constructor.
*/
enum FQLNullValueTag
{
    FQL_NULL_VALUE_TAG
};

enum FQLIntegerValueTag
{
    FQL_INTEGER_VALUE_TAG
};

enum FQLDoubleValueTag
{
    FQL_DOUBLE_VALUE_TAG
};

enum FQLBooleanValueTag
{
    FQL_BOOLEAN_VALUE_TAG
};

enum FQLStringValueTag
{
    FQL_STRING_VALUE_TAG
};

enum FQLPropertyNameTag
{
    FQL_PROPERTY_NAME_TAG
};

enum FQLDateTimeValueTag
{
    FQL_DATETIME_VALUE_TAG
};

enum FQLReferenceValueTag
{
    FQL_REFERENCE_VALUE_TAG
};

/** Used to represent the Query Statement operands.

    Instances of FQLOperand are used to represent the operands of the
    query. Instances of this class are created while parsing an
    FQL query and added to the FQLQueryStatement by calling the
    FQLQueryStatement::appendOperand() method. Consider the
    following example:

    <pre>
    TODO: Give valid examples
       ratio &gt; 1.4 AND size = 3 AND name = "Hello" AND str IS NULL
    </pre>

    In this example, the following are operands:

    <pre>
        ratio
        1.4
        size
        3
        name
        "Hello"
        str
    </pre>

    Operands are of one of the following types:
    All of the types may be either scalar or arrays.
    TODO expand to Array or sort out how array fits.
    <ul>
    <li>NULL_VALUE - contains a null value of any type</li>
    <li>INTEGER_VALUE - an integer literal (e.g., 10, -22)</li>
    <li>DOUBLE_VALUE - a double literal (e.g., 1.4, 1.375e-5)</li>
    <li>BOOLEAN_VALUE - a boolean literal (e.g., TRUE or FALSE)</li>
    <li>STRING_VALUE - a string literal (e.g., 'Hello World')</li>
    <li>PROPERTY_NAME- the name of a property (e.g., count, size)</li>
    <li>DATETIME_VALUE- a CIM DateTime type)</li>
    <li>REFERENCE_VALUE</li>

    </ul>
*/
class PEGASUS_FQL_LINKAGE FQLOperand
{
public:

    /** Defines allowed types of FQL operands.
    */
    enum Type
    {
        NULL_VALUE,
        INTEGER_VALUE,
        DOUBLE_VALUE,
        BOOLEAN_VALUE,
        STRING_VALUE,
        PROPERTY_NAME,
        DATETIME_VALUE,
        REFERENCE_VALUE
    };

    enum propertyType
    {
        UNKNOWN_TYPE,
        ARRAY_TYPE,
        SCALAR_TYPE
    };

    /** Default constructor. Initializes to null value.
    */
    FQLOperand();

    /** Copy constructor.
    */
    FQLOperand(const FQLOperand& x);

    /*
        The following FQLOperand Constructors are defined for every
        FQLType to construct Scalar and Array FQLOperands for each type.
    */
    /** Initializes object as NULL_VALUE.
    */
    FQLOperand(FQLNullValueTag):
        _type(NULL_VALUE),
        _isArray(false)
    {
    }

    /** Initializes object as INTEGER_VALUE.
    */
    FQLOperand(Sint64 x, FQLIntegerValueTag):
        _integerValue(x),
        _type(INTEGER_VALUE),
        _isArray(false)
    {
    }

    // Create an Operand FQLArrayInteger from an array of integers
    FQLOperand(Array<Sint64> x, FQLIntegerValueTag):
        _type(INTEGER_VALUE),
        _isArray(true)
    {
        _arraySize = x.size();
        _arrayIntegerValue = x;
    }

    FQLOperand(Real64 x, FQLDoubleValueTag):
        _doubleValue(x),
        _type(DOUBLE_VALUE),
        _isArray(false)
    {
    }

    FQLOperand(Array<Real64> x, FQLDoubleValueTag):
        _arrayDoubleValue(x),
        _type(DOUBLE_VALUE),
        _isArray(true)
    {
        _arraySize = x.size();
    }

    FQLOperand(Boolean x, FQLBooleanValueTag):
        _booleanValue(x),
        _type(BOOLEAN_VALUE),
        _isArray(false)
    {
    }

    FQLOperand(const Array<Boolean> x, FQLBooleanValueTag):
        _type(BOOLEAN_VALUE),
        _isArray(true)
    {
        _arraySize = x.size();
        _arrayBooleanValue = x;
    }

    FQLOperand(const String& x, FQLStringValueTag):
        _type(STRING_VALUE),
        _isArray(false)
    {
        new(_stringValue) String(x);
    }

    FQLOperand(const Array<String>& x, FQLStringValueTag):
        _type(STRING_VALUE),
        _isArray(true)
    {
        _arraySize = x.size();
        _arrayStringValue = x;
    }

    //
    // Create DateTime FQLOperand from String and CIMDateTime
    // sources
    //
    FQLOperand(const String& x, FQLDateTimeValueTag):
        _type(DATETIME_VALUE),
        _isArray(false)
    {
        try
        {
            _dateTimeValue.set(x);
        }
        catch (InvalidDateTimeFormatException)
        {
            throw TypeMismatchException();
        }
    }

    FQLOperand(const CIMDateTime& x, FQLDateTimeValueTag):
        _dateTimeValue(x),
        _type(DATETIME_VALUE),
        _isArray(false)
    {
    }

    FQLOperand(const Array<String>& x, FQLDateTimeValueTag):
        _type(DATETIME_VALUE),
        _isArray(true)
    {
        try
        {
            for (Uint32 i = 0; i < x.size(); i++)
            {
                CIMDateTime y;
                y.set(x[i]);
                _arrayDateTimeValue.append(y);
            }
        }
        catch (InvalidDateTimeFormatException)
        {
            throw TypeMismatchException();
        }
    }

    FQLOperand(const Array<CIMDateTime>& x, FQLDateTimeValueTag):
        _type(DATETIME_VALUE),
        _isArray(true)
    {
        _arrayDateTimeValue = x;
        _arraySize = x.size();
    }

    //
    // Initialize Objects and REFERENCE_VALUE FQLOperand
    //
    FQLOperand(const String& x, FQLReferenceValueTag):
        _type(REFERENCE_VALUE),
        _isArray(false)
    {
        try
        {
            _referenceValue = CIMObjectPath(x);
        }
        catch (MalformedObjectNameException)
        {
            throw TypeMismatchException();
        }
    }

    FQLOperand(const CIMObjectPath& x, FQLReferenceValueTag):
        _type(REFERENCE_VALUE),
        _isArray(false)
    {
        _referenceValue = x;
    }

    FQLOperand(const Array<CIMObjectPath>& x, FQLReferenceValueTag):
        _type(REFERENCE_VALUE),
        _isArray(true)
    {
        _arrayReferenceValue = x;
        _arraySize = x.size();
    }

    // Create array references from array of strings
    FQLOperand(const Array<String>& x, FQLReferenceValueTag):
        _type(REFERENCE_VALUE),
        _isArray(true)
    {
        try
        {
            for (Uint32 i = 0; i < x.size(); i++)
            {
                CIMObjectPath y;
                y.set(x[i]);
                _arrayReferenceValue.append(y);
            }
        }
        catch (MalformedObjectNameException)
        {
            throw TypeMismatchException();
        }
    }

    /** Initializes object as PROPERTY_NAME.
    */
    FQLOperand(const String& x, FQLPropertyNameTag):
        _type(PROPERTY_NAME),
        _isArray(false),
        _isIndexedProperty(false)
    {
        new(_propertyName) String(x);
    }

    /** Initializes object as PROPERTY_NAME with Index and dotted
     *  indicator
    */
    FQLOperand(const String& x, FQLPropertyNameTag, Uint32 index):
        _type(PROPERTY_NAME),
        _isArray(false),
        _isIndexedProperty(true),
        _propertyArrayIndex(index)
    {
        new(_propertyName) String(x);
    }

    // Create an array operand from an Array of scalars of the same type
    FQLOperand(Array<FQLOperand*> arr)
        {
#ifdef FQL_DOTRACE
            for (Uint32 i = 0; i < arr.size(); i++)
            {
                DCOUT << "arrayEntries " << arr[i]->toString() << endl;
            }
#endif
            if (arr.size() == 0)
            {
                throw TypeMismatchException("Array size 0");
            }
            Type t = arr[0]->getType();
            for (Uint32 i = 0 ; i < arr.size(); i++)
            {
                if (arr[i]->getType() != t)
                {
#ifdef FQL_DOTRACE
                    DCOUT << "FQL Type error FQL[i] type=" << arr[i]->getType()
                          << "does not match  " << i
                          << " FQLOperand=" << arr[i]->toString() << endl;
#endif
                    throw TypeMismatchException("Array literal type mismatch");
                }

                switch (t)
                {
                    case BOOLEAN_VALUE:
                        {
                            _arrayBooleanValue.append(
                                arr[i]->getBooleanValue());
                            _type = BOOLEAN_VALUE;
                            _isArray = true;
                            _arraySize = _arrayBooleanValue.size();
                        }
                        break;
                    case INTEGER_VALUE:
                        {
                            _arrayIntegerValue.append(
                                arr[i]->getIntegerValue());
                            _type = INTEGER_VALUE;
                            _isArray = true;
                            _arraySize = _arrayIntegerValue.size();

                        }
                        break;
                    case STRING_VALUE:
                        {
                            _arrayStringValue.append(
                                arr[i]->getStringValue());
                            _type = STRING_VALUE;
                            _isArray = true;
                            _arraySize = _arrayStringValue.size();
                        }
                        break;
                    case DOUBLE_VALUE:
                        {
                            _arrayDoubleValue.append(
                                arr[i]->getDoubleValue());
                            _type = DOUBLE_VALUE;
                            _isArray = true;
                            _arraySize = _arrayDoubleValue.size();
                        }
                        break;
                    case DATETIME_VALUE:
                        {
                            _arrayDateTimeValue.append(
                                arr[i]->getDateTimeValue());
                            _type = DATETIME_VALUE;
                            _isArray = true;
                            _arraySize = _arrayDateTimeValue.size();
                        }
                        break;
                    case NULL_VALUE:
                        {
                            _type = NULL_VALUE;
                        }
                        break;
                    case PROPERTY_NAME:
                    case REFERENCE_VALUE:
                        {
                            PEGASUS_ASSERT(0);
                        }
                }
            }
            //// delete the incoming or do not create with new at all.
            arr.clear();
        }

    /** Destructor.
    */
    ~FQLOperand();

    /** Assignment operator.
    */
    FQLOperand& operator=(const FQLOperand& x);

    /** Clears this object and sets its type to NULL_VALUE.
    */
    void clear();

    /** Assigns object from the given operand.
    */
    void assign(const FQLOperand& x);

    /** Accessor for getting the type of the operand.
    */
    Type getType() const { return _type; }

    void setNullValue()
    {
        clear();
        _type = NULL_VALUE;
        _isArray = false;
    }

    /** Sets this object to an Scalar INTEGER_VALUE.
    */
    void setIntegerValue(Sint64 x)
    {
        clear();
        _integerValue = x;
        _type = INTEGER_VALUE;
        _isArray = false;
    }

    /** Sets this object to an DOUBLE_VALUE.
    */
    void setDoubleValue(Real64 x)
    {
        clear();
        _doubleValue = x;
        _type = DOUBLE_VALUE;
        _isArray = false;
    }

    /** Sets this object to a BOOLEAN_VALUE.
    */
    void setBooleanValue(Boolean x)
    {
        clear();
        _booleanValue = x;
        _type = BOOLEAN_VALUE;
        _isArray = false;
    }

    /** Sets this object to a STRING_VALUE.
    */
    void setStringValue(const String& x)
    {
        clear();
        new(_stringValue) String(x);
        _type = STRING_VALUE;
        _isArray = false;
    }

    /** Sets this object to a REFERENCE_VALUE.
    */
    void setReferenceValue(const CIMObjectPath& x)
    {
        clear();
        _referenceValue = CIMObjectPath(x);
        _type = REFERENCE_VALUE;
        _isArray = false;
    }

    /** Sets this object to a CIMDATETIME_VALUE.
    */
    void setDateTimeValue(const CIMDateTime& x)
    {
        clear();
        _dateTimeValue = CIMDateTime(x);
        _type = DATETIME_VALUE;
        _isArray = false;
    }

    /** Gets this object as an INTEGER_VALUE.
    */

    // NOTE: NULL does not have a get function. It only has one value
    // NULL. The existens of an FQLOperand of type NULL_VALUE itself
    // defines NULL

    Sint64 getIntegerValue() const
    {
        if (_type != INTEGER_VALUE || _isArray)
            throw TypeMismatchException();

        return _integerValue;
    }

    Array<Sint64> getArrayIntegerValue() const
    {
        if (_type != INTEGER_VALUE || !_isArray)
        {
            throw TypeMismatchException();
        }
        return _arrayIntegerValue;
    }

    // TODO Make the conversion once and modify the operand itself.
    CIMDateTime getDateTimeValue() const
    {
        if (_type == STRING_VALUE && !_isArray)
        {
            try
            {
               return CIMDateTime(_stringValue);
            }
            catch (InvalidDateTimeFormatException)
            {
                throw TypeMismatchException();
            }
        }
        if (_type != DATETIME_VALUE || _isArray)
        {
            throw TypeMismatchException();
        }
        return _dateTimeValue;
    }

    // TODO Make the conversion once and modify the operand itself.
    CIMObjectPath getReferenceValue() const
    {
        if (_type == STRING_VALUE && !_isArray)
        {
            try
            {
               return CIMObjectPath(String(_stringValue));
            }
            catch (...)
            {
                throw TypeMismatchException();
            }
        }
        if (_type != REFERENCE_VALUE || _isArray)
        {
            throw TypeMismatchException();
        }
        return _referenceValue;
    }

    // TODO put reference back in return
    Array<CIMDateTime> getArrayDateTimeValue() const
    {
        if (!_isArray)
        {
            throw TypeMismatchException();
        }

        if (_type == STRING_VALUE)
        {
            Array<CIMDateTime> rtn;
            try
            {
                for (Uint32 i = 0; i < _arrayStringValue.size(); i++)
                {
                    rtn.append(CIMDateTime(_arrayStringValue[i]));
                }
            }
            catch (InvalidDateTimeFormatException)
            {
                throw TypeMismatchException();
            }
            return rtn;
        }

        if (_type != DATETIME_VALUE || !_isArray)
        {
            throw TypeMismatchException();
        }
        return _arrayDateTimeValue;
    }

    // TODO put reference back in return
    // KS_TODO This just converts current. We are using it
    // in the evaluator so converting every time.
    Array<CIMObjectPath> getArrayReferenceValue() const
    {
        if (!_isArray)
        {
            throw TypeMismatchException();
        }

        if (_type == STRING_VALUE)
        {
            Array<CIMObjectPath> rtn;
            try
            {
                for (Uint32 i = 0; i < _arrayStringValue.size(); i++)
                {
                    rtn.append(CIMObjectPath(_arrayStringValue[i]));
                }
            }
            catch (MalformedObjectNameException)
            {
                throw TypeMismatchException();
            }
            return rtn;
        }

        if (_type != DATETIME_VALUE || !_isArray)
        {
            throw TypeMismatchException();
        }
        return _arrayReferenceValue;
    }

    // Is this FQLOperand array type
    // KS_TODO make this inline explicitly.
    bool isArrayType() const
    {
        return _isArray;
    }

    /** Gets this object as an Array of INTEGER_VALUE.
    */
    Sint64 getArrayIntegerValue(Uint32 index) const
    {
        if (_type != INTEGER_VALUE || !_isArray)
        {
            throw TypeMismatchException();
        }
        if (_arrayIntegerValue.size() < index)
        {
            throw IndexOutOfBoundsException();
        }

        return (_arrayIntegerValue[index]);
    }

    const String& getArrayStringValue(Uint32 index) const
    {
        if (_type != STRING_VALUE || !_isArray)
        {
            throw TypeMismatchException();
        }
        if (_arrayStringValue.size() < index)
        {
            throw IndexOutOfBoundsException();
        }
        return (_arrayStringValue[index]);
    }

    const Real64& getArrayDoubleValue(Uint32 index) const
    {
        if (_type != DOUBLE_VALUE || !_isArray)
        {
            throw TypeMismatchException();
        }
        if (_arrayDoubleValue.size() < index)
        {
            //// TODO  Need exception for index out of range
            DCOUT << "array size error " << _arrayDoubleValue.size()
                << " index " << index << endl;
            throw IndexOutOfBoundsException();
        }
        return (_arrayDoubleValue[index]);
    }

    const Boolean& getArrayBooleanValue(Uint32 index) const
    {
        if (_type != BOOLEAN_VALUE || !_isArray)
        {
            throw TypeMismatchException();
        }
        if (_arrayBooleanValue.size() < index)
        {
            throw IndexOutOfBoundsException();
        }
        return (_arrayBooleanValue[index]);
    }

    // The only use for this one is for properties that are
    // date time type so there is NO reason to map from string
    const CIMDateTime& getArrayDateTimeValue(Uint32 index) const
    {
        if (_type != DATETIME_VALUE || !_isArray)
        {
            throw TypeMismatchException();
        }
        if (_arrayDateTimeValue.size() < index)
        {
            throw IndexOutOfBoundsException();
        }
        return (_arrayDateTimeValue[index]);
    }

    // The only use for this one is for properties that are
    // reference type so there is NO reason to map from string
    const CIMObjectPath& getArrayReferenceValue(Uint32 index) const
    {
        if (_type != REFERENCE_VALUE || !_isArray)
        {
            throw TypeMismatchException();
        }
        if (_arrayReferenceValue.size() < index)
        {
            throw IndexOutOfBoundsException();

        }
        return (_arrayReferenceValue[index]);
    }

    /*
        Gets this object as a DoubleValue, reaturning Real64
        @exception TypeMismatchException is not the expected type.
    */
    Real64 getDoubleValue() const
    {
        if (_type != DOUBLE_VALUE || _isArray)
        {
            throw TypeMismatchException();
        }
        return _doubleValue;
    }
    Array<Real64> getArrayDoubleValue() const
    {
        if (_type != DOUBLE_VALUE || !_isArray)
        {
            throw TypeMismatchException();
        }
        return _arrayDoubleValue;
    }

    /** Gets this object as an BOOLEAN_VALUE.
        @exception TypeMismatchException is not the expected type.
    */
    Boolean getBooleanValue() const
    {
        if (_type != BOOLEAN_VALUE || _isArray)
        {
            throw TypeMismatchException();
        }
        return _booleanValue;
    }

    Array<Boolean> getArrayBooleanValue() const
    {
        if (_type != BOOLEAN_VALUE || !_isArray)
        {
            throw TypeMismatchException();
        }
        return _arrayBooleanValue;
    }

    /** Gets this object as a STRING_VALUE.
    @exception TypeMismatchException is not the expected type.
    */
    const String& getStringValue() const
    {
        if (_type != STRING_VALUE || _isArray)
        {
            throw TypeMismatchException();
        }
        return *((String*)_stringValue);
    }

    Array<String> getArrayStringValue() const
    {
        if (_type != STRING_VALUE || !_isArray)
        {
            throw TypeMismatchException();
        }

        return _arrayStringValue;
    }
    /** Gets this object as a PROPERTY_NAME.
        @exception TypeMismatchException is not the expected type.
    */
    const String& getPropertyName() const
    {
        if (_type != PROPERTY_NAME)
        {
            throw TypeMismatchException();
        }

        return *((String*)_propertyName);
    }

    bool isIndexedProperty()
    {
        return _isIndexedProperty;
    }
    unsigned int propertyIndex() {return _propertyArrayIndex;}

    bool isArray() {return _isArray;}

    Uint32 arraySize() const;

    /** modify the FQLOperation to get one entry from the fromOp
        array and put it into this Operation.
        @param fromOp
        @param index

        @return Boolean
     */
    Boolean getIndexedValue(const FQLOperand& fromOp, Uint32 index);

    /** Convert this object to a string for display.
    */
    String toString() const;

    // Manipulate the _chain parameter that chains Operands to
    // an existing operand.  Used for dot property names
    void chain(const FQLOperand& x)
    {
        _chain.append(x);
    }
    Uint32 chainSize() const
    {
        return _chain.size();
    }
    FQLOperand chainItem(Uint32 i) const
    {
        return _chain[i];
    }
    Boolean isChained()
    {
        return _chain.size() != 0;
    }
    void setCIMType(CIMType x)
    {
        _cimType = x;
    }

private:

    union
    {
        Sint64 _integerValue;
        Real64 _doubleValue;
        Boolean _booleanValue;
        char _stringValue[sizeof(String)];
        char _propertyName[sizeof(String)];
        //// TODO we want to map these to converted form
        /////char _referenceValue[sizeof(String)];
        ////char _datetimeValue[sizeof(String)];
    };
    CIMDateTime _dateTimeValue;
    CIMObjectPath _referenceValue;

    Uint32 _arraySize;
    Array<Sint64> _arrayIntegerValue;
    Array<Boolean> _arrayBooleanValue;
    Array<Real64> _arrayDoubleValue;
    Array<String> _arrayStringValue;
    Array<CIMDateTime> _arrayDateTimeValue;
    Array<CIMObjectPath> _arrayReferenceValue;

    /// Type value for this operand
    Type _type;
    // Set when property types are resolved
    CIMType _cimType;
    bool _isArray;
    bool _resolved;
    propertyType propertyType;
    // Define existence of Property Operand with index
    // and the value of the index if _isIndexedProperty = true
    bool _isIndexedProperty;
    unsigned int _propertyArrayIndex;
    Array<FQLOperand> _chain;
};

PEGASUS_NAMESPACE_END

#endif /* Pegasus_FQLOperand_h */

