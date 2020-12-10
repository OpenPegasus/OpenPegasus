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

#include <cstdio>
#include "FQLOperand.h"
#include <Pegasus/Common/StringConversion.h>
#include <Pegasus/Common/String.h>

PEGASUS_NAMESPACE_BEGIN

FQLOperand::FQLOperand() : _type(NULL_VALUE), _resolved(false), _isArray(false)
{
}

FQLOperand::FQLOperand(const FQLOperand& x)
{
    assign(x);
}

FQLOperand::~FQLOperand()
{
    clear();
}

FQLOperand& FQLOperand::operator=(const FQLOperand& x)
{
    clear();
    assign(x);
    return *this;
}

void FQLOperand::clear()
{
    if (_isArray)
    {
        switch (_type)
        {
            case STRING_VALUE:
                {
                    _arrayStringValue.clear();
                    break;
                }
            case INTEGER_VALUE:
                {
                    _arrayIntegerValue.clear();
                    break;
                }
            case BOOLEAN_VALUE:
                {
                    _arrayBooleanValue.clear();
                    break;
                }
            case DOUBLE_VALUE:
                {
                    _arrayDoubleValue.clear();
                    break;
                }
            case DATETIME_VALUE:
                {
                    _arrayDateTimeValue.clear();
                    break;
                }
            case REFERENCE_VALUE:
                {
                    _arrayReferenceValue.clear();
                    break;
                }
        case NULL_VALUE:
        case PROPERTY_NAME:
            { /* nothing for momement*/
            }
        }
    }
    else
    {
        switch (_type)
        {
            case PROPERTY_NAME:
                {
                    ((String*)_propertyName)->~String();
                    break;
                }
            case STRING_VALUE:
                {
                    ((String*)_stringValue)->~String();
                    break;
                }
            case INTEGER_VALUE:
            case BOOLEAN_VALUE:
            case DOUBLE_VALUE:
            case DATETIME_VALUE:
            case REFERENCE_VALUE:
            case NULL_VALUE:
                {/* do nothing */}
        }
    }
    _type = NULL_VALUE;
    _resolved = false;
    _isArray = false;
    _isIndexedProperty = false;
    _propertyArrayIndex = 0;
    _chain.clear();
}

void FQLOperand::assign(const FQLOperand& x)
{
    switch (_type = x._type)
    {
    case PROPERTY_NAME:
        new(_propertyName) String(*((String*)x._propertyName));
        _isIndexedProperty = x._isIndexedProperty;
        _propertyArrayIndex = x._propertyArrayIndex;
        break;

    case STRING_VALUE:
        if (x._isArray)
        {
            _arrayStringValue = x._arrayStringValue;
        }
        else
        {
            new(_stringValue)String(*((String * )x._stringValue));
        }
        break;

    case INTEGER_VALUE:
        if (x._isArray)
        {
            _arrayIntegerValue = x._arrayIntegerValue;
        }
        else
        {
            _integerValue = x._integerValue;
        }
        break;

    case DOUBLE_VALUE:
        if (x._isArray)
        {
            _arrayDoubleValue = x._arrayDoubleValue;
        }
        else
        {
            _doubleValue = x._doubleValue;
        }
        break;

    case BOOLEAN_VALUE:
        if (x._isArray)
        {
            _arrayBooleanValue = x._arrayBooleanValue;
        }
        else
        {
            _booleanValue = x._booleanValue;
        }
        break;

    case NULL_VALUE:
        _integerValue = 0;
        //// KS_TODO set all non union values to empty also.
        break;

    case DATETIME_VALUE:
        if (x._isArray)
        {
            _arrayDateTimeValue = x._arrayDateTimeValue;
        }
        else
        {
            _dateTimeValue = x._dateTimeValue;
        }
        break;

    case REFERENCE_VALUE:
        if (x._isArray)
        {
            _arrayReferenceValue = x._arrayReferenceValue;
        }
        else
        {
            _referenceValue = x._referenceValue;
        }
        break;
    }
    _isArray = x._isArray;
    _type = x._type;
    _resolved = x._resolved;
    _arraySize = x._arraySize;
    _chain = x._chain;
    _cimType = x._cimType;
}

