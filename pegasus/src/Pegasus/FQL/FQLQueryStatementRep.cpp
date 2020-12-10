
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

#include <iostream>
#include <Pegasus/Common/Stack.h>
#include "FQLQueryStatementRep.h"
#include <Pegasus/Query/QueryCommon/QueryException.h>
#include "FQLInstancePropertySource.h"
#include "FQLRegularExpression.h"
#include "FQLOperand.h"
#include "Pegasus/Common/Exception.h"
PEGASUS_USING_STD;

PEGASUS_NAMESPACE_BEGIN

// The following are test classes.
// KS_TODO should be able to remove or completely disable
// these before release.

#ifdef FQL_DOTRACE
template<class T>
void _displayArray(const T& x)
{
    DCOUT<< "Array size " << x.size() << "{";
    for (Uint32 i = 0; i < x.size(); i++)
    {
        if (i > 0)
        {
            CCOUT << ", ";
        }
        CCOUT << x[i];
    }
    CCOUT << "}" << endl;
}

// Specializations of the _display Array function
inline void _displayArray(const Array<CIMDateTime>& x)
{
    DCOUT << "Array size " << x.size() << "{";
    for (Uint32 i = 0; i < x.size(); i++)
    {
        if (i > 0)
        {
            CCOUT << ", ";
        }
        CCOUT << x[i].toString();
    }
    CCOUT << "}" << endl;
}
inline void _displayArray(const Array<CIMObjectPath>& x)
{
    DCOUT << "Array size " << x.size() << "{";
    for (Uint32 i = 0; i < x.size(); i++)
    {
        if (i > 0)
        {
            CCOUT << ", ";
        }
        CCOUT << x[i].toString();
    }
    CCOUT << "}" << endl;
}

#endif

// template class to CompareArrays for each of the CIMTypes
template<class T>
inline static Boolean _CompareArray(const T& x, const T& y, FQLOperation op)
{
#ifdef FQL_DOTRACE
    DCOUT << "_CompareArrays" << endl;
#endif
    switch (op)
    {
        case FQL_UNKNOWN:
            PEGASUS_ASSERT(false);
        case FQL_EQ:
            {
#ifdef FQL_DOTRACE
                DCOUT << "Sizes " << x.size() << " " << y.size() << endl;
                _displayArray(x);
                _displayArray(y);
                DCOUT << "size compare " << boolToString(x.size() == y.size())
                                                         << endl;
#endif
                if (x.size() == y.size())
                {
                    return x == y;
                }
                return false;
            }
        case FQL_NE:
            return x != y;

        // NOPE array-op array_property scalar-comp-op array-property[index]
        // or scalar-property or scalar-literal
        // Compare array to the
        // KS_TODO The following are completely incorrect
        case FQL_ANY:
            for (Uint32 i = 0; i < x.size(); i++)
            {
                PEGASUS_ASSERT(false);
            }
            return x == y;

        case FQL_NOT_ANY:
            for (Uint32 i = 0; i < x.size(); i++)
            {
                PEGASUS_ASSERT(false);
            }
            return x == y;

        case FQL_EVERY:
            return x == y;

        case FQL_NOT_EVERY:
            return x != y;

        default:
            PEGASUS_ASSERT(0);
    }

    return false;
}
/*
    Template class compares two scalar constants of the same type and
    returns a boolean result.
*/
template<class T>
inline static Boolean _CompareScalar(const T& x, const T& y, FQLOperation op)
{
#ifdef FQL_DOTRACE
    DCOUT << "_CompareScalar " << FQLOperationToString(op) << endl;
#endif
    switch (op)
    {
        case FQL_UNKNOWN:
            PEGASUS_ASSERT(false);

        case FQL_EQ:
            return x == y;

        case FQL_NE:
            return x != y;

        case FQL_LT:
            return x < y;

        case FQL_LE:
            return x <= y;

        case FQL_GT:
            return x > y;

        case FQL_GE:
            return x >= y;

        // The following are not handled by this function
        // provide complete list to avoid default
        case FQL_NOT_LIKE:
        case FQL_LIKE:
        case FQL_ANY:
        case FQL_NOT_ANY:
        case FQL_EVERY:
        case FQL_NOT_EVERY:
        case FQL_IS_NULL:
            PEGASUS_ASSERT(false);

        default:
            PEGASUS_ASSERT(0);
    }

    return false;
}

