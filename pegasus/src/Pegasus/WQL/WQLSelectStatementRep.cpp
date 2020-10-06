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
#include "WQLSelectStatementRep.h"
#include <Pegasus/Query/QueryCommon/QueryContext.h>
#include <Pegasus/Query/QueryCommon/QueryException.h>
#include "WQLInstancePropertySource.h"
PEGASUS_USING_STD;

PEGASUS_NAMESPACE_BEGIN

template<class T>
inline static Boolean _Compare(const T& x, const T& y, WQLOperation op)
{
    switch (op)
    {
        case WQL_EQ:
            return x == y;

        case WQL_NE:
            return x != y;

        case WQL_LT:
            return x < y;
        case WQL_LE:
            return x <= y;

        case WQL_GT:
            return x > y;

        case WQL_GE:
            return x >= y;

        default:
            PEGASUS_UNREACHABLE(PEGASUS_ASSERT(0);)
    }

    return false;
}

static Boolean _Evaluate(
    const WQLOperand& lhs,
    const WQLOperand& rhs,
    WQLOperation op)
{
    switch (lhs.getType())
    {
        case WQLOperand::NULL_VALUE:
        {
#ifdef PEGASUS_SNIA_EXTENSIONS
            return (rhs.getType() == WQLOperand::NULL_VALUE);
#else
            // This cannot happen since expressions of the form
            // OPERAND OPERATOR NULL are converted to unary form.
            // For example: "count IS NULL" is treated as a unary
            // operation in which IS_NULL is the unary operation
            // and count is the the unary operand.

            PEGASUS_UNREACHABLE(PEGASUS_ASSERT(0);)
            break;
#endif
        }

        case WQLOperand::INTEGER_VALUE:
        {
            return _Compare(
            lhs.getIntegerValue(),
            rhs.getIntegerValue(),
            op);
        }

        case WQLOperand::DOUBLE_VALUE:
        {
            return _Compare(
            lhs.getDoubleValue(),
            rhs.getDoubleValue(),
            op);
        }

        case WQLOperand::BOOLEAN_VALUE:
        {
            return _Compare(
            lhs.getBooleanValue(),
            rhs.getBooleanValue(),
            op);
        }

        case WQLOperand::STRING_VALUE:
        {
            return _Compare(
            lhs.getStringValue(),
            rhs.getStringValue(),
            op);
        }

        default:
            PEGASUS_UNREACHABLE(PEGASUS_ASSERT(0);)
    }

    return false;
}

WQLSelectStatementRep::WQLSelectStatementRep(
    const String& queryLang,
    const String& query)
    :SelectStatementRep(queryLang,query)
{
    _operations.reserveCapacity(16);
    _operands.reserveCapacity(16);

    _allProperties = false;
}

WQLSelectStatementRep::WQLSelectStatementRep(
    const String& queryLang,
    const String& query,
    const QueryContext& inCtx)
    : SelectStatementRep(queryLang, query, inCtx)
{
    _operations.reserveCapacity(16);
    _operands.reserveCapacity(16);

    _allProperties = false;
}

WQLSelectStatementRep::WQLSelectStatementRep()
    :SelectStatementRep()
{
    //
    // Reserve space for a where clause with up to sixteen terms.
    //

    _operations.reserveCapacity(16);
    _operands.reserveCapacity(16);

    _allProperties = false;
}

WQLSelectStatementRep::WQLSelectStatementRep(const WQLSelectStatementRep& rep)
    : SelectStatementRep(rep),
    _className(rep._className),
    _allProperties(rep._allProperties),
    _selectPropertyNames(rep._selectPropertyNames),
    _wherePropertyNames(rep._wherePropertyNames),
    _operations(rep._operations),
    _operands(rep._operands)
{
}

WQLSelectStatementRep::~WQLSelectStatementRep()
{

}

void WQLSelectStatementRep::clear()
{
    _className.clear();
    _allProperties = false;
    _selectPropertyNames.clear();
    _operations.clear();
    _operands.clear();
}

