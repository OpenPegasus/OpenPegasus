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

#include "ResponseHandler.h"
#include "ResponseHandlerRep.h"
#include "InternalException.h"
#include "HashTable.h"
#include "Mutex.h"

PEGASUS_NAMESPACE_BEGIN

PEGASUS_TEMPLATE_SPECIALIZATION struct HashFunc<void*>
{
    static Uint32 hash(void* x) { return Uint32((unsigned long)x) + 13; }
};

typedef HashTable<ResponseHandler*, ResponseHandlerRep*,
                  EqualFunc<void*>,
                  HashFunc<void*> > RepTable;

static RepTable repTable(512);
static Mutex repTableMutex;

ResponseHandlerRep* _newRep(
    ResponseHandler* object)
{
    ResponseHandlerRep* newRep = new ResponseHandlerRep();

    AutoMutex lock(repTableMutex);
    repTable.insert(object, newRep);
    return newRep;
}

ResponseHandlerRep* _newRep(
    ResponseHandler* object,
    const ResponseHandlerRep* rep)
{
    ResponseHandlerRep* newRep = new ResponseHandlerRep(*rep);

    AutoMutex lock(repTableMutex);
    repTable.insert(object, newRep);
    return newRep;
}

ResponseHandlerRep* _getRep(
    const ResponseHandler* object)
{
    ResponseHandlerRep* rep = 0;
    Boolean found;

    AutoMutex lock(repTableMutex);
    found = repTable.lookup(const_cast<ResponseHandler*>(object), rep);
    PEGASUS_ASSERT(found == true);
    return rep;
}

void _deleteRep(
    ResponseHandler* object)
{
    ResponseHandlerRep* rep = 0;
    Boolean found;

    AutoMutex lock(repTableMutex);
    found = repTable.lookup(object, rep);
    PEGASUS_ASSERT(found == true);
    delete rep;
    repTable.remove(object);
}


ResponseHandler::ResponseHandler()
{
    _newRep(this);
}

ResponseHandler::ResponseHandler(const ResponseHandler& handler)
{
    _newRep(this, _getRep(&handler));
}

ResponseHandler& ResponseHandler::operator=(const ResponseHandler& handler)
{
    if (&handler != this)
    {
        _deleteRep(this);
        _newRep(this, _getRep(&handler));
    }
    return *this;
}

ResponseHandler::~ResponseHandler()
{
    _deleteRep(this);
}

OperationContext ResponseHandler::getContext() const
{
    return _getRep(this)->getContext();
}

void ResponseHandler::setContext(const OperationContext & context)
{
    _getRep(this)->setContext(context);
}

PEGASUS_NAMESPACE_END