template<>
inline  Boolean _CompareScalar(const CIMObjectPath& x,
    const CIMObjectPath& y,FQLOperation op)
{
#ifdef FQL_DOTRACE
    DCOUT << "_CompareScalar CIMObjectPath "
        << FQLOperationToString(op) << endl;
#endif
    switch (op)
    {
        case FQL_UNKNOWN:
            PEGASUS_ASSERT(false);

        case FQL_EQ:
            return x == y;

        case FQL_NE:
            return x != y;

        case FQL_LT:
        case FQL_LE:
        case FQL_GT:
        case FQL_GE:
        case FQL_LIKE:
        case FQL_NOT_LIKE:
        case FQL_ANY:
        case FQL_NOT_ANY:
        case FQL_EVERY:
        case FQL_NOT_EVERY:
        case FQL_IS_NULL:
            PEGASUS_ASSERT(false);

        default:
            PEGASUS_ASSERT(0);
    }

    return false;
}


// Returns true/false based only on the operation type.
// KS_TODO NOT Sure this is correct.  What if the rhs is NULL?
inline static Boolean _CompareLhsIsNull(FQLOperation op)
{
#ifdef FQL_DOTRACE
    DCOUT << "_Compare lhsNULL " << FQLOperationToString(op) << endl;
#endif

    switch (op)
    {
        case FQL_IS_NULL:
        case FQL_NE:
            return true;
        case FQL_LE:
        case FQL_GE:
        case FQL_EQ:
        case FQL_LT:
        case FQL_GT:
        case FQL_LIKE:
        case FQL_NOT_LIKE:
        case FQL_ANY:
        case FQL_NOT_ANY:
        case FQL_EVERY:
        case FQL_NOT_EVERY:
            return false;
        case FQL_AND:
        case FQL_OR:
        case FQL_UNKNOWN:
        case FQL_NOT:
          PEGASUS_ASSERT(0);
    }
    return false;
}

/*
    Evaluate two Operands and an operator.
*/
static Boolean _Evaluate(
    const FQLOperand& lhs,
    const FQLOperand& rhs,
    FQLOperation op)
{
#ifdef FQL_DOTRACE
    DCOUT << "Evaluate. OP " << FQLOperationToString(op)
        << " lhs:" << lhs.toString()
        << " rhs:" << rhs.toString() << endl;
#endif
    // Select Compare function based on left hand Operand.

    // This covers indexOutOfBounds exception for getArray...Value
    // that is out of range of the data array
    try
    {
        switch (lhs.getType())
        {
            // Indicates that property does not exist. Compare to rhs.type
            // and return true if it is a null value. Everything else returns
            // false
            case FQLOperand::NULL_VALUE:
            {
                return(!_CompareLhsIsNull(op));
            }

            case FQLOperand::INTEGER_VALUE:
            {
                if (lhs.isArrayType())
                {
                    return _CompareArray(
                        lhs.getArrayIntegerValue(),
                        rhs.getArrayIntegerValue(),
                        op);
                }
                else
                {
                    return _CompareScalar(
                        lhs.getIntegerValue(),
                        rhs.getIntegerValue(),
                        op);
                }
            }

            case FQLOperand::DOUBLE_VALUE:
            {
                if (lhs.isArrayType())
                {
                    return _CompareArray(
                        lhs.getArrayDoubleValue(),
                        rhs.getArrayDoubleValue(),
                        op);
                }
                else
                {
                    return _CompareScalar(
                        lhs.getDoubleValue(),
                        rhs.getDoubleValue(),
                        op);
                }
            }

            case FQLOperand::BOOLEAN_VALUE:
            {
                if (lhs.isArrayType())
                {
                    return _CompareArray(
                        lhs.getArrayBooleanValue(),
                        rhs.getArrayBooleanValue(),
                        op);
                }
                else
                {
                    return _CompareScalar(
                        lhs.getBooleanValue(),
                        rhs.getBooleanValue(),
                        op);
                }
            }

            case FQLOperand::STRING_VALUE:
            {
                // Future, build these into standard evaluate.
                if (op == FQL_LIKE )
                {
                    FQLRegularExpression regex(rhs.getStringValue());
                    return regex.match(lhs.getStringValue());
                }
                if (op == FQL_NOT_LIKE )
                {
                    FQLRegularExpression regex(rhs.getStringValue());
                    return !regex.match(lhs.getStringValue());
                }
                if (lhs.isArrayType())
                {
                    return _CompareArray(
                        lhs.getArrayStringValue(),
                        rhs.getArrayStringValue(),
                        op);
                }
                else
                {
                    return _CompareScalar(
                        lhs.getStringValue(),
                        rhs.getStringValue(),
                        op);
                }
            }

            // In this case, the right hand might still be String and we
            // must convert
            // KS_TODO Review this in detail
            case FQLOperand::DATETIME_VALUE:
            {
                if (lhs.isArrayType())
                {
                    return _CompareArray(
                        lhs.getArrayDateTimeValue(),
                        rhs.getArrayDateTimeValue(),
                        op);
                }
                else
                {
                    return _CompareScalar(
                        lhs.getDateTimeValue(),
                        rhs.getDateTimeValue(),
                        op);
                }
            }
            // In this case, the right hand might still be String and we
            // must convert
            case FQLOperand::REFERENCE_VALUE:
            {
                if (lhs.isArrayType())
                {
                    return _CompareArray(
                        lhs.getArrayReferenceValue(),
                        rhs.getArrayReferenceValue(),
                        op);
                }
                else
                {
                    return _CompareScalar(
                        lhs.getReferenceValue(),
                        rhs.getReferenceValue(),
                        op);
                }
            }
            case FQLOperand::PROPERTY_NAME:
                PEGASUS_ASSERT(0);
        }

    }

    // Catch the specific IndexOutOf range set by FQLOperand.h
    catch (IndexOutOfBoundsException)
    {
#ifdef FQL_DOTRACE
        DCOUT << "Caught index out of Bounds Exception" << endl;
#endif
        return false;
    }


    return false;
}

