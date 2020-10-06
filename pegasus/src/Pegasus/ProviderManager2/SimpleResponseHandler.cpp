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

#include "SimpleResponseHandler.h"

#include <Pegasus/ProviderManager2/OperationResponseHandler.h>

#include <Pegasus/Common/Tracer.h>

PEGASUS_NAMESPACE_BEGIN

//
// SimpleResponseHandler
//

SimpleResponseHandler::SimpleResponseHandler()
{
}

SimpleResponseHandler::~SimpleResponseHandler()
{
}

void SimpleResponseHandler::processing()
{
    PEG_TRACE_CSTRING(
        TRC_PROVIDERMANAGER,
        Tracer::LEVEL4,
        "SimpleResponseHandler::processing()");
    // do nothing
}

void SimpleResponseHandler::complete()
{
    PEG_TRACE_CSTRING(
        TRC_PROVIDERMANAGER,
        Tracer::LEVEL4,
        "SimpleResponseHandler::complete()");

    send(true);
}

// return the number of objects in this handler
Uint32 SimpleResponseHandler::size() const
{
    return 0;
}

// clear any objects in this handler
void SimpleResponseHandler::clear()
{
}

ContentLanguageList SimpleResponseHandler::getLanguages()
{
    PEG_TRACE_CSTRING(
        TRC_PROVIDERMANAGER,
        Tracer::LEVEL4,
        "SimpleResponseHandler: getLanguages()");

    ContentLanguageList langs;

    // Try to get the ContentLanguageList out of the
    // OperationContext in the base ResponseHandler.
    OperationContext context = getContext();

    if (context.contains(ContentLanguageListContainer::NAME))
    {
        ContentLanguageListContainer cntr =
            context.get(ContentLanguageListContainer::NAME);
        langs = cntr.getLanguages();
    }

    return langs;
}

void SimpleResponseHandler::send(Boolean isComplete)
{
    // If this was NOT instantiated as a derived OperationResponseHandle class,
    // then this will be null but is NOT an error. In this case, there is no
    // response attached, hence no data,so there is nothing to send. else we
    // have a valid "cross-cast" to the operation side

    OperationResponseHandler* operation =
        dynamic_cast<OperationResponseHandler*>(this);

    if (operation)
    {
        operation->send(isComplete);
    }
}

//
// SimpleInstanceResponseHandler
//

SimpleInstanceResponseHandler::SimpleInstanceResponseHandler()
{
}

SimpleInstanceResponseHandler::~SimpleInstanceResponseHandler()
{
}

void SimpleInstanceResponseHandler::processing()
{
    SimpleResponseHandler::processing();
}

void SimpleInstanceResponseHandler::complete()
{
    SimpleResponseHandler::complete();
}

Uint32 SimpleInstanceResponseHandler::size() const
{
    return _objects.size()+_scmoObjects.size();
}

void SimpleInstanceResponseHandler::clear()
{
    _objects.clear();
    _scmoObjects.clear();
}

void SimpleInstanceResponseHandler::deliver(const CIMInstance& instance)
{
    PEG_TRACE_CSTRING(
        TRC_PROVIDERMANAGER,
        Tracer::LEVEL4,
        "SimpleInstanceResponseHandler::deliver()");

    _objects.append(instance);

    send(false);
}

void SimpleInstanceResponseHandler::deliver(const SCMOInstance& instance)
{
    PEG_TRACE_CSTRING(
        TRC_PROVIDERMANAGER,
        Tracer::LEVEL4,
        "SimpleInstanceResponseHandler::deliver(SCMOInstance)");

    //fprintf(stderr, "SimpleInstanceResponseHandler::deliver\n");
    _scmoObjects.append(instance);

    send(false);
}

void SimpleInstanceResponseHandler::deliver(const Array<CIMInstance>& instances)
{
    // call deliver for each object in the array
    for (Uint32 i = 0, n = instances.size(); i < n; i++)
    {
        deliver(instances[i]);
    }
}

const Array<CIMInstance> SimpleInstanceResponseHandler::getObjects() const
{
    return _objects;
}

const Array<SCMOInstance> SimpleInstanceResponseHandler::getSCMOObjects() const
{
    return _scmoObjects;
}

//
// SimpleObjectPathResponseHandler
//

SimpleObjectPathResponseHandler::SimpleObjectPathResponseHandler()
{
}

SimpleObjectPathResponseHandler::~SimpleObjectPathResponseHandler()
{
}

void SimpleObjectPathResponseHandler::processing()
{
    SimpleResponseHandler::processing();
}

void SimpleObjectPathResponseHandler::complete()
{
    SimpleResponseHandler::complete();
}

Uint32 SimpleObjectPathResponseHandler::size() const
{
    return _objects.size() + _scmoObjects.size();
}

void SimpleObjectPathResponseHandler::clear()
{
    _objects.clear();
    _scmoObjects.clear();
}

