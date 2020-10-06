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

#include "CIMQualifierList.h"
#include "DeclContext.h"
#include "Resolver.h"
#include "CIMQualifierDecl.h"
#include "CIMName.h"
#include "XmlWriter.h"
#include <Pegasus/Common/Tracer.h>
#include <Pegasus/Common/MessageLoader.h>
#include "StrLit.h"
#include "ArrayIterator.h"
#include "CIMQualifierRep.h"

PEGASUS_NAMESPACE_BEGIN
PEGASUS_USING_STD;

static CIMName _KEY("Key");

CIMQualifierList::CIMQualifierList() :
                    _keyIndex(PEGASUS_ORDEREDSET_INDEX_UNKNOWN)
{

}

CIMQualifierList::~CIMQualifierList()
{

}

CIMQualifierList& CIMQualifierList::add(const CIMQualifier& qualifier)
{
    if (qualifier.isUninitialized())
        throw UninitializedObjectException();

    if (find(qualifier.getName()) != PEG_NOT_FOUND)
    {
        MessageLoaderParms parms("Common.CIMQualifierList.QUALIFIER",
            "qualifier \"$0\"",
            qualifier.getName().getString());
        throw AlreadyExistsException(parms);
    }

    _qualifiers.append(qualifier);

    // Update key index:
    if (_keyIndex == PEGASUS_ORDEREDSET_INDEX_UNKNOWN &&
            qualifier._rep->_name == _KEY)
        _keyIndex = _qualifiers.size()-1;

    return *this;
}

CIMQualifier& CIMQualifierList::getQualifier(Uint32 index)
{
    return _qualifiers[index];
}

void CIMQualifierList::removeQualifier(Uint32 index)
{
    _qualifiers.remove(index);
    _keyIndex = PEGASUS_ORDEREDSET_INDEX_UNKNOWN;
}

void CIMQualifierList::clear()
{
    _qualifiers.clear();
}

Uint32 CIMQualifierList::find(const CIMName& name) const
{
    return _qualifiers.find(name, generateCIMNameTag(name));
}

Boolean CIMQualifierList::isTrue(const CIMName& name) const
{
    Uint32 index = find(name);

    if (index == PEG_NOT_FOUND)
        return false;

    Boolean flag;
    const CIMValue& value = getQualifier(index).getValue();

    if (value.getType() != CIMTYPE_BOOLEAN)
        return false;

    value.get(flag);
    return flag;
}

