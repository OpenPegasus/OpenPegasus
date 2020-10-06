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

#ifndef Pegasus_CQLSelectStatement_h
#define Pegasus_CQLSelectStatement_h

#include <Pegasus/CQL/Linkage.h>
#include <Pegasus/Common/AutoPtr.h>
#include <Pegasus/Query/QueryCommon/SelectStatement.h>
#include <Pegasus/CQL/CQLChainedIdentifier.h>
#include <Pegasus/CQL/CQLPredicate.h>
#include <Pegasus/CQL/CQLIdentifier.h>


#ifdef PEGASUS_USE_EXPERIMENTAL_INTERFACES

PEGASUS_NAMESPACE_BEGIN

class CQLSelectStatementRep;

/**
This class is derived from the SelectStatement base class.
The purpose of this class is to perform the select statement operations for
CIM Query Language (CQL).
*/
class PEGASUS_CQL_LINKAGE CQLSelectStatement : public SelectStatement
{
  public:

    /**
      Constructs a CQLSelectStatement default object.

      <I><B>Experimental Interface</B></I><BR>
    */
    CQLSelectStatement();

    /**
      Constructs a CQLSelectStatement object.

      @param  inQlang - String containing the language of the query.
      @param  inQuery - String containing the query.
      @param  inCtx - Context in which the select statement is running.

      <I><B>Experimental Interface</B></I><BR>
    */
    CQLSelectStatement(
        const String& inQlang,
        const String& inQuery,
        const QueryContext& inCtx);

    /**
      Constructs a CQLSelectStatement object.

      @param  inQlang - String containing the language of the query.
      @param  inQuery - String containing the query.

      <I><B>Experimental Interface</B></I><BR>
    */
    CQLSelectStatement(
        const String& inQlang,
        const String& inQuery);

    /**
      Copy constructs a CQLSelectStatement object.

      @param  statement - CQLSelectStatement to be copied.

      <I><B>Experimental Interface</B></I><BR>
    */
    CQLSelectStatement(const CQLSelectStatement& statement);

    /**
      Destructs a CQLSelectStatement object.

      <I><B>Experimental Interface</B></I><BR>
    */
    ~CQLSelectStatement();

    /**
      Assigns a CQLSelectStatement to this object.

      @param rhs - CQLSelectStatement to be assigned to this object.
      @return - Updated this object.
      @throw  - None.

      <I><B>Experimental Interface</B></I><BR>
    */
    CQLSelectStatement& operator=(const CQLSelectStatement& rhs);

    /**
      Applies the class contexts from the FROM list to the
      chained identifiers in the statement. This will transform
      each chained identifier into a normalized form.  The FROM
      classname is prepended if needed, and all class aliases are resolved.

      Note: there are cases where the FROM classname is not prepended.
      This can occur for the classname on the right side of ISA,
      or the classname at the beginning of a symbolic constant chained
      identifier.  Neither of these classnames need to be the FROM class.

      This function also validates that each chained identifier
      is well-formed.  It is possible for a chained identifier
      to be syntactically correct in the CQL language, but
      cannot be processed by the CQL engine.

      Pre-condition: QueryContext has been set into this object.
      Post-condition: Chained identifiers have been normalized.

      @throw CQLRuntimeException if the QueryContext had not been set.
      @throw CQLValidationException if a chained identifier is not well formed.
      @throw CQLSyntaxErrorException if a chained identifier is not well
        formed.

      <I><B>Experimental Interface</B></I><BR>
     */
    void applyContext();

    /**
      Evaluates the WHERE clause of the select statement using
      a CIM instance as the source of properties.

      Pre-condition: QueryContext has been set into this object.
      Post-condition: Chained identifiers have been normalized
      ie.applyContext has been called.  See the applyContext function.

      @param inCI - The instance to be evaluated.
      @return True, if the WHERE clause evaluates to true based on
              the type of the instance and its properties.
      @throw CQLRuntimeException if the instance cannot be evaluated
      @throw CQLValidationException for applyContext error.
      @throw CQLSyntaxErrorException for applyContext error.

      <I><B>Experimental Interface</B></I><BR>
    */
    Boolean evaluate(const CIMInstance& inCI);

    /**
      Projects the properties in the SELECT list of the select
      statement onto the instance.  This involves checking that
      all required properties exist on the instance passed in,
      and removing any unneeded properties from that instance.

      Pre-condition: QueryContext has been set into this object.
      Post-condition: Chained identifiers have been normalized
      ie.applyContext has been called.  See the applyContext function.

      @param inCI - The instance to be projected.
      @param allowMissing indicates whether missing project properties are
                          allowed
      @throw  CQLRuntimeException if the instance cannot be projected
      @throw CQLValidationException for applyContext error.
      @throw CQLSyntaxErrorException for applyContext error.

      <I><B>Experimental Interface</B></I><BR>
    */
    void applyProjection(CIMInstance& inCI, Boolean allowMissing);

