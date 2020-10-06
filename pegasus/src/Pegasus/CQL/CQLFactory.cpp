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

#include "CQLFactory.h"
#include "CQLValueRep.h"
#include "CQLFactorRep.h"
#include "CQLSimplePredicateRep.h"
#include "CQLPredicateRep.h"
#include "CQLExpressionRep.h"
#include "CQLTermRep.h"
#include <Pegasus/Common/Tracer.h>

PEGASUS_NAMESPACE_BEGIN

CQLOBJECTPTR _CQLObjectPtr;

/*
#define PEGASUS_ARRAY_T CQLObjectPtr
#include <Pegasus/Common/ArrayImpl.h>
#undef PEGASUS_ARRAY_T
*/

/*
CQLFactory::CQLFactory()
{

}
*/
CQLFactory::~CQLFactory()
{
    cleanup();
}

void CQLFactory::cleanup()
{
    PEG_METHOD_ENTER(TRC_CQL, "CQLFactory::cleanup");

    cleanupArray(_makeObjectIdentifiers, Identifier);
    cleanupArray(_makeObjectChainedIdentifiers, ChainedIdentifier);
    cleanupArray(_makeObjectValues, Value);
    cleanupArray(_makeObjectFunctions, Function);
    cleanupArray(_makeObjectFactors, Factor);
    cleanupArray(_makeObjectTerms, Term);
    cleanupArray(_makeObjectExpressions, Expression);
    cleanupArray(_makeObjectSimplePredicates, SimplePredicate);
    cleanupArray(_makeObjectPredicates, Predicate);
    cleanupArray(_getObjectIdentifiers, Identifier);
    cleanupArray(_getObjectChainedIdentifiers, ChainedIdentifier);
    cleanupArray(_getObjectValues, Value);
    cleanupArray(_getObjectFunctions, Function);
    cleanupArray(_getObjectFactors, Factor);
    cleanupArray(_getObjectTerms, Term);
    cleanupArray(_getObjectExpressions, Expression);
    cleanupArray(_getObjectSimplePredicates, SimplePredicate);
    cleanupArray(_getObjectPredicates, Predicate);

    PEG_METHOD_EXIT();
}

void CQLFactory::cleanupArray(Array<CQLObjectPtr>& arr, FactoryType type)
{
    for(Uint32 i=0;i < arr.size(); i++)
    {
        if(arr[i]._ptr)
        {
            switch(type)
            {
                case Predicate:
                                delete (CQLPredicate*)(arr[i]._ptr);
                                break;
                case SimplePredicate:
                                delete (CQLSimplePredicate*)(arr[i]._ptr);
                                break;
                case Expression:
                                delete (CQLExpression*)(arr[i]._ptr);
                                break;
                case Term:
                                delete (CQLTerm*)(arr[i]._ptr);
                                break;
                case Factor:
                                delete (CQLFactor*)(arr[i]._ptr);
                                break;
                case Function:
                                delete (CQLFunction*)(arr[i]._ptr);
                                break;
                case Value:
                                delete (CQLValue*)(arr[i]._ptr);
                                break;
                case ChainedIdentifier:
                                delete (CQLChainedIdentifier*)(arr[i]._ptr);
                                break;
                case Identifier:
                                delete (CQLIdentifier*)(arr[i]._ptr);
                                break;
            }
        }
    }
}

void* CQLFactory::makeObject(CQLIdentifier* obj, FactoryType target)
{
    //printf("CQLFactory::makeObject(identifier)\n");
    PEG_METHOD_ENTER(TRC_CQL, "CQLFactory::makeObject,CQLIdentifier");

    void *cqlIdentifier = NULL;
    _CQLObjectPtr._ptr = new CQLChainedIdentifier(*obj);
    _makeObjectChainedIdentifiers.append(_CQLObjectPtr);

    switch(target)
    {
        case ChainedIdentifier:
            cqlIdentifier =
                _makeObjectChainedIdentifiers[
                    _makeObjectChainedIdentifiers.size()-1]._ptr;
            break;
        case Identifier:
            cqlIdentifier = NULL;
            break;
      default:
            cqlIdentifier = makeObject(
                (CQLChainedIdentifier*)(_CQLObjectPtr._ptr), target);
        break;
    }
    PEG_METHOD_EXIT();
    return cqlIdentifier;
}

