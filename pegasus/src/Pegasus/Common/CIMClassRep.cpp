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

#include "CIMClassRep.h"
#include "DeclContext.h"
#include "Resolver.h"
#include "CIMName.h"
#include "CIMQualifierNames.h"
#include "CIMScope.h"
#include <Pegasus/Common/Tracer.h>
#include <Pegasus/Common/MessageLoader.h>
#include "StrLit.h"
#include "CIMInstanceRep.h"
#include "CIMPropertyInternal.h"
#include "CIMMethodRep.h"

PEGASUS_NAMESPACE_BEGIN
PEGASUS_USING_STD;

CIMClassRep::CIMClassRep(
    const CIMName& className,
    const CIMName& superClassName)
    :
    CIMObjectRep(CIMObjectPath(String(), CIMNamespaceName(), className)),
    _superClassName(superClassName)
{
}

CIMClassRep::~CIMClassRep()
{
}

Boolean CIMClassRep::isAssociation() const
{
    Uint32 index = findQualifier(CIMQualifierNames::ASSOCIATION);

    if (index == PEG_NOT_FOUND)
        return false;

    Boolean flag;

    const CIMValue& value = getQualifier(index).getValue();

    if (value.getType() != CIMTYPE_BOOLEAN)
        return false;

    value.get(flag);
    return flag;
}

Boolean CIMClassRep::isAbstract() const
{
    Uint32 index = findQualifier(CIMQualifierNames::ABSTRACT);

    if (index == PEG_NOT_FOUND)
        return false;

    Boolean flag;
    const CIMValue& value = getQualifier(index).getValue();

    if (value.getType() != CIMTYPE_BOOLEAN)
        return false;

    value.get(flag);
    return flag;
}

void CIMClassRep::addProperty(const CIMProperty& x)
{
    if (x.isUninitialized())
        throw UninitializedObjectException();

    // Reject addition of duplicate property name:

    if (findProperty(x.getName()) != PEG_NOT_FOUND)
    {
        MessageLoaderParms parms(
            "Common.CIMClassRep.PROPERTY",
            "property \"$0\"",
            x.getName().getString());
        throw AlreadyExistsException(parms);
    }

    // Reject addition of a reference property without a referenceClassName

    if ((x.getType() == CIMTYPE_REFERENCE) &&
        (x.getReferenceClassName().isNull()))
    {
        throw TypeMismatchException();
    }

    // Add the property:

    _properties.append(x);
}

void CIMClassRep::addMethod(const CIMMethod& x)
{
    if (x.isUninitialized())
        throw UninitializedObjectException();

    // Reject duplicate method names:

    if (findMethod(x.getName()) != PEG_NOT_FOUND)
    {
        MessageLoaderParms parms(
            "Common.CIMClassRep.METHOD",
            "method \"$0\"",
            x.getName().getString());
        throw AlreadyExistsException(parms);
    }

    // Add the method:

    _methods.append(x);
}