Boolean WQLSelectStatementRep::getAllProperties() const
{
    return _allProperties;
}

void WQLSelectStatementRep::setAllProperties(const Boolean allProperties)
{
    _allProperties = allProperties;
}

const CIMPropertyList WQLSelectStatementRep::getSelectPropertyList(
    const CIMObjectPath& inClassName) const
{
    //
    //  Check for "*"
    //
    if (_allProperties)
    {
        //
        //  Return null CIMPropertyList for all properties
        //
        return CIMPropertyList ();
    }

    CIMName className = inClassName.getClassName();
    if (className.isNull())
    {
        //
        //  If the caller passed in an empty className, then the FROM class is
        //  to be used
        //
        className = _className;
    }

    //
    //  Check if inClassName is the FROM class
    //
    if (!(className == _className))
    {
        //
        //  Check for NULL Query Context
        //
        if (_ctx == NULL)
        {
            MessageLoaderParms parms
                ("WQL.WQLSelectStatementRep.QUERY_CONTEXT_IS_NULL",
                "Trying to process a query with a NULL Query Context.");
            throw QueryRuntimeException(parms);
        }

        //
        //  Check if inClassName is a subclass of the FROM class
        //
        if (!_ctx->isSubClass(_className,className))
        {
            MessageLoaderParms parms
                ("WQL.WQLSelectStatementRep.CLASS_NOT_FROM_LIST_CLASS",
                "Class $0 does not match the FROM class or any of its "
                "subclasses.",
                className.getString());
            throw QueryRuntimeException(parms);
        }
    }

    //
    //  Return CIMPropertyList for properties referenced in the projection
    //  list (SELECT clause)
    //
    return CIMPropertyList (_selectPropertyNames);
}

const CIMPropertyList WQLSelectStatementRep::getWherePropertyList(
    const CIMObjectPath& inClassName) const
{
    CIMName className = inClassName.getClassName();
    if (className.isNull())
    {
        //
        //  If the caller passed in an empty className, then the FROM class is
        //  to be used
        //
        className = _className;
    }

    //
    //  Check if inClassName is the FROM class
    //
    if (!(className == _className))
    {
        //
        //  Check for NULL Query Context
        //
        if (_ctx == NULL)
        {
            MessageLoaderParms parms
                ("WQL.WQLSelectStatementRep.QUERY_CONTEXT_IS_NULL",
                "Trying to process a query with a NULL Query Context.");
            throw QueryRuntimeException(parms);
        }

        //
        //  Check if inClassName is a subclass of the FROM class
        //
        if (!_ctx->isSubClass(_className,className))
        {
            MessageLoaderParms parms
                ("WQL.WQLSelectStatementRep.CLASS_NOT_FROM_LIST_CLASS",
                "Class $0 does not match the FROM class or any of its "
                "subclasses.",
                className.getString());
            throw QueryRuntimeException(parms);
        }
    }

    //
    //  Return CIMPropertyList for properties referenced in the condition
    //  (WHERE clause)
    //  The list may be empty, but may not be NULL
    //
    return CIMPropertyList (_wherePropertyNames);
}

Boolean WQLSelectStatementRep::appendWherePropertyName(const CIMName& x)
{
    //
    // Reject duplicate property names by returning false.
    //

    for (Uint32 i = 0, n = _wherePropertyNames.size(); i < n; i++)
    {
    if (_wherePropertyNames[i] == x)
        return false;
    }

    //
    // Append the new property.
    //

    _wherePropertyNames.append(x);
    return true;
}

static inline void _ResolveProperty(
    WQLOperand& op,
    const WQLPropertySource* source)
{
    //
    // Resolve the operand: if it's a property name, look up its value:
    //

    if (op.getType() == WQLOperand::PROPERTY_NAME)
    {
        const CIMName& propertyName = op.getPropertyName();

        if (!source->getValue(propertyName, op))
            op = WQLOperand();
    }
}

