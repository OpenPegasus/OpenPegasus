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

#ifndef Pegasus_WQLOperand_h
#define Pegasus_WQLOperand_h

#include <new>
#include <Pegasus/Common/Config.h>
#include <Pegasus/Common/String.h>
#include <Pegasus/Common/InternalException.h>
#include <Pegasus/WQL/Linkage.h>

PEGASUS_NAMESPACE_BEGIN

/** Tag used to force invocation of the integer value form of the WQLOperand
    Constructor.
*/
enum WQLIntegerValueTag
{
    WQL_INTEGER_VALUE_TAG
};

/** Tag used to force invocation of the double value form of the WQLOperand
    Constructor.
*/
enum WQLDoubleValueTag
{
    WQL_DOUBLE_VALUE_TAG
};

/** Tag used to force invocation of the boolean value form of the WQLOperand
    Constructor.
*/
enum WQLBooleanValueTag
{
    WQL_BOOLEAN_VALUE_TAG
};

/** Tag used to force invocation of the string value form of the WQLOperand
    Constructor.
*/
enum WQLStringValueTag
{
    WQL_STRING_VALUE_TAG
};

/** Tag used to force invocation of the property name form of the WQLOperand
    Constructor.
*/
enum WQLPropertyNameTag
{
    WQL_PROPERTY_NAME_TAG
};

/** Used to represent SQL where clause operands.

    Instances of WQLOperand are used to represent the operands of the
    SQL where clause. Instances of this class are created while parsing
    a SQL where clause and added to the WQLSelectStatement by calling the
    WQLSelectStatement::appendOperand() method. Consider the following
    example:

    <pre>
       SELECT ratio, size, name, str
       FROM MyClass
        WHERE ratio &gt; 1.4 AND size = 3 AND name = "Hello" AND str IS NULL
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

    <ul>
    <li>NULL_VALUE - contains a null value of any type</li>
    <li>INTEGER_VALUE - an integer literal (e.g., 10, -22)</li>
    <li>DOUBLE_VALUE - a double literal (e.g., 1.4, 1.375e-5)</li>
    <li>BOOLEAN_VALUE - a boolean literal (e.g., TRUE or FALSE)</li>
    <li>STRING_VALUE - a string literal (e.g., "Hello World")</li>
    <li>PROPERTY_NAME- the name of a property (e.g., count, size)</li>
    </ul>
*/
class PEGASUS_WQL_LINKAGE WQLOperand
{
public:

    /** Defines allowed types of WQL operands (NULL_VALUE, INTEGER_VALUE,
    DOUBLE_VALUE, BOOLEAN_VALUE, STRING_VALUE, PROPERTY_NAME).
    */
    enum Type
    {
        NULL_VALUE,
        INTEGER_VALUE,
        DOUBLE_VALUE,
        BOOLEAN_VALUE,
        STRING_VALUE,
        PROPERTY_NAME
    };

    /** Desfault constructor. Initializes to null value.
    */
    WQLOperand();

    /** Copy constructor.
    */
    WQLOperand(const WQLOperand& x);

    /** Initializes object as INTEGER_VALUE.
    */
    WQLOperand(Sint64 x, WQLIntegerValueTag)
    {
        _integerValue = x;
        _type = INTEGER_VALUE;
    }

    /** Initializes object as DOUBLE_VALUE.
    */
    WQLOperand(Real64 x, WQLDoubleValueTag)
    {
        _doubleValue = x;
        _type = DOUBLE_VALUE;
    }

    /** Initializes object as BOOLEAN_VALUE.
    */
    WQLOperand(Boolean x, WQLBooleanValueTag)
    {
        _booleanValue = x;
        _type = BOOLEAN_VALUE;
    }

    /** Initializes object as STRING_VALUE.
    */
    WQLOperand(const String& x, WQLStringValueTag)
    {
        new(_stringValue) String(x);
        _type = STRING_VALUE;
    }

    /** Initializes object as PROPERTY_NAME.
    */
    WQLOperand(const String& x, WQLPropertyNameTag)
    {
        new(_propertyName) String(x);
        _type = PROPERTY_NAME;
    }

    /** Destructor.
    */
    ~WQLOperand();

    /** Assignment operator.
    */
    WQLOperand& operator=(const WQLOperand& x);

    /** Clears this object and sets its type to NULL_VALUE.
    */
    void clear();

    /** Assigns object from the given operand.
    */
    void assign(const WQLOperand& x);

    /** Accessor for getting the type of the operand.
    */
    Type getType() const { return _type; }

    /** Sets this object to an INTEGER_VALUE.
    */
    void setIntegerValue(Sint64 x)
    {
        clear();
        _integerValue = x;
        _type = INTEGER_VALUE;
    }

    /** Sets this object to an DOUBLE_VALUE.
    */
    void setDoubleValue(Real64 x)
    {
        clear();
        _doubleValue = x;
        _type = DOUBLE_VALUE;
    }

    /** Sets this object to a BOOLEAN_VALUE.
    */
    void setBooleanValue(Boolean x)
    {
        clear();
        _booleanValue = x;
        _type = BOOLEAN_VALUE;
    }

    /** Sets this object to a STRING_VALUE.
    */
    void setStringValue(const String& x)
    {
        clear();
        new(_stringValue) String(x);
        _type = STRING_VALUE;
    }

    /** Sets this object to a PROPERTY_NAME.
    */
    void setPropertyName(const String& x)
    {
        clear();
        new(_propertyName) String(x);
        _type = PROPERTY_NAME;
    }

    /** Gets this object as an INTEGER_VALUE.
    */
    Sint64 getIntegerValue() const
    {
        if (_type != INTEGER_VALUE)
            throw TypeMismatchException();

        return _integerValue;
    }

    /** Gets this object as an DOUBLE_VALUE.
    @exception TypeMismatchException is not the expected type.
    */
    Real64 getDoubleValue() const
    {
        if (_type != DOUBLE_VALUE)
            throw TypeMismatchException();

        return _doubleValue;
    }

    /** Gets this object as an BOOLEAN_VALUE.
    @exception TypeMismatchException is not the expected type.
    */
    Boolean getBooleanValue() const
    {
        if (_type != BOOLEAN_VALUE)
            throw TypeMismatchException();

        return _booleanValue;
    }

    /** Gets this object as a STRING_VALUE.
    @exception TypeMismatchException is not the expected type.
    */
    const String& getStringValue() const
    {
        if (_type != STRING_VALUE)
            throw TypeMismatchException();

        return *((String*)_stringValue);
    }

    /** Gets this object as a PROPERTY_NAME.
    @exception TypeMismatchException is not the expected type.
    */
    const String& getPropertyName() const
    {
        if (_type != PROPERTY_NAME)
            throw TypeMismatchException();

        return *((String*)_propertyName);
    }

    /** Converts this object to a string for output purposes.
    */
    String toString() const;

private:

    union
    {
        Sint64 _integerValue;
        Real64 _doubleValue;
        Boolean _booleanValue;
        char _stringValue[sizeof(String)];
        char _propertyName[sizeof(String)];
    };

    Type _type;
};

PEGASUS_NAMESPACE_END

#endif /* Pegasus_WQLOperand_h */
