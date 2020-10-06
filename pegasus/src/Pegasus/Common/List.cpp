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

#include "List.h"
#include "PegasusAssert.h"

PEGASUS_NAMESPACE_BEGIN

ListRep::ListRep(void (*destructor)(Linkable*))
    : _front(0), _back(0), _size(0)
{
    if (destructor)
        _destructor = destructor;
    else
        _destructor = 0;
}

ListRep::~ListRep()
{
    PEGASUS_DEBUG_ASSERT(_magic);

    clear();
#ifdef PEGASUS_DEBUG
    memset(this, 0xDD, sizeof(ListRep));
#endif
}

void ListRep::clear()
{
    PEGASUS_DEBUG_ASSERT(_magic);

    if (_destructor)
    {
        // Reset _front, _back, and _size in case the destructor calls
        // a method of List.

        Linkable* front = _front;

        _front = 0;
        _back = 0;
        _size = 0;

        for (Linkable* p = front; p; )
        {
            PEGASUS_DEBUG_ASSERT(p->magic);
            Linkable* next = p->next;
            p->list = 0;
            _destructor(p);
            p = next;
        }
    }
}

void ListRep::insert_front(Linkable* elem)
{
    PEGASUS_DEBUG_ASSERT(_magic);
    PEGASUS_DEBUG_ASSERT(elem != 0);
    PEGASUS_DEBUG_ASSERT(elem->magic);
    PEGASUS_DEBUG_ASSERT(elem->list == 0);

    elem->list = this;
    elem->next = _front;
    elem->prev = 0;

    if (_front)
        _front->prev = elem;
    else
        _back = elem;

    _front = elem;
    _size++;
}

void ListRep::insert_back(Linkable* elem)
{
    PEGASUS_DEBUG_ASSERT(_magic);
    PEGASUS_DEBUG_ASSERT(elem != 0);
    PEGASUS_DEBUG_ASSERT(elem->magic);
    PEGASUS_DEBUG_ASSERT(elem->list == 0);

    elem->list = this;
    elem->prev = _back;
    elem->next = 0;

    if (_back)
        _back->next = elem;
    else
        _front = elem;

    _back = elem;
    _size++;
}

void ListRep::insert_after(
    Linkable* pos,
    Linkable* elem)
{
    PEGASUS_DEBUG_ASSERT(_magic);
    PEGASUS_DEBUG_ASSERT(pos != 0);
    PEGASUS_DEBUG_ASSERT(elem != 0);
    PEGASUS_DEBUG_ASSERT(elem->magic);
    PEGASUS_DEBUG_ASSERT(pos->magic);
    PEGASUS_DEBUG_ASSERT(elem->list == 0);

    elem->list = this;
    elem->prev = pos;
    elem->next = pos->next;

    if (pos->next)
        pos->next->prev = elem;

    pos->next = elem;

    if (pos == _back)
        _back = elem;

    _size++;
}

void ListRep::insert_before(
    Linkable* pos,
    Linkable* elem)
{
    PEGASUS_DEBUG_ASSERT(_magic);
    PEGASUS_DEBUG_ASSERT(pos != 0);
    PEGASUS_DEBUG_ASSERT(elem != 0);
    PEGASUS_DEBUG_ASSERT(pos->magic);
    PEGASUS_DEBUG_ASSERT(elem->magic);
    PEGASUS_DEBUG_ASSERT(elem->list == 0);

    elem->list = this;
    elem->next = pos;
    elem->prev = pos->prev;

    if (pos->prev)
        pos->prev->next = elem;

    pos->prev = elem;

    if (pos == _front)
        _front = elem;

    _size++;
}

void ListRep::remove(Linkable* pos)
{
    PEGASUS_DEBUG_ASSERT(_magic);
    PEGASUS_DEBUG_ASSERT(pos != 0);
    PEGASUS_DEBUG_ASSERT(pos->magic);
    PEGASUS_DEBUG_ASSERT(pos->list == this);
    PEGASUS_DEBUG_ASSERT(_size != 0);

    if (_size == 0)
        return;

    if (pos->prev)
        pos->prev->next = pos->next;

    if (pos->next)
        pos->next->prev = pos->prev;

    if (pos == _front)
        _front = pos->next;

    if (pos == _back)
        _back = pos->prev;

    pos->list = 0;

    _size--;
}

bool ListRep::contains(const Linkable* elem)
{
    PEGASUS_DEBUG_ASSERT(_magic);
    PEGASUS_DEBUG_ASSERT(elem != 0);
    PEGASUS_DEBUG_ASSERT(elem->magic);

    for (const Linkable* p = _front; p; p = p->next)
    {
        if (p == elem)
            return true;
    }

    // Not found!
    return false;
}

Linkable* ListRep::remove_front()
{
    PEGASUS_DEBUG_ASSERT(_magic);

    if (_size == 0)
        return 0;

    Linkable* elem = _front;
    remove(elem);

    return elem;
}

Linkable* ListRep::remove_back()
{
    PEGASUS_DEBUG_ASSERT(_magic);

    if (_size == 0)
        return 0;

    Linkable* elem = _back;
    remove(elem);

    return elem;
}

Linkable* ListRep::find(ListRep::Equal equal, const void* client_data)
{
    PEGASUS_DEBUG_ASSERT(_magic);
    PEGASUS_DEBUG_ASSERT(equal != 0);

    for (Linkable* p = _front; p; p = p->next)
    {
        if ((*equal)(p, client_data))
        {
            PEGASUS_DEBUG_ASSERT(p->magic);
            return p;
        }
    }

    // Not found!
    return 0;
}

Linkable* ListRep::remove(ListRep::Equal equal, const void* client_data)
{
    PEGASUS_DEBUG_ASSERT(_magic);
    PEGASUS_DEBUG_ASSERT(equal != 0);

    Linkable* p = find(equal, client_data);

    if (p)
    {
        PEGASUS_DEBUG_ASSERT(p->magic);
        remove(p);
    }

    return p;
}

void ListRep::apply(ListRep::Apply apply, const void* client_data)
{
    PEGASUS_DEBUG_ASSERT(_magic);
    PEGASUS_DEBUG_ASSERT(apply != 0);

    for (Linkable* p = _front; p; p = p->next)
        (*apply)(p, client_data);
}

PEGASUS_NAMESPACE_END