void SimpleObjectPathResponseHandler::deliver(const CIMObjectPath& objectPath)
{
    PEG_TRACE_CSTRING(
        TRC_PROVIDERMANAGER,
        Tracer::LEVEL4,
        "SimpleObjectPathResponseHandler::deliver()");

    _objects.append(objectPath);

    send(false);
}

void SimpleObjectPathResponseHandler::deliver(const SCMOInstance& objectPath)
{
    PEG_TRACE_CSTRING(
        TRC_PROVIDERMANAGER,
        Tracer::LEVEL4,
        "SimpleObjectPathResponseHandler::deliver()");

    _scmoObjects.append(objectPath);

    send(false);
}

void SimpleObjectPathResponseHandler::deliver(
    const Array<CIMObjectPath>& objectPaths)
{
    // call deliver for each object in the array
    for (Uint32 i = 0, n = objectPaths.size(); i < n; i++)
    {
        deliver(objectPaths[i]);
    }
}

const Array<CIMObjectPath> SimpleObjectPathResponseHandler::getObjects() const
{
    return _objects;
}

const Array<SCMOInstance>
SimpleObjectPathResponseHandler::getSCMOObjects() const
{
    return _scmoObjects;
}

//
// SimpleMethodResultResponseHandler
//

SimpleMethodResultResponseHandler::SimpleMethodResultResponseHandler()
{
}

void SimpleMethodResultResponseHandler::processing()
{
    SimpleResponseHandler::processing();
}

void SimpleMethodResultResponseHandler::complete()
{
    SimpleResponseHandler::complete();
}

Uint32 SimpleMethodResultResponseHandler::size() const
{
    return _objects.size();
}

void SimpleMethodResultResponseHandler::clear()
{
    _objects.clear();

    _returnValue.clear();
}

void SimpleMethodResultResponseHandler::deliverParamValue(
    const CIMParamValue& outParamValue)
{
    _objects.append(outParamValue);

    // async delivers not yet supported for parameters
    //send(false);
}

void SimpleMethodResultResponseHandler::deliverParamValue(
    const Array<CIMParamValue>& outParamValues)
{
    // call deliver for each object in the array
    for (Uint32 i = 0, n = outParamValues.size(); i < n; i++)
    {
        deliverParamValue(outParamValues[i]);
    }
}

void SimpleMethodResultResponseHandler::deliver(const CIMValue& returnValue)
{
    PEG_TRACE_CSTRING(
        TRC_PROVIDERMANAGER,
        Tracer::LEVEL4,
        "SimpleMethodResultResponseHandler::deliver()");

    _returnValue = returnValue;
    // async delivers are not supported for returnValues and parameters
    //send(false);
}

const Array<CIMParamValue>
    SimpleMethodResultResponseHandler::getParamValues() const
{
    return _objects;
}

const CIMValue SimpleMethodResultResponseHandler::getReturnValue() const
{
    return _returnValue;
}

//
// SimpleIndicationResponseHandler
//

SimpleIndicationResponseHandler::SimpleIndicationResponseHandler()
{
}

void SimpleIndicationResponseHandler::processing()
{
    SimpleResponseHandler::processing();
}

void SimpleIndicationResponseHandler::complete()
{
    SimpleResponseHandler::complete();
}

Uint32 SimpleIndicationResponseHandler::size() const
{
    return _objects.size();
}

void SimpleIndicationResponseHandler::clear()
{
    _objects.clear();
}

void SimpleIndicationResponseHandler::deliver(const CIMIndication& indication)
{
    PEG_TRACE_CSTRING(
        TRC_PROVIDERMANAGER,
        Tracer::LEVEL4,
        "SimpleIndicationResponseHandler::deliver()");

    _objects.append(indication);

    send(false);
}

void SimpleIndicationResponseHandler::deliver(
    const Array<CIMIndication>& indications)
{
    // call deliver for each object in the array
    for (Uint32 i = 0, n = indications.size(); i < n; i++)
    {
        deliver(indications[i]);
    }
}

void SimpleIndicationResponseHandler::deliver(
    const OperationContext& context,
    const CIMIndication& indication)
{
    PEG_TRACE_CSTRING(
        TRC_PROVIDERMANAGER,
        Tracer::LEVEL4,
        "SimpleIndicationResponseHandler::deliver()");

    _objects.append(indication);
}

void SimpleIndicationResponseHandler::deliver(
    const OperationContext& context,
    const Array<CIMIndication>& indications)
{
    // call deliver for each object in the array
    for (Uint32 i = 0, n = indications.size(); i < n; i++)
    {
        deliver(indications[i]);
    }
}

const Array<CIMIndication> SimpleIndicationResponseHandler::getObjects() const
{
    return _objects;
}


//
// SimpleObjectResponseHandler
//

SimpleObjectResponseHandler::SimpleObjectResponseHandler()
{
}