/*
    Dynamically resolve rhs operands that cannot be resolved
    at compile and also tests for NULL lhs.
    Return  true if null lhs. Otherwise thries
*/
void _fixNotEqualOperands(
   FQLOperand& lhs,
   FQLOperand& rhs,
   Uint32 rhsIndex,
   Array<FQLOperand>& _queryOperands)
{
    // Resolve the Datetime and reference type
    // dynamically because there is no way to know the types
    // at query compile time. If the rhs is a string and the
    // lhs is either datetime or reference, try to map the
    // rhs string
    if(rhs.getType() == FQLOperand::STRING_VALUE)
    {
#ifdef FQL_DOTRACE
        DCOUT << "fix NotQualOperands "
            << " lhs " << lhs.toString()
            << " rhs " << rhs.toString()
            << " rhsIndex " << rhsIndex
            << endl;
#endif
       if (lhs.getType() == FQLOperand::DATETIME_VALUE)
       {
            // Replace the entry in the operands list
            // with the correct type
            if (!rhs.isArray())
            {
                _queryOperands[rhsIndex] =
                    FQLOperand(rhs.getStringValue(),
                    FQL_DATETIME_VALUE_TAG);
            }
            else
            {
                _queryOperands[rhsIndex] =
                    FQLOperand(rhs.getArrayStringValue(),
                    FQL_DATETIME_VALUE_TAG);
            }
        }
        else if (lhs.getType() == FQLOperand::REFERENCE_VALUE)
        {
            if (!rhs.isArray())
            {
                _queryOperands[rhsIndex] =
                    FQLOperand(rhs.getStringValue(),
                    FQL_REFERENCE_VALUE_TAG);
            }
            else
            {
                _queryOperands[rhsIndex] =
                    FQLOperand(rhs.getArrayStringValue(),
                    FQL_REFERENCE_VALUE_TAG);
            }
        }
        else
        {
            throw TypeMismatchException(
                "Operand types not the same");
        }
    }
}

FQLQueryStatementRep::FQLQueryStatementRep(
    const CIMClass& queryClass,
    const String& query)
    :  _query(query)
{
    _operations.reserveCapacity(16);
    _operands.reserveCapacity(16);
}

FQLQueryStatementRep::FQLQueryStatementRep(
    const String& query)
    : _query(query)
{
    _operations.reserveCapacity(16);
    _operands.reserveCapacity(16);
}


FQLQueryStatementRep::FQLQueryStatementRep()
{
    //
    // Reserve space for a query clause with up to sixteen terms.
    //

    _operations.reserveCapacity(16);
    _operands.reserveCapacity(16);
}

