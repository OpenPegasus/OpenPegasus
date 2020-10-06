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

#include <Pegasus/Common/InternalException.h>
#include "CQLSimplePredicate.h"
#include "CQLSimplePredicateRep.h"
//#include <Pegasus/CQL/CQLExpression.h>
#include <Pegasus/CQL/CQLFactory.h>
#include <Pegasus/Query/QueryCommon/QueryContext.h>
#include <Pegasus/Query/QueryCommon/QueryException.h>
#include <Pegasus/Common/Tracer.h>

PEGASUS_NAMESPACE_BEGIN

CQLSimplePredicateRep::CQLSimplePredicateRep(){
    _isSimple = true;
    _operator = NOOP;
}

CQLSimplePredicateRep::CQLSimplePredicateRep(const CQLExpression& inExpression)
{
    _leftSide = inExpression;
    _isSimple = true;
    _operator = NOOP;
}

CQLSimplePredicateRep::CQLSimplePredicateRep(const CQLExpression& inExpression,
                         ExpressionOpType inOperator)
{
    _leftSide = inExpression;
    _operator = inOperator;
    _isSimple = true;
}

CQLSimplePredicateRep::CQLSimplePredicateRep(
                         const CQLExpression& leftSideExpression,
                         const CQLExpression& rightSideExpression,
                         ExpressionOpType inOperator)
{
    _leftSide = leftSideExpression;
    _rightSide = rightSideExpression;
    _operator = inOperator;
    _isSimple = false;
}

CQLSimplePredicateRep::CQLSimplePredicateRep(const CQLSimplePredicateRep* rep)
{
    _leftSide = rep->_leftSide;
    _rightSide = rep->_rightSide;
    _operator = rep->_operator;
    _isSimple = rep->_isSimple;
}

Boolean CQLSimplePredicateRep::evaluate(CIMInstance CI, QueryContext& QueryCtx)
{
    PEG_METHOD_ENTER(TRC_CQL, "CQLSimplePredicateRep::evaluate");
    // Resolve the value of the left side
    CQLValue leftVal = _leftSide.resolveValue(CI, QueryCtx);

    // If there isn't a right side then operator must by IS_NULL
    // or IS_NOT_NULL
    if (isSimple())
    {
        PEGASUS_ASSERT(_operator == IS_NULL || _operator == IS_NOT_NULL);

        return (_operator == IS_NULL) ? leftVal.isNull() : !leftVal.isNull();
    }

    PEGASUS_ASSERT(_operator != IS_NULL && _operator != IS_NOT_NULL);

    if (_operator == ISA)
    {
        // Special processing for ISA.  The CQLValue on the right side of ISA
        // has a CQLChainedIdentifier with one element that contains
        // the class name.  We don't want to resolve the right side because
        // CQLValue would assume that a single element chained identifier
        // refers to an instance of the FROM class.
        if (!_rightSide.isSimpleValue())
        {
            MessageLoaderParms parms(
                "CQL.CQLSimplePredicateRep.RHS_ISA_NOT_SIMPLE",
                "The expression $0 on the right side of the ISA operator"
                    " must be a simple value.",
                _rightSide.toString());
            throw CQLRuntimeException(parms);
        }

        CQLValue isaRightVal =
             _rightSide.getTerms()[0].getFactors()[0].getValue();

        CQLChainedIdentifier isaRightId = isaRightVal.getChainedIdentifier();
        return leftVal.isa(isaRightId, QueryCtx);
    }

    if (_operator == LIKE)
    {
    // Special processing for LIKE.  The right side expression must be
    // a simple CQLValue (ie. not nested, and not a function), and
    // must be a literal.  Note that this code will test whether
    // the right side is a literal, but not whether it is a string
    // literal.
    if (!_rightSide.isSimpleValue())
    {
        MessageLoaderParms parms(
            "CQL.CQLSimplePredicateRep.RHS_LIKE_NOT_SIMPLE",
            "The expression $0 on the right side of the LIKE operator must "
                "be a simple value.",
            _rightSide.toString());
        throw CQLRuntimeException(parms);
    }

    CQLValue likeRightVal =
        _rightSide.getTerms()[0].getFactors()[0].getValue();
    if (!likeRightVal.isResolved())
    {
        MessageLoaderParms parms(
            "CQL.CQLSimplePredicateRep.RHS_LIKE_NOT_LITERAL",
            "The expression $0 on the right side of the LIKE operator must "
                "be a literal.",
            _rightSide.toString());
        throw CQLRuntimeException(parms);
    }

    return leftVal.like(likeRightVal);
    }

    // No special processing needed.
    // Resolve the value of the right side
    CQLValue rightVal = _rightSide.resolveValue(CI, QueryCtx);

    switch(_operator)
    {
        case LT:
            return leftVal < rightVal;
            break;

        case GT:
            return leftVal > rightVal;
            break;

        case LE:
            return leftVal <= rightVal;
            break;

        case GE:
            return leftVal >= rightVal;
            break;

        case EQ:
            return leftVal == rightVal;
            break;

        case NE:
            return leftVal != rightVal;
            break;

        case LIKE:
        case ISA:
            // Never get here due to special processing above.
            PEGASUS_UNREACHABLE(PEGASUS_ASSERT(false);)
        case IS_NULL:
        case IS_NOT_NULL:
        case NOOP:
            // Never get here due to the assert.
            break;
    }
    PEG_METHOD_EXIT();
    return true;  // keep the compiler happy
}