Boolean WQLSelectStatementRep::evaluateWhereClause(
    const WQLPropertySource* source) const
{
    if (!hasWhereClause())
    return true;

    Stack<Boolean> stack;
    stack.reserveCapacity(16);

    //
    // Counter for operands:
    //

    Uint32 j = 0;

    //
    // Process each of the operations:
    //

    for (Uint32 i = 0, n = _operations.size(); i < n; i++)
    {
    WQLOperation operation = _operations[i];

    switch (operation)
    {
        case WQL_OR:
        {
            PEGASUS_ASSERT(stack.size() >= 2);

            Boolean operand1 = stack.top();
            stack.pop();

            Boolean operand2 = stack.top();

            stack.top() = operand1 || operand2;
            break;
        }

        case WQL_AND:
        {
            PEGASUS_ASSERT(stack.size() >= 2);

            Boolean operand1 = stack.top();
            stack.pop();

            Boolean operand2 = stack.top();

            stack.top() = operand1 && operand2;
            break;
        }

        case WQL_NOT:
        {
            PEGASUS_ASSERT(stack.size() >= 1);

            Boolean operand = stack.top();
            stack.top() = !operand;
            break;
        }

        case WQL_EQ:
        case WQL_NE:
        case WQL_LT:
        case WQL_LE:
        case WQL_GT:
        case WQL_GE:
        {
            Array<WQLOperand> whereOperands(_operands);
            PEGASUS_ASSERT(whereOperands.size() >= 2);

            //
            // Resolve the left-hand-side to a value (if not already
            // a value).
            //

            WQLOperand& lhs = whereOperands[j++];
            _ResolveProperty(lhs, source);

            //
            // Resolve the right-hand-side to a value (if not already
            // a value).
            //

            WQLOperand& rhs = whereOperands[j++];
            _ResolveProperty(rhs, source);

            //
            // Check for a type mismatch:
            //

            // PEGASUS_OUT(lhs.toString());
            // PEGASUS_OUT(rhs.toString());

            if (rhs.getType() != lhs.getType())
                throw TypeMismatchException();

            //
            // Now that the types are known to be alike, apply the
            // operation:
            //

            stack.push(_Evaluate(lhs, rhs, operation));
            break;
        }

        case WQL_IS_TRUE:
        case WQL_IS_NOT_FALSE:
        {
            PEGASUS_ASSERT(stack.size() >= 1);
            break;
        }

        case WQL_IS_FALSE:
        case WQL_IS_NOT_TRUE:
        {
            PEGASUS_ASSERT(stack.size() >= 1);
            stack.top() = !stack.top();
            break;
        }

        case WQL_IS_NULL:
        {
            Array<WQLOperand> whereOperands(_operands);
            PEGASUS_ASSERT(whereOperands.size() >= 1);
            WQLOperand& operand = whereOperands[j++];
            _ResolveProperty(operand, source);
            stack.push(operand.getType() == WQLOperand::NULL_VALUE);
            break;
        }

        case WQL_IS_NOT_NULL:
        {
            Array<WQLOperand> whereOperands(_operands);
            PEGASUS_ASSERT(whereOperands.size() >= 1);
            WQLOperand& operand = whereOperands[j++];
            _ResolveProperty(operand, source);
            stack.push(operand.getType() != WQLOperand::NULL_VALUE);
            break;
        }
    }
    }

    PEGASUS_ASSERT(stack.size() == 1);
    return stack.top();
}

