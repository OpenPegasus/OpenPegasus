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

#ifndef Pegasus_HandlerTable_h
#define Pegasus_HandlerTable_h

#include <Pegasus/Common/Config.h>
#include <Pegasus/Common/ArrayInternal.h>
#include <Pegasus/Common/InternalException.h>

#include <Pegasus/General/DynamicLibrary.h>

#include <Pegasus/Handler/CIMHandler.h>

#include <Pegasus/HandlerService/Linkage.h>

PEGASUS_NAMESPACE_BEGIN

// The handler table maintains a list of handlers which have been
// dynamically loaded. It maintains a mapping between string
// handler identifiers and handlers. Indication Processor will use the
// handler table to find handler for the purposes of request dispatching.

class PEGASUS_HANDLER_SERVICE_LINKAGE HandlerTable
{
public:

    HandlerTable();

    CIMHandler* getHandler(
        const String& handlerId,
        CIMRepository* repository);

    ~HandlerTable();

private:

    class HandlerEntry
    {
    public:
        HandlerEntry(const String& id, const String& fileName)
            : handlerId(id), handlerLibrary(fileName)
        {
        }

        // NOTE: The compiler default implementations of the copy constructor
        // and assignment operator are used for this class.

        String handlerId;
        DynamicLibrary handlerLibrary;
        CIMHandler* handler;

    private:
        HandlerEntry();
    };

    Array<HandlerEntry> _handlers;

    CIMHandler* _lookupHandler(const String& handlerId);

    CIMHandler* _loadHandler(const String& handlerId);

    ReadWriteSem _handlerTableLock;
};

PEGASUS_NAMESPACE_END

#endif /* Pegasus_HandlerTable_h */
