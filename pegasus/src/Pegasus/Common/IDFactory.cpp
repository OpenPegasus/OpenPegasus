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

#include <new>
#include "IDFactory.h"

PEGASUS_NAMESPACE_BEGIN

IDFactory::IDFactory(Uint32 firstID) : _firstID(firstID), _nextID(_firstID)
{
}

IDFactory::~IDFactory()
{
    PEGASUS_DEBUG_ASSERT(_magic);
}

Uint32 IDFactory::getID() const
{
    PEGASUS_DEBUG_ASSERT(_magic);

    IDFactory* self = (IDFactory*)this;

    Uint32 id;

    self->_mutex.lock();
    {
        if (_pool.isEmpty())
        {
            if (self->_nextID < _firstID)
                self->_nextID = _firstID;

            id = self->_nextID++;
        }
        else
        {
            id = _pool.top();
            self->_pool.pop();
        }
    }
    self->_mutex.unlock();

    return id;
}

void IDFactory::putID(Uint32 id)
{
    PEGASUS_DEBUG_ASSERT(_magic);
    PEGASUS_DEBUG_ASSERT(id >= _firstID);

    if (id < _firstID)
        return;

    _mutex.lock();
    {
        _pool.push(id);
    }
    _mutex.unlock();
}

PEGASUS_NAMESPACE_END