void CIMQualifierList::resolve(
    DeclContext* declContext,
    const CIMNamespaceName & nameSpace,
    CIMScope scope, // Scope of the entity being resolved.
    Boolean isInstancePart,
    CIMQualifierList& inheritedQualifiers,
    Boolean propagateQualifiers)
{
    _keyIndex = PEGASUS_ORDEREDSET_INDEX_UNKNOWN;

    PEG_METHOD_ENTER(TRC_OBJECTRESOLUTION, "CIMQualifierList::resolve()");
    // For each qualifier in the qualifiers array, the following
    // is checked:
    //
    //     1. Whether it is declared (can be obtained from the declContext).
    //
    //     2. Whether it has the same type as the declaration.
    //
    //     3. Whether the qualifier is valid for the given scope.
    //
    //     4. Whether the qualifier can be overriden (the flavor is
    //        ENABLEOVERRIDE on the corresponding reference qualifier).
    //
    //     5. Whether the qualifier should be propagated to the subclass.
    //
    // If the qualifier should be overriden, then it is injected into the
    // qualifiers array (from the inheritedQualifiers array).

    for (Uint32 i = 0, n = _qualifiers.size(); i < n; i++)
    {
        CIMQualifier q = _qualifiers[i];
        //----------------------------------------------------------------------
        // 1. Check to see if it's declared.
        //----------------------------------------------------------------------
        // set this back to  CIMConstQualifierDecl
        CIMQualifierDecl qd = declContext->lookupQualifierDecl(
            nameSpace, q.getName());

        if (qd.isUninitialized())
        {
            PEG_METHOD_EXIT();
            throw UndeclaredQualifier(q.getName().getString ());
        }

        //----------------------------------------------------------------------
        // 2. Check the type and isArray.  Must be the same:
        //----------------------------------------------------------------------

        if (!(q.getType() == qd.getType() && q.isArray() == qd.isArray()))
        {
            PEG_METHOD_EXIT();
            throw BadQualifierType(q.getName().getString ());
        }

        //----------------------------------------------------------------------
        // 3. If the qualifier is the EmbeddedInstance qualifier, then check
        // that the class specified by the qualifier exists in the namespace.
        //----------------------------------------------------------------------
        if (q.getName().equal(PEGASUS_QUALIFIERNAME_EMBEDDEDINSTANCE))
        {
            String className;
            q.getValue().get(className);
            CIMClass classDef = declContext->lookupClass(nameSpace,
                CIMName(className));
            if (classDef.isUninitialized())
            {
                PEG_METHOD_EXIT();
                throw BadQualifierType(
                     PEGASUS_QUALIFIERNAME_EMBEDDEDINSTANCE.getString(),
                     className);
            }
        }

        //----------------------------------------------------------------------
        // 4. Check the scope: Must be legal for this qualifier
        //----------------------------------------------------------------------
        // ATTN:  These lines throw a bogus exception if the qualifier has
        // a valid scope (such as Scope::ASSOCIATION) which is not Scope::CLASS
        // ks Mar 2002. Reinstalled 23 March 2002 to test.

        if (!(qd.getScope().hasScope (scope)))
        {
            PEG_METHOD_EXIT();
            throw BadQualifierScope
                (qd.getName().getString (), scope.toString ());
        }

        //----------------------------------------------------------------------
        // Resolve the qualifierflavor. Since Flavors are a combination of
        // inheritance and input characteristics, resolve the inherited
        // characteristics against those provided with the creation.  If
        // there is a superclass the flavor is resolved against that
        // superclass.  If not, it is resolved against the declaration. If
        // the flavor is disableoverride and tosubclass the resolved
        // qualifier value must be identical to the original
        //----------------------------------------------------------------------
        // Test for Qualifier found in SuperClass. If found and qualifier
        // is not overridable.
        // Abstract (not Overridable and restricted) can be found in subclasses
        // Can have nonabstracts below abstracts. That is function of
        // nottosubclass Association (notOverridable and tosubclass) can be
        // found in subclasses but cannot be changed. No non-associatons below
        // associations. In other words once a class becomes an association,
        // no subclass can override that definition apparently
        // Throw exception if DisableOverride and tosubclass and different
        // value.  Gets the source from superclass if qualifier exists or from
        // declaraction.
        // If we do not throw the exception, resolve the flavor from the
        // inheritance point and resolve against current input.
        // Diableoverride is defined in the CIM Spec to mean that the value
        // cannot change.
        // The other characteristics including the flavor can change
        // apparently. Thus, we need only confirm that the value is the same
        // (2.2 pp 33).  Strange since we would think that override implies
        // that you cannot override any of the characteristics (value, type,
        // flavor, etc.) This also leaves the question of NULL or no values.
        // The implication is that we must move the value from the superclass
        // or declaration.

        Uint32 index = inheritedQualifiers.find(q.getName());

        if (index == PEG_NOT_FOUND)
        {   // Qualifier does not exist in superclass
            /* If from declaration, we can override the default value.
              However, we need some way to get the value if we have a Null.
            if (!qd.getFlavor().hasFlavor(CIMFlavor::OVERRIDABLE) &&
                qd.getFlavor().hasFlavor(CIMFlavor::TOSUBCLASS))
            {
                //throw BadQualifierOverride(q.getName());
            }
            */
            // Do not allow change from disable override to enable override.
            if ((!qd.getFlavor ().hasFlavor(CIMFlavor::OVERRIDABLE))
                  && (q.getFlavor ().hasFlavor(CIMFlavor::OVERRIDABLE)))
            {
                PEG_METHOD_EXIT();
                throw BadQualifierOverride(q.getName().getString ());
            }

            Resolver::resolveQualifierFlavor(
                q, CIMFlavor (qd.getFlavor ()));
        }
        else  // qualifier exists in superclass
        {   ////// Make Const again
            CIMQualifier iq = inheritedQualifiers.getQualifier(index);
            // don't allow change override to notoverride.
            if (!(iq.getFlavor ().hasFlavor(CIMFlavor::OVERRIDABLE))
                  && q.getFlavor ().hasFlavor (CIMFlavor::OVERRIDABLE))
            {
                PEG_METHOD_EXIT();
                throw BadQualifierOverride(q.getName().getString ());
            }

            if (!(iq.getFlavor ().hasFlavor(CIMFlavor::OVERRIDABLE))
                  && iq.getFlavor ().hasFlavor(CIMFlavor::TOSUBCLASS))
            {
                // test if values the same.
                CIMValue qv = q.getValue();
                CIMValue iqv = iq.getValue();
                if (!(qv == iqv))
                {
                    PEG_METHOD_EXIT();
                    throw BadQualifierOverride(q.getName().getString());
                }
            }

            Resolver::resolveQualifierFlavor(
                q, CIMFlavor(iq.getFlavor()));
        }
    }   // end of this objects qualifier loop

    //--------------------------------------------------------------------------
    // Propagate qualifiers to subclass or to instance that do not have
    // already have those qualifiers:
    //--------------------------------------------------------------------------

    if (propagateQualifiers)
    {
        for (Uint32 i = 0, n = inheritedQualifiers.getCount(); i < n; i++)
        {
            CIMQualifier iq = inheritedQualifiers.getQualifier(i);

            if (isInstancePart)
            {
                if (!(iq.getFlavor().hasFlavor(CIMFlavor::TOINSTANCE)))
                    continue;
            }
            else
            {
                if (!(iq.getFlavor().hasFlavor(CIMFlavor::TOSUBCLASS)))
                    continue;
            }

            // If the qualifiers list does not already contain this qualifier,
            // then propagate it (and set the propagated flag to true).  Else
            // we keep current value. Note we have already eliminated any
            // possibility that a nonoverridable qualifier can be in the list.
            if (find(iq.getName()) != PEG_NOT_FOUND)
                continue;

            CIMQualifier q = iq.clone();
            q.setPropagated(true);
            _qualifiers.insert(0, q);
        }
    }
    PEG_METHOD_EXIT();
}

