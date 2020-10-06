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

#include <Pegasus/CQL/CQLFunction.h>
#include <Pegasus/CQL/CQLFunctionRep.h>
//#include <Pegasus/CQL/CQLExpression.h>
#include <Pegasus/CQL/CQLPredicate.h>
#include <Pegasus/CQL/CQLFactory.h>
PEGASUS_NAMESPACE_BEGIN

CQLFunction::CQLFunction()
{
    _rep = new CQLFunctionRep();
}

CQLFunction::CQLFunction(CQLIdentifier inOpType, Array<CQLPredicate> inParms)
{
    _rep = new CQLFunctionRep(inOpType,inParms);
}

CQLFunction::CQLFunction(const CQLFunction& inFunc)
{
    _rep = new CQLFunctionRep(inFunc._rep);
}

CQLFunction::~CQLFunction(){
    if(_rep)
        delete _rep;
}

CQLValue CQLFunction::resolveValue(const CIMInstance& CI,
                                   const QueryContext& queryCtx)
{
    return _rep->resolveValue(CI,queryCtx);
}

String CQLFunction::toString()const
{
    return _rep->toString();
}


Array<CQLPredicate> CQLFunction::getParms()const
{
    return _rep->getParms();
}

FunctionOpType CQLFunction::getFunctionType()const
{
    return _rep->getFunctionType();
}

void CQLFunction::applyContext(const QueryContext& inContext)
{
    _rep->applyContext(inContext);
}

CQLFunction& CQLFunction::operator=(const CQLFunction& rhs)
{
    if(&rhs != this)
    {
        if(_rep) delete _rep;
        _rep = new CQLFunctionRep(rhs._rep);
    }
    return *this;
}

/*
Boolean CQLFunction::operator==(const CQLFunction& func)const{
    return (*_rep == *(func._rep));
}
Boolean CQLFunction::operator!=(const CQLFunction& func)const{
    return (_rep != func._rep);
}
*/
PEGASUS_NAMESPACE_END