void CIMClassRep::resolve(
    DeclContext* context,
    const CIMNamespaceName& nameSpace)
{
    PEG_METHOD_ENTER(TRC_OBJECTRESOLUTION, "CIMClassRep::resolve()");

    if (!context)
        throw NullPointer();

    PEG_TRACE((TRC_OBJECTRESOLUTION, Tracer::LEVEL4,
        "CIMClassRep::resolve  class = %s, superclass = %s",
        (const char*)_reference.getClassName().getString().getCString(),
        (const char*)_superClassName.getString().getCString()));

    if (!_superClassName.isNull())
    {
        //----------------------------------------------------------------------
        // First check to see if the super-class really exists and the
        // subclassing legal:
        //----------------------------------------------------------------------
        CIMConstClass superClass =
            context->lookupClass(nameSpace, _superClassName);

        if (superClass.isUninitialized())
            throw PEGASUS_CIM_EXCEPTION(CIM_ERR_INVALID_SUPERCLASS,
                _superClassName.getString());

        // If subclass is abstract but superclass not, throw CIM Exception

        /* ATTN:KS-24 Mar 2002 P1 - Test this and confirm that rule is correct
        if isAbstract() && !superclass.isAbstract()
            throw PEGASUS_CIM_EXCEPTION(
                CIM_ERR_INVALID_SUPERCLASS, _superClassName);
        */
        /*if (superclass.isTrueQualifier(CIMQualifierNames::TERMINAL)
            throw PEGASUS_CIM_EXCEPTION(
                CIM_ERR_INVALID_SUPERCLASS, _superClassName);
        */
        //----------------------------------------------------------------------
        // Iterate all the properties of *this* class. Resolve each one and
        // set the class-origin:
        //----------------------------------------------------------------------

        Boolean isAssociationClass = isAssociation();

        for (Uint32 i = 0, n = _properties.size(); i < n; i++)
        {
            CIMProperty& property = _properties[i];

            if (!isAssociationClass &&
                property.getValue().getType() == CIMTYPE_REFERENCE)
            {
                throw PEGASUS_CIM_EXCEPTION_L(CIM_ERR_INVALID_PARAMETER,
                    MessageLoaderParms(
                        "Common.CIMClassRep.NON_ASSOCIATION_CLASS_CONTAINS_"
                            "REFERENCE_PROPERTY",
                        "Non-association class contains reference property"));
            }

            Uint32 index = superClass.findProperty(property.getName());

            if (index == PEG_NOT_FOUND)
            {
                Resolver::resolveProperty(
                    property, context, nameSpace, false, true);
                if (property.getClassOrigin().isNull())
                {
                    property.setClassOrigin(getClassName());
                }
                property.setPropagated(false);
            }
            else
            {
                CIMConstProperty superClassProperty =
                    superClass.getProperty(index);
                Resolver::resolveProperty(property, context,
                    nameSpace, false, superClassProperty, true);
                if (property.getClassOrigin().isNull())
                {
                    property.setClassOrigin(
                        superClassProperty.getClassOrigin());
                }
            }
        }

        //----------------------------------------------------------------------
        // Now prepend all properties inherited from the super-class (that
        // are not overriden by this sub-class).
        //----------------------------------------------------------------------

        // Iterate super-class properties:

        for (Uint32 i = 0, m = 0, n = superClass.getPropertyCount(); i < n; i++)
        {
            CIMConstProperty superClassProperty = superClass.getProperty(i);

            // Find the property in *this* class; if not found, then clone and
            // insert it (setting the propagated flag). Otherwise, change
            // the class-origin and propagated flag accordingly.

            Uint32 index = findProperty(superClassProperty.getName());

            // If property exists in super class but not in this one, then
            // clone and insert it. Otherwise, the properties class
            // origin was set above.

            CIMProperty superproperty = superClassProperty.clone();

            if (index == PEG_NOT_FOUND)
            {
                superproperty.setPropagated(true);
                _properties.insert(m++, superproperty);
            }
            else
            {
                // Property Qualifiers must propagate if allowed
                // If property exists in the superclass and in the subclass,
                // then, enumerate the qualifiers of the superclass's property.
                // If a qualifier is defined on the superclass's property
                // but not on the subclass's, then add it to the subclass's
                // property's qualifier list.
                CIMProperty subproperty = _properties[index];
                for (Uint32 j = 0, qc = superproperty.getQualifierCount();
                     j < qc; j++)
                {
                    CIMQualifier superClassQualifier =
                        superproperty.getQualifier(j);
                    const CIMName name = superClassQualifier.getName();
                    if (subproperty.findQualifier(name) == PEG_NOT_FOUND)
                    {
                        subproperty.addQualifier(superClassQualifier);
                    }
                } // end iteration over superclass property's qualifiers
            }
        }

        //----------------------------------------------------------------------
        // Iterate all the methods of *this* class. Resolve each one and
        // set the class-origin:
        //----------------------------------------------------------------------

        for (Uint32 i = 0, n = _methods.size(); i < n; i++)
        {
            CIMMethod& method = _methods[i];
            Uint32 index = superClass.findMethod(method.getName());

            if (index == PEG_NOT_FOUND)
            {
                Resolver::resolveMethod(method, context, nameSpace);
                if (method.getClassOrigin().isNull())
                {
                    method.setClassOrigin(getClassName());
                }
                method.setPropagated(false);
            }
            else
            {
                CIMConstMethod superClassMethod = superClass.getMethod(index);
                Resolver::resolveMethod(
                    method, context, nameSpace, superClassMethod);
            }
        }

        //----------------------------------------------------------------------
        // Now prepend all methods inherited from the super-class (that
        // are not overriden by this sub-class).
        //----------------------------------------------------------------------

        for (Uint32 i = 0, m = 0, n = superClass.getMethodCount(); i < n; i++)
        {
            CIMConstMethod superClassMethod = superClass.getMethod(i);

            // Find the method in *this* class; if not found, then clone and
            // insert it (setting the propagated flag). Otherwise, change
            // the class-origin and propagated flag accordingly.

            Uint32 index = PEG_NOT_FOUND;
            /**********************     KS move to simpler version
            for (Uint32 j = m, n = _methods.size(); j < n; j++)
            {
                if (_methods[j].getName() == superClassMethod.getName())
                {
                    index = j;
                    break;
                }
            }

            // If method exists in super class but not in this one, then
            // clone and insert it. Otherwise, the method's class origin
            // has already been set above.

            if (index == PEG_NOT_FOUND)
            {
                CIMMethod method = superClassMethod.clone();
                method.setPropagated(true);
                _methods.insert(m++, method);
            }
            */
            if ((index = findMethod(superClassMethod.getName())) ==
                PEG_NOT_FOUND)
            {
                CIMMethod method = superClassMethod.clone();
                method.setPropagated(true);
                _methods.insert(m++, method);
            }

        }

        //----------------------------------------------------------------------
        // Validate the qualifiers of this class:
        //----------------------------------------------------------------------
        _qualifiers.resolve(
            context,
            nameSpace,
            isAssociation() ? CIMScope::ASSOCIATION : CIMScope::CLASS,
            false,
            superClass._rep->_qualifiers,
            true);
    }
    else     // No SuperClass is defined
    {
        //----------------------------------------------------------------------
        // Resolve each property:
        //----------------------------------------------------------------------

        Boolean isAssociationClass = isAssociation();

        for (Uint32 i = 0, n = _properties.size(); i < n; i++)
        {
            CIMProperty& property = _properties[i];

            if (!isAssociationClass &&
                property.getValue().getType() == CIMTYPE_REFERENCE)
            {
                throw PEGASUS_CIM_EXCEPTION_L(CIM_ERR_INVALID_PARAMETER,
                    MessageLoaderParms(
                        "Common.CIMClassRep.NON_ASSOCIATION_CLASS_CONTAINS_"
                            "REFERENCE_PROPERTY",
                        "Non-association class contains reference property"));
            }

            Resolver::resolveProperty(
                property, context, nameSpace, false, true);
            property.setClassOrigin(getClassName());
            property.setPropagated(false);
        }

        //----------------------------------------------------------------------
        // Resolve each method:
        //----------------------------------------------------------------------

        for (Uint32 i = 0, n = _methods.size(); i < n; i++)
        {
            Resolver::resolveMethod (_methods[i], context, nameSpace);
            _methods[i].setClassOrigin(getClassName());
            _methods[i].setPropagated(false);
        }

        //----------------------------------------------------------------------
        // Resolve the qualifiers:
        //----------------------------------------------------------------------

        CIMQualifierList dummy;

        _qualifiers.resolve(
            context,
            nameSpace,
            isAssociation() ? CIMScope::ASSOCIATION : CIMScope::CLASS,
            false,
            dummy,
            true);
    }
}

