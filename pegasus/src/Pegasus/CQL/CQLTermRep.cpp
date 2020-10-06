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

#include <Pegasus/CQL/CQLTerm.h>
#include <Pegasus/CQL/CQLTermRep.h>
#include <Pegasus/CQL/CQLFactory.h>
#include <Pegasus/Query/QueryCommon/QueryContext.h>
#include <Pegasus/Query/QueryCommon/QueryException.h>
#include <Pegasus/Common/Tracer.h>

PEGASUS_NAMESPACE_BEGIN

CQLTermRep::CQLTermRep(){}

CQLTermRep::CQLTermRep(const CQLFactor& theFactor)
{
    PEG_METHOD_ENTER(TRC_CQL,"CQLTermRep:CQLTermRep()");

    _Factors.append(theFactor);

    PEG_METHOD_EXIT();
}

CQLTermRep::CQLTermRep(const CQLTermRep& rep){
    _Factors = rep._Factors;
    _FactorOperators = rep._FactorOperators;
}

CQLTermRep::~CQLTermRep(){
}

CQLValue CQLTermRep::resolveValue(const CIMInstance& CI,
                                  const QueryContext& QueryCtx)
{
    PEG_METHOD_ENTER(TRC_CQL,"CQLTermRep:resolveValue()");
    CQLValue returnVal = _Factors[0].resolveValue(CI,QueryCtx);

    for(Uint32 i = 0; i < _FactorOperators.size(); ++i)
    {
        switch(_FactorOperators[i])
        {
             /*
             case mult:
                returnVal = returnVal *
                            _Factors[i+1].resolveValue(CI,QueryCtx);
                break;
             case divide:
                returnVal = returnVal /
                            _Factors[i+1].resolveValue(CI,QueryCtx);
                break;
            */
             case concat:
                returnVal = returnVal +
                            _Factors[i+1].resolveValue(CI,QueryCtx);
                break;

             default:
               MessageLoaderParms mload(
                   "CQL.CQLTermRep.OPERATION_NOT_SUPPORTED",
                   "Operation is not supported.");
               throw CQLRuntimeException(mload);
        }
    }

    PEG_METHOD_EXIT();
    return returnVal;
}

void CQLTermRep::appendOperation(FactorOpType inFactorOpType,
                                 const CQLFactor& inFactor)
{
    PEG_METHOD_ENTER(TRC_CQL,"CQLTermRep::appendOperation()");

    _FactorOperators.append(inFactorOpType);
    _Factors.append(inFactor);

    PEG_METHOD_EXIT();
}

String CQLTermRep::toString()const
{
    PEG_METHOD_ENTER(TRC_CQL,"CQLTermRep::toString()");

    String returnStr;

    returnStr.append(_Factors[0].toString());

    for(Uint32 i = 0; i < _FactorOperators.size(); ++i)
    {
        /*
        returnStr.append(_FactorOperators[i] ==
               mult ? String(" * ") : divide ? String(" / ")
               : String(" concat "));
        */
        if(_FactorOperators[i] == concat)
            returnStr.append(String(" || "));
        returnStr.append(_Factors[i+1].toString());
    }

    PEG_METHOD_EXIT();
    return returnStr;
}

Boolean CQLTermRep::isSimple()const
{
    return (_Factors.size() == 1);
}

Boolean CQLTermRep::isSimpleValue()const
{
    PEG_METHOD_ENTER(TRC_CQL,"CQLTermRep::isSimpleValue()");
    if(_Factors.size() == 1)
    {
        PEG_METHOD_EXIT();
        return _Factors[0].isSimpleValue();
    }

    PEG_METHOD_EXIT();
    return false;
}

Array<CQLFactor> CQLTermRep::getFactors()const
{
    return _Factors;
}

Array<FactorOpType> CQLTermRep::getOperators()const
{
    return _FactorOperators;
}

void CQLTermRep::applyContext(const QueryContext& inContext,
                              const CQLChainedIdentifier& inCid)
{
    PEG_METHOD_ENTER(TRC_CQL,"CQLTermRep::applyContext()");

    for(Uint32 i = 0; i < _Factors.size(); ++i)
    {
        _Factors[i].applyContext(inContext, inCid);
    }

    PEG_METHOD_EXIT();
}
/*
Boolean CQLTermRep::operator==(const CQLTermRep& rhs)const
{
  PEG_METHOD_ENTER(TRC_CQL,"CQLTermRep::operator==()");

  for(Uint32 i = 0; i < _FactorOperators.size(); ++i)
    {
      if(_FactorOperators[i] != rhs._FactorOperators[i])
    {
      PEG_METHOD_EXIT();
      return false;
    }
    }

  for(Uint32 i = 0; i < _Factors.size(); ++i)
    {
      if(_Factors[i] != rhs._Factors[i])
    {
      PEG_METHOD_EXIT();
      return false;
    }
    }

  PEG_METHOD_EXIT();
  return true;
}

Boolean CQLTermRep::operator!=(const CQLTermRep& rhs)const
{
  return (!operator==(rhs));
}
*/
PEGASUS_NAMESPACE_END
