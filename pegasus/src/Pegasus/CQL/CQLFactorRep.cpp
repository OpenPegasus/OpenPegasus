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

#include "CQLFactorRep.h"
#include <Pegasus/CQL/CQLExpression.h>
#include <Pegasus/CQL/CQLFunction.h>
#include <Pegasus/CQL/CQLFactory.h>
#include <Pegasus/CQL/CQLValue.h>
#include <Pegasus/Query/QueryCommon/QueryContext.h>
#include <Pegasus/Common/Tracer.h>

PEGASUS_NAMESPACE_BEGIN

CQLFactorRep::CQLFactorRep():
   _CQLExp(),
    _CQLVal(),
    _CQLFunct(),
    _invert(false),
    _simpleValue(false),
    _containedType(VALUE)
{

}

CQLFactorRep::CQLFactorRep(const CQLFactorRep* rep):
    _CQLExp(rep->_CQLExp),
    _CQLVal(rep->_CQLVal),
    _CQLFunct(rep->_CQLFunct),
    _invert(rep->_invert),
    _simpleValue(rep->_simpleValue),
    _containedType(rep->_containedType)

{
  PEG_METHOD_ENTER(TRC_CQL,
          "CQLFactorRep::CQLFactorRep(const CQLFactorRep* rep)");

  PEG_METHOD_EXIT();
}

CQLFactorRep::CQLFactorRep(const CQLValue& inCQLVal):
    _CQLExp(),
    _CQLVal(inCQLVal),
    _CQLFunct(),
    _invert(false),
    _simpleValue(true),
    _containedType(VALUE)

{
  PEG_METHOD_ENTER(TRC_CQL,
          "CQLFactorRep::CQLFactorRep(const CQLValue& inCQLVal)");

  PEG_METHOD_EXIT();
}

CQLFactorRep::CQLFactorRep(const CQLExpression& inCQLExp):
    _CQLExp(inCQLExp),
    _CQLVal(),
    _CQLFunct(),
    _invert(false),
    _simpleValue(false),
    _containedType(EXPRESSION)
{
  PEG_METHOD_ENTER(TRC_CQL,
          "CQLFactorRep::CQLFactorRep(const CQLExpression& inCQLExp)");

  PEG_METHOD_EXIT();
}

CQLFactorRep::CQLFactorRep(const CQLFunction& inCQLFunc):
    _CQLExp(),
    _CQLVal(),
    _CQLFunct(inCQLFunc),
    _invert(false),
    _simpleValue(false),
    _containedType(FUNCTION)
{
  PEG_METHOD_ENTER(TRC_CQL,
          "CQLFactorRep::CQLFactorRep(const CQLFunction& inCQLFunc)");

    PEG_METHOD_EXIT();
}

CQLValue CQLFactorRep::getValue()const
{
    return _CQLVal;
}

CQLValue CQLFactorRep::resolveValue(const CIMInstance& CI,
                                    const QueryContext& QueryCtx)
{
    PEG_METHOD_ENTER(TRC_CQL,"CQLFactorRep::resolveValue()");

    if(_containedType == EXPRESSION)
    {
        PEG_METHOD_EXIT();
        return _CQLExp.resolveValue(CI,QueryCtx);
    }
    else if (_containedType == FUNCTION)
    {
        PEG_METHOD_EXIT();
        return _CQLFunct.resolveValue(CI,QueryCtx);
    }
    else if (_containedType == VALUE)
    {
        _CQLVal.resolve(CI,QueryCtx);
        PEG_METHOD_EXIT();
        return _CQLVal;
    }
    else
    {
        MessageLoaderParms msg("CQL.CQLFactorRep.FACTOR_HAS_NO_TYPE",
                              "The CQLFactor was constructed without a type.");
        throw CQLRuntimeException(msg);
    }
}

Boolean CQLFactorRep::isSimple()const
{
   return _simpleValue;
}

Boolean CQLFactorRep::isSimpleValue()const
{
   return _simpleValue;
}

CQLFunction CQLFactorRep::getCQLFunction()const
{
   return _CQLFunct;
}

CQLExpression CQLFactorRep::getCQLExpression()const
{
   return _CQLExp;
}

String CQLFactorRep::toString()const
{
    PEG_METHOD_ENTER(TRC_CQL,"CQLFactorRep::toString()");

    if(_containedType == VALUE)
    {
        PEG_METHOD_EXIT();
        return _CQLVal.toString();
    }

    if(_containedType == FUNCTION)
    {
        PEG_METHOD_EXIT();
        return _CQLFunct.toString();
    }
    if (_containedType == EXPRESSION)
    {
        PEG_METHOD_EXIT();
        return _CQLExp.toString();
    }
    return String("");
}

void CQLFactorRep::applyContext(const QueryContext& inContext,
                                const CQLChainedIdentifier& inCid)
{
    PEG_METHOD_ENTER(TRC_CQL,"CQLFactorRep::applyContext()");

    if(_containedType == FUNCTION)
    {
        _CQLFunct.applyContext(inContext);
    }
    else if(_containedType == EXPRESSION)
    {
        _CQLExp.applyContext(inContext);
    }
    else if(_containedType == VALUE)
    {
        _CQLVal.applyContext(inContext,inCid);
    }
    else
    {
        MessageLoaderParms msg("CQL.CQLFactorRep.FACTOR_HAS_NO_TYPE",
                             "The CQLFactor was constructed without a type.");
        throw CQLRuntimeException(msg);
    }

    PEG_METHOD_EXIT();
    return;
}
/*
Boolean CQLFactorRep::operator==(const CQLFactorRep& rep)const
{
    PEG_METHOD_ENTER(TRC_CQL,"CQLFactorRep::operator==()");

    if(_CQLExp != rep._CQLExp)
    {
        PEG_METHOD_EXIT();
        return false;
    }
    if(CQLValue(_CQLVal) != rep._CQLVal) // Why?
    {
        PEG_METHOD_EXIT();
        return false;
    }
    if(_CQLFunct != rep._CQLFunct)
    {
        PEG_METHOD_EXIT();
        return false;
    }
    if(_invert != rep._invert)
    {
        PEG_METHOD_EXIT();
        return false;
    }
    if(_simpleValue != rep._simpleValue)
    {
        PEG_METHOD_EXIT();
        return false;
    }
    if(_containedType != rep._containedType)
    {
        PEG_METHOD_EXIT();
        return false;
    }

    PEG_METHOD_EXIT();
    return true;
}

Boolean CQLFactorRep::operator!=(const CQLFactorRep& rep)const
{
  return (!operator==(rep));
}
*/
PEGASUS_NAMESPACE_END
