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

#include "CQLSelectStatement.h"
#include "CQLSelectStatementRep.h"

#include <Pegasus/Common/Constants.h>
#include <Pegasus/Common/CIMValue.h>
#include <Pegasus/Common/CIMInstance.h>
#include <Pegasus/Common/CIMProperty.h>
#include <Pegasus/Query/QueryCommon/QueryException.h>
#include <Pegasus/Query/QueryCommon/QueryIdentifier.h>
#include <Pegasus/Query/QueryCommon/QueryChainedIdentifier.h>
#include <Pegasus/Common/Tracer.h>
#include <Pegasus/Common/InternalException.h>
#include <Pegasus/Common/CIMStatusCode.h>
#include <Pegasus/Common/AutoPtr.h>
#include "CQLValue.h"
#include "CQLIdentifier.h"
#include "CQLChainedIdentifier.h"
#include "Cql2Dnf.h"

// ATTN: TODOs -
// optimize
// documentation

PEGASUS_NAMESPACE_BEGIN

struct PropertyNode
{
    CIMName name;              // property name
    CIMName scope;             // class the property is on
    Boolean wildcard;          // true if this property is wildcarded
    Boolean endpoint;          // true if this property is an endpoint
                             // of a chained identifier
    AutoPtr<PropertyNode> sibling;
    AutoPtr<PropertyNode> firstChild;

    PropertyNode()
        : wildcard(false),
            endpoint(false),
            sibling(NULL),
            firstChild(NULL)
    {}

    ~PropertyNode() {}
};


CQLSelectStatementRep::CQLSelectStatementRep()
    :SelectStatementRep(),
    _hasWhereClause(false),
    _contextApplied(false)
{
    PEG_METHOD_ENTER (TRC_CQL, "CQLSelectStatementRep()");
    PEG_METHOD_EXIT();
}

CQLSelectStatementRep::CQLSelectStatementRep(
    const String& inQlang,
    const String& inQuery,
    const QueryContext& inCtx)
    :SelectStatementRep(inQlang, inQuery, inCtx),
    _hasWhereClause(false),
    _contextApplied(false)
{
    PEG_METHOD_ENTER (TRC_CQL, "CQLSelectStatementRep(inQlang,inQuery,inCtx)");
    PEG_METHOD_EXIT();
}

CQLSelectStatementRep::CQLSelectStatementRep(
    const String& inQlang,
    const String& inQuery)
    :SelectStatementRep(inQlang, inQuery),
    _hasWhereClause(false),
    _contextApplied(false)
{
    PEG_METHOD_ENTER (TRC_CQL, "CQLSelectStatementRep(inQlang,inQuery)");
    PEG_METHOD_EXIT();
}

CQLSelectStatementRep::CQLSelectStatementRep(const CQLSelectStatementRep& rep)
    :SelectStatementRep(rep),
    _selectIdentifiers(rep._selectIdentifiers),
    _hasWhereClause(rep._hasWhereClause),
    _predicate(rep._predicate),
    _contextApplied(rep._contextApplied)
{
    PEG_METHOD_ENTER (TRC_CQL, "CQLSelectStatementRep(rep)");
    PEG_METHOD_EXIT();
}

CQLSelectStatementRep::~CQLSelectStatementRep()
{
    PEG_METHOD_ENTER (TRC_CQL, "~CQLSelectStatementRep()");
    PEG_METHOD_EXIT();
}

CQLSelectStatementRep& CQLSelectStatementRep::operator=(
        const CQLSelectStatementRep& rhs)
{
    PEG_METHOD_ENTER (TRC_CQL, "CQLSelectStatementRep::operator=");

    if (this ==  &rhs)
    {
        PEG_METHOD_EXIT();
        return *this;
    }

    SelectStatementRep::operator=(rhs);

    _selectIdentifiers = rhs._selectIdentifiers;
    _predicate = rhs._predicate;
    _contextApplied = rhs._contextApplied;
    _hasWhereClause = rhs._hasWhereClause;

    PEG_METHOD_EXIT();
    return *this;
}

Boolean CQLSelectStatementRep::evaluate(const CIMInstance& inCI)
{
    PEG_METHOD_ENTER (TRC_CQL, "CQLSelectStatementRep::evaluate");

    CheckQueryContext();

    // Apply to class contexts to the identifiers.
    // This will check for a well-formed statement.
    if (!_contextApplied)
        applyContext();

    // Make sure the type of instance passed in is the FROM class,
    // or a subclass of the FROM class.
    if (!isFromChild(inCI.getClassName()))
    {
        PEG_METHOD_EXIT();
        return false;
    }

    if (!hasWhereClause())
    {
        PEG_METHOD_EXIT();
        return true;
    }

    PEG_METHOD_EXIT();
    return _predicate.evaluate(inCI, *_ctx);
}