FQLQueryStatementRep::FQLQueryStatementRep(const FQLQueryStatementRep& rep)
    :
    _query(rep._query),
    _queryPropertyNames(rep._queryPropertyNames),
    _operations(rep._operations),
    _operands(rep._operands)
{
}

FQLQueryStatementRep::~FQLQueryStatementRep()
{

}

void FQLQueryStatementRep::clear()
{
    _queryPropertyNames.clear();
    _operations.clear();
    _operands.clear();
}


const CIMPropertyList FQLQueryStatementRep::getQueryPropertyList(
    const CIMObjectPath& inClassName) const
{
    //
    //  Return CIMPropertyList for properties referenced in the FQL
    //  statement
    //  The list may be empty, but may not be NULL
    //
    return CIMPropertyList (_queryPropertyNames);
}

Boolean FQLQueryStatementRep::appendQueryPropertyName(const CIMName& x)
{
    //
    // Reject duplicate property names by returning false.
    //

    for (Uint32 i = 0, n = _queryPropertyNames.size(); i < n; i++)
    {
        if (_queryPropertyNames[i] == x)
        {
                return false;
        }
    }
    // Append the new property.

    _queryPropertyNames.append(x);
    return true;
}

static inline void _ResolveProperty(
    FQLOperand& op,
    const FQLPropertySource* source)
{
    //
    // Resolve the operand: if it's a property name, look up its value,
    // confirm, set correct type.
    // If dot property, get chained properties.

#ifdef FQL_DOTRACE
    DCOUT << "_ResolveProperty " << op.toString()
         << " type= " << op.getType() << " constant= "
         << FQLOperand::PROPERTY_NAME << endl;
#endif

    if (op.getType() == FQLOperand::PROPERTY_NAME)
    {
        unsigned int indexValue;
        bool isIndexed = op.isIndexedProperty();
        if (isIndexed)
        {
            indexValue = op.propertyIndex();
        }
        String propertyName = op.getPropertyName();

        // If the source does not exist. Set to NULL This is consistent
        // with the DMTF specification.
        // TODO do this in one step, not two. getValue or getIndexedValue

        if (!source->getValue(propertyName, op))
        {
#ifdef FQL_DOTRACE
            DCOUT << "_resolveProperty propertyName= "
                << propertyName
                << " did not get value. FQLOperand="
                << op.toString() << endl;
#endif
            op = FQLOperand();
        }
        else
        {
            // If this is indexed, need to get the value itself from
            // the property.
            // NOTE: Determine if this is an array property
            // ERROR: We are using op after it has been modified above.
#ifdef FQL_DOTRACE
            DCOUT << "_ResolveProperty.isIndexedProperty got value. op = "
                << op.toString()
                << " isIndexedProperty "
                << (op.isIndexedProperty()?"true" : "false") << endl;
#endif
            if (isIndexed)
            {
                FQLOperand fqlOp;
                // KS_TODO - Important. This returns a boolean return
                fqlOp.getIndexedValue(op,indexValue);
                op = fqlOp;

#ifdef FQL_DOTRACE
                DCOUT << "_ResolveProperty.isIndexedProperty Result"
                     << " Return op= "<< op.toString()
                     << " fqlOp= " << fqlOp.toString()<< endl;
#endif
            }
        }
    }
}
/*
    Evalutate the Query using the _operations list and _operands list.
    This process the operations list and process operands which depend
    on the operation type.
*/
Boolean FQLQueryStatementRep::evaluateQuery(
    const FQLPropertySource* source) const
{
    Stack<Boolean> stack;
    stack.reserveCapacity(16);

    //
    // Counter for operands:
    //

    Uint32 j = 0;

    //
    // Process each of the operations:
    //

    bool allComparesAllowed = false;
    // This bool determines whether this is ANY/NOT EVERY
    // or EVERY/NOT_ANY.  Preset to false indicate that
    // this is EVERY/ NOT_ANY
    bool anyOpType = false;
    bool isNotType = false;

    for (Uint32 i = 0, n = _operations.size(); i < n; i++)
    {
        FQLOperation operation = _operations[i];

        switch (operation)
        {
            case FQL_UNKNOWN:
                PEGASUS_ASSERT(false);
                break;

            case FQL_OR:
            {
                PEGASUS_ASSERT(stack.size() >= 2);

                Boolean operand1 = stack.top();
                stack.pop();

                Boolean operand2 = stack.top();

                stack.top() = operand1 || operand2;
                break;
            }

            case FQL_AND:
            {
                PEGASUS_ASSERT(stack.size() >= 2);

                Boolean operand1 = stack.top();
                stack.pop();

                Boolean operand2 = stack.top();

                stack.top() = operand1 && operand2;
                break;
            }

            case FQL_NOT:
            {
                PEGASUS_ASSERT(stack.size() >= 1);

                Boolean operand = stack.top();
                stack.top() = !operand;
                break;
            }

            // All of the following require 2 operands.
            // Confirm that the LHS and RHS types are always the same.
            case FQL_EQ:
            case FQL_NE:
            {
                // All operations allowed
                allComparesAllowed = true;
            }
            case FQL_LT:
            case FQL_LE:
            case FQL_GT:
            case FQL_GE:
            case FQL_LIKE:
            case FQL_NOT_LIKE:
            {
                Array<FQLOperand> queryOperands(_operands);
                PEGASUS_ASSERT(queryOperands.size() >= 2);

                //
                // Resolve the left-hand-side to a value (if not already
                // a value).
                //
                FQLOperand& lhs = queryOperands[j++];
#ifdef FQL_DOTRACE
                DCOUT << "Before resolution lhs " << lhs.toString()
                    << " Operand index= " << (j-1) << endl;
#endif
                _ResolveProperty(lhs, source);
#ifdef FQL_DOTRACE
                DCOUT << "After resolution lhs " << lhs.toString() << endl;
#endif
                //
                // Resolve the right-hand-side to a value (if not already
                // a value).
                //
                Uint32 rhsIndex = j;
                FQLOperand& rhs = queryOperands[j++];

                _ResolveProperty(rhs, source);

                //
                // Check for a type mismatch:
                //
                if (rhs.getType() != lhs.getType())
                {
#ifdef FQL_DOTRACE
                    DCOUT << "Comparing: operation= "
                        << FQLOperationToString(operation)
                        << "\nlhs= " << lhs.toString()
                        << "\nrhs= " << rhs.toString() << endl;
#endif
                    // lhs NULL if property not found which means type
                    // mismatch.
                    if (lhs.getType() == FQLOperand::NULL_VALUE)
                    {
                        stack.push(_CompareLhsIsNull(operation));
                        break;
                    }
                    // We have to resolve the rhs Datetime and reference type
                    // dynamically because there is no way to know the types
                    // at query compile time. If the rhs is a string and the
                    // lhs is either datetime or reference, try to map the
                    // rhs string
                    _fixNotEqualOperands(lhs,rhs,rhsIndex,queryOperands);

                    if (lhs.getType() != rhs.getType())
                    {
                        throw TypeMismatchException(
                            "Operand types mismatched");
                    }
                }

                // test for matching array typness
                if (rhs.isArray() != lhs.isArray())
                {
                    throw TypeMismatchException(
                       "Cannot to match array and non-Array types");
                }

                // Only EQ and NE scalar compares against Boolean or
                // Reference type or arrays
                if (!allComparesAllowed)
                {
                    if ((lhs.getType() == FQLOperand::BOOLEAN_VALUE)
                        || (lhs.getType() == FQLOperand::REFERENCE_VALUE)
                        || lhs.isArray())
                    {
                        throw TypeMismatchException("Invalid Compare operator");
                    }
                }
                //
                // Now that the types are known to be alike, apply the
                // evaluation operation
                //
                stack.push(_Evaluate(lhs, rhs, operation));
                break;
            }

            case FQL_NOT_ANY:
            case FQL_NOT_EVERY:
                isNotType = true;

            case FQL_EVERY:
            case FQL_ANY:
                {
                // set to indicate ANY type operation
                if ((operation == FQL_ANY) || (operation == FQL_NOT_ANY))
                {
                    anyOpType = true;
                }
                Array<FQLOperand> queryOperands(_operands);
                PEGASUS_ASSERT(queryOperands.size() >= 2);

                /// Resolve the left hand size to get a value
                FQLOperand& lhs = queryOperands[j++];
                _ResolveProperty(lhs, source);

                //
                // Resolve the right-hand-side to a value (if not already
                // a value).
                //
                Uint32 rhsIndex = j;
                FQLOperand& rhs = queryOperands[j++];

                _ResolveProperty(rhs, source);

                // get the next operation which is the actual compare op.
                // NOTE: Need to check this one for one of the scalar
                // operations. The assert is OK since the compiler should
                // ever allow an incomplete statement
                PEGASUS_ASSERT(++i < n);
                FQLOperation compareOp = _operations[i];

                // Fail if lhs not array.
                if (!lhs.isArray())
                {
                    throw TypeMismatchException(
                        "Left side Cannot be Array Ty");
                }

                // Test for every entry in the lhs array
                bool evalResult;
                for (Uint32 k = 0; k < lhs.arraySize(); k++)
                {
                    FQLOperand lhsIndexedValue;

                    if (!lhsIndexedValue.getIndexedValue(lhs, k))
                    {
                        throw TypeMismatchException(
                            "Left side not indexed or index out of bounds");
                    }
                    if (rhs.getType() != lhsIndexedValue.getType())
                    {
                        _fixNotEqualOperands(lhs,rhs,rhsIndex,queryOperands);

                        if (rhs.getType() != lhsIndexedValue.getType())
                        {
                            throw TypeMismatchException(
                                "Operand types mismatched");
                        }
                    }

                    if (rhs.isArray() != lhsIndexedValue.isArray())
                    {
                        throw TypeMismatchException(
                           "Cannot match array and non-Array types");
                    }
#ifdef FQL_DOTRACE
                    DCOUT << "ANYEVERY Comparing Individual: operation= "
                        << FQLOperationToString(operation)
                        << " indexedValue= " << lhsIndexedValue.toString()
                        << " " << FQLOperationToString(compareOp)
                         << " rhs= " << rhs.toString() << endl;
#endif
                    evalResult = _Evaluate(lhsIndexedValue, rhs, compareOp);

                    if (anyOpType)   // If any result passes, stop
                    {
                        if (evalResult)
                        {
                            break;
                        }
                    }
                    else    // Every. If any eval fails, stop
                    {
                        if (!evalResult)
                        {
                            break;
                        }
                    }
                }   // end of for loop

                // NOT_ANY is negative of EVERY.
                if (isNotType)
                {
                    evalResult = !evalResult;
                }
                stack.push(evalResult);
                break;
            }

            case FQL_IS_NULL:
            {
                Array<FQLOperand> queryOperands(_operands);
                PEGASUS_ASSERT(queryOperands.size() >= 1);

                FQLOperand& operand = queryOperands[j++];

                _ResolveProperty(operand, source);
                stack.push(operand.getType() == FQLOperand::NULL_VALUE);
                break;
            }
        }
    }
    PEGASUS_ASSERT(stack.size() == 1);
    return stack.top();
}

