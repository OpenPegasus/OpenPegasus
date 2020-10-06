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

#ifndef Pegasus_CQLFactory_h
#define Pegasus_CQLFactory_h

#include <Pegasus/CQL/Linkage.h>
#include <Pegasus/Common/ArrayInternal.h>
#include <Pegasus/CQL/CQLValue.h>
#include <Pegasus/CQL/CQLChainedIdentifier.h>
//#include <Pegasus/CQL/CQLSelectStatement.h>
#include <Pegasus/CQL/CQLPredicate.h>
#include <Pegasus/CQL/CQLSimplePredicate.h>
#include <Pegasus/CQL/CQLIdentifier.h>

#include <Pegasus/CQL/CQLTerm.h>
#include <Pegasus/CQL/CQLFactor.h>
//#include <Pegasus/CQL/CQLParserState.h>
#include <Pegasus/CQL/CQLExpression.h>
#include <Pegasus/CQL/CQLFunction.h>

#ifdef PEGASUS_USE_EXPERIMENTAL_INTERFACES

PEGASUS_NAMESPACE_BEGIN

typedef struct CQLObjectPtr {
        void* _ptr;
} CQLOBJECTPTR;

/*
#ifndef PEGASUS_ARRAY_T
#define PEGASUS_ARRAY_T CQLObjectPtr
#include <Pegasus/Common/ArrayInter.h>
#undef PEGASUS_ARRAY_T
#endif
*/

enum FactoryType { Identifier, ChainedIdentifier, Value, Function,
                    Factor, Term, Expression, SimplePredicate, Predicate };

/**
   The CQLFactory class enables the user to construct and access complex
   CQL objects from simple objects without having to call each constructor
   or each getter in the CQL object tree.

   Three uses:
        makeObject - allows the caller to pass in a CQL object and a target
        object type.  This returns the target type built from the initial
        object.

        getObject  - allows caller to pass in a CQL object and target query
        object type.  This returns the target type contained within the CQL
        object passed in.

        setObject  - allows caller to pass in a CQLPredicate, and object to
        set into the CQlPredicate and its type.
*/

class CQLFactory
{
  public:
  //CQLFactory();
  ~CQLFactory();

  /**
      Makes a target type object from a CQLIdentifier

      @param  - obj.  The CQLIdentifier to make the object from
      @param  - target. The target type of the object to be constructed.
      @return - A CQL object of type target cast to a void*.  Caller must
                then cast to the appropriate type.
      @throw  - None.
      <I><B>Experimental Interface</B></I><BR>
  */
  void* makeObject(CQLIdentifier* obj, FactoryType target);

  /**
      Makes a target type object from a CQLChainedIdentifier

      @param  - obj.  The CQLChainedIdentifier to make the object from
      @param  - target. The target type of the object to be constructed.
      @return - A CQL object of type target cast to a void*.  Caller must
                then cast to the appropriate type.
      @throw  - None.
      <I><B>Experimental Interface</B></I><BR>
  */
  void* makeObject(CQLChainedIdentifier* obj, FactoryType target);

  /**
      Makes a target type object from a CQLValue

      @param  - obj.  The CQLValue to make the object from
      @param  - target. The target type of the object to be constructed.
      @return - A CQL object of type target cast to a void*.  Caller must
                then cast to the appropriate type.
      @throw  - None.
      <I><B>Experimental Interface</B></I><BR>
  */
  void* makeObject(CQLValue* obj, FactoryType target);

  /**
      Makes a target type object from a CQLFactor

      @param  - obj.  The CQLFactor to make the object from
      @param  - target. The target type of the object to be constructed.
      @return - A CQL object of type target cast to a void*.  Caller must
                then cast to the appropriate type.
      @throw  - None.
      <I><B>Experimental Interface</B></I><BR>
  */
  void* makeObject(CQLFactor* obj, FactoryType target);

  /**
      Makes a target type object from a CQLFunction

      @param  - obj.  The CQLFunction to make the object from
      @param  - target. The target type of the object to be constructed.
      @return - A CQL object of type target cast to a void*.  Caller must
                then cast to the appropriate type.
      @throw  - None.
      <I><B>Experimental Interface</B></I><BR>
  */
  void* makeObject(CQLFunction* obj, FactoryType target);