void CQLSelectStatementRep::applyProjection(
    CIMInstance& inCI,
    Boolean allowMissing)
{
    PEG_METHOD_ENTER (TRC_CQL, "CQLSelectStatementRep::applyProjection(inCI)");

    CheckQueryContext();

    // Apply to class contexts to the identifiers.
    // This will check for a well-formed statement.
    if (!_contextApplied)
        applyContext();

    //
    // Build a tree to represent the projected properties from the select list
    // of chained identifiers.  This is needed because embedded instances below
    // the FROM class form a tree structure when projected.
    //
    // The design of the tree is to gather all the required properties for
    // an instance at a node as child nodes.  The root node
    // of the tree represents the instance passed in to this function.
    // Below the
    // root there can be nodes that are required embedded instance properties.
    // The child nodes of these embedded instance nodes represent the required
    // properties on the embedded instance (which may themselves be embedded
    // instances).
    //
    // Each node has a name, which is in 2 parts -- the property name and the
    // scope (ie. the class the property is on).  This allows the scoping
    // operator to be handled correctly, so that the parent instance can be
    // checked to see if it is the right class to provide the property.
    // Note that the scoping is a base class; ie. the parent instance of a node
    // may be a subclass of the scope.
    //

    // Set up the root node of the tree.  This represents the instance
    // passed in.
    AutoPtr<PropertyNode> rootNode(new PropertyNode);
    Array<QueryIdentifier> fromList = _ctx->getFromList();
    rootNode->name = fromList[0].getName();  // not doing joins
    rootNode->scope = fromList[0].getName(); // not used on root, just var fill
    rootNode->wildcard = false;

    // Build the tree below the root.
    for (Uint32 i = 0; i < _selectIdentifiers.size(); i++)
    {
        PEG_TRACE((TRC_CQL, Tracer::LEVEL4,"select chained id = %s",
            (const char*)_selectIdentifiers[i].toString().getCString()));

        // Get the chain elements
        Array<CQLIdentifier> ids = _selectIdentifiers[i].getSubIdentifiers();

        PEGASUS_ASSERT(ids.size() > 1);

        PropertyNode * curNode = rootNode.get();
        PropertyNode * curChild = curNode->firstChild.get();

        // Loop through the identifiers in the chain.
        // NOTE: this starts at the position *after* the FROM class
        // So, the loop index is always one position after the current node,
        // ie. it will become a child node of the current node.
        for (Uint32 j = 1; j < ids.size(); j++)
        {
            PEG_TRACE((TRC_CQL,Tracer::LEVEL4,"curNode = %s",
                (const char*)curNode->name.getString().getCString()));
            PEG_TRACE((TRC_CQL,Tracer::LEVEL4,"id = %s",
                (const char*)ids[j].toString().getCString()));

            // If the child is wildcarded, then every property exposed by the
            // class of the instance at the current node is required.
            // Mark the current node as wildcarded.
            if (ids[j].isWildcard())
            {
                PEG_TRACE_CSTRING (TRC_CQL, Tracer::LEVEL4,"id is wildcard");
                curNode->wildcard = true;
                break;
            }

            // Determine if this identifier is already a child node of
            // the current node.
            Boolean found = false;
            while (curChild != NULL && !found)
            {
                // The scoping class is either the scope of the identifier
                // or the FROM class if the identifier is not scoped.
                String scope = fromList[0].getName().getString();
                if (ids[j].isScoped())
                {
                  scope = ids[j].getScope();
                }

                PEG_TRACE((TRC_CQL,Tracer::LEVEL4,"scope to compare = %s",
                    (const char*)scope.getCString()));

                if (curChild->name == ids[j].getName() &&
                    String::equalNoCase(curChild->scope.getString(), scope))
                {
                    // Name and scope match.  The identifier is already
                    // child node.
                    PEG_TRACE_CSTRING (TRC_CQL,
                          Tracer::LEVEL4,"id is already a child node");
                    found = true;
                }
                else
                {
                    curChild = curChild->sibling.get();
                }
            }

            if (!found)
            {
                // The identifier is not already a child node.
                // Create a node and add it as a child to the current node.
                PEG_TRACE((TRC_CQL, Tracer::LEVEL4,"new child %s",
                    (const char*)ids[j].getName().getString().getCString()));
                curChild = new PropertyNode;
                curChild->sibling.reset(curNode->firstChild.release());
                curChild->name = ids[j].getName();
                curChild->wildcard = false;
                curChild->endpoint = false;
                // the following is safer than using th = operator
                curNode->firstChild.reset(curChild);
            }

            // Set the scope for the child node
            if (ids[j].isScoped())
            {
                // Child node has a scoping class
                PEGASUS_ASSERT(ids[j].getScope().size() > 0);
                PEG_TRACE((TRC_CQL,Tracer::LEVEL4,
                    "child set with scoping class: %s",
                    (const char*)ids[j].getScope().getCString()));
                curChild->scope =  CIMName(ids[j].getScope());
            }
            else
            {
                // Not scoped.  The scope is the FROM class.
                PEGASUS_ASSERT(j == 1);
                PEGASUS_ASSERT(fromList[0].getName().getString().size() > 0);
                PEG_TRACE((TRC_CQL,Tracer::LEVEL4,
                    "child set with scoping class: %s",
                    (const char*)fromList[0].getName()
                           .getString().getCString()));
                curChild->scope = fromList[0].getName();
            }

            // If the identifier is the last element of the chain,
            // then mark it as an endpoint
            if ((ids.size() - 1) == j)
            {
                curChild->endpoint = true;
            }

            curNode = curChild;
            curChild = curNode->firstChild.get();
        }
    }

    //
    // Do the projection.
    //

    Array<CIMName> requiredProps;
    Boolean allPropsRequired = rootNode->wildcard;

    // Loop through the children of the root node.
    // The root node represents the FROM class,
    // and the child nodes are the required properties on the FROM class.
    PropertyNode* childNode = rootNode->firstChild.get();
    while (childNode != NULL)
    {
        PEG_TRACE((TRC_CQL, Tracer::LEVEL4,"project childNode = %s",
            (const char*)childNode->name.getString().getCString()));

        // Determine if the instance passed in meets the class scoping
        // rules for the current child node.
        Boolean filterable = isFilterable(inCI, childNode);

        // Indicates if the child node is still required after the
        // recursive call.
        Boolean childRequired = true;

        // If the instance is filterable, and the child node has children,
        // or is wildcarded, then the child is assumed to be an
        // embedded instance, and we need to recurse to apply the projection
        // on the embedded instance.
        // (the check for embedded instance is done in the recursive call)
        if (filterable &&
            (childNode->firstChild.get() != NULL || childNode->wildcard))
        {
            // We need to project on an embedded instance property.
            // The steps are to remove the embedded instance property from
            // the instance passed in, project on that embedded instance
            // property, and then add the projected
            // embedded instance property back to the instance passed in.
            PEG_TRACE((TRC_CQL, Tracer::LEVEL4,"about to recurse: %s",
                (const char*)childNode->name.getString().getCString()));
            Uint32 index = inCI.findProperty(childNode->name);
            if (index != PEG_NOT_FOUND)
            {
                // The instance passed in has the required embedded instance
                // property.
                // Note: embedded instance property missing is caught below.

                // Remove the embedded instance property
                CIMProperty childProp = inCI.getProperty(index);
                inCI.removeProperty(index);

                // Project onto the embedded instance property.
                // If the last parameter is true, then the childNode
                // will not remove properties when filtering the
                // embedded instance.
                // This call returns whether the embedded instance property
                // should be added to the required property list.
                childRequired = applyProjection(childNode, childProp,
                                                allPropsRequired,
                                                allowMissing);
                inCI.addProperty(childProp);
            }
        }

        // If the instance passed in is filterable,
        // then add the current child to the list if it is still required.
        if (filterable && childRequired)
        {
          PEG_TRACE((TRC_CQL,Tracer::LEVEL4,"add req prop: %s",
              (const char*)childNode->name.getString().getCString()));
          requiredProps.append(childNode->name);
        }

    childNode = childNode->sibling.get();
    }

    // Remove the properties that are not in the projection.
    // This also checks for missing required properties.
    Boolean preserve = false;
    filterInstance(inCI,
                 allPropsRequired,
                 fromList[0].getName(),
                 requiredProps,
                 preserve,
                 allowMissing);
}

