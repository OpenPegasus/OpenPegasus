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

#include "CIMInstanceRep.h"
#include "CIMInstance.h"
#include "CIMClassRep.h"
#include "CIMScope.h"
#include "DeclContext.h"
#include "Resolver.h"
#include "CIMName.h"
#include "Constants.h"
#include "StrLit.h"

PEGASUS_USING_STD;

PEGASUS_NAMESPACE_BEGIN

CIMInstanceRep::CIMInstanceRep(const CIMObjectPath& reference)
    : CIMObjectRep(reference)
{

}

CIMInstanceRep::~CIMInstanceRep()
{

}

void CIMInstanceRep::resolve(
    DeclContext* context,
    const CIMNamespaceName& nameSpace,
    CIMConstClass& cimClassOut,
    Boolean propagateQualifiers)
{
    // ATTN: Verify that references are initialized.

    if (!context)
        throw NullPointer();

    //----------------------------------------------------------------------
    // First obtain the class:
    //----------------------------------------------------------------------

    CIMConstClass cimClass =
        context->lookupClass(nameSpace, _reference.getClassName());

    if (cimClass.isUninitialized())
        throw PEGASUS_CIM_EXCEPTION(CIM_ERR_INVALID_CLASS,
            _reference.getClassName().getString ());

    cimClassOut = cimClass;

    //----------------------------------------------------------------------
    // Disallow instantiation of abstract classes.
    //----------------------------------------------------------------------

    if (cimClass.isAbstract())
        throw InstantiatedAbstractClass(_reference.getClassName().getString ());

    //----------------------------------------------------------------------
    // Validate and propagate qualifiers.
    //----------------------------------------------------------------------
    _qualifiers.resolve(
        context,
        nameSpace,
        (cimClass.isAssociation()) ? CIMScope::ASSOCIATION : CIMScope::CLASS,
        false,
        cimClass._rep->_qualifiers,
        propagateQualifiers);

    //----------------------------------------------------------------------
    // First iterate the properties of this instance and verify that
    // each one is defined in the class and then resolve each one.
    //----------------------------------------------------------------------

    CIMName className = cimClass.getClassName();

    for (Uint32 i = 0, n = _properties.size(); i < n; i++)
    {
        CIMProperty& property = _properties[i];

        Uint32 index = cimClass.findProperty(property.getName());

        if (index == PEG_NOT_FOUND)
        {
            //
            //  Allow addition of Creator property to Indication Subscription,
            //  Filter and Handler instances
            //
// l10n add language property support
            if (!(((className.equal
                    (CIMName (PEGASUS_CLASSNAME_INDSUBSCRIPTION))) ||
                (className.equal
                    (CIMName (PEGASUS_CLASSNAME_FORMATTEDINDSUBSCRIPTION))) ||
                (className.equal
                    (CIMName (PEGASUS_CLASSNAME_INDHANDLER_CIMXML))) ||
#ifdef  PEGASUS_ENABLE_PROTOCOL_WSMAN
                (className.equal
                    (CIMName (PEGASUS_CLASSNAME_INDHANDLER_WSMAN))) ||
#endif
                (className.equal
                    (CIMName (PEGASUS_CLASSNAME_LSTNRDST_CIMXML))) ||
                (className.equal
                    (CIMName (PEGASUS_CLASSNAME_INDHANDLER_SNMP))) ||
#ifdef  PEGASUS_ENABLE_SYSTEM_LOG_HANDLER
                (className.equal
                    (CIMName (PEGASUS_CLASSNAME_LSTNRDST_SYSTEM_LOG))) ||
#endif
#ifdef  PEGASUS_ENABLE_EMAIL_HANDLER
                (className.equal
                    (CIMName (PEGASUS_CLASSNAME_LSTNRDST_EMAIL))) ||
#endif
                (className.equal
                    (CIMName (PEGASUS_CLASSNAME_LSTNRDST_FILE))) ||

                (className.equal (CIMName (PEGASUS_CLASSNAME_INDFILTER)))) &&
                ((property.getName ().equal
                    (CIMName (PEGASUS_PROPERTYNAME_INDSUB_CREATOR))) ||
                (property.getName ().equal
                    (CIMName (PEGASUS_PROPERTYNAME_INDSUB_ACCEPTLANGS))) ||
                (property.getName ().equal
                    (CIMName (PEGASUS_PROPERTYNAME_LSTNRDST_CREATIONTIME))) ||
                (property.getName ().equal
                    (CIMName (PEGASUS_PROPERTYNAME_INDSUB_CONTENTLANGS))))))
            {
                throw NoSuchProperty(property.getName().getString ());
            }
        }
        else
        {
            // resolve the property
            Resolver::resolveProperty (property, context, nameSpace, true,
                cimClass.getProperty (index), propagateQualifiers);
        }
    }

    //----------------------------------------------------------------------
    // Inject all properties from the class that are not included in the
    // instance. Copy over the class-origin and set the propagated flag
    // to true. NOTE: The propagated flag indicates that the property
    // was not part of the property set input with the create and
    // was inherited from the default in the class (see cimxml spec sect 3.1.5)
    //----------------------------------------------------------------------

    for (Uint32 i = 0, m = 0, n = cimClass.getPropertyCount(); i < n; i++)
    {
        CIMConstProperty property = cimClass.getProperty(i);
        const CIMName& name = property.getName();

        // See if this instance already contains a property with this name:

        Boolean found = false;

        for (Uint32 j = m, s = _properties.size(); j < s; j++)
        {
            if (name.equal(_properties[j].getName()))
            {
                found = true;
                break;
            }
        }

        if (!found)
        {
            CIMProperty p;
            if (propagateQualifiers)
            {
                p = property.clone();
            }
            else
            {
                p = CIMProperty(
                    property.getName(),
                    property.getValue(),
                    property.getArraySize(),
                    property.getReferenceClassName(),
                    property.getClassOrigin(),
                    property.getPropagated());
            }
            p.setPropagated(true);
            _properties.insert(m++, p);
        }
    }
}

CIMInstanceRep::CIMInstanceRep(const CIMInstanceRep& x) : CIMObjectRep(x)
{
}

CIMObjectPath CIMInstanceRep::buildPath(
    const CIMConstClass& cimClass) const
{
    //--------------------------------------------------------------------------
    // Get class name:
    //--------------------------------------------------------------------------

    CIMName className = getClassName();

    //--------------------------------------------------------------------------
    // Get key names:
    //--------------------------------------------------------------------------

    Array<CIMName> keyNames;
    cimClass.getKeyNames(keyNames);

    if (keyNames.size() == 0)
        return CIMObjectPath("", CIMNamespaceName(), className);

    //--------------------------------------------------------------------------
    // Get type and value for each key (building up key bindings):
    //--------------------------------------------------------------------------

    Array<CIMKeyBinding> keyBindings;

    for (Uint32 i = 0, n = keyNames.size(); i < n; i++)
    {
        const CIMName& keyName = keyNames[i];

        Uint32 index = findProperty(keyName);
        if (index == PEG_NOT_FOUND)
        {
            throw NoSuchProperty(keyName.getString());
        }

        CIMConstProperty tmp = getProperty(index);
        keyBindings.append(CIMKeyBinding(keyName, tmp.getValue()));
    }

    return CIMObjectPath(String(), CIMNamespaceName(), className, keyBindings);
}
void CIMInstanceRep::filter(
    Boolean,
    Boolean,
    const CIMPropertyList&)
{
    return;
}

PEGASUS_NAMESPACE_END