  /**
      Makes a target type object from a CQLTerm

      @param  - obj.  The CQLTerm to make the object from
      @param  - target. The target type of the object to be constructed.
      @return - A CQL object of type target cast to a void*.  Caller must
                then cast to the appropriate type.
      @throw  - None.
      <I><B>Experimental Interface</B></I><BR>
  */
  void* makeObject(CQLTerm* obj, FactoryType target);

  /**
      Makes a target type object from a CQLExpression

      @param  - obj.  The CQLExpression to make the object from
      @param  - target. The target type of the object to be constructed.
      @return - A CQL object of type target cast to a void*.  Caller must
                then cast to the appropriate type.
      @throw  - None.
      <I><B>Experimental Interface</B></I><BR>
  */
  void* makeObject(CQLExpression* obj, FactoryType target);

  /**
      Makes a target type object from a CQLSimplePredicate

      @param  - obj.  The CQLSimplePredicate to make the object from
      @param  - target. The target type of the object to be constructed.
      @return - A CQL object of type target cast to a void*.  Caller must
                then cast to the appropriate type.
      @throw  - None.
      <I><B>Experimental Interface</B></I><BR>
  */
  void* makeObject(CQLSimplePredicate* obj, FactoryType target);

  /**
      Returns an object of targetType contained within CQL object being
      passed in.
      This method is a general use method that internally calls one of the
      specific getObject flavors.

      @param  - inObject.  The object to extract the target type from.
      @param  - inObjectType. The target type of the object being passed in.
      @param  - targetType. The target type of the object to extract from the
                CQL object passed in.
      @return - A CQL object of type target cast to a void*.  Caller must then
                cast to the appropriate type.
      @throw  - None.
      <I><B>Experimental Interface</B></I><BR>
  */
  void* getObject(void* inObject, FactoryType inObjectType,
          FactoryType targetType);

  void* getObject(CQLChainedIdentifier* obj, FactoryType target);
  void* getObject(CQLValue* obj, FactoryType target);
  void* getObject(CQLFactor* obj, FactoryType target);
  void* getObject(CQLTerm* obj, FactoryType target);
  void* getObject(CQLExpression* obj, FactoryType target);
  void* getObject(CQLSimplePredicate* obj, FactoryType target);
  void* getObject(CQLPredicate* obj, FactoryType target);

  /**
      Sets an object of targetType into the CQLPredicate.
      The caller may want to replace the CQLValue within the CQLPredicate.
      This enables the caller to do this with one call.

      @param  - predicate.  The CQLPredicate to set the target type object into.
      @param  - obj.  The object to set into the CQLPredicate
      @param  - objType. The type of "obj"
      @return - None.
      @throw  - None.
      <I><B>Experimental Interface</B></I><BR>
  */
  void setObject(CQLPredicate* predicate, void* obj, FactoryType objType);


  /**
      Frees all memory allocated.

      @param  - None.
      @return - None.
      @throw  - None.
      <I><B>Experimental Interface</B></I><BR>
  */
  void cleanup();
private:
  void cleanupArray(Array<CQLObjectPtr>& arr, FactoryType type);

  Array<CQLObjectPtr> _makeObjectPredicates;
  Array<CQLObjectPtr> _makeObjectSimplePredicates;
  Array<CQLObjectPtr> _makeObjectExpressions;
  Array<CQLObjectPtr> _makeObjectTerms;
  Array<CQLObjectPtr> _makeObjectFactors;
  Array<CQLObjectPtr> _makeObjectFunctions;
  Array<CQLObjectPtr> _makeObjectValues;
  Array<CQLObjectPtr> _makeObjectChainedIdentifiers;
  Array<CQLObjectPtr> _makeObjectIdentifiers;

  Array<CQLObjectPtr> _getObjectPredicates;
  Array<CQLObjectPtr> _getObjectSimplePredicates;
  Array<CQLObjectPtr> _getObjectExpressions;
  Array<CQLObjectPtr> _getObjectTerms;
  Array<CQLObjectPtr> _getObjectFactors;
  Array<CQLObjectPtr> _getObjectFunctions;
  Array<CQLObjectPtr> _getObjectValues;
  Array<CQLObjectPtr> _getObjectChainedIdentifiers;
  Array<CQLObjectPtr> _getObjectIdentifiers;

};
PEGASUS_NAMESPACE_END
#endif
#endif