Boolean CQLSelectStatementRep::applyProjection(
    PropertyNode* node,
    CIMProperty& nodeProp,
    Boolean& preservePropsForParent,
    Boolean allowMissing) const
{
    PEG_METHOD_ENTER (TRC_CQL,
      "CQLSelectStatementRep::applyProjection(node, nodeProp)");

    PEGASUS_ASSERT(node->firstChild.get() != NULL || node->wildcard);

    //
    // The property passed in must be an embedded instance. It is not
    // allowed to project properties on embedded classes.
    // Get the embedded instance from the property.
    //

    // First check that it is an embedded object
    CIMValue nodeVal = nodeProp.getValue();
    CIMType nodeValType = nodeVal.getType();
    if (nodeValType != CIMTYPE_OBJECT
        && nodeValType != CIMTYPE_INSTANCE )
    {
        PEG_TRACE_CSTRING (TRC_CQL, Tracer::LEVEL1,"not emb");
        PEG_METHOD_EXIT();
        MessageLoaderParms parms(
            "CQL.CQLSelectStatementRep.PROP_NOT_EMB",
            "The property $0 must contain an embedded object.",
            nodeProp.getName().getString());
        throw CQLRuntimeException(parms);
    }

    if (nodeVal.isNull())
    {
        // Since we will be projecting on the embedded object, it
        //  cannot be null
        PEG_TRACE_CSTRING (TRC_CQL, Tracer::LEVEL1,"value is null");
        PEG_METHOD_EXIT();
        MessageLoaderParms parms(
            "CQL.CQLSelectStatementRep.NULL_EMB_OBJ",
            "The embedded object property $0 cannot contain a null value.",
            nodeProp.getName().getString());
        throw CQLRuntimeException(parms);
    }

    if (nodeVal.isArray() &&
      (node->firstChild.get() != NULL || node->wildcard))
    {
        // NOTE - since we are blocking projection of array elements, we can
        // assume that if we get here we were told to project a whole array
        // (ie. no index used), as
        // an embedded object with properties or wildcard.
        // Examples not allowed:
        //   SELECT fromClass.someArrayProp.scope::notAllowedProp FROM fromClass
        //   SELECT fromClass.someArrayProp.* FROM fromClass
        PEG_TRACE_CSTRING (TRC_CQL, Tracer::LEVEL1,"array index needed");
        PEG_METHOD_EXIT();
        MessageLoaderParms parms("CQL.CQLSelectStatementRep.PROJ_WHOLE_ARRAY",
                      "CQL requires that array indexing is used on embedded"
                            " object property $0.",
                       nodeProp.getName().getString());
        throw CQLRuntimeException(parms);
    }

    CIMObject nodeObj;  // this starts uninitialized
    CIMInstance nodeInst;
    if(nodeValType == CIMTYPE_OBJECT)
    {
    nodeVal.get(nodeObj);
    if (nodeObj.isUninitialized())
    {
        // Not allowed to project on an uninitialized object
        PEG_TRACE_CSTRING (TRC_CQL, Tracer::LEVEL1,"is uninitialized");
        PEG_METHOD_EXIT();
        MessageLoaderParms parms("CQL.CQLSelectStatementRep.PROJ_UNINIT",
            "The embedded object property $0 is uninitialized.",
            nodeProp.getName().getString());
        throw CQLRuntimeException(parms);
    }

    if (!nodeObj.isInstance())
    {
        // Not allowed to project on a Class
        PEG_TRACE_CSTRING (TRC_CQL, Tracer::LEVEL1,"is a class");
        PEG_METHOD_EXIT();
        MessageLoaderParms parms("CQL.CQLSelectStatementRep.PROJ_CLASS",
            "CQL does not allow properties to be projected on class $0.",
            nodeProp.getName().getString());
        throw CQLRuntimeException(parms);
    }
    nodeInst = CIMInstance(nodeObj);
    }

    else if(nodeValType == CIMTYPE_INSTANCE)
    {
        nodeVal.get(nodeInst);
        if (nodeInst.isUninitialized())
        {
            // Not allowed to project on an uninitialized object
            PEG_TRACE_CSTRING (TRC_CQL, Tracer::LEVEL1,"is uninitialized");
            PEG_METHOD_EXIT();
            MessageLoaderParms parms(
                "CQL.CQLSelectStatementRep.PROJ_UNINIT",
                "The embedded object property $0 is uninitialized.",
                nodeProp.getName().getString());
            throw CQLRuntimeException(parms);
        }
    }
    //
    // Do the projection.
    //

    Array<CIMName> requiredProps;
    Boolean allPropsRequired = node->wildcard;

    // Loop through the children of the node.
    // The node represents an embedded instance,
    // and the child nodes are the required properties on the embedded instance.
    PropertyNode * curChild = node->firstChild.get();
    while (curChild != NULL)
    {
        PEG_TRACE((TRC_CQL,Tracer::LEVEL4,"project childNode = %s",
            (const char*)curChild->name.getString().getCString()));

        // Determine if the embedded instance meets the class scoping
        // rules for the current child node
        Boolean filterable = isFilterable(nodeInst, curChild);

        // Indicates if the child node is still required after the recursive
        // call.
        Boolean childRequired = true;

        // If the embedded instance is filterable, and the child node has
        // children, or is wildcarded, then the child is assumed to be an
        // embedded instance, and we need to recurse to apply the projection
        // on the embedded instance.
        // (the check for embedded instance is done in the recursive call)
        if (filterable &&
            (curChild->firstChild.get() != NULL || curChild->wildcard))
        {
            // We need to project on an embedded instance property.
            // The steps are to remove the embedded instance property from
            // the current instance, project on that embedded instance
            // property, and then add the projected
            // embedded instance property back to the current instance.
            PEG_TRACE((TRC_CQL, Tracer::LEVEL4,"about to recurse: %s",
                (const char*)curChild->name.getString().getCString()));
            Uint32 index = nodeInst.findProperty(curChild->name);
            if (index != PEG_NOT_FOUND)
            {
                // The current instance has the required embedded instance
                // property.
                // Note: embedded instance property missing is caught below.

                // Remove the embedded instance property
                CIMProperty childProp = nodeInst.getProperty(index);
                nodeInst.removeProperty(index);

                // Project onto the embedded instance property.
                // If the last parameter is true, then the childNode
                // will not remove properties when filtering the
                // embedded instance.
                // This call returns whether the embedded instance property
                // should be added to the required property list.
                Boolean preserve =
                  node->endpoint || allPropsRequired || preservePropsForParent;
                childRequired = applyProjection(curChild, childProp, preserve,
                    allowMissing);
                nodeInst.addProperty(childProp);
            }
        }

        // If the embedded instance is filterable,
        // then add the current child to the list if it is still required.
        if (filterable && childRequired)
        {
            // The instance is filterable, add the property to the
            // required list.
            PEG_TRACE((TRC_CQL,Tracer::LEVEL4,"add req prop: %s",
                (const char*)curChild->name.getString().getCString()));
            requiredProps.append(curChild->name);
        }

    curChild = curChild->sibling.get();
    }

    // Filter the instance.
    // This removes unneeded properties, unless this
    // embedded instance node was an endpoint (last element)
    // in a chained identifier.
    // This also checks for missing required properties on the instance.
    Boolean preserveProps = node->endpoint || preservePropsForParent;
    filterInstance(nodeInst,
                 allPropsRequired,
                 nodeInst.getClassName(),
                 requiredProps,
                 preserveProps,
                 allowMissing);

    // Put the projected instance back into the property.
    if(nodeValType == CIMTYPE_INSTANCE)
    {
        nodeProp.setValue(nodeInst);
    }
    else
    {
        CIMObject newNodeObj(nodeInst);
        CIMValue newNodeVal(newNodeObj);
        nodeProp.setValue(newNodeVal);
    }

    // Indicate to the caller whether the projected instance
    // is still a required property.  It is required if it is an endpoint
    // (ie. the last element of a chained identifier)
    // OR if it still has properties after being projected
    if (node->endpoint || nodeInst.getPropertyCount() > 0)
    {
        return true;
    }

    return false;
}