template<class T>
inline void wqlSelectStatementApplyProjection(
    T& object,
    Boolean allowMissing,
    const Array<CIMName>& selectPropertyNames)
{
    for (int i=object.getPropertyCount(); i!=0; i--)
    {
        CIMName pn=object.getProperty(i-1).getName();
        Boolean foundInSel = false;
        for (int ii=0,mm=selectPropertyNames.size(); ii<mm; ii++)
        {
            if (selectPropertyNames[ii]==pn)
            {
               foundInSel = true;
               break;
            }
        }

        if (!foundInSel)
        {
            object.removeProperty(i-1);
        }
    }

    //check for properties on select list missing from the instance
    if (!allowMissing)
    {
        Boolean foundInInst;
        for (Uint32 i=0; i < selectPropertyNames.size(); i++)
        {
            foundInInst = false;
            CIMName sn=selectPropertyNames[i];
            for (Uint32 j = object.getPropertyCount(); j != 0; j--)
            {
                CIMName in = object.getProperty(j-1).getName();
                if (sn == in) foundInInst = true;
            }

            if(!foundInInst)
            {
                MessageLoaderParms parms
                    ("WQL.WQLSelectStatementRep.MISSING_PROPERTY_ON_INSTANCE",
                    "A property in the Select list is missing from the "
                    "instance");
                throw QueryRuntimePropertyException(parms);
            }
        }
    }
}

void WQLSelectStatementRep::applyProjection(
    CIMInstance& ci,
    Boolean allowMissing)
{
    if (_allProperties)
    {
        return;
    }

    wqlSelectStatementApplyProjection(ci, allowMissing, _selectPropertyNames);
}

void WQLSelectStatementRep::applyProjection(
    CIMObject& ci,
    Boolean allowMissing)
{
    if (_allProperties)
    {
        return;
    }

    wqlSelectStatementApplyProjection(ci, allowMissing, _selectPropertyNames);
}

void WQLSelectStatementRep::print() const
{
    //
    // Print the header:
    //

    cout << "WQLSelectStatement" << endl;
    cout << "{" << endl;

    //
    // Print the class name:
    //

    cout << "    _className: \"" << _className.getString() << '"' << endl;

    //
    // Print the select properties:
    //

    if (_allProperties)
    {
        cout << endl;
        cout << "    _allProperties: TRUE" << endl;
    }

    else for (Uint32 i = 0; i < _selectPropertyNames.size(); i++)
    {
    if (i == 0)
        cout << endl;

    cout << "    _selectPropertyNames[" << i << "]: ";
    cout << '"' << _selectPropertyNames[i].getString() << '"' << endl;
    }

    //
    // Print the operations:
    //

    for (Uint32 i = 0; i < _operations.size(); i++)
    {
        if (i == 0)
            cout << endl;

        cout << "    _operations[" << i << "]: ";
        cout << '"' << WQLOperationToString(_operations[i]) << '"' << endl;
    }

    //
    // Print the operands:
    //

    for (Uint32 i = 0; i < _operands.size(); i++)
    {
        if (i == 0)
        cout << endl;

    cout << "    _operands[" << i << "]: ";
    cout << '"' << _operands[i].toString() << '"' << endl;
    }

    //
    // Print the trailer:
    //

    cout << "}" << endl;
}

Boolean WQLSelectStatementRep::evaluate(const CIMInstance& inCI)
{
    WQLInstancePropertySource source(inCI);
    return evaluateWhereClause(&source);
}