CQLExpression CQLSimplePredicateRep::getLeftExpression()const
{
    return _leftSide;
}

CQLExpression CQLSimplePredicateRep::getRightExpression()const
{
        return _rightSide;
}

enum ExpressionOpType CQLSimplePredicateRep::getOperation()const
{
    return _operator;
}

void CQLSimplePredicateRep::applyContext(const QueryContext& queryContext)
{
    PEG_METHOD_ENTER(TRC_CQL, "CQLSimplePredicateRep::applyContext");
    CQLIdentifier _id;

    _id = _leftSide.getTerms()[0].getFactors()[0].
                  getValue().getChainedIdentifier().getLastIdentifier();

    if(_leftSide.isSimpleValue() &&
        _id.isSymbolicConstant() &&
        _id.getName().getString().size() == 0)
    {
        // We have a standalone symbolic constant.
        if(!isSimple() &&
         _rightSide.isSimpleValue() &&
         _rightSide.getTerms()[0].getFactors()[0].
                getValue().getChainedIdentifier().getLastIdentifier().
                getName().getString().size() > 0)
        {
             _rightSide.applyContext(queryContext);

             // We need to add context to the symbolic constant
             _leftSide.applyContext(queryContext,
                                    _rightSide.getTerms()[0].getFactors()[0].
                                    getValue().getChainedIdentifier());
        }
        else
        {
             // There is no valid context for the symbolic constant
            MessageLoaderParms parms("CQL.CQLSimplePredicateRep."
             "RIGHT_STANDALONE_SYMCONST_ERROR",
             "The right side of predicate $0 must be a simple property "
                "name because a symbolic constant exists on the left side.",
             toString());
            throw CQLSyntaxErrorException(parms);
        }
    }
    else
    {
        _leftSide.applyContext(queryContext);
    }

    if (!isSimple())
    {
        _id = _rightSide.getTerms()[0].getFactors()[0].
                     getValue().getChainedIdentifier().getLastIdentifier();

        if(_rightSide.isSimpleValue() &&
            _id.isSymbolicConstant() &&
            _id.getName().getString().size() == 0)
        {
            // We have a standalone symbolic constant.
            if(!isSimple() &&
                _leftSide.isSimpleValue() &&
                _leftSide.getTerms()[0].getFactors()[0].
                    getValue().getChainedIdentifier().getLastIdentifier().
                    getName().getString().size() > 0)
            {
                // We need to add context to the symbolic constant
                _rightSide.applyContext(queryContext,
                    _leftSide.getTerms()[0].getFactors()[0].
                    getValue().getChainedIdentifier());
            }
            else
            {
                // There is no valid context for the symbolic constant
                MessageLoaderParms parms("CQL.CQLSimplePredicateRep."
                     "LEFT_STANDALONE_SYMCONST_ERROR",
                     "The left side of predicate $0 must be a simple property "
                           "name because a symbolic constant exists on"
                           " the right side.",
                     toString());
                throw CQLSyntaxErrorException(parms);
            }
        }

        else
        {
            // Right side is not simple OR it is a not a standalone
            // symbolic constant
            if (_operator != ISA)
            {
                // Not doing an ISA, go ahead and applyContext to right side
                _rightSide.applyContext(queryContext);
            }
            else
            {
                // Operation is an ISA.. The right side must be simple.
                // We don't want to applyContext to the right side because
                // it could be a classname unrelated to the FROM class.
                if (!_rightSide.isSimpleValue())
                {
                    MessageLoaderParms parms("CQL.CQLSimplePredicateRep."
                        "APPLY_CTX_RHS_ISA_NOT_SIMPLE",
                        "The right side expression $0 of the ISA operator "
                            "must be a simple value.",
                      _rightSide.toString());
                    throw CQLSyntaxErrorException(parms);
                }

                // Add the right side identifier to the list of WHERE
                // identifiers in the QueryContext
                QueryChainedIdentifier isaId = _rightSide.getTerms()[0].
                    getFactors()[0].
                    getValue().getChainedIdentifier();

                    queryContext.
                        addWhereIdentifier(isaId);
            }
        }
   }
   PEG_METHOD_EXIT();
}

String CQLSimplePredicateRep::toString()const
{
    String s = _leftSide.toString();
    if(!_isSimple)
    {
       switch(_operator)
       {
            case LT:
                s.append(" < ");
                break;
            case GT:
                s.append(" > ");
                break;
            case LE:
                s.append(" <= ");
                break;
            case GE:
                s.append(" >= ");
                break;
            case EQ:
                s.append(" = ");
                break;
            case NE:
                s.append(" <> ");
                break;
            case IS_NULL:
                s.append(" IS NULL ");
                break;
            case IS_NOT_NULL:
                s.append(" IS NOT NULL ");
                break;
            case ISA:
                s.append(" ISA ");
                break;
            case LIKE:
                s.append(" LIKE ");
                break;
            case NOOP:
                break;
       }
       s.append(_rightSide.toString());
    }
    switch(_operator)
    {
        case IS_NULL:
            s.append(" IS NULL ");
            break;
        case IS_NOT_NULL:
            s.append(" IS NOT NULL ");
            break;
        default:
            break;
    }
    return s;
}
Boolean CQLSimplePredicateRep::isSimple()const{
    return _isSimple;
}
Boolean CQLSimplePredicateRep::isSimpleValue()const{
    return _leftSide.isSimpleValue();
}

void CQLSimplePredicateRep::setOperation(ExpressionOpType op){
    _operator = op;
}

PEGASUS_NAMESPACE_END