Boolean CQLSelectStatementRep::isFilterable(
    const CIMInstance& inst,
    PropertyNode* node) const
{
    PEG_METHOD_ENTER (TRC_CQL, "CQLSelectStatementRep::isFilterable");
    PEG_TRACE((TRC_CQL,Tracer::LEVEL4,"instance = %s",
        (const char*)inst.getClassName().getString().getCString()));
    PEG_TRACE((TRC_CQL,Tracer::LEVEL4,"scope = %s",
        (const char*)node->scope.getString().getCString()));

    //
    // Determine if an instance is filterable for a scoped property (ie. its
    // type is the scoping class or a subclass of the scoping class where the
    // property exists)
    //
    // Note that an instance that is unfilterable is not considered
    // an error.  In CQL, an instance that is not of the required scope
    // would cause a NULL to be placed in the result set column for the
    // property. However since we are not implementing result set in stage1,
    // just skip the property.  This can lead to an instance having
    // NO required properties even though it is derived from the FROM class.
    // This can easily happen if the scoping operator is used.
    //

    Boolean filterable = false;
    if (inst.getClassName() == node->scope)
    {
        // The instance's class is the same as the required scope
        PEG_TRACE_CSTRING (TRC_CQL, Tracer::LEVEL4,"instance matches scope");
        filterable = true;
    }
    else
    {
        try
        {
            if (_ctx->isSubClass(node->scope, inst.getClassName()))
            {
                // The instance's class is a subclass of the required scope.
                PEG_TRACE_CSTRING (TRC_CQL,
                    Tracer::LEVEL4,"instance is subclass of scope");
                filterable = true;
            }
        }
        catch (const CIMException& ce)
        {
            if (ce.getCode() == CIM_ERR_INVALID_CLASS ||
              ce.getCode() == CIM_ERR_NOT_FOUND)
            {
                // The scoping class was not found in the schema.
                // Just swallow this error because according to the
                // spec we should be putting NULL in the result column,
                // which means skipping the property on the instance.
                PEG_TRACE_CSTRING (TRC_CQL, Tracer::LEVEL2,
                    "scope class not in schema");
            }
            else
            {
                PEG_METHOD_EXIT();
                throw;
            }
        }
    }

    PEG_METHOD_EXIT();
    return filterable;
}

void CQLSelectStatementRep::filterInstance(CIMInstance& inst,
                                           Boolean& allPropsRequired,
                                           const CIMName& allPropsClass,
                                           Array<CIMName>& requiredProps,
                                           Boolean& preserveProps,
                                           Boolean allowMissing) const
{
    PEG_METHOD_ENTER (TRC_CQL, "CQLSelectStatementRep::filterInstance");
    PEG_TRACE((TRC_CQL,Tracer::LEVEL4,"instance = %s",
        (const char*)inst.getClassName().getString().getCString()));
    PEG_TRACE((TRC_CQL,Tracer::LEVEL4,"allPropsClass = %s",
        (const char*)allPropsClass.getString().getCString()));

    // Implementation note:
    // Scoping operator before a wildcard is not allowed:
    // Example:
    // SELECT fromclass.embobj1.scope1::* FROM fromclass
    //
    // However, the following are allowed:
    // SELECT fromclass.embobj1.* FROM fromclass
    // (this means that all the properties on the class of instance embobj1
    //  are required)
    //
    // SELECT fromclass.* FROM fromclass
    // (this means that all the properties on class fromclass are required
    //  to be on the instance being projected, not including any
    //  properties on a subclass of fromclass)

    // If all properties are required (ie. wildcarded), then add
    // all the properties of allPropsClass to the required list.
    // The allPropsClass is either the FROM class or the class of an
    // embedded instance.
    if (allPropsRequired)
    {
        PEG_TRACE_CSTRING (TRC_CQL, Tracer::LEVEL4,"all props required");
        CIMClass cls = _ctx->getClass(allPropsClass);
        Array<CIMName> clsProps;
        for (Uint32 i = 0; i < cls.getPropertyCount(); i++)
        {
            if (!containsProperty(cls.getProperty(i).getName(), requiredProps))
            {
                requiredProps.append(cls.getProperty(i).getName());
            }
        }
    }

    // Find out what properties are on the instance.
    Array<CIMName> supportedProps;
    for (Uint32 i = 0; i < inst.getPropertyCount(); i++)
    {
        supportedProps.append(inst.getProperty(i).getName());
    }

    // Check that all required properties are on the instance.
    if (!allowMissing)
    {
        for (Uint32 i = 0; i < requiredProps.size(); i++)
        {
            if (!containsProperty(requiredProps[i], supportedProps))
            {
                PEG_TRACE((TRC_CQL, Tracer::LEVEL1,"missing: %s",
                    (const char*)requiredProps[i].getString().getCString()));
                PEG_METHOD_EXIT();
                MessageLoaderParms parms(
                    "CQL.CQLSelectStatementRep.PROJ_MISSING_PROP",
                    "The property $0 is missing on the instance of class $1.",
                    requiredProps[i].getString(),
                    inst.getClassName().getString());
                throw QueryRuntimePropertyException(parms);
            }
        }
    }

    // If requested, remove the properties on the instance that are not
    // required.
    if (!preserveProps)
    {
        for (Uint32 i = 0; i < supportedProps.size(); i++)
        {
            if (!containsProperty(supportedProps[i], requiredProps))
            {
                Uint32 index = inst.findProperty(supportedProps[i]);
                PEGASUS_ASSERT(index != PEG_NOT_FOUND);
                PEG_TRACE((TRC_CQL,Tracer::LEVEL4,"removing: %s",
                    (const char*)supportedProps[i].getString().getCString()));
                inst.removeProperty(index);
            }
        }
    }
    PEG_METHOD_EXIT();
}

//
// Validates that all the chained identifiers in the statement meet
// the rules in the CQL spec vs.the class definitions in the repository
//
void CQLSelectStatementRep::validate()
{
    PEG_METHOD_ENTER (TRC_CQL, "CQLSelectStatementRep::validate");

    CheckQueryContext();

    if (!_contextApplied)
        applyContext();

    for (Uint32 i = 0; i < _selectIdentifiers.size(); i++)
    {
        validateProperty(_selectIdentifiers[i]);
    }

    Array<QueryChainedIdentifier> _whereIdentifiers = _ctx->getWhereList();
    for (Uint32 i = 0; i < _whereIdentifiers.size(); i++)
    {
        validateProperty(_whereIdentifiers[i]);
    }

    PEG_METHOD_EXIT();
}