CIMInstance CIMClassRep::buildInstance(Boolean includeQualifiers,
    Boolean includeClassOrigin,
    const CIMPropertyList& propertyList) const
{

    // Create the new instance representation
    CIMInstanceRep* newInstanceRep = new CIMInstanceRep(
        CIMObjectPath(String::EMPTY,
                      CIMNamespaceName(),
                      _reference.getClassName()));

    // Copy qualifiers if required
    if (includeQualifiers)
    {
        for (Uint32 i = 0 ; i < getQualifierCount() ; i++)
        {
            newInstanceRep->_qualifiers.add(getQualifier(i).clone());
        }
    }

    newInstanceRep->_properties.reserveCapacity(_properties.size());

    // Copy Properties
    for (Uint32 i = 0 ; i < _properties.size() ; i++)
    {
        CIMConstProperty cp = getProperty(i);
        CIMName name = cp.getName();
        Array<CIMName> pl = propertyList.getPropertyNameArray();
        if (propertyList.isNull() || Contains(pl, name))
        {
            CIMProperty p;

            if (includeQualifiers)
            {
                p = getProperty(i).clone();
            }
            else
            {
                p = CIMProperty(cp.getName(),
                                cp.getValue(),
                                cp.getArraySize(),
                                cp.getReferenceClassName(),
                                cp.getClassOrigin(),
                                cp.getPropagated());
            }

            // Delete class origin attribute if required
            if (!includeClassOrigin)
            {
                p.setClassOrigin(CIMName());
            }

            newInstanceRep->_properties.append(p);
        }
    }

    // Create new CIMInstance from CIMInstanceRep
    CIMInstance newInstance(newInstanceRep);

    return newInstance;
}