    /**
      Validates the classes and properties used in the select statement
      against the class schema.  The existence of classes and properties
      are checked, along with class relationships defined in the CQL
      specification.

      Pre-condition: QueryContext has been set into this object.
      Post-condition: Chained identifiers have been normalized
      ie.applyContext has been called.  See the applyContext function.

      @throw CQLRuntimeException if the QueryContext had not been set.
      @throw CQLValidationException for applyContext error, the select
      statement is invalid against the schema, or the QueryContext has
      not been set.
      @throw CQLSyntaxErrorException for applyContext error.

      <I><B>Experimental Interface</B></I><BR>
    */
    void validate();

    /**
      Normalizes the predicates in the WHERE clause to a
      disjunction of conjunctions.

      Post-condition: Chained identifiers have been normalized
      ie.applyContext has been called.  See the applyContext function.

      @param None
      @return None
      @throw None

      <I><B>Experimental Interface</B></I><BR>
     */
    void normalizeToDOC();

    /**
      Returns an array of CIMObjectPath objects that are the
      class paths in the FROM list of the select statement.

      Note:  Currently CQL only supports one class path in the
      FROM list.  This class path does not support WBEM-URI, so that
      only the namespace and classname parts are filled in.

      Pre-condition: QueryContext has been set into this object.

      @param None
      @return Array of FROM list class paths.
      @throw CQLRuntimeException if the QueryContext had not been set.

      <I><B>Experimental Interface</B></I><BR>
     */
    Array<CIMObjectPath> getClassPathList() const;

    /**
      Returns the required properties from the combined SELECT and WHERE
      clauses for the classname passed in.  The classname parameter is
      needed in case scoping operators are used in the select statement.
      This function does not return properties for the classname if they
      are required on embedded objects.  This function does not return
      required array indices.

      Pre-condition: QueryContext has been set into this object.
      Post-condition: Chained identifiers have been normalized
      ie.applyContext has been called.  See the applyContext function.

      @param inClassName - class to determine the required properties.
      Defaults to the FROM class.
      @return CIMPropertyList containing the required properties for the class.
      If all the properties are required, a null CIMPropertyList is returned.
      If no properties are required, an empty CIMPropertyList is returned.
      @throw CQLRuntimeException if the QueryContext had not been set.
      @throw CQLValidationException for applyContext error.
      @throw CQLSyntaxErrorException for applyContext error.

      <I><B>Experimental Interface</B></I><BR>
    */
    CIMPropertyList getPropertyList(
        const CIMObjectPath& inClassName = CIMObjectPath());

    /**
      Returns the required properties from the SELECT clause for the
      classname passed in.  The classname parameter is needed in case
      scoping operators are used in the select statement.  This function
      does not return properties for the classname if they are required
      on embedded objects. This function does not return required
      array indices.

      Pre-condition: QueryContext has been set into this object.
      Post-condition: Chained identifiers have been normalized
      ie.applyContext has been called.  See the applyContext function.

      @param inClassName - class to determine the required properties.
      Defaults to the FROM class.
      @return CIMPropertyList containing the required properties for the class.
      If all the properties are required, a null CIMPropertyList is returned.
      If no properties are required, an empty CIMPropertyList is returned.
      @throw CQLRuntimeException if the QueryContext had not been set.
      @throw CQLValidationException for applyContext error.
      @throw CQLSyntaxErrorException for applyContext error.

      <I><B>Experimental Interface</B></I><BR>
    */
    CIMPropertyList getSelectPropertyList(
        const CIMObjectPath& inClassName = CIMObjectPath());

    /**
      Returns the required properties from the WHERE clause for the
      classname passed in.  The classname parameter is needed in case
      scoping operators are used in the select statement.  This function
      does not return properties for the classname if they are required
      on embedded objects.  This function does not return required array
      indices.

      Pre-condition: QueryContext has been set into this object.
      Post-condition: Chained identifiers have been normalized
      ie.applyContext has been called.  See the applyContext function.

      @param inClassName - class to determine the required properties.
      Defaults to the FROM class.
      @return CIMPropertyList containing the required properties for the class.
      If all the properties are required, a null CIMPropertyList is returned.
      If no properties are required, an empty CIMPropertyList is returned.
      @throw CQLRuntimeException if the QueryContext had not been set.
      @throw CQLValidationException for applyContext error.
      @throw CQLSyntaxErrorException for applyContext error.

      <I><B>Experimental Interface</B></I><BR>
    */
    CIMPropertyList getWherePropertyList(
        const CIMObjectPath& inClassName = CIMObjectPath());