//
// Validates that the chained identifier meets all the rules in the CQL
// spec vs.the class definitions in the repository
//
void CQLSelectStatementRep::validateProperty(
    const QueryChainedIdentifier& chainId) const
{
    PEG_METHOD_ENTER (TRC_CQL, "CQLSelectStatementRep::validateProperty");

    // Note: applyContext has been called beforehand

    Array<QueryIdentifier> ids = chainId.getSubIdentifiers();

    Uint32 startingPos = 0;
    CIMName curContext;
    for (Uint32 pos = startingPos; pos < ids.size(); pos++)
    {
        // Determine the current class context
        if (ids[pos].isScoped())
        {
            // The chain element is scoped.  Use the scoping
            // class as the current context.  Note: this depends
            // on applyContext resolving the class aliases before we get here.
            curContext = CIMName(ids[pos].getScope());
        }
        else
        {
            // The chain element is not scoped.  Assume that we are
            // before a position that is required to be scoped.
            // (applyContext validates that the chained identifier
            // has scoped identifiers in the required positions).
            // The current context is the name at the first position,
            // which must be a classname (ie. right side of ISA where
            // the only element in the chain is a classname, or
            // cases where the FROM class is the first, and maybe only,
            // element of the chain).
            PEGASUS_ASSERT((pos < startingPos + 2) || ids[pos].isWildcard());
            curContext = ids[0].getName();
        }

        PEG_TRACE((TRC_CQL, Tracer::LEVEL4,"current context: %s",
            (const char*)curContext.getString().getCString()));

        // Get the class definition of the current class context
        // Note: keep this code here so that class existence is always
        // checked.  Eg. SELECT * FROM fromClass
        CIMClass classDef;
        try
        {
            classDef = _ctx->getClass(curContext);
        }
        catch (const CIMException& ce)
        {
            PEG_TRACE_CSTRING (TRC_CQL, Tracer::LEVEL1,"repository error");
            PEG_METHOD_EXIT();
            if (ce.getCode() == CIM_ERR_NOT_FOUND ||
              ce.getCode() == CIM_ERR_INVALID_CLASS)
            {
                MessageLoaderParms parms(
                    "CQL.CQLSelectStatementRep.VAL_CLASS_NOT_EXIST",
                    "The class $0 does not exist.",
                    curContext.getString());
                throw QueryValidationException(parms);
            }
            else
            {
                throw;
            }
        }

        // Now do the checks for properties existing on the current
        // class context
        // and the class relationship rules in section 5.4.1.
        // Only do these checks if the chain id has a property.
        if (pos > startingPos)
        {
            if (ids[pos].isWildcard())
            {
                // The wildcard is at the end (verified by applyContext), so
                // no checking is required at this position.
                continue;
            }

            // Determine if the property name at the current position
            // exists on the current class context.
            Uint32 propertyIndex = classDef.findProperty(ids[pos].getName());
            if (propertyIndex == PEG_NOT_FOUND)
            {
                PEG_TRACE((TRC_CQL, Tracer::LEVEL1,"prop not on context %s",
                    (const char*)ids[pos].getName().getString().getCString()));
                PEG_METHOD_EXIT();
                MessageLoaderParms parms(
                    "CQL.CQLSelectStatementRep.VAL_PROP_NOT_ON_CLASS",
                    "The property $0 does not exist on class $1.",
                    ids[pos].getName().getString(),
                    classDef.getClassName().getString());
                throw QueryMissingPropertyException(parms);
            }

            // Checking class relationship rules in section 5.4.1.
            // For validateProperties, this only applies to the first
            // property in the chain.  This is because once we get into
            // embedded properties we don't know what the class will be
            // until we have an instance.
            if ((pos == (startingPos+1)) && !curContext.equal(ids[0].getName()))
            {
                // Its the first property, and the class context is not
                // the FROM class.
                // Check the class relationship between the scoping class and
                // the FROM class.
                if (_ctx->getClassRelation(ids[0].getName(), curContext) ==
                        QueryContext::NOTRELATED)
                {
                    PEG_TRACE((TRC_CQL, Tracer::LEVEL1,
                        "scope violation for: %s",
                        (const char*)ids[0].getName()
                              .getString().getCString()));
                    PEG_METHOD_EXIT();
                    MessageLoaderParms parms(
                      "CQL.CQLSelectStatementRep.VAL_SCOPE_VIOLATION",
                      "The class $0 is not a superclass, subclass, or the same"
                      " class as $1.",
                      curContext.getString(), ids[0].getName().getString());
                    throw QueryValidationException(parms);
                }
            }

            // If the current position implies an embedded object, then
            // verify that the property is an embedded object
            if ((pos > startingPos) && (pos < (ids.size() - 1)))
            {
                CIMProperty embObj = classDef.getProperty(propertyIndex);

                if (embObj.findQualifier(PEGASUS_QUALIFIERNAME_EMBEDDEDOBJECT)
                        == PEG_NOT_FOUND)
                {
                    PEG_TRACE((TRC_CQL, Tracer::LEVEL1,"prop not emb %s",
                        (const char*)embObj.getName()
                              .getString().getCString()));
                    PEG_METHOD_EXIT();
                    MessageLoaderParms parms(
                        "CQL.CQLSelectStatementRep.PROP_NOT_EMB",
                        "The property $0 must be an embedded object.",
                        embObj.getName().getString());
                    throw QueryValidationException(parms);
                }
            }
        }
    }

  PEG_METHOD_EXIT();
}

CIMName CQLSelectStatementRep::lookupFromClass(const String& lookup) const
{
    PEG_METHOD_ENTER (TRC_CQL, "CQLSelectStatementRep::lookupFromClass");

    QueryIdentifier id = _ctx->findClass(lookup);

    PEG_METHOD_EXIT();

    return id.getName();
}

Array<CIMObjectPath> CQLSelectStatementRep::getClassPathList() const
{
    PEG_METHOD_ENTER (TRC_CQL, "CQLSelectStatementRep::getClassPathList");

    CheckQueryContext();

    Array<QueryIdentifier> ids = _ctx->getFromList();
    PEGASUS_ASSERT(ids.size() == 1);  // no joins yet

    // No wbem-uri support yet.
    CIMObjectPath path(String::EMPTY, _ctx->getNamespace(), ids[0].getName());

    Array<CIMObjectPath> paths;
    paths.append(path);

    PEG_METHOD_EXIT();

    return paths;

}

CIMPropertyList CQLSelectStatementRep::getPropertyList(
    const CIMObjectPath& inClassName)
{
    PEG_METHOD_ENTER (TRC_CQL, "CQLSelectStatementRep::getPropertyList");
    try
    {
        return getPropertyListInternal(inClassName, true, true);
    }
    catch (const CIMException& ce)
    {
        PEG_TRACE_CSTRING (TRC_CQL, Tracer::LEVEL4,"cim exception");
        PEG_METHOD_EXIT();
        if (ce.getCode() == CIM_ERR_NOT_FOUND ||
            ce.getCode() == CIM_ERR_INVALID_CLASS)
        {
            MessageLoaderParms parms(
                "CQL.CQLSelectStatementRep.GPL_CLASS_NOT_EXIST",
                "A class required to determine the property list was"
                    " not found.");
            throw CQLRuntimeException(parms);
        }
        else
        {
            throw;
        }
    }
}

CIMPropertyList CQLSelectStatementRep::getSelectPropertyList(
    const CIMObjectPath& inClassName)
{
    PEG_METHOD_ENTER (TRC_CQL, "CQLSelectStatementRep::getSelectPropertyList");
    try
    {
        return getPropertyListInternal(inClassName, true, false);
    }
    catch (const CIMException& ce)
    {
        PEG_TRACE_CSTRING (TRC_CQL, Tracer::LEVEL4,"cim exception");
        PEG_METHOD_EXIT();
        if (ce.getCode() == CIM_ERR_NOT_FOUND ||
            ce.getCode() == CIM_ERR_INVALID_CLASS)
        {
            MessageLoaderParms parms(
                "CQL.CQLSelectStatementRep.GPL_CLASS_NOT_EXIST",
                "A class required to determine the property list was"
                    " not found.");
            throw CQLRuntimeException(parms);
        }
        else
        {
            throw;
        }
    }
}