/* Modify the FQLOperand to define a single indexed Property from the
   FQLOperand defined in the op parameter
*/
Boolean FQLOperand::getIndexedValue(const FQLOperand& fromOp, Uint32 index)
{
#ifdef FQL_DOTRACE
    DCOUT << "FQLOperand::getIndexedValue " << fromOp.toString()
        << " fromOp._isIndexedProperty= "
        << (fromOp._isIndexedProperty? "true" : "false") << endl;
#endif

    if (fromOp._isArray)
    {
        try
        {
            PEGASUS_ASSERT(fromOp._isArray == true);
            switch (fromOp._type)
            {
                case NULL_VALUE:
                    {
                        PEGASUS_ASSERT(false);
                        break;
                    }
                case INTEGER_VALUE:
                    {
                        Sint64 value = fromOp.getArrayIntegerValue(index);
                        setIntegerValue(value);
                        break;
                    }
                case DOUBLE_VALUE:
                    {
                        Sint64 value = fromOp.getArrayDoubleValue(index);
                        setDoubleValue(value);
                        break;
                    }
                case BOOLEAN_VALUE:
                    {
                        Sint64 value = fromOp.getArrayBooleanValue(index);
                        setBooleanValue(value);
                        break;
                    }
                case STRING_VALUE:
                    {
                        const String value = fromOp.getArrayStringValue(index);
                        setStringValue(value);
                    }
                    break;

                case PROPERTY_NAME:
                    {
                        PEGASUS_ASSERT(false);
                        break;
                    }
                case DATETIME_VALUE:
                    {
                        CIMDateTime value = fromOp.getArrayDateTimeValue(index);
                        setDateTimeValue(value);
                    }
                break;
                case REFERENCE_VALUE:
                    {
                        const CIMObjectPath value =
                            fromOp.getArrayReferenceValue(index);
                        setReferenceValue(value);
                    }
                break;
            }
        }
        catch (IndexOutOfBoundsException)
        {
            return false;
        }
#ifdef FQL_TOTRACE
        DCOUT << "FQLOperand::getIndexedValue Result = " << toString() << endl;
#endif
        return true;
    }
#ifdef FQL_DOTRACE
    DCOUT << "FQLOperand::getIndexedValue Result. return false. toString() = "
          << toString() << endl;
#endif
    return false;
}

Uint32 FQLOperand::arraySize() const
{
    PEGASUS_ASSERT(_isArray);
    switch (_type)
    {
        case STRING_VALUE:
        return _arrayStringValue.size();
        break;
        case INTEGER_VALUE:
            return _arrayIntegerValue.size();
        break;
        case DOUBLE_VALUE:
            return _arrayDoubleValue.size();
        break;
        case BOOLEAN_VALUE:
            return _arrayBooleanValue.size();
        break;
        case DATETIME_VALUE:
            return _arrayDateTimeValue.size();
        break;
        case REFERENCE_VALUE:
            return _arrayReferenceValue.size();
        break;
        default:
        {
            PEGASUS_ASSERT(false);
        }
    }
}