void FQLQueryStatementRep::print() const
{
    cout << toString();
}

String FQLQueryStatementRep::toString() const
{
    //
    // Print the header:
    //
    String result;
    result.appendPrintf("FQLQueryStatement\n{\n");

    //
    // Print the operations:
    //

    for (Uint32 i = 0; i < _operations.size(); i++)
    {
        result.appendPrintf("    _operations[%u]: \"%s\"\n",
            // ((i == 0)? "\n" : ""),
            i, FQLOperationToString(_operations[i]));
    }

    //
    // Print the operands:
    //
    for (Uint32 i = 0; i < _operands.size(); i++)
    {
        result.appendPrintf("%s    _operands[%u]: \"%s\"\n",
            ((i == 0)? "\n" : ""), i,
             (const char*)_operands[i].toString().getCString());

        if (_operands[i].getType() == FQLOperand::PROPERTY_NAME)
        {
            Uint32 chainSize = _operands[i].chainSize();
            if (chainSize != 0)
            {
                for (Uint32 j = 0; j < chainSize ; j++)
                {
                    FQLOperand x = _operands[i].chainItem(j);
                    result.appendPrintf(
                        "    _operands[%u][%u]: \"%s\"\n", i, j,
                        (const char*)x.toString().getCString());
                }
            }
        }
    }
    result.append("}\n");
    return result;
}

Boolean FQLQueryStatementRep::evaluate(const CIMInstance& inCI)
{
    FQLInstancePropertySource source(inCI);

#ifndef FQL_DOTRACE
    return evaluateQuery(&source);
#else
    Boolean x = evaluateQuery(&source);
    DCOUT << "after evaluateQuery" << endl;
    return x;
#endif
}


PEGASUS_NAMESPACE_END