CIMPropertyList CQLSelectStatementRep::getWherePropertyList(
    const CIMObjectPath& inClassName)
{
    PEG_METHOD_ENTER (TRC_CQL, "CQLSelectStatementRep::getWherePropertyList");
    try
    {
        return getPropertyListInternal(inClassName, false, true);
    }
    catch (const CIMException& ce)
    {
        PEG_TRACE_CSTRING (TRC_CQL, Tracer::LEVEL4,"cim exception");
        PEG_METHOD_EXIT();
        if (ce.getCode() == CIM_ERR_NOT_FOUND ||
            ce.getCode() == CIM_ERR_INVALID_CLASS)
        {
            MessageLoaderParms parms(
                "CQL.CQLSelectStatementRep.GPL_CLASS_NOT_EXIST",
                "A class required to determine the property list was"
                    " not found.");
            throw CQLRuntimeException(parms);
        }
        else
        {
            throw;
        }
    }
}

CIMPropertyList CQLSelectStatementRep::getPropertyListInternal(
    const CIMObjectPath& inClassName,
    Boolean includeSelect,
    Boolean includeWhere)
{
    PEG_METHOD_ENTER (TRC_CQL,
        "CQLSelectStatementRep::getPropertyListInternal");

    CheckQueryContext();

    if (!_contextApplied)
        applyContext();

    // Get the FROM class.
    CIMName fromClass = _ctx->getFromList()[0].getName();

    // Get the classname passed in.  Note: since wbem-uri is not supported yet,
    // only use the classname part of the path.
    CIMName className = inClassName.getClassName();
    if (className.isNull())
    {
        // If the caller passed in an empty className, then the
        // FROM class is to be used.
        className = fromClass;
    }
    else
    {
        // The caller passed in some class name.  Verify that it is the FROM
        // class or a subclass of the FROM class.
        if(!(className == fromClass))
        {
            // Check if subclass of the FROM class
            if(!_ctx->isSubClass(fromClass, className))
            {
                MessageLoaderParms parms(
                    "CQL.CQLSelectStatementRep.CLASS_NOT_FROM_LIST_CLASS",
                    "Class $0 does not match the FROM class or any of its"
                        " subclasses.",
                    className.getString());
                throw CQLRuntimeException(parms);
            }
        }
    }

    Boolean isWildcard;
    Array<CIMName> reqProps;
    Array<CIMName> matchedScopes;
    Array<CIMName> unmatchedScopes;

    // Add required properties from the select list.
    if (includeSelect)
    {
        for (Uint32 i = 0; i < _selectIdentifiers.size(); i++)
        {
            isWildcard = addRequiredProperty(reqProps,
                                           className,
                                           _selectIdentifiers[i],
                                           matchedScopes,
                                           unmatchedScopes);

            if (isWildcard)
            {
                // This indicates that a wildcard was found,
                // and the class passed in was the FROM class.
                // Return null property list to indicate all
                // properties required.
                return CIMPropertyList();
            }
        }
    }

    // Add required properties from the WHERE clause.
    if (includeWhere)
    {
        Array<QueryChainedIdentifier> _whereIdentifiers = _ctx->getWhereList();
        for (Uint32 i = 0; i < _whereIdentifiers.size(); i++)
        {
            isWildcard = addRequiredProperty(reqProps,
                                           className,
                                           _whereIdentifiers[i],
                                           matchedScopes,
                                           unmatchedScopes);

            // Wildcards are not allowed in the WHERE clause
            PEGASUS_ASSERT(!isWildcard);
        }
    }

    // Check if every property on the class is required.
    CIMClass theClass = _ctx->getClass(className);
    Uint32 propCnt = theClass.getPropertyCount();
    Boolean allProps = true;
    for (Uint32 i = 0; i < propCnt; i++)
    {
        if (!containsProperty(theClass.getProperty(i).getName(), reqProps))
        {
            allProps = false;
            break;
        }
    }

    if (allProps)
    {
        // Return null property list to indicate all properties are required.
        PEG_TRACE_CSTRING (TRC_CQL, Tracer::LEVEL4,"all props req");
        PEG_METHOD_EXIT();
        return CIMPropertyList();
    }
    else
    {
        // Return the required property list.
        // Note that it is possible to return
        // an empty list in the case of no required properties for the classname
        // passed in.  This can happen when the scoping operator is used.
        PEG_METHOD_EXIT();
        return CIMPropertyList(reqProps);
    }
}

