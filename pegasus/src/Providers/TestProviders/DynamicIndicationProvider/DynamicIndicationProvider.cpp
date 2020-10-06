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

#include <Pegasus/Common/Config.h>
#include <Pegasus/Common/Tracer.h>
#include <Pegasus/Common/System.h>
#include <Pegasus/General/MofWriter.h>

#include "DynamicIndicationProvider.h"


PEGASUS_NAMESPACE_BEGIN
PEGASUS_USING_STD;

static const String _DynamicIndicationClassName("PG_DynamicIndication");
static const String _DynamicIndicationNameSpace("root/cimv2");
static const String _DynamicIndicationMethod("sendIndication");

Uint32 previousState = 0;
Uint32 currentState = 0;


AtomicInt DynamicIndicationProvider::_refCnt(0);

static CIMOMHandle* _cimom = 0;

DynamicIndicationProvider::DynamicIndicationProvider()
    : _pHandler(0)
{
}

DynamicIndicationProvider::~DynamicIndicationProvider()
{
}

void DynamicIndicationProvider::initialize(CIMOMHandle& cimom)
{
    _cimom = &cimom;
}

void DynamicIndicationProvider::terminate()
{
    delete this;
}

// CIMInstanceProvider interface
void DynamicIndicationProvider::getInstance(
    const OperationContext& context,
    const CIMObjectPath& instanceReference,
    const Boolean includeQualifiers,
    const Boolean includeClassOrigin,
    const CIMPropertyList& propertyList,
    InstanceResponseHandler& handler)
{
    throw CIMException(
        CIM_ERR_NOT_SUPPORTED, "DynamicIndicationProvider::getInstance");
}


void DynamicIndicationProvider::enumerateInstances(
    const OperationContext& context,
    const CIMObjectPath& classReference,
    const Boolean includeQualifiers,
    const Boolean includeClassOrigin,
    const CIMPropertyList& propertyList,
    InstanceResponseHandler& handler)
{
    throw CIMException(
        CIM_ERR_NOT_SUPPORTED, "DynamicIndicationProvider::enumerateInstances");
}


void DynamicIndicationProvider::enumerateInstanceNames(
    const OperationContext& context,
    const CIMObjectPath& classReference,
    ObjectPathResponseHandler& handler)
{
    throw CIMException(
        CIM_ERR_NOT_SUPPORTED,
         "DynamicIndicationProvider::enumerateInstanceNames");
}


void DynamicIndicationProvider::createInstance(
    const OperationContext& context,
    const CIMObjectPath& instanceReference,
    const CIMInstance& instanceObject,
    ObjectPathResponseHandler& handler)
{
    throw CIMException(
       CIM_ERR_NOT_SUPPORTED, "DynamicIndicationProvider::createInstance");
}


void DynamicIndicationProvider::modifyInstance(
    const OperationContext& context,
    const CIMObjectPath& instanceReference,
    const CIMInstance& instanceObject,
    const Boolean includeQualifiers,
    const CIMPropertyList& propertyList,
    ResponseHandler& handler)
{
    throw CIMException(
        CIM_ERR_NOT_SUPPORTED, "DynamicIndicationProvider::modifyInstance");
}


void DynamicIndicationProvider::deleteInstance(
    const OperationContext& context,
    const CIMObjectPath& instanceReference,
    ResponseHandler& handler)
{
    throw CIMException(
        CIM_ERR_NOT_SUPPORTED, "DynamicIndicationProvider::deleteInstance");
}



// CIMIndicationProvider interface
void DynamicIndicationProvider::enableIndications(
    IndicationResponseHandler& handler)
{
    PEG_METHOD_ENTER(TRC_CONTROLPROVIDER,
        "DynamicIndicationProvider::enableIndications");
    _refCnt++;

    if (_refCnt.get() == 1)
    {
        _pHandler = &handler;

        _pHandler->processing();
    }

    PEG_METHOD_EXIT();
}

void DynamicIndicationProvider::disableIndications()
{
    PEG_METHOD_ENTER(TRC_CONTROLPROVIDER,
        "DynamicIndicationProvider::disableIndications");
    _refCnt--;

    if ((_refCnt.get() == 0) && (_pHandler != 0))
    {
        _pHandler->complete();

        _pHandler = 0;
    }

    PEG_METHOD_EXIT();
}

void DynamicIndicationProvider::createSubscription(
    const OperationContext& context,
    const CIMObjectPath& subscriptionName,
    const Array<CIMObjectPath>& classNames,
    const CIMPropertyList& propertyList,
    const Uint16 repeatNotificationPolicy)
{
}