// KS_TODO - Simplfy by creating a toStringValue with case for each
// type then common function for all of the array vs non array displays.
String FQLOperand::toString() const
{
    String result;

    switch (_type)
    {
        case NULL_VALUE:
            {
                result = "NULL_VALUE";
                break;
            }
        case PROPERTY_NAME:
            {
                result = "PROPERTY_NAME: ";
                result.append(*((String*)_propertyName));
                if (_isIndexedProperty)
                {
                    result.append("[");
                    char tmp[22];
                    Uint32 cvtSize;
                    result.append(Uint32ToString(tmp, _propertyArrayIndex,
                        cvtSize));
                    result.append("]");
                }
                if (_chain.size() != 0)
                {
                    result.append(" chained. ");
                }
                break;
            }

        case STRING_VALUE:
            {
                result = "STRING_VALUE: ";
                if (_isArray)
                {
                    result.append("[");
                    char tmp[22];
                    Uint32 cvtSize;
                    result.append(Uint32ToString(tmp,
                        _arrayStringValue.size() , cvtSize));
                    result.append("]: ");

                    for (Uint32 i = 0; i < _arrayStringValue.size(); i++)
                    {
                        if (i > 0)
                        {
                            result.append(", ");
                        }
                        result.append("\"");
                        result.append(_arrayStringValue[i]);
                        result.append("\"");
                    }
                }
                else
                {
                    result.append(*((String*)_stringValue));
                }
                break;
            }

        case INTEGER_VALUE:
            {
                result = "INTEGER_VALUE: ";
                if (_isArray)
                {
                    result.append("[");
                    char tmp[22];
                    Uint32 cvtSize;
                    result.append(Uint32ToString(tmp,
                        _arrayIntegerValue.size() , cvtSize));
                    result.append("]: ");

                    for (Uint32 i = 0; i < _arrayIntegerValue.size(); i++)
                    {
                        if (i > 0)
                        {
                            result.append(", ");
                        }
                        char buffer[32];
                        sprintf(buffer, "%" PEGASUS_64BIT_CONVERSION_WIDTH "d",
                                    _arrayIntegerValue[i]);
                        result.append(buffer);
                    }
                }
                else
                {
                    char buffer[32];
                    sprintf(buffer, "%" PEGASUS_64BIT_CONVERSION_WIDTH "d",
                                _integerValue);
                    result.append(buffer);
                }
                break;
            }

        case DOUBLE_VALUE:
            {
                result = "DOUBLE_VALUE: ";
                if (_isArray)
                {
                    result.append("[");
                    char tmp[22];
                    Uint32 cvtSize;
                    result.append(Uint32ToString(tmp,
                        _arrayDoubleValue.size() , cvtSize));
                    result.append("]: ");
                    for (Uint32 i = 0; i < _arrayDoubleValue.size(); i++)
                    {
                        if (i > 0)
                        {
                            result.append(", ");
                        }
                        char buffer[32];
                        sprintf(buffer, "%lf", _arrayDoubleValue[i]);
                        result.append(buffer);
                    }
                }
                else
                {
                    char buffer[32];
                    sprintf(buffer, "%lf", _doubleValue);
                    result.append(buffer);
                }
                break;
            }

        case BOOLEAN_VALUE:
            {
                result = "BOOLEAN_VALUE: ";

                if (_isArray)
                {
                    result.append("[");
                    char tmp[22];
                    Uint32 cvtSize;
                    result.append(Uint32ToString(tmp,
                        _arrayBooleanValue.size() , cvtSize));
                    result.append("]: ");

                    for (Uint32 i = 0; i < _arrayBooleanValue.size(); i++)
                    {
                        if (i > 0)
                        {
                            result.append(", ");
                        }
                        result.append(_arrayBooleanValue[i]? "TRUE" : "FALSE");
                    }
                }
                else
                {
                    result.append(_booleanValue? "TRUE" : "FALSE");
                }
                break;
            }
        case DATETIME_VALUE:
            {
                result = "DATETIME_VALUE: ";
                if (_isArray)
                {
                    result.append("[");
                    char tmp[22];
                    Uint32 cvtSize;
                    result.append(Uint32ToString(tmp,
                        _arrayDateTimeValue.size() , cvtSize));
                    result.append("]: ");
                    for (Uint32 i = 0; i < _arrayDateTimeValue.size(); i++)
                    {
                        if (i > 0)
                        {
                            result.append(", ");
                        }
                        result.append(_arrayDateTimeValue[i].toString());
                    }
                }
                else
                {
                    result.append(_dateTimeValue.toString());
                }
                break;
            }
        case REFERENCE_VALUE:
            {
                result = "REFERENCE_VALUE: ";
                if (_isArray)
                {
                    result.append("[");
                    char tmp[22];
                    Uint32 cvtSize;
                    result.append(Uint32ToString(tmp,
                        _arrayReferenceValue.size() , cvtSize));
                    result.append("]: ");
                    for (Uint32 i = 0; i < _arrayReferenceValue.size(); i++)
                    {
                        if (i > 0)
                        {
                            result.append(", ");
                        }
                        result.append(_arrayReferenceValue[i].toString());
                    }
                }
                else
                {
                    result.append(_referenceValue.toString());
                }
                break;
            }
    }

    return result;
}

PEGASUS_NAMESPACE_END
