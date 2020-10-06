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

#include "CIMPropertyListRep.h"
#include "CIMPropertyList.h"
#include "OrderedSet.h"

PEGASUS_NAMESPACE_BEGIN

template<class REP>
inline void Ref(REP* rep)
{
        rep->_refCounter++;
}

template<class REP>
inline void Unref(REP* rep)
{
    if (rep->_refCounter.decAndTestIfZero())
        delete rep;
}

CIMPropertyList::CIMPropertyList()
{
    _rep = new CIMPropertyListRep();
}

CIMPropertyList::CIMPropertyList(const CIMPropertyList& x)
{
    _rep = x._rep;
    Ref(_rep);
}

static inline CIMPropertyListRep* _copyOnWriteCIMPropertyListRep(
    CIMPropertyListRep* rep)
{
    if (rep->_refCounter.get() > 1)
    {
        CIMPropertyListRep* tmpRep= new CIMPropertyListRep(*rep);
        Unref(rep);
        return tmpRep;
    }
    else
    {
        return rep;
    }
}

CIMPropertyList::CIMPropertyList(const Array<CIMName>& propertyNames)
{
    // ATTN: the following code is inefficient and problematic. besides
    // adding overhead to check for null property names, it has the
    // disadvantage of returning an error if only 1 of n properties are null
    // without informing the caller of which one. this is mainly a problem
    // with this object's interface. it should be more like CIMQualifierList,
    // which has a add() method that would validate one at a time.

    // ensure names are not null
    for (Uint32 i = 0, n = propertyNames.size(); i < n; i++)
    {
        if (propertyNames[i].isNull())
        {
            throw UninitializedObjectException();
        }
    }
    _rep = new CIMPropertyListRep();
    _rep->propertyNames = propertyNames;
    _rep->isNull = false;
}

CIMPropertyList::~CIMPropertyList()
{
    Unref(_rep);
}

void CIMPropertyList::set(const Array<CIMName>& propertyNames)
{
    // ATTN: the following code is inefficient and problematic. besides
    // adding overhead to check for null property names, it has the
    // disadvantage of returning an error if only 1 of n properties are null
    // without informing the caller of which one. this is mainly a problem
    // with this object's interface. it should be more like CIMQualifierList,
    // which has a add() method that would validate one at a time.

    // ensure names are not null
    for (Uint32 i = 0, n = propertyNames.size(); i < n; i++)
    {
        if (propertyNames[i].isNull())
        {
            throw UninitializedObjectException();
        }
    }
    _rep = _copyOnWriteCIMPropertyListRep(_rep);

    _rep->propertyNames = propertyNames;
    _rep->cimNameTags.clear();
    _rep->isNull = false;
    _rep->isCimNameTagsUpdated = false;
}

CIMPropertyList& CIMPropertyList::operator=(const CIMPropertyList& x)
{
    if (x._rep != _rep)
    {
        Unref(_rep);
        _rep = x._rep;
        Ref(_rep);
    }
    return *this;
}

void CIMPropertyList::clear()
{
    // If there is more than one reference
    // remove reference and get a new shiny empty representation
    if (_rep->_refCounter.get() > 1)
    {
        Unref(_rep);
        _rep = new CIMPropertyListRep();
    }
    else
    {
        // If there is only one reference
        // no need to copy the data, we own it
        // just clear the fields
        _rep->propertyNames.clear();
        _rep->isNull = true;
        if(_rep->isCimNameTagsUpdated)
        {
            _rep->cimNameTags.clear();
            _rep->isCimNameTagsUpdated = false;
        }
    }
}

Boolean CIMPropertyList::isNull() const
{
    return _rep->isNull;
}

Uint32 CIMPropertyList::size() const
{
    return _rep->propertyNames.size();
}

const CIMName& CIMPropertyList::operator[](Uint32 index) const
{
    return _rep->propertyNames[index];
}

Array<CIMName> CIMPropertyList::getPropertyNameArray() const
{
    return _rep->propertyNames;
}

Uint32 CIMPropertyList::getCIMNameTag(Uint32 index) const
{
    return _rep->cimNameTags[index];
}
void CIMPropertyList::append(Array<String> & propertyListArray)
{
    _rep = _copyOnWriteCIMPropertyListRep(_rep);
    Array<Uint32> cimNameTags;
    Array<CIMName> cimNameArray;
    for (Uint32 i = 0; i < propertyListArray.size(); i++)
    {
        CIMName name(propertyListArray[i]);
        Uint32 tag = generateCIMNameTag(name);
        Boolean dupFound=false;
        for(Uint32 j=0;j<cimNameTags.size();j++)
        {
            if ((tag == cimNameTags[j]) && (name == cimNameArray[j]))
            {
                dupFound = true;
                break;
            }
        }
        if(!dupFound)
        {
            cimNameTags.append(tag);
            cimNameArray.append(name);
        }
    }
    if(cimNameTags.size() != 0 )
    {
        _rep->cimNameTags.appendArray(cimNameTags);
        _rep->propertyNames = cimNameArray;
        _rep->isCimNameTagsUpdated = true;
    }
    _rep->isNull = false;
}

void CIMPropertyList::appendCIMNameTag(Uint32 nameTag)
{
    _rep = _copyOnWriteCIMPropertyListRep(_rep);
    _rep->cimNameTags.append(nameTag);
}

Boolean CIMPropertyList::contains(const CIMName& name) const
{
    Uint32 n = _rep->propertyNames.size();

    for (Uint32 i = 0; i < n; i++)
    {
        if (_rep->propertyNames[i] == name)
        {
            return true;
        }
    }

    return false;
}

Boolean CIMPropertyList::useThisProperty(const CIMName& name) const
{
    if (_rep->isNull)
    {
        return true;
    }
    return contains(name);
}

String CIMPropertyList::toString() const
{
    if (_rep->isNull)
    {
        return("NULL");
    }

    if (_rep->propertyNames.size() == 0)
    {
        return("EMPTY");
    }

    String rtn(_rep->propertyNames[0].getString());
    for (Uint32 i = 1 ; i < _rep->propertyNames.size() ; i++)
    {
        rtn.append(", ");
        rtn.append(_rep->propertyNames[i].getString());
    }
    return(rtn);
}

PEGASUS_NAMESPACE_END