void WQLSelectStatementRep::validate()
{
    if(_ctx == NULL){
        MessageLoaderParms parms(
            "WQL.WQLSelectStatementRep.QUERY_CONTEXT_IS_NULL",
            "Trying to process a query with a NULL Query Context.");
      throw QueryValidationException(parms);
   }
    CIMClass fromClass;
    try
   {
     fromClass = _ctx->getClass(_className);

    CIMObjectPath className (String::EMPTY, _ctx->getNamespace (), _className);
     Array<CIMName> whereProps =
        getWherePropertyList(className).getPropertyNameArray();
     Array<CIMName> selectProps =
        getSelectPropertyList(className).getPropertyNameArray();

     // make sure all properties match properties on the from class
     for(Uint32 i = 0; i < whereProps.size(); i++){
         Uint32 index = fromClass.findProperty(whereProps[i]);
            if(index == PEG_NOT_FOUND){
                MessageLoaderParms parms(
                    "WQL.WQLSelectStatementRep.PROP_NOT_FOUND",
                    "The property $0 was not found in the FROM class $1",
                    whereProps[i].getString(),
                    fromClass.getClassName().getString());
            throw QueryMissingPropertyException(parms);
            }
         else
         {
           //
           //  Property exists in class
           //  Verify it is not an array property
           //
           CIMProperty classProperty = fromClass.getProperty(index);
           if (classProperty.isArray ())
           {
             MessageLoaderParms parms(
                 "WQL.WQLSelectStatementRep.WHERE_PROP_IS_ARRAY",
                 "Array property $0 is not supported in the WQL WHERE clause.",
                 whereProps[i].getString());
             throw QueryValidationException(parms);
           }
         }
        }

     for(Uint32 i = 0; i < selectProps.size(); i++){
       if(fromClass.findProperty(selectProps[i]) == PEG_NOT_FOUND){
         MessageLoaderParms parms(
             "WQL.WQLSelectStatementRep.PROP_NOT_FOUND",
             "The property $0 was not found in the FROM class $1",
             selectProps[i].getString(),
             fromClass.getClassName().getString());
         throw QueryMissingPropertyException(parms);
       }
     }
   }
   catch (const CIMException& ce)
   {
     if (ce.getCode() == CIM_ERR_INVALID_CLASS ||
         ce.getCode() == CIM_ERR_NOT_FOUND)
     {
       MessageLoaderParms parms(
           "WQL.WQLSelectStatementRep.CLASSNAME_NOT_IN_REPOSITORY",
           "The class name $0 was not found in the repository.",
           _className.getString());
       throw QueryValidationException(parms);
     }
     else
     {
       throw;
     }
   }
}

CIMPropertyList WQLSelectStatementRep::getPropertyList(
        const CIMObjectPath& inClassName)
{
    if(_ctx == NULL){
        MessageLoaderParms parms(
            "WQL.WQLSelectStatementRep.QUERY_CONTEXT_IS_NULL",
            "Trying to process a query with a NULL Query Context.");
      throw QueryRuntimeException(parms);
    }

    if(_allProperties)
     return CIMPropertyList();

    CIMName className = inClassName.getClassName();
    if (className.isNull())
    {
     // If the caller passed in an empty className, then the
     // FROM class is to be used.
     className = _className;
    }

    // check if inClassName is the From class
    if(!(className == _className)){
        // check if inClassName is a subclass of the From class
        if(!_ctx->isSubClass(_className,className)){
            MessageLoaderParms parms(
                "WQL.WQLSelectStatementRep.CLASS_NOT_FROM_LIST_CLASS",
                "Class $0 does not match the FROM class or any of its"
                    " subclasses.",
                className.getString());
            throw QueryRuntimeException(parms);
        }
    }

    Array<CIMName> names =
            getWherePropertyList(inClassName).getPropertyNameArray();
    Array<CIMName> selectList =
            getSelectPropertyList(inClassName).getPropertyNameArray();

    // check for duplicates and remove them
    for(Uint32 i = 0; i < names.size(); i++){
        for(Uint32 j = 0; j < selectList.size(); j++){
            if(names[i] == selectList[j])
                selectList.remove(j);
            }
    }

    names.appendArray(selectList);
    CIMPropertyList list = CIMPropertyList();
    list.set(names);
    return list;
}

Array<CIMObjectPath> WQLSelectStatementRep::getClassPathList() const
{
    if(_ctx == NULL){
        MessageLoaderParms parms(
            "WQL.WQLSelectStatementRep.QUERY_CONTEXT_IS_NULL",
            "Trying to process a query with a NULL Query Context.");
      throw QueryRuntimeException(parms);
   }
    CIMObjectPath path(String::EMPTY, _ctx->getNamespace(), _className);
    Array<CIMObjectPath> paths;
    paths.append(path);
    return paths;
}

PEGASUS_NAMESPACE_END