void* CQLFactory::makeObject(CQLChainedIdentifier* obj, FactoryType target)
{
    //printf("CQLFactory::makeObject(chainedidentifier)\n");
    PEG_METHOD_ENTER(TRC_CQL, "CQLFactory::makeObject,CQLChainedIdentifier");

    void *cqlChainedIdentifier = NULL;
    _CQLObjectPtr._ptr = new CQLValue(*obj);
    _makeObjectValues.append(_CQLObjectPtr);
    switch(target)
    {
        case Value:
            cqlChainedIdentifier =
                _makeObjectValues[_makeObjectValues.size()-1]._ptr;
            break;
        case ChainedIdentifier:
            cqlChainedIdentifier = NULL;
            break;
        default:
            cqlChainedIdentifier =
                makeObject((CQLValue*)(_CQLObjectPtr._ptr), target);
            break;
    }
    PEG_METHOD_EXIT();
    return cqlChainedIdentifier;
}
void* CQLFactory::makeObject(CQLValue* obj, FactoryType target)
{
    //printf("CQLFactory::makeObject(value)\n");
    PEG_METHOD_ENTER(TRC_CQL, "CQLFactory::makeObject,CQLValue");

    void *cqlValue = NULL;
    _CQLObjectPtr._ptr = new CQLFactor(*obj);
        _makeObjectFactors.append(_CQLObjectPtr);
    switch(target)
    {
        case Factor:
            cqlValue = _makeObjectFactors[_makeObjectFactors.size()-1]._ptr;
            break;
        case Value:
            cqlValue = NULL;
            break;
        default:
            cqlValue = makeObject((CQLFactor*)(_CQLObjectPtr._ptr), target);
            break;
    }
    PEG_METHOD_EXIT();
    return cqlValue;
}
void* CQLFactory::makeObject(CQLFunction* obj, FactoryType target)
{
    //printf("CQLFactory::makeObject(function)\n");
    PEG_METHOD_ENTER(TRC_CQL, "CQLFactory::makeObject,CQLFunction");

    void *cqlFunction = NULL;
    _CQLObjectPtr._ptr = new CQLFactor(*obj);
    _makeObjectFactors.append(_CQLObjectPtr);
    switch(target)
    {
        case Factor:
            cqlFunction =
                _makeObjectFactors[_makeObjectFactors.size()-1]._ptr;
            break;
        default:
            cqlFunction =
                makeObject((CQLFactor*)(_CQLObjectPtr._ptr), target);
            break;
    }
    PEG_METHOD_EXIT();
    return cqlFunction;
}
void* CQLFactory::makeObject(CQLFactor* obj, FactoryType target)
{
    //printf("CQLFactory::makeObject(factor)\n");
    PEG_METHOD_ENTER(TRC_CQL, "CQLFactory::makeObject,CQLFactor");

    void *cqlFactor = NULL;
    _CQLObjectPtr._ptr = new CQLTerm(*obj);
        _makeObjectTerms.append(_CQLObjectPtr);
    switch(target)
    {
        case Term:
            cqlFactor = _makeObjectTerms[_makeObjectTerms.size()-1]._ptr;
            break;
        case Factor:
            cqlFactor = NULL;
            break;
        default:
            cqlFactor = makeObject((CQLTerm*)(_CQLObjectPtr._ptr), target);
            break;
    }
    PEG_METHOD_EXIT();
    return cqlFactor;
}
void* CQLFactory::makeObject(CQLTerm* obj, FactoryType target)
{
    //printf("CQLFactory::makeObject(term)\n");
    PEG_METHOD_ENTER(TRC_CQL, "CQLFactory::makeObject,CQLTerm");

    void *cqlTerm = NULL;
    _CQLObjectPtr._ptr = new CQLExpression(*obj);
        _makeObjectExpressions.append(_CQLObjectPtr);
    switch(target)
    {
        case Expression:
            cqlTerm =
               _makeObjectExpressions[_makeObjectExpressions.size()-1]._ptr;
            break;
        case Term:
            cqlTerm = NULL;
            break;
        default:
            cqlTerm =
                makeObject((CQLExpression*)(_CQLObjectPtr._ptr), target);
            break;
    }
    PEG_METHOD_EXIT();
    return cqlTerm;
}
void* CQLFactory::makeObject(CQLExpression* obj, FactoryType target)
{
    //printf("CQLFactory::makeObject(expression)\n");
    PEG_METHOD_ENTER(TRC_CQL, "CQLFactory::makeObject,CQLExpression");
    void *cqlExpression = NULL;
    _CQLObjectPtr._ptr = new CQLSimplePredicate(*obj);
    _makeObjectSimplePredicates.append(_CQLObjectPtr);
    switch(target)
    {
        case SimplePredicate:
            cqlExpression =
                _makeObjectSimplePredicates[
                    _makeObjectSimplePredicates.size()-1]._ptr;
            break;
        case Expression:
            cqlExpression = NULL;
            break;
        default:
            cqlExpression =
                makeObject((CQLSimplePredicate*)(_CQLObjectPtr._ptr),
                    target);
            break;
    }
    PEG_METHOD_EXIT();
    return cqlExpression;
}
void* CQLFactory::makeObject(CQLSimplePredicate* obj, FactoryType target)
{
    //printf("CQLFactory::makeObject(simplepredicate)\n");
    PEG_METHOD_ENTER(TRC_CQL, "CQLFactory::makeObject,CQLSimplePredicate");

    void *cqlSimplePredicate = NULL;
    _CQLObjectPtr._ptr = new CQLPredicate(*obj);
    _makeObjectPredicates.append(_CQLObjectPtr);
    switch(target){
        case Predicate:
            cqlSimplePredicate =
                _makeObjectPredicates[_makeObjectPredicates.size()-1]._ptr;
            break;
        default:
            cqlSimplePredicate = NULL;
            break;
    }
    PEG_METHOD_EXIT();
    return cqlSimplePredicate;
}