CIMClassRep::CIMClassRep(const CIMClassRep& x) :
    CIMObjectRep(x),
    _superClassName(x._superClassName)
{
    _methods.reserveCapacity(x._methods.size());

    for (Uint32 i = 0, n = x._methods.size(); i < n; i++)
        _methods.append(x._methods[i].clone());
}

Boolean CIMClassRep::identical(const CIMObjectRep* x) const
{
    if (!CIMObjectRep::identical(x))
        return false;

    const CIMClassRep* tmprep = dynamic_cast<const CIMClassRep*>(x);
    if (!tmprep)
        return false;

    // If the pointers are the same, the objects must be identical
    if (this == tmprep)
    {
        return true;
    }

    if (!_superClassName.equal (tmprep->_superClassName))
        return false;

    //
    // Check methods:
    //

    {
        const MethodSet& tmp1 = _methods;
        const MethodSet& tmp2 = tmprep->_methods;

        if (tmp1.size() != tmp2.size())
            return false;

        for (Uint32 i = 0, n = tmp1.size(); i < n; i++)
        {
            if (!tmp1[i].identical(tmp2[i]))
                return false;

            if (!tmp1[i].getClassOrigin().equal (tmp2[i].getClassOrigin()))
                return false;

            if (tmp1[i].getPropagated() != tmp2[i].getPropagated())
                return false;
        }
    }

    return true;
}

void CIMClassRep::getKeyNames(Array<CIMName>& keyNames) const
{
    keyNames.clear();

    for (Uint32 i = 0, n = getPropertyCount(); i < n; i++)
    {
        CIMConstProperty property = getProperty(i);

        if (CIMPropertyInternal::isKeyProperty(property))
        {
            keyNames.append(property.getName());
        }
    }
}

Boolean CIMClassRep::hasKeys() const
{
    for (Uint32 i = 0, n = getPropertyCount(); i < n; i++)
    {
        CIMConstProperty property = getProperty(i);
        if (CIMPropertyInternal::isKeyProperty(property))
        {
            return true;
        }
    }
    return false;
}

PEGASUS_NAMESPACE_END