void CIMQualifierList::print(PEGASUS_STD(ostream) &os) const
{
    Buffer tmp;
    for (Uint32 i = 0, n = getCount(); i < n; i++)
        XmlWriter::appendQualifierElement(tmp, _qualifiers[i]);
    os << tmp.getData() << PEGASUS_STD(endl);
}

Boolean CIMQualifierList::identical(const CIMQualifierList& x) const
{
    Uint32 count = getCount();

    if (count != x.getCount())
        return false;

    for (Uint32 i = 0; i < count; i++)
    {
        if (!_qualifiers[i].identical(x._qualifiers[i]))
            return false;
    }

    return true;
}

void CIMQualifierList::cloneTo(CIMQualifierList& x) const
{
    x._keyIndex = PEGASUS_ORDEREDSET_INDEX_UNKNOWN;
    x._qualifiers.clear();
    x._qualifiers.reserveCapacity(_qualifiers.size());

    for (Uint32 i = 0, n = _qualifiers.size(); i < n; i++)
        x._qualifiers.append(_qualifiers[i].clone());
    x._keyIndex = _keyIndex;
}

Boolean CIMQualifierList::isKey() const
{
    static Uint32 _KEY_TAG = generateCIMNameTag(_KEY);

    // Resolve key index if unresolved.

    if (_keyIndex == PEGASUS_ORDEREDSET_INDEX_UNKNOWN)
    {
        Uint32 pos = _qualifiers.find(_KEY, _KEY_TAG);
        ((CIMQualifierList*)this)->_keyIndex = int(pos);
    }

    // If no key qualifier in list, then return false (default key value).

    if (_keyIndex == PEGASUS_ORDEREDSET_INDEX_NOTFOUND)
        return false;

    // Obtain value of key qualifier.

    const CIMValue& value = _qualifiers[_keyIndex]._rep->_value;
    if (!value.isNull() &&
        value.getType() == CIMTYPE_BOOLEAN)
    {
        Boolean boolVal;
        value.get(boolVal);
        return boolVal;
    }

    return false;
}

CIMQualifierList& CIMQualifierList::addUnchecked(const CIMQualifier& qualifier)
{
    if (qualifier.isUninitialized())
        throw UninitializedObjectException();

    _qualifiers.append(qualifier);

    if (_keyIndex == PEGASUS_ORDEREDSET_INDEX_UNKNOWN &&
            qualifier._rep->_name == _KEY)
        _keyIndex = _qualifiers.size()-1;

    return *this;
}

PEGASUS_NAMESPACE_END