void* CQLFactory::getObject(void* inObject, FactoryType inObjectType,
                            FactoryType targetType)
{
    PEG_METHOD_ENTER(TRC_CQL, "CQLFactory::getObject");
    void * obj = NULL;

    switch(inObjectType)
    {
        case Predicate:
            obj = getObject(((CQLPredicate*)inObject),targetType);
            break;
        case SimplePredicate:
            obj = getObject(((CQLSimplePredicate*)inObject),targetType);
            break;
        case Expression:
            obj = getObject(((CQLExpression*)inObject),targetType);
            break;
        case Term:
            obj = getObject(((CQLTerm*)inObject),targetType);
            break;
        case Factor:
            obj = getObject(((CQLFactor*)inObject),targetType);
            break;
        case Function:
            obj = NULL;
            break;
        case Value:
            obj = getObject(((CQLValue*)inObject),targetType);
            break;
        case ChainedIdentifier:
            obj = getObject(((CQLChainedIdentifier*)inObject),targetType);
            break;
        default:
            obj = NULL;
            break;
    }

    PEG_METHOD_EXIT();
    return obj;
}

void* CQLFactory::getObject(CQLChainedIdentifier* obj, FactoryType target)
{
    //  printf("CQLFactory::getObject(CQLChainedIdentifier* obj)\n");
    Array<CQLIdentifier> cqlIds;

    switch(target)
    {
        case Identifier:
            //if(obj->_rep->_subIdentifiers.size() > 0){
            cqlIds = obj->getSubIdentifiers();
            if(cqlIds.size() > 0)
            {
                //   _CQLObjectPtr._ptr =
                //       new CQLIdentifier(obj->_rep->_subIdentifiers[0]);
                _CQLObjectPtr._ptr = new CQLIdentifier(cqlIds[0]);
                _getObjectIdentifiers.append(_CQLObjectPtr);
                        return _getObjectIdentifiers[
                            _getObjectIdentifiers.size()-1]._ptr;
             }
            return NULL;
        default:
            return NULL;
    }
}

void* CQLFactory::getObject(CQLValue* obj, FactoryType target)
{
    //  printf("CQLFactory::getObject(CQLValue* obj)\n");
    switch(target)
    {
        case ChainedIdentifier:
          _CQLObjectPtr._ptr =
              new CQLChainedIdentifier(obj->_rep->_CQLChainId);
          _getObjectChainedIdentifiers.append(_CQLObjectPtr);
          return _getObjectChainedIdentifiers[
              _getObjectChainedIdentifiers.size()-1]._ptr;
        case Identifier:
          return getObject(&(obj->_rep->_CQLChainId),target);
        default:
          return NULL;
          break;
    }
}

void* CQLFactory::getObject(CQLFactor* obj, FactoryType target)
{
    //  printf("CQLFactory::getObject(CQLFactor* obj)\n");
    switch(target)
    {
        case Function:
            _CQLObjectPtr._ptr = new CQLFunction(obj->_rep->_CQLFunct);
                _getObjectFunctions.append(_CQLObjectPtr);
            return _getObjectFunctions[_getObjectFunctions.size()-1]._ptr;
        case Value:
            _CQLObjectPtr._ptr = new CQLValue(obj->_rep->_CQLVal);
            _getObjectValues.append(_CQLObjectPtr);
            return _getObjectValues[_getObjectValues.size()-1]._ptr;
        case ChainedIdentifier:
        case Identifier:
            return getObject(&(obj->_rep->_CQLVal),target);
        default:
            return NULL;
            break;
    }
}