    /**
      Returns the chained identifiers from the SELECT clause.  These chained
      identifiers contain the fully qualified property names, including
      embedded objects, array indices, scoping operators,
      and symbolic constants.

      Post-condition: Chained identifiers have been normalized
      ie.applyContext has been called.  See the applyContext function.

      @param None
      @return Array of chained identifiers.
      @throw CQLValidationException for applyContext error.
      @throw CQLSyntaxErrorException for applyContext error.

      <I><B>Experimental Interface</B></I><BR>
     */
    Array<CQLChainedIdentifier> getSelectChainedIdentifiers();

    /**
      Returns the chained identifiers from the WHERE clause.  These chained
      identifiers contain the fully qualified property names, including
      embedded objects, array indices, scoping operators,
      and symbolic constants.

      Pre-condition: QueryContext has been set into this object.
      Post-condition: Chained identifiers have been normalized
      ie.applyContext has been called.  See the applyContext function.

      @param None
      @return Array of chained identifiers.
      @throw CQLRuntimeException if the QueryContext had not been set.
      @throw CQLValidationException for applyContext error.
      @throw CQLSyntaxErrorException for applyContext error.

      <I><B>Experimental Interface</B></I><BR>
     */
    Array<CQLChainedIdentifier> getWhereChainedIdentifiers();

    /**
      Gets the top-level CQLPredicate of this object.
      This is the predicate that contains the top-level
      of any nested boolean operations in the WHERE clause.

      @param None.
      @return Top-level predicate
      @throw None.

      <I><B>Experimental Interface</B></I><BR>
    */
    CQLPredicate getPredicate() const;

    /**
      Determines whether the select statement has a WHERE clause.

      @param None
      @return True if there is a WHERE clause.
      @throw None

      <I><B>Experimental Interface</B></I><BR>
    */
    Boolean hasWhereClause() const;

    /**
      Returns the select statement in string form.
      Note that this can be different than the original
      statement after it has been parsed, and applyContext
      has been called.

      Pre-condition: QueryContext has been set into this object.

      @param None
      @return String containing the select statement.
      @throw

      <I><B>Experimental Interface</B></I><BR>
    */
    String toString() const;

    /*
       ATTN - the following methods should only be set by CQLParser.
       These should be made private, and CQLParser made a friend.
    */

    /**
      Appends a CQLIdentifier/alias combination into the FROM list of the
      statement. This method should only be called by the CQL parser.

      @param inIdentifier - CQLIdentifier to append.
      @param inAlias - Alias of the identifier.
      @return None
      @throw CQLRuntimeException if the QueryContext had not been set.
      @throw QueryException if the identifier/alias cannot be appended.

      <I><B>Experimental Interface</B></I><BR>
    */
    void insertClassPathAlias(
        const CQLIdentifier& inIdentifier,
        const String& inAlias);

    /**
      Appends a CQLIdentifier into the FROM list of the statement.
      This method should only be called by the CQL parser.

      @param inIdentifier - CQLIdentifier to append.
      @return None
      @throw CQLRuntimeException if the QueryContext had not been set.
      @throw QueryException if the identifier cannot be appended.

      <I><B>Experimental Interface</B></I><BR>
    */
    void appendClassPath(const CQLIdentifier& inIdentifier);

    /**
      Appends a CQLChainedIdentifier to the select list.
      This method should only be called by the CQL parser.

      @param x - CQLChainedIdentifier to append.
      @return None
      @throw None

      <I><B>Experimental Interface</B></I><BR>
    */
    void appendSelectIdentifier(const CQLChainedIdentifier& x);

    /** Sets a predicate into this object. This method should only
            be called by Bison.
        */
    /**
      Sets the top-level CQLPredicate into this object.
      This method should only be called by the CQL parser.

      @param Predicate to set into this object.
      @return None
      @throw None.

      <I><B>Experimental Interface</B></I><BR>
    */
    void setPredicate(const CQLPredicate& inPredicate);

    /**
      Sets the select statement to have a WHERE clause.
      This method should only be called by the CQL parser.

      @param None
      @return None
      @throw None

      <I><B>Experimental Interface</B></I><BR>
    */
    void setHasWhereClause();

    /**
      Clears the internal data structures.
      This method should only be called by the CQL parser.

      Pre-condition: QueryContext has been set into this object.

      @param None
      @return None.
      @throw CQLRuntimeException if the QueryContext had not been set.

      <I><B>Experimental Interface</B></I><BR>
    */
    void clear();

  private:
    CQLSelectStatementRep* _rep;

};

PEGASUS_NAMESPACE_END
#endif
#endif