void SimpleObjectResponseHandler::processing()
{
    SimpleResponseHandler::processing();
}

void SimpleObjectResponseHandler::complete()
{
    SimpleResponseHandler::complete();
}

Uint32 SimpleObjectResponseHandler::size() const
{
    return _objects.size()+_scmoObjects.size();
}

void SimpleObjectResponseHandler::clear()
{
    _objects.clear();
    _scmoObjects.clear();
}

void SimpleObjectResponseHandler::deliver(const CIMObject& object)
{
    PEG_TRACE_CSTRING(
        TRC_PROVIDERMANAGER,
        Tracer::LEVEL4,
        "SimpleObjectResponseHandler::deliver()");

    _objects.append(object);

    send(false);
}

void SimpleObjectResponseHandler::deliver(const CIMInstance& instance)
{
    PEG_TRACE_CSTRING(
        TRC_PROVIDERMANAGER,
        Tracer::LEVEL4,
        "SimpleObjectResponseHandler::deliver()");

    _objects.append(instance);

    send(false);
}

void SimpleObjectResponseHandler::deliver(const SCMOInstance& object)
{
    PEG_TRACE_CSTRING(
        TRC_PROVIDERMANAGER,
        Tracer::LEVEL4,
        "SimpleObjectResponseHandler::deliver()");

    _scmoObjects.append(object);
    send(false);
}

void SimpleObjectResponseHandler::deliver(const Array<CIMObject>& objects)
{
    // call deliver for each object in the array
    for (Uint32 i = 0, n = objects.size(); i < n; i++)
    {
        deliver(objects[i]);
    }
}

const Array<CIMObject> SimpleObjectResponseHandler::getObjects() const
{
    return _objects;
}

const Array<SCMOInstance> SimpleObjectResponseHandler::getSCMOObjects() const
{
    return _scmoObjects;
}

//
// SimpleInstance2ObjectResponseHandler
//

SimpleInstance2ObjectResponseHandler::SimpleInstance2ObjectResponseHandler()
{
}

void SimpleInstance2ObjectResponseHandler::processing()
{
    SimpleResponseHandler::processing();
}

void SimpleInstance2ObjectResponseHandler::complete()
{
    SimpleResponseHandler::complete();
}

Uint32 SimpleInstance2ObjectResponseHandler::size() const
{
    return _objects.size() + _scmoObjects.size();
}

void SimpleInstance2ObjectResponseHandler::clear()
{
    _objects.clear();
    _scmoObjects.clear();
}

void SimpleInstance2ObjectResponseHandler::deliver(const CIMInstance& object)
{
    PEG_TRACE_CSTRING(
        TRC_PROVIDERMANAGER,
        Tracer::LEVEL4,
        "SimpleInstance2ObjectResponseHandler::deliver()");

    _objects.append(CIMObject(object));

    // async delivers not yet supported
    //send(false);
}

void SimpleInstance2ObjectResponseHandler::deliver(const SCMOInstance& object)
{
    PEG_TRACE_CSTRING(
        TRC_PROVIDERMANAGER,
        Tracer::LEVEL4,
        "SimpleInstance2ObjectResponseHandler::deliver(SCMO)");

    _scmoObjects.append(object);

    // async delivers not yet supported
    //send(false);
}

void SimpleInstance2ObjectResponseHandler::deliver(
    const Array<CIMInstance>& objects)
{
    // call deliver for each object in the array
    for (Uint32 i = 0, n = objects.size(); i < n; i++)
    {
        deliver(objects[i]);
    }
}

const Array<CIMObject> SimpleInstance2ObjectResponseHandler::getObjects() const
{
    return _objects;
}

const Array<SCMOInstance>
SimpleInstance2ObjectResponseHandler::getSCMOObjects() const
{
    return _scmoObjects;
}

//
// SimpleValueResponseHandler
//

SimpleValueResponseHandler::SimpleValueResponseHandler()
{
}

void SimpleValueResponseHandler::processing()
{
    SimpleResponseHandler::processing();
}

void SimpleValueResponseHandler::complete()
{
    SimpleResponseHandler::complete();
}

Uint32 SimpleValueResponseHandler::size() const
{
    return _objects.size();
}

void SimpleValueResponseHandler::clear()
{
    _objects.clear();
}

void SimpleValueResponseHandler::deliver(const CIMValue& value)
{
    PEG_TRACE_CSTRING(
        TRC_PROVIDERMANAGER,
        Tracer::LEVEL4,
        "SimpleValueResponseHandler::deliver()");

    _objects.append(value);

    send(false);
}

void SimpleValueResponseHandler::deliver(const Array<CIMValue>& values)
{
    // call deliver for each object in the array
    for (Uint32 i = 0, n = values.size(); i < n; i++)
    {
        deliver(values[i]);
    }
}

const Array<CIMValue> SimpleValueResponseHandler::getObjects() const
{
    return _objects;
}

PEGASUS_NAMESPACE_END