void* CQLFactory::getObject(CQLTerm* obj, FactoryType target)
{
    //  printf("CQLFactory::getObject(CQLTerm* obj)\n");
    switch(target)
    {
        case Factor:
            _CQLObjectPtr._ptr = new CQLFactor(obj->getFactors()[0]);
                _getObjectFactors.append(_CQLObjectPtr);
            return _getObjectFactors[_getObjectFactors.size()-1]._ptr;
        case Function:
        case Value:
        case ChainedIdentifier:
        case Identifier:
            return getObject(&(obj->getFactors()[0]),target);
        default:
            return NULL;
            break;
    }
}

void* CQLFactory::getObject(CQLExpression* obj, FactoryType target)
{
    //  printf("CQLFactory::getObject(CQLExpression* obj)\n");
    switch(target)
    {
        case Term:
            _CQLObjectPtr._ptr = new CQLTerm(obj->getTerms()[0]);
                _getObjectTerms.append(_CQLObjectPtr);
            return _getObjectTerms[_getObjectTerms.size()-1]._ptr;
        case Factor:
        case Function:
        case Value:
        case ChainedIdentifier:
        case Identifier:
            return getObject(&(obj->getTerms()[0]), target);
        default:
            return NULL;
            break;
    }
}

void* CQLFactory::getObject(CQLSimplePredicate* obj, FactoryType target)
{
    //  printf("CQLFactory::getObject(CQLSimplePredicate* obj)\n");
    switch(target)
    {
        case Expression:
            _CQLObjectPtr._ptr = new CQLExpression(obj->_rep->_leftSide);
            _getObjectExpressions.append(_CQLObjectPtr);
            return _getObjectExpressions[_getObjectExpressions.size()-1]._ptr;
        case Term:
        case Factor:
        case Function:
        case Value:
        case ChainedIdentifier:
        case Identifier:
            return getObject(&(obj->_rep->_leftSide), target);
        default:
            return NULL;
            break;
    }
}

void* CQLFactory::getObject(CQLPredicate* obj, FactoryType target)
{
    //  printf("CQLFactory::getObject(CQLPredicate* obj)\n");
    switch(target)
    {
        case SimplePredicate:
            _CQLObjectPtr._ptr =
                new CQLSimplePredicate(obj->_rep->_simplePredicate);
                _getObjectSimplePredicates.append(_CQLObjectPtr);
                return _getObjectSimplePredicates[
                    _getObjectSimplePredicates.size()-1]._ptr;
        case Expression:
        case Term:
        case Factor:
        case Function:
        case Value:
        case ChainedIdentifier:
        case Identifier:
            return getObject(&(obj->_rep->_simplePredicate), target);
        default:
            return NULL;
            break;
    }
}

void CQLFactory::setObject(CQLPredicate* predicate, void* obj,
                           FactoryType objType)
{
    PEG_METHOD_ENTER(TRC_CQL, "CQLFactory::setObject");
    switch(objType)
    {
        case SimplePredicate:
            predicate->_rep->_simplePredicate = *((CQLSimplePredicate*)obj);
            break;
        case Expression:
            predicate->_rep->_simplePredicate._rep->_leftSide =
                *((CQLExpression*)obj);
            break;
        case Term:
            predicate->_rep->_simplePredicate._rep->_leftSide._rep->
                _CQLTerms[0] = *((CQLTerm*)obj);
            break;
        case Factor:
            predicate->_rep->_simplePredicate._rep->
                _leftSide._rep->_CQLTerms[0]._rep->_Factors[0] =
                *((CQLFactor*)obj);
            break;
        case Function:
            predicate->_rep->_simplePredicate._rep->_leftSide._rep->
                _CQLTerms[0]._rep->_Factors[0]._rep->_CQLFunct =
                            *((CQLFunction*)obj);
            break;
        case Value:
            predicate->_rep->_simplePredicate._rep->_leftSide._rep->
                _CQLTerms[0]._rep->_Factors[0]._rep->_CQLVal =
                            *((CQLValue*)obj);
            break;
        case ChainedIdentifier:
            predicate->_rep->_simplePredicate._rep->_leftSide._rep->
                _CQLTerms[0]._rep->_Factors[0]._rep->_CQLVal._rep->_CQLChainId
                = *((CQLChainedIdentifier*)obj);
            break;
        case Identifier:
            break;
        default:
            break;
    }
    PEG_METHOD_EXIT();
}
PEGASUS_NAMESPACE_END