Boolean CQLSelectStatementRep::addRequiredProperty(
    Array<CIMName>& reqProps,
    const CIMName& className,
    const QueryChainedIdentifier& chainId,
    Array<CIMName>& matchedScopes,
    Array<CIMName>& unmatchedScopes) const
{
    PEG_METHOD_ENTER (TRC_CQL, "CQLSelectStatementRep::addRequiredProperty");

    //
    // Implementation notes:
    // This function does not look for required properties on embedded objects.
    // This function assumes that applyContext has been called.
    //

    Array<QueryIdentifier> ids = chainId.getSubIdentifiers();

    // After applyContext has been called, a single element
    // chained identifier refers to either an instance of the
    // FROM class, or is the classname on the right side of ISA.
    if (ids.size() == 1)
    {
        // This identifier is not a property name
        PEG_METHOD_EXIT();
        return false;
    }

    PEG_TRACE((TRC_CQL, Tracer::LEVEL4,"id[1] = %s",
        (const char*)ids[1].toString().getCString()));

    if (ids[1].isSymbolicConstant())
    {
        // Non-embedded symbolic constants are not properties
        // Note that an embedded symbolic constant like this:
        // fromclass.embobj.scope::someprop#'ok'
        // implies that embobj is a required property, because
        // embobj could be null, and that affects how the
        // identifier is evaluated.
        PEG_METHOD_EXIT();
        return false;
    }

    // Since applyContext has been called, the first chain element
    // will be the FROM class, so go to the 2nd chain element.
    if (ids[1].isScoped())
    {
        // The 2nd chain element is a scoped property.
        // Eg. fromclass.someclass::someprop

        // Determine the class that the property is being scoped to.
        // This could be the FROM class, or some other class not in
        // the FROM list
        CIMName scopingClass = CIMName(ids[1].getScope());

        // Check if the scoping class is the same as the class passed in.
        if (scopingClass == className)
        {
            // The scoping class is the same as the class passed,
            // add the property if not already added
            if (!containsProperty(ids[1].getName(), reqProps))
            {
                reqProps.append(ids[1].getName());
            }
        }
        else
        {
            // The scoping class is not the same as the class passed in.
            // Check if we already know that the scoping class is a subclass
            // of the class passed in
            if (containsProperty(scopingClass, unmatchedScopes))
            {
                // Scoping class is a subclass.
                PEG_TRACE_CSTRING(TRC_CQL, Tracer::LEVEL4,
                    "scoping class is a subclass");
                PEG_METHOD_EXIT();
                return false;
            }

            // Check if we already know that the scoping class is a superclass
            // of the class passed in
            Boolean isSuper = false;
            if (containsProperty(scopingClass, matchedScopes))
            {
                // Scoping class is a superclass.
                isSuper = true;
            }

            // Check if the scoping class is a superclass of the class passed in
            if (isSuper || _ctx->isSubClass(scopingClass, className))
            {
                PEG_TRACE_CSTRING (TRC_CQL,
                    Tracer::LEVEL4,"scoping class is a superclass");

            // Scoping class is a superclass of the class passed in.
            if (!isSuper)
            {
                // Save this information
                matchedScopes.append(scopingClass);
            }

            // Add to the required property list if not already there.
            if (!containsProperty(ids[1].getName(), reqProps))
            {
                reqProps.append(ids[1].getName());
            }
            }
            else
            {
                PEG_TRACE_CSTRING (TRC_CQL,
                    Tracer::LEVEL4,"scoping class is NOT a superclass");

                // Scoping class is not superclass of class passed in.
                // Save this information.
                unmatchedScopes.append(scopingClass);
            }
        }  // end else scoping class not == class passed in
    }  // end if first id is scoped
    else
    {
        // The 2nd chain element is an unscoped property
        // Check if it is wildcarded
        if (ids[1].isWildcard())
        {
            // Wildcard.
            // If the class passed in is the FROM class, then
            // all properties are required on the class passed in.
            CIMName fromClassName = _ctx->getFromList()[0].getName();
            if (fromClassName == className)
            {
                PEG_TRACE_CSTRING (TRC_CQL, Tracer::LEVEL4,
                    "wildcard and = FROM");
                PEG_METHOD_EXIT();
                return true;
            }

            // Add all the properties on the FROM class to
            // the required property list.
            CIMClass fromClass = _ctx->getClass(fromClassName);
            for (Uint32 n = 0; n < fromClass.getPropertyCount(); n++)
            {
                // Add to the required property list if not already there.
                if (!containsProperty(fromClass.getProperty(n).getName(),
                                      reqProps))
                {
                  reqProps.append(fromClass.getProperty(n).getName());
                }
            }

            PEG_METHOD_EXIT();
            return false;
        }
        // Implementation note:
        // Since this API assumes that the class passed in
        // is the FROM class or a subclass of the FROM class,
        // AND validateProperties can be called to check if
        // unscoped properties are on the FROM class,
        // we can just add the required property because
        // it is assumed to be on the FROM class.

        // Add to the required property list if not already there.
        if (!containsProperty(ids[1].getName(), reqProps))
        {
            reqProps.append(ids[1].getName());
        }
    }

    // Indicate the required property is not a wildcard
    PEG_METHOD_EXIT();
    return false;
}

Array<CQLChainedIdentifier> CQLSelectStatementRep::getSelectChainedIdentifiers()
{
    PEG_METHOD_ENTER (TRC_CQL,
        "CQLSelectStatementRep::getSelectChainedIdentifiers");

    if (!_contextApplied)
        applyContext();

    PEG_METHOD_EXIT();

    return _selectIdentifiers;
}

Array<CQLChainedIdentifier> CQLSelectStatementRep::getWhereChainedIdentifiers()
{
    PEG_METHOD_ENTER (TRC_CQL,
        "CQLSelectStatementRep::getWhereChainedIdentifiers");

    CheckQueryContext();

    if (!_contextApplied)
        applyContext();

    Array<QueryChainedIdentifier> qChainIds = _ctx->getWhereList();
    Array<CQLChainedIdentifier> cqlChainIds;
    for (Uint32 i = 0; i < qChainIds.size(); i++)
    {
        Array<QueryIdentifier> qSubs = qChainIds[i].getSubIdentifiers();
        CQLChainedIdentifier cqlChainId;
        for (Uint32 j = 0; j < qSubs.size(); j++)
        {
          cqlChainId.append(qSubs[j]);
        }

    cqlChainIds.append(cqlChainId);
    }

    PEG_METHOD_EXIT();

    return cqlChainIds;
}

Boolean CQLSelectStatementRep::containsProperty(
    const CIMName& name,
    const Array<CIMName>& props)
{
    PEG_METHOD_ENTER (TRC_CQL, "CQLSelectStatementRep::containsProperty");

    for (Uint32 i = 0; i < props.size(); i++)
    {
        if (props[i] == name)
        {
            PEG_METHOD_EXIT();
            return true;
        }
    }

    PEG_METHOD_EXIT();
    return false;
}

//
// Checks if the classname passed in is the FROM class, or
// a subclass of the FROM class
//
Boolean CQLSelectStatementRep::isFromChild(const CIMName& className) const
{
    PEG_METHOD_ENTER (TRC_CQL, "CQLSelectStatementRep::isFromChild");

    QueryContext::ClassRelation rel =
        _ctx->getClassRelation(_ctx->getFromList()[0].getName(), className);

    PEG_METHOD_EXIT();
    return (rel == QueryContext::SAMECLASS || rel == QueryContext::SUBCLASS);
}

void CQLSelectStatementRep::appendClassPath(const CQLIdentifier& inIdentifier)
{
    PEG_METHOD_ENTER (TRC_CQL, "CQLSelectStatementRep::appendClassPath");

    CheckQueryContext();

    _ctx->insertClassPath(inIdentifier);

    PEG_METHOD_EXIT();
}

void CQLSelectStatementRep::setPredicate(const CQLPredicate& inPredicate)
{
    PEG_METHOD_ENTER (TRC_CQL, "CQLSelectStatementRep::setPredicate");
    _predicate = inPredicate;
    PEG_METHOD_EXIT();
}

CQLPredicate CQLSelectStatementRep::getPredicate() const
{
    PEG_METHOD_ENTER (TRC_CQL, "CQLSelectStatementRep::getPredicate");
    return _predicate;
}

void CQLSelectStatementRep::insertClassPathAlias(
    const CQLIdentifier& inIdentifier,
    const String& inAlias)
{
    PEG_METHOD_ENTER (TRC_CQL, "CQLSelectStatementRep::insertClassPathAlias");

    CheckQueryContext();

    _ctx->insertClassPath(inIdentifier,inAlias);

    PEG_METHOD_EXIT();
}

void CQLSelectStatementRep::appendSelectIdentifier(
    const CQLChainedIdentifier& x)
{
    PEG_METHOD_ENTER (TRC_CQL, "CQLSelectStatementRep::appendSelectIdentifier");
    _selectIdentifiers.append(x);
    PEG_METHOD_EXIT();
}

void CQLSelectStatementRep::applyContext()
{
    PEG_METHOD_ENTER (TRC_CQL, "CQLSelectStatementRep::applyContext");

    CheckQueryContext();

    for (Uint32 i = 0; i < _selectIdentifiers.size(); i++)
    {
        _selectIdentifiers[i].applyContext(*_ctx);
        checkWellFormedIdentifier(_selectIdentifiers[i], true);
    }

    if (hasWhereClause())
    {
        _predicate.applyContext(*_ctx);

        // Note: must be after call to predicate's applyContext
        Array<QueryChainedIdentifier> _whereIdentifiers = _ctx->getWhereList();
        for (Uint32 i = 0; i < _whereIdentifiers.size(); i++)
        {
            checkWellFormedIdentifier(_whereIdentifiers[i], false);
        }
    }

    _contextApplied = true;
    PEG_METHOD_EXIT();
}