void DynamicIndicationProvider::modifySubscription(
    const OperationContext& context,
    const CIMObjectPath& subscriptionName,
    const Array<CIMObjectPath>& classNames,
    const CIMPropertyList& propertyList,
    const Uint16 repeatNotificationPolicy)
{
}

void DynamicIndicationProvider::deleteSubscription(
    const OperationContext& context,
    const CIMObjectPath& subscriptionName,
    const Array<CIMObjectPath>& classNames)
{
}


// CIMMethodProvider interface
void DynamicIndicationProvider::invokeMethod(
    const OperationContext& context,
    const CIMObjectPath& objectReference,
    const CIMName& methodName,
    const Array<CIMParamValue>& inParameters,
    MethodResultResponseHandler& handler)
{
    PEG_METHOD_ENTER(TRC_CONTROLPROVIDER,
        "DynamicIndicationProvider::invokeMethod");

    Uint32 severity;
    String classname;
    String description;

    PEG_TRACE((TRC_CONTROLPROVIDER, Tracer::LEVEL4,"Class %s",
        (const char*)objectReference.toString().getCString()));
    PEG_TRACE((TRC_CONTROLPROVIDER, Tracer::LEVEL4,"Method %s",
        (const char*)methodName.getString().getCString()));

    handler.processing();

    if (String::equal(
            objectReference.getClassName().getString(),
            _DynamicIndicationClassName))
    {
        if (String::equal(methodName.getString(), _DynamicIndicationMethod))
        {
            try
            {
                ((inParameters[0]).getValue()).get(severity);
                ((inParameters[1]).getValue()).get(classname);
                ((inParameters[2]).getValue()).get(description);

                PEG_TRACE((
                    TRC_CONTROLPROVIDER,
                    Tracer::LEVEL4,
                    "severity:!%d!, classname!%s! description!%s!\n",
                    severity,
                    (const char*)classname.getCString(),
                    (const char*)description.getCString()));
            }
            catch (...)
            {
                PEG_METHOD_EXIT();
                throw CIMException(CIM_ERR_INVALID_PARAMETER);
            }
        }
        else
        {
            PEG_METHOD_EXIT();
            throw CIMException(CIM_ERR_METHOD_NOT_FOUND);
        }
    }
    else
    {
        PEG_METHOD_EXIT();
        throw CIMException(CIM_ERR_INVALID_CLASS);
    }

    sendIndication(severity, classname, description);

    unsigned int execStatus = 0;
    CIMValue cimRetValue(execStatus);
    handler.deliver(cimRetValue);
    handler.complete();

    PEG_METHOD_EXIT();
}

void DynamicIndicationProvider::sendIndication(
    int severity,
    String classname,
    String description)
{
    PEG_METHOD_ENTER(TRC_CONTROLPROVIDER,
        "DynamicIndicationProvider::sendIndications");

    CIMInstance instance(classname);
    instance.addProperty(CIMProperty(
        CIMName("SystemCreationClassName"),
        CIMValue(String("CIM_AlertIndication"))));
    instance.addProperty(CIMProperty(
        CIMName("SystemName"),
        CIMValue(System::getHostName())));
    instance.addProperty(CIMProperty(
        CIMName("AlertType"),
        CIMValue(Uint16(5))));   //Device Alert
    instance.addProperty(CIMProperty(
        CIMName("PerceivedSeverity"),
        CIMValue(severity)));
    instance.addProperty(CIMProperty(
        CIMName("Description"),
        CIMValue(description)));
    instance.addProperty(CIMProperty(
        CIMName("ProbableCause"),
        CIMValue(Uint16(1))));
    instance.addProperty(CIMProperty(
        CIMName("Trending"),
        CIMValue(Uint16(1))));
    instance.addProperty(CIMProperty(
        CIMName("TimeStamp"),
        CIMValue(CIMDateTime::getCurrentDateTime())));

    Array<CIMKeyBinding> keys;

    //you have to set the namespace
    CIMObjectPath cop(
        System::getHostName(), _DynamicIndicationNameSpace, classname, keys);
    instance.setPath(cop);

    Buffer buffer;
    MofWriter::appendInstanceElement(buffer, instance);

    PEG_TRACE_CSTRING(TRC_CONTROLPROVIDER, Tracer::LEVEL4, buffer.getData());
    PEG_TRACE_CSTRING(TRC_CONTROLPROVIDER, Tracer::LEVEL4,
        (const char*)instance.getPath().toString().getCString());

    CIMIndication cimIndication(instance);
    _pHandler->deliver(cimIndication);

    PEG_METHOD_EXIT();
}

PEGASUS_NAMESPACE_END
