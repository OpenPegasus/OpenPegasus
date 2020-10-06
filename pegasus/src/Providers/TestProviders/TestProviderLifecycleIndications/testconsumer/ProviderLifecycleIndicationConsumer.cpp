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

#include <Pegasus/Common/Mutex.h>
#include <cstring>
#include "ProviderLifecycleIndicationConsumer.h"

PEGASUS_NAMESPACE_BEGIN

PEGASUS_USING_STD;

#define CONSUMER_LOGFILE PEGASUS_ROOT \
   "/src/Providers/TestProviders/TestProviderLifecycleIndications/testclient" \
       "/PLIconsumer.log"


static Mutex _indicationMtx;

const char *alertValues[] = {
              "Unknown", "Other", "Provider module created",
              "Provider module deleted", "Provider module enabled",
              "Provider module disabled", "Provider module degraded",
              "Provider module with no active indication subscriptions"
                  " failed/crashed",
              "Provider module with active indication subscriptions"
                  " restarted automatically after failure",
              "Provider module group changed",
              "Provider is added to the provider module",
              "Provider is removed from the provider module",
              "Provider module(s) enabled due to CIMServer start/restart",
              "Provider module(s) disabled due to CIMServer shutdown"};

extern "C"
PEGASUS_EXPORT CIMProvider* PegasusCreateProvider(const String& providerName)
{
    if (String::equalNoCase(
        providerName,
        "ProviderLifecycleIndicationConsumer"))
    {
         return(new ProviderLifecycleIndicationConsumer());
    }
    return 0;
}

ProviderLifecycleIndicationConsumer::ProviderLifecycleIndicationConsumer()
{
}

ProviderLifecycleIndicationConsumer::~ProviderLifecycleIndicationConsumer()
{
}

void ProviderLifecycleIndicationConsumer::initialize(CIMOMHandle& handle)
{
}

void ProviderLifecycleIndicationConsumer::terminate()
{
    delete this;
}

void ProviderLifecycleIndicationConsumer::invokeMethod(
        const OperationContext & context,
        const CIMObjectPath & objectReference,
        const CIMName & methodName,
        const Array<CIMParamValue> & inParameters,
        MethodResultResponseHandler & handler)
{
    handler.processing();
    handler.deliver(CIMValue((Uint32)0));
    handler.complete();
}

void ProviderLifecycleIndicationConsumer::consumeIndication(
    const OperationContext & context,
    const String& url,
    const CIMInstance& indicationInstance)
{
    AutoMutex mtx(_indicationMtx);

    FILE *indicationLogHandle = fopen(CONSUMER_LOGFILE, "a+");
    if (indicationLogHandle == NULL)
    {
        return;
    }

    Uint16 value;
    indicationInstance.getProperty(
        indicationInstance.findProperty("AlertCause")).getValue().get(value);
    String providerModule;
    String provider;
    Array<CIMObject> moduleObjects;

    if (value != 13 && value != 14)
    {
        indicationInstance.getProperty(
            indicationInstance.findProperty(
                "providerModules")).getValue().get(moduleObjects);
        CIMInstance inst(moduleObjects[0]);
        inst.getProperty(
            inst.findProperty("Name")).getValue().get(providerModule);
        
        if (value == 12 || value == 11)
        {
            indicationInstance.getProperty(
                indicationInstance.findProperty(
                    "ProviderName")).getValue().get(provider);

            fprintf(
                indicationLogHandle,
                "Alert - %s, ProviderModule - %s Provider %s\n",
                alertValues[value-1],
                (const char*)providerModule.getCString(),
                (const char*)provider.getCString());
        }
        else
        {
            fprintf(
                indicationLogHandle,
                "Alert - %s, ProviderModule - %s\n",
                alertValues[value-1],
                (const char*)providerModule.getCString());
       }
    }
    else
    {
         
        fprintf(indicationLogHandle, "Alert - %s\n", alertValues[value-1]);
    }

    fclose(indicationLogHandle);
}

PEGASUS_NAMESPACE_END