void CQLSelectStatementRep::checkWellFormedIdentifier(
    const QueryChainedIdentifier& chainId,
    Boolean isSelectListId)
{
    PEG_METHOD_ENTER (TRC_CQL,
      "CQLSelectStatementRep::checkWellFormedIdentifier");

    // This function assumes that applyContext has been called.
    Array<QueryIdentifier> ids = chainId.getSubIdentifiers();

    if (ids.size() == 0)
    {
        PEG_METHOD_EXIT();
        MessageLoaderParms parms("CQL.CQLSelectStatementRep.EMPTY_CHAIN",
                                 "An empty chained identifier was found.");
        throw CQLSyntaxErrorException(parms);
    }

    PEG_TRACE((TRC_CQL, Tracer::LEVEL4,"chain = %s",
        (const char*)chainId.toString().getCString()));

    if (ids.size() == 1 && isSelectListId)
    {
        // Single element chain ids are not allow in the select list.
        // The select list can only have properties.
        PEG_METHOD_EXIT();
        MessageLoaderParms parms(
            "CQL.CQLSelectStatementRep.SINGLE_CHID_IN_SELECT",
            "A property on the FROM class must be selected.");
        throw CQLSyntaxErrorException(parms);
    }

    if (ids[0].isScoped()
        || ids[0].isWildcard()
        || ids[0].isSymbolicConstant()
        || ids[0].isArray())
    {
        // The first identifier must be a classname (it could be the FROM
        // class, or some other class for the right side of ISA)
        PEG_METHOD_EXIT();
        MessageLoaderParms parms("CQL.CQLSelectStatementRep.FIRST_ID_ILLEGAL",
                                 "The chained identifier $0 is illegal.",
                                 chainId.toString());
        throw CQLSyntaxErrorException(parms);
    }

    Uint32 startingPos = 1;
    for (Uint32 pos = startingPos; pos < ids.size(); pos++)
    {
        if (ids[pos].isArray() && isSelectListId)
        {
            PEG_METHOD_EXIT();
            MessageLoaderParms parms(
                "CQL.CQLSelectStatementRep.ARRAY_IN_SELECT",
                "The identifier $0 of $1 in the SELECT list cannot use"
                    " an array index.",
                ids[pos].toString(), chainId.toString());
            throw CQLSyntaxErrorException(parms);
        }

        if (ids[pos].isSymbolicConstant() && isSelectListId)
        {
            PEG_METHOD_EXIT();
            MessageLoaderParms parms(
                "CQL.CQLSelectStatementRep.SYMCONST_IN_SELECT",
                "The identifier $0 of $1 in the SELECT list cannot use a"
                " symbolic constant.",
                ids[pos].toString(), chainId.toString());
            throw CQLSyntaxErrorException(parms);
        }

        if (ids[pos].isSymbolicConstant() && pos != (ids.size() -1))
        {
            PEG_METHOD_EXIT();
            MessageLoaderParms parms(
                "CQL.CQLSelectStatementRep.SYMCONST_NOT_LAST",
                "The symbolic constant identifier $0 of $1 must be the"
                    " last element.",
                ids[pos].toString(), chainId.toString());
            throw CQLSyntaxErrorException(parms);
        }

        if (ids[pos].isWildcard())
        {
            if ( !isSelectListId)
            {
                PEG_METHOD_EXIT();
                MessageLoaderParms parms(
                    "CQL.CQLSelectStatementRep.WILD_IN_WHERE",
                    "The identifier $0 of $1 in the WHERE list cannot use a"
                        " wildcard.",
                    ids[pos].toString(), chainId.toString());
                throw CQLSyntaxErrorException(parms);
            }

            if ( pos != ids.size() - 1)
            {
                PEG_METHOD_EXIT();
                MessageLoaderParms parms(
                    "CQL.CQLSelectStatementRep.WILD_NOT_END",
                    "The wildcard identifier $0 of $1 must be the last"
                        " element.",
                    ids[pos].toString(), chainId.toString());
                throw CQLSyntaxErrorException(parms);
            }
        }

        if (pos > startingPos && !ids[pos].isWildcard())
        {
            if (!ids[pos].isScoped())
            {
                PEG_METHOD_EXIT();
                MessageLoaderParms parms(
                    "CQL.CQLSelectStatementRep.EMB_PROP_NOT_SCOPED",
                    "The identifier $0 of $1 must use the scope operator.",
                    ids[pos].toString(), chainId.toString());
                throw CQLSyntaxErrorException(parms);
            }
        }
    }

    PEG_METHOD_EXIT();
}

void CQLSelectStatementRep::normalizeToDOC()
{
    PEG_METHOD_ENTER (TRC_CQL, "CQLSelectStatementRep::normalizeToDOC");

    if (!_contextApplied)
        applyContext();

    if(_hasWhereClause)
    {
        Cql2Dnf DNFer(_predicate);
        _predicate = DNFer.getDnfPredicate();
    }

    PEG_METHOD_EXIT();
}

String CQLSelectStatementRep::toString() const
{
    PEG_METHOD_ENTER (TRC_CQL, "CQLSelectStatementRep::toString");

    CheckQueryContext();

    String s("SELECT ");
    for(Uint32 i = 0; i < _selectIdentifiers.size(); i++)
    {
        if((i > 0) && (i < _selectIdentifiers.size()))
        {
            s.append(",");
        }
        s.append(_selectIdentifiers[i].toString());
    }

    s.append(" ");
    s.append(_ctx->getFromString());

    if(_hasWhereClause)
    {
        s.append(" WHERE ");
        s.append(_predicate.toString());
    }

    PEG_METHOD_EXIT();
    return s;
}

void CQLSelectStatementRep::setHasWhereClause()
{
    PEG_METHOD_ENTER (TRC_CQL, "CQLSelectStatementRep::setHasWhereClause");
    _hasWhereClause = true;
    PEG_METHOD_EXIT();
}

Boolean CQLSelectStatementRep::hasWhereClause() const
{
    PEG_METHOD_ENTER (TRC_CQL, "CQLSelectStatementRep::hasWhereClause");
    return _hasWhereClause;
}

void  CQLSelectStatementRep::clear()
{
    PEG_METHOD_ENTER (TRC_CQL, "CQLSelectStatementRep::clear");

    CheckQueryContext();

    _ctx->clear();
    _hasWhereClause = false;
    _contextApplied = false;
    _predicate = CQLPredicate();
    _selectIdentifiers.clear();

    PEG_METHOD_EXIT();
}

void CQLSelectStatementRep::reportNullContext() const
{
    PEG_TRACE_CSTRING(
        TRC_CQL,
        Tracer::LEVEL1,
        "CQL Query Context not set in select statement representation.");
    MessageLoaderParms parms(
        "CQL.CQLSelectStatementRep.QUERY_CONTEXT_IS_NULL",
        "Trying to process a query with a NULL Query Context.");
    throw CQLRuntimeException(parms);
}

PEGASUS_NAMESPACE_END
